// Complete geometry schedules; names are hypotheses, new explicit interfaces.
// Evidence, original ABI, call sites and limits: docs/SHIP_AI_NAV_CIRCLE_TANGENT.md.
#include "bsp/ship_ai_nav_circle_tangent.hpp"
#include "bsp/native_vector2_math.hpp"
#include "bsp/ship_ai_path_follower.hpp"
#include <cstddef>

namespace bsp {
namespace {
const float circle_negative_zero = -0.0f; // 00D7A208: 00 00 00 80
static_assert(sizeof(ShipAiAttackMoveXZ) == 8);
static_assert(offsetof(ShipAiAttackMoveXZ, z) == 4);
}

__declspec(naked) bool __stdcall circle_relative_equal_004f3560(float, float) noexcept {
    __asm {
        SUB ESP,0x8 // 004f3560
        FLD dword ptr [ESP + 0xc] // 004f3563
        MOVSS XMM0,dword ptr [ESP + 0x10] // 004f3567
        FSUB dword ptr [ESP + 0x10] // 004f356d
        XORPS XMM4,XMM4 // 004f3571
        COMISS XMM0,XMM4 // 004f3574
        MOVSS XMM3,dword ptr [circle_negative_zero] // 004f3577
        FSTP dword ptr [ESP + 0x4] // 004f357f
        JBE L_004f358a // 004f3583
        MOVAPS XMM1,XMM0 // 004f3585
        JMP L_004f3591 // 004f3588
L_004f358a:
        MOVAPS XMM1,XMM3 // 004f358a
        SUBSS XMM1,XMM0 // 004f358d
L_004f3591:
        MOVSS XMM2,dword ptr [ESP + 0xc] // 004f3591
        COMISS XMM2,XMM4 // 004f3597
        MOVSS dword ptr [ESP],XMM1 // 004f359a
        JBE L_004f35a6 // 004f359f
        MOVAPS XMM0,XMM2 // 004f35a1
        JMP L_004f35ad // 004f35a4
L_004f35a6:
        MOVAPS XMM0,XMM3 // 004f35a6
        SUBSS XMM0,XMM2 // 004f35a9
L_004f35ad:
        FLD dword ptr [ESP] // 004f35ad
        MOVSS dword ptr [ESP + 0x10],XMM0 // 004f35b0
        FLD dword ptr [ESP + 0x10] // 004f35b6
        FCOMIP st(0),st(1) // 004f35ba
        FSTP st(0) // 004f35bc
        JA L_004f35c3 // 004f35be
        MOVAPS XMM0,XMM1 // 004f35c0
L_004f35c3:
        MOVSS XMM2,dword ptr [ESP + 0x4] // 004f35c3
        COMISS XMM2,XMM4 // 004f35c9
        MOVSS dword ptr [ESP + 0x10],XMM0 // 004f35cc
        JBE L_004f35dc // 004f35d2
        MOVSS dword ptr [ESP + 0xc],XMM2 // 004f35d4
        JMP L_004f35e6 // 004f35da
L_004f35dc:
        SUBSS XMM3,XMM2 // 004f35dc
        MOVSS dword ptr [ESP + 0xc],XMM3 // 004f35e0
L_004f35e6:
        FLD dword ptr [ESP + 0x10] // 004f35e6
        FLD1 // 004f35ea
        FCOMIP st(0),st(1) // 004f35ec
        FSTP st(0) // 004f35ee
        JBE L_004f35fa // 004f35f0
        MOVSS XMM0,dword ptr [kApproachTangentMinSeparation] // 004f35f2
L_004f35fa:
        FLD dword ptr [ESP + 0xc] // 004f35fa
        MOVSS dword ptr [ESP + 0x10],XMM0 // 004f35fe
        FLD dword ptr [ESP + 0x10] // 004f3604
        FMUL dword ptr [kShipAiCircleRelativeTolerance] // 004f3608
        FCOMIP st(0),st(1) // 004f360e
        FSTP st(0) // 004f3610
        JBE L_004f361f // 004f3612
        MOV EAX,0x1 // 004f3614
        ADD ESP,0x8 // 004f3619
        RET 0x8 // 004f361c
L_004f361f:
        XOR EAX,EAX // 004f361f
        ADD ESP,0x8 // 004f3621
        RET 0x8 // 004f3624
    }
}

namespace {
// Original x87/SSE schedule. Only stack argument offsets change to save EBX
// for the added CRT binding; original locals, comparisons and stores remain.
__declspec(naked) int __fastcall circle_intersection_kernel(
    const float*, const float*, float, float, float*, float*, const CameraAxesCrtAccess*) {
    __asm {
        PUSH EBX
        MOV EBX,dword ptr [ESP + 0x18] // additional last stack argument: borrowed CRT
        SUB ESP,0x28 // 004f4520
        FLD dword ptr [ECX] // 004f4523
        FSTP dword ptr [ESP + 0x8] // 004f4525
        FLD dword ptr [EDX] // 004f4529
        FSUB dword ptr [ESP + 0x8] // 004f452b
        FSTP dword ptr [ESP + 0x18] // 004f452f
        FLD dword ptr [ECX + 0x4] // 004f4533
        FSTP dword ptr [ESP + 0x10] // 004f4536
        FLD dword ptr [EDX + 0x4] // 004f453a
        FSUB dword ptr [ESP + 0x10] // 004f453d
        FSTP dword ptr [ESP + 0x1c] // 004f4541
        FLD dword ptr [ESP + 0x1c] // 004f4545
        FLD dword ptr [ESP + 0x18] // 004f4549
        FMUL st(0),st(0) // 004f454d
        FLD st(1) // 004f454f
        FMULP st(2),st(0) // 004f4551
        FADDP st(1),st(0) // 004f4553
        FSTP dword ptr [ESP] // 004f4555
        FLD qword ptr [kApproachTangentEpsilonSq] // 004f4558
        FLD dword ptr [ESP] // 004f455e
        FCOMI st(0),st(1) // 004f4561
        FSTP st(1) // 004f4563
        JBE L_004f4577 // 004f4565
        MOV ECX,EBX // borrowed CRT, no added FP operation
        CALL native_crt_sqrt_st0_00bf7030 // 004f4567
        FSTP dword ptr [ESP] // 004f456c
        FLD dword ptr [ESP] // 004f456f
        FSTP dword ptr [ESP] // 004f4572
        JMP L_004f4581 // 004f4575
L_004f4577:
        XORPS XMM0,XMM0 // 004f4577
        FSTP st(0) // 004f457a
        MOVSS dword ptr [ESP],XMM0 // 004f457c
L_004f4581:
        FLD dword ptr [ESP + 0x30] // 004f4581
        FLD st(0) // 004f4585
        FLD dword ptr [ESP + 0x34] // 004f4587
        FLD st(0) // 004f458b
        FADDP st(2),st(0) // 004f458d
        FXCH // 004f458f
        FST qword ptr [ESP + 0x20] // 004f4591
        FLD dword ptr [ESP] // 004f4595
        FCOMI st(0),st(1) // 004f4598
        FSTP st(1) // 004f459a
        JBE L_004f45ac // 004f459c
        FSTP st(1) // 004f459e
        XOR EAX,EAX // 004f45a0
        FSTP st(1) // 004f45a2
        FSTP st(0) // 004f45a4
        ADD ESP,0x28 // 004f45a6
        POP EBX // restore additional CRT holder
        RET 0x14 // original four arguments plus borrowed CRT // 004f45a9
L_004f45ac:
        FLD dword ptr [kShipAiCircleRelativeTolerance] // 004f45ac
        FCOMIP st(0),st(1) // 004f45b2
        JBE L_004f45e5 // 004f45b4
        SUB ESP,0x8 // 004f45b6
        FSTP st(0) // 004f45b9
        FSTP dword ptr [ESP + 0x4] // 004f45bb
        FSTP dword ptr [ESP] // 004f45bf
        CALL circle_relative_equal_004f3560 // 004f45c2
        TEST AL,AL // 004f45c7
        JZ L_004f45d6 // 004f45c9
        MOV EAX,0xfffffffe // 004f45cb
        ADD ESP,0x28 // 004f45d0
        POP EBX // restore additional CRT holder
        RET 0x14 // original four arguments plus borrowed CRT // 004f45d3
L_004f45d6:
        FLD dword ptr [ESP + 0x34] // 004f45d6
        FLD dword ptr [ESP] // 004f45da
        FLD dword ptr [ESP + 0x30] // 004f45dd
        FXCH st(2) // 004f45e1
        FXCH // 004f45e3
L_004f45e5:
        MOVSS XMM0,dword ptr [ESP + 0x34] // 004f45e5
        FXCH st(2) // 004f45eb
        FCOMI st(0),st(1) // 004f45ed
        MOVSS XMM1,dword ptr [ESP + 0x30] // 004f45ef
        JBE L_004f45ff // 004f45f5
        MOVSS dword ptr [ESP + 0x34],XMM1 // 004f45f7
        JMP L_004f4605 // 004f45fd
L_004f45ff:
        MOVSS dword ptr [ESP + 0x34],XMM0 // 004f45ff
L_004f4605:
        FXCH // 004f4605
        FCOMI st(0),st(1) // 004f4607
        JBE L_004f4613 // 004f4609
        MOVSS dword ptr [ESP + 0x30],XMM1 // 004f460b
        JMP L_004f4619 // 004f4611
L_004f4613:
        MOVSS dword ptr [ESP + 0x30],XMM0 // 004f4613
L_004f4619:
        FLD dword ptr [ESP + 0x34] // 004f4619
        FSUB dword ptr [ESP + 0x30] // 004f461d
        FCOMIP st(0),st(3) // 004f4621
        JBE L_004f4634 // 004f4623
        FSTP st(0) // 004f4625
        OR EAX,0xffffffff // 004f4627
        FSTP st(0) // 004f462a
        FSTP st(0) // 004f462c
        ADD ESP,0x28 // 004f462e
        POP EBX // restore additional CRT holder
        RET 0x14 // original four arguments plus borrowed CRT // 004f4631
L_004f4634:
        FLD st(2) // 004f4634
        FLD st(2) // 004f4636
        FMULP st(3),st(0) // 004f4638
        FMUL st(0),st(0) // 004f463a
        FLD st(1) // 004f463c
        FMULP st(2),st(0) // 004f463e
        FSUBRP st(1),st(0) // 004f4640
        FADD st(0),st(1) // 004f4642
        FXCH st(2) // 004f4644
        FADD st(0),st(0) // 004f4646
        FDIVP st(2),st(0) // 004f4648
        FXCH // 004f464a
        FSTP dword ptr [ESP + 0x34] // 004f464c
        FLD dword ptr [ESP + 0x34] // 004f4650
        FMUL st(0),st(0) // 004f4654
        FSUBP st(1),st(0) // 004f4656
        FSTP dword ptr [ESP + 0x30] // 004f4658
        FLD dword ptr [ESP + 0x30] // 004f465c
        MOV ECX,EBX // borrowed CRT, no added FP operation
        CALL native_crt_sqrt_st0_00bf7030 // 004f4660
        FSTP dword ptr [ESP + 0x30] // 004f4665
        FLD dword ptr [ESP + 0x30] // 004f4669
        SUB ESP,0x8 // 004f466d
        FSTP dword ptr [ESP + 0x38] // 004f4670
        FLD dword ptr [ESP + 0x3c] // 004f4674
        FLD dword ptr [ESP + 0x8] // 004f4678
        FLD st(0) // 004f467c
        FDIVP st(2),st(0) // 004f467e
        FXCH // 004f4680
        FSTP dword ptr [ESP + 0x3c] // 004f4682
        FLD dword ptr [ESP + 0x3c] // 004f4686
        FLD st(0) // 004f468a
        FMUL dword ptr [ESP + 0x20] // 004f468c
        FSTP dword ptr [ESP + 0x8] // 004f4690
        FMUL dword ptr [ESP + 0x24] // 004f4694
        FSTP dword ptr [ESP + 0xc] // 004f4698
        FLD dword ptr [ESP + 0x10] // 004f469c
        FADD dword ptr [ESP + 0x8] // 004f46a0
        FSTP dword ptr [ESP + 0x10] // 004f46a4
        FLD dword ptr [ESP + 0x18] // 004f46a8
        FADD dword ptr [ESP + 0xc] // 004f46ac
        FSTP dword ptr [ESP + 0x14] // 004f46b0
        FLD qword ptr [ESP + 0x28] // 004f46b4
        FSTP dword ptr [ESP + 0x3c] // 004f46b8
        FLD dword ptr [ESP + 0x3c] // 004f46bc
        FSTP dword ptr [ESP + 0x4] // 004f46c0
        FSTP dword ptr [ESP] // 004f46c4
        CALL circle_relative_equal_004f3560 // 004f46c7
        TEST AL,AL // 004f46cc
        JZ L_004f4702 // 004f46ce
        MOV EAX,dword ptr [ESP + 0x3c] // 004f46d0
        MOVSS XMM0,dword ptr [ESP + 0x8] // 004f46d4
        MOV ECX,dword ptr [ESP + 0x38] // 004f46da
        MOVSS XMM1,dword ptr [ESP + 0xc] // 004f46de
        MOVSS dword ptr [EAX + 0x4],XMM1 // 004f46e4
        MOVSS dword ptr [EAX],XMM0 // 004f46e9
        MOVSS dword ptr [ECX],XMM0 // 004f46ed
        FLD dword ptr [EAX + 0x4] // 004f46f1
        FSTP dword ptr [ECX + 0x4] // 004f46f4
        MOV EAX,0x1 // 004f46f7
        ADD ESP,0x28 // 004f46fc
        POP EBX // restore additional CRT holder
        RET 0x14 // original four arguments plus borrowed CRT // 004f46ff
L_004f4702:
        LEA ECX,[ESP + 0x18] // 004f4702
        MOV EDX,EBX // existing reciprocal kernel adds CRT in EDX
        CALL native_vector2_reciprocal_length_00419260 // 004f4706
        FSTP dword ptr [ESP + 0x34] // 004f470b
        FLD dword ptr [ESP + 0x34] // 004f470f
        MOV EAX,dword ptr [ESP + 0x38] // 004f4713
        FLD st(0) // 004f4717
        FMUL dword ptr [ESP + 0x18] // 004f4719
        FSTP dword ptr [ESP + 0x10] // 004f471d
        FMUL dword ptr [ESP + 0x1c] // 004f4721
        FSTP dword ptr [ESP + 0x14] // 004f4725
        FLD dword ptr [ESP + 0x14] // 004f4729
        FCHS // 004f472d
        FSTP dword ptr [ESP + 0x20] // 004f472f
        FLD dword ptr [ESP + 0x20] // 004f4733
        FLD dword ptr [ESP + 0x30] // 004f4737
        FLD st(0) // 004f473b
        FMULP st(2),st(0) // 004f473d
        FXCH // 004f473f
        FSTP dword ptr [ESP + 0x18] // 004f4741
        FMUL dword ptr [ESP + 0x10] // 004f4745
        FSTP dword ptr [ESP + 0x1c] // 004f4749
        FLD dword ptr [ESP + 0x8] // 004f474d
        FLD st(0) // 004f4751
        FLD dword ptr [ESP + 0x18] // 004f4753
        FLD st(0) // 004f4757
        FSUBP st(2),st(0) // 004f4759
        FXCH // 004f475b
        FSTP dword ptr [ESP + 0x10] // 004f475d
        FLD dword ptr [ESP + 0xc] // 004f4761
        FLD st(0) // 004f4765
        FLD dword ptr [ESP + 0x1c] // 004f4767
        FLD st(0) // 004f476b
        FSUBP st(2),st(0) // 004f476d
        FXCH // 004f476f
        FSTP dword ptr [ESP + 0x14] // 004f4771
        FLD dword ptr [ESP + 0x10] // 004f4775
        FSTP dword ptr [EAX] // 004f4779
        FLD dword ptr [ESP + 0x14] // 004f477b
        FSTP dword ptr [EAX + 0x4] // 004f477f
        MOV EAX,dword ptr [ESP + 0x3c] // 004f4782
        FXCH st(2) // 004f4786
        FADDP st(3),st(0) // 004f4788
        FXCH st(2) // 004f478a
        FSTP dword ptr [ESP + 0x18] // 004f478c
        FADDP st(1),st(0) // 004f4790
        FSTP dword ptr [ESP + 0x1c] // 004f4792
        FLD dword ptr [ESP + 0x18] // 004f4796
        FSTP dword ptr [EAX] // 004f479a
        FLD dword ptr [ESP + 0x1c] // 004f479c
        FSTP dword ptr [EAX + 0x4] // 004f47a0
        MOV EAX,0x2 // 004f47a3
        ADD ESP,0x28 // 004f47a8
        POP EBX // restore additional CRT holder
        RET 0x14 // original four arguments plus borrowed CRT // 004f47ab
    }
}

__declspec(naked) float __fastcall circle_sqrt(const CameraAxesCrtAccess*, float) {
    __asm {
        fld dword ptr [esp + 4]
        call native_crt_sqrt_st0_00bf7030
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        ret 4
    }
}
}

ShipAiCircleIntersection circle_intersections_004f4520(
    const ShipAiAttackMoveXZ& first_center, const ShipAiAttackMoveXZ& second_center,
    float first_radius, float second_radius,
    ShipAiAttackMoveXZ& first, ShipAiAttackMoveXZ& second,
    const CameraAxesCrtAccess& crt) {
    return static_cast<ShipAiCircleIntersection>(circle_intersection_kernel(
        &first_center.x, &second_center.x, first_radius, second_radius,
        &first.x, &second.x, &crt));
}

ShipAiCircleIntersection circle_intersections_004f47b0(
    const ShipAiCircleTangentCircle& first_circle,
    const ShipAiCircleTangentCircle& second_circle,
    ShipAiAttackMoveXZ& first, ShipAiAttackMoveXZ& second,
    const CameraAxesCrtAccess& crt) {
    // 004F47B4/004F47C4 forward each circle's radius, ECX/EDX its center.
    return circle_intersections_004f4520({first_circle.x, first_circle.z},
        {second_circle.x, second_circle.z}, first_circle.radius,
        second_circle.radius, first, second, crt);
}

ShipAiCircleGeometryHost::ShipAiCircleGeometryHost(
    const ShipAiCircleTangentCircle& circle, const CameraAxesCrtAccess& crt,
    ShipAiCircleRandomDraw random_draw, void* random_context) noexcept
    : circle_(circle), crt_(crt), random_draw_(random_draw), random_context_(random_context) {}

bool ShipAiCircleGeometryHost::tangent_points_009d6550(
    const ShipAiAttackMoveXZ& point, ShipAiAttackMoveXZ& first, ShipAiAttackMoveXZ& second) {
    const auto pair = ship_ai_path_tangent_points_009d6550(circle_, {{point.x, point.z}});
    if (!pair.valid) return false; // preserve BOTH incoming outputs on false
    first = {pair.first[0], pair.first[1]};
    second = {pair.second[0], pair.second[1]};
    return true;
}

void ShipAiCircleGeometryHost::offset_points_004f47b0(
    const ShipAiAttackMoveXZ& point, float clearance,
    ShipAiAttackMoveXZ& first, ShipAiAttackMoveXZ& second) {
    // 009D6A8F ignores EAX. The shared kernel preserves outputs on <= 0.
    circle_intersections_004f47b0({point.x, point.z, clearance}, circle_, first, second, crt_);
}

float ShipAiCircleGeometryHost::random_stream1_00bd2f10(float low, float high) {
    return random_draw_(random_context_, low, high);
}

float ShipAiCircleGeometryHost::sqrt_00bf7030(float value) {
    return circle_sqrt(&crt_, value);
}
} // namespace bsp
