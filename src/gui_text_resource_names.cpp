#include "bsp/gui_text_resource_names.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/game_native_mutable_crt_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_gui_text_shader_storage.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_render_context.hpp"
#include <initializer_list>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {

GuiTextFontRegistryAccess::GuiTextFontRegistryAccess(FontRegistryStartup* existing,
    FontRegistryStartup** publication, SingletonLifetimeManager* lifetime) noexcept
    : existing_(existing), publication_(publication), lifetime_(lifetime) {}

GuiTextFontRegistryAccess GuiTextFontRegistryAccess::published_singleton(
    FontRegistryStartup*& actual_publication, SingletonLifetimeManager& real_lifetime) noexcept {
    return GuiTextFontRegistryAccess(nullptr, &actual_publication, &real_lifetime);
}

GuiTextFontRegistryAccess GuiTextFontRegistryAccess::borrowed_existing(
    FontRegistryStartup& existing) noexcept {
    return GuiTextFontRegistryAccess(&existing, nullptr, nullptr);
}
GuiTextFontRegistryAccess GuiTextFontRegistryAccess::borrowed_native_images(
    NativeFontResourceOwners& images) noexcept {
    GuiTextFontRegistryAccess result(nullptr, nullptr, nullptr);
    result.native_images_ = &images;
    return result;
}

FontRegistryStartup* GuiTextFontRegistryAccess::get() const {
    if (native_images_)
        throw std::logic_error("image Font registry has no descriptor-name lookup authority");
    if (existing_) return existing_;
    return get_font_registry_007371d0(*publication_, *lifetime_);
}

FontRegistryStartup* GuiTextFontRegistryAccess::existing_registry() const noexcept {
    if (native_images_) return nullptr;
    return existing_ ? existing_ : *publication_;
}
void* GuiTextFontRegistryAccess::existing_native_registry_image() const noexcept {
    return native_images_ ? native_images_->native_registry_image() : nullptr;
}

GuiTextFontNameServices::GuiTextFontNameServices(GuiTextResourceNameServices& names_in,
    GuiTextFontRegistryAccess registry_in, NativeFontResourceOwners& fonts_in,
    const volatile float& one_in) noexcept
    : names(names_in), registry(registry_in), fonts(fonts_in), one_00d7a24c(one_in) {}

GuiTextFontNameServices::GuiTextFontNameServices(GuiTextResourceNameServices& names_in,
    FontRegistryStartup*& publication, SingletonLifetimeManager& lifetime,
    NativeFontResourceOwners& fonts_in, const volatile float& one_in) noexcept
    : GuiTextFontNameServices(names_in,
        GuiTextFontRegistryAccess::published_singleton(publication, lifetime), fonts_in, one_in) {}

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void require_owner(GuiTextLifetime& lifetime, GuiTextResourceNameServices& services) {
    auto binding = lifetime.content_binding();
    require(binding.widget.layout().type == GuiWidgetType::Text &&
        binding.widget.layout().transform.type_id == 3 &&
        &services.widgets.owner(binding.widget.layout()) == &binding.widget &&
        &services.widgets.environment().models.retained_owners == &services.actual_owners,
        "Text resource names require the same canonical Text and actual resource domain");
}
void require_name(const std::string& name) {
    require(name.size() <= (std::numeric_limits<std::uint32_t>::max)() &&
        name.find('\0') == std::string::npos,
        "Text resource name requires a null-free native DWORD-length string");
}
bool equal_name(const std::string& current, const std::string& incoming) {
    // Nonowning header views borrow the canonical string bytes solely for
    // the existing00435C40 comparison; no second cached name is introduced.
    struct Header { std::uint32_t length; const char* data; };
    static_assert(sizeof(Header) == 8, "Text native name comparison requires Win32");
    const Header left{static_cast<std::uint32_t>(current.size()), current.c_str()};
    const Header right{static_cast<std::uint32_t>(incoming.size()), incoming.c_str()};
    return equal_native_string_headers_00435c40(&left, &right);
}
void invalidate_shader(GuiTextLifetime& lifetime, NativeRenderActualOwners& owners) {
    void* const captured = lifetime.cached_shader_slot_1ec();
    if (!captured) return;
    release_native_render_actual_owner(owners, captured);
    //8CC5/8EC7 stores after terminal callback, even if it published another value.
    lifetime.cached_shader_slot_1ec() = nullptr;
    lifetime.text().has_cached_shader = false;
}
} // namespace

