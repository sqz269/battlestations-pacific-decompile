#include "bsp/native_front_end_frame_operations.hpp"

#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeFrontEndFrameFlagScratch) == 8);

Word address(const volatile void* p) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
Word load(const volatile void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<std::uintptr_t>(address(p) + offset));
}
Word selector_bits(const volatile std::int32_t& selector) noexcept {
    return static_cast<Word>(selector);
}

void apply_small_group(void* frame, Word base_word,
    const volatile std::int32_t& selector,
    const NativeFrontEndFrameOperationsContext& c) {
    for (Word slot = 0; slot < 2; ++slot) {
        const Word own = load(frame, 0x160);
        const Word index = slot + own * 2u + base_word;
        if (load(frame, index * 4u) == 0) continue;
        const Word selected = selector_bits(selector);
        // Native keeps the SAME own value for this group's test and call.
        void* const widget = pointer(load(frame, index * 4u));
        const Word profile = load(widget);
        // Raw original-VA TABLE DATA read, never a callable-pointer cast.
        const Word target = load(pointer(profile), 0x34);
        const Word carrier = (index & 0xffffff00u) | static_cast<Word>(slot == selected);
        c.bindings.call_widget_virtual34(widget, profile, target, carrier);
    }
}
} // namespace

NativeFrontEndFrameOperationLiterals borrow_native_front_end_frame_operation_literals(
    const game::GameNativeReadOnlyData& data) {
    const auto* const bytes = static_cast<const unsigned char*>(data.data_at(0x00cec3f0u, 16));
    Word fingerprint = 2166136261u;
    for (Word i = 0; i != 16; ++i) fingerprint = (fingerprint ^ bytes[i]) * 16777619u;
    if (reinterpret_cast<std::uintptr_t>(bytes) != 0x00cec3f0u ||
        bytes[15] != 0 || fingerprint != 0x53993661u)
        throw std::logic_error("Frame flag literal differs from authenticated original PE data");
    return NativeFrontEndFrameOperationLiterals(data, reinterpret_cast<const char*>(bytes));
}

__declspec(noinline) void set_native_front_end_frame_flag_00518b90(
    Word flag, const NativeFrontEndFrameOperationsContext& c,
    NativeFrontEndFrameFlagScratch& scratch) {
    // Native CMP panel,0; MOV flag,ECX; JL. The branch uses PRESTORE comparison.
    const bool negative_panel = c.panel_00e18d88 < 0;
    c.flag_00e08654 = flag;
    if (negative_panel) return;

    void* const initial_slots_frame = c.bindings.frame_004c1ac0();
    void* const initial_index_frame = c.bindings.frame_004c1ac0();
    const Word initial_own = load(initial_index_frame, 0x160);
    const Word initial_panel = selector_bits(c.panel_00e18d88);
    const Word initial_index = initial_panel + initial_own * 2u + 0x44u;
    if (load(initial_slots_frame, initial_index * 4u) == 0) return;

    // Native state0 is armed only AFTER the actual constructor returns.
    construct_native_string_header_0041e870(scratch.name_header, c.strings,
        c.literals.flag_icon_name_00cec3f0());
    try {
        void* const slots_frame = c.bindings.frame_004c1ac0();
        void* const index_frame = c.bindings.frame_004c1ac0();
        const Word own = load(index_frame, 0x160);
        const Word panel = selector_bits(c.panel_00e18d88);
        void* const page = pointer(load(slots_frame, (panel + own * 2u + 0x44u) * 4u));
        void* const captured_widget = c.bindings.find_child_00aa7e00(
            page, scratch.name_header, 1);

        Word captured_profile;
        Word captured_target;
        Word frame_word;
        Word alpha_float_bits;
        volatile Word* const flag_word_address = &c.flag_00e08654;
        // Exact native FLD1 -> actual CURRENT profile/+88 -> CURRENT LOW WORD
        // -> FSTP interval. No C++ resolver/provider call while ST0 is live.
        // The numeric profile is authenticated original-VA TABLE DATA backing.
        __asm {
            mov eax, captured_widget
            fld1
            mov edx, dword ptr [eax]
            mov captured_profile, edx
            mov edx, dword ptr [edx + 0x88]
            mov captured_target, edx
            mov eax, flag_word_address
            movzx ecx, word ptr [eax]
            mov frame_word, ecx
            fstp dword ptr [alpha_float_bits]
        }
        c.bindings.call_widget_virtual88(captured_widget, captured_profile,
            captured_target, frame_word, 0, alpha_float_bits);
    } catch (...) {
        // Once-only ordinary Source cleanup. A cleanup exception escapes;
        // Native secondary FH3/SEH identity is not inferred or re-created.
        destroy_native_string_header_0041dd20(scratch.name_header, c.strings);
        throw;
    }
    // Native state=-1 before explicit normal return; never retry this cleanup.
    destroy_native_string_header_0041dd20(scratch.name_header, c.strings);
}

