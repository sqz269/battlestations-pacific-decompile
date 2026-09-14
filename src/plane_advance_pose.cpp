#include "bsp/plane_advance_pose.hpp"

#include <cmath>

// Reconstruction of 007C6500 and the two pose writers it dispatches to.
// docs/PLANE_ADVANCE_POSE.md carries the evidence and the coverage table; every
// name is a hypothesis, not a recovered symbol.

namespace bsp {
namespace {

// 00419440 BSP_Vector3f_Length, re-read in docs/PLANE_POSE_COMMIT.md: the three
// squares are rounded back to float, the two adds stay on the x87 stack.
float length_00419440(const float v[3]) {
    const float xx = v[0] * v[0];
    const float yy = v[1] * v[1];
    const float zz = v[2] * v[2];
    return std::sqrt(static_cast<float>(static_cast<double>(xx) +
                                        static_cast<double>(yy) +
                                        static_cast<double>(zz)));
}

// The reciprocal guard 0085DAE5..0085DB00 shares with 00419510 and 0085DC96:
// a length that is not strictly positive, NaN included, yields +0.0f, so the
// vector collapses to zero rather than producing a NaN.
float reciprocal_or_zero(float len) { return len > 0.0f ? 1.0f / len : 0.0f; }

void scale3(float v[3], float s) {
    const float x = v[0] * s;
    const float y = v[1] * s;
    const float z = v[2] * s;
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

void normalize3(float v[3]) { scale3(v, reciprocal_or_zero(length_00419440(v))); }

// The inlined x87 dot: (x*x' + y*y') + z*z' with a single rounding at the end.
float dot3_x87(const float a[3], const float b[3]) {
    return static_cast<float>(static_cast<double>(a[0]) * b[0] +
                              static_cast<double>(a[1]) * b[1] +
                              static_cast<double>(a[2]) * b[2]);
}

// 004F9B30 BSP_Vector3f_Cross, out = a x b, the standard right-hand formula
// traced in docs/PLANE_POSE_COMMIT.md.
void cross_004f9b30(float out[3], const float a[3], const float b[3]) {
    const double x = static_cast<double>(a[1]) * b[2] - static_cast<double>(a[2]) * b[1];
    const double y = static_cast<double>(a[2]) * b[0] - static_cast<double>(a[0]) * b[2];
    const double z = static_cast<double>(a[0]) * b[1] - static_cast<double>(a[1]) * b[0];
    out[0] = static_cast<float>(x);
    out[1] = static_cast<float>(y);
    out[2] = static_cast<float>(z);
}

void copy3(float out[3], const float in[3]) {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

// 0085E7CF..0085E7C4's full 4x4 transpose, read off as six element swaps:
// (1,4) (2,8) (3,12) (6,9) (7,13) (11,14).
void transpose4x4(AdvanceMatrix& m) {
    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 4; ++j) {
            const float t = m.m[4 * i + j];
            m.m[4 * i + j] = m.m[4 * j + i];
            m.m[4 * j + i] = t;
        }
    }
}

}  // namespace

PlaneAdvanceSelection select_advance_arm_007c6500(const PlaneAdvanceGate& gate) {
    PlaneAdvanceSelection out;

    // 007C6509..007C6522: the step is scaled by unit+340h only when that field is
    // strictly positive. COMISS/JBE, so a zero, a negative and a NaN all leave the
    // step alone rather than scaling by them.
    out.step = gate.time_scale > 0.0f ? gate.step * gate.time_scale : gate.step;

    // 007C6528 CMP byte [unit+5Ch],0 / JNZ 007C657F. Clear takes the early exit,
    // which invalidates the pose cache and returns without writing a matrix.
    if (!gate.active) {
        out.arm = PlaneAdvanceArm::Inactive;
        return out;
    }

    // 007C66DD..007C66F2. The blocker is only consulted when unit+9D4h is not
    // null; when it is null 007C66E5 jumps straight past both byte tests.
    if (gate.special_blocker && (gate.self_blocked || gate.blocker_blocks)) {
        out.arm = PlaneAdvanceArm::Blocked;  // 007C6714 FSTP ST0
        return out;
    }

    // 007C66F4 CMP byte [unit+520h],0 / 007C6704 JZ. Set picks the short arm.
    out.arm = gate.translate_only ? PlaneAdvanceArm::TranslateOnly
                                  : PlaneAdvanceArm::RotateAndTranslate;
    return out;
}

ControlSurfaceDrive control_surface_drive_007c65da(float roll, float pitch, float yaw) {
    ControlSurfaceDrive out;
    out.roll_group = roll;   // 007C65F0 FLD [unit+9E4h], no sign change
    out.pitch_group = -pitch;  // 007C6628 SUBSS from the -0.0f at 00D7A208
    out.yaw_negative_group = -yaw;  // 007C6668, the same negation
    out.yaw_positive_group = yaw;   // 007C6695 FLD [unit+9ECh], no sign change
    return out;
}

void orthonormalize_up_first_0085dad0(AdvanceMatrix& m) {
    float* const row0 = m.row(0);  // 0085DAD4 MOV EBX,ECX
    float* const row1 = m.row(1);  // 0085DAD7 LEA EDI,[EBX+10h], the authority
    float* const row2 = m.row(2);  // 0085DB0D LEA ESI,[EBX+20h]

    // 0085DADC..0085DB3D: row 1 is renormalised first and is never rebuilt.
    normalize3(row1);

    // 0085DB40..0085DB91: row 2 loses its row-1 component.
    const float d = dot3_x87(row2, row1);
    row2[0] -= row1[0] * d;
    row2[1] -= row1[1] * d;
    row2[2] -= row1[2] * d;

    // 0085DB94..0085DBF6.
    normalize3(row2);

    // 0085DBC0 PUSH ESI (row 2) / 0085DBC5 MOV EDX,EDI (row 1) / 0085DBF9
    // CALL 004F9B30: row0 = row1 x row2, the same handedness 0085DC80 asserts.
    cross_004f9b30(row0, row1, row2);

    // 0085DC1C..0085DC75.
    normalize3(row0);
}

RotationAxisAngle rotation_axis_angle_0085e4d0(const float w[3]) {
    RotationAxisAngle out;
    copy3(out.axis, w);

    // 0085E4F0..0085E506: (x*x + y*y) + z*z, rounded once to float.
    const float len2 = static_cast<float>(static_cast<double>(w[0]) * w[0] +
                                          static_cast<double>(w[1]) * w[1] +
                                          static_cast<double>(w[2]) * w[2]);

    // 0085E50A..0085E532: below the 1e-10 floor the length is forced to +0.0f
    // instead of being square-rooted.
    const float len = static_cast<double>(len2) > kRotationLengthSqFloor_00ce3820
                          ? std::sqrt(len2)
                          : 0.0f;
    out.angle = len;

    // 0085E53C..0085E544 FCOMIP / JA: the 1e-8 floor is compared *against* the
    // length, and JA leaves through 0085E871 without writing the output at all.
    if (kRotationAngleFloor_00d7a350 > static_cast<double>(len)) {
        out.rotates = false;
        return out;
    }
    out.rotates = true;

    // 0085E54A..0085E588: the argument vector is divided by the length in place,
    // on the caller's stack. FDIV, not a reciprocal multiply.
    out.axis[0] = w[0] / len;
    out.axis[1] = w[1] / len;
    out.axis[2] = w[2] / len;

    // 0085E58F..0085E6DD. What is proven: XMM6 and XMM5 are both zeroed at
    // 0085E55B/0085E55E, XMM6 is never written again, and exactly one of XMM0 and
    // XMM5 receives the 1.0f at 00D7A24C (0085E6BD / 0085E6C7). So the up
    // reference always has y = 0 and is either world +X or world +Z. The chain
    // that chooses between them builds the absolute values of the axis components
    // (0085E58F..0085E5D0 are |x| and |y|) and compares them pairwise; only those
    // first two stores were read, so the exact predicate is NOT recovered.
    //
    // It does not affect the result. Any two look-at frames built about the same
    // axis differ by a rotation about their own Z, and that commutes with the
    // RotZ in the middle of L * RotZ * transpose(L). The selection exists to keep
    // the up reference from being parallel to the axis, which is what the rule
    // below reproduces; it is a degeneracy guard, not part of the rotation.
    out.up_reference_predicate_unread = true;
    const bool axis_leans_z = std::fabs(out.axis[2]) > std::fabs(out.axis[0]);
    out.up_reference[0] = axis_leans_z ? 1.0f : 0.0f;  // lean on Z, reference X
    out.up_reference[1] = 0.0f;
    out.up_reference[2] = axis_leans_z ? 0.0f : 1.0f;
    return out;
}

bool rotate_about_axis_0085e4d0(AdvanceMatrix& out, const AdvanceMatrix& in,
                                const float w[3], float scale,
                                const float eye_00f87574[3], AdvanceMatrixOps& ops) {
    const RotationAxisAngle ra = rotation_axis_angle_0085e4d0(w);
    if (!ra.rotates) {
        return false;  // 0085E871, `out` is left exactly as the caller had it
    }

    // 0085E6EA MOV EDX,0F87574h is the eye; 0085E6E2's pushed pointer is the
    // normalised axis as the target; 0085E6CF's twelve bytes are the up vector.
    AdvanceMatrix look_at{};
    ops.build_look_at_00b63f10(look_at, eye_00f87574, ra.axis, ra.up_reference);

    // 0085E708..0085E7C4: a second copy of the look-at is transposed in place.
    // For an orthonormal look-at that is its inverse.
    AdvanceMatrix look_at_t = look_at;
    transpose4x4(look_at_t);

    // 0085E719..0085E758: the angle is the length times the fourth stack argument,
    // then negated by 0085E750 FCHS.
    //
    // 00B64780 was read here and builds, row-major,
    //   [ cos  sin 0 0 ][ -sin cos 0 0 ][ 0 0 1 0 ][ 0 0 0 1 ]
    // (00B647C0 cos at m[0] and m[5], 00B647D5 sin at m[1], 00B647E4 -sin at
    // m[4], the -sin coming from the -0.0f at 00D7A208). Under the row-vector
    // convention that turns +X toward +Y, so it is +angle about +Z in the frame's
    // own handedness. Which way that lands in world depends on 00B63F10's
    // handedness and on the eye global, neither of which was read: see the
    // doc's "What could not be established".
    AdvanceMatrix rot_z{};
    ops.build_rotation_z_00b64780(rot_z, -(ra.angle * scale));

    // 0085E816..0085E826, three chained 00413920 calls with ECX = the previous
    // result. 00413920 is ECX = left, stack (dst, right), so the product is
    //   ((in * look_at) * rot_z) * transpose(look_at)
    // Post-multiplication in the row-vector convention: the rotation acts in
    // world space, after `in`, which makes `w` a world-space angular velocity.
    AdvanceMatrix a{};
    AdvanceMatrix b{};
    ops.multiply_00413920(a, in, look_at);
    ops.multiply_00413920(b, a, rot_z);
    ops.multiply_00413920(out, b, look_at_t);

    // 0085E833..0085E85A: row 3 is restored from the input's own copy, because
    // the look-at pair carries a translation the product would otherwise leave in
    // place. 0085E85F then calls 0085D3D0 on the result; that routine was not
    // read, so this reconstruction stops here. See the doc's coverage table.
    out.m[12] = in.m[12];
    out.m[13] = in.m[13];
    out.m[14] = in.m[14];
    return true;
}

void advance_position_007da218(float out[3], const float committed_translation[3],
                               const float linear_velocity[3],
                               const float external_velocity[3], float step) {
    // 007DA218..007DA267, and 007D827B..007D82BA in the short arm. Each product
    // reaches memory as a float before the addition reads it.
    for (int i = 0; i < 3; ++i) {
        out[i] = committed_translation[i] + linear_velocity[i] * step;
    }
    // 007DA26B..007DA2AD, and 007D82BE..007D8300.
    for (int i = 0; i < 3; ++i) {
        out[i] = out[i] + external_velocity[i] * step;
    }
}

float bank_yaw_angle_007da0ae(float blend, float bank_angle, float pitch_angle,
                              float bank_yaw_gain, float difficulty, float step) {
    // 007DA00C FCOS on unit+C64h and 007DA037 FSIN on unit+C68h. Both are taken
    // through a float spill first, so the argument is a float, not a double.
    const float c = static_cast<float>(std::cos(static_cast<double>(pitch_angle)));
    const float s = static_cast<float>(std::sin(static_cast<double>(bank_angle)));

    // 007DA0AE..007DA0D0, in the native's own order: sin, then cos, then the
    // class gain, then the blend, then the difficulty scalar.
    float angle = s;
    angle *= c;
    angle *= bank_yaw_gain;
    angle *= blend;
    angle *= difficulty;
    angle *= step;      // 007DA0D4..007DA0DF
    return -angle;      // 007DA0E7 FCHS
}

float level_blend_007da179(float level_rate, float step) {
    // 007DA179..007DA19C. FCOMIP against the 1.0 left on the stack by 007DA161's
    // FLD1. 007DA190 JBE keeps the product at 007DA19C; only the fall-through at
    // 007DA192 loads the 1.0f from 00D7A24C. An unordered compare sets CF and ZF,
    // so a NaN product takes JBE and survives unclamped - which `t > 1.0f` being
    // false for a NaN reproduces exactly.
    const float t = level_rate * step;
    return t > 1.0f ? 1.0f : t;
}

void apply_up_levelling_007da14d(AdvanceMatrix& m, const float transformed_level_axis[3],
                                 float blend) {
    // 007DA14D..007DA175: the delta is world up minus the transformed axis. The
    // native builds it as (0 - a, 1 - b, 0 - c) with FLDZ and FLD1, one component
    // at a time.
    const float delta[3] = {0.0f - transformed_level_axis[0],
                            1.0f - transformed_level_axis[1],
                            0.0f - transformed_level_axis[2]};

    // 007DA1A2..007DA208: row 1 takes the scaled delta. Each scaled component
    // reaches memory as a float first.
    float* const row1 = m.row(1);
    const float sx = delta[0] * blend;
    const float sy = delta[1] * blend;
    const float sz = delta[2] * blend;
    row1[0] = row1[0] + sx;
    row1[1] = row1[1] + sy;
    row1[2] = row1[2] + sz;

    // 007DA20C, the up-first orthonormaliser, because row 1 is what just moved.
    orthonormalize_up_first_0085dad0(m);
}

PlaneAdvanceOutput translate_only_007d8230(const PlaneAdvanceInputs& in) {
    PlaneAdvanceOutput out;
    out.live_pose = in.committed_pose;  // 007D8252..007D8261, the 64-byte copy

    // 007D8239..007D8247 UCOMISS / LAHF / TEST AH,44h / JNP. The step being
    // exactly zero skips everything below, including the first 004134F0 - the
    // live pose is not written at all on that path.
    if (in.step == 0.0f) {
        return out;
    }

    // 007D826B writes unit+74h here, before row 3 is touched. Nothing reads it
    // between that call and 007D831C, which writes the same destination again, so
    // the first write is dead. It is reproduced by the copy above only because the
    // result is identical; no separate store is modelled.
    float pos[3];
    const float committed[3] = {in.committed_pose.m[12], in.committed_pose.m[13],
                                in.committed_pose.m[14]};
    advance_position_007da218(pos, committed, in.linear_velocity, in.external_velocity,
                              in.step);
    out.live_pose.m[12] = pos[0];  // 007D8304..007D8318
    out.live_pose.m[13] = pos[1];
    out.live_pose.m[14] = pos[2];
    return out;
}

PlaneAdvanceOutput rotate_and_translate_007d9f60(const PlaneAdvanceInputs& in,
                                                 AdvanceMatrixOps& ops) {
    PlaneAdvanceOutput out;
    out.live_pose = in.committed_pose;  // 007D9F6E..007D9F7D

    // 007D9F95..007D9FA0, the same exact-zero test as the short arm. 007D9F87 has
    // already copied the committed pose into unit+74h by this point, so a zero
    // step does leave the live pose equal to the committed one.
    if (in.step == 0.0f) {
        return out;
    }

    // 007D9FA6..007D9FF6. The angular velocity times the step is the rotation
    // vector; 007D9FDC FLD1 puts 1.0f in the fourth stack slot, which 0085E723
    // multiplies the angle by.
    const float w[3] = {in.angular_velocity[0] * in.step, in.angular_velocity[1] * in.step,
                        in.angular_velocity[2] * in.step};
    AdvanceMatrix rotated{};
    if (rotate_about_axis_0085e4d0(rotated, in.committed_pose, w, 1.0f, in.eye_00f87574,
                                   ops)) {
        out.live_pose = rotated;
        out.rotation_applied = true;
    }

    // 007DA080..007DA10D, the bank-driven yaw about world +Y, post-multiplied.
    const float blend = ops.interpolate_clamped_00419010(in.blend_curve_x0, 0.0f,
                                                         in.blend_curve_x1, 1.0f,
                                                         in.level_blend_input);
    AdvanceMatrix rot_y{};
    ops.build_rotation_y_00b646e0(
        rot_y, bank_yaw_angle_007da0ae(blend, in.bank_angle, in.pitch_angle,
                                       in.bank_yaw_gain, in.difficulty, in.step));
    AdvanceMatrix yawed{};
    ops.multiply_00413920(yawed, out.live_pose, rot_y);
    out.live_pose = yawed;  // 007DA112..007DA117

    // 007DA11C COMISS against 0.0f / JBE: the levelling runs only for a strictly
    // positive rate.
    if (in.level_rate > 0.0f) {
        float transformed[3];
        ops.transform_direction_0042d0d0(transformed, in.level_axis, out.live_pose);
        apply_up_levelling_007da14d(out.live_pose,
                                    transformed,
                                    level_blend_007da179(in.level_rate, in.step));
        out.levelling_applied = true;
    }

    // 007DA218..007DA2AD. The committed translation comes from unit+6A4h, which is
    // row 3 of the committed pose, not from the rotated working copy.
    float pos[3];
    const float committed[3] = {in.committed_pose.m[12], in.committed_pose.m[13],
                                in.committed_pose.m[14]};
    advance_position_007da218(pos, committed, in.linear_velocity, in.external_velocity,
                              in.step);

    // 007DA2B1..007DA2FE. The attached branch re-expresses the position through
    // the parent's world pose and then clamps it. It was not read; the flag says
    // so rather than this rule guessing at it.
    out.attached_branch_unmodelled = in.attached;

    out.live_pose.m[12] = pos[0];  // 007DA33A..007DA363
    out.live_pose.m[13] = pos[1];
    out.live_pose.m[14] = pos[2];
    return out;
}

}  // namespace bsp
