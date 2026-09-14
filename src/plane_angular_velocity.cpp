#include "bsp/plane_angular_velocity.hpp"
#include "bsp/unit_rudder.hpp"

#include <cmath>

// docs/PLANE_ANGULAR_VELOCITY.md carries the evidence and the coverage table;
// every name is a hypothesis, not a recovered symbol.

namespace bsp {
namespace {

// The same length and reciprocal guard the whole family shares: 00419440, and
// the `len > 0 ? 1/len : +0.0f` at 00B63F26 / 00B63FB0, which is byte for byte
// the guard in 00419510 and 0085DC96.
float length3(const float v[3]) {
    const float xx = v[0] * v[0];
    const float yy = v[1] * v[1];
    const float zz = v[2] * v[2];
    return std::sqrt(static_cast<float>(static_cast<double>(xx) +
                                        static_cast<double>(yy) +
                                        static_cast<double>(zz)));
}

void scale_by_reciprocal(float v[3], float len) {
    const float inv = len > 0.0f ? 1.0f / len : 0.0f;
    const float x = v[0] * inv;
    const float y = v[1] * inv;
    const float z = v[2] * inv;
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

// 004F9B30, out = a x b, traced in docs/PLANE_POSE_COMMIT.md.
void cross3(float out[3], const float a[3], const float b[3]) {
    const double x = static_cast<double>(a[1]) * b[2] - static_cast<double>(a[2]) * b[1];
    const double y = static_cast<double>(a[2]) * b[0] - static_cast<double>(a[0]) * b[2];
    const double z = static_cast<double>(a[0]) * b[1] - static_cast<double>(a[1]) * b[0];
    out[0] = static_cast<float>(x);
    out[1] = static_cast<float>(y);
    out[2] = static_cast<float>(z);
}

float dot3(const float a[3], const float b[3]) {
    return static_cast<float>(static_cast<double>(a[0]) * b[0] +
                              static_cast<double>(a[1]) * b[1] +
                              static_cast<double>(a[2]) * b[2]);
}

void identity(AdvanceMatrix& m) {
    for (int i = 0; i < 16; ++i) {
        m.m[i] = 0.0f;
    }
    m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0f;
}

}  // namespace

void build_look_at_lh_00b63f10(AdvanceMatrix& out, const float eye[3],
                               const float target[3], const float up[3]) {
    // 00B63F1D: the up vector is normalised first, before anything else.
    float yaxis[3] = {up[0], up[1], up[2]};
    scale_by_reciprocal(yaxis, length3(yaxis));

    // 00B63F87..00B63FEF: zaxis = normalize(target - eye). The subtraction order
    // is what makes this the LEFT-handed look-at, and it is the single fact the
    // rotation sense turns on.
    float zaxis[3] = {target[0] - eye[0], target[1] - eye[1], target[2] - eye[2]};
    scale_by_reciprocal(zaxis, length3(zaxis));

    // 00B64113..00B64129: xaxis = normalize(up x zaxis). 00B64117 PUSH EAX makes
    // zaxis the second operand, 00B64118 LEA EDX makes the up slot the first.
    float xaxis[3];
    cross3(xaxis, yaxis, zaxis);
    scale_by_reciprocal(xaxis, length3(xaxis));

    // 00B6415D..00B641A1: yaxis = normalize(zaxis x xaxis). 00B6415D PUSH ECX is
    // xaxis, 00B64162 LEA EDX is zaxis. Note the order is (zaxis, xaxis), which
    // is what makes (x, y, z) a right-handed triple: cross(x, y) = z.
    cross3(yaxis, zaxis, xaxis);
    scale_by_reciprocal(yaxis, length3(yaxis));

    // 00B641D7..00B6425D: the axes go into the COLUMNS.
    out.m[0] = xaxis[0];  out.m[1] = yaxis[0];  out.m[2] = zaxis[0];  out.m[3] = 0.0f;
    out.m[4] = xaxis[1];  out.m[5] = yaxis[1];  out.m[6] = zaxis[1];  out.m[7] = 0.0f;
    out.m[8] = xaxis[2];  out.m[9] = yaxis[2];  out.m[10] = zaxis[2]; out.m[11] = 0.0f;

    // 00B64260..00B64296 and the two that follow it.
    out.m[12] = -dot3(xaxis, eye);
    out.m[13] = -dot3(yaxis, eye);
    out.m[14] = -dot3(zaxis, eye);
    out.m[15] = 1.0f;  // 00B64248, the 1.0f at 00D7A24C
}

void build_rotation_z_00b64780(AdvanceMatrix& out, float angle) {
    // 00B6478E FSIN and 00B64705 FCOS, both on a float spill of [EDX].
    const float s = static_cast<float>(std::sin(static_cast<double>(angle)));
    const float c = static_cast<float>(std::cos(static_cast<double>(angle)));
    identity(out);
    out.m[0] = c;   // 00B647C0
    out.m[1] = s;   // 00B647D5
    out.m[4] = -s;  // 00B647E4, the -0.0f - s at 00D7A208
    out.m[5] = c;   // 00B647C4
}

void build_rotation_y_00b646e0(AdvanceMatrix& out, float angle) {
    // 00B646EE FSIN and 00B64705 FCOS, the same spill shape.
    const float s = static_cast<float>(std::sin(static_cast<double>(angle)));
    const float c = static_cast<float>(std::cos(static_cast<double>(angle)));
    identity(out);
    out.m[0] = c;    // 00B64731
    out.m[2] = -s;   // 00B64724
    out.m[8] = s;    // 00B64753
    out.m[10] = c;   // 00B6475D
}

void multiply_00413920(AdvanceMatrix& dst, const AdvanceMatrix& left,
                       const AdvanceMatrix& right) {
    // Written through a temporary because 0085E81D chains a result straight back
    // in as the next left, so dst may alias either operand.
    AdvanceMatrix tmp;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            double sum = 0.0;
            for (int k = 0; k < 4; ++k) {
                sum += static_cast<double>(left.m[4 * i + k]) * right.m[4 * k + j];
            }
            tmp.m[4 * i + j] = static_cast<float>(sum);
        }
    }
    dst = tmp;
}

