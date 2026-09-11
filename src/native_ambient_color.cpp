#include "bsp/native_ambient_color.hpp"
#include "bsp/native_render_batch_keys.hpp"

namespace bsp {

__declspec(naked) const void* __fastcall native_ambient_color_address_00b84c60(
    const void*) noexcept {
    __asm { lea eax, [ecx + 8] }
    __asm { ret }
}

__declspec(naked) void* __fastcall convert_native_float_rgba_to_argb_004fb850(
    void*, const void*, const volatile double*, const volatile std::uint32_t*) {
    __asm {
        push esi
        push edi
        mov edi, edx
        fld dword ptr [edi]
        mov esi, ecx
        mov eax, dword ptr [esp + 0ch]
        fld qword ptr [eax]
        // DC C9 multiplies ST1 by ST0, retaining the original scale in ST0.
        fmul st(1), st(0)
        fxch st(1)
        mov ecx, dword ptr [esp + 10h]
        call native_crt_truncate_st0_00bf7420
        test eax, eax
        jge red_nonnegative
        xor eax, eax
        jmp red_store
    red_nonnegative:
        cmp eax, 0ffh
        jle red_store
        mov eax, 0ffh
    red_store:
        mov byte ptr [esi + 2], al
        fld dword ptr [edi + 4]
        fmul st(0), st(1)
        mov ecx, dword ptr [esp + 10h]
        call native_crt_truncate_st0_00bf7420
        test eax, eax
        jge green_nonnegative
        xor eax, eax
        jmp green_store
    green_nonnegative:
        cmp eax, 0ffh
        jle green_store
        mov eax, 0ffh
    green_store:
        mov byte ptr [esi + 1], al
        fld dword ptr [edi + 8]
        fmul st(0), st(1)
        mov ecx, dword ptr [esp + 10h]
        call native_crt_truncate_st0_00bf7420
        test eax, eax
        jge blue_nonnegative
        xor eax, eax
        jmp blue_store
    blue_nonnegative:
        cmp eax, 0ffh
        jle blue_store
        mov eax, 0ffh
    blue_store:
        mov byte ptr [esi], al
        fmul dword ptr [edi + 0ch]
        mov ecx, dword ptr [esp + 10h]
        call native_crt_truncate_st0_00bf7420
        test eax, eax
        jge alpha_nonnegative
        xor eax, eax
        mov byte ptr [esi + 3], al
        pop edi
        mov eax, esi
        pop esi
        ret 8
    alpha_nonnegative:
        cmp eax, 0ffh
        jle alpha_store
        mov eax, 0ffh
    alpha_store:
        mov byte ptr [esi + 3], al
        pop edi
        mov eax, esi
        pop esi
        ret 8
    }
}

} // namespace bsp
