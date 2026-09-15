#include "bsp/native_postprocess_pose.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered x87 operation schedule requires MSVC Win32.
#endif

extern "C" double __cdecl _CIatan2();

namespace bsp {
namespace {
const std::uint32_t pose_negative_zero = 0x80000000u; // 00D7A208
const std::uint64_t pose_cosine_threshold = 0x3f1a36e2e0000000ull; // 00D7A268

// EBX carries the required borrowed CRT binding through the private kernel.
// The canonical length entry saves/restores EBX; no extra FP operation or
// spill is introduced by this adapter.
__declspec(naked) float __cdecl pose_vector_length() {
    __asm {
        mov edx, ebx
        jmp camera_vector_length_00419440
    }
}

// Exact native caller instructions follow. CALL targets and the two constant
// addresses are relocated; no library implementation is copied here.
__declspec(naked) float* __fastcall pose_kernel(const float*, void*, float*) {
    __asm {
        sub esp, 0x28 // 00b630f0
        push esi // 00b630f3
        mov esi, ecx // 00b630f4
        call pose_vector_length // 00b630f6
        fstp dword ptr [esp + 4] // 00b630fb
        fldz  // 00b630ff
        fld dword ptr [esp + 4] // 00b63101
        fcomi st(0), st(1) // 00b63105
        fstp st(1) // 00b63107
        jbe L_00b63115 // 00b63109
        fld1  // 00b6310b
        fdivrp st(1), st(0) // 00b6310d
        fstp dword ptr [esp + 4] // 00b6310f
        jmp L_00b63120 // 00b63113
L_00b63115:
        xorps xmm0, xmm0 // 00b63115
        fstp st(0) // 00b63118
        movss dword ptr [esp + 4], xmm0 // 00b6311a
L_00b63120:
        fld dword ptr [esi] // 00b63120
        push edi // 00b63122
        fld dword ptr [esp + 8] // 00b63123
        lea edi, [esi + 0x10] // 00b63127
        fld st(0) // 00b6312a
        mov ecx, edi // 00b6312c
        fmulp st(2), st(0) // 00b6312e
        fxch st(1) // 00b63130
        fstp dword ptr [esp + 0xc] // 00b63132
        fld dword ptr [esi + 4] // 00b63136
        fmul st(0), st(1) // 00b63139
        fstp dword ptr [esp + 0x10] // 00b6313b
        fmul dword ptr [esi + 8] // 00b6313f
        fstp dword ptr [esp + 0x14] // 00b63142
        call pose_vector_length // 00b63146
        fstp dword ptr [esp + 8] // 00b6314b
        fldz  // 00b6314f
        fld dword ptr [esp + 8] // 00b63151
        fcomi st(0), st(1) // 00b63155
        fstp st(1) // 00b63157
        jbe L_00b63165 // 00b63159
        fld1  // 00b6315b
        fdivrp st(1), st(0) // 00b6315d
        fstp dword ptr [esp + 8] // 00b6315f
        jmp L_00b63170 // 00b63163
L_00b63165:
        xorps xmm0, xmm0 // 00b63165
        fstp st(0) // 00b63168
        movss dword ptr [esp + 8], xmm0 // 00b6316a
L_00b63170:
        fld dword ptr [edi] // 00b63170
        add esi, 0x20 // 00b63172
        fld dword ptr [esp + 8] // 00b63175
        mov ecx, esi // 00b63179
        fld st(0) // 00b6317b
        fmulp st(2), st(0) // 00b6317d
        fxch st(1) // 00b6317f
        fstp dword ptr [esp + 0x24] // 00b63181
        fmul dword ptr [edi + 4] // 00b63185
        fstp dword ptr [esp + 0x28] // 00b63188
        call pose_vector_length // 00b6318c
        xorps xmm2, xmm2 // 00b63191
        fstp dword ptr [esp + 8] // 00b63194
        fldz  // 00b63198
        pop edi // 00b6319a
        fld dword ptr [esp + 4] // 00b6319b
        fcomi st(0), st(1) // 00b6319f
        fstp st(1) // 00b631a1
        jbe L_00b631af // 00b631a3
        fld1  // 00b631a5
        fdivrp st(1), st(0) // 00b631a7
        fstp dword ptr [esp + 4] // 00b631a9
        jmp L_00b631b7 // 00b631ad
L_00b631af:
        fstp st(0) // 00b631af
        movss dword ptr [esp + 4], xmm2 // 00b631b1
L_00b631b7:
        fld dword ptr [esi] // 00b631b7
        fld dword ptr [esp + 4] // 00b631b9
        fld st(0) // 00b631bd
        fmulp st(2), st(0) // 00b631bf
        fxch st(1) // 00b631c1
        fstp dword ptr [esp + 0x14] // 00b631c3
        fmul dword ptr [esi + 8] // 00b631c7
        fstp dword ptr [esp + 0x1c] // 00b631ca
        fld dword ptr [esp + 0xc] // 00b631ce
        fsin  // 00b631d2
        fstp dword ptr [esp + 4] // 00b631d4
        movss xmm0, dword ptr [pose_negative_zero] // 00b631d8
        mov esi, dword ptr [esp + 0x30] // 00b631e0
        movaps xmm1, xmm0 // 00b631e4
        subss xmm1, dword ptr [esp + 4] // 00b631e7
        movss dword ptr [esi + 4], xmm1 // 00b631ed
        movss dword ptr [esp + 4], xmm1 // 00b631f2
        fld dword ptr [esp + 4] // 00b631f8
        fcos  // 00b631fc
        fstp dword ptr [esp + 0x30] // 00b631fe
        movss xmm1, dword ptr [esp + 0x30] // 00b63202
        comiss xmm1, xmm2 // 00b63208
        jbe L_00b63215 // 00b6320b
        movss dword ptr [esp + 4], xmm1 // 00b6320d
        jmp L_00b6321f // 00b63213
L_00b63215:
        subss xmm0, xmm1 // 00b63215
        movss dword ptr [esp + 4], xmm0 // 00b63219
L_00b6321f:
        fld qword ptr [pose_cosine_threshold] // 00b6321f
        fld dword ptr [esp + 4] // 00b63225
        fcomip st(0), st(1) // 00b63229
        fstp st(0) // 00b6322b
        jbe L_00b632a3 // 00b6322d
        fld dword ptr [esp + 0xc] // 00b6322f
        fld dword ptr [esp + 0x30] // 00b63233
        fld st(0) // 00b63237
        fdivp st(2), st(0) // 00b63239
        fxch st(1) // 00b6323b
        fstp dword ptr [esp + 4] // 00b6323d
        fld dword ptr [esp + 4] // 00b63241
        fld dword ptr [esp + 0x24] // 00b63245
        fdivrp st(2), st(0) // 00b63249
        fxch st(1) // 00b6324b
        fstp dword ptr [esp + 4] // 00b6324d
        fld dword ptr [esp + 4] // 00b63251
        call _CIatan2 // 00b63255
        fstp dword ptr [esp + 4] // 00b6325a
        fld dword ptr [esp + 4] // 00b6325e
        fstp dword ptr [esi + 8] // 00b63262
        fld dword ptr [esp + 0x14] // 00b63265
        fld dword ptr [esp + 0x30] // 00b63269
        fld st(0) // 00b6326d
        fdivp st(2), st(0) // 00b6326f
        fxch st(1) // 00b63271
        fstp dword ptr [esp + 0x30] // 00b63273
        fld dword ptr [esp + 0x30] // 00b63277
        fld dword ptr [esp + 0x1c] // 00b6327b
        fdivrp st(2), st(0) // 00b6327f
        fxch st(1) // 00b63281
        fstp dword ptr [esp + 0x30] // 00b63283
        fld dword ptr [esp + 0x30] // 00b63287
        call _CIatan2 // 00b6328b
        fstp dword ptr [esp + 0x30] // 00b63290
        fld dword ptr [esp + 0x30] // 00b63294
        mov eax, esi // 00b63298
        fstp dword ptr [esi] // 00b6329a
        pop esi // 00b6329c
        add esp, 0x28 // 00b6329d
        ret 4 // 00b632a0
L_00b632a3:
        fld dword ptr [esp + 0x20] // 00b632a3
        movss dword ptr [esi + 8], xmm2 // 00b632a7
        fld dword ptr [esp + 8] // 00b632ac
        call _CIatan2 // 00b632b0
        fstp dword ptr [esp + 0x30] // 00b632b5
        fld dword ptr [esp + 0x30] // 00b632b9
        mov eax, esi // 00b632bd
        fstp dword ptr [esi] // 00b632bf
        pop esi // 00b632c1
        add esp, 0x28 // 00b632c2
        ret 4 // 00b632c5
    }
}

} // namespace

__declspec(naked) float* __fastcall extract_native_animator_angles_00b630f0(
    const float*, const CameraAxesCrtAccess*, float*) {
    __asm {
        push ebx
        mov ebx, edx
        push dword ptr [esp + 8]
        call pose_kernel
        pop ebx
        ret 4
    }
}

} // namespace bsp