void transform_direction_0042d0d0(float out[3], const float v[3], const AdvanceMatrix& m) {
    // 0042D0F2..0042D116: column 0 is m[0], m[4], m[8]. No translation row.
    const float x = static_cast<float>(static_cast<double>(v[0]) * m.m[0] +
                                       static_cast<double>(v[1]) * m.m[4] +
                                       static_cast<double>(v[2]) * m.m[8]);
    const float y = static_cast<float>(static_cast<double>(v[0]) * m.m[1] +
                                       static_cast<double>(v[1]) * m.m[5] +
                                       static_cast<double>(v[2]) * m.m[9]);
    const float z = static_cast<float>(static_cast<double>(v[0]) * m.m[2] +
                                       static_cast<double>(v[1]) * m.m[6] +
                                       static_cast<double>(v[2]) * m.m[10]);
    out[0] = x;
    out[1] = y;
    out[2] = z;
}

void NativeAdvanceMatrixOps::build_look_at_00b63f10(AdvanceMatrix& out, const float eye[3],
                                                    const float target[3], const float up[3]) {
    build_look_at_lh_00b63f10(out, eye, target, up);
}
void NativeAdvanceMatrixOps::build_rotation_z_00b64780(AdvanceMatrix& out, float angle) {
    bsp::build_rotation_z_00b64780(out, angle);
}
void NativeAdvanceMatrixOps::build_rotation_y_00b646e0(AdvanceMatrix& out, float angle) {
    bsp::build_rotation_y_00b646e0(out, angle);
}
void NativeAdvanceMatrixOps::multiply_00413920(AdvanceMatrix& dst, const AdvanceMatrix& left,
                                               const AdvanceMatrix& right) {
    bsp::multiply_00413920(dst, left, right);
}
void NativeAdvanceMatrixOps::transform_direction_0042d0d0(float out[3], const float v[3],
                                                          const AdvanceMatrix& m) {
    bsp::transform_direction_0042d0d0(out, v, m);
}
float NativeAdvanceMatrixOps::interpolate_clamped_00419010(float x0, float y0, float x1,
                                                           float y1, float x) {
    return clamped_interpolate_00419010(x0, y0, x1, y1, x);
}

