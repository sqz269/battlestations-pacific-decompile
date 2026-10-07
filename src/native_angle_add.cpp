#include "bsp/native_angle_add.hpp"

namespace bsp {
__declspec(naked) float __fastcall native_angle_add_00438aa0(
    const NativeAngleAddConstants*,void*,float,float) {
    __asm {
        fld dword ptr [esp+4]
        fadd dword ptr [esp+8]
        fstp dword ptr [esp+4]
        fld dword ptr [esp+4]
        mov edx,[ecx]
        fld qword ptr [edx]
        fcomi st(0),st(1)
        jc upper_arm
        mov edx,[ecx+8]
        fld qword ptr [edx]
        jmp lower_add
    lower_repeat:
        fxch st(1)
        fxch st(2)
    lower_add:
        fadd st(2),st(0)
        fxch st(2)
        fstp dword ptr [esp+4]
        fld dword ptr [esp+4]
        fxch st(1)
        fcomi st(0),st(1)
        jnc lower_repeat
        fstp st(0)
        fstp st(1)
        ret 8
    upper_arm:
        fstp st(0)
        mov edx,[ecx+4]
        fld qword ptr [edx]
        fxch st(1)
        fcomi st(0),st(1)
        jbe finish_in_range
        mov edx,[ecx+8]
        fld qword ptr [edx]
        jmp upper_subtract
    upper_repeat:
        fxch st(1)
    upper_subtract:
        fsub st(1),st(0)
        fxch st(1)
        fstp dword ptr [esp+4]
        fld dword ptr [esp+4]
        fcomi st(0),st(2)
        ja upper_repeat
        fstp st(2)
        fstp st(0)
        ret 8
    finish_in_range:
        fstp st(1)
        ret 8
    }
}
} // namespace bsp
