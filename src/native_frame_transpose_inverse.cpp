#include "bsp/native_frame_transpose_inverse.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/material_effect_plane.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native frame transpose reconstruction requires MSVC Win32 x87/SSE assembly.
#endif

namespace bsp {
__declspec(naked) void __fastcall transpose_inverse_native_frame_0085dea0(
    void*, const void*, const volatile float*) {
    __asm {
        sub esp, 18h
        push esi
        push edx
        mov esi, ecx
        call copy_native_camera_matrix_004134f0
        movss xmm0, dword ptr [esi + 30h]
        movss xmm1, dword ptr [esi + 34h]
        movss dword ptr [esp + 4], xmm0
        xorps xmm0, xmm0
        movss dword ptr [esi + 30h], xmm0
        movss dword ptr [esi + 34h], xmm0
        movss dword ptr [esp + 8], xmm1
        movss xmm1, dword ptr [esi + 38h]
        movss dword ptr [esi + 38h], xmm0
        fld dword ptr [esi + 4]
        movss xmm0, dword ptr [esi + 10h]
        fstp dword ptr [esi + 10h]
        movss dword ptr [esi + 4], xmm0
        fld dword ptr [esi + 8]
        movss xmm0, dword ptr [esi + 20h]
        fstp dword ptr [esi + 20h]
        movss dword ptr [esi + 8], xmm0
        fld dword ptr [esi + 18h]
        movss xmm0, dword ptr [esi + 24h]
        fstp dword ptr [esi + 24h]
        push 0
        push esi
        lea edx, [esp + 0ch]
        lea ecx, [esp + 18h]
        movss dword ptr [esp + 14h], xmm1
        movss dword ptr [esi + 18h], xmm0
        call transform_native_effect_direction_0042d0d0_no_normalize
        // Its Source RET4 pops matrix; remove the selected-false word here.
        add esp, 4
        movss xmm1, dword ptr [eax]
        mov ecx, dword ptr [esp + 20h]
        movss xmm0, dword ptr [ecx]
        movss xmm2, dword ptr [eax + 4]
        movss xmm3, dword ptr [eax + 8]
        movaps xmm4, xmm0
        subss xmm4, xmm1
        movaps xmm1, xmm0
        subss xmm1, xmm2
        subss xmm0, xmm3
        movss dword ptr [esi + 30h], xmm4
        movss dword ptr [esi + 34h], xmm1
        movss dword ptr [esi + 38h], xmm0
        pop esi
        add esp, 18h
        ret 4
    }
}
} // namespace bsp