void rotate_about_axis_equivalent(AdvanceMatrix& out, const AdvanceMatrix& in,
                                  const float w[3]) {
    out = in;

    // 0085E4F0..0085E544's two guards, reproduced so the fast path agrees with
    // the native on when *not* to rotate at all.
    const float len2 = static_cast<float>(static_cast<double>(w[0]) * w[0] +
                                          static_cast<double>(w[1]) * w[1] +
                                          static_cast<double>(w[2]) * w[2]);
    const float len = static_cast<double>(len2) > kRotationLengthSqFloor_00ce3820
                          ? std::sqrt(len2)
                          : 0.0f;
    if (kRotationAngleFloor_00d7a350 > static_cast<double>(len)) {
        return;  // 0085E871 writes nothing
    }

    const float axis[3] = {w[0] / len, w[1] / len, w[2] / len};

    // The sense: -len, not +len. 0085E750 FCHS supplies one negation, and
    // 00B63F10 being the LEFT-handed look-at means view +Z is +axis rather than
    // -axis, so nothing cancels it. See the doc.
    const double theta = -static_cast<double>(len);
    const double c = std::cos(theta);
    const double s = std::sin(theta);
    const double t = 1.0 - c;
    const double ax = axis[0];
    const double ay = axis[1];
    const double az = axis[2];

    // Rodrigues, in the row-vector convention: a row vector v becomes v * R, so
    // R is the transpose of the usual column-vector form.
    const double r[9] = {
        t * ax * ax + c,        t * ax * ay + s * az,   t * ax * az - s * ay,
        t * ax * ay - s * az,   t * ay * ay + c,        t * ay * az + s * ax,
        t * ax * az + s * ay,   t * ay * az - s * ax,   t * az * az + c,
    };

    for (int row = 0; row < 3; ++row) {
        const double x = in.m[4 * row + 0];
        const double y = in.m[4 * row + 1];
        const double z = in.m[4 * row + 2];
        out.m[4 * row + 0] = static_cast<float>(x * r[0] + y * r[3] + z * r[6]);
        out.m[4 * row + 1] = static_cast<float>(x * r[1] + y * r[4] + z * r[7]);
        out.m[4 * row + 2] = static_cast<float>(x * r[2] + y * r[5] + z * r[8]);
    }
    // Row 3 stays as `in` had it, matching 0085E833's restore.
}

void world_to_body_007d9c10(ControllerVelocities& v, const AdvanceMatrix& world_to_body) {
    // 007D9C2D..007D9C51 and 007D9C47..007D9C6A. Both transforms pass
    // normalise = 0 (007D9C2A and 007D9C3E PUSH 0).
    transform_direction_0042d0d0(v.linear_body, v.linear_world, world_to_body);
    transform_direction_0042d0d0(v.angular_body, v.angular_world, world_to_body);
}

void body_to_world_007d9c80(ControllerVelocities& v, const AdvanceMatrix& live_pose) {
    // 007D9C8F..007D9CB5 and 007D9CB2..007D9CD1, the mirror, through unit+74h.
    transform_direction_0042d0d0(v.linear_world, v.linear_body, live_pose);
    transform_direction_0042d0d0(v.angular_world, v.angular_body, live_pose);
}

}  // namespace bsp
