#include "bsp/native_gui_text_default_storage.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_wide_string.hpp"
#include <cstdlib>
#include <exception>
#include <initializer_list>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text default construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
void* at(void* storage, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(storage) + offset;
}
volatile Word& word(void* storage, std::size_t offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(storage, offset));
}
volatile unsigned char& byte(void* storage, std::size_t offset) noexcept {
    return *static_cast<volatile unsigned char*>(at(storage, offset));
}
struct Region { const volatile void* begin; std::size_t size; };
bool disjoint(Region l, Region r) noexcept {
    const auto left = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(l.begin));
    const auto right = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(r.begin));
    return left + l.size <= right || right + r.size <= left;
}
// Read the existing FLOAT object's physical DWORD without a floating-point
// lvalue conversion or wrong-type C++ alias. MOVSS in Original also transports
// all signaling-NaN bits unchanged; never perform FLD/FSTP on these defaults.
__declspec(noinline) Word shadow_word(const volatile float& channel) noexcept {
    const volatile void* source = &channel;
    Word result;
    __asm {
        mov eax, source
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}
void preflight(void* raw, NativeGuiTextDefaultStorageFrame& f,
    NativeGuiTextDefaultStorageContext& c, NativeGuiTextDefaultStorageAcquired& a) {
    auto& s = c.sections;
    auto& actual = s.actual;
    auto& owners = actual.geometry.actual_owners();
    const auto mesh = actual.geometry.mesh_constants();
    require(raw && (reinterpret_cast<std::uintptr_t>(raw) & 3u) == 0 &&
        !a.started && !a.complete && a.native_eh_state == -1 && !a.sections.started,
        "Text default requires fresh aligned storage and one fresh complete retained graph");
    const Region writes[] = {{raw, 0x1f4}, {&f, sizeof(f)}, {&a, sizeof(a)},
        {&f.sections, sizeof(f.sections)}, {&f.sections.locals[0], sizeof(f.sections.locals)},
        {&f.sections.parenting, sizeof(f.sections.parenting)}};
    for (std::size_t i = 0; i != 6; ++i) {
        for (std::size_t j = i + 1; j != 6; ++j)
            require(disjoint(writes[i], writes[j]), "Text default caller regions must be disjoint");
        for (const Region borrowed : {Region{&c, sizeof(c)}, {&s, sizeof(s)},
            {&c.lifetime, sizeof(c.lifetime)}, {&c.state, sizeof(c.state)},
            {&actual, sizeof(actual)}, {&s.node_constants, sizeof(s.node_constants)},
            {&s.strings, sizeof(s.strings)}, {&s.string_process, sizeof(s.string_process)},
            {&s.effects, sizeof(s.effects)}, {&s.parenting, sizeof(s.parenting)},
            {&s.decrement_00ce2220, sizeof(s.decrement_00ce2220)},
            {&actual.unchanged_00d7a260, sizeof(actual.unchanged_00d7a260)},
            {&s.effects.effects.construction.current_renderer_00f8d394, sizeof(void*)},
            {&c.shadow_offset_00d5c5c0, sizeof(Word)}, {&c.normal_rgb_00ce3e18, sizeof(Word)},
            {&c.one_00d7a24c, sizeof(Word)}, {&c.disabled_alpha_00ce3800, sizeof(Word)}})
            require(disjoint(writes[i], borrowed), "Text default writes cannot alias its provider/global graph");
    }
    // No read of fresh unconstructed Text188/4C. These are public provider
    // identity checks before the first AA9390 native effect; 85 rechecks its
    // current reached profile/domain at its own entry after the derived stores.
    require(&c.one_00d7a24c == &s.node_constants.one_00d7a24c &&
        &c.lifetime.nodes == &actual.models.nodes && c.lifetime.actual_base_table_00d5c130 &&
        actual.prepare_model && actual.retire_failed_model && actual.bind_completed_model &&
        !actual.models.actual_names && actual.models.nodes.uses_raw_name_pool() &&
        &actual.models.nodes.require_raw_name_pool() == &s.strings &&
        &s.strings == &s.string_process.raw_context() && &actual.strings == &s.string_process.strings() &&
        &s.parenting.node.strings == &s.strings && &actual.models.retained_owners == &owners &&
        &actual.materials.retained_owners == &owners && &s.parenting.node.scenes.owners == &owners &&
        &s.parenting.node.trees.owners == &owners &&
        &s.parenting.node.scenes.decrement_00ce2220 == &s.decrement_00ce2220 &&
        &s.parenting.node.trees.decrement_00ce2220 == &s.decrement_00ce2220 && s.decrement_00ce2220 &&
        &s.effects.effects.owners.actual_owners() == &owners && &s.effects.strings == &actual.strings &&
        &actual.materials.parameter_names == &actual.strings &&
        &s.effects.effects.construction.strings == &actual.strings && s.effects.effects.cache == &s.effects &&
        s.effects.effects.construction.current_renderer_00f8d394 &&
        word(s.effects.effects.construction.current_renderer_00f8d394) == 0x00d5f0a8u &&
        s.effects.effects.renderer_profile_00d5f0a8 &&
        s.effects.effects.renderer_profile_00d5f0a8[0x48 / 4] == 0x00b318b0u &&
        s.shadow_00d15ec4 && s.guidefault_00d5c25c && actual.material_vtable_00d5e520 &&
        &actual.models.constants.maximum_00ce4970 == &s.node_constants.positive_bound_00ce4970 &&
        &actual.models.constants.minimum_00ce4adc == &s.node_constants.negative_bound_00ce4adc &&
        &mesh.maximum_00ce4970 == &s.node_constants.positive_bound_00ce4970 &&
        &mesh.minimum_00ce4adc == &s.node_constants.negative_bound_00ce4adc &&
        &actual.models.constants.unchanged_00d7a260 == &actual.unchanged_00d7a260,
        "Text default requires the same genuine raw constants, string, owner and cleanup domains");
}
void unwind(void* raw, NativeGuiTextDefaultStorageContext& c,
    NativeGuiTextDefaultStorageAcquired& a) noexcept {
    a.exception_cleanup_started = true;
    try {
        // Source C++ projection of DEEEAC, regardless of a SOURCE provider's
        // exception-origin diagnostics. Do not interpret provider_call as an
        // Original/native exception classification or discover creators here.
        while (a.native_eh_state >= 0) {
            const auto state = a.native_eh_state;
            a.native_eh_state = state - 1;
            a.consumed_cleanup_actions |= Word{1} << state;
            switch (state) {
            case 6: a.active_call_site = 0x00cb8097;
                destroy_native_string_header_0041dd20(at(raw, 0x1c8), c.sections.strings); break;
            case 5: a.active_call_site = 0x00cb8089;
                destroy_native_string_header_0041dd20(at(raw, 0x1c0), c.sections.strings); break;
            case 4: a.active_call_site = 0x00cb807b;
                destroy_native_wide_string_header_00436430(at(raw, 0x1a4), c.sections.strings); break;
            case 3: a.active_call_site = 0x00cb806d;
                tidy_native_gui_text_pointer_vector_005d6cf0(at(raw, 0x194)); break;
            case 2: a.active_call_site = 0x00cb805f;
                destroy_native_string_header_0041dd20(at(raw, 0xf4), c.sections.strings); break;
            case 1: a.active_call_site = 0x00cb8051;
                destroy_native_wide_string_header_00436430(at(raw, 0xec), c.sections.strings); break;
            case 0: a.active_call_site = 0x00cb8043;
                destroy_native_gui_widget_base_00aa9730(raw, c.lifetime);
                a.base_lifetime_ended = true; break;
            default: std::terminate();
            }
        }
    } catch (...) { std::terminate(); }
}
} // namespace

