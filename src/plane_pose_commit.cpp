#include "bsp/plane_pose_commit.hpp"

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