void set_gui_text_shader_name_00ab8e70(GuiTextLifetime& lifetime,
    const std::string& name, GuiTextResourceNameServices& services) {
    require_owner(lifetime, services);
    require_name(name);
    auto& current = lifetime.text().shader_name;
    if (&current != &name) current = name; // Native header identity skips copy only.
    invalidate_shader(lifetime, services.actual_owners);
}

bool set_gui_text_font_name_00ab8c30(GuiTextLifetime& lifetime,
    const std::string& name, GuiTextFontNameServices& services) {
    require_owner(lifetime, services.names);
    require_name(name);
    auto& text = lifetime.text();
    require_name(text.font_name);
    if (equal_name(text.font_name, name)) return false;
    if (&text.font_name != &name) text.font_name = name;
    // Getter callbacks see the changed name. The subsequent lookup reads the
    // LIVE stored member, not the earlier source argument or compatibility copy.
    const FontDescriptor* selected = nullptr;
    if (auto* images = services.registry.native_image_owners()) {
        require(images == &services.fonts,
            "Text image lookup requires the SAME canonical native registry custody");
        struct Header { std::uint32_t length; const char* data; };
        static_assert(sizeof(Header) == 8);
        const Header incoming{static_cast<std::uint32_t>(text.font_name.size()), text.font_name.c_str()};
        // ONLY the actual list/native Font+0C name selects an image. Resolving
        // that exact image to its descriptor supplies old Text companion
        // identity; no descriptor name is inspected or substituted.
        if (auto* actual = images->find_native_font_00ac3570(&incoming))
            selected = &actual->descriptor();
    } else {
        auto* registry = services.registry.get();
        require(registry != nullptr, "Text font lookup requires its actual registry owner");
        const auto* found = registry->find_font_00ac3570(text.font_name);
        selected = found ? &found->descriptor : nullptr;
    }
    if (selected) {
        const auto& actual = services.fonts.resolve(selected);
        require(&actual.descriptor() == selected &&
            &actual.actual_owners() == &services.names.actual_owners,
            "Text font lookup must resolve the same actual font and image owner domain");
    }
    //AB8C86 publishes borrowed font BEFORE reading its live+1C scalar.
    text.font = selected;
    if (text.font) {
        const auto& alpha = services.fonts.resolve(text.font).alpha_texture_scale_1c();
        std::memcpy(&text.alpha_texture_scale, &alpha, sizeof(float));
    } else {
        const float one = services.one_00d7a24c;
        std::memcpy(&text.alpha_texture_scale, &one, sizeof(float));
    }
    invalidate_shader(lifetime, services.names.actual_owners);
    return true;
}
namespace {
using ShaderNameWord = std::uint32_t;
static_assert(sizeof(void*) == sizeof(ShaderNameWord));
ShaderNameWord shader_name_bits(const volatile void* p) {
    return reinterpret_cast<ShaderNameWord>(p);
}
void* shader_name_pointer(ShaderNameWord value) {
    return reinterpret_cast<void*>(value);
}
void* shader_name_at(const void* p, ShaderNameWord offset) {
    return shader_name_pointer(shader_name_bits(p) + offset);
}
ShaderNameWord shader_name_read(const void* p, ShaderNameWord offset = 0) {
    return *reinterpret_cast<const volatile ShaderNameWord*>(shader_name_bits(p) + offset);
}
void shader_name_write(void* p, ShaderNameWord offset, ShaderNameWord value) {
    *reinterpret_cast<volatile ShaderNameWord*>(shader_name_bits(p) + offset) = value;
}
struct ShaderNameRegion { const volatile void* begin; std::size_t bytes; };
bool shader_name_representable(ShaderNameRegion region) {
    return region.begin &&
        static_cast<std::uint64_t>(shader_name_bits(region.begin)) + region.bytes <= 0x100000000ull;
}
bool shader_name_separate(ShaderNameRegion a, ShaderNameRegion b) {
    const auto aa = static_cast<std::uint64_t>(shader_name_bits(a.begin));
    const auto bb = static_cast<std::uint64_t>(shader_name_bits(b.begin));
    return aa + a.bytes <= bb || bb + b.bytes <= aa;
}
void* shader_name_receiver(NativeGuiTextRawModelReference& reference) {
    const auto& c = reference.storage_context();
    void* const raw = reference.storage();
    require(reference.phase() == NativeGuiTextRawModelReference::Phase::bound && raw &&
        !(shader_name_bits(raw) & 3u) && c.owners.find(raw) == &reference &&
        static_cast<void*>(&reference.reference_count) == shader_name_at(raw, 4),
        "raw Shader-name setter requires SAME live canonical122 before native fields");
    const auto& a = c.construction;
    const auto& r = c.retirement.resources;
    const auto& m = c.retirement.model;
    require(a.started && a.complete && a.base_complete && a.derived_complete && a.sections_entered &&
        a.destination == raw && !a.exception_cleanup_started && !a.base_lifetime_ended &&
        !a.consumed_cleanup_actions && a.failure == decltype(a.failure)::none &&
        a.sections.complete && !a.sections.exception_cleanup_started &&
        a.sections.failure == decltype(a.sections.failure)::none &&
        a.sections.model_constructed && a.sections.shadow_published && a.sections.model_registered &&
        &c.construction_context.sections == &r && &a.sections.model == &m &&
        m.actual && m.reference && !m.reference->retired() && m.construction.complete &&
        m.construction.metadata_bound && m.construction.actual_model == m.actual &&
        m.construction.context == &r.model.storage && !m.lifetime.started &&
        !m.lifetime.scalar_started && !m.release.started &&
        m.association.phase == NativeModelStorageGeometryPhase::complete &&
        m.association.context == &r.association &&
        m.association.frame == &c.construction_frame.sections.association &&
        &r.geometry.actual_owners() == &c.owners && c.owners.find(m.actual) == &*m.reference,
        "raw Shader-name setter requires exact completed93/85/internal188/B75170 graph");
    require(shader_name_read(raw) == 0x00d5c6c8u,
        "raw Shader-name setter excludes active destruction/base prefixes");
    return raw;
}
void* shader_name_preflight(NativeGuiTextRawModelReference& reference, const void* input,
    const NativeGuiTextReferenceFrame& reference_frame, NativeGuiTextShaderStorageFrame& shader,
    const NativeGuiTextRawShaderNameStorageInvocation& invocation,
    const game::GameNativeCanonicalDataOwner& canonical,
    const volatile ShaderNameWord* feature) {
    void* const raw = shader_name_receiver(reference);
    const auto& c = reference.storage_context();
    const auto& r = c.retirement.resources;
    const auto& m = c.retirement.model;
    const auto& node = r.model.storage.node;
    const auto* shader_context = shader.prepared_raw_context();
    require(reference.matches_reference_frame(reference_frame) &&
        c.font_shader_frame == &shader && c.retirement.font_shader_frame == &shader &&
        shader_context && shader.matches_credit_backing(*shader_context, reference_frame),
        "raw Shader-name setter requires SAME reserved credited ShaderFrame/canonical reference frame");
    const auto& cache = r.effects;
    const auto& effects = cache.effects;
    require(&r.association.storage == &r.model.storage && &r.parenting.node == &node &&
        &node.strings == &r.strings && c.retirement.base.nodes.uses_raw_name_pool() &&
        &c.retirement.base.nodes.require_raw_name_pool() == &r.strings &&
        &node.scenes.owners == &c.owners && &node.trees.owners == &c.owners &&
        &r.materials.retained_owners == &c.owners && effects.cache == &cache &&
        &cache.strings == &r.material_strings && &effects.construction.strings == &r.material_strings &&
        &effects.programs.strings == &r.material_strings &&
        &effects.owners.string_storage() == &r.material_strings &&
        &effects.owners.actual_owners() == &c.owners &&
        &effects.construction.current_renderer_00f8d394 == &effects.programs.current_renderer_00f8d394 &&
        same_native_string_storage_raw_pool_cells(r.material_strings, r.strings) &&
        same_native_string_raw_pool_cells(r.strings, r.string_process.raw_context()) &&
        &node.scenes.decrement_00ce2220 == &node.trees.decrement_00ce2220,
        "raw Shader-name setter requires SAME real pool/cache/renderer/Model/owner/import cell domains");
    // Source HOST metadata only. No feature value, current1EC or import sample.
    require(&c.data == &canonical.read_only_data() &&
        feature == &canonical.mutable_crt_data().feature_word(),
        "raw Shader-name setter requires actual canonical owner and feature-cell ADDRESS affinity");
    const ShaderNameRegion writes[] = {{&invocation, sizeof(invocation)}, {&shader, sizeof(shader)}};
    require(shader_name_representable(writes[0]) && shader_name_representable(writes[1]) &&
        !(shader_name_bits(&invocation) & 3u) && !(shader_name_bits(&shader) & 3u) &&
        shader_name_separate(writes[0], writes[1]),
        "raw Shader-name setter requires separate aligned retained invocation/ShaderFrame");
    for (const ShaderNameRegion b : {ShaderNameRegion{raw, 0x1f8}, {input, 8},
        {&reference, sizeof(reference)}, {&c, sizeof(c)}, {&reference_frame, sizeof(reference_frame)},
        {&c.owners, sizeof(c.owners)}, {&c.data, sizeof(c.data)},
        {&canonical, sizeof(canonical)}, {&canonical.mutable_crt_data(), sizeof(canonical.mutable_crt_data())},
        {feature, sizeof(*feature)}, {&c.construction_context, sizeof(c.construction_context)},
        {&c.construction_frame, sizeof(c.construction_frame)}, {&c.construction, sizeof(c.construction)},
        {&c.retirement, sizeof(c.retirement)}, {&r, sizeof(r)}, {&m, sizeof(m)},
        {m.actual, 0x188}, {&*m.reference, sizeof(*m.reference)},
        {&r.model, sizeof(r.model)}, {&r.model.storage, sizeof(r.model.storage)},
        {&node, sizeof(node)}, {&node.scenes, sizeof(node.scenes)}, {&node.trees, sizeof(node.trees)},
        {&r.materials, sizeof(r.materials)}, {&r.material_strings, sizeof(r.material_strings)},
        {&r.strings, sizeof(r.strings)}, {&r.string_process, sizeof(r.string_process)},
        {&r.strings.actual_published_01090aa8, sizeof(void*)},
        {&r.strings.actual_small_returns_disabled_01090aa4, sizeof(ShaderNameWord)},
        {&r.strings.actual_manager_publication_01090aa0, sizeof(void*)},
        {&cache, sizeof(cache)}, {&effects, sizeof(effects)},
        {&effects.construction, sizeof(effects.construction)}, {&effects.programs, sizeof(effects.programs)},
        {&effects.owners, sizeof(effects.owners)},
        {&effects.construction.current_renderer_00f8d394, sizeof(void*)},
        {&r.parenting, sizeof(r.parenting)}, {&node.trees.decrement_00ce2220, sizeof(node.trees.decrement_00ce2220)},
        {&c.construction_context.one_00d7a24c, sizeof(ShaderNameWord)},
        {shader_context, sizeof(*shader_context)}, {&shader_context->fonts, sizeof(shader_context->fonts)},
        {shader_context->configuration_0109cf04.identity(), sizeof(void*)}}) {
        require(shader_name_representable(b) && shader_name_separate(writes[0], b) &&
            shader_name_separate(writes[1], b),
            "raw Shader-name mutable backing must not overlap actual input/native/provider/feature extents");
    }
    return raw;
}
} // namespace

