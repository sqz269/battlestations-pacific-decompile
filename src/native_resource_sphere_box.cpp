#include "bsp/native_resource_sphere_box.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native sphere bounds require MSVC Win32 x87/SSE instruction semantics.
#endif

namespace bsp {

// The retained x/y/z values occupy the x87 stack until all maxima have been
// rounded to float. Radius itself is then passed through x87 float stores
// before subtraction. A C++ c +/- r expression loses this schedule, notably
// for signaling NaNs, denormals, FP status and overlapping input/output.
__declspec(naked) float* __fastcall project_native_sphere_bounds_00b7d160(
    float*, const float*) noexcept {
    __asm {
        sub esp, 2ch
        fld dword ptr [edx]
        movss xmm0, dword ptr [edx + 0ch]
        fstp dword ptr [esp]
        movss dword ptr [esp + 8], xmm0
        fld dword ptr [esp]
        movss dword ptr [esp + 0ch], xmm0
        fld st(0)
        movss dword ptr [esp + 10h], xmm0
        fadd dword ptr [esp + 8]
        movss dword ptr [esp + 4], xmm0
        mov eax, ecx
        fstp dword ptr [esp + 20h]
        fld dword ptr [edx + 4]
        movss xmm0, dword ptr [esp + 20h]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fld st(0)
        fadd dword ptr [esp + 0ch]
        fstp dword ptr [esp + 24h]
        fld dword ptr [edx + 8]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fld st(0)
        fadd dword ptr [esp + 10h]
        fstp dword ptr [esp + 28h]
        fld dword ptr [esp + 4]
        fst dword ptr [esp + 8]
        fst dword ptr [esp + 0ch]
        fstp dword ptr [esp + 10h]
        fld dword ptr [esp + 8]
        fsubp st(3), st(0)
        fxch st(2)
        fstp dword ptr [esp + 14h]
        fsub dword ptr [esp + 0ch]
        fstp dword ptr [esp + 18h]
        fsub dword ptr [esp + 10h]
        fstp dword ptr [esp + 1ch]
        fld dword ptr [esp + 14h]
        fstp dword ptr [eax]
        fld dword ptr [esp + 18h]
        fstp dword ptr [eax + 4]
        fld dword ptr [esp + 1ch]
        fstp dword ptr [eax + 8]
        movss dword ptr [eax + 0ch], xmm0
        movss xmm0, dword ptr [esp + 24h]
        movss dword ptr [eax + 10h], xmm0
        movss xmm0, dword ptr [esp + 28h]
        movss dword ptr [eax + 14h], xmm0
        add esp, 2ch
        ret
    }
}

__declspec(naked) float* __fastcall set_native_bounds_from_sphere_00b7d220(
    float*, void*, const float*) noexcept {
    __asm {
        mov edx, dword ptr [esp + 4]
        sub esp, 18h
        push esi
        mov esi, ecx
        lea ecx, [esp + 4]
        call project_native_sphere_bounds_00b7d160
        fld dword ptr [eax]
        fstp dword ptr [esi]
        fld dword ptr [eax + 4]
        fstp dword ptr [esi + 4]
        fld dword ptr [eax + 8]
        fstp dword ptr [esi + 8]
        fld dword ptr [eax + 0ch]
        fstp dword ptr [esi + 0ch]
        fld dword ptr [eax + 10h]
        fstp dword ptr [esi + 10h]
        fld dword ptr [eax + 14h]
        mov eax, esi
        fstp dword ptr [esi + 14h]
        pop esi
        add esp, 18h
        ret 4
    }
}

} // namespace bsp
