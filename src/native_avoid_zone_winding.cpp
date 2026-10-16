#include "bsp/native_avoid_zone_winding.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These complete raw x87 bodies require MSVC Win32.
#endif

// These are actual linked Win32 CRT intrinsic entries. CALL supplies x87
// ST0 (sqrt), or ST0=x/ST1=y (atan2), and receives ST0. No C++ call expression,
// stack-double bridge, substituted implementation or extra FP spill is used.
extern "C" double __cdecl _CIatan2();
extern "C" double __cdecl _CIsqrt();

namespace bsp {
namespace {
// Exact immutable bit payloads verified in live Native memory and installed PE.
alignas(8) constexpr std::uint64_t squared_length_cutoff_00ce3820 = 0x3ddb7cdfd9d7bdbbULL;
alignas(8) constexpr std::uint64_t full_turn_00ce3828 = 0x401921fb60000000ULL;
alignas(8) constexpr std::uint64_t quarter_turn_00ce3830 = 0x3ff921fb60000000ULL;
alignas(8) constexpr std::uint64_t negative_half_turn_00ce3d18 = 0xc00921fb60000000ULL;
alignas(8) constexpr std::uint64_t half_turn_00ce3d28 = 0x400921fb60000000ULL;
constexpr std::uint32_t zero_00d7a218 = 0;
} // namespace

__declspec(naked) bool __fastcall derive_native_avoid_zone_0041a200(void*, void*, std::uint32_t) {
    __asm {
        SUB ESP,0x44 // 0041a200
        XORPS XMM0,XMM0 // 0041a203
        PUSH EBX // 0041a206
        PUSH ESI // 0041a207
        MOV ESI,ECX // 0041a208
        MOV EAX,dword ptr [ESI + 0x4] // 0041a20a
        MOV ECX,dword ptr [ESI] // 0041a20d
        PUSH EDI // 0041a20f
        MOV EDI,dword ptr [ESI] // 0041a210
        LEA EDX,[ECX + EAX*0x4] // 0041a212
        CMP EDI,EDX // 0041a215
        MOVSS dword ptr [ESP + 0x14],XMM0 // 0041a217
        JZ L_0041a4ce // 0041a21d
        PUSH EBP // 0041a223
L_0041a224:
        MOV EAX,dword ptr [ESI + 0x4] // 0041a224
        TEST EAX,EAX // 0041a227
        JNZ L_0041a22f // 0041a229
        MOV EDX,dword ptr [ESI] // 0041a22b
        JMP L_0041a245 // 0041a22d
L_0041a22f:
        CMP EDI,dword ptr [ESI] // 0041a22f
        JNZ L_0041a240 // 0041a231
        MOV ECX,dword ptr [ESI + 0x4] // 0041a233
        MOV EDX,dword ptr [ESI] // 0041a236
        LEA EAX,[EDX + ECX*0x4] // 0041a238
        ADD EAX,-0x4 // 0041a23b
        JMP L_0041a243 // 0041a23e
L_0041a240:
        LEA EAX,[EDI + -0x4] // 0041a240
L_0041a243:
        MOV EDX,EAX // 0041a243
L_0041a245:
        MOV EAX,dword ptr [ESI + 0x4] // 0041a245
        TEST EAX,EAX // 0041a248
        JNZ L_0041a250 // 0041a24a
        MOV ECX,dword ptr [ESI] // 0041a24c
        JMP L_0041a270 // 0041a24e
L_0041a250:
        MOV EAX,dword ptr [ESI] // 0041a250
        MOV ECX,dword ptr [ESI + 0x4] // 0041a252
        LEA ECX,[EAX + ECX*0x4] // 0041a255
        ADD ECX,-0x4 // 0041a258
        CMP EDI,ECX // 0041a25b
        JZ L_0041a26e // 0041a25d
        MOV EAX,dword ptr [ESI] // 0041a25f
        MOV ECX,dword ptr [ESI + 0x4] // 0041a261
        LEA ECX,[EAX + ECX*0x4] // 0041a264
        CMP EDI,ECX // 0041a267
        JZ L_0041a26e // 0041a269
        LEA EAX,[EDI + 0x4] // 0041a26b
L_0041a26e:
        MOV ECX,EAX // 0041a26e
L_0041a270:
        MOV EAX,dword ptr [EDX] // 0041a270
        FLD dword ptr [EAX] // 0041a272
        MOV EBX,dword ptr [EDI] // 0041a274
        FSTP dword ptr [ESP + 0x3c] // 0041a276
        FLD dword ptr [EAX + 0x4] // 0041a27a
        MOV EAX,dword ptr [ECX] // 0041a27d
        MOVSS XMM0,dword ptr [EAX] // 0041a27f
        FSTP dword ptr [ESP + 0x40] // 0041a283
        FLD dword ptr [EBX] // 0041a287
        MOVSS dword ptr [ESP + 0x44],XMM0 // 0041a289
        MOVSS XMM0,dword ptr [EAX + 0x4] // 0041a28f
        FSTP dword ptr [ESP + 0x24] // 0041a294
        FLD dword ptr [EBX + 0x4] // 0041a298
        LEA ECX,[ESP + 0x34] // 0041a29b
        FSTP dword ptr [ESP + 0x28] // 0041a29f
        MOVSS dword ptr [ESP + 0x48],XMM0 // 0041a2a3
        FLD dword ptr [ESP + 0x24] // 0041a2a9
        FSUB dword ptr [ESP + 0x3c] // 0041a2ad
        FSTP dword ptr [ESP + 0x34] // 0041a2b1
        FLD dword ptr [ESP + 0x28] // 0041a2b5
        FSUB dword ptr [ESP + 0x40] // 0041a2b9
        FSTP dword ptr [ESP + 0x38] // 0041a2bd
        CALL reciprocal_length_native_vector2_00419260 // 0041a2c1
        FSTP dword ptr [ESP + 0x14] // 0041a2c6
        FLD dword ptr [ESP + 0x34] // 0041a2ca
        FLD dword ptr [ESP + 0x14] // 0041a2ce
        FLD st(0) // 0041a2d2
        FMULP st(2), st(0) // 0041a2d4
        FXCH // 0041a2d6
        FSTP dword ptr [ESP + 0x2c] // 0041a2d8
        FMUL dword ptr [ESP + 0x38] // 0041a2dc
        FSTP dword ptr [ESP + 0x30] // 0041a2e0
        FLD dword ptr [ESP + 0x44] // 0041a2e4
        FSUB dword ptr [ESP + 0x24] // 0041a2e8
        FSTP dword ptr [ESP + 0x1c] // 0041a2ec
        FLD dword ptr [ESP + 0x48] // 0041a2f0
        FSUB dword ptr [ESP + 0x28] // 0041a2f4
        FSTP dword ptr [ESP + 0x20] // 0041a2f8
        FLD dword ptr [ESP + 0x1c] // 0041a2fc
        FLD dword ptr [ESP + 0x20] // 0041a300
        FMUL st(0), st(0) // 0041a304
        FLD st(1) // 0041a306
        FMULP st(2), st(0) // 0041a308
        FADDP st(1), st(0) // 0041a30a
        FSTP dword ptr [ESP + 0x14] // 0041a30c
        FLD qword ptr [squared_length_cutoff_00ce3820] // 0041a310
        FLD dword ptr [ESP + 0x14] // 0041a316
        FCOMI st(0),st(1) // 0041a31a
        FSTP st(1) // 0041a31c
        JBE L_0041a339 // 0041a31e
        CALL _CIsqrt // 0041a320
        FSTP dword ptr [ESP + 0x14] // 0041a325
        FLD dword ptr [ESP + 0x14] // 0041a329
        FSTP dword ptr [ESP + 0x14] // 0041a32d
        MOVSS XMM0,dword ptr [ESP + 0x14] // 0041a331
        JMP L_0041a33e // 0041a337
L_0041a339:
        XORPS XMM0,XMM0 // 0041a339
        FSTP st(0) // 0041a33c
L_0041a33e:
        MOVSS dword ptr [EBX + 0x18],XMM0 // 0041a33e
        MOV EAX,dword ptr [EDI] // 0041a343
        FLD dword ptr [EAX + 0x18] // 0041a345
        FSTP dword ptr [ESP + 0x14] // 0041a348
        FLD dword ptr [ESP + 0x1c] // 0041a34c
        FLD dword ptr [ESP + 0x14] // 0041a350
        FLD st(0) // 0041a354
        FDIVP st(2),st(0) // 0041a356
        FXCH // 0041a358
        FSTP dword ptr [ESP + 0x1c] // 0041a35a
        FDIVR dword ptr [ESP + 0x20] // 0041a35e
        FSTP dword ptr [ESP + 0x20] // 0041a362
        FLD dword ptr [ESP + 0x1c] // 0041a366
        FSTP dword ptr [EAX + 0x8] // 0041a36a
        FLD dword ptr [ESP + 0x20] // 0041a36d
        FSTP dword ptr [EAX + 0xc] // 0041a371
        FLD dword ptr [ESP + 0x30] // 0041a374
        FLD dword ptr [ESP + 0x2c] // 0041a378
        CALL _CIatan2 // 0041a37c
        FSTP dword ptr [ESP + 0x14] // 0041a381
        FLD dword ptr [ESP + 0x14] // 0041a385
        FSUBR qword ptr [quarter_turn_00ce3830] // 0041a389
        FSTP dword ptr [ESP + 0x14] // 0041a38f
        FLD dword ptr [ESP + 0x14] // 0041a393
        FLDZ // 0041a397
        FCOMIP st(0),st(1) // 0041a399
        JBE L_0041a3a9 // 0041a39b
        FADD qword ptr [full_turn_00ce3828] // 0041a39d
        FSTP dword ptr [ESP + 0x14] // 0041a3a3
        JMP L_0041a3ab // 0041a3a7
L_0041a3a9:
        FSTP st(0) // 0041a3a9
L_0041a3ab:
        FLD dword ptr [ESP + 0x20] // 0041a3ab
        FLD dword ptr [ESP + 0x1c] // 0041a3af
        CALL _CIatan2 // 0041a3b3
        FSTP dword ptr [ESP + 0x10] // 0041a3b8
        FLD dword ptr [ESP + 0x10] // 0041a3bc
        FSUBR qword ptr [quarter_turn_00ce3830] // 0041a3c0
        FSTP dword ptr [ESP + 0x10] // 0041a3c6
        FLD dword ptr [ESP + 0x10] // 0041a3ca
        FLDZ // 0041a3ce
        FCOMIP st(0),st(1) // 0041a3d0
        JBE L_0041a3e2 // 0041a3d2
        FADD qword ptr [full_turn_00ce3828] // 0041a3d4
        FSTP dword ptr [ESP + 0x10] // 0041a3da
        FLD dword ptr [ESP + 0x10] // 0041a3de
L_0041a3e2:
        FLD dword ptr [ESP + 0x14] // 0041a3e2
        SUB ESP,0x8 // 0041a3e6
        FSTP dword ptr [ESP + 0x4] // 0041a3e9
        FSTP dword ptr [ESP] // 0041a3ed
        CALL subtract_native_wrapped_angle_00438b10 // 0041a3f0
        MOV EDX,dword ptr [EDI] // 0041a3f5
        FSTP dword ptr [EDX + 0x1c] // 0041a3f7
        MOV EAX,dword ptr [EDI] // 0041a3fa
        FLD dword ptr [ESP + 0x2c] // 0041a3fc
        FADD dword ptr [ESP + 0x1c] // 0041a400
        FSTP dword ptr [ESP + 0x1c] // 0041a404
        FLD dword ptr [ESP + 0x30] // 0041a408
        FADD dword ptr [ESP + 0x20] // 0041a40c
        FSTP dword ptr [ESP + 0x20] // 0041a410
        FLD dword ptr [ESP + 0x20] // 0041a414
        FSTP dword ptr [EAX + 0x10] // 0041a418
        MOV ECX,dword ptr [EDI] // 0041a41b
        FLD dword ptr [ESP + 0x1c] // 0041a41d
        FCHS // 0041a421
        FSTP dword ptr [ECX + 0x14] // 0041a423
        MOV EBP,dword ptr [EDI] // 0041a426
        LEA EBX,[EBP + 0x10] // 0041a428
        MOV ECX,EBX // 0041a42b
        CALL reciprocal_length_native_vector2_00419260 // 0041a42d
        FSTP dword ptr [ESP + 0x14] // 0041a432
        FLD dword ptr [EBX] // 0041a436
        ADD EDI,0x4 // 0041a438
        FLD dword ptr [ESP + 0x14] // 0041a43b
        FLD st(0) // 0041a43f
        FMULP st(2), st(0) // 0041a441
        FXCH // 0041a443
        FSTP dword ptr [ESP + 0x4c] // 0041a445
        FMUL dword ptr [EBX + 0x4] // 0041a449
        FSTP dword ptr [ESP + 0x50] // 0041a44c
        FLD dword ptr [ESP + 0x4c] // 0041a450
        FSTP dword ptr [EBX] // 0041a454
        FLD dword ptr [ESP + 0x50] // 0041a456
        FSTP dword ptr [EBX + 0x4] // 0041a45a
        MOV EDX,dword ptr [ESI + 0x4] // 0041a45d
        FLD dword ptr [EBP + 0x1c] // 0041a460
        MOV EAX,dword ptr [ESI] // 0041a463
        FADD dword ptr [ESP + 0x18] // 0041a465
        LEA ECX,[EAX + EDX*0x4] // 0041a469
        CMP EDI,ECX // 0041a46c
        FSTP dword ptr [ESP + 0x18] // 0041a46e
        JNZ L_0041a224 // 0041a472
        MOVSS XMM0,dword ptr [ESP + 0x18] // 0041a478
        COMISS XMM0,dword ptr [zero_00d7a218] // 0041a47e
        POP EBP // 0041a485
        JBE L_0041a4ce // 0041a486
        CMP byte ptr [ESP + 0x54],0x0 // 0041a488
        JZ L_0041a4c3 // 0041a48d
        MOV EAX,dword ptr [ESI + 0x4] // 0041a48f
        SUB EAX,0x1 // 0041a492
        XOR ECX,ECX // 0041a495
        TEST EAX,EAX // 0041a497
        JLE L_0041a4ba // 0041a499
        JMP L_0041a4a0 // 0041a49b
        // Unreachable 0041A49D..49F padding, skipped by the real JMP.
        _emit 0x8d
        _emit 0x49
        _emit 0x00
L_0041a4a0:
        MOV EDX,dword ptr [ESI] // 0041a4a0
        MOV EBX,dword ptr [EDX + EAX*0x4] // 0041a4a2
        MOV EDI,dword ptr [EDX + ECX*0x4] // 0041a4a5
        MOV dword ptr [EDX + ECX*0x4],EBX // 0041a4a8
        MOV EDX,dword ptr [ESI] // 0041a4ab
        MOV dword ptr [EDX + EAX*0x4],EDI // 0041a4ad
        ADD ECX,0x1 // 0041a4b0
        SUB EAX,0x1 // 0041a4b3
        CMP EAX,ECX // 0041a4b6
        JG L_0041a4a0 // 0041a4b8
L_0041a4ba:
        PUSH 0x0 // 0041a4ba
        MOV ECX,ESI // 0041a4bc
        CALL derive_native_avoid_zone_0041a200 // 0041a4be
L_0041a4c3:
        POP EDI // 0041a4c3
        POP ESI // 0041a4c4
        XOR AL,AL // 0041a4c5
        POP EBX // 0041a4c7
        ADD ESP,0x44 // 0041a4c8
        RET 0x4 // 0041a4cb
L_0041a4ce:
        POP EDI // 0041a4ce
        POP ESI // 0041a4cf
        MOV AL,0x1 // 0041a4d0
        POP EBX // 0041a4d2
        ADD ESP,0x44 // 0041a4d3
        RET 0x4 // 0041a4d6
    }
}

__declspec(naked) float __fastcall reciprocal_length_native_vector2_00419260(const float*, void*) {
    __asm {
        SUB ESP,0x8 // 00419260
        FLD dword ptr [ECX] // 00419263
        FSTP dword ptr [ESP + 0x4] // 00419265
        FLD dword ptr [ECX + 0x4] // 00419269
        FSTP dword ptr [ESP] // 0041926c
        FLD dword ptr [ESP] // 0041926f
        FLD dword ptr [ESP + 0x4] // 00419272
        FMUL st(0), st(0) // 00419276
        FSTP dword ptr [ESP + 0x4] // 00419278
        FLD dword ptr [ESP + 0x4] // 0041927c
        FLD st(1) // 00419280
        FMULP st(2), st(0) // 00419282
        FXCH // 00419284
        FSTP dword ptr [ESP + 0x4] // 00419286
        FADD dword ptr [ESP + 0x4] // 0041928a
        FSTP dword ptr [ESP + 0x4] // 0041928e
        FLD dword ptr [ESP + 0x4] // 00419292
        CALL _CIsqrt // 00419296
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
        RET // 004192c2
L_004192c3:
        FSTP st(0) // 004192c3
        FLD1 // 004192c5
        FDIVRP st(1), st(0) // 004192c7
        FSTP dword ptr [ESP + 0x4] // 004192c9
        FLD dword ptr [ESP + 0x4] // 004192cd
        ADD ESP,0x8 // 004192d1
        RET // 004192d4
    }
}

__declspec(naked) float __stdcall subtract_native_wrapped_angle_00438b10(float, float) {
    __asm {
        FLD dword ptr [ESP + 0x4] // 00438b10
        FSUB dword ptr [ESP + 0x8] // 00438b14
        FSTP dword ptr [ESP + 0x4] // 00438b18
        FLD dword ptr [ESP + 0x4] // 00438b1c
        FLD qword ptr [negative_half_turn_00ce3d18] // 00438b20
        FCOMI st(0),st(1) // 00438b26
        JC L_00438b4f // 00438b28
        FLD qword ptr [full_turn_00ce3828] // 00438b2a
        JMP L_00438b36 // 00438b30
L_00438b32:
        FXCH // 00438b32
        FXCH st(2) // 00438b34
L_00438b36:
        FADD st(2),st(0) // 00438b36
        FXCH st(2) // 00438b38
        FSTP dword ptr [ESP + 0x4] // 00438b3a
        FLD dword ptr [ESP + 0x4] // 00438b3e
        FXCH // 00438b42
        FCOMI st(0),st(1) // 00438b44
        JNC L_00438b32 // 00438b46
        FSTP st(0) // 00438b48
L_00438b4a:
        FSTP st(1) // 00438b4a
        RET 0x8 // 00438b4c
L_00438b4f:
        FSTP st(0) // 00438b4f
        FLD qword ptr [half_turn_00ce3d28] // 00438b51
        FXCH // 00438b57
        FCOMI st(0),st(1) // 00438b59
        JBE L_00438b4a // 00438b5b
        FLD qword ptr [full_turn_00ce3828] // 00438b5d
        JMP L_00438b67 // 00438b63
L_00438b65:
        FXCH // 00438b65
L_00438b67:
        FSUB st(1),st(0) // 00438b67
        FXCH // 00438b69
        FSTP dword ptr [ESP + 0x4] // 00438b6b
        FLD dword ptr [ESP + 0x4] // 00438b6f
        FCOMI st(0),st(2) // 00438b73
        JA L_00438b65 // 00438b75
        FSTP st(2) // 00438b77
        FSTP st(0) // 00438b79
        RET 0x8 // 00438b7b
    }
}

} // namespace bsp
