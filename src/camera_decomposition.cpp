#include "bsp/camera_decomposition.hpp"
#include "bsp/camera_position_modes.hpp"

extern "C" double __cdecl _CIsqrt();
extern "C" double __cdecl _CIatan2();

namespace bsp {
namespace {
// Read-only image doubles CE3820, CE3C70 and D7A268 respectively. The last
// is promoted binary32 1e-4, not the nearest binary64 representation of 1e-4.
const double squared_length_cutoff = 1e-10;
const double minimum_length = 1e-5;
const double cosine_cutoff = 9.99999974737875163555145263671875e-5;
}

__declspec(naked) void __fastcall normalize_camera_basis_0042b260(
    std::array<float, 3>&) {
    __asm {
        sub esp, 0x10
        push esi
        mov esi, ecx
        fld dword ptr [esi + 4]
        fstp dword ptr [esp + 8]
        fld dword ptr [esp + 8]
        fld dword ptr [esi]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fld dword ptr [esi + 8]
        fstp dword ptr [esp + 0xc]
        fld dword ptr [esp + 0xc]
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp + 0x10]
        fld qword ptr [squared_length_cutoff]
        fld dword ptr [esp + 0x10]
        fcomi st(0), st(1)
        fstp st(1)
        jbe minimum_divisor
        call _CIsqrt
        fstp dword ptr [esp + 0x10]
        fld dword ptr [esp + 0x10]
        jmp store_divisor
    minimum_divisor:
        fstp st(0)
        fld qword ptr [minimum_length]
    store_divisor:
        fstp dword ptr [esp + 0x10]
        fld dword ptr [esp + 4]
        fld dword ptr [esp + 0x10]
        fld st(0)
        fdivp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esi]
        fld dword ptr [esp + 8]
        fdiv st(0), st(1)
        fstp dword ptr [esi + 4]
        fdivr dword ptr [esp + 0xc]
        fstp dword ptr [esi + 8]
        pop esi
        add esp, 0x10
        ret
    }
}

__declspec(naked) void __fastcall extract_camera_matrix_angles_0042d2e0(
    const CameraMatrix&, float&, float&, float&) {
    __asm {
        sub esp, 0x2c
        // Row2.Y is captured through x87; the other eight words use MOVSS.
        movss xmm0, dword ptr [ecx]
        fld dword ptr [ecx + 0x24]
        movss dword ptr [esp + 0x20], xmm0
        fstp dword ptr [esp + 0xc]
        movss xmm0, dword ptr [ecx + 4]
        movss dword ptr [esp + 0x24], xmm0
        movss xmm0, dword ptr [ecx + 8]
        movss dword ptr [esp + 0x28], xmm0
        movss xmm0, dword ptr [ecx + 0x10]
        movss dword ptr [esp + 0x14], xmm0
        movss xmm0, dword ptr [ecx + 0x14]
        movss dword ptr [esp + 0x18], xmm0
        movss xmm0, dword ptr [ecx + 0x18]
        movss dword ptr [esp + 0x1c], xmm0
        movss xmm0, dword ptr [ecx + 0x20]
        movss dword ptr [esp + 8], xmm0
        movss xmm0, dword ptr [ecx + 0x28]
        push esi
        lea ecx, [esp + 0x24]
        mov esi, edx
        movss dword ptr [esp + 0x14], xmm0
        call normalize_camera_basis_0042b260
        lea ecx, [esp + 0x18]
        call normalize_camera_basis_0042b260
        lea ecx, [esp + 0xc]
        call normalize_camera_basis_0042b260
        fld dword ptr [esp + 0x10]
        push ecx
        fstp dword ptr [esp]
        call camera_asin_clamped_0042cf10
        fchs
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fst dword ptr [esi]
        fstp dword ptr [esp + 8]
        fld dword ptr [esp + 8]
        fcos
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        pop esi
        fld st(0)
        fabs
        fstp dword ptr [esp + 4]
        fld qword ptr [cosine_cutoff]
        fld dword ptr [esp + 4]
        fcomip st(0), st(1)
        fstp st(0)
        jbe degenerate
        // CRT ST1=normalized row0.Y/cos, ST0=row1.Y/cos; both float-spilled.
        fld dword ptr [esp + 0x24]
        fdiv st(0), st(1)
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fld dword ptr [esp + 0x18]
        fdivrp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        call _CIatan2
        fstp dword ptr [esp + 4]
        mov eax, dword ptr [esp + 0x34]
        fld dword ptr [esp + 4]
        fstp dword ptr [eax]
        // Z's pointer slot becomes scratch only after its ordered output write.
        fld dword ptr [esp + 8]
        fld dword ptr [esp]
        fld st(0)
        fdivp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 0x34]
        fld dword ptr [esp + 0x34]
        fld dword ptr [esp + 0x10]
        fdivrp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 0x34]
        fld dword ptr [esp + 0x34]
        call _CIatan2
        fstp dword ptr [esp + 0x34]
        fld dword ptr [esp + 0x34]
        mov ecx, dword ptr [esp + 0x30]
        fstp dword ptr [ecx]
        add esp, 0x2c
        ret 8
    degenerate:
        mov edx, dword ptr [esp + 0x34]
        fstp st(0)
        fld dword ptr [esp + 0x14]
        xorps xmm0, xmm0
        fld dword ptr [esp + 0x20]
        movss dword ptr [edx], xmm0
        call _CIatan2
        fstp dword ptr [esp + 0x34]
        fld dword ptr [esp + 0x34]
        mov eax, dword ptr [esp + 0x30]
        fstp dword ptr [eax]
        add esp, 0x2c
        ret 8
    }
}

} // namespace bsp
