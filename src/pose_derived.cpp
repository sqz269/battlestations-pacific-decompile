#include "bsp/pose_derived.hpp"

namespace bsp {
namespace {
const std::uint32_t affine_one_bits = 0x3f800000u; // native D7A24C, verified bytes
// Source-assembled original register/stack schedule, not executable image bytes.
// Preserve every float spill and source reread after destination stores.
__declspec(naked) void __fastcall derive_kernel(float*, const float*) {
    __asm {
        sub esp,0x18 // 00b63d50
        fld dword ptr [edx + 0x4] // 00b63d53
        movss xmm0,dword ptr [edx] // 00b63d56
        fstp dword ptr [esp] // 00b63d5a
        movss dword ptr [esp + 0x4],xmm0 // 00b63d5d
        fld dword ptr [edx + 0x8] // 00b63d63
        movss dword ptr [esp + 0xc],xmm0 // 00b63d66
        fstp dword ptr [esp + 0x8] // 00b63d6c
        fld dword ptr [esp] // 00b63d70
        fld dword ptr [esp + 0x4] // 00b63d73
        fld dword ptr [esp + 0x8] // 00b63d77
        fld st(1) // 00b63d7b
        fmulp st(2),st(0) // 00b63d7d
        fxch st(1) // 00b63d7f
        fstp dword ptr [esp + 0x8] // 00b63d81
        fld dword ptr [esp + 0x8] // 00b63d85
        fld st(2) // 00b63d89
        fmulp st(3),st(0) // 00b63d8b
        fxch st(2) // 00b63d8d
        fstp dword ptr [esp + 0x8] // 00b63d8f
        fld dword ptr [esp + 0x8] // 00b63d93
        faddp st(2),st(0) // 00b63d97
        fmul st(0),st(0) // 00b63d99
        fstp dword ptr [esp + 0x8] // 00b63d9b
        fadd dword ptr [esp + 0x8] // 00b63d9f
        fstp dword ptr [esp + 0x10] // 00b63da3
        fld dword ptr [edx + 0x10] // 00b63da7
        fstp dword ptr [esp + 0x4] // 00b63daa
        fld dword ptr [edx + 0x14] // 00b63dae
        fstp dword ptr [esp + 0x8] // 00b63db1
        fld dword ptr [edx + 0x18] // 00b63db5
        fstp dword ptr [esp] // 00b63db8
        fld dword ptr [esp + 0x8] // 00b63dbb
        fld dword ptr [esp + 0x4] // 00b63dbf
        fld dword ptr [esp] // 00b63dc3
        fld st(1) // 00b63dc6
        fmulp st(2),st(0) // 00b63dc8
        fxch st(1) // 00b63dca
        fstp dword ptr [esp + 0x8] // 00b63dcc
        fld dword ptr [esp + 0x8] // 00b63dd0
        fld st(2) // 00b63dd4
        fmulp st(3),st(0) // 00b63dd6
        fxch st(2) // 00b63dd8
        fstp dword ptr [esp + 0x8] // 00b63dda
        fld dword ptr [esp + 0x8] // 00b63dde
        faddp st(2),st(0) // 00b63de2
        fmul st(0),st(0) // 00b63de4
        fstp dword ptr [esp + 0x8] // 00b63de6
        fadd dword ptr [esp + 0x8] // 00b63dea
        fstp dword ptr [esp + 0x14] // 00b63dee
        fld dword ptr [edx + 0x20] // 00b63df2
        fstp dword ptr [esp + 0x4] // 00b63df5
        fld dword ptr [edx + 0x24] // 00b63df9
        fstp dword ptr [esp + 0x8] // 00b63dfc
        fld dword ptr [edx + 0x28] // 00b63e00
        fstp dword ptr [esp] // 00b63e03
        fld dword ptr [esp + 0x8] // 00b63e06
        fld dword ptr [esp + 0x4] // 00b63e0a
        fld dword ptr [esp] // 00b63e0e
        fld st(1) // 00b63e11
        fmulp st(2),st(0) // 00b63e13
        fxch st(1) // 00b63e15
        fstp dword ptr [esp + 0x8] // 00b63e17
        fld dword ptr [esp + 0x8] // 00b63e1b
        fld st(2) // 00b63e1f
        fmulp st(3),st(0) // 00b63e21
        fxch st(2) // 00b63e23
        fstp dword ptr [esp + 0x8] // 00b63e25
        fld dword ptr [esp + 0x8] // 00b63e29
        faddp st(2),st(0) // 00b63e2d
        fmul st(0),st(0) // 00b63e2f
        fstp dword ptr [esp + 0x8] // 00b63e31
        fadd dword ptr [esp + 0x8] // 00b63e35
        fstp dword ptr [esp + 0x8] // 00b63e39
        fld dword ptr [esp + 0xc] // 00b63e3d
        fld dword ptr [esp + 0x10] // 00b63e41
        fld st(0) // 00b63e45
        fdivp st(2),st(0) // 00b63e47
        fxch st(1) // 00b63e49
        fstp dword ptr [ecx] // 00b63e4b
        fld dword ptr [edx + 0x4] // 00b63e4d
        fdiv st(0),st(1) // 00b63e50
        fstp dword ptr [ecx + 0x10] // 00b63e52
        xorps xmm0,xmm0 // 00b63e55
        fdivr dword ptr [edx + 0x8] // 00b63e58
        fstp dword ptr [ecx + 0x20] // 00b63e5b
        fld dword ptr [edx + 0x10] // 00b63e5e
        fld dword ptr [esp + 0x14] // 00b63e61
        fld st(0) // 00b63e65
        fdivp st(2),st(0) // 00b63e67
        fxch st(1) // 00b63e69
        fstp dword ptr [ecx + 0x4] // 00b63e6b
        fld dword ptr [edx + 0x14] // 00b63e6e
        fdiv st(0),st(1) // 00b63e71
        fstp dword ptr [ecx + 0x14] // 00b63e73
        fdivr dword ptr [edx + 0x18] // 00b63e76
        fstp dword ptr [ecx + 0x24] // 00b63e79
        fld dword ptr [edx + 0x20] // 00b63e7c
        fld dword ptr [esp + 0x8] // 00b63e7f
        fld st(0) // 00b63e83
        fdivp st(2),st(0) // 00b63e85
        fxch st(1) // 00b63e87
        fstp dword ptr [ecx + 0x8] // 00b63e89
        fld dword ptr [edx + 0x24] // 00b63e8c
        fdiv st(0),st(1) // 00b63e8f
        fstp dword ptr [ecx + 0x18] // 00b63e91
        fdivr dword ptr [edx + 0x28] // 00b63e94
        fstp dword ptr [ecx + 0x28] // 00b63e97
        fld dword ptr [ecx] // 00b63e9a
        fchs // 00b63e9c
        fmul dword ptr [edx + 0x30] // 00b63e9e
        fld dword ptr [edx + 0x34] // 00b63ea1
        fmul dword ptr [ecx + 0x10] // 00b63ea4
        fsubp st(1),st(0) // 00b63ea7
        fld dword ptr [edx + 0x38] // 00b63ea9
        fmul dword ptr [ecx + 0x20] // 00b63eac
        fsubp st(1),st(0) // 00b63eaf
        fstp dword ptr [ecx + 0x30] // 00b63eb1
        fld dword ptr [ecx + 0x4] // 00b63eb4
        fchs // 00b63eb7
        fmul dword ptr [edx + 0x30] // 00b63eb9
        fld dword ptr [edx + 0x34] // 00b63ebc
        fmul dword ptr [ecx + 0x14] // 00b63ebf
        fsubp st(1),st(0) // 00b63ec2
        fld dword ptr [edx + 0x38] // 00b63ec4
        fmul dword ptr [ecx + 0x24] // 00b63ec7
        fsubp st(1),st(0) // 00b63eca
        fstp dword ptr [ecx + 0x34] // 00b63ecc
        fld dword ptr [ecx + 0x8] // 00b63ecf
        fchs // 00b63ed2
        fmul dword ptr [edx + 0x30] // 00b63ed4
        fld dword ptr [edx + 0x34] // 00b63ed7
        fmul dword ptr [ecx + 0x18] // 00b63eda
        fsubp st(1),st(0) // 00b63edd
        fld dword ptr [edx + 0x38] // 00b63edf
        movss dword ptr [ecx + 0xc],xmm0 // 00b63ee2
        fmul dword ptr [ecx + 0x28] // 00b63ee7
        movss dword ptr [ecx + 0x1c],xmm0 // 00b63eea
        movss dword ptr [ecx + 0x2c],xmm0 // 00b63eef
        movss xmm0,dword ptr [affine_one_bits] // 00b63ef4
        fsubp st(1),st(0) // 00b63efc
        movss dword ptr [ecx + 0x3c],xmm0 // 00b63efe
        fstp dword ptr [ecx + 0x38] // 00b63f03
        add esp,0x18 // 00b63f06
        ret // 00b63f09
    }
}
}

void derive_pose_affine_inverse_00b63d50(CameraMatrix& destination, const CameraMatrix& source) {
    derive_kernel(destination.data(), source.data());
}

const CameraMatrix& get_pose_derived_00414e10(PoseDerivedView& view) {
    if (view.pose.derived_valid_10c == 0) {
        refresh_pose_00414db0(view.pose);
        view.pose.derived_valid_10c = 1; // native store precedes the leaf call
        derive_pose_affine_inverse_00b63d50(view.derived_110, view.pose.world_cc);
    }
    return view.derived_110;
}

} // namespace bsp
