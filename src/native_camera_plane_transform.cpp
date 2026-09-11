#include "bsp/native_camera_plane_transform.hpp"

namespace bsp {

__declspec(naked) void* __fastcall transform_native_vector4_00b62d10(
    const void*, void*, const void*) {
    __asm {
        sub esp, 10h
        fld dword ptr [ecx + 4]
        mov eax, edx
        fstp dword ptr [esp]
        fld dword ptr [ecx]
        fstp dword ptr [esp + 4]
        fld dword ptr [ecx + 8]
        fstp dword ptr [esp + 8]
        fld dword ptr [ecx + 0ch]
        mov ecx, dword ptr [esp + 14h]
        fstp dword ptr [esp + 0ch]
        fld dword ptr [ecx + 10h]
        fld dword ptr [esp]
        fld st(0)
        fmulp st(2), st(0)
        fld dword ptr [esp + 4]
        fld st(0)
        fmul dword ptr [ecx]
        faddp st(3), st(0)
        fld dword ptr [ecx + 20h]
        fld dword ptr [esp + 8]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(4)
        faddp st(1), st(0)
        fld dword ptr [ecx + 30h]
        fld dword ptr [esp + 0ch]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(2)
        faddp st(1), st(0)
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fmul st(0), st(2)
        fld dword ptr [ecx + 14h]
        fmul st(0), st(4)
        faddp st(1), st(0)
        fld dword ptr [ecx + 24h]
        fmul st(0), st(5)
        faddp st(1), st(0)
        fld dword ptr [ecx + 34h]
        fmul st(0), st(2)
        faddp st(1), st(0)
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fmul st(0), st(2)
        fld dword ptr [ecx + 18h]
        fmul st(0), st(4)
        faddp st(1), st(0)
        fld dword ptr [ecx + 28h]
        fmul st(0), st(5)
        faddp st(1), st(0)
        fld dword ptr [ecx + 38h]
        fmul st(0), st(2)
        faddp st(1), st(0)
        fstp dword ptr [eax + 8]
        fld dword ptr [ecx + 0ch]
        fmulp st(2), st(0)
        fld dword ptr [ecx + 1ch]
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ecx + 2ch]
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul dword ptr [ecx + 3ch]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0ch]
        add esp, 10h
        ret 4
    }
}

__declspec(naked) void* __fastcall transform_native_plane_00b65ba0(
    const void*, void*, const void*) {
    __asm {
        push edx // Preserve the caller's output independently of the child.
        sub esp, 20h
        movss xmm0, dword ptr [ecx]
        mov eax, dword ptr [esp + 28h]
        movss dword ptr [esp], xmm0
        movss xmm0, dword ptr [ecx + 4]
        movss dword ptr [esp + 4], xmm0
        movss xmm0, dword ptr [ecx + 8]
        movss dword ptr [esp + 8], xmm0
        movss xmm0, dword ptr [ecx + 0ch]
        push eax
        lea edx, [esp + 14h]
        lea ecx, [esp + 4]
        movss dword ptr [esp + 10h], xmm0
        call transform_native_vector4_00b62d10
        movss xmm0, dword ptr [eax]
        movss dword ptr [esp], xmm0
        movss xmm1, dword ptr [eax + 4]
        movss dword ptr [esp + 4], xmm1
        movss xmm2, dword ptr [eax + 8]
        movss dword ptr [esp + 8], xmm2
        movss xmm3, dword ptr [eax + 0ch]
        mov eax, dword ptr [esp + 20h]
        movss dword ptr [eax], xmm0
        movss dword ptr [eax + 4], xmm1
        movss dword ptr [eax + 8], xmm2
        movss dword ptr [eax + 0ch], xmm3
        add esp, 24h
        ret 4
    }
}

} // namespace bsp
