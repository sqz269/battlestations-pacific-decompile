#include "bsp/plane_attitude_angles.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"

namespace bsp {
namespace {

// 00CE3820 and 00D7A23C, the two guards on the cross product's length.
constexpr float kCrossLengthSquaredFloor = 1.0842e-10f;
constexpr float kCrossLengthFloor = 0.001f;
// 00CE3C64 = 3FC90FDBh, the FLOAT pi/2 - not the double. The heading is
// `SubtractWrappedAngle(pi/2, raw)` against that value.
constexpr float kFloatHalfPi = 1.5707964f;
// 00CE3830 and 00CE3828, both the float constant widened to double: the bearing
// arm subtracts from 1.5707963705062866 and wraps with 6.2831854820251465.
constexpr double kBearingHalfPi = 1.5707963705062866;
constexpr double kBearingTwoPi = 6.2831854820251465;

}  // namespace

PlaneAttitudeAngles plane_attitude_angles_007c1900(const AdvanceMatrix& world_pose,
                                                   AdvanceMatrixOps& ops) noexcept {
    PlaneAttitudeAngles out;

    // 007C18F9-007C1966. The forward row, matrix offsets +20h/+24h/+28h.
    const float fx = world_pose.m[8];
    const float fy = world_pose.m[9];
    const float fz = world_pose.m[10];

    // The horizontal magnitude, through the same sqrt the native calls at
    // 007C1932. atan2's second operand is therefore never negative, so no wrap
    // is applied and the range is [-pi/2, +pi/2] with positive meaning nose up.
    const float horizontal = std::sqrt(fx * fx + fz * fz);
    out.pitch = std::atan2(fy, horizontal);

    // 007C195F-007C19D4. The axis is `fwd x worldUp` with worldUp = (0,1,0),
    // which is (-fz, 0, fx) - horizontal and perpendicular to the forward axis.
    // Both guards leave the heading and bank untouched when it is degenerate,
    // which is a forward axis pointing straight up or straight down.
    const float cross[3] = {-fz, 0.0f, fx};
    const float cross_len_sq = cross[0] * cross[0] + cross[2] * cross[2];
    if (cross_len_sq <= kCrossLengthSquaredFloor) {
        return out;
    }
    const float cross_len = std::sqrt(cross_len_sq);
    if (cross_len <= kCrossLengthFloor) {
        return out;
    }
    const float axis[3] = {cross[0] / cross_len, 0.0f, cross[2] / cross_len};

    // 007C1A14. Rotating the pose about that axis by the pitch lays the forward
    // row flat: 0085E4D0 turns by -|w| * scale, and the axis passed here is
    // already unit length, so the turn is exactly -pitch.
    static const float eye_00f87574[3] = {0.0f, 0.0f, 0.0f};
    AdvanceMatrix flattened{};
    if (!rotate_about_axis_0085e4d0(flattened, world_pose, axis, out.pitch,
                                    eye_00f87574, ops)) {
        return out;
    }

    // 007C1A19-007C1ACA. The flattened forward row is (cos raw, 0, sin raw)
    // measured from +X, and the stored heading is pi/2 minus it, wrapped - which
    // is the same angle measured from +Z toward +X, the sense unit+C6Ch is read
    // in everywhere else.
    const float raw = std::atan2(flattened.m[10], flattened.m[8]);
    out.heading = wrapped_angle_subtract_00438b10(kFloatHalfPi, raw);

    // 007C1A47-007C1A94. Multiplying the flattened pose by RotationY(raw) puts
    // the forward row exactly on +X, so what is left in row 1 is a pure roll
    // about it and the bank is read straight off. Doing it this way rather than
    // by a closed form is deliberate: the residual is what the native reads, and
    // a hand derivation is where the sign would go missing.
    AdvanceMatrix rotation_y{};
    ops.build_rotation_y_00b646e0(rotation_y, raw);
    AdvanceMatrix residual{};
    ops.multiply_00413920(residual, flattened, rotation_y);
    out.bank = std::atan2(residual.m[6], residual.m[5]);   // m12, m11

    out.heading_and_bank_written = true;
    return out;
}

float plane_bearing_to_target_009ac190(const float self_position[3],
                                       const float target_position[3]) noexcept {
    // 009AC1E0-009AC1F1: only x and z. The y components are never read, so this
    // is a ground-plane bearing and a target directly above the plane has the
    // same bearing as one directly ahead of it.
    const double dx = static_cast<double>(target_position[0]) -
                      static_cast<double>(self_position[0]);
    const double dz = static_cast<double>(target_position[2]) -
                      static_cast<double>(self_position[2]);

    // 009AC235-009AC25C. The same pi/2-minus-atan2 form the heading uses, then a
    // single conditional 2*pi add rather than a general wrap - the native adds
    // once if the result is negative and does not loop.
    double bearing = kBearingHalfPi - std::atan2(dz, dx);
    if (bearing < 0.0) {
        bearing += kBearingTwoPi;
    }
    return static_cast<float>(bearing);
}

}  // namespace bsp
