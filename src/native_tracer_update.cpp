#include "bsp/native_tracer_update.hpp"
#include "bsp/native_vector2_math.hpp"
#include "bsp/gui_widget_owner.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tracer kernels require MSVC Win32.
#endif

namespace bsp {
namespace {
// Actual immutable image constants; integer bit patterns preserve signed zero.
alignas(8) const std::uint32_t constant_00d7a24c = 0x3f800000u;
alignas(8) const std::uint32_t constant_00ce9bac = 0x3ca3d70au;
alignas(8) const std::uint32_t constant_00d7a2f0 = 0x3dcccccdu;
alignas(8) const std::uint32_t constant_00d7a208 = 0x80000000u;
alignas(8) const std::uint32_t constant_00ce4970 = 0x501502f9u;
alignas(8) const std::uint32_t constant_00ce4adc = 0xd01502f9u;
alignas(8) const std::uint64_t constant_00d7a280 = 0x3fe0000000000000ull;
struct TracerUpdateFrame {
    const CameraAxesCrtAccess* crt;
    NativeTracerTransformAccess* transforms;
    NativeTracerTransformTarget captured;
};
static_assert(sizeof(TracerUpdateFrame) == 16);
static_assert(offsetof(TracerUpdateFrame, crt) == 0);
static_assert(offsetof(TracerUpdateFrame, captured) == 8);

void* __fastcall geometry_bridge(void* actual, void*, std::uint32_t ignored) noexcept {
    return gui_model_geometry_00b74640(
        *reinterpret_cast<const NativeModelTailStorage*>(
            static_cast<const std::byte*>(actual) + 0x174), ignored);
}
void* __fastcall element_bridge(const void* actual, void*, std::int32_t index) noexcept {
    return gui_geometry_element_00b732c0(actual, index);
}
void* __fastcall diffuse_bridge(void* actual, void*, std::uint32_t) noexcept {
    // Complete consumed B179F0 leaf is LEA EAX,[ECX+38]; RET4. The address
    // aliases actual material storage; no lighting wrapper or dirty flag.
    return static_cast<std::byte*>(actual) + 0x38;
}
__declspec(naked) void __fastcall visibility_zero_recurse_bridge(
    void*, void*, float, std::uint32_t) noexcept {
    // Every BAABB0 call supplies recurse0. Fold exactly that B6DA70 branch;
    // full recursive scene visibility remains with the existing node runtime.
    __asm {
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx + 0xac], xmm0
        ret 8
    }
}
NativeTracerTransformTarget* __fastcall capture_transform_impl(
    TracerUpdateFrame* frame, void* actual, std::uint32_t captured_table) noexcept {
    frame->captured = frame->transforms->capture_current_34(actual, captured_table);
    return &frame->captured;
}
__declspec(naked) NativeTracerTransformTarget* __fastcall capture_transform_preserving_xmm(
    TracerUpdateFrame*, void*, std::uint32_t) noexcept {
    // The native operation is an integer current-slot load between SSE stores.
    // Preserve volatile SSE registers across the pure canonical lookup. Its
    // contract also forbids FP-environment effects and application mutations.
    __asm {
        sub esp, 0x60
        movups [esp], xmm0
        movups [esp + 0x10], xmm1
        movups [esp + 0x20], xmm2
        movups [esp + 0x30], xmm3
        movups [esp + 0x40], xmm4
        movups [esp + 0x50], xmm5
        push dword ptr [esp + 0x64]
        call capture_transform_impl
        movups xmm0, [esp]
        movups xmm1, [esp + 0x10]
        movups xmm2, [esp + 0x20]
        movups xmm3, [esp + 0x30]
        movups xmm4, [esp + 0x40]
        movups xmm5, [esp + 0x50]
        add esp, 0x60
        ret 4
    }
}
void __fastcall invoke_captured_transform(void* actual, NativeTracerTransformTarget* target,
    const CameraMatrix& matrix) {
    target->invoke(target->context, actual, matrix);
}


__declspec(naked) void __fastcall bounds_kernel(void*,void*,const float*) noexcept {
    __asm {
        MOV EAX,dword ptr [ESP + 0x4] // 00b74390
        FLD dword ptr [EAX] // 00b74394
        FSTP dword ptr [ECX + 0x8] // 00b74396
        FLD dword ptr [EAX + 0x4] // 00b74399
        FSTP dword ptr [ECX + 0xc] // 00b7439c
        FLD dword ptr [EAX + 0x8] // 00b7439f
        FSTP dword ptr [ECX + 0x10] // 00b743a2
        FLD dword ptr [EAX + 0xc] // 00b743a5
        FSTP dword ptr [ECX + 0x14] // 00b743a8
        MOV EAX,dword ptr [ECX + 0x138] // 00b743ab
        AND EAX,0xffffffcf // 00b743b1
        OR EAX,0xb // 00b743b4
        MOV dword ptr [ECX + 0x138],EAX // 00b743b7
        RET 0x4 // 00b743bd
    }
}

__declspec(naked) void __fastcall alpha_kernel(void*,void*,float) noexcept {
    __asm {
        PUSH ESI // 00ba9960
        MOV ESI,ECX // 00ba9961
        MOV ECX,dword ptr [ESI + 0x1bc] // 00ba9963
        TEST ECX,ECX // 00ba9969
        JZ L_00ba997f // 00ba996b
        PUSH 0x0 // 00ba996d
        call diffuse_bridge // 00ba996f
        MOVSS XMM0,dword ptr [ESP + 0x8] // 00ba9974
        MOVSS dword ptr [EAX + 0xc],XMM0 // 00ba997a
L_00ba997f:
        MOV ECX,dword ptr [ESI + 0x254] // 00ba997f
        TEST ECX,ECX // 00ba9985
        POP ESI // 00ba9987
        JZ L_00ba99b1 // 00ba9988
        PUSH 0x0 // 00ba998a
        PUSH 0x0 // 00ba998c
        call geometry_bridge // 00ba998e
        MOV ECX,EAX // 00ba9993
        call element_bridge // 00ba9995
        MOV EAX,dword ptr [EAX + 0x20] // 00ba999a
        PUSH 0x0 // 00ba999d
        MOV ECX,EAX // 00ba999f
        call diffuse_bridge // 00ba99a1
        MOVSS XMM0,dword ptr [ESP + 0x4] // 00ba99a6
        MOVSS dword ptr [EAX + 0xc],XMM0 // 00ba99ac
L_00ba99b1:
        RET 0x4 // 00ba99b1
    }
}

__declspec(naked) std::uint8_t __fastcall predicate_kernel(void*,const CameraAxesCrtAccess*) {
    __asm {
        push ebx
        mov ebx, edx // added borrowed CRT access
        SUB ESP,0xc // 00baa510
        CMP byte ptr [ECX + 0x201],0x0 // 00baa513
        JZ L_00baa5d5 // 00baa51a
        MOV AL,byte ptr [ECX + 0x200] // 00baa520
        CMP AL,0x1 // 00baa526
        JZ L_00baa5d5 // 00baa528
        CMP byte ptr [ECX + 0x202],0x0 // 00baa52e
        PUSH ESI // 00baa535
        JZ L_00baa553 // 00baa536
        TEST AL,AL // 00baa538
        JNZ L_00baa553 // 00baa53a
        XORPS XMM0,XMM0 // 00baa53c
        COMISS XMM0,dword ptr [ECX + 0x7a8] // 00baa53f
L_00baa546:
        JC L_00baa5ce // 00baa546
        XOR AL,AL // 00baa54c
        POP ESI // 00baa54e
        ADD ESP,0xc // 00baa54f
        pop ebx // restore added CRT holder
        RET // 00baa552
L_00baa553:
        MOV ESI,dword ptr [ECX + 0x1b8] // 00baa553
        TEST ESI,ESI // 00baa559
        JZ L_00baa5ce // 00baa55b
        FLD dword ptr [ESI + 0x10] // 00baa55d
        MOVSS XMM0,dword ptr [constant_00d7a24c] // 00baa560
        FSUB dword ptr [ECX + 0x1c4] // 00baa568
        MOVSS dword ptr [ESP + 0x8],XMM0 // 00baa56e
        XORPS XMM0,XMM0 // 00baa574
        MOVSS dword ptr [ESP + 0xc],XMM0 // 00baa577
        FMUL dword ptr [ECX + 0x1ec] // 00baa57d
        FCHS // 00baa583
        FSTP dword ptr [ESP + 0x4] // 00baa585
        FLD dword ptr [ESP + 0x4] // 00baa589
        mov ecx, ebx
        call native_crt_sqrt_st0_00bf7030 // 00baa58d
        FSTP dword ptr [ESP + 0x4] // 00baa592
        FLD dword ptr [ESP + 0x4] // 00baa596
        LEA EAX,[ESP + 0x8] // 00baa59a
        PUSH EAX // 00baa59e
        FSTP dword ptr [ESP + 0x8] // 00baa59f
        LEA EDX,[ESP + 0x10] // 00baa5a3
        LEA ECX,[ESP + 0x8] // 00baa5a7
        call clamp_native_float_004155b0 // 00baa5ab
        FLD1 // 00baa5b0
        FSUBRP st(1),st(0) // 00baa5b2
        FMUL dword ptr [ESI + 0x24] // 00baa5b4
        FSTP dword ptr [ESP + 0xc] // 00baa5b7
        FLD dword ptr [ESP + 0xc] // 00baa5bb
        FLD dword ptr [constant_00ce9bac] // 00baa5bf
        FCOMIP st(0),st(1) // 00baa5c5
        FSTP st(0) // 00baa5c7
        JMP L_00baa546 // 00baa5c9
L_00baa5ce:
        MOV AL,0x1 // 00baa5ce
        POP ESI // 00baa5d0
        ADD ESP,0xc // 00baa5d1
        pop ebx // restore added CRT holder
        RET // 00baa5d4
L_00baa5d5:
        MOV AL,0x1 // 00baa5d5
        ADD ESP,0xc // 00baa5d7
        pop ebx // restore added CRT holder
        RET // 00baa5da
    }
}

__declspec(naked) void __fastcall append_kernel(void*,void*,const float*,const float*,const CameraAxesCrtAccess*) {
    __asm {
        SUB ESP,0x10 // 00baa670
        PUSH ESI // 00baa673
        MOV ESI,ECX // 00baa674
        MOV EAX,dword ptr [ESI + 0x1ac] // 00baa676
        PUSH EDI // 00baa67c
        LEA EDI,[EAX + EAX*0x2] // 00baa67d
        SHL EDI,0x4 // 00baa680
        ADD EDI,dword ptr [ESI + 0x194] // 00baa683
        MOV ECX,0x1 // 00baa689
        ADD dword ptr [ESI + 0x1f8],ECX // 00baa68e
        MOV dword ptr [ESI + 0x1d0],EAX // 00baa694
        MOV EAX,dword ptr [EDI + 0x2c] // 00baa69a
        XOR EDX,EDX // 00baa69d
        CMP EAX,EDX // 00baa69f
        JZ L_00baa6a6 // 00baa6a1
        MOV dword ptr [EAX + 0x28],EDX // 00baa6a3
L_00baa6a6:
        ADD dword ptr [ESI + 0x1ac],ECX // 00baa6a6
        MOV EAX,dword ptr [ESI + 0x1b8] // 00baa6ac
        MOV dword ptr [EDI + 0x28],EAX // 00baa6b2
        MOV EAX,dword ptr [ESP + 0x20] // 00baa6b5
        PUSH EBX // 00baa6b9
        PUSH EBP // 00baa6ba
        MOV EBP,dword ptr [ESP + 0x24] // 00baa6bb
        MOVSS XMM0,dword ptr [EBP] // 00baa6bf
        MOVSS XMM1,dword ptr [EBP + 0x8] // 00baa6c4
        MOVSS dword ptr [EDI],XMM0 // 00baa6c9
        MOVSS dword ptr [EDI + 0x4],XMM1 // 00baa6cd
        MOVSS XMM0,dword ptr [EAX] // 00baa6d2
        MOVSS XMM1,dword ptr [EAX + 0x8] // 00baa6d6
        MOVSS dword ptr [EDI + 0x8],XMM0 // 00baa6db
        MOVSS dword ptr [EDI + 0xc],XMM1 // 00baa6e0
        FLD dword ptr [ESI + 0x184] // 00baa6e5
        FSTP dword ptr [ESP + 0x24] // 00baa6eb
        XORPS XMM2,XMM2 // 00baa6ef
        FLD dword ptr [ESI + 0x188] // 00baa6f2
        FLD dword ptr [ESP + 0x24] // 00baa6f8
        FLD st(0) // 00baa6fc
        FSUBP st(2),st(0) // 00baa6fe
        FLD dword ptr [ESI + 0x1dc] // 00baa700
        FMULP st(2),st(0) // 00baa706
        FADDP st(1),st(0) // 00baa708
        FSTP dword ptr [ESP + 0x24] // 00baa70a
        FLD dword ptr [ESP + 0x24] // 00baa70e
        FMUL dword ptr [ESI + 0x1e0] // 00baa712
        FSTP dword ptr [EDI + 0x1c] // 00baa718
        FLD dword ptr [ESI + 0x184] // 00baa71b
        FSTP dword ptr [ESP + 0x24] // 00baa721
        FLD dword ptr [ESI + 0x188] // 00baa725
        FLD dword ptr [ESP + 0x24] // 00baa72b
        FLD st(0) // 00baa72f
        FSUBP st(2),st(0) // 00baa731
        FLD dword ptr [ESI + 0x1dc] // 00baa733
        FMULP st(2),st(0) // 00baa739
        FADDP st(1),st(0) // 00baa73b
        FSTP dword ptr [ESP + 0x24] // 00baa73d
        FLD dword ptr [ESP + 0x24] // 00baa741
        FMUL dword ptr [ESI + 0x1e0] // 00baa745
        FSTP dword ptr [EDI + 0x20] // 00baa74b
        FLD dword ptr [ESI + 0x1f4] // 00baa74e
        FMUL dword ptr [ESI + 0x1dc] // 00baa754
        FMUL dword ptr [ESI + 0x208] // 00baa75a
        FSTP dword ptr [ESP + 0x24] // 00baa760
        FLD dword ptr [ESP + 0x24] // 00baa764
        FLDZ // 00baa768
        FCOMIP st(0),st(1) // 00baa76a
        FSTP st(0) // 00baa76c
        JBE L_00baa775 // 00baa76e
        MOVAPS XMM0,XMM2 // 00baa770
        JMP L_00baa78b // 00baa773
L_00baa775:
        MOVSS XMM0,dword ptr [ESP + 0x24] // 00baa775
        MOVSS XMM1,dword ptr [constant_00d7a24c] // 00baa77b
        COMISS XMM0,XMM1 // 00baa783
        JBE L_00baa78b // 00baa786
        MOVAPS XMM0,XMM1 // 00baa788
L_00baa78b:
        MOVSS dword ptr [EDI + 0x24],XMM0 // 00baa78b
        MOV EAX,dword ptr [ESI + 0x1b8] // 00baa790
        CMP EAX,EDX // 00baa796
        JZ L_00baa79d // 00baa798
        MOV dword ptr [EAX + 0x2c],EDI // 00baa79a
L_00baa79d:
        MOV ECX,dword ptr [ESI + 0x1ac] // 00baa79d
        CMP ECX,dword ptr [ESI + 0x198] // 00baa7a3
        MOV dword ptr [ESI + 0x1b8],EDI // 00baa7a9
        JNZ L_00baa7b7 // 00baa7af
        MOV dword ptr [ESI + 0x1ac],EDX // 00baa7b1
L_00baa7b7:
        MOV EBX,dword ptr [EDI + 0x28] // 00baa7b7
        CMP EBX,EDX // 00baa7ba
        JZ L_00baa8bc // 00baa7bc
        FLD dword ptr [EDI] // 00baa7c2
        LEA ECX,[ESP + 0x10] // 00baa7c4
        FSUB dword ptr [EBX] // 00baa7c8
        FSTP dword ptr [ESP + 0x10] // 00baa7ca
        FLD dword ptr [EDI + 0x4] // 00baa7ce
        FSUB dword ptr [EBX + 0x4] // 00baa7d1
        FSTP dword ptr [ESP + 0x14] // 00baa7d4
        mov edx, dword ptr [esp + 0x2c]
        call native_vector2_reciprocal_length_00419260 // 00baa7d8
        FSTP dword ptr [ESP + 0x24] // 00baa7dd
        FLD dword ptr [ESP + 0x24] // 00baa7e1
        FLD st(0) // 00baa7e5
        FMUL dword ptr [ESP + 0x10] // 00baa7e7
        FSTP dword ptr [ESP + 0x18] // 00baa7eb
        FMUL dword ptr [ESP + 0x14] // 00baa7ef
        FSTP dword ptr [ESP + 0x1c] // 00baa7f3
        FLD dword ptr [ESP + 0x18] // 00baa7f7
        FSTP dword ptr [EBX + 0x8] // 00baa7fb
        FLD dword ptr [ESP + 0x1c] // 00baa7fe
        FSTP dword ptr [EBX + 0xc] // 00baa802
        FLD dword ptr [EBP] // 00baa805
        FSTP dword ptr [ESP + 0x18] // 00baa808
        FLD dword ptr [EBP + 0x8] // 00baa80c
        FSTP dword ptr [ESP + 0x1c] // 00baa80f
        FLD dword ptr [ESI + 0x1c8] // 00baa813
        FSUB dword ptr [ESP + 0x18] // 00baa819
        FSTP dword ptr [ESP + 0x10] // 00baa81d
        FLD dword ptr [ESI + 0x1cc] // 00baa821
        FSUB dword ptr [ESP + 0x1c] // 00baa827
        FSTP dword ptr [ESP + 0x24] // 00baa82b
        FLD dword ptr [ESP + 0x10] // 00baa82f
        FLD dword ptr [ESP + 0x24] // 00baa833
        FMUL st(0),st(0) // 00baa837
        FSTP dword ptr [ESP + 0x24] // 00baa839
        FLD dword ptr [ESP + 0x24] // 00baa83d
        FLD st(1) // 00baa841
        FMULP st(2),st(0) // 00baa843
        FXCH // 00baa845
        FSTP dword ptr [ESP + 0x24] // 00baa847
        FADD dword ptr [ESP + 0x24] // 00baa84b
        FSTP dword ptr [ESP + 0x24] // 00baa84f
        FLD dword ptr [ESP + 0x24] // 00baa853
        mov ecx, dword ptr [esp + 0x2c]
        call native_crt_sqrt_st0_00bf7030 // 00baa857
        FSTP dword ptr [ESP + 0x24] // 00baa85c
        FLD dword ptr [ESP + 0x24] // 00baa860
        MOV EDX,dword ptr [EDI + 0x28] // 00baa864
        FSTP dword ptr [EDX + 0x14] // 00baa867
        MOV EBX,dword ptr [EDI + 0x28] // 00baa86a
        FLD dword ptr [EBX] // 00baa86d
        FSUB dword ptr [EDI] // 00baa86f
        FSTP dword ptr [ESP + 0x18] // 00baa871
        FLD dword ptr [EBX + 0x4] // 00baa875
        FSUB dword ptr [EDI + 0x4] // 00baa878
        FSTP dword ptr [ESP + 0x24] // 00baa87b
        FLD dword ptr [ESP + 0x18] // 00baa87f
        FLD dword ptr [ESP + 0x24] // 00baa883
        FMUL st(0),st(0) // 00baa887
        FSTP dword ptr [ESP + 0x24] // 00baa889
        FLD dword ptr [ESP + 0x24] // 00baa88d
        FLD st(1) // 00baa891
        FMULP st(2),st(0) // 00baa893
        FXCH // 00baa895
        FSTP dword ptr [ESP + 0x24] // 00baa897
        FADD dword ptr [ESP + 0x24] // 00baa89b
        FSTP dword ptr [ESP + 0x24] // 00baa89f
        FLD dword ptr [ESP + 0x24] // 00baa8a3
        mov ecx, dword ptr [esp + 0x2c]
        call native_crt_sqrt_st0_00bf7030 // 00baa8a7
        FSTP dword ptr [ESP + 0x24] // 00baa8ac
        FLD dword ptr [ESP + 0x24] // 00baa8b0
        FADD dword ptr [EBX + 0x18] // 00baa8b4
        FSTP dword ptr [EDI + 0x18] // 00baa8b7
        JMP L_00baa8c1 // 00baa8ba
L_00baa8bc:
        MOVSS dword ptr [EDI + 0x18],XMM2 // 00baa8bc
L_00baa8c1:
        MOV ESI,dword ptr [ESI + 0x1b8] // 00baa8c1
        CMP dword ptr [ESI + 0x28],0x0 // 00baa8c7
        POP EBP // 00baa8cb
        POP EBX // 00baa8cc
        JNZ L_00baa8dc // 00baa8cd
        MOVSS XMM0,dword ptr [constant_00d7a2f0] // 00baa8cf
        MOVSS dword ptr [ESI + 0x24],XMM0 // 00baa8d7
L_00baa8dc:
        POP EDI // 00baa8dc
        POP ESI // 00baa8dd
        ADD ESP,0x10 // 00baa8de
        RET 0xc // 00baa8e1
    }
}

__declspec(naked) void __fastcall update_kernel(void*,void*,const CameraMatrix*,float,const float*,const float*,float,float,TracerUpdateFrame*) {
    __asm {
        PUSH EBP // 00baabb0
        MOV EBP,ESP // 00baabb1
        AND ESP,0xfffffff8 // 00baabb3
        SUB ESP,0x7c // 00baabb6
        MOVSS XMM0,dword ptr [EBP + 0x18] // 00baabb9
        FLD dword ptr [EBP + 0xc] // 00baabbe
        PUSH EBX // 00baabc1
        MOV EBX,dword ptr [EBP + 0x14] // 00baabc2
        PUSH ESI // 00baabc5
        MOV ESI,ECX // 00baabc6
        FSUB dword ptr [ESI + 0x1c4] // 00baabc8
        MOVSS dword ptr [ESI + 0x1dc],XMM0 // 00baabce
        MOVSS XMM0,dword ptr [EBP + 0xc] // 00baabd6
        MOVSS dword ptr [ESI + 0x1c4],XMM0 // 00baabdb
        PUSH EDI // 00baabe3
        FSTP dword ptr [ESP + 0x28] // 00baabe4
        MOV EDI,dword ptr [EBP + 0x10] // 00baabe8
        MOVSS XMM0,dword ptr [EDI] // 00baabeb
        MOVSS XMM1,dword ptr [EDI + 0x8] // 00baabef
        MOVSS dword ptr [ESI + 0x234],XMM0 // 00baabf4
        MOVSS dword ptr [ESI + 0x238],XMM1 // 00baabfc
        FLD dword ptr [EBX] // 00baac04
        FSTP dword ptr [ESP + 0x2c] // 00baac06
        LEA ECX,[ESP + 0x2c] // 00baac0a
        FLD dword ptr [EBX + 0x8] // 00baac0e
        FSTP dword ptr [ESP + 0x30] // 00baac11
        mov edx, dword ptr [ebp + 0x20]
        mov edx, dword ptr [edx]
        call native_vector2_reciprocal_length_00419260 // 00baac15
        FSTP dword ptr [ESP + 0x10] // 00baac1a
        FLD dword ptr [ESP + 0x10] // 00baac1e
        FLD st(0) // 00baac22
        FMUL dword ptr [ESP + 0x2c] // 00baac24
        FSTP dword ptr [ESP + 0x38] // 00baac28
        FMUL dword ptr [ESP + 0x30] // 00baac2c
        FSTP dword ptr [ESP + 0x3c] // 00baac30
        FLD dword ptr [ESP + 0x38] // 00baac34
        FSTP dword ptr [ESI + 0x23c] // 00baac38
        FLD dword ptr [ESP + 0x3c] // 00baac3e
        FSTP dword ptr [ESI + 0x240] // 00baac42
        CMP byte ptr [ESI + 0x1c0],0x0 // 00baac48
        JZ L_00baac76 // 00baac4f
        MOV EDX,EBX // 00baac51
        LEA ECX,[ESP + 0x38] // 00baac53
        mov eax, dword ptr [ebp + 0x20]
        push dword ptr [eax]
        call camera_vector_normalize_00419510 // 00baac57
        FLD dword ptr [EAX] // 00baac5c
        FSTP dword ptr [ESI + 0x224] // 00baac5e
        FLD dword ptr [EAX + 0x4] // 00baac64
        FSTP dword ptr [ESI + 0x228] // 00baac67
        FLD dword ptr [EAX + 0x8] // 00baac6d
        FSTP dword ptr [ESI + 0x22c] // 00baac70
L_00baac76:
        MOV ECX,EBX // 00baac76
        mov edx, dword ptr [ebp + 0x20]
        mov edx, dword ptr [edx]
        call camera_vector_length_00419440 // 00baac78
        FSTP dword ptr [ESP + 0x10] // 00baac7d
        FLDZ // 00baac81
        FLD dword ptr [ESP + 0x10] // 00baac83
        FCOMI st(0),st(1) // 00baac87
        FSTP st(1) // 00baac89
        FLD1 // 00baac8b
        JBE L_00baac9b // 00baac8d
        FLD st(0) // 00baac8f
        FDIVRP st(2),st(0) // 00baac91
        FXCH // 00baac93
        FSTP dword ptr [ESP + 0x10] // 00baac95
        JMP L_00baaca6 // 00baac99
L_00baac9b:
        XORPS XMM0,XMM0 // 00baac9b
        FSTP st(1) // 00baac9e
        MOVSS dword ptr [ESP + 0x10],XMM0 // 00baaca0
L_00baaca6:
        FLD dword ptr [EBX] // 00baaca6
        MOV ECX,EBX // 00baaca8
        FLD dword ptr [ESP + 0x10] // 00baacaa
        FLD st(0) // 00baacae
        FMULP st(2),st(0) // 00baacb0
        FXCH // 00baacb2
        FSTP dword ptr [ESP + 0x38] // 00baacb4
        FLD st(0) // 00baacb8
        FMUL dword ptr [EBX + 0x4] // 00baacba
        FSTP dword ptr [ESP + 0x3c] // 00baacbd
        FMUL dword ptr [EBX + 0x8] // 00baacc1
        FSTP dword ptr [ESP + 0x40] // 00baacc4
        FLD dword ptr [ESI + 0x228] // 00baacc8
        FMUL dword ptr [ESP + 0x3c] // 00baacce
        FLD dword ptr [ESI + 0x224] // 00baacd2
        FMUL dword ptr [ESP + 0x38] // 00baacd8
        FADDP st(1),st(0) // 00baacdc
        FLD dword ptr [ESI + 0x22c] // 00baacde
        FMUL dword ptr [ESP + 0x40] // 00baace4
        FADDP st(1),st(0) // 00baace8
        FSTP dword ptr [ESP + 0x10] // 00baacea
        FDIV dword ptr [ESP + 0x10] // 00baacee
        FSTP dword ptr [ESI + 0x230] // 00baacf2
        mov edx, dword ptr [ebp + 0x20]
        mov edx, dword ptr [edx]
        call camera_vector_length_00419440 // 00baacf8
        FSTP dword ptr [ESP + 0x10] // 00baacfd
        FLDZ // 00baad01
        FLD dword ptr [ESP + 0x10] // 00baad03
        FCOMI st(0),st(1) // 00baad07
        JBE L_00baad15 // 00baad09
        FLD1 // 00baad0b
        FDIVRP st(1),st(0) // 00baad0d
        FSTP dword ptr [ESP + 0x10] // 00baad0f
        JMP L_00baad20 // 00baad13
L_00baad15:
        XORPS XMM0,XMM0 // 00baad15
        FSTP st(0) // 00baad18
        MOVSS dword ptr [ESP + 0x10],XMM0 // 00baad1a
L_00baad20:
        FLD dword ptr [EBX] // 00baad20
        FLD dword ptr [ESP + 0x10] // 00baad22
        FLD st(0) // 00baad26
        FMULP st(2),st(0) // 00baad28
        FXCH // 00baad2a
        FSTP dword ptr [ESP + 0x38] // 00baad2c
        FLD st(0) // 00baad30
        FMUL dword ptr [EBX + 0x4] // 00baad32
        FSTP dword ptr [ESP + 0x3c] // 00baad35
        FMUL dword ptr [EBX + 0x8] // 00baad39
        FSTP dword ptr [ESP + 0x40] // 00baad3c
        FLD dword ptr [ESP + 0x38] // 00baad40
        FSTP dword ptr [ESI + 0x224] // 00baad44
        FLD dword ptr [ESP + 0x3c] // 00baad4a
        FSTP dword ptr [ESI + 0x228] // 00baad4e
        FLD dword ptr [ESP + 0x40] // 00baad54
        FSTP dword ptr [ESI + 0x22c] // 00baad58
        MOV ECX,dword ptr [ESI + 0x254] // 00baad5e
        TEST ECX,ECX // 00baad64
        JZ L_00baad75 // 00baad66
        PUSH 0x0 // 00baad68
        PUSH ECX // 00baad6a
        FSTP dword ptr [ESP] // 00baad6b
        call visibility_zero_recurse_bridge // 00baad6e
        JMP L_00baad77 // 00baad73
L_00baad75:
        FSTP st(0) // 00baad75
L_00baad77:
        CMP byte ptr [ESI + 0x200],0x0 // 00baad77
        JZ L_00bab2ce // 00baad7e
        MOV EAX,dword ptr [ESI + 0x1b8] // 00baad84
        TEST EAX,EAX // 00baad8a
        JZ L_00baafc7 // 00baad8c
        CMP byte ptr [ESI + 0x202],0x0 // 00baad92
        FLD dword ptr [EDI] // 00baad99
        JZ L_00baaf19 // 00baad9b
        FSTP dword ptr [ESP + 0x38] // 00baada1
        LEA ECX,[ESP + 0x2c] // 00baada5
        FLD dword ptr [EDI + 0x8] // 00baada9
        FSTP dword ptr [ESP + 0x3c] // 00baadac
        FLD dword ptr [ESP + 0x38] // 00baadb0
        FSUB dword ptr [ESI + 0x1c8] // 00baadb4
        FSTP dword ptr [ESP + 0x2c] // 00baadba
        FLD dword ptr [ESP + 0x3c] // 00baadbe
        FSUB dword ptr [ESI + 0x1cc] // 00baadc2
        FSTP dword ptr [ESP + 0x30] // 00baadc8
        mov edx, dword ptr [ebp + 0x20]
        mov edx, dword ptr [edx]
        call native_vector2_length_00419210 // 00baadcc
        FMUL dword ptr [ESI + 0x1f0] // 00baadd1
        LEA EDX,[ESP + 0x38] // 00baadd7
        LEA ECX,[ESP + 0x18] // 00baaddb
        FADD dword ptr [ESI + 0x204] // 00baaddf
        FSTP dword ptr [ESI + 0x204] // 00baade5
        MOVSS XMM0,dword ptr [EDI] // 00baadeb
        MOVSS XMM1,dword ptr [EDI + 0x8] // 00baadef
        MOVSS dword ptr [ESI + 0x1c8],XMM0 // 00baadf4
        MOVSS dword ptr [ESI + 0x1cc],XMM1 // 00baadfc
        MOVSS XMM1,dword ptr [EBX + 0x8] // 00baae04
        MOVSS XMM0,dword ptr [EBX] // 00baae09
        MOVSS dword ptr [ESP + 0x40],XMM1 // 00baae0d
        MOVSS XMM1,dword ptr [EDI] // 00baae13
        MOVSS dword ptr [ESP + 0x2c],XMM1 // 00baae17
        MOVSS XMM1,dword ptr [EDI + 0x8] // 00baae1d
        MOVSS dword ptr [ESP + 0x38],XMM0 // 00baae22
        XORPS XMM0,XMM0 // 00baae28
        MOVSS dword ptr [ESP + 0x34],XMM1 // 00baae2b
        MOVSS XMM1,dword ptr [constant_00d7a24c] // 00baae31
        MOVSS dword ptr [ESP + 0x3c],XMM0 // 00baae39
        MOVSS dword ptr [ESP + 0x48],XMM1 // 00baae3f
        MOVSS dword ptr [ESP + 0x4c],XMM0 // 00baae45
        MOVSS dword ptr [ESP + 0x50],XMM0 // 00baae4b
        MOVSS dword ptr [ESP + 0x54],XMM0 // 00baae51
        MOVSS dword ptr [ESP + 0x58],XMM0 // 00baae57
        MOVSS dword ptr [ESP + 0x5c],XMM1 // 00baae5d
        MOVSS dword ptr [ESP + 0x60],XMM0 // 00baae63
        MOVSS dword ptr [ESP + 0x64],XMM0 // 00baae69
        MOVSS dword ptr [ESP + 0x68],XMM0 // 00baae6f
        MOVSS dword ptr [ESP + 0x6c],XMM0 // 00baae75
        MOVSS dword ptr [ESP + 0x70],XMM1 // 00baae7b
        MOVSS dword ptr [ESP + 0x74],XMM0 // 00baae81
        MOVSS dword ptr [ESP + 0x78],XMM0 // 00baae87
        MOVSS dword ptr [ESP + 0x7c],XMM0 // 00baae8d
        MOVSS dword ptr [ESP + 0x80],XMM0 // 00baae93
        MOVSS dword ptr [ESP + 0x84],XMM1 // 00baae9c
        mov eax, dword ptr [ebp + 0x20]
        push dword ptr [eax]
        call camera_vector_normalize_00419510 // 00baaea5
        MOVSS XMM1,dword ptr [EAX + 0x8] // 00baaeaa
        MOVSS XMM0,dword ptr [EAX] // 00baaeaf
        MOVSS XMM2,dword ptr [EAX + 0x4] // 00baaeb3
        MOV EAX,dword ptr [ESI] // 00baaeb8
        push eax // native table identity captured at BAAEB8
        mov ecx, dword ptr [ebp + 0x20]
        mov edx, esi
        call capture_transform_preserving_xmm
        mov edx, eax // captured target, not an image vtable dereference
        MOVSS dword ptr [ESP + 0x60],XMM2 // 00baaebd
        MOVSS XMM2,dword ptr [ESP + 0x34] // 00baaec3
        MOVSS dword ptr [ESP + 0x80],XMM2 // 00baaec9
        MOVSS XMM2,dword ptr [constant_00d7a208] // 00baaed2
        LEA ECX,[ESP + 0x48] // 00baaeda
        SUBSS XMM2,XMM1 // 00baaede
        MOVSS dword ptr [ESP + 0x50],XMM0 // 00baaee2
        MOVSS dword ptr [ESP + 0x70],XMM1 // 00baaee8
        XORPS XMM1,XMM1 // 00baaeee
        MOVSS dword ptr [ESP + 0x68],XMM0 // 00baaef1
        MOVSS XMM0,dword ptr [ESP + 0x2c] // 00baaef7
        PUSH ECX // 00baaefd
        MOV ECX,ESI // 00baaefe
        MOVSS dword ptr [ESP + 0x4c],XMM2 // 00baaf00
        MOVSS dword ptr [ESP + 0x5c],XMM1 // 00baaf06
        MOVSS dword ptr [ESP + 0x7c],XMM0 // 00baaf0c
        call invoke_captured_transform // native current34 call at00BAAF12
        JMP L_00baafc7 // 00baaf14
L_00baaf19:
        FSTP dword ptr [ESP + 0x2c] // 00baaf19
        LEA ECX,[ESP + 0x38] // 00baaf1d
        FLD dword ptr [EDI + 0x8] // 00baaf21
        FSTP dword ptr [ESP + 0x30] // 00baaf24
        FLD dword ptr [ESP + 0x2c] // 00baaf28
        FSUB dword ptr [EAX] // 00baaf2c
        FSTP dword ptr [ESP + 0x38] // 00baaf2e
        FLD dword ptr [ESP + 0x30] // 00baaf32
        FSUB dword ptr [EAX + 0x4] // 00baaf36
        FSTP dword ptr [ESP + 0x3c] // 00baaf39
        mov edx, dword ptr [ebp + 0x20]
        mov edx, dword ptr [edx]
        call native_vector2_reciprocal_length_00419260 // 00baaf3d
        FSTP dword ptr [ESP + 0x10] // 00baaf42
        MOV EAX,dword ptr [ESI + 0x1b8] // 00baaf46
        FLD dword ptr [ESP + 0x38] // 00baaf4c
        LEA ECX,[ESP + 0x38] // 00baaf50
        FLD dword ptr [ESP + 0x10] // 00baaf54
        FLD st(0) // 00baaf58
        FMULP st(2),st(0) // 00baaf5a
        FXCH // 00baaf5c
        FSTP dword ptr [ESP + 0x10] // 00baaf5e
        FMUL dword ptr [ESP + 0x3c] // 00baaf62
        FSTP dword ptr [ESP + 0x14] // 00baaf66
        FLD dword ptr [ESP + 0x10] // 00baaf6a
        FSTP dword ptr [EAX + 0x8] // 00baaf6e
        FLD dword ptr [ESP + 0x14] // 00baaf71
        FSTP dword ptr [EAX + 0xc] // 00baaf75
        MOV EAX,dword ptr [ESI + 0x1b8] // 00baaf78
        FLD dword ptr [ESP + 0x2c] // 00baaf7e
        FSUB dword ptr [EAX] // 00baaf82
        FSTP dword ptr [ESP + 0x38] // 00baaf84
        FLD dword ptr [ESP + 0x30] // 00baaf88
        FSUB dword ptr [EAX + 0x4] // 00baaf8c
        FSTP dword ptr [ESP + 0x3c] // 00baaf8f
        mov edx, dword ptr [ebp + 0x20]
        mov edx, dword ptr [edx]
        call native_vector2_length_00419210 // 00baaf93
        MOV EAX,dword ptr [ESI + 0x1b8] // 00baaf98
        FSTP dword ptr [EAX + 0x14] // 00baaf9e
        MOV EAX,dword ptr [ESI + 0x1b8] // 00baafa1
        MOV ECX,dword ptr [EAX + 0x28] // 00baafa7
        TEST ECX,ECX // 00baafaa
        JZ L_00baafb7 // 00baafac
        FLD dword ptr [ECX + 0x18] // 00baafae
        FADD dword ptr [EAX + 0x14] // 00baafb1
        FSTP dword ptr [EAX + 0x18] // 00baafb4
L_00baafb7:
        MOV EAX,dword ptr [ESI + 0x1b8] // 00baafb7
        MOVSS XMM0,dword ptr [EBP + 0xc] // 00baafbd
        MOVSS dword ptr [EAX + 0x10],XMM0 // 00baafc2
L_00baafc7:
        CMP byte ptr [ESI + 0x1c0],0x0 // 00baafc7
        JZ L_00bab078 // 00baafce
        CMP byte ptr [ESI + 0x202],0x0 // 00baafd4
        JNZ L_00bab017 // 00baafdb
        MOVSS XMM0,dword ptr [EDI] // 00baafdd
        MOVSS dword ptr [ESP + 0x38],XMM0 // 00baafe1
        MOVSS XMM0,dword ptr [EDI + 0x4] // 00baafe7
        MOVSS dword ptr [ESP + 0x3c],XMM0 // 00baafec
        MOVSS XMM0,dword ptr [EDI + 0x8] // 00baaff2
        LEA ECX,[ESP + 0x38] // 00baaff7
        MOVSS dword ptr [ESP + 0x40],XMM0 // 00baaffb
        MOVSS XMM0,dword ptr [ESI + 0x1d8] // 00bab001
        PUSH ECX // 00bab009
        MOV ECX,ESI // 00bab00a
        MOVSS dword ptr [ESP + 0x48],XMM0 // 00bab00c
        call bounds_kernel // 00bab012
L_00bab017:
        MOVSS XMM0,dword ptr [EBP + 0xc] // 00bab017
        MOVSS dword ptr [ESI + 0x20c],XMM0 // 00bab01c
        MOVSS XMM0,dword ptr [EDI] // 00bab024
        MOVSS XMM1,dword ptr [EDI + 0x8] // 00bab028
        MOV EDX,EBX // 00bab02d
        LEA ECX,[ESP + 0x38] // 00bab02f
        MOVSS dword ptr [ESI + 0x1c8],XMM0 // 00bab033
        MOVSS dword ptr [ESI + 0x1cc],XMM1 // 00bab03b
        mov eax, dword ptr [ebp + 0x20]
        push dword ptr [eax]
        call camera_vector_normalize_00419510 // 00bab043
        mov edx, dword ptr [ebp + 0x20]
        push dword ptr [edx] // added actual CRT binding, consumed by append kernel
        PUSH EAX // 00bab048
        PUSH EDI // 00bab049
        MOV ECX,ESI // 00bab04a
        call append_kernel // 00bab04c
        XORPS XMM0,XMM0 // 00bab051
        MOV byte ptr [ESI + 0x1c0],0x0 // 00bab054
        MOVSS dword ptr [ESI + 0x218],XMM0 // 00bab05b
        MOVSS dword ptr [ESI + 0x21c],XMM0 // 00bab063
        MOVSS dword ptr [ESI + 0x220],XMM0 // 00bab06b
        JMP L_00bab2ce // 00bab073
L_00bab078:
        CMP byte ptr [ESI + 0x202],0x0 // 00bab078
        JNZ L_00bab325 // 00bab07f
        FLD dword ptr [ESI + 0x218] // 00bab085
        LEA ECX,[ESP + 0x38] // 00bab08b
        FSUB dword ptr [EDI] // 00bab08f
        FSTP dword ptr [ESP + 0x38] // 00bab091
        FLD dword ptr [ESI + 0x21c] // 00bab095
        FSUB dword ptr [EDI + 0x4] // 00bab09b
        FSTP dword ptr [ESP + 0x3c] // 00bab09e
        FLD dword ptr [ESI + 0x220] // 00bab0a2
        FSUB dword ptr [EDI + 0x8] // 00bab0a8
        FSTP dword ptr [ESP + 0x40] // 00bab0ab
        mov edx, dword ptr [ebp + 0x20]
        mov edx, dword ptr [edx]
        call camera_vector_length_00419440 // 00bab0af
        FADD dword ptr [ESI + 0x214] // 00bab0b4
        LEA ECX,[ESP + 0x2c] // 00bab0ba
        FSTP dword ptr [ESI + 0x214] // 00bab0be
        FLD dword ptr [EDI] // 00bab0c4
        FSTP dword ptr [ESI + 0x218] // 00bab0c6
        FLD dword ptr [EDI + 0x4] // 00bab0cc
        FSTP dword ptr [ESI + 0x21c] // 00bab0cf
        FLD dword ptr [EDI + 0x8] // 00bab0d5
        FSTP dword ptr [ESI + 0x220] // 00bab0d8
        MOVSS XMM0,dword ptr [EDI] // 00bab0de
        FLD dword ptr [EDI + 0x8] // 00bab0e2
        MOVSS dword ptr [ESP + 0x38],XMM0 // 00bab0e5
        FSTP dword ptr [ESP + 0x3c] // 00bab0eb
        MOVSS dword ptr [ESP + 0x18],XMM0 // 00bab0ef
        FLD dword ptr [ESP + 0x38] // 00bab0f5
        FSUB dword ptr [ESI + 0x1c8] // 00bab0f9
        FSTP dword ptr [ESP + 0x2c] // 00bab0ff
        FLD dword ptr [ESP + 0x3c] // 00bab103
        FSUB dword ptr [ESI + 0x1cc] // 00bab107
        FSTP dword ptr [ESP + 0x30] // 00bab10d
        mov edx, dword ptr [ebp + 0x20]
        mov edx, dword ptr [edx]
        call native_vector2_length_00419210 // 00bab111
        FLD dword ptr [ESI + 0x1d8] // 00bab116
        FXCH // 00bab11c
        FCOMIP st(0),st(1) // 00bab11e
        FSTP st(0) // 00bab120
        JC L_00bab2ce // 00bab122
        MOVSS XMM0,dword ptr [ESP + 0x18] // 00bab128
        MOVSS dword ptr [ESP + 0x10],XMM0 // 00bab12e
        MOVSS XMM0,dword ptr [EDI + 0x8] // 00bab134
        MOV EDX,EBX // 00bab139
        LEA ECX,[ESP + 0x38] // 00bab13b
        MOVSS dword ptr [ESP + 0x14],XMM0 // 00bab13f
        mov eax, dword ptr [ebp + 0x20]
        push dword ptr [eax]
        call camera_vector_normalize_00419510 // 00bab145
        mov edx, dword ptr [ebp + 0x20]
        push dword ptr [edx] // added actual CRT binding, consumed by append kernel
        PUSH EAX // 00bab14a
        PUSH EDI // 00bab14b
        MOV ECX,ESI // 00bab14c
        call append_kernel // 00bab14e
        FLD dword ptr [EBX] // 00bab153
        FSTP dword ptr [ESP + 0x38] // 00bab155
        LEA ECX,[ESP + 0x38] // 00bab159
        FLD dword ptr [EBX + 0x8] // 00bab15d
        FSTP dword ptr [ESP + 0x3c] // 00bab160
        mov edx, dword ptr [ebp + 0x20]
        mov edx, dword ptr [edx]
        call native_vector2_reciprocal_length_00419260 // 00bab164
        MOV EAX,dword ptr [ESI + 0x1b8] // 00bab169
        FSTP dword ptr [ESP + 0x18] // 00bab16f
        FLD dword ptr [ESP + 0x18] // 00bab173
        XORPS XMM0,XMM0 // 00bab177
        FLD st(0) // 00bab17a
        ADD EAX,0x8 // 00bab17c
        FMUL dword ptr [ESP + 0x38] // 00bab17f
        FSTP dword ptr [ESP + 0x2c] // 00bab183
        FMUL dword ptr [ESP + 0x3c] // 00bab187
        FSTP dword ptr [ESP + 0x30] // 00bab18b
        FLD dword ptr [ESP + 0x2c] // 00bab18f
        FSTP dword ptr [EAX] // 00bab193
        FLD dword ptr [ESP + 0x30] // 00bab195
        FSTP dword ptr [EAX + 0x4] // 00bab199
        MOV EDX,dword ptr [ESI + 0x1b8] // 00bab19c
        MOVSS dword ptr [EDX + 0x14],XMM0 // 00bab1a2
        MOV EAX,dword ptr [ESI + 0x1b8] // 00bab1a7
        MOVSS XMM0,dword ptr [EBP + 0xc] // 00bab1ad
        MOVSS dword ptr [EAX + 0x10],XMM0 // 00bab1b2
        MOVSS XMM0,dword ptr [ESP + 0x10] // 00bab1b7
        MOVSS dword ptr [ESI + 0x1c8],XMM0 // 00bab1bd
        MOVSS XMM0,dword ptr [ESP + 0x14] // 00bab1c5
        MOVSS dword ptr [ESI + 0x1cc],XMM0 // 00bab1cb
        MOVSS XMM0,dword ptr [constant_00ce4970] // 00bab1d3
        MOV EDI,dword ptr [ESI + 0x1b8] // 00bab1db
        TEST EDI,EDI // 00bab1e1
        MOVSS dword ptr [ESP + 0x18],XMM0 // 00bab1e3
        MOVSS dword ptr [ESP + 0x1c],XMM0 // 00bab1e9
        MOVSS XMM0,dword ptr [constant_00ce4adc] // 00bab1ef
        MOVSS dword ptr [ESP + 0x10],XMM0 // 00bab1f7
        MOVSS dword ptr [ESP + 0x14],XMM0 // 00bab1fd
        JZ L_00bab252 // 00bab203
L_00bab205:
        PUSH EDI // 00bab205
        LEA EDX,[ESP + 0x1c] // 00bab206
        LEA ECX,[ESP + 0x3c] // 00bab20a
        call native_vector2_min_00b9a7a0 // 00bab20e
        MOVSS XMM0,dword ptr [EAX] // 00bab213
        MOVSS dword ptr [ESP + 0x18],XMM0 // 00bab217
        MOVSS XMM0,dword ptr [EAX + 0x4] // 00bab21d
        PUSH EDI // 00bab222
        LEA EDX,[ESP + 0x14] // 00bab223
        LEA ECX,[ESP + 0x30] // 00bab227
        MOVSS dword ptr [ESP + 0x20],XMM0 // 00bab22b
        call native_vector2_max_00b9a820 // 00bab231
        MOVSS XMM0,dword ptr [EAX] // 00bab236
        MOV EDI,dword ptr [EDI + 0x28] // 00bab23a
        TEST EDI,EDI // 00bab23d
        MOVSS dword ptr [ESP + 0x10],XMM0 // 00bab23f
        MOVSS XMM0,dword ptr [EAX + 0x4] // 00bab245
        MOVSS dword ptr [ESP + 0x14],XMM0 // 00bab24a
        JNZ L_00bab205 // 00bab250
L_00bab252:
        FLD dword ptr [ESP + 0x18] // 00bab252
        XORPS XMM0,XMM0 // 00bab256
        FLD st(0) // 00bab259
        LEA ECX,[ESP + 0x38] // 00bab25b
        FLD dword ptr [ESP + 0x10] // 00bab25f
        FLD st(0) // 00bab263
        FADDP st(2),st(0) // 00bab265
        FLD qword ptr [constant_00d7a280] // 00bab267
        FMUL st(2),st(0) // 00bab26d: DC CA writes ST2, not ST0
        FXCH st(2) // 00bab26f
        FSTP dword ptr [ESP + 0x2c] // 00bab271
        FLD dword ptr [ESP + 0x1c] // 00bab275
        MOVSS dword ptr [ESP + 0x1c],XMM0 // 00bab279
        FLD st(0) // 00bab27f
        FLD dword ptr [ESP + 0x14] // 00bab281
        FLD st(0) // 00bab285
        FADDP st(2),st(0) // 00bab287
        FXCH // 00bab289
        FMULP st(4),st(0) // 00bab28b
        FXCH st(3) // 00bab28d
        FSTP dword ptr [ESP + 0x34] // 00bab28f
        FXCH // 00bab293
        FSUBRP st(3),st(0) // 00bab295
        FXCH st(2) // 00bab297
        FSTP dword ptr [ESP + 0x38] // 00bab299
        FSUBRP st(1),st(0) // 00bab29d
        FSTP dword ptr [ESP + 0x3c] // 00bab29f
        FLD dword ptr [ESP + 0x2c] // 00bab2a3
        FSTP dword ptr [ESP + 0x18] // 00bab2a7
        FLD dword ptr [ESP + 0x34] // 00bab2ab
        FSTP dword ptr [ESP + 0x20] // 00bab2af
        mov edx, dword ptr [ebp + 0x20]
        mov edx, dword ptr [edx]
        call native_vector2_length_00419210 // 00bab2b3
        FMUL qword ptr [constant_00d7a280] // 00bab2b8
        LEA ECX,[ESP + 0x18] // 00bab2be
        PUSH ECX // 00bab2c2
        MOV ECX,ESI // 00bab2c3
        FSTP dword ptr [ESP + 0x28] // 00bab2c5
        call bounds_kernel // 00bab2c9
L_00bab2ce:
        CMP byte ptr [ESI + 0x202],0x0 // 00bab2ce
        JNZ L_00bab325 // 00bab2d5
        MOV EDI,dword ptr [ESI + 0x1f8] // 00bab2d7
        PUSH 0x0 // 00bab2dd
        PUSH 0x0 // 00bab2df
        MOV ECX,ESI // 00bab2e1
        call geometry_bridge // 00bab2e3
        MOV ECX,EAX // 00bab2e8
        call element_bridge // 00bab2ea
        MOV EDX,dword ptr [ESI + 0x190] // 00bab2ef
        MOV ECX,dword ptr [EDX + 0x10] // 00bab2f5
        SUB ECX,0x1 // 00bab2f8
        ADD EDI,-0x2 // 00bab2fb
        JNS L_00bab304 // 00bab2fe
        XOR ECX,ECX // 00bab300
        JMP L_00bab30a // 00bab302
L_00bab304:
        CMP EDI,ECX // 00bab304
        JG L_00bab30a // 00bab306
        MOV ECX,EDI // 00bab308
L_00bab30a:
        MOV EDI,dword ptr [EDX + 0x24] // 00bab30a
        MOV EDX,dword ptr [EDX + 0x18] // 00bab30d
        MOV EDI,dword ptr [EDI + ECX*0x4] // 00bab310
        MOV ECX,dword ptr [EDX + ECX*0x4] // 00bab313
        MOV dword ptr [EAX + 0x10],ECX // 00bab316
        MOV dword ptr [EAX + 0x18],EDI // 00bab319
        CMP dword ptr [ESI + 0x1f8],0x2 // 00bab31c
        JC L_00bab32c // 00bab323
L_00bab325:
        MOV byte ptr [ESI + 0x201],0x1 // 00bab325
L_00bab32c:
        FLD dword ptr [EBP + 0xc] // 00bab32c
        FSUB dword ptr [ESI + 0x20c] // 00bab32f
        FMUL dword ptr [ESI + 0x210] // 00bab335
        FSTP dword ptr [ESP + 0x10] // 00bab33b
        FLD1 // 00bab33f
        FLD dword ptr [ESP + 0x10] // 00bab341
        FCOMIP st(0),st(1) // 00bab345
        FSTP st(0) // 00bab347
        JBE L_00bab355 // 00bab349
        MOVSS XMM0,dword ptr [constant_00d7a24c] // 00bab34b
        JMP L_00bab35b // 00bab353
L_00bab355:
        MOVSS XMM0,dword ptr [ESP + 0x10] // 00bab355
L_00bab35b:
        PUSH 0x0 // 00bab35b
        PUSH 0x0 // 00bab35d
        MOV ECX,ESI // 00bab35f
        MOVSS dword ptr [ESI + 0x208],XMM0 // 00bab361
        call geometry_bridge // 00bab369
        MOV ECX,EAX // 00bab36e
        call element_bridge // 00bab370
        XOR EDX,EDX // 00bab375
        CMP dword ptr [EAX + 0x10],EDX // 00bab377
        PUSH 0x0 // 00bab37a
        SETNZ DL // 00bab37c
        PUSH ECX // 00bab37f
        MOV ECX,ESI // 00bab380
        CVTSI2SS XMM0,EDX // 00bab382
        MOVSS dword ptr [ESP],XMM0 // 00bab386
        call visibility_zero_recurse_bridge // 00bab38b
        CMP byte ptr [ESI + 0x202],0x0 // 00bab390
        JZ L_00bab3de // 00bab397
        CMP byte ptr [ESI + 0x200],0x0 // 00bab399
        JNZ L_00bab3c5 // 00bab3a0
        FLD dword ptr [ESI + 0x7a8] // 00bab3a2
        PUSH ECX // 00bab3a8
        FSUB dword ptr [ESP + 0x2c] // 00bab3a9
        MOV ECX,ESI // 00bab3ad
        FSTP dword ptr [ESP + 0x2c] // 00bab3af
        FLD dword ptr [ESP + 0x2c] // 00bab3b3
        FST dword ptr [ESI + 0x7a8] // 00bab3b7
        FSTP dword ptr [ESP] // 00bab3bd
        call alpha_kernel // 00bab3c0
L_00bab3c5:
        MOV ESI,dword ptr [ESI + 0x254] // 00bab3c5
        TEST ESI,ESI // 00bab3cb
        JZ L_00bab3de // 00bab3cd
        FLDZ // 00bab3cf
        PUSH 0x0 // 00bab3d1
        PUSH ECX // 00bab3d3
        FSTP dword ptr [ESP] // 00bab3d4
        MOV ECX,ESI // 00bab3d7
        call visibility_zero_recurse_bridge // 00bab3d9
L_00bab3de:
        POP EDI // 00bab3de
        POP ESI // 00bab3df
        POP EBX // 00bab3e0
        MOV ESP,EBP // 00bab3e1
        POP EBP // 00bab3e3
        RET 0x1c // 00bab3e4
    }
}

} // namespace

std::uint8_t predicate_native_tracer_00baa510(RegisteredType4TracerView view,
    const CameraAxesCrtAccess& crt) {
    return predicate_kernel(view.actual_owner, &crt);
}
void update_native_tracer_00baabb0(RegisteredType4TracerView view,
    const CameraMatrix& matrix, float age, const float* position,
    const std::array<float,3>& direction, float speed, float subject58,
    NativeTracerUpdateBindings bindings) {
    TracerUpdateFrame frame{&bindings.crt, &bindings.transforms, {}};
    update_kernel(view.actual_owner, nullptr, &matrix, age, position,
        direction.data(), speed, subject58, &frame);
}
void append_native_tracer_point_00baa670(RegisteredType4TracerView view,
    const float* position, const float* direction, const CameraAxesCrtAccess& crt) {
    append_kernel(view.actual_owner, nullptr, position, direction, &crt);
}
void set_native_tracer_alpha_00ba9960(RegisteredType4TracerView view, float alpha) noexcept {
    alpha_kernel(view.actual_owner, nullptr, alpha);
}
void set_native_generated_model_bounds_00b74390(void* actual, const float* words) noexcept {
    bounds_kernel(actual, nullptr, words);
}
} // namespace bsp
