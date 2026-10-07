#include "bsp/native_session_target_operations.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Session target operations reconstruction requires MSVC Win32.
#endif

namespace bsp {
void release_native_session_target_slot_007827d0(std::uint32_t original_index,
    volatile std::uint16_t& actual_slots_f871b0) noexcept {
    volatile std::uint16_t* const slots = &actual_slots_f871b0;
    __asm {
        mov ecx, original_index
        cmp ecx, 0fh
        ja slot_done
        mov eax, 1
        shl eax, cl
        not eax
        mov edx, slots
        and word ptr [edx], ax
    slot_done:
    }
}

void append_native_session_target_history_007831e0(NativeSessionTargetStorage* target,
    std::int32_t original_sample) noexcept {
    __asm {
        mov ecx, target
        mov eax, dword ptr [ecx + 0d54h]
        mov ecx, dword ptr [eax + 10h]
        fld dword ptr [eax + 14h]
        mov edx, dword ptr [eax + 0ch]
        fsub dword ptr [edx + ecx * 4]
        cvtsi2ss xmm0, dword ptr original_sample
        lea ecx, [edx + ecx * 4]
        fstp dword ptr [eax + 14h]
        movss dword ptr [ecx], xmm0
        mov ecx, dword ptr [eax + 10h]
        mov edx, dword ptr [eax + 0ch]
        fld dword ptr [edx + ecx * 4]
        add ecx, 1
        cmp ecx, dword ptr [eax + 4]
        fadd dword ptr [eax + 14h]
        mov dword ptr [eax + 10h], ecx
        fstp dword ptr [eax + 14h]
        jne append_done
        mov dword ptr [eax + 10h], 0
    append_done:
    }
}

void reset_native_session_target_history_00783230(NativeSessionTargetStorage* target,
    std::uint32_t original_initial_float_word) noexcept {
    __asm {
        mov ecx, target
        mov eax, dword ptr [ecx + 0d54h]
        movss xmm0, dword ptr original_initial_float_word
        xor ecx, ecx
        cmp dword ptr [eax + 4], ecx
        movss dword ptr [eax + 8], xmm0
        jle reset_total
    reset_loop:
        mov edx, dword ptr [eax + 0ch]
        fld dword ptr [eax + 8]
        fstp dword ptr [edx + ecx * 4]
        add ecx, 1
        cmp ecx, dword ptr [eax + 4]
        jl reset_loop
    reset_total:
        fild dword ptr [eax + 4]
        mov dword ptr [eax + 10h], 0
        fmul dword ptr [eax + 8]
        fstp dword ptr [eax + 14h]
    }
}
} // namespace bsp
