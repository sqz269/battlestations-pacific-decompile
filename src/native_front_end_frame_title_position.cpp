#include "bsp/native_front_end_frame_title_position.hpp"

#include "bsp/cc13_gui1.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeFrontEndFrameTitleScratch) == 32);
static_assert(offsetof(NativeFrontEndFrameTitleScratch, frame_box_name_header) == 8);
static_assert(offsetof(NativeFrontEndFrameTitleScratch, plus_or_fixed_size_float_bits) == 16);
static_assert(offsetof(NativeFrontEndFrameTitleScratch, minimum_size_float_bits) == 24);
static_assert(sizeof(NativeFrontEndFramePositionScratch) == 16);

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
void put(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::uintptr_t>(address(p) + offset)) = value;
}
Word selector_bits(const volatile std::int32_t& selector) noexcept {
    return static_cast<Word>(selector);
}

const void* authenticated_data(const game::GameNativeReadOnlyData& data,
    Word va, Word bytes, Word expected_fnv) {
    const auto* const p = static_cast<const unsigned char*>(data.data_at(va, bytes));
    Word fingerprint = 2166136261u;
    for (Word i = 0; i != bytes; ++i) fingerprint = (fingerprint ^ p[i]) * 16777619u;
    if (reinterpret_cast<std::uintptr_t>(p) != va || fingerprint != expected_fnv)
        throw std::logic_error("Frame title data differs from authenticated original PE data");
    return p;
}

void* current_title_page(const NativeFrontEndFrameTitlePositionContext& c,
    bool selector_before_own) {
    // The getter results are separate identities: first supplies the array,
    // second supplies own160. No early publication/selection snapshot.
    void* const slots_frame = c.bindings.frame_004c1ac0();
    void* const index_frame = c.bindings.frame_004c1ac0();
    Word own;
    Word selected;
    if (selector_before_own) {
        selected = selector_bits(c.title_00e18d8c);
        own = load(index_frame, 0x160);
    } else {
        own = load(index_frame, 0x160);
        selected = selector_bits(c.title_00e18d8c);
    }
    return pointer(load(slots_frame, (selected + own * 2u + 0x4eu) * 4u));
}

void* find_and_return_name(void* actual_header8,
    const NativeFrontEndFrameTitlePositionContext& c, bool selector_before_own) {
    void* captured_widget;
    try {
        void* const page = current_title_page(c, selector_before_own);
        captured_widget = c.bindings.find_child_00aa7e00(page, actual_header8, 1);
    } catch (...) {
        // Each FH3 map arms just this header, after construction/copy returns.
        // Ordinary Source secondary transport only; no terminate/retry policy.
        destroy_native_string_header_0041dd20(actual_header8, c.strings);
        throw;
    }
    // Native state=-1 BEFORE this normal return. Capture CURRENT header data
    // then CURRENT length+1 inside actual41DD20; pool callbacks cannot retrigger it.
    destroy_native_string_header_0041dd20(actual_header8, c.strings);
    return captured_widget;
}

void construct_position_name(void* actual_header8, const char* actual_literal,
    Word length, const NativeFrontEndFrameTitlePositionContext& c) {
    put(actual_header8, 0, 0);
    put(actual_header8, 4, 0);
    resize_native_string_header_0041dd40(actual_header8, c.strings, length, true);
    void* const captured_data = pointer(load(actual_header8, 4));
    if (captured_data) {
        const Word count = load(actual_header8) + 1u;
        copy_native_crt_bytes_00bf7680(captured_data, actual_literal, count,
            c.actual_feature_word_0109eea4);
    }
    // Caller arms cleanup only after the actual copy path returns.
}

void copy_position_components(const void* captured_source_xyz, void* captured_output_xyz) {
    // Sequential loads/stores retain overlap and the reached ambient FP state.
    // No memcpy/vector snapshot; every component is read AFTER the prior store.
    __asm {
        mov eax, captured_source_xyz
        mov edx, captured_output_xyz
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax + 4]
        fstp dword ptr [edx + 4]
        fld dword ptr [eax + 8]
        fstp dword ptr [edx + 8]
    }
}