void tidy_native_gui_text_pointer_vector_005d6cf0(void* vector) noexcept {
    void* const begin = reinterpret_cast<void*>(word(vector, 4));
    if (begin) std::free(begin); // 5D6CFB; SAME Source CRT backing, no element destruction
    word(vector, 4) = 0; // 5D6D03; even when begin was null
    word(vector, 8) = 0;
    word(vector, 0xc) = 0;
}

void* construct_native_gui_text_default_storage_00ab9650(void* raw,
    NativeGuiTextDefaultStorageFrame& f, NativeGuiTextDefaultStorageContext& c,
    NativeGuiTextDefaultStorageAcquired& a) {
    preflight(raw, f, c, a);
    a.started = true;
    a.destination = raw;
    try {
        a.active_call_site = 0x00ab9671;
        construct_native_gui_widget_base_00aa9390(raw, 3, c.one_00d7a24c);
        a.base_complete = true;
        word(raw) = 0x00d5c6c8; // AB9678
        a.native_eh_state = 0; // AB967E, AFTER the derived stamp
        word(raw, 0xec) = 0;
        word(raw, 0xf0) = 0;
        word(raw, 0xf4) = 0;
        word(raw, 0xf8) = 0;
        const Word shadow_offset = c.shadow_offset_00d5c5c0; // AB969D
        word(raw, 0x100) = 0;
        word(raw, 0x104) = 0;
        word(raw, 0x108) = 0;
        word(raw, 0x10c) = 0;
        word(raw, 0x110) = 0;
        word(raw, 0x114) = 0;
        byte(raw, 0x118) = 0;
        byte(raw, 0x15c) = 0;
        word(raw, 0x160) = 0;
        word(raw, 0x164) = shadow_offset;
        byte(raw, 0xfc) = 1;
        // Four distinct load/store pairs; no early/copy quartet or float conversion.
        word(raw, 0x168) = shadow_word(c.state.shadow_color_00e12fd8()[0]);
        word(raw, 0x16c) = shadow_word(c.state.shadow_color_00e12fd8()[1]);
        word(raw, 0x170) = shadow_word(c.state.shadow_color_00e12fd8()[2]);
        word(raw, 0x174) = shadow_word(c.state.shadow_color_00e12fd8()[3]);
        word(raw, 0x178) = 0;
        word(raw, 0x17c) = 2;
        word(raw, 0x180) = 0;
        word(raw, 0x184) = 0;
        word(raw, 0x188) = 0;
        word(raw, 0x18c) = 0;
        word(raw, 0x190) = 0;
        word(raw, 0x198) = 0;
        word(raw, 0x19c) = 0;
        word(raw, 0x1a0) = 0;
        word(raw, 0x1a4) = 0;
        word(raw, 0x1a8) = 0;
        word(raw, 0x1ac) = 0;
        word(raw, 0x1b0) = 0;
        byte(raw, 0x1b4) = 0;
        word(raw, 0x1c0) = 0;
        word(raw, 0x1c4) = 0;
        word(raw, 0x1c8) = 0;
        word(raw, 0x1cc) = 0;
        const Word normal = c.normal_rgb_00ce3e18; // AB979B, one capture
        const Word one = c.one_00d7a24c; // AB97A3; distinct from both AA9390 loads
        word(raw, 0x1ec) = 0;
        word(raw, 0x11c) = normal;
        byte(raw, 0x1f0) = 1;
        word(raw, 0x120) = normal;
        word(raw, 0x124) = normal;
        a.native_eh_state = 6; // AB97F1
        word(raw, 0x1d0) = 0xffffffff;
        word(raw, 0x1d8) = one;
        word(raw, 0x128) = one;
        word(raw, 0x12c) = one;
        word(raw, 0x130) = one;
        word(raw, 0x134) = one;
        word(raw, 0x13c) = one;
        word(raw, 0x138) = one;
        const Word disabled_alpha = c.disabled_alpha_00ce3800; // AB987C, BEFORE140
        word(raw, 0x140) = one;
        word(raw, 0x144) = one;
        word(raw, 0x14c) = 0;
        word(raw, 0x148) = one;
        word(raw, 0x150) = 0;
        word(raw, 0x154) = 0;
        word(raw, 0x158) = disabled_alpha;
        a.derived_complete = true;
        a.sections_entered = true;
        a.active_call_site = 0x00ab98d8;
        ensure_native_gui_text_sections_storage_00ab8530(raw, f.sections, c.sections, a.sections);
        a.complete = true;
        return raw;
    } catch (...) {
        // AA9390 failure is outer state-1 and its own provider already cleans
        // the failed base prefix. After successful base construction, unwind
        // only the represented Text subobjects/base, never independent creators.
        if (a.native_eh_state >= 0) unwind(raw, c, a);
        throw;
    }
}
} // namespace bsp
