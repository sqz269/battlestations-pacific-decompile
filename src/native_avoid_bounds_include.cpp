#include "bsp/native_avoid_bounds_include.hpp"

namespace bsp {
__declspec(naked) void __fastcall include_native_avoid_bounds_point_00415010(
    void*, void*, const float*) {
    __asm {
        mov edx, dword ptr [esp + 4]
        fld dword ptr [edx]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fld dword ptr [ecx]
        fcomip st(0), st(1)
        fstp st(0)
        jbe maximum_x
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx], xmm0
    maximum_x:
        fld dword ptr [edx]
        fstp dword ptr [esp + 4]
        fld dword ptr [ecx + 8]
        fld dword ptr [esp + 4]
        fcomip st(0), st(1)
        fstp st(0)
        jbe minimum_y
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx + 8], xmm0
    minimum_y:
        fld dword ptr [edx + 4]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fld dword ptr [ecx + 4]
        fcomip st(0), st(1)
        fstp st(0)
        jbe maximum_y
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx + 4], xmm0
    maximum_y:
        fld dword ptr [edx + 4]
        fstp dword ptr [esp + 4]
        fld dword ptr [ecx + 0ch]
        fld dword ptr [esp + 4]
        fcomip st(0), st(1)
        fstp st(0)
        jbe done
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx + 0ch], xmm0
    done:
        ret 4
    }
}
} // namespace bsp
