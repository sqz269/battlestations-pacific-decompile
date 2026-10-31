#include "bsp/native_gui_text_field_storage.hpp"
#include "bsp/native_gui_text_shader_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_gui_widget_color_storage.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include <cstring>
#include <exception>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Acquired = NativeGuiTextFieldStorageAcquired;
static_assert(sizeof(void*) == 4);
static_assert(std::is_trivially_destructible_v<Acquired>);
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(const void* p, Word offset = 0) noexcept { return pointer(bits(p) + offset); }
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(bits(p) + offset) = value;
}
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
struct Region { const volatile void* begin; std::size_t bytes; };
bool representable(Region r) noexcept {
    return r.begin && static_cast<std::uint64_t>(bits(r.begin)) + r.bytes <= 0x100000000ull;
}
bool separate(Region a, Region b) noexcept {
    const auto aa = static_cast<std::uint64_t>(bits(a.begin));
    const auto bb = static_cast<std::uint64_t>(bits(b.begin));
    return aa + a.bytes <= bb || bb + b.bytes <= aa;
}
void separate_scratch(Region region, NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    require(representable(region) && separate(region, {&frame, sizeof(frame)}) &&
        separate(region, {&a, sizeof(a)}), "Text field scratch must be separate from live borrowed backing");
}
void* preflight(NativeGuiTextReference& reference, NativeGuiTextFieldStorageContext& context,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a, const void* input, std::size_t input_bytes) {
    const auto& c = reference.storage_context();
    void* const raw = reference.storage();
    require(reference.phase() == NativeGuiTextReference::Phase::bound && raw &&
        (bits(raw) & 3u) == 0 && c.owners.find(raw) == &reference,
        "Text fields require the SAME live full-storage canonical122 binding");
    require(c.construction.complete && c.construction.destination == raw &&
        !c.construction.exception_cleanup_started && !c.construction.base_lifetime_ended,
        "Text fields require the SAME genuine completed93 construction");
    require(a.phase == Acquired::Phase::fresh && a.entry == Acquired::Entry::none &&
        !a.destination && !a.active_call_site && !a.shader_decrement_entered &&
        !a.state_entered && !a.color_entered,
        "Text field invocation requires a fresh retained diagnostic record");
    const Region scratch[] = {{&frame, sizeof(frame)}, {&a, sizeof(a)}};
    require(representable(scratch[0]) && representable(scratch[1]) &&
        (bits(&frame) & 3u) == 0 && separate(scratch[0], scratch[1]),
        "Text field frame/diagnostics require separate representable backing");
    const auto& r = c.retirement.resources;
    for (const Region borrowed : {Region{raw, 0x1f8}, {&reference, sizeof(reference)},
        {&c, sizeof(c)}, {&c.owners, sizeof(c.owners)}, {&c.data, sizeof(c.data)},
        {&c.construction_context, sizeof(c.construction_context)},
        {&c.construction_frame, sizeof(c.construction_frame)}, {&c.construction, sizeof(c.construction)},
        {&c.retirement, sizeof(c.retirement)}, {&r, sizeof(r)},
        {&r.actual, sizeof(r.actual)}, {&r.actual.models, sizeof(r.actual.models)},
        {&r.strings, sizeof(r.strings)}, {&r.effects, sizeof(r.effects)},
        {&r.parenting, sizeof(r.parenting)}, {&r.decrement_00ce2220, sizeof(r.decrement_00ce2220)},
        {&c.construction_context.one_00d7a24c, sizeof(Word)},
        {&context, sizeof(context)}, {&context.fonts, sizeof(context.fonts)},
        {&context.current, sizeof(context.current)}})
        separate_scratch(borrowed, frame, a);
    if (input_bytes) separate_scratch({input, input_bytes}, frame, a);
    return raw; // No Native count load/gate/retain or current-table read.
}
template<class Operation>
void enter(NativeGuiTextReference& reference, NativeGuiTextFieldStorageContext& c,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a, Acquired::Entry entry,
    const void* input, std::size_t input_bytes, Operation operation) {
    void* const raw = preflight(reference, c, frame, a, input, input_bytes);
    a.destination = raw;
    a.entry = entry;
    a.phase = Acquired::Phase::entered;
    try {
        operation(raw, reference.storage_context());
        a.phase = Acquired::Phase::complete;
    } catch (...) {
        a.phase = Acquired::Phase::failed;
        throw; // Preserve reached raw fields/credits; no cleanup or retry.
    }
}
Word current_target(void* raw, unsigned slot, Word& profile,
    NativeGuiTextFieldStorageContext& c, NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    profile = read(raw); // AFTER the preceding native field/arithmetic effects.
    const auto* table = c.current.current_text_table(raw, profile); // Pure metadata only.
    require(table && bits(table) == profile && (profile & 3u) == 0,
        "Text current dispatch requires its genuine captured actual-profile table");
    separate_scratch({table, 0x84}, frame, a);
    return table[slot / 4]; // Numeric selector, never a numeric host call.
}
// Whole B74640 field access; its native stacked DWORD is unused.
// Read the captured Model's data word without forming a typed tail view.
void* raw_model_geometry_00b74640(const void* actual_model) noexcept {
    void* geometry;
    __asm {
        mov ecx, actual_model
        mov eax, dword ptr [ecx + 180h]
        mov geometry, eax
    }
    return geometry;
}
bool has_geometry_00b74650(const void* model) noexcept {
    return read(model, 0x180) != 0; // Genuine whole B74650 field read.
}
void color_body(void* raw, const void* original_rgba,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    a.active_call_site = 0x00ab6b59;
    set_native_gui_widget_color_storage_00aa6870(raw, original_rgba);
    a.base_color_returned = true;
    void* model = pointer(read(raw, 0x188));
    if (!model) return;
    a.active_call_site = 0x00ab6b68;
    if (!has_geometry_00b74650(model)) return;
    model = pointer(read(raw, 0x188));
    a.active_call_site = 0x00ab6b79;
    void* mesh = raw_model_geometry_00b74640(model);
    a.active_call_site = 0x00ab6b80;
    if (!gui_mesh_element_count_00b72b40(mesh)) return;
    model = pointer(read(raw, 0x188));
    a.active_call_site = 0x00ab6b93;
    mesh = raw_model_geometry_00b74640(model);
    a.active_call_site = 0x00ab6b9a;
    void* const element = gui_geometry_element_00b732c0(mesh, 0);

    const auto* shadow = static_cast<const volatile float*>(at(raw, 0x174));
    const auto* incoming = static_cast<const volatile float*>(at(original_rgba, 0xc));
    auto* product = &frame.shadow_alpha;
    void* material;
    // Native loads/multiplies before capturing material, then spills before
    // B179F0. This is scratch only, never a parallel Text/Material field.
    __asm {
        mov eax, shadow
        mov edx, incoming
        fld dword ptr [eax]
        fmul dword ptr [edx]
        mov eax, element
        mov eax, dword ptr [eax + 0x20]
        mov material, eax
        mov eax, product
        fstp dword ptr [eax]
    }
    a.active_call_site = 0x00ab6bb3;
    auto* const diffuse = native_material_diffuse_00b179f0(
        *static_cast<NativeMaterialStorage*>(material), 0);
    __asm {
        mov eax, product
        mov edx, diffuse
        fld dword ptr [eax]
        fstp dword ptr [edx + 0xc]
    }
    a.shadow_alpha_published = true;
}
void brighten(NativeGuiTextFieldStorageFrame& frame, const NativeGuiTextReferenceContext& c,
    Acquired& a) {
    const auto* rgb = static_cast<const volatile double*>(c.data.data_at(0x00d5c5d0, 8));
    const auto* alpha = static_cast<const volatile double*>(c.data.data_at(0x00d7a258, 8));
    separate_scratch({rgb, 8}, frame, a);
    separate_scratch({alpha, 8}, frame, a);
    auto* rgba = frame.rgba;
    __asm {
        mov eax, rgba
        mov edx, rgb
        fld dword ptr [eax]
        fld qword ptr [edx]
        fadd st(1), st(0)
        fxch st(1)
        fstp dword ptr [eax]
        fld dword ptr [eax + 4]
        fadd st(0), st(1)
        fstp dword ptr [eax + 4]
        fadd dword ptr [eax + 8]
        fstp dword ptr [eax + 8]
        fld dword ptr [eax + 0xc]
        mov edx, alpha
        fadd qword ptr [edx]
        fstp dword ptr [eax + 0xc]
    }
}
void state_body(void* raw, std::int32_t state, const NativeGuiTextReferenceContext& owner,
    NativeGuiTextFieldStorageContext& c, NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    const auto flag = *reinterpret_cast<const volatile std::uint8_t*>(at(raw, 0x77));
    const Word offset = flag ? 0x14cu : (static_cast<Word>(state) << 4) + 0x11cu;
    const void* const row = at(raw, offset); // Native DWORD wrap, no state clamp.
    frame.rgba[0] = read(row);
    frame.rgba[1] = read(row, 4);
    const Word third = read(row, 8);
    const Word fourth = read(row, 0xc);
    frame.rgba[2] = third;
    frame.rgba[3] = fourth;
    a.state_row_copied = true;
    if (state == 1) {
        brighten(frame, owner, a);
        a.state_brightened = true;
    }
    a.active_call_site = 0x00ab727c;
    a.color_target = current_target(raw, 0x50, a.color_profile, c, frame, a);
    a.color_entered = true;
    const void* rgba = const_cast<const Word*>(frame.rgba);
    if (a.color_target == 0x00ab6b50)
        color_body(raw, rgba, frame, a);
    else
        c.current.call_unknown_color_50(raw, a.color_profile, a.color_target, rgba);
    a.color_returned = true;
}
struct FontNameView {
    NativeRenderActualOwners& owners;
    NativeStringRawPoolContext& strings;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement;
    const volatile Word& one;
};
struct FontNameResult {
    bool equal{}, resized{}, self{};
    Word length{}, alpha{};
    void* data{};
    void* font{};
};
using FontCredit = NativeGuiTextShaderStorageFrame::CreditConsumer;
void font_scratch(Region region, NativeGuiTextFieldStorageFrame& frame, Acquired& a,
    Region outer_frame, Region outer_acquired) {
    separate_scratch(region, frame, a);
    if (outer_frame.begin)
        require(separate(region, outer_frame) && separate(region, outer_acquired),
            "raw Font body must retain separate full outer frame/acquisition backing");
}
FontNameResult font_name_body(void* raw, const void* name, NativeFontResourceOwners& fonts,
    FontNameView v, NativeGuiTextFieldStorageFrame& frame, Acquired& a, FontCredit* credit,
    Region outer_frame = {}, Region outer_acquired = {}) {
    FontNameResult out;
    void* const cached = at(raw, 0x1c8);
    a.active_call_site = 0x00ab8c42;
    if (equal_native_string_headers_00435c40(cached, name)) {
        a.equal_name = true;
        out.equal = true;
        if (credit) credit->finish_without_capture(); // actual native early return only
        return out;
    }
    out.self = cached == name;
    if (cached != name) {
        const Word length = read(name);
        a.active_call_site = 0x00ab8c5a;
        resize_native_string_header_0041dd40(cached, v.strings, length, true);
        a.name_resized = true;
        out.resized = true;
        if (read(name) != 0) {
            const Word bytes = read(cached);
            const void* const source = pointer(read(name, 4));
            void* const destination = pointer(read(cached, 4));
            if (credit && bytes) { // raw-only metadata extent guards, no staged native inputs
                font_scratch({source, bytes}, frame, a, outer_frame, outer_acquired);
                font_scratch({destination, bytes}, frame, a, outer_frame, outer_acquired);
            }
            a.active_call_site = 0x00ab8c6f;
            if (bytes) std::memmove(destination, source, bytes);
            a.name_copied = true;
        }
    }
    if (credit) { // raw-only provenance snapshot while its bound Text/header is live
        out.length = read(cached);
        out.data = pointer(read(cached, 4));
    }
    a.active_call_site = 0x00ab8c7f;
    auto* const font = fonts.find_native_font_00ac3570(cached);
    void* image = nullptr;
    if (font) {
        require(font->live() && font->native_image() && &font->actual_owners() == &v.owners,
            "Raw Text Font lookup must use SAME image/actual owner domain");
        image = font->native_image();
        font_scratch({font, sizeof(*font)}, frame, a, outer_frame, outer_acquired);
        font_scratch({image, 0xac}, frame, a, outer_frame, outer_acquired);
    }
    a.selected_font = image;
    write(raw, 0x108, bits(image)); // BEFORE current alpha read, borrow only
    a.font_published = true;
    const Word alpha = image ? read(image, 0x1c) : v.one;
    write(raw, 0x1d4, alpha);
    a.alpha_published = true;
    out.font = image;
    out.alpha = alpha;
    void* const shader = pointer(read(raw, 0x1ec)); // CURRENT, after all preceding providers
    a.shader = shader;
    if (credit) credit->capture(shader);
    if (!shader) return out;
    a.active_call_site = 0x00ab8cb3;
    const auto decrement = v.decrement; // Original reached CURRENT import epoch
    require(decrement != nullptr, "Raw Text shader release requires CURRENT CE2220 import");
    a.captured_decrement = decrement;
    if (credit) credit->decrement_entered(decrement);
    a.shader_decrement_entered = true;
    // Keep the legacy diagnostic operand behavior; raw authority uses its
    // captured local, never mutable public diagnostics across a provider.
    const long result = decrement(reinterpret_cast<volatile long*>(at(credit ? shader : a.shader, 4)));
    a.shader_result = result;
    a.shader_decremented = true;
    if (credit) credit->decrement_returned(result);
    if (result == 0) {
        a.active_call_site = 0x00ab8cc3;
        if (credit) credit->zero_entered();
        a.shader_zero_entered = true;
        dispatch_native_render_actual_owner_zero(v.owners, credit ? shader : a.shader);
        a.shader_zero_returned = true;
        if (credit) credit->zero_returned();
    }
    write(raw, 0x1ec, 0); // AFTER callback, only the originally nonnull arm
    a.shader_slot_cleared = true;
    if (credit) credit->slot_cleared();
    return out;
}
using RawContext = NativeGuiTextRawFontNameStorageContext;
using RawFrame = NativeGuiTextRawFontNameStorageFrame;
using RawAcquired = NativeGuiTextRawFontNameStorageAcquired;
void* raw_font_receiver(NativeGuiTextRawModelReference& reference) {
    const auto& c = reference.storage_context();
    void* const raw = reference.storage();
    require(reference.phase() == NativeGuiTextRawModelReference::Phase::bound && raw &&
        !(bits(raw) & 3u) && c.owners.find(raw) == &reference &&
        static_cast<void*>(&reference.reference_count) == at(raw, 4),
        "raw Font setter requires the SAME live canonical122 before native fields");
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
        "raw Font setter requires the exact completed93/85/internal188 provider graph");
    require(read(raw) == 0x00d5c6c8, "raw Font setter excludes active destruction/base prefixes");
    return raw;
}
RawFrame::HeaderEntry raw_font_preflight(NativeGuiTextRawModelReference& reference,
    const void* input, RawContext& context, RawFrame& frame, RawAcquired& acquired) {
    void* const raw = raw_font_receiver(reference);
    const auto& c = reference.storage_context();
    const auto& r = c.retirement.resources;
    const auto& m = c.retirement.model;
    const auto& node = r.model.storage.node;
    const auto* shader = frame.prepared_shader_frame();
    const auto* shader_context = shader ? shader->prepared_raw_context() : nullptr;
    require(frame.prepared_reference_frame() && reference.matches_reference_frame(*frame.prepared_reference_frame()) &&
        shader && c.font_shader_frame == shader && c.retirement.font_shader_frame == shader &&
        shader_context && &shader_context->fonts == &context.fonts &&
        shader->matches_credit_backing(*shader_context, *frame.prepared_reference_frame()) &&
        context.fonts.native_registry_image(),
        "raw Font setter needs its SAME real collection/reserved credited shader and canonical reference frame");
    require(&r.association.storage == &r.model.storage && &r.parenting.node == &node &&
        &node.strings == &r.strings && c.retirement.base.nodes.uses_raw_name_pool() &&
        &c.retirement.base.nodes.require_raw_name_pool() == &r.strings &&
        &node.scenes.owners == &c.owners && &node.trees.owners == &c.owners &&
        &r.materials.retained_owners == &c.owners &&
        same_native_string_storage_raw_pool_cells(r.material_strings, r.strings) &&
        same_native_string_raw_pool_cells(r.strings, r.string_process.raw_context()) &&
        &node.scenes.decrement_00ce2220 == &node.trees.decrement_00ce2220,
        "raw Font setter needs SAME real VFS/string/Model/actual-owner/import cell domains");
    require(input == at(raw, 0x1c8) || separate({input, 8}, {raw, 0x1f8}),
        "raw Font input must be its actual external header or the provenance-qualified cached header");
    const Region writes[] = {{&frame, sizeof(frame)}, {&acquired, sizeof(acquired)}};
    require(representable(writes[0]) && representable(writes[1]) &&
        !(bits(&frame) & 3u) && !(bits(&acquired) & 3u) && separate(writes[0], writes[1]),
        "raw Font setter requires separate aligned full frame/acquisition");
    for (const Region b : {Region{raw, 0x1f8}, {&reference, sizeof(reference)}, {&c, sizeof(c)},
        {&c.owners, sizeof(c.owners)}, {&c.data, sizeof(c.data)},
        {&c.construction_context, sizeof(c.construction_context)}, {&c.construction_frame, sizeof(c.construction_frame)},
        {&c.construction, sizeof(c.construction)}, {&c.retirement, sizeof(c.retirement)}, {&r, sizeof(r)},
        {&m, sizeof(m)}, {m.actual, 0x188}, {&*m.reference, sizeof(*m.reference)},
        {&r.model, sizeof(r.model)}, {&r.model.storage, sizeof(r.model.storage)},
        {&node, sizeof(node)}, {&node.scenes, sizeof(node.scenes)}, {&node.trees, sizeof(node.trees)},
        {&r.materials, sizeof(r.materials)}, {&r.material_strings, sizeof(r.material_strings)},
        {&r.strings, sizeof(r.strings)}, {&r.string_process, sizeof(r.string_process)},
        {&r.strings.actual_published_01090aa8, sizeof(void*)},
        {&r.strings.actual_small_returns_disabled_01090aa4, sizeof(Word)},
        {&r.strings.actual_manager_publication_01090aa0, sizeof(void*)}, {&r.effects, sizeof(r.effects)},
        {&r.parenting, sizeof(r.parenting)}, {&node.trees.decrement_00ce2220, sizeof(node.trees.decrement_00ce2220)},
        {&c.construction_context.one_00d7a24c, sizeof(Word)}, {&context, sizeof(context)},
        {&context.fonts, sizeof(context.fonts)}, {context.fonts.native_registry_image(), 0x10},
        {shader, sizeof(*shader)}, {shader_context, sizeof(*shader_context)},
        {shader_context->configuration_0109cf04.identity(), sizeof(void*)},
        {frame.prepared_reference_frame(), sizeof(*frame.prepared_reference_frame())}, {input, 8}}) {
        require(representable(b) && separate(writes[0], b) && separate(writes[1], b),
            "raw Font setter scratch must not overlap actual input/native/provider/backing extents");
    }
    const auto& d = acquired.diagnostics;
    require(d.phase == Acquired::Phase::fresh && d.entry == Acquired::Entry::none && !d.destination &&
        !d.active_call_site && !d.shader_decrement_entered && !d.state_entered && !d.color_entered,
        "raw Font setter diagnostics must also be fresh, never authoritative");
    const auto* prior = frame.prior_header();
    if (!prior) {
        require(input != at(raw, 0x1c8) && !read(raw, 0x1c8) && !read(raw, 0x1cc) && !read(raw, 0x108),
            "without prior provenance raw Font setter admits ONLY genuine constructor-empty1C8/108");
        return RawFrame::HeaderEntry::constructor_empty;
    }
    require(prior->issued() && prior->context() && prior->frame() && prior->acquisition() &&
        prior->acquisition()->publication() == prior &&
        prior->frame()->matches(*prior->context(), prior->frame()->prepared_input_header8(),
            *frame.prepared_reference_frame(), *shader, *prior->acquisition(), RawFrame::Phase::complete) &&
        prior->reference_context() == &c && prior->reference_frame() == frame.prepared_reference_frame() &&
        prior->text_identity() == raw && prior->header_identity() == at(raw, 0x1c8) &&
        prior->strings() == &r.strings && prior->fonts() == &context.fonts &&
        prior->length() == read(raw, 0x1c8) && prior->data_identity() == pointer(read(raw, 0x1cc)) &&
        prior->font_identity() == pointer(read(raw, 0x108)) && prior->alpha_bits() == read(raw, 0x1d4),
        "existing/self cached1C8 needs its SAME private normal header/Font publication");
    if (prior->font_identity()) {
        auto& font = context.fonts.resolve_image(prior->font_identity());
        require(font.live() && &font.actual_owners() == &c.owners,
            "prior Font publication requires its SAME still-live image custody");
    }
    return RawFrame::HeaderEntry::prior_normal;
}
} // namespace