__declspec(noinline) void apply_native_front_end_frame_selection_00518c80(
    void* captured_frame, const NativeFrontEndFrameOperationsContext& c,
    NativeFrontEndFrameFlagScratch& scratch) {
    apply_small_group(captured_frame, 0x44, c.panel_00e18d88, c);
    apply_small_group(captured_frame, 0x4e, c.title_00e18d8c, c);

    void* const current_game = c.actual_game_00e188a8;
    if (load(current_game, 0x5d4) == 0x0d) return;
    for (Word slot = 0; slot < 13; ++slot) {
        const Word tested_own = load(captured_frame, 0x160);
        const Word tested_index = tested_own * 13u + slot;
        if (load(captured_frame, 0x0c + tested_index * 4u) == 0) continue;
        // Backdrop rereads own after the slot test, unlike panel/title.
        const Word called_own = load(captured_frame, 0x160);
        const Word called_index = called_own * 13u + slot;
        const Word selected = selector_bits(c.backdrop_00e18d84);
        void* const widget = pointer(load(captured_frame, 0x0c + called_index * 4u));
        const Word profile = load(widget);
        const Word target = load(pointer(profile), 0x34);
        const Word carrier = (called_index & 0xffffff00u) |
            static_cast<Word>(slot == selected);
        c.bindings.call_widget_virtual34(widget, profile, target, carrier);
    }
    const Word current_flag = c.flag_00e08654;
    set_native_front_end_frame_flag_00518b90(current_flag, c, scratch);
}

__declspec(noinline) void select_native_front_end_frame_00518d60(
    Word backdrop, Word panel, const volatile Word& title_dword_home,
    const void* const volatile& text_pointer_home,
    const NativeFrontEndFrameOperationsContext& c,
    NativeFrontEndFrameFlagScratch& scratch) {
    const Word title = title_dword_home; // FIRST native input-home read.
    c.backdrop_00e18d84 = static_cast<std::int32_t>(backdrop);
    c.panel_00e18d88 = static_cast<std::int32_t>(panel);
    c.title_00e18d8c = static_cast<std::int32_t>(title);
    void* const frame = c.bindings.frame_004c1ac0();
    apply_native_front_end_frame_selection_00518c80(frame, c, scratch);
    const void* const current_text = text_pointer_home;
    if (load(current_text) != 0) c.bindings.title_005189b0(current_text);
}

__declspec(noinline) void select_native_front_end_frame_backdrop_00518da0(
    Word backdrop, const NativeFrontEndFrameOperationsContext& c,
    NativeFrontEndFrameFlagScratch& scratch) {
    c.backdrop_00e18d84 = static_cast<std::int32_t>(backdrop);
    void* const frame = c.bindings.frame_004c1ac0();
    apply_native_front_end_frame_selection_00518c80(frame, c, scratch);
}

} // namespace bsp
