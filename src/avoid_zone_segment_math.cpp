#include "bsp/avoid_zone_segment_math.hpp"
#include "bsp/native_vector2_math.hpp"
#include "bsp/ship_ai_nav_circle_tangent.hpp"

namespace bsp {
namespace {
const double segment_squared_length_cutoff = 1e-10; // 00CE3820, live bytes checked.
const float segment_one = 1.0f; // 00D7A24C.

// Existing vector_helpers.cpp uses current _CIsqrt. This private consumed-callee
// bridge retains 00414C60's schedule with the required actual borrowed CRT.
__declspec(naked) float __fastcall segment_cutoff_length(const float*, const CameraAxesCrtAccess*) {
    __asm {
        push ebx
        mov ebx, edx
        PUSH ECX // 00414c60
        FLD dword ptr [ECX + 0x4] // 00414c61
        FLD dword ptr [ECX] // 00414c64
        FMUL st(0),st(0) // 00414c66
        FLD st(1) // 00414c68
        FMULP st(2),st(0) // 00414c6a
        FADDP st(1),st(0) // 00414c6c
        FSTP dword ptr [ESP] // 00414c6e
        FLD qword ptr [segment_squared_length_cutoff] // 00414c71
        FLD dword ptr [ESP] // 00414c77
        FCOMI st(0),st(1) // 00414c7a
        FSTP st(1) // 00414c7c
        JBE L_00414c93 // 00414c7e
        mov ecx, ebx
        CALL native_crt_sqrt_st0_00bf7030 // 00414c80
        FSTP dword ptr [ESP] // 00414c85
        FLD dword ptr [ESP] // 00414c88
        FSTP dword ptr [ESP] // 00414c8b
        FLD dword ptr [ESP] // 00414c8e
        POP ECX // 00414c91
        pop ebx
        RET // 00414c92
L_00414c93:
        XORPS XMM0,XMM0 // 00414c93
        FSTP st(0) // 00414c96
        MOVSS dword ptr [ESP],XMM0 // 00414c98
        FLD dword ptr [ESP] // 00414c9d
        POP ECX // 00414ca0
        pop ebx
        RET // 00414ca1
    }
}
} // namespace

__declspec(naked) bool __fastcall native_segment_parameters_004f3630(
    const float*, const float*, const float*, const float*, float*, float*) noexcept {
    __asm {
        SUB ESP,0x1c // 004f3630
        MOVSS XMM0,dword ptr [ECX] // 004f3633
        FLD dword ptr [EDX] // 004f3637
        MOV EAX,dword ptr [ESP + 0x20] // 004f3639
        FSTP dword ptr [ESP + 0x8] // 004f363d
        FLD dword ptr [EDX + 0x4] // 004f3641
        MOVSS dword ptr [ESP + 0x10],XMM0 // 004f3644
        MOVSS XMM0,dword ptr [ECX + 0x4] // 004f364a
        FSTP dword ptr [ESP + 0x4] // 004f364f
        MOVSS dword ptr [ESP + 0x18],XMM0 // 004f3653
        MOVSS XMM0,dword ptr [EAX] // 004f3659
        MOVSS dword ptr [ESP + 0x14],XMM0 // 004f365d
        MOVSS XMM0,dword ptr [EAX + 0x4] // 004f3663
        MOV EAX,dword ptr [ESP + 0x24] // 004f3668
        FLD dword ptr [EAX] // 004f366c
        SUB ESP,0x8 // 004f366e
        FSTP dword ptr [ESP + 0x28] // 004f3671
        MOVSS dword ptr [ESP + 0x14],XMM0 // 004f3675
        FLD dword ptr [EAX + 0x4] // 004f367b
        FSTP dword ptr [ESP + 0x2c] // 004f367e
        FLD dword ptr [ESP + 0x28] // 004f3682
        FMUL dword ptr [ESP + 0xc] // 004f3686
        FLD dword ptr [ESP + 0x2c] // 004f368a
        FMUL dword ptr [ESP + 0x10] // 004f368e
        FSUBP st(1),st(0) // 004f3692
        FSTP dword ptr [ESP + 0x8] // 004f3694
        FLDZ // 004f3698
        FSTP dword ptr [ESP + 0x4] // 004f369a
        FLD dword ptr [ESP + 0x8] // 004f369e
        FSTP dword ptr [ESP] // 004f36a2
        CALL circle_relative_equal_004f3560 // 004f36a5
        TEST AL,AL // 004f36aa
        JZ L_004f36b6 // 004f36ac
        XOR AL,AL // 004f36ae
        ADD ESP,0x1c // 004f36b0
        RET 0x10 // 004f36b3
L_004f36b6:
        FLD dword ptr [ESP + 0x20] // 004f36b6
        MOV EAX,dword ptr [ESP + 0x28] // 004f36ba
        FLD st(0) // 004f36be
        MOV ECX,dword ptr [ESP + 0x2c] // 004f36c0
        FLD dword ptr [ESP + 0xc] // 004f36c4
        FLD st(0) // 004f36c8
        FMULP st(2),st(0) // 004f36ca
        FLD dword ptr [ESP + 0x24] // 004f36cc
        FLD st(0) // 004f36d0
        FLD dword ptr [ESP + 0x10] // 004f36d2
        FLD st(0) // 004f36d6
        FMULP st(2),st(0) // 004f36d8
        FXCH st(4) // 004f36da
        FADDP st(1),st(0) // 004f36dc
        FLD dword ptr [ESP + 0x14] // 004f36de
        FLD st(0) // 004f36e2
        FMULP st(3),st(0) // 004f36e4
        FXCH // 004f36e6
        FSUBRP st(2),st(0) // 004f36e8
        FLD dword ptr [ESP + 0x18] // 004f36ea
        FLD st(0) // 004f36ee
        FMULP st(6),st(0) // 004f36f0
        FXCH st(2) // 004f36f2
        FSUBRP st(5),st(0) // 004f36f4
        FLD dword ptr [ESP] // 004f36f6
        FLD st(0) // 004f36f9
        FDIVP st(6),st(0) // 004f36fb
        FXCH st(5) // 004f36fd
        FSTP dword ptr [EAX] // 004f36ff
        FLD dword ptr [ESP + 0x4] // 004f3701
        FLD st(0) // 004f3705
        MOV AL,0x1 // 004f3707
        FMULP st(2),st(0) // 004f3709
        FLD dword ptr [ESP + 0x8] // 004f370b
        FLD st(0) // 004f370f
        FMULP st(4),st(0) // 004f3711
        FXCH st(2) // 004f3713
        FADDP st(3),st(0) // 004f3715
        FMULP st(4),st(0) // 004f3717
        FXCH // 004f3719
        FSUBRP st(3),st(0) // 004f371b
        FMULP st(1),st(0) // 004f371d
        FSUBP st(1),st(0) // 004f371f
        FXCH // 004f3721
        FCHS // 004f3723
        FDIVP st(1),st(0) // 004f3725
        FSTP dword ptr [ECX] // 004f3727
        ADD ESP,0x1c // 004f3729
        RET 0x10 // 004f372c
    }
}

namespace {
// Added saved EBP holds CRT. Native local offsets stay unchanged; only the
// original query argument/scratch slot moves +4. Its initial pointer bits are
// deliberately retained if the parameter solver declines to write outputs.
__declspec(naked) float __fastcall segment_distance_kernel(
    const float*, const float*, const float*, const CameraAxesCrtAccess*) {
    __asm {
        push ebp
        mov ebp, dword ptr [esp + 0xc]
        SUB ESP,0x18 // 00419ab0
        PUSH EBX // 00419ab3
        PUSH ESI // 00419ab4
        PUSH EDI // 00419ab5
        MOV EDI,EDX // 00419ab6
        FLD dword ptr [EDI] // 00419ab8
        MOV ESI,ECX // 00419aba
        FSUB dword ptr [ESI] // 00419abc
        LEA ECX,[ESP + 0x14] // 00419abe
        FSTP dword ptr [ESP + 0x14] // 00419ac2
        FLD dword ptr [EDI + 0x4] // 00419ac6
        FSUB dword ptr [ESI + 0x4] // 00419ac9
        FSTP dword ptr [ESP + 0x18] // 00419acc
        mov edx, ebp
        CALL native_vector2_reciprocal_length_00419260 // 00419ad0
        FSTP dword ptr [ESP + 0xc] // 00419ad5
        MOV EBX,dword ptr [ESP + 0x2c] // 00419ad9
        FLD dword ptr [ESP + 0x14] // 00419add
        LEA EAX,[ESP + 0xc] // 00419ae1
        FLD dword ptr [ESP + 0xc] // 00419ae5
        PUSH EAX // 00419ae9
        FLD st(0) // 00419aea
        LEA ECX,[ESP + 0x30] // 00419aec
        FMULP st(2),st(0) // 00419af0
        PUSH ECX // 00419af2
        FXCH // 00419af3
        LEA EDX,[ESP + 0x24] // 00419af5
        PUSH EDX // 00419af9
        FSTP dword ptr [ESP + 0x18] // 00419afa
        PUSH EBX // 00419afe
        LEA EDX,[ESP + 0x24] // 00419aff
        FMUL dword ptr [ESP + 0x28] // 00419b03
        MOV ECX,ESI // 00419b07
        FSTP dword ptr [ESP + 0x20] // 00419b09
        FLD dword ptr [ESP + 0x20] // 00419b0d
        FCHS // 00419b11
        FSTP dword ptr [ESP + 0x2c] // 00419b13
        FLD dword ptr [ESP + 0x1c] // 00419b17
        FSTP dword ptr [ESP + 0x30] // 00419b1b
        CALL native_segment_parameters_004f3630 // 00419b1f
        XORPS XMM0,XMM0 // 00419b24
        MOVSS XMM1,dword ptr [ESP + 0x2c] // 00419b27
        COMISS XMM0,XMM1 // 00419b2d
        JBE L_00419b9b // 00419b30
        FLD dword ptr [EBX] // 00419b32
        FSUB dword ptr [ESI] // 00419b34
        FSTP dword ptr [ESP + 0x14] // 00419b36
        FLD dword ptr [EBX + 0x4] // 00419b3a
        FSUB dword ptr [ESI + 0x4] // 00419b3d
        FSTP dword ptr [ESP + 0x18] // 00419b40
        FLD dword ptr [ESP + 0x18] // 00419b44
        FLD dword ptr [ESP + 0x14] // 00419b48
        FMUL st(0),st(0) // 00419b4c
        FLD st(1) // 00419b4e
        FMULP st(2),st(0) // 00419b50
        FADDP st(1),st(0) // 00419b52
        FSTP dword ptr [ESP + 0x2c] // 00419b54
        FLD qword ptr [segment_squared_length_cutoff] // 00419b58
        FLD dword ptr [ESP + 0x2c] // 00419b5e
        FCOMI st(0),st(1) // 00419b62
        FSTP st(1) // 00419b64
        JBE L_00419b86 // 00419b66
        mov ecx, ebp
        CALL native_crt_sqrt_st0_00bf7030 // 00419b68
        FSTP dword ptr [ESP + 0x2c] // 00419b6d
        FLD dword ptr [ESP + 0x2c] // 00419b71
        POP EDI // 00419b75
        POP ESI // 00419b76
        FSTP dword ptr [ESP + 0x24] // 00419b77
        FLD dword ptr [ESP + 0x24] // 00419b7b
        POP EBX // 00419b7f
        ADD ESP,0x18 // 00419b80
        pop ebp
        RET 0x8 // 00419b83
L_00419b86:
        POP EDI // 00419b86
        FSTP st(0) // 00419b87
        POP ESI // 00419b89
        MOVSS dword ptr [ESP + 0x24],XMM0 // 00419b8a
        FLD dword ptr [ESP + 0x24] // 00419b90
        POP EBX // 00419b94
        ADD ESP,0x18 // 00419b95
        pop ebp
        RET 0x8 // 00419b98
L_00419b9b:
        COMISS XMM1,dword ptr [segment_one] // 00419b9b
        JBE L_00419bc8 // 00419ba2
        FLD dword ptr [EBX] // 00419ba4
        LEA ECX,[ESP + 0x1c] // 00419ba6
        FSUB dword ptr [EDI] // 00419baa
        FSTP dword ptr [ESP + 0x1c] // 00419bac
        FLD dword ptr [EBX + 0x4] // 00419bb0
        FSUB dword ptr [EDI + 0x4] // 00419bb3
        FSTP dword ptr [ESP + 0x20] // 00419bb6
        mov edx, ebp
        CALL segment_cutoff_length // 00419bba
        POP EDI // 00419bbf
        POP ESI // 00419bc0
        POP EBX // 00419bc1
        ADD ESP,0x18 // 00419bc2
        pop ebp
        RET 0x8 // 00419bc5
L_00419bc8:
        MOVSS XMM0,dword ptr [ESP + 0xc] // 00419bc8
        MOVSS dword ptr [ESP + 0x2c],XMM0 // 00419bce
        MOV EAX,dword ptr [ESP + 0x2c] // 00419bd4
        POP EDI // 00419bd8
        AND EAX,0x7fffffff // 00419bd9
        POP ESI // 00419bde
        MOV dword ptr [ESP + 0x24],EAX // 00419bdf
        FLD dword ptr [ESP + 0x24] // 00419be3
        POP EBX // 00419be7
        ADD ESP,0x18 // 00419be8
        pop ebp
        RET 0x8 // 00419beb
    }
}

// EDX carries CRT. Added EBX save leaves native locals intact and moves only
// the output/query stack arguments +4; original RET8 still cleans those two.
__declspec(naked) float* __fastcall segment_closest_point_kernel(
    const float*, const CameraAxesCrtAccess*, float*, const float*) {
    __asm {
        push ebx
        mov ebx, edx
        SUB ESP,0x20 // 004f4b50
        PUSH ESI // 004f4b53
        MOV ESI,ECX // 004f4b54
        FLD dword ptr [ESI] // 004f4b56
        LEA ECX,[ESP + 0x14] // 004f4b58
        FSTP dword ptr [ESP + 0x4] // 004f4b5c
        FLD dword ptr [ESI + 0x8] // 004f4b60
        FSUB dword ptr [ESP + 0x4] // 004f4b63
        FSTP dword ptr [ESP + 0x14] // 004f4b67
        FLD dword ptr [ESI + 0x4] // 004f4b6b
        FSTP dword ptr [ESP + 0x8] // 004f4b6e
        FLD dword ptr [ESI + 0xc] // 004f4b72
        FSUB dword ptr [ESP + 0x8] // 004f4b75
        FSTP dword ptr [ESP + 0x18] // 004f4b79
        mov edx, ebx
        CALL native_vector2_reciprocal_length_00419260 // 004f4b7d
        FSTP dword ptr [ESP + 0xc] // 004f4b82
        LEA EAX,[ESP + 0xc] // 004f4b86
        FLD dword ptr [ESP + 0xc] // 004f4b8a
        PUSH EAX // 004f4b8e
        MOV EAX,dword ptr [ESP + 0x34] // 004f4b8f
        FLD st(0) // 004f4b93
        FMUL dword ptr [ESP + 0x18] // 004f4b95
        LEA ECX,[ESP + 0x34] // 004f4b99
        PUSH ECX // 004f4b9d
        LEA EDX,[ESP + 0x24] // 004f4b9e
        FSTP dword ptr [ESP + 0x14] // 004f4ba2
        PUSH EDX // 004f4ba6
        PUSH EAX // 004f4ba7
        FMUL dword ptr [ESP + 0x28] // 004f4ba8
        LEA EDX,[ESP + 0x24] // 004f4bac
        MOV ECX,ESI // 004f4bb0
        FSTP dword ptr [ESP + 0x20] // 004f4bb2
        FLD dword ptr [ESP + 0x20] // 004f4bb6
        FCHS // 004f4bba
        FSTP dword ptr [ESP + 0x2c] // 004f4bbc
        FLD dword ptr [ESP + 0x1c] // 004f4bc0
        FSTP dword ptr [ESP + 0x30] // 004f4bc4
        CALL native_segment_parameters_004f3630 // 004f4bc8
        XORPS XMM1,XMM1 // 004f4bcd
        MOVSS XMM0,dword ptr [ESP + 0x30] // 004f4bd0
        COMISS XMM1,XMM0 // 004f4bd6
        JBE L_004f4be3 // 004f4bd9
L_004f4bdb:
        MOVSS dword ptr [ESP + 0x30],XMM1 // 004f4bdb
        JMP L_004f4bf6 // 004f4be1
L_004f4be3:
        MOVSS XMM1,dword ptr [segment_one] // 004f4be3
        COMISS XMM0,XMM1 // 004f4beb
        JA L_004f4bdb // 004f4bee
        MOVSS dword ptr [ESP + 0x30],XMM0 // 004f4bf0
L_004f4bf6:
        FLD dword ptr [ESI + 0x8] // 004f4bf6
        MOV EAX,dword ptr [ESP + 0x2c] // 004f4bf9
        FLD dword ptr [ESP + 0x30] // 004f4bfd
        FLD st(0) // 004f4c01
        FMULP st(2),st(0) // 004f4c03
        FXCH // 004f4c05
        FSTP dword ptr [ESP + 0x14] // 004f4c07
        FLD dword ptr [ESI + 0xc] // 004f4c0b
        POP ESI // 004f4c0e
        FMUL st(0),st(1) // 004f4c0f
        FSTP dword ptr [ESP + 0x14] // 004f4c11
        FLD1 // 004f4c15
        FSUBRP st(1),st(0) // 004f4c17
        FSTP dword ptr [ESP + 0x2c] // 004f4c19
        FLD dword ptr [ESP] // 004f4c1d
        FLD dword ptr [ESP + 0x2c] // 004f4c20
        FLD st(0) // 004f4c24
        FMULP st(2),st(0) // 004f4c26
        FXCH // 004f4c28
        FSTP dword ptr [ESP + 0x8] // 004f4c2a
        FMUL dword ptr [ESP + 0x4] // 004f4c2e
        FSTP dword ptr [ESP + 0xc] // 004f4c32
        FLD dword ptr [ESP + 0x8] // 004f4c36
        FADD dword ptr [ESP + 0x10] // 004f4c3a
        FSTP dword ptr [EAX] // 004f4c3e
        FLD dword ptr [ESP + 0x14] // 004f4c40
        FADD dword ptr [ESP + 0xc] // 004f4c44
        FSTP dword ptr [EAX + 0x4] // 004f4c48
        ADD ESP,0x20 // 004f4c4b
        pop ebx
        RET 0x8 // 004f4c4e
    }
}
} // namespace

float avoid_zone_segment_distance_00419ab0(const std::array<float, 2>& start,
    const std::array<float, 2>& end, const std::array<float, 2>& query,
    const CameraAxesCrtAccess& crt) {
    return segment_distance_kernel(start.data(), end.data(), query.data(), &crt);
}

std::array<float, 2>& avoid_zone_segment_closest_point_004f4b50(
    const std::array<float, 4>& endpoints, std::array<float, 2>& output,
    const std::array<float, 2>& query, const CameraAxesCrtAccess& crt) {
    segment_closest_point_kernel(endpoints.data(), &crt, output.data(), query.data());
    return output;
}
} // namespace bsp
