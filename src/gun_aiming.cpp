// Gun aiming and automatic fire. Evidence: docs/GUN_AIMING.md,
// reports/gun_aiming.json. Every routine below names the native address it
// projects; none of them is a binary-compatible replacement.
#include "bsp/gun_aiming.hpp"

#include <cmath>

namespace bsp {
namespace {

// 0085AE19 and 0085AE42 call BSP_Math_SubtractWrappedAngle 00438B10. Its body was
// not read for this packet; the projection is the shortest signed difference,
// which is what the two uses require (the caller takes the absolute value for the
// dead band and the sign for the step direction).
float wrapped_difference_00438b10(float target, float current) noexcept
{
    return gun_wrap_angle_0085abbd(target - current);
}

// 0085AFA6 and 0085B094 call BSP_Math_InterpolateClamped 00419010 with
// (0, 0.5, 10deg, 1.0, remaining). The five arguments are a clamped linear remap
// from [0, 10deg] onto [0.5, 1.0].
float interpolate_clamped_00419010(float lo_in,
                                   float lo_out,
                                   float hi_in,
                                   float hi_out,
                                   float x) noexcept
{
    if (hi_in <= lo_in) {
        return lo_out;
    }
    float t = (x - lo_in) / (hi_in - lo_in);
    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }
    return lo_out + (hi_out - lo_out) * t;
}

} // namespace

float gun_wrap_angle_0085abbd(float radians) noexcept
{
    // 0085ABBD: fmod(a, 2pi), then one correction in each direction. The native
    // code tests `-pi < a` first, so exactly -pi takes the add branch.
    float a = std::fmod(radians, kGunAimTwoPi);
    if (!(a > -kGunAimPi)) {
        a += kGunAimTwoPi;
    } else if (a > kGunAimPi) {
        a -= kGunAimTwoPi;
    }
    return a;
}

float gun_clamp_horizontal_007f5fc0(float radians) noexcept
{
    // 007F5FC0's first block: above +pi becomes +pi (00D7A264), below -pi becomes
    // -pi (00CE684C). This is a clamp, not a wrap; the callers have already wrapped.
    if (radians > kGunAimPi) {
        return kGunAimPi;
    }
    if (radians < -kGunAimPi) {
        return -kGunAimPi;
    }
    return radians;
}

bool gun_arc_contains_007f5fc0(const GunFiringArc& arc, float horz, float vert) noexcept
{
    const float h = gun_clamp_horizontal_007f5fc0(horz);
    const float eps = kGunAimArcEpsilon;
    return arc.min_horz <= h + eps && h - eps <= arc.max_horz
        && arc.min_vert <= vert + eps && vert - eps <= arc.max_vert;
}

bool gun_traverse_allowed_007f5fc0(const GunPlatformArcs& arcs, float horz, float vert) noexcept
{
    for (std::size_t i = 0; i < arcs.count; ++i) {
        const GunFiringArc& arc = arcs.first[i];
        if (gun_arc_contains_007f5fc0(arc, horz, vert) && (arc.flags & kGunArcFlagTraverse) != 0) {
            return true;
        }
    }
    return false;
}

bool gun_fire_allowed_007f60a0(const GunPlatformArcs& arcs, float horz, float vert) noexcept
{
    // 007F60A0 differs from 007F5FC0 in the flag bit only.
    for (std::size_t i = 0; i < arcs.count; ++i) {
        const GunFiringArc& arc = arcs.first[i];
        if (gun_arc_contains_007f5fc0(arc, horz, vert) && (arc.flags & kGunArcFlagFire) != 0) {
            return true;
        }
    }
    return false;
}

bool gun_arc_contains_horizontal_007f5960(const GunFiringArc& arc, float horz) noexcept
{
    const float h = gun_clamp_horizontal_007f5fc0(horz);
    const float eps = kGunAimArcEpsilon;
    return arc.min_horz <= h + eps && h - eps <= arc.max_horz;
}