void constant_width_size(const void* captured_size, const void* actual_width,
    Word* actual_float2) {
    __asm {
        mov eax, captured_size
        movss xmm0, dword ptr [eax + 4]
        mov edx, actual_width
        movss xmm1, dword ptr [edx]
        mov ecx, actual_float2
        movss dword ptr [ecx], xmm1
        movss dword ptr [ecx + 4], xmm0
    }
}
} // namespace

NativeFrontEndFrameTitlePositionData borrow_native_front_end_frame_title_position_data(
    const game::GameNativeReadOnlyData& data) {
    const auto* const title = static_cast<const char*>(
        authenticated_data(data, 0x00cec3d8u, 11, 0x00ccf4cbu));
    const auto* const box = static_cast<const char*>(
        authenticated_data(data, 0x00cec3c4u, 18, 0x81a834c3u));
    const void* const divisor = authenticated_data(data, 0x00cec380u, 8, 0xde8b100bu);
    const void* const pad = authenticated_data(data, 0x00cec3e8u, 8, 0xf824fd23u);
    const void* const minimum = authenticated_data(data, 0x00cec3e4u, 4, 0x119be001u);
    const void* const fixed = authenticated_data(data, 0x00ce3e18u, 4, 0x3d39a85du);
    return NativeFrontEndFrameTitlePositionData(data, title, box, divisor, pad, minimum, fixed);
}

__declspec(noinline) void set_native_front_end_frame_title_005189b0(
    const void* captured_native_header8, const NativeFrontEndFrameTitlePositionContext& c,
    NativeFrontEndFrameTitleScratch& scratch) {
    if (!current_title_page(c, false)) return;

    construct_native_string_header_0041e870(scratch.title_text_name_header, c.strings,
        c.data.title_text_name_00cec3d8());
    void* const captured_text = find_and_return_name(scratch.title_text_name_header, c, false);
    construct_native_string_header_0041e870(scratch.frame_box_name_header, c.strings,
        c.data.frame_box_name_00cec3c4());
    void* const captured_box = find_and_return_name(scratch.frame_box_name_header, c, true);

    c.bindings.text_source_00abaed0(captured_text, captured_native_header8, 1);
    void* const captured_game = c.actual_game_00e188a8;
    const auto* const game_bytes = static_cast<const volatile unsigned char*>(captured_game);
    Word* actual_size;
    if (game_bytes[0x61f] != 0 || game_bytes[0x634] != 0) {
        const void* const captured_size = c.bindings.widget_size_00aa6740(captured_box);
        actual_size = scratch.plus_or_fixed_size_float_bits;
        constant_width_size(captured_size, c.data.fixed_width_00ce3e18(), actual_size);
    } else {
        const void* const actual_divisor = c.data.divisor_00cec380();
        const void* const actual_pad = c.data.pad_00cec3e8();
        Word* const quotient = &scratch.title_text_name_header[0];
        Word minimum_arm;
        // Exact 518AE7..518B09/27 comparison. FDIV uses a real double, the
        // quotient rounds to float, FCOMI+FSTP preserve flags for JBE, including
        // unordered -> minimum. Both arms empty this interval BEFORE the getter.
        __asm {
            mov eax, captured_text
            fld dword ptr [eax + 0x114]
            mov edx, actual_divisor
            fdiv qword ptr [edx]
            mov ecx, quotient
            fstp dword ptr [ecx]
            mov edx, actual_pad
            fld qword ptr [edx]
            fld dword ptr [ecx]
            fcomi st(0), st(1)
            fstp st(1)
            jbe title_minimum_arm
            fstp dword ptr [ecx]
            mov minimum_arm, 0
            jmp title_comparison_done
        title_minimum_arm:
            fstp st(0)
            mov minimum_arm, 1
        title_comparison_done:
        }
        const void* const captured_size = c.bindings.widget_size_00aa6740(captured_box);
        if (minimum_arm) {
            actual_size = scratch.minimum_size_float_bits;
            constant_width_size(captured_size, c.data.minimum_width_00cec3e4(), actual_size);
        } else {
            actual_size = scratch.plus_or_fixed_size_float_bits;
            // The true getter has run between spills; reload BOTH the saved
            // float quotient and CURRENT pad at this reached native epoch.
            __asm {
                mov ecx, quotient
                fld dword ptr [ecx]
                mov eax, captured_size
                movss xmm0, dword ptr [eax + 4]
                mov edx, actual_pad
                fadd qword ptr [edx]
                mov ecx, actual_size
                fstp dword ptr [ecx]
                movss dword ptr [ecx + 4], xmm0
            }
        }
    }
    const Word profile = load(captured_box);
    // SAME authenticated original-VA RO table DATA; not a callable-pointer cast.
    const Word target = load(pointer(profile), 0x58);
    c.bindings.call_widget_virtual58(captured_box, profile, target, actual_size);
}

