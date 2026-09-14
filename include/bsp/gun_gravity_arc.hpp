#pragma once
// ---------------------------------------------------------------------------
// The gun gravity arc - 00955630, body 00955630..0095581F, RET 10h.
//
// Evidence, the instruction-level derivation and the failure analysis are in
// docs/GUN_GRAVITY_ARC.md. Report: reports/cc7_gun_gravity_arc.json.
//
// Native ABI, proven from the RET immediate and confirmed at all six call
// sites (006DFAD4, 0095A0AA, 0095A4E3, 005476EB, 0085B8DE, 006DEFA8):
//
//   bool __fastcall BSP_Gun_SolveGravityArc(
//           void* mount /*ECX, may be NULL*/, const float3* aimPoint /*EDX*/,
//           const float3* muzzlePos, float muzzleSpeed,
//           float* outPitch, float* outYaw);
//
// This header publishes the RULE, not that ABI: explicit inputs, a returned
// value, no host interface and no globals. It is a bounded implementation -
// reconstructed and build-tested, not ABI-compatible and not game-validated.
//
// BSP_Gun_SolveGravityArc is a hypothesis, not a recovered symbol; so are the
// parameter names below, which describe how the six call sites use the slots.
// ---------------------------------------------------------------------------
#include "bsp/bomb_torpedo_tick.hpp"       // BombVector3, the ballistics float3
#include "bsp/bot_fire_target.hpp"         // GunAimAngles, kGunBotHorzAngleNegateBase (00D7A208)
#include "bsp/gun_bot_ticks.hpp"           // kGunBotGravity (00CF9058), kGunBotQuarterPi (00CEB5A8)
#include "bsp/ship_ai_bearing_rating.hpp"  // ship_ai_firepower_wrap_angle_00605070

namespace bsp {

// 00D7A328, the double 4.0 that 009556E4 multiplies the drop term by. Kept as
// a double because the native FMUL reads eight bytes.
inline constexpr double kGunGravityArcDiscriminantFactor = 4.0;

// The three basis rows of the mount's cached affine inverse at mount+110h,
// which 00B63D50 writes from the world matrix at mount+0CCh. 0042D0D0 is
// called with normalize = 0 and uses only these three rows - the matrix is
// 4x4 with a 10h row stride and its translation row at +30h is not read.
//
// PRODUCER: 00B63D50 (body 00B63D50..00B63F09) writes the matrix; 00414DB0
// writes the world matrix it is derived from and CLEARS the mount+10Ch valid
// byte, which is why 009557AA re-sets that byte AFTER the 00414DB0 call.
struct GunGravityArcMountFrame {
    BombVector3 inverse_x_basis{};  // mount+110h, +00h..+08h
    BombVector3 inverse_y_basis{};  // mount+110h, +10h..+18h
    BombVector3 inverse_z_basis{};  // mount+110h, +20h..+28h
};

// Everything 00955630 reads. The routine has no other input: gravity is the
// immediate 9.81 at 00CF9058, not a field, and there is no drag coefficient,
// air density or time-of-flight term anywhere in the body. The lead is
// already baked into aim_point by the caller (006DF520 step 6).
struct GunGravityArcQuery {
    BombVector3 aim_point{};       // EDX, world space
    BombVector3 muzzle_position{};  // stack argument 1, world space
    float muzzle_speed{0.0f};       // stack argument 2, metres per second

    // ECX. Null reproduces the mount == NULL path, which 0085B8DE takes on
    // purpose: the world-frame pair is returned without the local-frame
    // round trip. The negate and the two wraps still run.
    const GunGravityArcMountFrame* mount_frame{nullptr};
};

// The pair 00955630 writes through outPitch/outYaw plus the boolean in AL.
// angles.vert is *outPitch (argument 5) and angles.horz is *outYaw (argument
// 6); 006DFAD4 names the locals that way. Both are radians in (-pi, pi], and
// both are written on EVERY path - see `solved`.
struct GunGravityArcSolution {
    GunAimAngles angles{};

    // AL. False means only that the discriminant went negative at 009556F8;
    // the angles are still filled in, from a pi/4 lob toward the target's
    // bearing. Callers that ignore the flag still get a usable pair.
    bool solved{false};
};

// 009556AE..009556C4: k = (hsq * g) / (v * (v + v)), the drop of a
// horizontally fired round over the horizontal distance sqrt(hsq). g is the
// double at 00CF9058; the product is formed at double width and the quotient
// is rounded to float by the FSTP at 009556C4.
float gun_gravity_arc_drop_term(float horizontal_distance_squared,
                                float muzzle_speed) noexcept;

// 009556C8..009556EE: D = R*R - 4*k*(k + h). `height_delta` is
// aim_point.y - muzzle_position.y. The native code rounds k + h to a float
// before the product (009556DA/009556DE) and this reproduces that.
float gun_gravity_arc_discriminant(float horizontal_distance,
                                   float drop_term,
                                   float height_delta) noexcept;

// DERIVED, not a native routine: the horizontal distance at which the above
// discriminant reaches zero, (v/g) * sqrt(v^2 - 2*g*h). Returns 0.0f when
// v*v <= 2*g*h, the case where no range at all has a solution.
float gun_gravity_arc_max_range(float muzzle_speed, float height_delta) noexcept;

// The whole of 00955630. Reproduces the x87 evaluation order and the float
// round trips of k, k + h, D, sqrt(D) and the tangent; it does not reproduce
// the 53-bit x87 intermediate precision, because Win32 MSVC evaluates double
// expressions at 64 bits.
GunGravityArcSolution solve_gun_gravity_arc_00955630(
    const GunGravityArcQuery& query) noexcept;

}  // namespace bsp