bool gun_set_target_angles_0085aba0(GunTurningAngles& angles,
                                    const GunPlatformArcs& arcs,
                                    const GunRotationSpeeds& speeds,
                                    float horz,
                                    float vert,
                                    float& accepted_mark) noexcept
{
    // 0085ABB5, before any test.
    accepted_mark = kGunAimAcceptedMarkValue;

    const float h = gun_wrap_angle_0085abbd(horz);
    const float v = gun_wrap_angle_0085abbd(vert);

    if (!gun_traverse_allowed_007f5fc0(arcs, h, v)) { // 0085AC94
        return false;
    }
    if (speeds.vert == 0.0f) { // 0085ACA5
        return false;
    }
    if (speeds.horz == 0.0f) { // 0085ACB9
        return false;
    }
    angles.target_horz = h; // 0085ACD0
    angles.target_vert = v; // 0085ACDE
    return true;
}

float gun_soft_approach_scale_00419010(float remaining_radians) noexcept
{
    return interpolate_clamped_00419010(0.0f,
                                        kGunAimSoftApproachFloor,
                                        kGunAimSoftApproachSpan,
                                        1.0f,
                                        remaining_radians);
}

float gun_step_axis_0085ad80(float delta, float rate, float dt, bool is_rapid_fixed_slave) noexcept
{
    // 0085AEEB / 0085AFE2: the per-step clamp. The native branch writes
    // sign(delta) * rate * dt only when the unclamped magnitude exceeds it.
    float step = delta;
    const float limit = rate * dt;
    if (limit < std::fabs(step)) {
        step = (step >= 0.0f) ? limit : -limit;
    }
    // 0085AF76 / 0085B064: every class except MRFSGun eases in over the last ten
    // degrees. The scale is taken from the magnitude that remains, not the step.
    if (!is_rapid_fixed_slave) {
        step *= gun_soft_approach_scale_00419010(std::fabs(delta));
    }
    return step;
}

bool gun_aim_settled_0085ae4a(const GunTurningAngles& angles) noexcept
{
    const float dh = std::fabs(wrapped_difference_00438b10(angles.target_horz, angles.horz));
    const float dv = std::fabs(wrapped_difference_00438b10(angles.target_vert, angles.vert));
    return dh < kGunAimDeadBand && dv < kGunAimDeadBand;
}

bool gun_step_aim_0085ad80(GunTurningAngles& angles,
                           const GunRotationSpeeds& speeds,
                           const GunStepDeltas& deltas,
                           float dt,
                           bool is_rapid_fixed_slave) noexcept
{
    // 0085ADEB and 0085ADF9: an axis with no rotation speed snaps to its target
    // and takes no part in the rest of the step.
    if (speeds.horz <= 0.0f) {
        angles.horz = angles.target_horz;
    }
    if (speeds.vert <= 0.0f) {
        angles.vert = angles.target_vert;
    }
    // 0085AE4A: the dead band ends the step before the platform is touched.
    if (gun_aim_settled_0085ae4a(angles)) {
        return false;
    }
    const float remaining_horz =
        std::fabs(wrapped_difference_00438b10(angles.target_horz, angles.horz));
    const float remaining_vert =
        std::fabs(wrapped_difference_00438b10(angles.target_vert, angles.vert));

    float step_horz = deltas.horz;
    const float limit_horz = speeds.horz * dt;
    if (limit_horz < std::fabs(step_horz)) {
        step_horz = (step_horz >= 0.0f) ? limit_horz : -limit_horz;
    }
    if (!is_rapid_fixed_slave) {
        step_horz *= gun_soft_approach_scale_00419010(remaining_horz);
    }
    angles.horz += step_horz;

    float step_vert = deltas.vert;
    const float limit_vert = speeds.vert * dt;
    if (limit_vert < std::fabs(step_vert)) {
        step_vert = (step_vert >= 0.0f) ? limit_vert : -limit_vert;
    }
    if (!is_rapid_fixed_slave) {
        step_vert *= gun_soft_approach_scale_00419010(remaining_vert);
    }
    angles.vert += step_vert;

    // 0085B0C5 calls 007F6840 on the stepped pair and restores the traverse angle
    // when it answers false. That routine was not read, so the caller runs it.
    return step_horz != 0.0f || step_vert != 0.0f;
}

