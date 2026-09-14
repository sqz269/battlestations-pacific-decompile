// Mount-frame scale rules. See include/bsp/mount_frame_scale.hpp and
// docs/MOUNT_FRAME_SCALE.md.
#include "bsp/mount_frame_scale.hpp"

#include <cmath>
#include <limits>

namespace bsp {
namespace {

float dot3(const BombVector3& a, const BombVector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float length3(const BombVector3& v) {
    return std::sqrt(dot3(v, v));
}

// |cos| between two rows, 0 when either row is degenerate - the caller reports
// that case through has_zero_row instead.
float abs_cosine(const BombVector3& a, const BombVector3& b, float la, float lb) {
    if (la <= 0.0f || lb <= 0.0f) {
        return 0.0f;
    }
    return std::fabs(dot3(a, b)) / (la * lb);
}

}  // namespace

MountFrameScale mount_frame_scale(const GunGravityArcMountFrame& basis) {
    MountFrameScale out;
    out.x = length3(basis.inverse_x_basis);
    out.y = length3(basis.inverse_y_basis);
    out.z = length3(basis.inverse_z_basis);
    out.has_zero_row = (out.x <= 0.0f) || (out.y <= 0.0f) || (out.z <= 0.0f);

    const float xy = abs_cosine(basis.inverse_x_basis, basis.inverse_y_basis, out.x, out.y);
    const float xz = abs_cosine(basis.inverse_x_basis, basis.inverse_z_basis, out.x, out.z);
    const float yz = abs_cosine(basis.inverse_y_basis, basis.inverse_z_basis, out.y, out.z);
    out.max_off_diagonal = xy;
    if (xz > out.max_off_diagonal) {
        out.max_off_diagonal = xz;
    }
    if (yz > out.max_off_diagonal) {
        out.max_off_diagonal = yz;
    }
    return out;
}

bool mount_frame_is_invertible_by_00b63d50(const MountFrameScale& scale,
                                           float orthogonality_tolerance) {
    if (scale.has_zero_row) {
        return false;
    }
    return scale.max_off_diagonal <= orthogonality_tolerance;
}

bool mount_frame_arc_is_exact(const MountFrameScale& scale,
                              float unit_tolerance,
                              float orthogonality_tolerance) {
    if (!mount_frame_is_invertible_by_00b63d50(scale, orthogonality_tolerance)) {
        return false;
    }
    return std::fabs(scale.x - 1.0f) <= unit_tolerance &&
           std::fabs(scale.y - 1.0f) <= unit_tolerance &&
           std::fabs(scale.z - 1.0f) <= unit_tolerance;
}

float mount_frame_arc_reported_pitch(float true_pitch_radians, float scale) {
    const float quiet_nan = std::numeric_limits<float>::quiet_NaN();
    if (!(scale > 0.0f)) {
        return quiet_nan;
    }
    const float y = std::sin(true_pitch_radians) / scale;
    if (!(y >= -1.0f && y <= 1.0f)) {
        return quiet_nan;
    }
    return std::asin(y);
}

float mount_frame_arc_pitch_error(float true_pitch_radians, float scale) {
    return mount_frame_arc_reported_pitch(true_pitch_radians, scale) - true_pitch_radians;
}

}  // namespace bsp
