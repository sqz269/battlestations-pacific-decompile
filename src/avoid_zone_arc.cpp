#include "bsp/avoid_zone_arc.hpp"
#include "bsp/avoid_zone_segment_math.hpp"
#include "bsp/unit_rudder.hpp"

extern "C" double __cdecl _CIatan2(); // existing host CRT ST0=x/ST1=y boundary

namespace bsp {
namespace {
const double arc_squared_length_cutoff=1e-10; // 00CE3820
const double arc_quarter_turn=1.57079637050628662109375; // 00CE3830, widened float
const double arc_full_turn=6.283185482025146484375; // 00CE3828, widened float

// Original ECX/EDX and three stack arguments preserved; required CRT appended
// as a fourth stack argument. Native scratch/argument spill locations unchanged.
__declspec(naked) int __fastcall circle_segment_kernel(const float*,const float*,
    float,const float*,float*,const CameraAxesCrtAccess*) {
    __asm {
        SUB ESP,0x28 // 004f3ba0
        MOV EAX,dword ptr [ESP + 0x30] // 004f3ba3
        PUSH EBX // 004f3ba7
        PUSH EBP // 004f3ba8
        PUSH ESI // 004f3ba9
        MOV ESI,EDX // 004f3baa
        FLD dword ptr [ESI] // 004f3bac
        MOV EBX,dword ptr [ESP + 0x40] // 004f3bae
        FSTP dword ptr [ESP + 0xc] // 004f3bb2
        PUSH EDI // 004f3bb6
        FLD dword ptr [EAX] // 004f3bb7
        MOV EBP,ECX // 004f3bb9
        FSUB dword ptr [ESP + 0x10] // 004f3bbb
        MOV EDI,EBX // 004f3bbf
        FSTP dword ptr [ESP + 0x20] // 004f3bc1
        FLD dword ptr [ESI + 0x4] // 004f3bc5
        FSTP dword ptr [ESP + 0x40] // 004f3bc8
        FLD dword ptr [EAX + 0x4] // 004f3bcc
        FSUB dword ptr [ESP + 0x40] // 004f3bcf
        FSTP dword ptr [ESP + 0x24] // 004f3bd3
        FLD dword ptr [ESP + 0x20] // 004f3bd7
        FLD dword ptr [ESP + 0x24] // 004f3bdb
        FMUL st(0),st(0) // 004f3bdf
        FLD st(1) // 004f3be1
        FMULP st(2),st(0) // 004f3be3
        FADDP st(1),st(0) // 004f3be5
        FSTP dword ptr [ESP + 0x44] // 004f3be7
        FLD qword ptr [arc_squared_length_cutoff] // 004f3beb
        FLD dword ptr [ESP + 0x44] // 004f3bf1
        FCOMI st(0),st(1) // 004f3bf5
        FSTP st(1) // 004f3bf7
        JBE L_004f3c0e // 004f3bf9
        mov ecx, dword ptr [esp + 0x48] // required borrowed CRT; ST0 unspilled
        CALL native_crt_sqrt_st0_00bf7030 // 004f3bfb
        FSTP dword ptr [ESP + 0x44] // 004f3c00
        FLD dword ptr [ESP + 0x44] // 004f3c04
        FSTP dword ptr [ESP + 0x44] // 004f3c08
        JMP L_004f3c19 // 004f3c0c
L_004f3c0e:
        XORPS XMM0,XMM0 // 004f3c0e
        FSTP st(0) // 004f3c11
        MOVSS dword ptr [ESP + 0x44],XMM0 // 004f3c13
L_004f3c19:
        FLD dword ptr [ESP + 0x20] // 004f3c19
        LEA EAX,[ESP + 0x18] // 004f3c1d
        FLD dword ptr [ESP + 0x44] // 004f3c21
        PUSH EAX // 004f3c25
        FLD st(0) // 004f3c26
        LEA ECX,[ESP + 0x24] // 004f3c28
        FDIVP st(2),st(0) // 004f3c2c
        PUSH ECX // 004f3c2e
        LEA EDX,[ESP + 0x38] // 004f3c2f
        PUSH EDX // 004f3c33
        PUSH EBP // 004f3c34
        LEA EDX,[ESP + 0x38] // 004f3c35
        MOV ECX,ESI // 004f3c39
        FXCH // 004f3c3b
        FSTP dword ptr [ESP + 0x38] // 004f3c3d
        FDIVR dword ptr [ESP + 0x34] // 004f3c41
        FSTP dword ptr [ESP + 0x3c] // 004f3c45
        FLD dword ptr [ESP + 0x3c] // 004f3c49
        FCHS // 004f3c4d
        FSTP dword ptr [ESP + 0x40] // 004f3c4f
        FLD dword ptr [ESP + 0x38] // 004f3c53
        FSTP dword ptr [ESP + 0x44] // 004f3c57
        CALL native_segment_parameters_004f3630 // 004f3c5b
        FLD dword ptr [ESP + 0x3c] // 004f3c60
        FLD st(0) // 004f3c64
        FCHS // 004f3c66
        FLD dword ptr [ESP + 0x18] // 004f3c68
        FCOMI st(0),st(1) // 004f3c6c
        FSTP st(1) // 004f3c6e
        JBE L_004f3d67 // 004f3c70
        FXCH // 004f3c76
        FCOMI st(0),st(1) // 004f3c78
        JBE L_004f3d7c // 004f3c7a
        FMUL st(0),st(0) // 004f3c80
        FLD st(1) // 004f3c82
        FMULP st(2),st(0) // 004f3c84
        FSUBRP st(1),st(0) // 004f3c86
        FSTP dword ptr [ESP + 0x3c] // 004f3c88
        FLD dword ptr [ESP + 0x3c] // 004f3c8c
        mov ecx, dword ptr [esp + 0x48] // required borrowed CRT; ST0 unspilled
        CALL native_crt_sqrt_st0_00bf7030 // 004f3c90
        FSTP dword ptr [ESP + 0x3c] // 004f3c95
        FLD dword ptr [ESP + 0x3c] // 004f3c99
        FSTP dword ptr [ESP + 0x3c] // 004f3c9d
        FLD dword ptr [ESP + 0x20] // 004f3ca1
        FLD st(0) // 004f3ca5
        FLD dword ptr [ESP + 0x3c] // 004f3ca7
        FLD st(0) // 004f3cab
        FSUBP st(2),st(0) // 004f3cad
        FXCH // 004f3caf
        FSTP dword ptr [ESP + 0x3c] // 004f3cb1
        FADDP st(1),st(0) // 004f3cb5
        FSTP dword ptr [ESP + 0x20] // 004f3cb7
        FLDZ // 004f3cbb
        FLD dword ptr [ESP + 0x3c] // 004f3cbd
        FCOMI st(0),st(1) // 004f3cc1
        JBE L_004f3d09 // 004f3cc3
        FLD dword ptr [ESP + 0x44] // 004f3cc5
        FCOMIP st(0),st(1) // 004f3cc9
        JBE L_004f3d09 // 004f3ccb
        FLD dword ptr [ESP + 0x28] // 004f3ccd
        LEA EDI,[EBX + 0x8] // 004f3cd1
        FMUL st(0),st(1) // 004f3cd4
        FSTP dword ptr [ESP + 0x18] // 004f3cd6
        FMUL dword ptr [ESP + 0x2c] // 004f3cda
        FSTP dword ptr [ESP + 0x1c] // 004f3cde
        FLD dword ptr [ESP + 0x10] // 004f3ce2
        FADD dword ptr [ESP + 0x18] // 004f3ce6
        FSTP dword ptr [ESP + 0x10] // 004f3cea
        FLD dword ptr [ESP + 0x40] // 004f3cee
        FADD dword ptr [ESP + 0x1c] // 004f3cf2
        FSTP dword ptr [ESP + 0x14] // 004f3cf6
        FLD dword ptr [ESP + 0x10] // 004f3cfa
        FSTP dword ptr [EBX] // 004f3cfe
        FLD dword ptr [ESP + 0x14] // 004f3d00
        FSTP dword ptr [EBX + 0x4] // 004f3d04
        JMP L_004f3d0b // 004f3d07
L_004f3d09:
        FSTP st(0) // 004f3d09
L_004f3d0b:
        FLD dword ptr [ESP + 0x20] // 004f3d0b
        FCOMI st(0),st(1) // 004f3d0f
        FSTP st(1) // 004f3d11
        JBE L_004f3d7e // 004f3d13
        FLD dword ptr [ESP + 0x44] // 004f3d15
        FCOMIP st(0),st(1) // 004f3d19
        JBE L_004f3d7e // 004f3d1b
        FLD dword ptr [ESP + 0x28] // 004f3d1d
        MOV EAX,EDI // 004f3d21
        FMUL st(0),st(1) // 004f3d23
        ADD EDI,0x8 // 004f3d25
        FSTP dword ptr [ESP + 0x20] // 004f3d28
        FMUL dword ptr [ESP + 0x2c] // 004f3d2c
        FSTP dword ptr [ESP + 0x24] // 004f3d30
        FLD dword ptr [ESI] // 004f3d34
        FADD dword ptr [ESP + 0x20] // 004f3d36
        FSTP dword ptr [ESP + 0x28] // 004f3d3a
        FLD dword ptr [ESP + 0x24] // 004f3d3e
        FADD dword ptr [ESI + 0x4] // 004f3d42
        FSTP dword ptr [ESP + 0x2c] // 004f3d45
        FLD dword ptr [ESP + 0x28] // 004f3d49
        FSTP dword ptr [EAX] // 004f3d4d
        FLD dword ptr [ESP + 0x2c] // 004f3d4f
        FSTP dword ptr [EAX + 0x4] // 004f3d53
        MOV EAX,EDI // 004f3d56
        POP EDI // 004f3d58
        POP ESI // 004f3d59
        SUB EAX,EBX // 004f3d5a
        POP EBP // 004f3d5c
        SAR EAX,0x3 // 004f3d5d
        POP EBX // 004f3d60
        ADD ESP,0x28 // 004f3d61
        RET 0x10 // 004f3d64
L_004f3d67:
        MOV EAX,EDI // 004f3d67
        FSTP st(1) // 004f3d69
        POP EDI // 004f3d6b
        FSTP st(0) // 004f3d6c
        POP ESI // 004f3d6e
        SUB EAX,EBX // 004f3d6f
        POP EBP // 004f3d71
        SAR EAX,0x3 // 004f3d72
        POP EBX // 004f3d75
        ADD ESP,0x28 // 004f3d76
        RET 0x10 // 004f3d79
L_004f3d7c:
        FSTP st(0) // 004f3d7c
L_004f3d7e:
        MOV EAX,EDI // 004f3d7e
        FSTP st(0) // 004f3d80
        POP EDI // 004f3d82
        POP ESI // 004f3d83
        SUB EAX,EBX // 004f3d84
        POP EBP // 004f3d86
        SAR EAX,0x3 // 004f3d87
        POP EBX // 004f3d8a
        ADD ESP,0x28 // 004f3d8b
        RET 0x10 // 004f3d8e
    }
}

// Save added EDX CRT before the native frame. Only original argument references
// move four bytes; local spills and candidate/selection ordering are unchanged.
__declspec(naked) AvoidZoneSelectedSegment* __fastcall selected_arc_kernel(
    AvoidZoneSelectedSegment* const*,const CameraAxesCrtAccess*,const float*,
    float,float,float*) {
    __asm {
        push edx // added saved CRT binding; native locals retain their offsets
        SUB ESP,0x80 // 00415970
        FLD dword ptr [ESP + 0x8c] // 00415976
        PUSH ESI // 0041597d
        MOV ESI,dword ptr [ECX] // 0041597e
        FMUL st(0),st(0) // 00415980
        XOR EAX,EAX // 00415982
        TEST ESI,ESI // 00415984
        FSTP dword ptr [ESP + 0x14] // 00415986
        MOV dword ptr [ESP + 0x10],EAX // 0041598a
        JZ L_00415d65 // 0041598e
        PUSH EBX // 00415994
        PUSH EBP // 00415995
        MOV EBP,dword ptr [ESP + 0x94] // 00415996
        PUSH EDI // 0041599d
        MOV EDI,EDI // 0041599e
L_004159a0:
        FLD dword ptr [ESP + 0x20] // 004159a0
        LEA EDI,[ESI + 0x8] // 004159a4
        FLD dword ptr [EBP] // 004159a7
        FSTP dword ptr [ESP + 0x18] // 004159aa
        FLD dword ptr [EDI] // 004159ae
        FLD dword ptr [ESP + 0x18] // 004159b0
        FLD st(0) // 004159b4
        FSUBP st(2),st(0) // 004159b6
        FXCH // 004159b8
        FSTP dword ptr [ESP + 0x48] // 004159ba
        FLD dword ptr [EBP + 0x4] // 004159be
        FSTP dword ptr [ESP + 0x14] // 004159c1
        FLD dword ptr [EDI + 0x4] // 004159c5
        FLD dword ptr [ESP + 0x14] // 004159c8
        FLD st(0) // 004159cc
        FSUBP st(2),st(0) // 004159ce
        FXCH // 004159d0
        FSTP dword ptr [ESP + 0x4c] // 004159d2
        FLD dword ptr [ESI] // 004159d6
        FSTP dword ptr [ESP + 0x10] // 004159d8
        FLD dword ptr [ESP + 0x10] // 004159dc
        FLD st(0) // 004159e0
        FSUB st(0),st(3) // 004159e2
        FSTP dword ptr [ESP + 0x50] // 004159e4
        FLD dword ptr [ESI + 0x4] // 004159e8
        FSTP dword ptr [ESP + 0x14] // 004159eb
        FLD dword ptr [ESP + 0x14] // 004159ef
        FLD st(0) // 004159f3
        FSUBRP st(3),st(0) // 004159f5
        FXCH st(2) // 004159f7
        FSTP dword ptr [ESP + 0x54] // 004159f9
        FLD dword ptr [ESP + 0x4c] // 004159fd
        FLD dword ptr [ESP + 0x48] // 00415a01
        FMUL st(0),st(0) // 00415a05
        FLD st(1) // 00415a07
        FMULP st(2),st(0) // 00415a09
        FADDP st(1),st(0) // 00415a0b
        FSTP dword ptr [ESP + 0x2c] // 00415a0d
        FLD dword ptr [ESP + 0x2c] // 00415a11
        FCOMIP st(0),st(4) // 00415a15
        JNC L_00415a59 // 00415a17
        FLD dword ptr [ESP + 0x54] // 00415a19
        FLD dword ptr [ESP + 0x50] // 00415a1d
        FMUL st(0),st(0) // 00415a21
        FLD st(1) // 00415a23
        FMULP st(2),st(0) // 00415a25
        FADDP st(1),st(0) // 00415a27
        FSTP dword ptr [ESP + 0x2c] // 00415a29
        FLD dword ptr [ESP + 0x2c] // 00415a2d
        FCOMIP st(0),st(4) // 00415a31
        FSTP st(3) // 00415a33
        JNC L_00415a5b // 00415a35
        FSTP st(2) // 00415a37
        FSTP st(1) // 00415a39
L_00415a3b:
        FSTP st(0) // 00415a3b
L_00415a3d:
        CMP byte ptr [ESI + 0x1c],0x0 // 00415a3d
        JNZ L_00415d53 // 00415a41
        MOV EAX,dword ptr [ESI + 0x10] // 00415a47
        TEST EAX,EAX // 00415a4a
        JZ L_00415d53 // 00415a4c
        MOV ESI,EAX // 00415a52
        JMP L_00415d56 // 00415a54
L_00415a59:
        FSTP st(3) // 00415a59
L_00415a5b:
        FLD dword ptr [EDI] // 00415a5b
        FSUB st(0),st(3) // 00415a5d
        FSTP dword ptr [ESP + 0x38] // 00415a5f
        FLD dword ptr [EDI + 0x4] // 00415a63
        FSUB st(0),st(1) // 00415a66
        FSTP dword ptr [ESP + 0x3c] // 00415a68
        FLD dword ptr [ESP + 0x3c] // 00415a6c
        FLD dword ptr [ESP + 0x38] // 00415a70
        FMUL st(0),st(0) // 00415a74
        FLD st(1) // 00415a76
        FMULP st(2),st(0) // 00415a78
        FADDP st(1),st(0) // 00415a7a
        FSTP dword ptr [ESP + 0x2c] // 00415a7c
        FLD qword ptr [arc_squared_length_cutoff] // 00415a80
        FLD dword ptr [ESP + 0x2c] // 00415a86
        FCOMI st(0),st(1) // 00415a8a
        FSTP st(1) // 00415a8c
        JBE L_00415ab5 // 00415a8e
        FSTP st(3) // 00415a90
        FSTP st(0) // 00415a92
        FSTP st(0) // 00415a94
        mov ecx, dword ptr [esp + 0x90] // required borrowed CRT; ST0 unspilled
        CALL native_crt_sqrt_st0_00bf7030 // 00415a96
        FSTP dword ptr [ESP + 0x2c] // 00415a9b
        FLD dword ptr [ESP + 0x2c] // 00415a9f
        FSTP dword ptr [ESP + 0x24] // 00415aa3
        FLD dword ptr [ESP + 0x18] // 00415aa7
        FLD dword ptr [ESP + 0x10] // 00415aab
        FLD dword ptr [ESP + 0x14] // 00415aaf
        JMP L_00415ac6 // 00415ab3
L_00415ab5:
        XORPS XMM0,XMM0 // 00415ab5
        FSTP st(0) // 00415ab8
        FXCH // 00415aba
        MOVSS dword ptr [ESP + 0x24],XMM0 // 00415abc
        FXCH st(2) // 00415ac2
        FXCH // 00415ac4
L_00415ac6:
        FLD dword ptr [ESP + 0x38] // 00415ac6
        FLD dword ptr [ESP + 0x24] // 00415aca
        FLD st(0) // 00415ace
        FDIVP st(2),st(0) // 00415ad0
        FXCH // 00415ad2
        FSTP dword ptr [ESP + 0x40] // 00415ad4
        FLD dword ptr [ESP + 0x3c] // 00415ad8
        FDIV st(0),st(1) // 00415adc
        FSTP dword ptr [ESP + 0x44] // 00415ade
        FLD st(3) // 00415ae2
        FSUB st(0),st(3) // 00415ae4
        FSTP dword ptr [ESP + 0x58] // 00415ae6
        FLD dword ptr [EBP + 0x4] // 00415aea
        FSTP dword ptr [ESP + 0x18] // 00415aed
        FLD dword ptr [ESP + 0x18] // 00415af1
        FSUB st(0),st(2) // 00415af5
        FSTP dword ptr [ESP + 0x5c] // 00415af7
        FLD dword ptr [ESP + 0x5c] // 00415afb
        FLD dword ptr [ESP + 0x44] // 00415aff
        FLD st(0) // 00415b03
        FMULP st(2),st(0) // 00415b05
        FLD dword ptr [ESP + 0x58] // 00415b07
        FMUL dword ptr [ESP + 0x40] // 00415b0b
        FADDP st(2),st(0) // 00415b0f
        FXCH // 00415b11
        FSTP dword ptr [ESP + 0x2c] // 00415b13
        FLD dword ptr [ESP + 0x2c] // 00415b17
        FLDZ // 00415b1b
        FCOMIP st(0),st(1) // 00415b1d
        JBE L_00415b3d // 00415b1f
        MOVSS XMM0,dword ptr [ESP + 0x10] // 00415b21
        FSTP st(4) // 00415b27
        FSTP st(2) // 00415b29
        MOVSS dword ptr [ESP + 0x30],XMM0 // 00415b2b
        MOVSS XMM0,dword ptr [ESP + 0x14] // 00415b31
        FSTP st(0) // 00415b37
        FSTP st(1) // 00415b39
        JMP L_00415b8e // 00415b3b
L_00415b3d:
        FXCH st(2) // 00415b3d
        FCOMIP st(0),st(2) // 00415b3f
        JBE L_00415b79 // 00415b41
        FLD dword ptr [ESP + 0x40] // 00415b43
        FMUL st(0),st(2) // 00415b47
        FSTP dword ptr [ESP + 0x60] // 00415b49
        FMULP st(1),st(0) // 00415b4d
        FSTP dword ptr [ESP + 0x64] // 00415b4f
        FLD dword ptr [ESP + 0x60] // 00415b53
        FADDP st(2),st(0) // 00415b57
        FXCH // 00415b59
        FSTP dword ptr [ESP + 0x68] // 00415b5b
        FADD dword ptr [ESP + 0x64] // 00415b5f
        FSTP dword ptr [ESP + 0x6c] // 00415b63
        FLD dword ptr [ESP + 0x68] // 00415b67
        FSTP dword ptr [ESP + 0x30] // 00415b6b
        FLD dword ptr [ESP + 0x6c] // 00415b6f
        FSTP dword ptr [ESP + 0x34] // 00415b73
        JMP L_00415b96 // 00415b77
L_00415b79:
        MOVSS XMM0,dword ptr [EDI] // 00415b79
        FSTP st(3) // 00415b7d
        FSTP st(1) // 00415b7f
        MOVSS dword ptr [ESP + 0x30],XMM0 // 00415b81
        MOVSS XMM0,dword ptr [EDI + 0x4] // 00415b87
        FSTP st(0) // 00415b8c
L_00415b8e:
        MOVSS dword ptr [ESP + 0x34],XMM0 // 00415b8e
        FSTP st(0) // 00415b94
L_00415b96:
        FSUBR dword ptr [ESP + 0x30] // 00415b96
        FSTP dword ptr [ESP + 0x70] // 00415b9a
        FLD dword ptr [ESP + 0x34] // 00415b9e
        FSUB dword ptr [ESP + 0x18] // 00415ba2
        FSTP dword ptr [ESP + 0x74] // 00415ba6
        FLD dword ptr [ESP + 0x74] // 00415baa
        FLD dword ptr [ESP + 0x70] // 00415bae
        FMUL st(0),st(0) // 00415bb2
        FLD st(1) // 00415bb4
        FMULP st(2),st(0) // 00415bb6
        FADDP st(1),st(0) // 00415bb8
        FSTP dword ptr [ESP + 0x2c] // 00415bba
        FLD qword ptr [arc_squared_length_cutoff] // 00415bbe
        FLD dword ptr [ESP + 0x2c] // 00415bc4
        FCOMI st(0),st(1) // 00415bc8
        FSTP st(1) // 00415bca
        JBE L_00415be1 // 00415bcc
        mov ecx, dword ptr [esp + 0x90] // required borrowed CRT; ST0 unspilled
        CALL native_crt_sqrt_st0_00bf7030 // 00415bce
        FSTP dword ptr [ESP + 0x2c] // 00415bd3
        FLD dword ptr [ESP + 0x2c] // 00415bd7
        FSTP dword ptr [ESP + 0x28] // 00415bdb
        JMP L_00415bec // 00415bdf
L_00415be1:
        XORPS XMM0,XMM0 // 00415be1
        FSTP st(0) // 00415be4
        MOVSS dword ptr [ESP + 0x28],XMM0 // 00415be6
L_00415bec:
        FLD dword ptr [ESP + 0x28] // 00415bec
        FLD dword ptr [ESP + 0x9c] // 00415bf0
        FCOMI st(0),st(1) // 00415bf7
        FSTP st(1) // 00415bf9
        JC L_00415a3b // 00415bfb
        LEA EAX,[ESP + 0x80] // 00415c01
        push dword ptr [esp + 0x90] // append helper CRT argument before its original three args
        PUSH EAX // 00415c08
        PUSH EDI // 00415c09
        PUSH ECX // 00415c0a
        MOV EDX,ESI // 00415c0b
        FSTP dword ptr [ESP] // 00415c0d
        MOV ECX,EBP // 00415c10
        CALL circle_segment_kernel // 00415c12
        MOV EBX,EAX // 00415c17
        TEST EBX,EBX // 00415c19
        JLE L_00415a3d // 00415c1b
        FLD dword ptr [ESP + 0xa0] // 00415c21
        MOV ECX,dword ptr [ESP + 0xa4] // 00415c28
        SUB ESP,0x8 // 00415c2f
        FSTP dword ptr [ESP + 0x4] // 00415c32
        FLD dword ptr [ECX] // 00415c36
        FSTP dword ptr [ESP] // 00415c38
        CALL wrapped_angle_subtract_00438b10 // 00415c3b
        add esp, 8 // existing typed wrapped-angle API is cdecl, native callee RET8
        XOR EDI,EDI // 00415c40
        FSTP dword ptr [ESP + 0x18] // 00415c42
        TEST EBX,EBX // 00415c46
        JLE L_00415a3d // 00415c48
        MOV EDI,EDI // 00415c4e
L_00415c50:
        FLD dword ptr [ESP + EDI*0x8 + 0x80] // 00415c50
        FSUB dword ptr [EBP] // 00415c57
        FSTP dword ptr [ESP + 0x78] // 00415c5a
        FLD dword ptr [ESP + EDI*0x8 + 0x84] // 00415c5e
        FSUB dword ptr [EBP + 0x4] // 00415c65
        FSTP dword ptr [ESP + 0x7c] // 00415c68
        FLD dword ptr [ESP + 0x7c] // 00415c6c
        FLD dword ptr [ESP + 0x78] // 00415c70
        CALL _CIatan2 // 00415c74
        FSTP dword ptr [ESP + 0x2c] // 00415c79
        FLD dword ptr [ESP + 0x2c] // 00415c7d
        FSUBR qword ptr [arc_quarter_turn] // 00415c81
        FSTP dword ptr [ESP + 0x14] // 00415c87
        FLD dword ptr [ESP + 0x14] // 00415c8b
        FLDZ // 00415c8f
        FCOMIP st(0),st(1) // 00415c91
        JBE L_00415ca3 // 00415c93
        FADD qword ptr [arc_full_turn] // 00415c95
        FSTP dword ptr [ESP + 0x14] // 00415c9b
        FLD dword ptr [ESP + 0x14] // 00415c9f
L_00415ca3:
        FLD dword ptr [ESP + 0xa0] // 00415ca3
        SUB ESP,0x8 // 00415caa
        FSTP dword ptr [ESP + 0x4] // 00415cad
        FSTP dword ptr [ESP] // 00415cb1
        CALL wrapped_angle_subtract_00438b10 // 00415cb4
        add esp, 8 // existing typed wrapped-angle API is cdecl, native callee RET8
        MOVSS XMM1,dword ptr [ESP + 0x18] // 00415cb9
        FSTP dword ptr [ESP + 0x10] // 00415cbf
        XORPS XMM0,XMM0 // 00415cc3
        COMISS XMM1,XMM0 // 00415cc6
        JBE L_00415d07 // 00415cc9
        MOVSS XMM1,dword ptr [ESP + 0x10] // 00415ccb
        COMISS XMM1,XMM0 // 00415cd1
        JBE L_00415d43 // 00415cd4
        FLD dword ptr [ESP + 0x10] // 00415cd6
        FLD dword ptr [ESP + 0x18] // 00415cda
        FCOMIP st(0),st(1) // 00415cde
        JBE L_00415d41 // 00415ce0
        SUB ESP,0x8 // 00415ce2
        FSTP dword ptr [ESP + 0x4] // 00415ce5
        MOV dword ptr [ESP + 0x24],ESI // 00415ce9
        FLD dword ptr [ESP + 0xa8] // 00415ced
        FSTP dword ptr [ESP] // 00415cf4
        CALL wrapped_angle_add_00438aa0 // 00415cf7
        add esp, 8 // existing typed wrapped-angle API is cdecl, native callee RET8
        MOV EDX,dword ptr [ESP + 0xa4] // 00415cfc
        FSTP dword ptr [EDX] // 00415d03
        JMP L_00415d43 // 00415d05
L_00415d07:
        COMISS XMM0,dword ptr [ESP + 0x10] // 00415d07
        JBE L_00415d43 // 00415d0c
        FLD dword ptr [ESP + 0x18] // 00415d0e
        FLD dword ptr [ESP + 0x10] // 00415d12
        FCOMI st(0),st(1) // 00415d16
        FSTP st(1) // 00415d18
        JBE L_00415d41 // 00415d1a
        SUB ESP,0x8 // 00415d1c
        FSTP dword ptr [ESP + 0x4] // 00415d1f
        MOV dword ptr [ESP + 0x24],ESI // 00415d23
        FLD dword ptr [ESP + 0xa8] // 00415d27
        FSTP dword ptr [ESP] // 00415d2e
        CALL wrapped_angle_add_00438aa0 // 00415d31
        add esp, 8 // existing typed wrapped-angle API is cdecl, native callee RET8
        MOV EAX,dword ptr [ESP + 0xa4] // 00415d36
        FSTP dword ptr [EAX] // 00415d3d
        JMP L_00415d43 // 00415d3f
L_00415d41:
        FSTP st(0) // 00415d41
L_00415d43:
        ADD EDI,0x1 // 00415d43
        CMP EDI,EBX // 00415d46
        JL L_00415c50 // 00415d48
        JMP L_00415a3d // 00415d4e
L_00415d53:
        MOV ESI,dword ptr [ESI + 0x18] // 00415d53
L_00415d56:
        TEST ESI,ESI // 00415d56
        JNZ L_004159a0 // 00415d58
        MOV EAX,dword ptr [ESP + 0x1c] // 00415d5e
        POP EDI // 00415d62
        POP EBP // 00415d63
        POP EBX // 00415d64
L_00415d65:
        POP ESI // 00415d65
        ADD ESP,0x80 // 00415d66
        pop edx // release saved CRT binding
        RET 0x10 // 00415d6c
    }
}
} // namespace

int avoid_zone_circle_segment_004f3ba0(const std::array<float,2>& center,
    const std::array<float,2>& start,float radius,const std::array<float,2>& end,
    std::array<std::array<float,2>,2>& points,const CameraAxesCrtAccess& crt) {
    return circle_segment_kernel(center.data(),start.data(),radius,end.data(),
        points[0].data(),&crt);
}

AvoidZoneSelectedSegment* avoid_zone_selected_segments_arc_00415970(
    AvoidZoneSelectedSegment* head,const std::array<float,2>& center,float radius,
    float start_bearing,float& end_bearing,const CameraAxesCrtAccess& crt) {
    return selected_arc_kernel(&head,&crt,center.data(),radius,start_bearing,&end_bearing);
}
} // namespace bsp
