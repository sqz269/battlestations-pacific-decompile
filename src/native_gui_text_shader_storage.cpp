#include "bsp/native_gui_text_shader_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Frame = NativeGuiTextShaderStorageFrame;
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
void* load_current_renderer(const void* name, std::int32_t state, Word site,
    const NativeGuiTextReferenceContext& c, NativeGuiTextShaderStorageContext& context, Frame& frame) {
    auto& cache = c.retirement.resources.effects;
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
    void* result;
    if (frame.renderer_target == 0x00b318b0) {
        result = load_native_renderer_material_effect_00b318b0(
            frame.captured_renderer, name, cache, &frame.cache);
    } else {
        require(context.unknown_renderer48,
            "Raw Text shader requires genuine provider for CURRENT unknown renderer48");
        result = context.unknown_renderer48->call_renderer_48(frame.captured_renderer,
            frame.renderer_profile, frame.renderer_target, name, cache, frame.cache);
    }
    frame.result = result;
    frame.renderer_returned = true;
    return result;
}
void cleanup_name(const NativeGuiTextReferenceContext& c, Frame& frame) noexcept {
    frame.exception_cleanup_started = true;
    const auto state = frame.native_eh_state;
    frame.native_eh_state = -1; // Consume before entering the required raw cleanup.
    if (state != 0 && state != 1) return;
    frame.cleanup_entered = true;
    location(frame, state == 0 ? 0x00cb7fe3u : 0x00cb7febu, Frame::Boundary::string_return);
    try {
        destroy_native_string_header_0041dd20(header(frame, static_cast<unsigned>(state)),
            c.retirement.resources.strings);
        frame.cleanup_returned = true;
    } catch (...) {
        std::terminate(); // No retry/double return after cleanup escape.
    }
}
} // namespace

NativeGuiTextShaderStorageFrame::~NativeGuiTextShaderStorageFrame() noexcept {
    if (phase == Phase::entered || phase == Phase::failed) std::terminate();
}
bool ensure_native_gui_text_font_shader_storage_00ab8ce0(NativeGuiTextReference& reference,
    NativeGuiTextShaderStorageContext& context, Frame& frame) {
    void* const raw = preflight(reference, context, frame);
    const auto& c = reference.storage_context();
    auto& strings = c.retirement.resources.strings;
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
            void* const effect = load_current_renderer(at(raw, 0x1c0), -1, 0x00ab8d5b, c, context, frame);
            write(raw, 0x1ec, bits(effect)); // Transfer owned return, no added retain.
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
                point = ordered_font_scale_match(frame.captured_font, &c.construction_context.one_00d7a24c);
                frame.font_scale_matches = point;
            }
            const unsigned index = point ? 0u : 1u;
            const auto* literal = static_cast<const char*>(c.data.data_at(
                point ? 0x00d5c794u : 0x00cefd78u, point ? 13u : 21u));
            location(frame, point ? 0x00ab8db0u : 0x00ab8dfbu, Frame::Boundary::string_construct);
            construct_native_string_header_0041e870(header(frame, index), strings, literal);
            frame.default_constructed = true;
            void* const effect = load_current_renderer(header(frame, index), point ? 0 : 1,
                point ? 0x00ab8dcdu : 0x00ab8e18u, c, context, frame);
            void* data;
            if (point) {
                data = pointer(read(header(frame, 0), 4)); // AB8DCF BEFORE publication.
                write(raw, 0x1ec, bits(effect));
            } else {
                write(raw, 0x1ec, bits(effect)); // AB8E1A BEFORE captured data.
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
} // namespace bsp
