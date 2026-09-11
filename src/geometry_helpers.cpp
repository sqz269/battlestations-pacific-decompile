#include "bsp/geometry_helpers.hpp"

// Actual host CRT x87 intrinsic entry. Its symbol is supplied by the Win32 UCRT
// import library; the original BF701A descriptor independently identifies atan2.
// Do not supply a replacement CRT implementation here.
extern "C" double __cdecl _CIatan2();

namespace bsp {
namespace {
// Native CE3830/CE3828 are promoted float constants, not full-precision pi.
const double quarter_turn = 1.57079637050628662109375;
const double full_turn = 6.283185482025146484375;

__declspec(naked) float __fastcall heading_kernel(const float*) {
    __asm {
        push ecx
        fld dword ptr [ecx + 4]
        fld dword ptr [ecx]
        call _CIatan2
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fsubr qword ptr [quarter_turn]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fldz
        fcomip st(0), st(1)
        jbe finished
        fadd qword ptr [full_turn]
        fstp dword ptr [esp]
        fld dword ptr [esp]
    finished:
        pop ecx
        ret
    }
}

// The private kernels use EDX for the borrowed point. Native public functions
// instead fetch it from their one stack argument. All floating loads, spills,
// comparisons and stores below retain the original instruction ordering.
__declspec(naked) bool __fastcall contains_kernel(const float*, const float*) {
    __asm {
        push eax
        fld dword ptr [edx]
        fstp dword ptr [esp]
        fld dword ptr [ecx]
        fld dword ptr [esp]
        fcomi st(0), st(1)
        fstp st(1)
        jc rejected_pop
        fld dword ptr [ecx + 8]
        fcomip st(0), st(1)
        fstp st(0)
        jbe rejected
        fld dword ptr [edx + 4]
        fstp dword ptr [esp]
        fld dword ptr [ecx + 4]
        fld dword ptr [esp]
        fcomi st(0), st(1)
        fstp st(1)
        jc rejected_pop
        fld dword ptr [ecx + 12]
        fcomip st(0), st(1)
        fstp st(0)
        jbe rejected
        mov eax, 1
        add esp, 4
        ret
    rejected_pop:
        fstp st(0)
    rejected:
        xor eax, eax
        add esp, 4
        ret
    }
}

__declspec(naked) void __fastcall include_kernel(float*, const float*) {
    __asm {
        push eax
        fld dword ptr [edx]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fld dword ptr [ecx]
        fcomip st(0), st(1)
        fstp st(0)
        jbe maximum_x
        movss xmm0, dword ptr [esp]
        movss dword ptr [ecx], xmm0
    maximum_x:
        fld dword ptr [edx]
        fstp dword ptr [esp]
        fld dword ptr [ecx + 8]
        fld dword ptr [esp]
        fcomip st(0), st(1)
        fstp st(0)
        jbe minimum_y
        movss xmm0, dword ptr [esp]
        movss dword ptr [ecx + 8], xmm0
    minimum_y:
        fld dword ptr [edx + 4]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fld dword ptr [ecx + 4]
        fcomip st(0), st(1)
        fstp st(0)
        jbe maximum_y
        movss xmm0, dword ptr [esp]
        movss dword ptr [ecx + 4], xmm0
    maximum_y:
        fld dword ptr [edx + 4]
        fstp dword ptr [esp]
        fld dword ptr [ecx + 12]
        fld dword ptr [esp]
        fcomip st(0), st(1)
        fstp st(0)
        jbe finished
        movss xmm0, dword ptr [esp]
        movss dword ptr [ecx + 12], xmm0
    finished:
        add esp, 4
        ret
    }
}
} // namespace

float heading_angle_00414eb0(const std::array<float, 2>& direction) {
    return heading_kernel(direction.data());
}
bool contains_point_00414f50(const std::array<float, 4>& bounds,
    const float* point_xy) noexcept {
    return contains_kernel(bounds.data(), point_xy);
}
void include_point_00415010(std::array<float, 4>& bounds,
    const float* point_xy) noexcept {
    include_kernel(bounds.data(), point_xy);
}
} // namespace bsp