bool gun_can_fire_00729a80(const GunFireGateInputs& in, bool check_reload) noexcept
{
    if (!in.fire_params_armed) { // [gun+3F8h]+34h
        return false;
    }
    if (in.disabled) { // gun+3B8h
        return false;
    }
    if (in.damage_counter > 0) { // gun+358h
        return false;
    }
    if (check_reload && (in.barrel_delay_time > 0.0f || in.secondary_delay > 0.0f)) {
        return false;
    }
    if (in.unit_cooldown_applies && in.unit_fire_cooldown > 0.0f) { // unit+6F8h
        return false;
    }
    if (in.weapon_type_id == kWeaponTypeTorpedo && in.unit_torpedo_cooldown > 0.0f) { // unit+6FCh
        return false;
    }
    // The muzzle must be at or above the waterline, or both submerged-fire tests
    // must answer no.
    const bool above_water = in.muzzle_world_y >= kGunMuzzleWaterlineY;
    if (!above_water && (in.muzzle_submerged || in.muzzle_blocked)) {
        return false;
    }
    // The only path that answers true: some barrel is out of reload.
    for (int i = 0; i < in.barrel_count; ++i) {
        if (!check_reload) {
            return true;
        }
        if (in.reload_timers != nullptr && in.reload_timers[i] <= 0.0f) {
            return true;
        }
    }
    return false;
}

bool gun_can_fire_turning_0085a830(bool fire_inhibited,
                                   const GunPlatformArcs& arcs,
                                   const GunTurningAngles& angles,
                                   const GunFireGateInputs& in,
                                   bool check_reload) noexcept
{
    if (fire_inhibited) { // 0085A833, gun+490h
        return false;
    }
    // 0085A887: the CURRENT angles, not the target pair, against a fire window.
    if (!gun_fire_allowed_007f60a0(arcs, angles.horz, angles.vert)) {
        return false;
    }
    return gun_can_fire_00729a80(in, check_reload); // 0085A899
}

void gun_update_0085a270(GunAimHost& host,
                         const GunTurningAngles& angles,
                         const GunBarrelRecoilView* barrels,
                         int barrel_count,
                         float dt)
{
    host.base_gun_update_0072b2d0(dt); // 0085A27E
    if (dt <= 0.0f) {                  // 0085A286
        return;
    }
    for (int i = 0; i < barrel_count; ++i) {
        if (barrels == nullptr || !barrels[i].active) { // 0085A340
            continue;
        }
        float forward[3] = {0.0f, 0.0f, 0.0f};
        host.barrel_node_forward_00b6db60(i, forward); // 0085A348
        float offset[3] = {0.0f, 0.0f, 0.0f};
        host.barrel_platform_offset(i, offset);
        const float dist = barrels[i].dist; // barrel+4h
        const float point[3] = {
            offset[0] + forward[0] * dist,
            offset[1] + forward[1] * dist,
            offset[2] + forward[2] * dist,
        };
        host.set_barrel_point_vtable_2ch(i, point); // 0085A3A3
    }
    host.apply_angles_to_nodes_00859550(angles.horz, angles.vert); // 0085A3B9
}

bool gun_fire_if_ready_00727e30(GunAimHost& host)
{
    if (!host.can_fire_vtable_1d0h(true)) { // 00727E3D
        return false;
    }
    host.fire_vtable_1d8h(0, 0.0f, 0.0f); // the call after 00727E47
    return true;
}

} // namespace bsp
