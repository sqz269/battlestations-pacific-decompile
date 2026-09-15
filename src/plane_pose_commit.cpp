#include "bsp/plane_pose_commit.hpp"
#include "bsp/system_camera_axes.hpp"

#include <cmath>

// Reconstruction of 0085DC80 and the three vector helpers it calls.
// docs/PLANE_POSE_COMMIT.md carries the evidence and the coverage table; every
// name is a hypothesis, not a recovered symbol.

namespace bsp {
namespace {

// 00419440 BSP_Vector3f_Length, __fastcall(const float* v /*ECX*/) -> ST0.
// Each square is rounded back to float through [ESP+8] (00419467, 00419475,
// 00419481) before the adds, so the three products are float; only the two adds
// at 0041947D and 00419485 stay on the x87 stack, which is why the accumulation
// is modelled in double and rounded once, at 00419489 FSTP float.
float length_00419440(const float v[3]) {
    const float xx = v[0] * v[0];  // 00419461 FLD ST1 / 00419463 FMULP ST2
    const float yy = v[1] * v[1];  // 0041946F FLD ST2 / 00419471 FMULP ST3
    const float zz = v[2] * v[2];  // 0041947F FMUL ST0, the self-multiply
    const float sum = static_cast<float>(static_cast<double>(xx) +
                                         static_cast<double>(yy) +
                                         static_cast<double>(zz));
    return std::sqrt(sum);  // 0041948D FLD float / 00419491 CALL 00BF7030
}

// 00419510 BSP_Vector3f_Normalize, __fastcall(float* out /*ECX*/, const float* in
// /*EDX*/) -> EAX = out. The native writes into a caller stack temp and the caller
// copies back; in place is equivalent because 00419543..0041955E reads each
// component before it writes the same component.
void normalize_00419510(float v[3]) {
    const float len = length_00419440(v);  // 00419519
    // 00419522 FLDZ / 00419528 FCOMI / 0041952C JBE. Length <= 0, and an
    // unordered compare against a NaN, both take 00419538 XORPS, which stores
    // +0.0f as the scale. A zero vector normalises to zero, never to a NaN.
    const float inv = len > 0.0f ? 1.0f / len : 0.0f;
    v[0] *= inv;  // 00419543..00419551
    v[1] *= inv;  // 00419553..00419558
    v[2] *= inv;  // 0041955B..0041955E
}

// 004F9B30 BSP_Vector3f_Cross, __fastcall(float* out /*ECX*/, const float* a
// /*EDX*/, const float* b /*stack*/), RET 4. The standard right-hand-rule formula,
// read off the x87 stack at 004F9B54..004F9B73 for x and 004F9B75..004F9B97 for y:
// out.x = a.y*b.z - a.z*b.y. Both products stay on the stack and the difference is
// rounded once, so the term is modelled in double.
void cross_004f9b30(float out[3], const float a[3], const float b[3]) {
    const double x = static_cast<double>(a[1]) * b[2] - static_cast<double>(a[2]) * b[1];
    const double y = static_cast<double>(a[2]) * b[0] - static_cast<double>(a[0]) * b[2];
    const double z = static_cast<double>(a[0]) * b[1] - static_cast<double>(a[1]) * b[0];
    out[0] = static_cast<float>(x);
    out[1] = static_cast<float>(y);
    out[2] = static_cast<float>(z);
}

// 0085DCEE..0085DD02 and 0085DD2A..0085DD42, the same inlined dot twice. The
// native order is (x*x' + y*y') + z*z' with everything on the x87 stack and a
// single FSTP float at the end.
float dot3_x87(const float a[3], const float b[3]) {
    const double sum = static_cast<double>(a[0]) * b[0] +
                       static_cast<double>(a[1]) * b[1] +
                       static_cast<double>(a[2]) * b[2];
    return static_cast<float>(sum);
}

// 0085DCB7..0085DCEB. The three scaled components are computed into [ESP+10h..18h]
// and copied back only afterwards, so the multiply never reads a component it has
// already overwritten. Each product is rounded to float on the way to the temp.
void scale3(float v[3], float s) {
    const float x = v[0] * s;  // 0085DCC0..0085DCC6
    const float y = v[1] * s;  // 0085DCCA..0085DCCF
    const float z = v[2] * s;  // 0085DCD3..0085DCD6
    v[0] = x;                  // 0085DCDA..0085DCDE
    v[1] = y;                  // 0085DCE0..0085DCE4
    v[2] = z;                  // 0085DCE7..0085DCEB
}

// 0085DE19..0085DE32 and 0085DD66..0085DD7F. The scaled term reaches memory as a
// float before the subtraction reads it, so plain float arithmetic is exact here.
void subtract_projection(float v[3], const float axis[3], float amount) {
    v[0] -= axis[0] * amount;
    v[1] -= axis[1] * amount;
    v[2] -= axis[2] * amount;
}

}  // namespace

PoseOrthonormalizeResult orthonormalize_basis_rows_0085dc80(const PoseBasis& in) {
    PoseOrthonormalizeResult out;
    out.basis = in;
    float* const row0 = out.basis.row0;  // 0085DC85 MOV ESI,ECX
    float* const row1 = out.basis.row1;  // 0085DCB9 LEA EDI,[ESI+10h]
    float* const row2 = out.basis.row2;  // 0085DC87 LEA EBX,[ESI+20h]

    // Step 1. Row 2 is the authority and is simply renormalised. 0085DC8B MOV
    // ECX,EBX / 0085DC8D CALL 00419440 passes row 2, not the whole matrix.
    out.forward_length = length_00419440(row2);

    // 0085DC96 FLDZ / 0085DC9C FCOMI / 0085DCA0 JBE. Exactly 00419510's guard:
    // a length that is not strictly positive, NaN included, yields a scale of
    // +0.0f from the XORPS at 0085DCAC. There is no error path - row 2 becomes
    // (0,0,0) and the rest of the routine runs on it.
    const float inv = out.forward_length > 0.0f ? 1.0f / out.forward_length : 0.0f;
    scale3(row2, inv);

    // Step 2. How much of row 1 already lies along row 2, with row 2 unit length.
    const float d = dot3_x87(row1, row2);  // 0085DCEE..0085DD02

    // 0085DD0C FABS, then 0085DD12 FLD double [00D0D0A0] and 0085DD1C FCOMIP.
    // 0085DD20 JBE jumps to the normal path. JBE is CF or ZF, and an unordered
    // FCOMIP sets both, so a NaN dot takes the *normal* path - hence the negated
    // form of the test rather than a plain <=.
    if (!(std::fabs(d) > kBasisParallelLimit_00d0d0a0)) {
        // Normal path, 0085DDFB. Row 1 is a usable up hint.
        out.branch = PoseOrthonormalizeBranch::UpReference;

        subtract_projection(row1, row2, d);  // 0085DDFB..0085DE32
        normalize_00419510(row1);            // 0085DE35 CALL 00419510, EDX = EDI

        // 0085DE3C PUSH EBX (row 2) / 0085DE3F MOV EDX,EDI (row 1) / 0085DE51
        // CALL 004F9B30: row0 = row1 x row2. No negation on this path.
        cross_004f9b30(row0, row1, row2);
        normalize_00419510(row0);  // 0085DE78 CALL 00419510, EDX = ESI
    } else {
        // Degenerate path, 0085DD26. Row 1 is within 2.56 degrees of row 2, so
        // the plane they should span is ill conditioned. Row 0 takes over as the
        // second reference and row 1 is the one that gets rebuilt.
        out.branch = PoseOrthonormalizeBranch::RightReference;

        const float e = dot3_x87(row2, row0);  // 0085DD2A..0085DD42
        subtract_projection(row0, row2, e);    // 0085DD46..0085DD7F
        normalize_00419510(row0);              // 0085DD82 CALL 00419510, EDX = ESI

        // 0085DD89 PUSH EBX (row 2) / 0085DD8C MOV EDX,ESI (row 0) / 0085DD9E
        // CALL 004F9B30, then 0085DDA3..0085DDDA subtract each component from the
        // -0.0f at 00D7A208. row1 = -(row0 x row2) = row2 x row0, which is the
        // handedness the normal path's row0 = row1 x row2 implies. Negating
        // before normalising is the same as after: the length is unchanged.
        cross_004f9b30(row1, row0, row2);
        row1[0] = -row1[0];  // 0085DDBC SUBSS XMM1,XMM2 -> 0085DDD1
        row1[1] = -row1[1];  // 0085DDC3 SUBSS XMM2,XMM3 -> 0085DDD5
        row1[2] = -row1[2];  // 0085DDC7 SUBSS XMM0,XMM4 -> 0085DDDA
        normalize_00419510(row1);  // 0085DDDF CALL 00419510, EDX = EDI
    }

    return out;
}

void orthonormalize_pose_matrix_0085dc80(float m[16]) {
    PoseBasis in;
    for (int i = 0; i < 3; ++i) {
        in.row0[i] = m[i];      // +00h..+08h
        in.row1[i] = m[4 + i];  // +10h..+18h
        in.row2[i] = m[8 + i];  // +20h..+28h
    }
    const PoseOrthonormalizeResult r = orthonormalize_basis_rows_0085dc80(in);
    for (int i = 0; i < 3; ++i) {
        m[i] = r.basis.row0[i];
        m[4 + i] = r.basis.row1[i];
        m[8 + i] = r.basis.row2[i];
    }
    // m[3], m[7], m[11] and m[12..15] are untouched, as in the native body: no
    // instruction between 0085DC80 and 0085DE93 addresses [ESI+0Ch], [ESI+1Ch],
    // [ESI+2Ch] or anything at or past [ESI+30h].
}

}  // namespace bsp

