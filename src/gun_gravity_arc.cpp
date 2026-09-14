// ---------------------------------------------------------------------------
// 00955630 BSP_Gun_SolveGravityArc, body 00955630..0095581F, RET 10h.
// Derivation and evidence: docs/GUN_GRAVITY_ARC.md.
//
// The native body is x87 throughout. Every `static_cast<float>` below marks an
// FSTP to a `float ptr` slot in the listing; the intermediate products and
// sums stay wide, as they do in the x87 registers. Win32 MSVC evaluates the
// wide parts at 64 bits rather than the native 80, which is the one deliberate
// departure from the original arithmetic.
// ---------------------------------------------------------------------------
#include "bsp/gun_gravity_arc.hpp"

#include <cmath>

namespace bsp {
namespace {

// The four helpers below re-project 0042B260, 004B4D80, 0042D0D0 and 0042CF10
// in this file's style. 0042B260 and 0042CF10 DO already have reconstructions -
// normalize_camera_basis_0042b260 (src/camera_decomposition.cpp) and
// camera_asin_clamped_0042cf10 (src/camera_position_modes.cpp) - but both are
// __declspec(naked) inline-assembly replicas of the native instruction stream,
// x86-MSVC-only and extern on _CIsqrt/_CIatan. This module publishes a portable
// rule with 64-bit intermediates, so calling a bit-exact replica from inside it
// would not buy exactness and would drag x86 asm into the header. The choice is
// deliberate and docs/GUN_GRAVITY_ARC.md has the table. 00605070 IS reused, and
// 0042D0D0 is re-projected because its published header pulls in D3D9.

// 00CF9058, the eight bytes 009556B2's FMUL reads: 9.81f widened to a double.
// kGunBotGravity (gun_bot_ticks.hpp) is the same number as a float.
constexpr double kGravity = static_cast<double>(kGunBotGravity);

// 0042B260's length floor: at or below 1e-10 (00CE3820) the divisor is the
// fixed 1e-5 (00CE3C70) instead of sqrt(lenSq). Both are doubles in the image.
constexpr double kNormalizeLengthSqFloor = 1e-10;
constexpr double kNormalizeLengthFloor = 1e-5;

// 00CE3C64 and 00CE3CCC, the two early outs of 0042CF10.
constexpr float kHalfPiPositive = 1.5707963705062866f;
constexpr float kHalfPiNegative = -1.5707963705062866f;

// 0042B260 BSP_Geometry_NormalizeVectorWithFloor, body 0042B260..0042B2E9.
// lenSq is (y*y + x*x) + z*z spilled to a float at 0042B296, the length is
// spilled again at 0042B2C1, and the three divisions share it.
BombVector3 normalize_with_floor_0042b260(const BombVector3& v) noexcept
{
    const float length_sq = static_cast<float>(
        (static_cast<double>(v.y) * static_cast<double>(v.y) +
         static_cast<double>(v.x) * static_cast<double>(v.x)) +
        static_cast<double>(v.z) * static_cast<double>(v.z));

    const float length =
        static_cast<double>(length_sq) > kNormalizeLengthSqFloor
            ? static_cast<float>(std::sqrt(static_cast<double>(length_sq)))
            : static_cast<float>(kNormalizeLengthFloor);

    BombVector3 out;
    out.x = static_cast<float>(static_cast<double>(v.x) / static_cast<double>(length));
    out.y = static_cast<float>(static_cast<double>(v.y) / static_cast<double>(length));
    out.z = static_cast<float>(static_cast<double>(v.z) / static_cast<double>(length));
    return out;
}

// 004B4D80 BSP_Lighting_DirectionFromAngles, body 004B4D80..004B4DF0, RET 8.
// Each FSIN/FCOS result is spilled to a float before the two products.
BombVector3 direction_from_angles_004b4d80(float pitch, float yaw) noexcept
{
    const float cos_pitch = static_cast<float>(std::cos(static_cast<double>(pitch)));
    const float cos_yaw = static_cast<float>(std::cos(static_cast<double>(yaw)));
    const float sin_pitch = static_cast<float>(std::sin(static_cast<double>(pitch)));
    const float sin_yaw = static_cast<float>(std::sin(static_cast<double>(yaw)));

    BombVector3 out;
    out.x = static_cast<float>(static_cast<double>(sin_yaw) * static_cast<double>(cos_pitch));
    out.y = sin_pitch;
    out.z = static_cast<float>(static_cast<double>(cos_yaw) * static_cast<double>(cos_pitch));
    return out;
}

// 0042D0D0 BSP_Vector3f_TransformDirectionOptionalNormalize, body
// 0042D0D0..0042D188, RET 8, with the normalize byte zero - the branch
// src/material_effect_plane.cpp publishes as
// transform_effect_direction_0042d0d0_no_normalize. Each row is
// (m0*v.x + m1*v.y) + m2*v.z with a single FSTP at the end; no translation.
BombVector3 transform_direction_0042d0d0(const GunGravityArcMountFrame& frame,
                                         const BombVector3& v) noexcept
{
    BombVector3 out;
    out.x = static_cast<float>(
        (static_cast<double>(frame.inverse_x_basis.x) * static_cast<double>(v.x) +
         static_cast<double>(frame.inverse_y_basis.x) * static_cast<double>(v.y)) +
        static_cast<double>(frame.inverse_z_basis.x) * static_cast<double>(v.z));
    out.y = static_cast<float>(
        (static_cast<double>(frame.inverse_x_basis.y) * static_cast<double>(v.x) +
         static_cast<double>(frame.inverse_y_basis.y) * static_cast<double>(v.y)) +
        static_cast<double>(frame.inverse_z_basis.y) * static_cast<double>(v.z));
    out.z = static_cast<float>(
        (static_cast<double>(frame.inverse_x_basis.z) * static_cast<double>(v.x) +
         static_cast<double>(frame.inverse_y_basis.z) * static_cast<double>(v.y)) +
        static_cast<double>(frame.inverse_z_basis.z) * static_cast<double>(v.z));
    return out;
}

// 0042CF10 BSP_Geometry_AsinClamped, body 0042CF10..0042CF9C, RET 4. Outside
// +-1.0 (00D7A24C / 00D7A260) it returns +-pi/2 outright; inside it evaluates
// the half-angle identity 2*atan(x / (1 + sqrt(1 - x*x))), where the 1 is the
// double at 00D7A210 and the numerator is the double-widened argument saved by
// 0042CF4B. Mathematically asin; the float spills are what differ from
// std::asin. A NaN argument fails both COMISS tests and reaches the identity,
// which propagates it.
float asin_clamped_0042cf10(float x) noexcept
{
    if (x > 1.0f) {
        return kHalfPiPositive;
    }
    if (-1.0f > x) {
        return kHalfPiNegative;
    }
    const float squared = static_cast<float>(static_cast<double>(x) * static_cast<double>(x));
    const float remainder = static_cast<float>(1.0 - static_cast<double>(squared));
    const float root = static_cast<float>(std::sqrt(static_cast<double>(remainder)));
    const float quotient =
        static_cast<float>(static_cast<double>(x) / (static_cast<double>(root) + 1.0));
    const float half = static_cast<float>(std::atan(static_cast<double>(quotient)));
    return static_cast<float>(static_cast<double>(half) + static_cast<double>(half));
}

// 00521370 BSP_Direction_ToPitchYaw's own clamp at 00521374..00521394:
// -1.0 (00D7A250) below, FLD1 above. A NaN survives both FCOMI branches.
float clamp_to_unit_00521374(float value) noexcept
{
    if (-1.0f > value) {
        return -1.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

}  // namespace

float gun_gravity_arc_drop_term(float horizontal_distance_squared,
                                float muzzle_speed) noexcept
{
    // 009556AE..009556C4: (hsq * g) / (v * (v + v)), one FSTP at the end.
    const double numerator = static_cast<double>(horizontal_distance_squared) * kGravity;
    const double speed = static_cast<double>(muzzle_speed);
    return static_cast<float>(numerator / (speed * (speed + speed)));
}

float gun_gravity_arc_discriminant(float horizontal_distance,
                                   float drop_term,
                                   float height_delta) noexcept
{
    // 009556C8..009556EE. R*R stays in a register; k + h is round-tripped
    // through a float slot at 009556DA/009556DE; the 4.0 multiplies k first.
    const double distance = static_cast<double>(horizontal_distance);
    const double distance_sq = distance * distance;
    const float sum = static_cast<float>(static_cast<double>(drop_term) +
                                         static_cast<double>(height_delta));
    const double scaled_drop =
        static_cast<double>(drop_term) * kGunGravityArcDiscriminantFactor;
    return static_cast<float>(distance_sq - static_cast<double>(sum) * scaled_drop);
}

float gun_gravity_arc_max_range(float muzzle_speed, float height_delta) noexcept
{
    // Derived from the discriminant, not a native routine: with
    // k = g*R^2/(2*v^2), D = (R^2/v^4) * (v^4 - 2*g*h*v^2 - g^2*R^2), so the
    // solvable band is R <= (v/g) * sqrt(v^2 - 2*g*h).
    const double speed = static_cast<double>(muzzle_speed);
    const double inner = speed * speed - 2.0 * kGravity * static_cast<double>(height_delta);
    if (!(inner > 0.0)) {
        return 0.0f;
    }
    return static_cast<float>((speed / kGravity) * std::sqrt(inner));
}

GunGravityArcSolution solve_gun_gravity_arc_00955630(
    const GunGravityArcQuery& query) noexcept
{
    GunGravityArcSolution result;

    // 00955637..00955655: three float subtractions, each spilled to the frame.
    BombVector3 delta;
    delta.x = query.aim_point.x - query.muzzle_position.x;
    delta.y = query.aim_point.y - query.muzzle_position.y;
    delta.z = query.aim_point.z - query.muzzle_position.z;

    // 00955659..00955669: dz*dz + dx*dx, in that operand order.
    const float horizontal_distance_sq = static_cast<float>(
        static_cast<double>(delta.z) * static_cast<double>(delta.z) +
        static_cast<double>(delta.x) * static_cast<double>(delta.x));

    // 0095566D..00955682: R, spilled twice before 0042B260 destroys the delta.
    const float horizontal_distance =
        static_cast<float>(std::sqrt(static_cast<double>(horizontal_distance_sq)));

    // 0095568E then 00955693..009556AC. The yaw is produced FIRST, in the
    // world frame, and is never rewritten by the failure path.
    const BombVector3 unit_delta = normalize_with_floor_0042b260(delta);
    float yaw = static_cast<float>(std::atan2(static_cast<double>(unit_delta.x),
                                              static_cast<double>(unit_delta.z)));

    const float drop_term = gun_gravity_arc_drop_term(horizontal_distance_sq, query.muzzle_speed);
    const float discriminant =
        gun_gravity_arc_discriminant(horizontal_distance, drop_term, delta.y);

    // 009556F2..009556FA: FLDZ / FCOMIP / JBE. The fall-through - the failure
    // path - needs CF = ZF = 0, so it is reached on exactly `0.0 > D`. An
    // unordered compare sets both flags, so a NaN discriminant solves.
    float pitch;
    result.solved = !(discriminant < 0.0f);
    if (result.solved) {
        // 0095571B..0095574D: the MINUS root, i.e. the low flat trajectory.
        // k*T^2 - R*T + (k + h) = 0 with T = tan(pitch).
        const float root = static_cast<float>(std::sqrt(static_cast<double>(discriminant)));
        const float tangent = static_cast<float>(
            (static_cast<double>(horizontal_distance) - static_cast<double>(root)) /
            (static_cast<double>(drop_term) + static_cast<double>(drop_term)));
        pitch = static_cast<float>(std::atan(static_cast<double>(tangent)));
    } else {
        // 00955702: pi/4 from 00CEB5A8, the same constant 006DF520 step 6
        // clamps its pre-estimate to.
        pitch = kGunBotQuarterPi;
    }

    // 00955752..00955765, on BOTH paths.
    const BombVector3 world_direction = direction_from_angles_004b4d80(pitch, yaw);

    // 0095576A..009557F3. Skipped entirely when the mount is NULL, which
    // 0085B8DE does on purpose; the caller signals that by leaving
    // mount_frame null.
    if (query.mount_frame != nullptr) {
        const BombVector3 local =
            transform_direction_0042d0d0(*query.mount_frame, world_direction);
        pitch = asin_clamped_0042cf10(clamp_to_unit_00521374(local.y));
        yaw = static_cast<float>(
            std::atan2(static_cast<double>(local.x), static_cast<double>(local.z)));
    }

    // 009557F8: -0.0f (00D7A208) minus the yaw - an exact negation, applied
    // unconditionally, world frame included.
    yaw = kGunBotHorzAngleNegateBase - yaw;

    // 0095580A then 0095580F: 00605070 in place on the pitch, then the yaw.
    result.angles.vert = ship_ai_firepower_wrap_angle_00605070(pitch);
    result.angles.horz = ship_ai_firepower_wrap_angle_00605070(yaw);
    return result;
}

}  // namespace bsp