NativeGuiTextRawFontNameStorageFrame::NativeGuiTextRawFontNameStorageFrame(const RawContext& context,
    const void* input, const NativeGuiTextReferenceFrame& reference_frame,
    NativeGuiTextShaderStorageFrame& shader, RawAcquired& acquired,
    const NativeGuiTextRawFontNameStoragePublication* prior) noexcept
    : context_(&context), input_(input), reference_frame_(&reference_frame), shader_(shader),
      acquired_(&acquired), prior_(prior) {
    if (acquired.failed_ || acquired.bound_frame_ || !input || (bits(input) & 3u) ||
        static_cast<std::uint64_t>(bits(input)) + 8u > 0x100000000ull) {
        acquired.failed_ = true;
        phase_ = Phase::failed;
    } else acquired.bound_frame_ = this; // one pure binding, no native input read
}
NativeGuiTextRawFontNameStorageFrame::~NativeGuiTextRawFontNameStorageFrame() noexcept {
    if (phase_ == Phase::entered || phase_ == Phase::failed || acquired_->failed_) std::terminate();
    if (phase_ == Phase::complete) {
        const auto& s = reference_frame_->scalar;
        if (!s.complete || !s.destructor_returned || !s.flags_captured || !s.pool_returned ||
            !(s.captured_flags_byte & 1u) || !s.destruction.complete ||
            !(s.destruction.normal_actions_returned & (1u << 6))) std::terminate();
    }
}
bool NativeGuiTextRawFontNameStorageFrame::matches(const RawContext& c, const void* input,
    const NativeGuiTextReferenceFrame& reference_frame, const NativeGuiTextShaderStorageFrame& shader,
    const RawAcquired& a, Phase phase) const noexcept {
    return context_ == &c && input_ == input && reference_frame_ == &reference_frame && &shader_ == &shader &&
        acquired_ == &a && a.bound_frame_ == this && !a.failed_ && phase_ == phase;
}
const NativeGuiTextRawFontNameStoragePublication* RawAcquired::publication() const noexcept {
    return !failed_ && bound_frame_ && bound_frame_->phase() == RawFrame::Phase::complete && publication_.issued_
        ? &publication_ : nullptr;
}