namespace bsp {

// Complete native in-place schedule. EBP retains the borrowed bindings while
// the original stack temporaries and x87/SSE instruction order stay intact.
__declspec(naked) void __fastcall orthonormalize_native_pose_matrix_0085dc80(
    float*, const NativePoseOrthonormalizationAccess*) {
    __asm {
        push ebp
        mov ebp, edx
        sub esp, 0x10 // 0085dc80
        push ebx // 0085dc83
        push esi // 0085dc84
        mov esi, ecx // 0085dc85
        lea ebx, [esi + 0x20] // 0085dc87
        push edi // 0085dc8a
        mov ecx, ebx // 0085dc8b
        mov edx, dword ptr [ebp]
        call camera_vector_length_00419440 // 0085dc8d
        fstp dword ptr [esp + 0xc] // 0085dc92
        fldz // 0085dc96
        fld dword ptr [esp + 0xc] // 0085dc98
        fcomi st(0), st(1) // 0085dc9c
        fstp st(1) // 0085dc9e
        jbe native_pose_0085dcac // 0085dca0
        fld1 // 0085dca2
        fdivrp st(1), st(0) // 0085dca4
        fstp dword ptr [esp + 0xc] // 0085dca6
        jmp native_pose_0085dcb7 // 0085dcaa
    native_pose_0085dcac:
        xorps xmm0, xmm0 // 0085dcac
        fstp st(0) // 0085dcaf
        movss dword ptr [esp + 0xc], xmm0 // 0085dcb1
    native_pose_0085dcb7:
        fld dword ptr [ebx] // 0085dcb7
        lea edi, [esi + 0x10] // 0085dcb9
        fld dword ptr [esp + 0xc] // 0085dcbc
        fld st(0) // 0085dcc0
        fmulp st(2), st(0) // 0085dcc2
        fxch st(1) // 0085dcc4
        fstp dword ptr [esp + 0x10] // 0085dcc6
        fld dword ptr [ebx + 4] // 0085dcca
        fmul st(0), st(1) // 0085dccd
        fstp dword ptr [esp + 0x14] // 0085dccf
        fmul dword ptr [ebx + 8] // 0085dcd3
        fstp dword ptr [esp + 0x18] // 0085dcd6
        fld dword ptr [esp + 0x10] // 0085dcda
        fstp dword ptr [ebx] // 0085dcde
        fld dword ptr [esp + 0x14] // 0085dce0
        fstp dword ptr [ebx + 4] // 0085dce4
        fld dword ptr [esp + 0x18] // 0085dce7
        fstp dword ptr [ebx + 8] // 0085dceb
        fld dword ptr [edi + 4] // 0085dcee
        fmul dword ptr [ebx + 4] // 0085dcf1
        fld dword ptr [edi] // 0085dcf4
        fmul dword ptr [ebx] // 0085dcf6
        faddp st(1), st(0) // 0085dcf8
        fld dword ptr [edi + 8] // 0085dcfa
        fmul dword ptr [ebx + 8] // 0085dcfd
        faddp st(1), st(0) // 0085dd00
        fstp dword ptr [esp + 0xc] // 0085dd02
        fld dword ptr [esp + 0xc] // 0085dd06
        fld st(0) // 0085dd0a
        fabs // 0085dd0c
        fstp dword ptr [esp + 0xc] // 0085dd0e
        mov eax, dword ptr [ebp+4]
        fld qword ptr [eax] // 0085dd12
        fld dword ptr [esp + 0xc] // 0085dd18
        fcomip st(0), st(1) // 0085dd1c
        fstp st(0) // 0085dd1e
        jbe native_pose_0085ddfb // 0085dd20
        fstp st(0) // 0085dd26
        mov edx, esi // 0085dd28
        fld dword ptr [ebx + 4] // 0085dd2a
        lea ecx, [esp + 0x10] // 0085dd2d
        fmul dword ptr [esi + 4] // 0085dd31
        fld dword ptr [ebx] // 0085dd34
        fmul dword ptr [esi] // 0085dd36
        faddp st(1), st(0) // 0085dd38
        fld dword ptr [ebx + 8] // 0085dd3a
        fmul dword ptr [esi + 8] // 0085dd3d
        faddp st(1), st(0) // 0085dd40
        fstp dword ptr [esp + 0xc] // 0085dd42
        fld dword ptr [ebx] // 0085dd46
        fld dword ptr [esp + 0xc] // 0085dd48
        fld st(0) // 0085dd4c
        fmulp st(2), st(0) // 0085dd4e
        fxch st(1) // 0085dd50
        fstp dword ptr [esp + 0x10] // 0085dd52
        fld dword ptr [ebx + 4] // 0085dd56
        fmul st(0), st(1) // 0085dd59
        fstp dword ptr [esp + 0x14] // 0085dd5b
        fmul dword ptr [ebx + 8] // 0085dd5f
        fstp dword ptr [esp + 0x18] // 0085dd62
        fld dword ptr [esi] // 0085dd66
        fsub dword ptr [esp + 0x10] // 0085dd68
        fstp dword ptr [esi] // 0085dd6c
        fld dword ptr [esi + 4] // 0085dd6e
        fsub dword ptr [esp + 0x14] // 0085dd71
        fstp dword ptr [esi + 4] // 0085dd75
        fld dword ptr [esi + 8] // 0085dd78
        fsub dword ptr [esp + 0x18] // 0085dd7b
        fstp dword ptr [esi + 8] // 0085dd7f
        push dword ptr [ebp]
        call camera_vector_normalize_00419510 // 0085dd82
        fld dword ptr [eax] // 0085dd87
        push ebx // 0085dd89
        fstp dword ptr [esi] // 0085dd8a
        mov edx, esi // 0085dd8c
        fld dword ptr [eax + 4] // 0085dd8e
        lea ecx, [esp + 0x14] // 0085dd91
        fstp dword ptr [esi + 4] // 0085dd95
        fld dword ptr [eax + 8] // 0085dd98
        fstp dword ptr [esi + 8] // 0085dd9b
        call camera_vector_cross_004f9b30 // 0085dd9e
        movss xmm2, dword ptr [eax] // 0085dda3
        mov ecx, dword ptr [ebp+8]
        movss xmm0, dword ptr [ecx] // 0085dda7
        movss xmm3, dword ptr [eax + 4] // 0085ddaf
        movss xmm4, dword ptr [eax + 8] // 0085ddb4
        movaps xmm1, xmm0 // 0085ddb9
        subss xmm1, xmm2 // 0085ddbc
        movaps xmm2, xmm0 // 0085ddc0
        subss xmm2, xmm3 // 0085ddc3
        subss xmm0, xmm4 // 0085ddc7
        mov edx, edi // 0085ddcb
        lea ecx, [esp + 0x10] // 0085ddcd
        movss dword ptr [edi], xmm1 // 0085ddd1
        movss dword ptr [edi + 4], xmm2 // 0085ddd5
        movss dword ptr [edi + 8], xmm0 // 0085ddda
        push dword ptr [ebp]
        call camera_vector_normalize_00419510 // 0085dddf
        fld dword ptr [eax] // 0085dde4
        fstp dword ptr [edi] // 0085dde6
        fld dword ptr [eax + 4] // 0085dde8
        fstp dword ptr [edi + 4] // 0085ddeb
        fld dword ptr [eax + 8] // 0085ddee
        fstp dword ptr [edi + 8] // 0085ddf1
        pop edi // 0085ddf4
        pop esi // 0085ddf5
        pop ebx // 0085ddf6
        add esp, 0x10 // 0085ddf7
        pop ebp
        ret // 0085ddfa
    native_pose_0085ddfb:
        fld st(0) // 0085ddfb
        mov edx, edi // 0085ddfd
        fmul dword ptr [ebx] // 0085ddff
        lea ecx, [esp + 0x10] // 0085de01
        fstp dword ptr [esp + 0x10] // 0085de05
        fld dword ptr [ebx + 4] // 0085de09
        fmul st(0), st(1) // 0085de0c
        fstp dword ptr [esp + 0x14] // 0085de0e
        fmul dword ptr [ebx + 8] // 0085de12
        fstp dword ptr [esp + 0x18] // 0085de15
        fld dword ptr [edi] // 0085de19
        fsub dword ptr [esp + 0x10] // 0085de1b
        fstp dword ptr [edi] // 0085de1f
        fld dword ptr [edi + 4] // 0085de21
        fsub dword ptr [esp + 0x14] // 0085de24
        fstp dword ptr [edi + 4] // 0085de28
        fld dword ptr [edi + 8] // 0085de2b
        fsub dword ptr [esp + 0x18] // 0085de2e
        fstp dword ptr [edi + 8] // 0085de32
        push dword ptr [ebp]
        call camera_vector_normalize_00419510 // 0085de35
        fld dword ptr [eax] // 0085de3a
        push ebx // 0085de3c
        fstp dword ptr [edi] // 0085de3d
        mov edx, edi // 0085de3f
        fld dword ptr [eax + 4] // 0085de41
        lea ecx, [esp + 0x14] // 0085de44
        fstp dword ptr [edi + 4] // 0085de48
        fld dword ptr [eax + 8] // 0085de4b
        fstp dword ptr [edi + 8] // 0085de4e
        call camera_vector_cross_004f9b30 // 0085de51
        movss xmm0, dword ptr [eax] // 0085de56
        movss xmm1, dword ptr [eax + 4] // 0085de5a
        movss xmm2, dword ptr [eax + 8] // 0085de5f
        mov edx, esi // 0085de64
        lea ecx, [esp + 0x10] // 0085de66
        movss dword ptr [esi], xmm0 // 0085de6a
        movss dword ptr [esi + 4], xmm1 // 0085de6e
        movss dword ptr [esi + 8], xmm2 // 0085de73
        push dword ptr [ebp]
        call camera_vector_normalize_00419510 // 0085de78
        fld dword ptr [eax] // 0085de7d
        fstp dword ptr [esi] // 0085de7f
        pop edi // 0085de81
        fld dword ptr [eax + 4] // 0085de82
        fstp dword ptr [esi + 4] // 0085de85
        fld dword ptr [eax + 8] // 0085de88
        fstp dword ptr [esi + 8] // 0085de8b
        pop esi // 0085de8e
        pop ebx // 0085de8f
        add esp, 0x10 // 0085de90
        pop ebp
        ret // 0085de93
    }
}

} // namespace bsp
