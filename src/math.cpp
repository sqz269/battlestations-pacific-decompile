#include "bsp/math.hpp"
#include <cmath>

namespace bsp {
float abs_00401170(float value) { return std::fabs(value); }

// Assembly evidence: bsp.py show 00401c20/00401cb0/00401cd0 --asm.
// The old double expressions allowed SSE arithmetic, which ignores the Dyn
// worker's x87 precision control. Retain the native instruction/store schedule
// at the caller's current precision (including overlapping input/output views).
__declspec(noinline) void cross_reversed_00401c20(Vec3d& destination, const Vec3d& a, const Vec3d& b) {
    __asm {
        mov eax, destination
        mov ecx, a
        mov edx, b
        fld qword ptr [ecx + 10h]
        fmul qword ptr [edx + 8]
        fld qword ptr [edx + 10h]
        fmul qword ptr [ecx + 8]
        fsubp st(1), st(0)
        fstp qword ptr [eax]
        fld qword ptr [edx + 10h]
        fmul qword ptr [ecx]
        fld qword ptr [ecx + 10h]
        fmul qword ptr [edx]
        fsubp st(1), st(0)
        fstp qword ptr [eax + 8]
        fld qword ptr [ecx + 8]
        fmul qword ptr [edx]
        fld qword ptr [ecx]
        fmul qword ptr [edx + 8]
        fsubp st(1), st(0)
        fstp qword ptr [eax + 10h]
    }
}

__declspec(noinline) void subtract_reversed_00401cb0(Vec3d& destination, const Vec3d& a, const Vec3d& b) {
    __asm {
        mov eax, destination
        mov ecx, a
        mov edx, b
        fld qword ptr [edx]
        fsub qword ptr [ecx]
        fstp qword ptr [eax]
        fld qword ptr [edx + 8]
        fsub qword ptr [ecx + 8]
        fstp qword ptr [eax + 8]
        fld qword ptr [edx + 10h]
        fsub qword ptr [ecx + 10h]
        fstp qword ptr [eax + 10h]
    }
}

__declspec(noinline) void scale_00401cd0(Vec3d& value, double factor) {
    __asm {
        mov eax, value
        fld qword ptr [eax]
        fld qword ptr factor
        fmul st(1), st(0)
        fxch st(1)
        fstp qword ptr [eax]
        fld qword ptr [eax + 8]
        fmul st(0), st(1)
        fstp qword ptr [eax + 8]
        fmul qword ptr [eax + 10h]
        fstp qword ptr [eax + 10h]
    }
}

double length_squared_00401cf0(const Vec3d& value) {
    // Assembly adds y^2 + x^2 first, then z^2. Pseudocode reassociated this.
    return (value.y * value.y + value.x * value.x) + value.z * value.z;
}

// PUSHFD/PUSHAD protect everything the original arithmetic-only leaves left
// unchanged. At ESP after PUSHAD: EDX +20, ECX +24, EAX +28, return +36;
// scale's original double is +40/+44. No floating conversion or CW change.
__declspec(naked) void cross_reversed_register_bridge() {
    __asm {
        pushfd
        pushad
        mov eax, [esp + 28]
        mov ecx, [esp + 24]
        mov edx, [esp + 20]
        push edx
        push ecx
        push eax
        call cross_reversed_00401c20
        lea esp, [esp + 12]
        popad
        popfd
        ret
    }
}
__declspec(naked) void subtract_reversed_register_bridge() {
    __asm {
        pushfd
        pushad
        mov eax, [esp + 28]
        mov ecx, [esp + 24]
        mov edx, [esp + 20]
        push edx
        push ecx
        push eax
        call subtract_reversed_00401cb0
        lea esp, [esp + 12]
        popad
        popfd
        ret
    }
}
__declspec(naked) void scale_register_bridge() {
    __asm {
        pushfd
        pushad
        mov eax, [esp + 28]
        mov ecx, [esp + 40]
        mov edx, [esp + 44]
        push edx
        push ecx
        push eax
        call scale_00401cd0
        lea esp, [esp + 12]
        popad
        popfd
        ret 8
    }
}
}