NativeGuiTextRawShaderNameStorageInvocation::NativeGuiTextRawShaderNameStorageInvocation(
    const NativeGuiTextRawModelReferenceContext& context,
    const NativeGuiTextReferenceFrame& reference_frame, NativeGuiTextShaderStorageFrame& shader,
    const void* input, const game::GameNativeCanonicalDataOwner& canonical)
    : context_(&context), reference_frame_(&reference_frame), shader_(shader), input_(input),
      canonical_data_(&canonical), feature_word_0109eea4_(&canonical.mutable_crt_data().feature_word()) {}
NativeGuiTextRawShaderNameStorageInvocation::Phase
NativeGuiTextRawShaderNameStorageInvocation::phase() const { return phase_; }
bool NativeGuiTextRawShaderNameStorageInvocation::requires_process_retention() const {
    return failed_ || phase_ != Phase::fresh;
}

void set_native_gui_text_shader_name_storage_00ab8e70(NativeGuiTextRawModelReference& reference,
    const void* name, NativeGuiTextRawShaderNameStorageInvocation& invocation) {
    using Invocation = NativeGuiTextRawShaderNameStorageInvocation;
    try {
        require(invocation.phase_ == Invocation::Phase::fresh && !invocation.failed_ &&
            !invocation.reference_ && !invocation.actual_text_,
            "raw Shader-name invocation is once-only and cannot reset/replace a failed binding");
        invocation.phase_ = Invocation::Phase::entered;
        invocation.reference_ = &reference; // actual live entry, never prebirth preparation
        invocation.actual_text_ = reference.storage();
        require(&reference.storage_context() == invocation.context_ && name == invocation.input_,
            "raw Shader-name entry requires its SAME prepared Context and actual input STORAGE");
        NativeGuiTextShaderStorageFrame::CreditConsumer credit(invocation.shader_, reference, invocation);
        try {
            void* const raw = shader_name_preflight(reference, name, *invocation.reference_frame_,
                invocation.shader_, invocation, *invocation.canonical_data_, invocation.feature_word_0109eea4_);
            const auto& c = *invocation.context_;
            void* const destination_header = shader_name_at(raw, 0x1c0);
            if (name != destination_header) { //8E7F: header identity skips ONLY resize/copy
                const ShaderNameWord request = shader_name_read(name); //8E83 CURRENT sourceLEN
                invocation.preserve_home_ = 1u; //8E85 actual retained DWORD; byte0 is callee HOME
                resize_native_string_header_0041dd40_exact(destination_header,
                    c.retirement.resources.strings, request, &invocation.preserve_home_,
                    *invocation.feature_word_0109eea4_);
                if (shader_name_read(name) != 0u) { //8E8F CURRENT sourceLEN after resize
                    const ShaderNameWord bytes = shader_name_read(destination_header); //8E94 CURRENT dstLEN
                    const void* const source = shader_name_pointer(shader_name_read(name, 4)); //8E96 CURRENT srcDATA
                    void* const destination = shader_name_pointer(shader_name_read(destination_header, 4)); //8E99 CURRENT dstDATA
                    copy_native_crt_bytes_00bf7680(destination, source, bytes,
                        *invocation.feature_word_0109eea4_); //8E9F actual call even when bytes==0
                }
            }
            void* const captured = shader_name_pointer(shader_name_read(raw, 0x1ec)); //8EA7 CURRENT, including self-alias
            credit.capture(captured); // genuine private credit only; unknown replacement fails BEFORE decrement
            if (captured) {
                const auto decrement = c.retirement.resources.model.storage.node.trees.decrement_00ce2220;
                credit.decrement_entered(decrement); // actual CURRENT import; rejects null without calling it
                const long result = decrement(reinterpret_cast<volatile long*>(shader_name_at(captured, 4)));
                credit.decrement_returned(result);
                if (result == 0) {
                    credit.zero_entered();
                    dispatch_native_render_actual_owner_zero(c.owners, captured);
                    credit.zero_returned();
                }
                shader_name_write(raw, 0x1ec, 0); //8EC7 LIVE clear AFTER callback, even a replacement
                credit.slot_cleared(); // ONLY now return the old captured credit; never rescan/release replacement
            }
            require(invocation.phase_ == Invocation::Phase::entered && !invocation.failed_ &&
                invocation.reference_ == &reference && invocation.actual_text_ == raw &&
                shader_name_receiver(reference) == raw && reference.matches_reference_frame(*invocation.reference_frame_),
                "caught nested rejection cannot overwrite raw Shader-name normal completion");
            credit.complete(); // AFTER real normal body and sticky/canonical checks; no ended Effect read
            require(invocation.phase_ == Invocation::Phase::entered && !invocation.failed_,
                "raw Shader-name completion cannot replace retained failure");
            invocation.phase_ = Invocation::Phase::complete;
        } catch (...) { credit.fail(); throw; }
    } catch (...) {
        invocation.failed_ = true;
        invocation.phase_ = Invocation::Phase::failed;
        throw; // retain actual provider debt/backing; no rollback, disposal, repair or retry
    }
}
} // namespace bsp