__declspec(noinline) void set_native_front_end_frame_title_position_005186b0(
    const void* captured_box_xyz, const void* captured_title_xyz,
    const NativeFrontEndFrameTitlePositionContext& c,
    NativeFrontEndFramePositionScratch& scratch) {
    construct_position_name(scratch.title_text_name_header, c.data.title_text_name_00cec3d8(), 10, c);
    void* const captured_text = find_and_return_name(scratch.title_text_name_header, c, true);
    construct_position_name(scratch.frame_box_name_header, c.data.frame_box_name_00cec3c4(), 17, c);
    void* const captured_box = find_and_return_name(scratch.frame_box_name_header, c, true);
    c.bindings.set_local_position_00aa7dc0(captured_box, captured_box_xyz);
    c.bindings.set_local_position_00aa7dc0(captured_text, captured_title_xyz);
}

__declspec(noinline) void get_native_front_end_frame_title_position_00518820(
    void* captured_box_output_xyz, void* captured_title_output_xyz,
    const NativeFrontEndFrameTitlePositionContext& c,
    NativeFrontEndFramePositionScratch& scratch) {
    construct_position_name(scratch.title_text_name_header, c.data.title_text_name_00cec3d8(), 10, c);
    void* const captured_text = find_and_return_name(scratch.title_text_name_header, c, true);
    construct_position_name(scratch.frame_box_name_header, c.data.frame_box_name_00cec3c4(), 17, c);
    void* const captured_box = find_and_return_name(scratch.frame_box_name_header, c, true);
    const void* const captured_box_position = cc13_gui1::gui_widget_get_local_position_00aa67f0(captured_box);
    copy_position_components(captured_box_position, captured_box_output_xyz);
    const void* const captured_title_position = cc13_gui1::gui_widget_get_local_position_00aa67f0(captured_text);
    copy_position_components(captured_title_position, captured_title_output_xyz);
}

__declspec(noinline) void collect_native_front_end_frame_children_00518e00(
    void* captured_frame, void* const volatile& actual_output_vector_home,
    const NativeFrontEndFrameTitlePositionContext& c) {
    const Word backdrop_own = load(captured_frame, 0x160);
    const Word backdrop = selector_bits(c.backdrop_00e18d84);
    const Word backdrop_offset = (backdrop_own * 13u + backdrop) * 4u + 0xcu;
    const bool backdrop_present = load(captured_frame, backdrop_offset) != 0;
    void* const backdrop_slot = pointer(address(captured_frame) + backdrop_offset);
    // Native loads this home even when the first slot is null, but not before
    // that slot's comparison. It stays captured across every subsequent append.
    void* const captured_output = actual_output_vector_home;
    if (backdrop_present) c.bindings.push_page_slot_004d6790(captured_output, backdrop_slot);

    const Word panel_own = load(captured_frame, 0x160);
    const Word panel = selector_bits(c.panel_00e18d88);
    const Word panel_offset = (panel + panel_own * 2u + 0x44u) * 4u;
    const bool panel_present = load(captured_frame, panel_offset) != 0;
    void* const panel_slot = pointer(address(captured_frame) + panel_offset);
    if (panel_present) c.bindings.push_page_slot_004d6790(captured_output, panel_slot);

    const Word title_own = load(captured_frame, 0x160);
    const Word title = selector_bits(c.title_00e18d8c);
    const Word title_offset = (title + title_own * 2u + 0x4eu) * 4u;
    const bool title_present = load(captured_frame, title_offset) != 0;
    void* const title_slot = pointer(address(captured_frame) + title_offset);
    if (title_present) c.bindings.push_page_slot_004d6790(captured_output, title_slot);
}
} // namespace bsp
