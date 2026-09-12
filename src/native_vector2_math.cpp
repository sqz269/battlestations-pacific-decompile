#include "bsp/native_vector2_math.hpp"

namespace bsp {
// Instruction schedules retain original x87 stack and binary32 boundaries.
__declspec(naked) float __fastcall clamp_native_float_004155b0(const float*,const float*,const float*) noexcept {
    __asm {
        SUB ESP,0x8 // 004155b0
        FLD dword ptr [ECX] // 004155b3
        FSTP dword ptr [ESP] // 004155b5
        FLD dword ptr [EDX] // 004155b8
        FSTP dword ptr [ESP + 0x4] // 004155ba
        FLD dword ptr [ESP] // 004155be
        FLD dword ptr [ESP + 0x4] // 004155c1
        FCOMI st(0),st(1) // 004155c5
        JA L_004155e7 // 004155c7
        MOV EAX,dword ptr [ESP + 0xc] // 004155c9
        FSTP st(0) // 004155cd
        FLD dword ptr [EAX] // 004155cf
        FSTP dword ptr [ESP + 0xc] // 004155d1
        FLD dword ptr [ESP + 0xc] // 004155d5
        FXCH // 004155d9
        FCOMI st(0),st(1) // 004155db
        JBE L_004155e7 // 004155dd
        FSTP st(0) // 004155df
        ADD ESP,0x8 // 004155e1
        RET 0x4 // 004155e4
L_004155e7:
        FSTP st(1) // 004155e7
        ADD ESP,0x8 // 004155e9
        RET 0x4 // 004155ec
    }
}

__declspec(naked) float __fastcall native_vector2_length_00419210(const float*,const CameraAxesCrtAccess*) {
    __asm {
        push ebx
        mov ebx, edx // added borrowed CRT access
        SUB ESP,0x8 // 00419210
        FLD dword ptr [ECX] // 00419213
        FSTP dword ptr [ESP + 0x4] // 00419215
        FLD dword ptr [ECX + 0x4] // 00419219
        FSTP dword ptr [ESP] // 0041921c
        FLD dword ptr [ESP] // 0041921f
        FLD dword ptr [ESP + 0x4] // 00419222
        FMUL st(0),st(0) // 00419226
        FSTP dword ptr [ESP + 0x4] // 00419228
        FLD dword ptr [ESP + 0x4] // 0041922c
        FLD st(1) // 00419230
        FMULP st(2),st(0) // 00419232
        FXCH // 00419234
        FSTP dword ptr [ESP + 0x4] // 00419236
        FADD dword ptr [ESP + 0x4] // 0041923a
        FSTP dword ptr [ESP + 0x4] // 0041923e
        FLD dword ptr [ESP + 0x4] // 00419242
        mov ecx, ebx
        call native_crt_sqrt_st0_00bf7030 // 00419246
        FSTP dword ptr [ESP + 0x4] // 0041924b
        FLD dword ptr [ESP + 0x4] // 0041924f
        ADD ESP,0x8 // 00419253
        pop ebx // restore added CRT holder
        RET // 00419256
    }
}

__declspec(naked) float __fastcall native_vector2_reciprocal_length_00419260(const float*,const CameraAxesCrtAccess*) {
    __asm {
        push ebx
        mov ebx, edx // added borrowed CRT access
        SUB ESP,0x8 // 00419260
        FLD dword ptr [ECX] // 00419263
        FSTP dword ptr [ESP + 0x4] // 00419265
        FLD dword ptr [ECX + 0x4] // 00419269
        FSTP dword ptr [ESP] // 0041926c
        FLD dword ptr [ESP] // 0041926f
        FLD dword ptr [ESP + 0x4] // 00419272
        FMUL st(0),st(0) // 00419276
        FSTP dword ptr [ESP + 0x4] // 00419278
        FLD dword ptr [ESP + 0x4] // 0041927c
        FLD st(1) // 00419280
        FMULP st(2),st(0) // 00419282
        FXCH // 00419284
        FSTP dword ptr [ESP + 0x4] // 00419286
        FADD dword ptr [ESP + 0x4] // 0041928a
        FSTP dword ptr [ESP + 0x4] // 0041928e
        FLD dword ptr [ESP + 0x4] // 00419292
        mov ecx, ebx
        call native_crt_sqrt_st0_00bf7030 // 00419296
        FSTP dword ptr [ESP + 0x4] // 0041929b
        FLD dword ptr [ESP + 0x4] // 0041929f
        FSTP dword ptr [ESP + 0x4] // 004192a3
        FLD dword ptr [ESP + 0x4] // 004192a7
        FLD st(0) // 004192ab
        FLDZ // 004192ad
        FLD st(0) // 004192af
        FXCH st(2) // 004192b1
        FUCOMIP st(0),st(2) // 004192b3
        FSTP st(1) // 004192b5
        LAHF // 004192b7
        TEST AH,0x44 // 004192b8
        JP L_004192c3 // 004192bb
        FSTP st(1) // 004192bd
        ADD ESP,0x8 // 004192bf
        pop ebx // restore added CRT holder
        RET // 004192c2
L_004192c3:
        FSTP st(0) // 004192c3
        FLD1 // 004192c5
        FDIVRP st(1),st(0) // 004192c7
        FSTP dword ptr [ESP + 0x4] // 004192c9
        FLD dword ptr [ESP + 0x4] // 004192cd
        ADD ESP,0x8 // 004192d1
        pop ebx // restore added CRT holder
        RET // 004192d4
    }
}

__declspec(naked) float* __fastcall native_vector2_min_00b9a7a0(float*,const float*,const float*) noexcept {
    __asm {
        PUSH ECX // 00b9a7a0
        FLD dword ptr [EDX + 0x4] // 00b9a7a1
        PUSH ESI // 00b9a7a4
        MOV ESI,dword ptr [ESP + 0xc] // 00b9a7a5
        FSTP dword ptr [ESP + 0xc] // 00b9a7a9
        FLD dword ptr [ESI + 0x4] // 00b9a7ad
        FSTP dword ptr [ESP + 0x4] // 00b9a7b0
        FLD dword ptr [ESP + 0xc] // 00b9a7b4
        FLD dword ptr [ESP + 0x4] // 00b9a7b8
        FCOMIP st(0),st(1) // 00b9a7bc
        FSTP st(0) // 00b9a7be
        JBE L_00b9a7ca // 00b9a7c0
        MOVSS XMM1,dword ptr [ESP + 0xc] // 00b9a7c2
        JMP L_00b9a7d0 // 00b9a7c8
L_00b9a7ca:
        MOVSS XMM1,dword ptr [ESP + 0x4] // 00b9a7ca
L_00b9a7d0:
        FLD dword ptr [EDX] // 00b9a7d0
        FSTP dword ptr [ESP + 0xc] // 00b9a7d2
        FLD dword ptr [ESI] // 00b9a7d6
        POP ESI // 00b9a7d8
        FSTP dword ptr [ESP] // 00b9a7d9
        FLD dword ptr [ESP + 0x8] // 00b9a7dc
        FLD dword ptr [ESP] // 00b9a7e0
        FCOMIP st(0),st(1) // 00b9a7e3
        FSTP st(0) // 00b9a7e5
        JBE L_00b9a7fe // 00b9a7e7
        MOVSS XMM0,dword ptr [ESP + 0x8] // 00b9a7e9
        MOVSS dword ptr [ECX],XMM0 // 00b9a7ef
        MOVSS dword ptr [ECX + 0x4],XMM1 // 00b9a7f3
        MOV EAX,ECX // 00b9a7f8
        POP ECX // 00b9a7fa
        RET 0x4 // 00b9a7fb
L_00b9a7fe:
        MOVSS XMM0,dword ptr [ESP] // 00b9a7fe
        MOVSS dword ptr [ECX],XMM0 // 00b9a803
        MOVSS dword ptr [ECX + 0x4],XMM1 // 00b9a807
        MOV EAX,ECX // 00b9a80c
        POP ECX // 00b9a80e
        RET 0x4 // 00b9a80f
    }
}

__declspec(naked) float* __fastcall native_vector2_max_00b9a820(float*,const float*,const float*) noexcept {
    __asm {
        PUSH ECX // 00b9a820
        FLD dword ptr [EDX + 0x4] // 00b9a821
        PUSH ESI // 00b9a824
        MOV ESI,dword ptr [ESP + 0xc] // 00b9a825
        FSTP dword ptr [ESP + 0xc] // 00b9a829
        FLD dword ptr [ESI + 0x4] // 00b9a82d
        FSTP dword ptr [ESP + 0x4] // 00b9a830
        FLD dword ptr [ESP + 0x4] // 00b9a834
        FLD dword ptr [ESP + 0xc] // 00b9a838
        FCOMIP st(0),st(1) // 00b9a83c
        FSTP st(0) // 00b9a83e
        JBE L_00b9a84a // 00b9a840
        MOVSS XMM1,dword ptr [ESP + 0xc] // 00b9a842
        JMP L_00b9a850 // 00b9a848
L_00b9a84a:
        MOVSS XMM1,dword ptr [ESP + 0x4] // 00b9a84a
L_00b9a850:
        FLD dword ptr [EDX] // 00b9a850
        FSTP dword ptr [ESP + 0xc] // 00b9a852
        FLD dword ptr [ESI] // 00b9a856
        POP ESI // 00b9a858
        FSTP dword ptr [ESP] // 00b9a859
        FLD dword ptr [ESP] // 00b9a85c
        FLD dword ptr [ESP + 0x8] // 00b9a85f
        FCOMIP st(0),st(1) // 00b9a863
        FSTP st(0) // 00b9a865
        JBE L_00b9a87e // 00b9a867
        MOVSS XMM0,dword ptr [ESP + 0x8] // 00b9a869
        MOVSS dword ptr [ECX],XMM0 // 00b9a86f
        MOVSS dword ptr [ECX + 0x4],XMM1 // 00b9a873
        MOV EAX,ECX // 00b9a878
        POP ECX // 00b9a87a
        RET 0x4 // 00b9a87b
L_00b9a87e:
        MOVSS XMM0,dword ptr [ESP] // 00b9a87e
        MOVSS dword ptr [ECX],XMM0 // 00b9a883
        MOVSS dword ptr [ECX + 0x4],XMM1 // 00b9a887
        MOV EAX,ECX // 00b9a88c
        POP ECX // 00b9a88e
        RET 0x4 // 00b9a88f
    }
}

} // namespace bsp
