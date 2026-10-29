#include "bsp/native_gui_text_shader_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>

namespace bsp {
// Private Source observation, not a native provider or callable public API.
// Only the raw outer entry can name/construct this nested type. The common
// native schedule passes the actual local operands at their reached epochs.
class NativeGuiTextShaderStorageFrame::RawProducer final {
public:
    explicit RawProducer(NativeGuiTextShaderStorageFrame& frame) noexcept : frame_(frame) {}
    void renderer_entered(void* renderer, std::uint32_t profile, std::uint32_t target) {
        check(!frame_.raw_evidence_.entered);
        frame_.raw_evidence_.renderer = renderer;
        frame_.raw_evidence_.profile = profile;
        frame_.raw_evidence_.target = target;
        frame_.raw_evidence_.entered = true;
    }
    void renderer_returned(void* result) {
        check(frame_.raw_evidence_.entered && !frame_.raw_evidence_.returned);
        frame_.raw_evidence_.result = result;
        frame_.raw_evidence_.returned = true;
    }
    void published(void* text, void* effect) {
        check(frame_.raw_evidence_.returned && !frame_.raw_evidence_.published &&
            text == frame_.credit_.actual_text && effect == frame_.raw_evidence_.result);
        frame_.raw_evidence_.published = true;
    }
private:
    NativeGuiTextShaderStorageFrame& frame_;
    void check(bool condition) {
        if (!condition || frame_.credit_.failed ||
            frame_.credit_.producer != RawPhase::entered) {
            frame_.fail_raw();
            throw std::invalid_argument("Raw Text shader private producer observation is not current");
        }
    }
};
namespace {
using Word = std::uint32_t;
using Frame = NativeGuiTextShaderStorageFrame;
struct NoRawProducer {
    void renderer_entered(void*, Word, Word) const noexcept {}
    void renderer_returned(void*) const noexcept {}
    void published(void*, void*) const noexcept {}
};
// Private borrows only. No Model transport, context copy, current-cell value
// snapshot or native ownership is introduced by the common body view.
struct ShaderView {
    game::GameNativeReadOnlyData& data;
    NativeRenderActualOwnerRegistry& owners;
    NativeMaterialEffectCacheContext& cache;
    NativeStringRawPoolContext& strings;
    const volatile Word& one_00d7a24c;
};
ShaderView view(const NativeGuiTextReferenceContext& c) {
    return {c.data, c.owners, c.retirement.resources.effects,
        c.retirement.resources.strings, c.construction_context.one_00d7a24c};
}
ShaderView view(const NativeGuiTextRawModelReferenceContext& c) {
    return {c.data, c.owners, c.retirement.resources.effects,
        c.retirement.resources.strings, c.construction_context.one_00d7a24c};
}
static_assert(sizeof(void*) == sizeof(Word));
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
void* header(Frame& frame, unsigned index) noexcept {
    return const_cast<Word*>(&frame.names[index * 2]);
}
struct Region { const volatile void* begin; std::size_t bytes; };
bool separate(Region a, Region b) noexcept {
    const auto aa = static_cast<std::uint64_t>(bits(a.begin));
    const auto bb = static_cast<std::uint64_t>(bits(b.begin));
    return aa + a.bytes <= bb || bb + b.bytes <= aa;
}
void exclude_frame(Region region, Frame& frame) {
    require(region.begin && static_cast<std::uint64_t>(bits(region.begin)) + region.bytes <= 0x100000000ull &&
        separate(region, {&frame, sizeof(frame)}),
        "Raw Text shader frame must be separate from genuine borrowed backing");
}
void* preflight(NativeGuiTextReference& reference, NativeGuiTextShaderStorageContext& context,
    Frame& frame) {
    const auto& c = reference.storage_context();
    void* const raw = reference.storage();
    require(reference.phase() == NativeGuiTextReference::Phase::bound && raw &&
        (bits(raw) & 3u) == 0 && c.owners.find(raw) == &reference &&
        c.construction.complete && c.construction.destination == raw &&
        !c.construction.exception_cleanup_started && !c.construction.base_lifetime_ended,
        "Raw Text shader requires SAME genuine full93/canonical122 binding");
    require(frame.phase == Frame::Phase::fresh && !frame.destination && !frame.selected_name &&
        frame.native_eh_state == -1 && !frame.renderer_entered && !frame.shader_published &&
        frame.cache.phase == NativeMaterialEffectCacheAcquired::Phase::not_started &&
        !frame.cache.loaded && !frame.cache.result && !frame.cache.caller_acquired &&
        !frame.cache.cache_published,
        "Raw Text shader requires fresh independently retained invocation backing");
    require((bits(&frame) & 3u) == 0 &&
        static_cast<std::uint64_t>(bits(&frame)) + sizeof(frame) <= 0x100000000ull,
        "Raw Text shader frame requires aligned representable storage");
    auto& r = c.retirement.resources;
    auto& cache = r.effects;
    auto& effects = cache.effects;
    require(effects.cache == &cache && &cache.strings == &r.actual.strings &&
        &effects.construction.strings == &r.actual.strings &&
        &effects.programs.strings == &r.actual.strings &&
        &effects.owners.actual_owners() == &c.owners &&
        &effects.construction.current_renderer_00f8d394 == &effects.programs.current_renderer_00f8d394,
        "Raw Text shader must use SAME93 effect cache/string/renderer/actual-owner graph");
    for (const Region borrowed : {Region{raw, 0x1f8}, {&reference, sizeof(reference)},
        {&c, sizeof(c)}, {&c.owners, sizeof(c.owners)}, {&c.data, sizeof(c.data)},
        {&c.construction_context, sizeof(c.construction_context)},
        {&c.construction_frame, sizeof(c.construction_frame)}, {&c.construction, sizeof(c.construction)},
        {&c.retirement, sizeof(c.retirement)}, {&r, sizeof(r)},
        {&r.actual, sizeof(r.actual)}, {&r.strings, sizeof(r.strings)},
        {&cache, sizeof(cache)}, {&effects, sizeof(effects)},
        {&effects.construction, sizeof(effects.construction)}, {&effects.programs, sizeof(effects.programs)},
        {&effects.owners, sizeof(effects.owners)},
        {&effects.construction.current_renderer_00f8d394, sizeof(void*)},
        {&c.construction_context.one_00d7a24c, sizeof(Word)},
        {&context, sizeof(context)}, {&context.fonts, sizeof(context.fonts)},
        {context.configuration_0109cf04.identity(), sizeof(void*)}})
        exclude_frame(borrowed, frame);
    if (context.unknown_renderer48)
        exclude_frame({context.unknown_renderer48, sizeof(*context.unknown_renderer48)}, frame);
    return raw; // Metadata only; no Text count or native cache/name/global read.
}
void* preflight(NativeGuiTextRawModelReference& reference, NativeGuiTextShaderStorageContext& context,
    Frame& frame) {
    const auto& c = reference.storage_context();
    void* const raw = reference.storage();
    require(reference.phase() == NativeGuiTextRawModelReference::Phase::bound && raw &&
        !(bits(raw) & 3u) && c.owners.find(raw) == &reference &&
        static_cast<void*>(&reference.reference_count) == at(raw, 4) &&
        c.font_shader_frame == &frame && frame.prepared_raw_context() == &context,
        "Raw-Model Text shader requires SAME bound canonical122 and reserved immutable Frame/context");
    const auto& a = c.construction;
    auto& r = c.retirement.resources;
    auto& m = c.retirement.model;
    require(a.started && a.complete && a.base_complete && a.derived_complete && a.sections_entered &&
        a.destination == raw && !a.exception_cleanup_started && !a.base_lifetime_ended &&
        !a.consumed_cleanup_actions && a.failure == decltype(a.failure)::none &&
        a.sections.complete && !a.sections.exception_cleanup_started &&
        a.sections.failure == decltype(a.sections.failure)::none && a.sections.model_registered &&
        a.sections.model_constructed && a.sections.shadow_published &&
        &c.construction_context.sections == &r && &a.sections.model == &m &&
        m.actual && m.reference && !m.reference->retired() && m.construction.complete &&
        m.construction.metadata_bound && m.construction.actual_model == m.actual &&
        m.construction.context == &r.model.storage && !m.lifetime.started &&
        !m.lifetime.scalar_started && !m.release.started &&
        m.association.phase == NativeModelStorageGeometryPhase::complete &&
        m.association.context == &r.association &&
        m.association.frame == &c.construction_frame.sections.association &&
        c.owners.find(m.actual) == &*m.reference,
        "Raw-Model Text shader requires its SAME genuine completed93/85/188/B75170 graph");
    require(frame.phase == Frame::Phase::fresh && !frame.destination && !frame.selected_name &&
        frame.native_eh_state == -1 && !frame.renderer_entered && !frame.shader_published &&
        frame.cache.phase == NativeMaterialEffectCacheAcquired::Phase::not_started &&
        !frame.cache.loaded && !frame.cache.result && !frame.cache.caller_acquired &&
        !frame.cache.cache_published && !frame.cache.key_resolution,
        "Raw-Model Text shader requires fresh independently retained producer backing");
    require(!(bits(&frame) & 3u) &&
        static_cast<std::uint64_t>(bits(&frame)) + sizeof(frame) <= 0x100000000ull,
        "Raw-Model Text shader frame requires aligned representable storage");
    auto& cache = r.effects;
    auto& effects = cache.effects;
    require(effects.cache == &cache && &cache.strings == &r.material_strings &&
        &effects.construction.strings == &r.material_strings &&
        &effects.programs.strings == &r.material_strings &&
        &effects.owners.string_storage() == &r.material_strings &&
        &effects.owners.actual_owners() == &c.owners && &r.materials.retained_owners == &c.owners &&
        &effects.construction.current_renderer_00f8d394 == &effects.programs.current_renderer_00f8d394 &&
        same_native_string_storage_raw_pool_cells(r.material_strings, r.strings) &&
        same_native_string_raw_pool_cells(r.strings, r.string_process.raw_context()),
        "Raw-Model Text shader must borrow SAME actual typed/raw strings/cache/renderer/owner graph");
    for (const Region borrowed : {Region{raw, 0x1f8}, {&reference, sizeof(reference)},
        {&c, sizeof(c)}, {&c.owners, sizeof(c.owners)}, {&c.data, sizeof(c.data)},
        {&c.construction_context, sizeof(c.construction_context)},
        {&c.construction_frame, sizeof(c.construction_frame)}, {&a, sizeof(a)},
        {&c.retirement, sizeof(c.retirement)}, {&r, sizeof(r)}, {&m, sizeof(m)},
        {m.actual, 0x188}, {&*m.reference, sizeof(*m.reference)},
        {&r.model, sizeof(r.model)}, {&r.model.storage, sizeof(r.model.storage)},
        {&r.material_strings, sizeof(r.material_strings)}, {&r.strings, sizeof(r.strings)},
        {&cache, sizeof(cache)}, {&effects, sizeof(effects)},
        {&effects.construction, sizeof(effects.construction)}, {&effects.programs, sizeof(effects.programs)},
        {&effects.owners, sizeof(effects.owners)},
        {&effects.construction.current_renderer_00f8d394, sizeof(void*)},
        {&c.construction_context.one_00d7a24c, sizeof(Word)},
        {&context, sizeof(context)}, {&context.fonts, sizeof(context.fonts)},
        {context.configuration_0109cf04.identity(), sizeof(void*)}})
        exclude_frame(borrowed, frame);
    return raw; // No current native Text/name/config/Font/renderer value read.
}
void location(Frame& frame, Word site, Frame::Boundary boundary) noexcept {
    frame.active_call_site = site;
    frame.boundary = boundary; // Source boundary; never Native exception-origin proof.
}
void return_captured_name(void* data, void* name, NativeStringRawPoolContext& strings,
    Word getter_site, Word return_site, Frame& frame) {
    frame.captured_name_data = data;
    if (!data) return; // Original skips the length and getter for null data.
    const Word bytes = read(name) + 1u; // AFTER EH-state consumption, only current length.
    frame.captured_name_bytes = bytes;
    location(frame, getter_site, Frame::Boundary::string_return);
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    location(frame, return_site, Frame::Boundary::string_return);
    return_native_string_pool_00bd1510(pool, data, bytes,
        strings.actual_small_returns_disabled_01090aa4);
    // Never clear/reload the consumed header, including provider mutations.
}
bool ordered_height_match(void* configuration, const volatile float* reference) noexcept {
    std::uint8_t matched;
    __asm {
        mov ecx, configuration
        mov edx, reference
        cvtsi2ss xmm0, dword ptr [ecx + 0x28]
        ucomiss xmm0, dword ptr [edx]
        lahf
        test ah, 0x44
        setnp al
        mov matched, al
    }
    return matched != 0;
}
bool ordered_font_scale_match(void* font, const volatile Word* one) noexcept {
    std::uint8_t matched;
    __asm {
        mov ecx, font
        mov edx, one
        movss xmm0, dword ptr [ecx + 0x18]
        ucomiss xmm0, dword ptr [edx]
        lahf
        test ah, 0x44
        setnp al
        mov matched, al
    }
    return matched != 0;
}
template<class Producer>
void* load_current_renderer(const void* name, std::int32_t state, Word site,
    const ShaderView& c, NativeGuiTextShaderStorageContext& context, Frame& frame, bool known_only,
    Producer& producer) {
    auto& cache = c.cache;
    frame.captured_renderer = cache.effects.construction.current_renderer_00f8d394;
    require(frame.captured_renderer, "Raw Text shader requires CURRENT actual renderer");
    exclude_frame({frame.captured_renderer, 4}, frame);
    frame.renderer_profile = read(frame.captured_renderer);
    location(frame, site, Frame::Boundary::metadata);
    const auto* const table = static_cast<const volatile Word*>(
        c.data.data_at(frame.renderer_profile, 0x4c));
    require(bits(table) == frame.renderer_profile,
        "Raw Text shader current48 requires SAME genuine Original-VA profile alias");
    exclude_frame({table, 0x4c}, frame);
    frame.renderer_target = table[0x48 / 4];
    // Original arms state0/1 only AFTER current renderer/profile/target capture.
    frame.native_eh_state = state;
    frame.selected_name = name;
    frame.renderer_entered = true;
    location(frame, site, Frame::Boundary::renderer48);
    producer.renderer_entered(frame.captured_renderer, frame.renderer_profile, frame.renderer_target);
    void* result;
    if (frame.renderer_target == 0x00b318b0) {
        result = load_native_renderer_material_effect_00b318b0(
            frame.captured_renderer, name, cache, &frame.cache);
    } else {
        require(!known_only, "Raw-Model Text shader rejects unsupported captured CURRENT renderer48");
        require(context.unknown_renderer48,
            "Raw Text shader requires genuine provider for CURRENT unknown renderer48");
        result = context.unknown_renderer48->call_renderer_48(frame.captured_renderer,
            frame.renderer_profile, frame.renderer_target, name, cache, frame.cache);
    }
    producer.renderer_returned(result);
    frame.result = result;
    frame.renderer_returned = true;
    return result;
}
void cleanup_name(const ShaderView& c, Frame& frame) noexcept {
    frame.exception_cleanup_started = true;
    const auto state = frame.native_eh_state;
    frame.native_eh_state = -1; // Consume before entering the required raw cleanup.
    if (state != 0 && state != 1) return;
    frame.cleanup_entered = true;
    location(frame, state == 0 ? 0x00cb7fe3u : 0x00cb7febu, Frame::Boundary::string_return);
    try {
        destroy_native_string_header_0041dd20(header(frame, static_cast<unsigned>(state)),
            c.strings);
        frame.cleanup_returned = true;
    } catch (...) {
        std::terminate(); // No retry/double return after cleanup escape.
    }
}
} // namespace

NativeGuiTextShaderStorageFrame::NativeGuiTextShaderStorageFrame(
    const NativeGuiTextShaderStorageContext& context) noexcept : prepared_raw_context_(&context) {}
NativeGuiTextShaderStorageFrame::NativeGuiTextShaderStorageFrame(
    const NativeGuiTextShaderStorageContext& context, const NativeGuiTextReferenceFrame& reference) noexcept
    : prepared_raw_context_(&context), prepared_reference_frame_(&reference) {}
NativeGuiTextShaderStorageFrame::~NativeGuiTextShaderStorageFrame() noexcept {
    if (phase == Phase::entered || phase == Phase::failed || credit_.failed ||
        credit_.producer == RawPhase::entered || active_consumer_ ||
        credit_.credit == CreditPhase::outstanding || credit_.credit == CreditPhase::reserved)
        std::terminate();
}
namespace {
template<class Producer>
bool ensure_shader(void* raw, const ShaderView& c,
    NativeGuiTextShaderStorageContext& context, Frame& frame, bool known_only, Producer& producer) {
    auto& strings = c.strings;
    frame.destination = raw;
    frame.phase = Frame::Phase::entered;
    try {
        if (read(raw, 0x1ec) != 0) {
            frame.cached = true;
            frame.phase = Frame::Phase::complete;
            return false; // BEFORE all native name/config/font/renderer reads.
        }
        location(frame, 0x00ab8d13, Frame::Boundary::string_construct);
        const auto* empty = static_cast<const char*>(c.data.data_at(0x00ce3a0c, 1));
        construct_native_string_header_0041e870(header(frame, 0), strings, empty);
        frame.empty_constructed = true;
        location(frame, 0x00ab8d25, Frame::Boundary::metadata);
        const bool has_override = unequal_native_string_headers_00449af0(at(raw, 0x1c0), header(frame, 0));
        frame.empty_compared = true;
        void* const empty_data = pointer(read(header(frame, 0), 4));
        return_captured_name(empty_data, header(frame, 0), strings, 0x00ab8d3f, 0x00ab8d46, frame);
        frame.empty_returned = true;
        if (has_override) {
            frame.override_selected = true;
            void* const effect = load_current_renderer(at(raw, 0x1c0), -1, 0x00ab8d5b,
                c, context, frame, known_only, producer);
            write(raw, 0x1ec, bits(effect)); // Transfer owned return, no added retain.
            producer.published(raw, effect);
            frame.shader_published = true;
        } else {
            frame.captured_configuration = context.configuration_0109cf04.load();
            require(frame.captured_configuration, "Raw Text shader needs actual configuration+28");
            exclude_frame({frame.captured_configuration, 0x2c}, frame);
            const auto* reference_height = static_cast<const volatile float*>(c.data.data_at(0x00d5c56c, 4));
            exclude_frame({reference_height, 4}, frame);
            frame.height_matches = ordered_height_match(frame.captured_configuration, reference_height);
            bool point = false;
            if (frame.height_matches) {
                frame.captured_font = pointer(read(raw, 0x108)); // ONLY on ordered height match.
                auto& font = context.fonts.resolve_image(frame.captured_font); // Identity -> custody only.
                require(font.live() && &font.actual_owners() == &c.owners,
                    "Raw Text shader requires SAME live image Font/actual-owner domain");
                exclude_frame({&font, sizeof(font)}, frame);
                exclude_frame({frame.captured_font, 0xac}, frame);
                point = ordered_font_scale_match(frame.captured_font, &c.one_00d7a24c);
                frame.font_scale_matches = point;
            }
            const unsigned index = point ? 0u : 1u;
            const auto* literal = static_cast<const char*>(c.data.data_at(
                point ? 0x00d5c794u : 0x00cefd78u, point ? 13u : 21u));
            location(frame, point ? 0x00ab8db0u : 0x00ab8dfbu, Frame::Boundary::string_construct);
            construct_native_string_header_0041e870(header(frame, index), strings, literal);
            frame.default_constructed = true;
            void* const effect = load_current_renderer(header(frame, index), point ? 0 : 1,
                point ? 0x00ab8dcdu : 0x00ab8e18u, c, context, frame, known_only, producer);
            void* data;
            if (point) {
                data = pointer(read(header(frame, 0), 4)); // AB8DCF BEFORE publication.
                write(raw, 0x1ec, bits(effect));
                producer.published(raw, effect);
            } else {
                write(raw, 0x1ec, bits(effect)); // AB8E1A BEFORE captured data.
                producer.published(raw, effect);
                data = pointer(read(header(frame, 1), 4));
            }
            frame.shader_published = true;
            frame.native_eh_state = -1; // Native normal cleanup consumes state before getter.
            frame.cleanup_entered = true;
            return_captured_name(data, header(frame, index), strings, 0x00ab8e3b, 0x00ab8e42, frame);
            frame.cleanup_returned = true;
        }
        frame.attempted = true; // A null return is STILL a completed attempt.
        frame.phase = Frame::Phase::complete;
        return true;
    } catch (...) {
        frame.escaped_boundary = frame.boundary;
        frame.escaped_call_site = frame.active_call_site;
        cleanup_name(c, frame);
        frame.phase = Frame::Phase::failed;
        throw; // Keep acquired credits, raw publication and independent metadata.
    }
}
} // namespace
NativeGuiTextShaderStorageFrame::CreditProgress
NativeGuiTextShaderStorageFrame::credit_progress() const noexcept {
    return credit_; // retained metadata only; never follow the local token
}
bool NativeGuiTextShaderStorageFrame::matches_credit_backing(
    const NativeGuiTextShaderStorageContext& context,
    const NativeGuiTextReferenceFrame& reference) const noexcept {
    return prepared_raw_context_ == &context && prepared_reference_frame_ == &reference;
}
void NativeGuiTextShaderStorageFrame::authenticate_bound_reference(
    const NativeGuiTextRawModelReference& reference) {
    const auto& c = reference.storage_context();
    void* const raw = reference.storage();
    require(reference.phase() == NativeGuiTextRawModelReference::Phase::bound && raw &&
        c.owners.find(raw) == &reference && c.font_shader_frame == this &&
        static_cast<const void*>(&reference.reference_count) == at(raw, 4) &&
        (!prepared_reference_frame_ || (reference.matches_reference_frame(*prepared_reference_frame_) &&
            c.retirement.font_shader_frame == this)),
        "Raw Text shader credit requires SAME live canonical122 and constructor-pinned frame");
    require((!credit_.reference || credit_.reference == &reference) &&
        (!credit_.reference_context || credit_.reference_context == &c) &&
        (!credit_.actual_text || credit_.actual_text == raw),
        "Raw Text shader credit cannot replace its canonical Text/reference/context generation");
    credit_.actual_text = raw;
    credit_.reference = &reference;
    credit_.reference_context = &c;
    credit_.reference_frame = prepared_reference_frame_;
    credit_.cache = &cache;
}
void NativeGuiTextShaderStorageFrame::begin_raw(NativeGuiTextRawModelReference& reference,
    NativeGuiTextShaderStorageContext& context) {
    try {
        require(!credit_.failed && !active_consumer_ && !retirement_started_ &&
            credit_.producer == RawPhase::fresh && credit_.credit == CreditPhase::none &&
            prepared_raw_context_ == &context,
            "Raw Text shader private producer is one-shot and excludes active/failed operations");
        credit_.producer = RawPhase::entered;
        authenticate_bound_reference(reference);
    } catch (...) {
        fail_raw();
        throw;
    }
}
void NativeGuiTextShaderStorageFrame::complete_raw(NativeGuiTextRawModelReference& reference,
    NativeGuiTextShaderStorageContext& context, bool actual_attempted) {
    require(!credit_.failed && !active_consumer_ && credit_.producer == RawPhase::entered &&
        prepared_raw_context_ == &context,
        "Raw Text shader private completion cannot overwrite an interrupted invocation");
    authenticate_bound_reference(reference); // canonical companion is still live HERE only
    if (!actual_attempted) {
        require(!raw_evidence_.entered && !raw_evidence_.returned && !raw_evidence_.published,
            "Cached Text1EC cannot manufacture a renderer-return credit");
    } else {
        require(raw_evidence_.entered && raw_evidence_.returned && raw_evidence_.published &&
            raw_evidence_.target == 0x00b318b0u,
            "Raw Text shader normal return requires private actual B318B0/publication observations");
    }
    // The old uncredited constructor retains its actual operation behavior.
    // Neither it, Text's cached fast return nor an attempted native null grants
    // new credit authority. Private result proof, not public result flags, is
    // what distinguishes the nonnull owning renderer return below.
    if (prepared_reference_frame_ && actual_attempted && raw_evidence_.result) {
        require(phase == Phase::complete && !cached && attempted && shader_published &&
            renderer_entered && renderer_returned && result == raw_evidence_.result &&
            empty_constructed && empty_compared && empty_returned &&
            native_eh_state == -1 && !exception_cleanup_started &&
            (override_selected || (default_constructed && cleanup_entered && cleanup_returned)) &&
            cache.phase == NativeMaterialEffectCacheAcquired::Phase::complete &&
            cache.result == raw_evidence_.result && cache.caller_acquired,
            "Raw Text shader credit requires genuine normal names/cache/caller acquisition completion");
        require(pointer(read(credit_.actual_text, 0x1ec)) == raw_evidence_.result,
            "Raw Text shader credit requires its SAME live published Text1EC generation");
        credit_.effect = raw_evidence_.result;
        credit_.credit = CreditPhase::outstanding;
    }
    credit_.producer = RawPhase::complete;
}
void NativeGuiTextShaderStorageFrame::fail_raw() noexcept {
    credit_.failed = true;
    credit_.producer = RawPhase::failed;
}

NativeGuiTextShaderStorageFrame::CreditConsumer::CreditConsumer(
    NativeGuiTextShaderStorageFrame& frame, NativeGuiTextRawModelReference& reference,
    const NativeGuiTextRawFontNameStorageContext& context,
    const NativeGuiTextRawFontNameStorageFrame& invocation,
    const NativeGuiTextRawFontNameStorageAcquired& acquired)
    : frame_(frame), kind_(Consumer::font_name), font_context_(&context),
      font_frame_(&invocation), font_acquired_(&acquired) {
    try {
        require(!frame_.credit_.failed && !frame_.active_consumer_ && !frame_.retirement_started_ &&
            frame_.credit_.producer != RawPhase::entered && frame_.prepared_raw_context_ &&
            frame_.prepared_reference_frame_,
            "Raw font-name shader consumer requires fresh stable credited preparation");
        frame_.credit_.active_consumer = kind_;
        frame_.credit_.active_font_context = font_context_;
        frame_.credit_.active_font_frame = font_frame_;
        frame_.credit_.active_font_acquired = font_acquired_;
        frame_.authenticate_bound_reference(reference);
        frame_.active_consumer_ = this;
        frame_.credit_.active = true;
        active_ = true;
    } catch (...) {
        frame_.credit_.failed = true;
        throw;
    }
}
NativeGuiTextShaderStorageFrame::CreditConsumer::CreditConsumer(
    NativeGuiTextShaderStorageFrame& frame, void* raw,
    NativeGuiTextRawModelRetirementContext& context, NativeGuiTextRetirementAcquired& acquired)
    : frame_(frame), kind_(Consumer::retirement), retirement_context_(&context),
      retirement_acquired_(&acquired) {
    try {
        require(!frame_.credit_.failed && !frame_.active_consumer_ && !frame_.retirement_started_ &&
            frame_.credit_.producer != RawPhase::entered && frame_.prepared_raw_context_ &&
            frame_.prepared_reference_frame_,
            "Raw118 shader consumer requires stable credited preparation and no prior entry");
        frame_.retirement_started_ = true; // private one-shot BEFORE all remaining admission
        frame_.credit_.active_consumer = kind_;
        frame_.credit_.active_retirement_context = retirement_context_;
        frame_.credit_.active_retirement_acquired = retirement_acquired_;
        frame_.credit_.active_retirement_shader = &acquired.shader;
        const auto& scalar = frame_.prepared_reference_frame_->scalar;
        require(context.font_shader_frame == &frame_ && &acquired == &scalar.destruction &&
            &acquired.shader == &scalar.destruction.shader && scalar.started &&
            scalar.destructor_entered && !acquired.started && raw &&
            (!frame_.credit_.actual_text || raw == frame_.credit_.actual_text) &&
            (!frame_.credit_.reference_context ||
                &context == &frame_.credit_.reference_context->retirement),
            "Raw118 shader consumer requires exact canonical scalar/destruction/context records");
        if (frame_.credit_.reference) {
            const auto& reference = *frame_.credit_.reference;
            require(reference.phase() == NativeGuiTextRawModelReference::Phase::retiring &&
                reference.storage() == raw && reference.matches_reference_frame(*frame_.prepared_reference_frame_) &&
                &reference.storage_context() == frame_.credit_.reference_context,
                "Raw118 shader consumer requires SAME live retiring canonical122 companion");
        }
        frame_.active_consumer_ = this;
        frame_.credit_.active = true;
        active_ = true;
    } catch (...) {
        frame_.credit_.failed = true;
        throw;
    }
}
NativeGuiTextShaderStorageFrame::CreditConsumer::~CreditConsumer() noexcept {
    if (active_) fail();
}
void NativeGuiTextShaderStorageFrame::CreditConsumer::require_stage(Stage expected) {
    if (!active_ || frame_.active_consumer_ != this || frame_.credit_.failed || stage_ != expected) {
        fail();
        throw std::invalid_argument("Raw Text shader consumer observation is not its current private epoch");
    }
}
void NativeGuiTextShaderStorageFrame::CreditConsumer::capture(void* captured) {
    require_stage(Stage::opened);
    try {
        // The late operand itself came from Original's reached read. Recheck
        // live canonical metadata here, before any Effect terminal can run.
        if (kind_ == Consumer::font_name) {
            require(frame_.credit_.reference != nullptr,
                "Raw font-name shader capture requires its canonical reference identity");
            frame_.authenticate_bound_reference(*frame_.credit_.reference);
        } else if (frame_.credit_.reference) {
            const auto& reference = *frame_.credit_.reference;
            require(reference.phase() == NativeGuiTextRawModelReference::Phase::retiring &&
                reference.storage() == frame_.credit_.actual_text &&
                reference.matches_reference_frame(*frame_.prepared_reference_frame_) &&
                &reference.storage_context() == frame_.credit_.reference_context,
                "Raw118 shader capture requires SAME live retiring canonical reference");
        }
        if (!captured) {
            require(frame_.credit_.credit != CreditPhase::outstanding &&
                frame_.credit_.credit != CreditPhase::reserved,
                "Null Text1EC cannot complete an outstanding private shader credit");
            stage_ = Stage::null_capture;
            return; // do not overwrite a prior real consumer's retained history
        }
        require(frame_.credit_.producer == RawPhase::complete &&
            frame_.credit_.credit == CreditPhase::outstanding &&
            frame_.credit_.effect == captured && frame_.credit_.reference &&
            frame_.credit_.reference_context && frame_.credit_.reference_frame == frame_.prepared_reference_frame_ &&
            frame_.credit_.cache == &frame_.cache,
            "Captured Text1EC must be the single genuine privately issued shader credit");
        auto& c = frame_.credit_;
        c.credit = CreditPhase::reserved;
        c.consumer = kind_;
        c.capture_entered = true;
        c.font_context = font_context_;
        c.font_frame = font_frame_;
        c.font_acquired = font_acquired_;
        c.retirement_context = retirement_context_;
        c.retirement_acquired = retirement_acquired_;
        c.retirement_shader = retirement_acquired_ ? &retirement_acquired_->shader : nullptr;
        stage_ = Stage::captured; // irreversible reservation BEFORE CE2220
    } catch (...) { fail(); throw; }
}
void NativeGuiTextShaderStorageFrame::CreditConsumer::decrement_entered(
    NativeTextureSurfaceReferenceIncrement current_import) {
    require_stage(Stage::captured);
    if (!current_import) {
        fail();
        throw std::invalid_argument("Raw Text shader release requires its reached CURRENT CE2220 import");
    }
    frame_.credit_.captured_decrement = current_import;
    frame_.credit_.decrement_entered = true;
    stage_ = Stage::decrement_entered;
}
void NativeGuiTextShaderStorageFrame::CreditConsumer::decrement_returned(long actual_result) {
    require_stage(Stage::decrement_entered);
    frame_.credit_.decrement_result = actual_result;
    frame_.credit_.decrement_returned = true;
    stage_ = Stage::decremented;
}
void NativeGuiTextShaderStorageFrame::CreditConsumer::zero_entered() {
    require_stage(Stage::decremented);
    if (frame_.credit_.decrement_result != 0) {
        fail();
        throw std::invalid_argument("Raw Text shader zero observation requires the actual zero result");
    }
    frame_.credit_.zero_entered = true;
    stage_ = Stage::zero_entered;
}
void NativeGuiTextShaderStorageFrame::CreditConsumer::zero_returned() {
    require_stage(Stage::zero_entered);
    frame_.credit_.zero_returned = true;
    stage_ = Stage::zero_returned;
}
void NativeGuiTextShaderStorageFrame::CreditConsumer::slot_cleared() {
    const auto expected = frame_.credit_.decrement_result == 0 ? Stage::zero_returned : Stage::decremented;
    require_stage(expected);
    frame_.credit_.slot_cleared = true;
    frame_.credit_.credit = CreditPhase::returned;
    stage_ = Stage::cleared;
}
void NativeGuiTextShaderStorageFrame::CreditConsumer::finish_without_capture() {
    require_stage(Stage::opened);
    if (kind_ != Consumer::font_name) {
        fail();
        throw std::invalid_argument("Only the real equal-name font setter may finish without capture");
    }
    stage_ = Stage::no_capture;
}
void NativeGuiTextShaderStorageFrame::CreditConsumer::complete() {
    if (!active_ || frame_.active_consumer_ != this || frame_.credit_.failed ||
        (stage_ != Stage::no_capture && stage_ != Stage::null_capture && stage_ != Stage::cleared)) {
        fail();
        throw std::invalid_argument("Raw Text shader consumer requires whole-body normal completion");
    }
    if (stage_ == Stage::cleared) frame_.credit_.consumer_returned = true;
    stage_ = Stage::complete;
    active_ = false;
    frame_.active_consumer_ = nullptr;
    frame_.credit_.active = false;
    frame_.credit_.active_consumer = Consumer::none;
    frame_.credit_.active_font_context = nullptr;
    frame_.credit_.active_font_frame = nullptr;
    frame_.credit_.active_font_acquired = nullptr;
    frame_.credit_.active_retirement_context = nullptr;
    frame_.credit_.active_retirement_acquired = nullptr;
    frame_.credit_.active_retirement_shader = nullptr;
}
void NativeGuiTextShaderStorageFrame::CreditConsumer::fail() noexcept {
    frame_.credit_.failed = true;
    if (frame_.active_consumer_ == this) {
        frame_.active_consumer_ = nullptr;
        frame_.credit_.active = false;
    }
    active_ = false;
    stage_ = Stage::failed;
}
bool ensure_native_gui_text_font_shader_storage_00ab8ce0(NativeGuiTextReference& reference,
    NativeGuiTextShaderStorageContext& context, Frame& frame) {
    void* const raw = preflight(reference, context, frame);
    NoRawProducer producer;
    return ensure_shader(raw, view(reference.storage_context()), context, frame, false, producer);
}
bool ensure_native_gui_text_font_shader_storage_00ab8ce0(NativeGuiTextRawModelReference& reference,
    NativeGuiTextShaderStorageContext& context, Frame& frame) {
    frame.begin_raw(reference, context);
    try {
        void* const raw = preflight(reference, context, frame);
        Frame::RawProducer producer(frame);
        const bool attempted = ensure_shader(raw, view(reference.storage_context()), context, frame, true, producer);
        frame.complete_raw(reference, context, attempted);
        return attempted;
    } catch (...) {
        frame.fail_raw();
        throw;
    }
}
} // namespace bsp
