#include "bsp/avoid_zone_clearance_distance.hpp"
#include <limits>

namespace bsp {
namespace {
const float clearance_no_result = std::numeric_limits<float>::max(); // 00D7A248
const float clearance_projection_gate = 1.0f; // 00D7A24C

// Explicit EDX CRT is saved before the original frame. Only the five native
// argument references shift by four bytes; every scratch spill remains at its
// original offset. The source exposes four used stack arguments (RET10h),
// where the original consumes six (RET18h). The input list is borrowed.
__declspec(naked) float __fastcall selected_clearance_kernel(
    const AvoidZoneSelectedSegment* const*, const CameraAxesCrtAccess*,
    const float*, float, const float*, const float*) {
    __asm {
        push edx // saved explicit CRT; original local offsets unchanged
        SUB ESP,0xc4 // 00415d70
        FLD dword ptr [ESP + 0xd0] // 00415d76
        MOVSS XMM0,dword ptr [clearance_no_result] // 00415d7d
        PUSH ESI // 00415d85
        FMUL st(0),st(0) // 00415d86
        MOV ESI,dword ptr [ECX] // 00415d88
        TEST ESI,ESI // 00415d8a
        FSTP dword ptr [ESP + 0x5c] // 00415d8c
        MOVSS dword ptr [ESP + 0x20],XMM0 // 00415d90
        JZ L_0041625d // 00415d96
        MOV EAX,dword ptr [ESP + 0xd8] // 00415d9c
        XORPS XMM1,XMM1 // 00415da3
        PUSH EBX // 00415da6
        MOV EBX,dword ptr [ESP + 0xe0] // 00415da7
        PUSH EDI // 00415dae
        MOV EDI,dword ptr [ESP + 0xd8] // 00415daf
        MOVSS XMM0,dword ptr [EDI + 0x4] // 00415db6
        MOVSS dword ptr [ESP + 0x30],XMM0 // 00415dbb
        MOVSS XMM0,dword ptr [EAX + 0x4] // 00415dc1
        MOVSS dword ptr [ESP + 0x38],XMM0 // 00415dc6
        MOVSS XMM0,dword ptr [EAX] // 00415dcc
        MOVSS dword ptr [ESP + 0x34],XMM0 // 00415dd0
        MOVSS XMM0,dword ptr [EDI] // 00415dd6
        MOVSS dword ptr [ESP + 0x2c],XMM0 // 00415dda
        FLD dword ptr [ESP + 0x2c] // 00415de0
L_00415de4:
        FLD dword ptr [ESP + 0x30] // 00415de4
        MOVSS XMM2,dword ptr [clearance_projection_gate] // 00415de8
        FLD dword ptr [ESI] // 00415df0
        FSTP dword ptr [ESP + 0x18] // 00415df2
        FLD dword ptr [ESI + 0x4] // 00415df6
        FSTP dword ptr [ESP + 0x1c] // 00415df9
        FLD dword ptr [ESI + 0x8] // 00415dfd
        FSTP dword ptr [ESP + 0x20] // 00415e00
        FLD dword ptr [ESI + 0xc] // 00415e04
        FSTP dword ptr [ESP + 0x24] // 00415e07
        FLD dword ptr [ESP + 0x18] // 00415e0b
        FLD st(0) // 00415e0f
        FSUB st(0),st(3) // 00415e11
        FSTP dword ptr [ESP + 0xc0] // 00415e13
        FLD dword ptr [ESP + 0x1c] // 00415e1a
        FLD st(0) // 00415e1e
        FSUB st(0),st(3) // 00415e20
        FSTP dword ptr [ESP + 0xc4] // 00415e22
        FLD dword ptr [ESP + 0x20] // 00415e29
        FLD st(0) // 00415e2d
        FSUB st(0),st(5) // 00415e2f
        FSTP dword ptr [ESP + 0x54] // 00415e31
        FLD dword ptr [ESP + 0x24] // 00415e35
        FLD st(0) // 00415e39
        FSUBRP st(5),st(0) // 00415e3b
        FXCH st(4) // 00415e3d
        FSTP dword ptr [ESP + 0x58] // 00415e3f
        FLD dword ptr [ESP + 0x38] // 00415e43
        FMUL dword ptr [ESP + 0xc4] // 00415e47
        FLD dword ptr [ESP + 0x34] // 00415e4e
        FMUL dword ptr [ESP + 0xc0] // 00415e52
        FADDP st(1),st(0) // 00415e59
        FSTP dword ptr [ESP + 0x14] // 00415e5b
        FLD dword ptr [ESP + 0x38] // 00415e5f
        FMUL dword ptr [ESP + 0x58] // 00415e63
        FLD dword ptr [ESP + 0x34] // 00415e67
        FMUL dword ptr [ESP + 0x54] // 00415e6b
        FADDP st(1),st(0) // 00415e6f
        FSTP dword ptr [ESP + 0x10] // 00415e71
        FLD1 // 00415e75
        MOVSS XMM0,dword ptr [ESP + 0x10] // 00415e77
        FLD dword ptr [ESP + 0x14] // 00415e7d
        FCOMI st(0),st(1) // 00415e81
        FSTP st(1) // 00415e83
        JA L_00415e90 // 00415e85
        COMISS XMM0,XMM2 // 00415e87
        JBE L_0041622d // 00415e8a
L_00415e90:
        COMISS XMM1,dword ptr [ESP + 0x14] // 00415e90
        JBE L_00415f22 // 00415e95
        FLD st(1) // 00415e9b
        FSUBP st(4),st(0) // 00415e9d
        FXCH st(3) // 00415e9f
        FSTP dword ptr [ESP + 0x90] // 00415ea1
        FLD st(3) // 00415ea8
        FSUBP st(2),st(0) // 00415eaa
        FXCH // 00415eac
        FSTP dword ptr [ESP + 0x94] // 00415eae
        FLD dword ptr [ESP + 0x10] // 00415eb5
        FLD st(0) // 00415eb9
        FSUBRP st(3),st(0) // 00415ebb
        FDIVRP st(2),st(0) // 00415ebd
        FXCH // 00415ebf
        FSTP dword ptr [ESP + 0xc] // 00415ec1
        FLD dword ptr [ESP + 0x90] // 00415ec5
        FLD dword ptr [ESP + 0xc] // 00415ecc
        FLD st(0) // 00415ed0
        FMULP st(2),st(0) // 00415ed2
        FXCH // 00415ed4
        FSTP dword ptr [ESP + 0x3c] // 00415ed6
        FMUL dword ptr [ESP + 0x94] // 00415eda
        FSTP dword ptr [ESP + 0x40] // 00415ee1
        FLD dword ptr [ESP + 0x3c] // 00415ee5
        FADD st(0),st(1) // 00415ee9
        FSTP dword ptr [ESP + 0xb0] // 00415eeb
        FLD dword ptr [ESP + 0x40] // 00415ef2
        FADD st(0),st(2) // 00415ef6
        FSTP dword ptr [ESP + 0xb4] // 00415ef8
        FLD dword ptr [ESP + 0xb0] // 00415eff
        FSTP dword ptr [ESP + 0x18] // 00415f06
        FLD dword ptr [ESP + 0xb4] // 00415f0a
        FSTP dword ptr [ESP + 0x1c] // 00415f11
        FLD dword ptr [ESP + 0x1c] // 00415f15
        FLD dword ptr [ESP + 0x18] // 00415f19
        JMP L_00415fb3 // 00415f1d
L_00415f22:
        COMISS XMM1,XMM0 // 00415f22
        JBE L_004160a8 // 00415f25
        FLD st(3) // 00415f2b
        FSUBP st(2),st(0) // 00415f2d
        FXCH // 00415f2f
        FSTP dword ptr [ESP + 0x70] // 00415f31
        FLD st(1) // 00415f35
        FSUBP st(4),st(0) // 00415f37
        FXCH st(3) // 00415f39
        FSTP dword ptr [ESP + 0x74] // 00415f3b
        FLD dword ptr [ESP + 0x10] // 00415f3f
        FSUBR st(0),st(3) // 00415f43
        FDIVP st(3),st(0) // 00415f45
        FXCH st(2) // 00415f47
        FSTP dword ptr [ESP + 0xc] // 00415f49
        FLD dword ptr [ESP + 0x70] // 00415f4d
        FLD dword ptr [ESP + 0xc] // 00415f51
        FLD st(0) // 00415f55
        FMULP st(2),st(0) // 00415f57
        FXCH // 00415f59
        FSTP dword ptr [ESP + 0xa0] // 00415f5b
        FMUL dword ptr [ESP + 0x74] // 00415f62
        FSTP dword ptr [ESP + 0xa4] // 00415f66
        FLD dword ptr [ESP + 0xa0] // 00415f6d
        FADD st(0),st(1) // 00415f74
        FSTP dword ptr [ESP + 0x80] // 00415f76
        FLD dword ptr [ESP + 0xa4] // 00415f7d
        FADD st(0),st(2) // 00415f84
        FSTP dword ptr [ESP + 0x84] // 00415f86
        FLD dword ptr [ESP + 0x80] // 00415f8d
        FSTP dword ptr [ESP + 0x20] // 00415f94
        FLD dword ptr [ESP + 0x84] // 00415f98
        FSTP dword ptr [ESP + 0x24] // 00415f9f
        FLD dword ptr [ESP + 0x24] // 00415fa3
        FLD dword ptr [ESP + 0x20] // 00415fa7
        FXCH // 00415fab
        FXCH st(3) // 00415fad
        FXCH // 00415faf
L_00415fb1:
        FXCH st(2) // 00415fb1
L_00415fb3:
        FLD st(0) // 00415fb3
        FSUB st(0),st(5) // 00415fb5
        FSTP dword ptr [ESP + 0x44] // 00415fb7
        FLD dword ptr [EDI + 0x4] // 00415fbb
        FSTP dword ptr [ESP + 0xc] // 00415fbe
        FLD st(1) // 00415fc2
        FLD dword ptr [ESP + 0xc] // 00415fc4
        FLD st(0) // 00415fc8
        FSUBP st(2),st(0) // 00415fca
        FXCH // 00415fcc
        FSTP dword ptr [ESP + 0x48] // 00415fce
        FLD st(3) // 00415fd2
        FSUB st(0),st(6) // 00415fd4
        FSTP dword ptr [ESP + 0x4c] // 00415fd6
        FSUBR st(0),st(4) // 00415fda
        FSTP dword ptr [ESP + 0x50] // 00415fdc
        FLD dword ptr [EBX + 0x4] // 00415fe0
        FSTP dword ptr [ESP + 0x10] // 00415fe3
        FLD dword ptr [EBX] // 00415fe7
        FSTP dword ptr [ESP + 0xc] // 00415fe9
        FLD dword ptr [ESP + 0x10] // 00415fed
        FMUL dword ptr [ESP + 0x48] // 00415ff1
        FLD dword ptr [ESP + 0xc] // 00415ff5
        FMUL dword ptr [ESP + 0x44] // 00415ff9
        FADDP st(1),st(0) // 00415ffd
        FSTP dword ptr [ESP + 0x14] // 00415fff
        FLD dword ptr [ESP + 0x10] // 00416003
        FMUL dword ptr [ESP + 0x50] // 00416007
        FLD dword ptr [ESP + 0xc] // 0041600b
        FMUL dword ptr [ESP + 0x4c] // 0041600f
        FADDP st(1),st(0) // 00416013
        FSTP dword ptr [ESP + 0x10] // 00416015
        FLD1 // 00416019
        MOVSS XMM0,dword ptr [ESP + 0x10] // 0041601b
        FLD dword ptr [ESP + 0x14] // 00416021
        FCOMI st(0),st(1) // 00416025
        FSTP st(1) // 00416027
        JA L_00416034 // 00416029
        COMISS XMM0,XMM2 // 0041602b
        JBE L_00416233 // 0041602e
L_00416034:
        COMISS XMM1,dword ptr [ESP + 0x14] // 00416034
        JBE L_004160af // 00416039
        FLD st(3) // 0041603b
        FSUBP st(2),st(0) // 0041603d
        FXCH // 0041603f
        FSTP dword ptr [ESP + 0x5c] // 00416041
        FLD st(3) // 00416045
        FSUBP st(2),st(0) // 00416047
        FXCH // 00416049
        FSTP dword ptr [ESP + 0x60] // 0041604b
        FLD dword ptr [ESP + 0x10] // 0041604f
        FLD st(0) // 00416053
        FSUBRP st(2),st(0) // 00416055
        FDIVRP st(1),st(0) // 00416057
        FSTP dword ptr [ESP + 0xc] // 00416059
        FLD dword ptr [ESP + 0x5c] // 0041605d
        FLD dword ptr [ESP + 0xc] // 00416061
        FLD st(0) // 00416065
        FMULP st(2),st(0) // 00416067
        FXCH // 00416069
        FSTP dword ptr [ESP + 0x68] // 0041606b
        FMUL dword ptr [ESP + 0x60] // 0041606f
        FSTP dword ptr [ESP + 0x6c] // 00416073
        FLD dword ptr [ESP + 0x68] // 00416077
        FADD st(0),st(1) // 0041607b
        FSTP dword ptr [ESP + 0x78] // 0041607d
        FLD dword ptr [ESP + 0x6c] // 00416081
        FADD st(0),st(2) // 00416085
        FSTP dword ptr [ESP + 0x7c] // 00416087
        FLD dword ptr [ESP + 0x78] // 0041608b
        FSTP dword ptr [ESP + 0x18] // 0041608f
        FLD dword ptr [ESP + 0x7c] // 00416093
        FSTP dword ptr [ESP + 0x1c] // 00416097
        FLD dword ptr [ESP + 0x1c] // 0041609b
        FLD dword ptr [ESP + 0x18] // 0041609f
        JMP L_0041614e // 004160a3
L_004160a8:
        FSTP st(0) // 004160a8
        JMP L_00415fb1 // 004160aa
L_004160af:
        COMISS XMM1,XMM0 // 004160af
        JBE L_004161d1 // 004160b2
        FLD st(1) // 004160b8
        FSUBP st(4),st(0) // 004160ba
        FXCH st(3) // 004160bc
        FSTP dword ptr [ESP + 0x88] // 004160be
        FLD st(1) // 004160c5
        FSUBP st(4),st(0) // 004160c7
        FXCH st(3) // 004160c9
        FSTP dword ptr [ESP + 0x8c] // 004160cb
        FLD dword ptr [ESP + 0x10] // 004160d2
        FSUBR st(0),st(2) // 004160d6
        FDIVP st(2),st(0) // 004160d8
        FXCH // 004160da
        FSTP dword ptr [ESP + 0xc] // 004160dc
        FLD dword ptr [ESP + 0x88] // 004160e0
        FLD dword ptr [ESP + 0xc] // 004160e7
        FLD st(0) // 004160eb
        FMULP st(2),st(0) // 004160ed
        FXCH // 004160ef
        FSTP dword ptr [ESP + 0x98] // 004160f1
        FMUL dword ptr [ESP + 0x8c] // 004160f8
        FSTP dword ptr [ESP + 0x9c] // 004160ff
        FLD dword ptr [ESP + 0x98] // 00416106
        FADD st(0),st(2) // 0041610d
        FSTP dword ptr [ESP + 0xa8] // 0041610f
        FLD dword ptr [ESP + 0x9c] // 00416116
        FADD st(0),st(1) // 0041611d
        FSTP dword ptr [ESP + 0xac] // 0041611f
        FLD dword ptr [ESP + 0xa8] // 00416126
        FSTP dword ptr [ESP + 0x20] // 0041612d
        FLD dword ptr [ESP + 0xac] // 00416131
        FSTP dword ptr [ESP + 0x24] // 00416138
        FLD dword ptr [ESP + 0x24] // 0041613c
        FLD dword ptr [ESP + 0x20] // 00416140
        FXCH // 00416144
        FXCH st(3) // 00416146
        FXCH // 00416148
        FXCH st(2) // 0041614a
        FXCH // 0041614c
L_0041614e:
        FSUB st(0),st(4) // 0041614e
        FSTP dword ptr [ESP + 0xb8] // 00416150
        FLD dword ptr [EDI + 0x4] // 00416157
        FSTP dword ptr [ESP + 0xc] // 0041615a
        FLD dword ptr [ESP + 0xc] // 0041615e
        FLD st(0) // 00416162
        FSUBP st(2),st(0) // 00416164
        FXCH // 00416166
        FSTP dword ptr [ESP + 0xbc] // 00416168
        FLD st(3) // 0041616f
        FSUBP st(2),st(0) // 00416171
        FXCH // 00416173
        FSTP dword ptr [ESP + 0xc8] // 00416175
        FSUBP st(1),st(0) // 0041617c
        FSTP dword ptr [ESP + 0xcc] // 0041617e
        FLD dword ptr [ESP + 0xbc] // 00416185
        FLD dword ptr [ESP + 0xb8] // 0041618c
        FMUL st(0),st(0) // 00416193
        FLD st(1) // 00416195
        FMULP st(2),st(0) // 00416197
        FADDP st(1),st(0) // 00416199
        FSTP dword ptr [ESP + 0x14] // 0041619b
        FLD dword ptr [ESP + 0xcc] // 0041619f
        FLD dword ptr [ESP + 0xc8] // 004161a6
        FMUL st(0),st(0) // 004161ad
        FLD st(1) // 004161af
        FMULP st(2),st(0) // 004161b1
        FADDP st(1),st(0) // 004161b3
        FSTP dword ptr [ESP + 0x10] // 004161b5
        FLD dword ptr [ESP + 0x64] // 004161b9
        FLD dword ptr [ESP + 0x14] // 004161bd
        FCOMI st(0),st(1) // 004161c1
        FLD dword ptr [ESP + 0x10] // 004161c3
        JA L_004161d8 // 004161c7
        FCOMI st(0),st(2) // 004161c9
        FSTP st(2) // 004161cb
        JBE L_00416239 // 004161cd
        JMP L_004161da // 004161cf
L_004161d1:
        FSTP st(0) // 004161d1
        JMP L_0041614e // 004161d3
L_004161d8:
        FSTP st(2) // 004161d8
L_004161da:
        FSTP st(2) // 004161da
        FCOMIP st(0),st(1) // 004161dc
        JA L_004161f2 // 004161de
        MOVSS XMM0,dword ptr [ESP + 0x10] // 004161e0
        FSTP st(0) // 004161e6
        MOVSS dword ptr [ESP + 0x14],XMM0 // 004161e8
        FLD dword ptr [ESP + 0x14] // 004161ee
L_004161f2:
        mov ecx, dword ptr [esp + 0xd0] // ST0 operand remains unspilled
        CALL native_crt_sqrt_st0_00bf7030 // 004161f2
        FSTP dword ptr [ESP + 0xc] // 004161f7
        FLD dword ptr [ESP + 0xc] // 004161fb
        XORPS XMM1,XMM1 // 004161ff
        FSUB dword ptr [ESP + 0xdc] // 00416202
        FSTP dword ptr [ESP + 0x14] // 00416209
        FLD dword ptr [ESP + 0x14] // 0041620d
        FLD dword ptr [ESP + 0x28] // 00416211
        FCOMIP st(0),st(1) // 00416215
        FSTP st(0) // 00416217
        FLD dword ptr [ESP + 0x2c] // 00416219
        JBE L_0041623d // 0041621d
        MOVSS XMM0,dword ptr [ESP + 0x14] // 0041621f
        MOVSS dword ptr [ESP + 0x28],XMM0 // 00416225
        JMP L_0041623d // 0041622b
L_0041622d:
        FSTP st(4) // 0041622d
        FSTP st(0) // 0041622f
        JMP L_00416237 // 00416231
L_00416233:
        FSTP st(4) // 00416233
        FSTP st(2) // 00416235
L_00416237:
        FSTP st(0) // 00416237
L_00416239:
        FSTP st(0) // 00416239
        FSTP st(0) // 0041623b
L_0041623d:
        CMP byte ptr [ESI + 0x1c],0x0 // 0041623d
        JNZ L_0041624e // 00416241
        MOV EAX,dword ptr [ESI + 0x10] // 00416243
        TEST EAX,EAX // 00416246
        JZ L_0041624e // 00416248
        MOV ESI,EAX // 0041624a
        JMP L_00416251 // 0041624c
L_0041624e:
        MOV ESI,dword ptr [ESI + 0x18] // 0041624e
L_00416251:
        TEST ESI,ESI // 00416251
        JNZ L_00415de4 // 00416253
        POP EDI // 00416259
        FSTP st(0) // 0041625a
        POP EBX // 0041625c
L_0041625d:
        FLD dword ptr [ESP + 0x20] // 0041625d
        POP ESI // 00416261
        ADD ESP,0xc4 // 00416262
        pop edx // release the saved CRT binding
        RET 0x10 // omit the two unread native stack arguments // 00416268
    }
}
} // namespace

float avoid_zone_selected_segments_clearance_00415d70(
    const AvoidZoneSelectedSegment* head,
    const std::array<float, 2>& center, float radius,
    const std::array<float, 2>& normal_a,
    const std::array<float, 2>& normal_b, const CameraAxesCrtAccess& crt) {
    return selected_clearance_kernel(&head, &crt, center.data(), radius,
        normal_a.data(), normal_b.data());
}
} // namespace bsp