void set_native_gui_text_font_name_storage_00ab8c30(NativeGuiTextReference& reference,
    const void* name, NativeGuiTextFieldStorageContext& c,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    enter(reference, c, frame, a, Acquired::Entry::font_name, name, 8,
        [&](void* raw, const NativeGuiTextReferenceContext& owner) {
        font_name_body(raw, name, c.fonts, {owner.owners, owner.retirement.resources.strings,
            owner.retirement.resources.decrement_00ce2220, owner.construction_context.one_00d7a24c},
            frame, a, nullptr);
    });
}
void set_native_gui_text_font_name_storage_00ab8c30(NativeGuiTextRawModelReference& reference,
    const void* name, RawContext& c, RawFrame& f, RawAcquired& a) {
    try {
        require(f.matches(c, name, *f.reference_frame_, f.shader_, a, RawFrame::Phase::fresh),
            "raw Font setter requires its exact once-prepared context/input/frame/acquisition");
        f.phase_ = RawFrame::Phase::entered; // private, before native admission/effects
        FontCredit credit(f.shader_, reference, c, f, a); // only this genuine outer friend may construct
        try {
            f.entry_header_ = raw_font_preflight(reference, name, c, f, a);
            void* const raw = reference.storage();
            const auto& owner = reference.storage_context();
            a.diagnostics.destination = raw;
            a.diagnostics.entry = Acquired::Entry::font_name;
            a.diagnostics.phase = Acquired::Phase::entered;
            const auto result = font_name_body(raw, name, c.fonts,
                {owner.owners, owner.retirement.resources.strings,
                    owner.retirement.resources.model.storage.node.trees.decrement_00ce2220,
                    owner.construction_context.one_00d7a24c},
                f.body_, a.diagnostics, &credit, {&f, sizeof(f)}, {&a, sizeof(a)});
            require(f.phase_ == RawFrame::Phase::entered && !a.failed_ && a.bound_frame_ == &f &&
                raw_font_receiver(reference) == raw && reference.matches_reference_frame(*f.reference_frame_),
                "caught reentry/failure cannot overwrite raw Font normal completion");
            if (!result.equal) {
                require((result.resized || (result.self && f.prior_)) &&
                    read(raw, 0x1c8) == result.length && pointer(read(raw, 0x1cc)) == result.data &&
                    pointer(read(raw, 0x108)) == result.font && read(raw, 0x1d4) == result.alpha,
                    "raw Font publication rejects unknown post-provider header/Font/alpha replacement");
                if (result.font) {
                    const auto& font = c.fonts.resolve_image(result.font); // C++ custody, no raw Font read
                    require(font.live() && &font.actual_owners() == &owner.owners,
                        "raw Font publication requires SAME live borrowed Font custody after callbacks");
                }
            }
            credit.complete(); // AFTER entire native body and final sticky metadata checks
            require(f.phase_ == RawFrame::Phase::entered && !a.failed_,
                "raw Font final completion cannot erase a nested rejection");
            f.equal_name_ = result.equal; // actual private branch outcome, never public diagnostics
            if (!result.equal) {
                auto& p = a.publication_;
                p.origin_ = result.resized ? NativeGuiTextRawFontNameStoragePublication::Origin::resized_normal
                    : NativeGuiTextRawFontNameStoragePublication::Origin::inherited_self;
                p.text_ = raw; p.header_ = at(raw, 0x1c8); p.data_ = result.data; p.length_ = result.length;
                p.font_ = result.font; p.alpha_ = result.alpha;
                p.reference_context_ = &owner; p.reference_frame_ = f.reference_frame_;
                p.context_ = &c; p.frame_ = &f; p.acquisition_ = &a;
                p.strings_ = &owner.retirement.resources.strings; p.fonts_ = &c.fonts; p.previous_ = f.prior_;
                p.issued_ = true; // no throwing operation after this normal-only seal
            }
            a.diagnostics.phase = Acquired::Phase::complete;
            f.phase_ = RawFrame::Phase::complete;
        } catch (...) { credit.fail(); throw; }
    } catch (...) {
        f.phase_ = RawFrame::Phase::failed;
        f.acquired_->failed_ = true;
        f.acquired_->diagnostics.phase = Acquired::Phase::failed;
        a.failed_ = true;
        a.diagnostics.phase = Acquired::Phase::failed;
        throw; // preserve every actual header/Font/effect mutation; no inferred cleanup
    }
}
void set_native_gui_text_flag77_storage_00ab6a70(NativeGuiTextReference& reference,
    std::uint8_t flag, NativeGuiTextFieldStorageContext& c,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    enter(reference, c, frame, a, Acquired::Entry::flag77, nullptr, 0,
        [&](void* raw, const NativeGuiTextReferenceContext& owner) {
        *reinterpret_cast<volatile std::uint8_t*>(at(raw, 0x77)) = flag;
        a.flag_published = true;
        a.active_call_site = flag ? 0x00ab6a8b : 0x00ab6a95;
        a.state_target = current_target(raw, 0x80, a.state_profile, c, frame, a);
        const std::int32_t state = flag ? 3 : 0;
        a.state_entered = true;
        if (a.state_target == 0x00ab7200)
            state_body(raw, state, owner, c, frame, a);
        else
            c.current.call_unknown_state_80(raw, a.state_profile, a.state_target, state);
        a.state_returned = true;
    });
}
void apply_native_gui_text_state_color_storage_00ab7200(NativeGuiTextReference& reference,
    std::int32_t state, NativeGuiTextFieldStorageContext& c,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    enter(reference, c, frame, a, Acquired::Entry::state_color, nullptr, 0,
        [&](void* raw, const NativeGuiTextReferenceContext& owner) {
        state_body(raw, state, owner, c, frame, a);
    });
}
void set_native_gui_text_color_storage_00ab6b50(NativeGuiTextReference& reference,
    const void* rgba, NativeGuiTextFieldStorageContext& c,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    enter(reference, c, frame, a, Acquired::Entry::color, rgba, 16,
        [&](void* raw, const NativeGuiTextReferenceContext&) {
        color_body(raw, rgba, frame, a);
    });
}
void set_native_gui_text_color_storage_00ab6b50(void* actual_text,
    const void* original_rgba, NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    require(actual_text && (bits(actual_text) & 3u) == 0,
        "Raw Text color requires aligned live actual storage");
    require(a.phase == Acquired::Phase::fresh && a.entry == Acquired::Entry::none &&
        !a.destination && !a.active_call_site && !a.shader_decrement_entered &&
        !a.state_entered && !a.color_entered,
        "Raw Text color requires a fresh retained diagnostic record");
    const Region scratch[] = {{&frame, sizeof(frame)}, {&a, sizeof(a)}};
    require(representable(scratch[0]) && representable(scratch[1]) &&
        (bits(&frame) & 3u) == 0 && separate(scratch[0], scratch[1]),
        "Raw Text color frame/diagnostics require separate representable backing");
    separate_scratch({actual_text, 0x1f8}, frame, a);
    separate_scratch({original_rgba, 16}, frame, a);
    a.destination = actual_text;
    a.entry = Acquired::Entry::color;
    a.phase = Acquired::Phase::entered;
    try {
        color_body(actual_text, original_rgba, frame, a);
        a.phase = Acquired::Phase::complete;
    } catch (...) {
        a.phase = Acquired::Phase::failed;
        throw; // Retain reached fields/backing/diagnostics; no Native-origin inference.
    }
}
} // namespace bsp
