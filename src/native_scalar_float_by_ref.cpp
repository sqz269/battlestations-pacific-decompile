#include "bsp/native_scalar_float_by_ref.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scalar float by-reference leaves require MSVC Win32 x87/SSE.
#endif

namespace bsp {
// Complete 00415510..0041554D; trailing addresses identify native instructions.
__declspec(naked) float __fastcall min_native_float_by_ref_00415510(
    const float*, const float*) {
    __asm {
        sub esp,0x8 // 00415510
        fld dword ptr [ecx] // 00415513
        fstp dword ptr [esp] // 00415515
        fld dword ptr [edx] // 00415518
        fstp dword ptr [esp + 0x4] // 0041551a
        fld dword ptr [esp] // 0041551e
        fld dword ptr [esp + 0x4] // 00415521
        fcomip st(0),st(1) // 00415525: DF F1
        fstp st(0) // 00415527
        jbe l_0041553c // 00415529: right spill for equality/unordered
        movss xmm0,dword ptr [esp] // 0041552b
        movss dword ptr [esp],xmm0 // 00415530
        fld dword ptr [esp] // 00415535
        add esp,0x8 // 00415538
        ret // 0041553b
    l_0041553c:
        movss xmm0,dword ptr [esp + 0x4] // 0041553c
        movss dword ptr [esp],xmm0 // 00415542
        fld dword ptr [esp] // 00415547
        add esp,0x8 // 0041554a
        ret // 0041554d
    }
}
} // namespace bsp
