#include "bsp/unit_forces.hpp"

#include <cmath>

// docs/UNIT_FORCE_COMMANDS.md. Every expression below is the native one in the native
// order; the x87 sequences keep their intermediates wider than float, which the double
// temporaries model. The absolute values are written as the listing writes them,
// `x <= 0.0f ? -0.0f - x : x`, because -0.0f - 0.0f is -0.0f where fabsf would give +0.0f.

namespace bsp {
namespace {

// 00937489 and friends: the listing's absolute value, 00D7A208 minus the value.
inline float native_abs(float x) noexcept {
    return (x <= kUnitForceZero) ? (kUnitForceNegativeZero - x) : x;
}

} // namespace

// ---------------------------------------------------------------------------
// The leak model.
// ---------------------------------------------------------------------------

UnitLeakTickResult unit_leak_tick_0074f930(UnitLeakEntry* entries, std::uint32_t count,
                                           float rate_cap, float capacity,
                                           float settings_flood_scale, float unit_health,
                                           bool health_gate_5d, float dt) noexcept {
    UnitLeakTickResult result{};
    if (entries == nullptr) {
        return result;
    }

    // 0074F94A..0074F9AF. total_rate accumulates the raw rate; the water takes the
    // capped one. The floor test at 0074F955 is `rate < 0.001f` (strict, with the
    // equal case falling through), so a rate exactly at the floor survives.
    for (std::uint32_t i = 0; i < count; ++i) {
        UnitLeakEntry& entry = entries[i];
        if (entry.rate <= kUnitForceLeakRateFloor && entry.rate != kUnitForceLeakRateFloor) {
            entry.rate = 0.0f;
        }
        const float applied = (rate_cap < entry.rate) ? rate_cap : entry.rate;
        result.total_rate += entry.rate;
        entry.water += applied * dt;
    }

    // 0074F9B2..0074F9C8.
    for (std::uint32_t i = 0; i < count; ++i) {
        result.total_water += entries[i].water;
    }

    // 0074F9CB..0074FA4F. The health read is 00923BE0; its +5Dh gate suppresses the cap.
    if (result.total_water > 0.0f && !health_gate_5d) {
        const double ratio = (static_cast<double>(1.0) - static_cast<double>(unit_health)) *
                             static_cast<double>(settings_flood_scale) *
                             static_cast<double>(capacity) / static_cast<double>(result.total_water);
        const float scale = static_cast<float>(ratio);
        if (scale < 1.0f) {
            for (std::uint32_t i = 0; i < count; ++i) {
                entries[i].water *= scale;
                entries[i].rate = 0.0f;
            }
            result.total_water *= scale;
        }
    }

    // 0074FA52.
    result.flooded = result.total_water > kUnitForceLeakFloodedMark;
    return result;
}

OceanVec3 unit_leak_torque_0074f2e0(const UnitLeakEntry* entries, std::uint32_t count,
                                    const float* pose_rows_ccb) noexcept {
    OceanVec3 out{};
    if (entries == nullptr || pose_rows_ccb == nullptr) {
        return out;
    }

    // The pose block is four rows of four floats starting at unit+CCh. 0074F35D reads
    // +CCh, +DCh and +ECh (column 0) and 0074F345/0074F34D/0074F355 read +D4h, +E4h and
    // +F4h (column 2).
    const float row0_x = pose_rows_ccb[0];
    const float row0_z = pose_rows_ccb[2];
    const float row1_x = pose_rows_ccb[4];
    const float row1_z = pose_rows_ccb[6];
    const float row2_x = pose_rows_ccb[8];
    const float row2_z = pose_rows_ccb[10];

    for (std::uint32_t i = 0; i < count; ++i) {
        const OceanVec3& p = entries[i].point;
        const float w = entries[i].water;
        out.z -= (p.x * row0_x + p.y * row1_x + p.z * row2_x) * w * 10.0f;
        out.x += w * (p.x * row0_z + row1_z * p.y + row2_z * p.z) * 10.0f;
    }
    return out;
}

// ---------------------------------------------------------------------------
// Throttle -> target speed.
// ---------------------------------------------------------------------------

UnitSpeedCommand unit_speed_command_00826985(const UnitSpeedCommandInputs& in) noexcept {
    UnitSpeedCommand cmd{};

    // 00826985..008269B1. The wave height is scaled by 0.5 (00D7A280, a double) before
    // the comparison; when it wins, the throttle is zeroed and BL - the gate on the whole
    // command pair - is cleared with it.
    const float wave = static_cast<float>(static_cast<double>(in.wave_height) *
                                          static_cast<double>(kUnitForceHalf));
    if (wave > in.deck_reference) {
        cmd.apply = false;
        cmd.throttle = 0.0f;
    } else {
        cmd.apply = true;
        cmd.throttle = in.throttle;
    }

    // 008269B1..008269F0: a submarine below the deck reference loses throttle.
    if (in.class_id == kUnitForceSubmarineClassId && kUnitForceDeckDepth > in.pose_base_y) {
        cmd.throttle = in.submerged_scale * cmd.throttle;
    }

    // 008269F4..00826A00.
    if (in.dead_5d) {
        cmd.throttle = 0.0f;
    }

    // 00826754..0082676E, with XMM1 zeroed at 008266B4.
    cmd.engine_gate = (!in.engine_jam && in.thrust_mod != 0.0f) ? 1.0f : 0.0f;

    // 00826A46..00826A5F: ((max_speed * gameplay_scale) * throttle) * engine_gate.
    const float scaled = in.max_speed * in.gameplay_scale;
    cmd.target_speed = scaled * cmd.throttle * cmd.engine_gate;
    return cmd;
}

// ---------------------------------------------------------------------------
// Target speed -> body linear velocity.
// ---------------------------------------------------------------------------

float unit_forward_acceleration_00825ec0(float class_drive_accel, float acceleration_boost,
                                         float settings_boost_top) noexcept {
    float accel = class_drive_accel;
    if (kUnitForceZero < acceleration_boost) {
        accel = (acceleration_boost * (settings_boost_top - 1.0f) + 1.0f) * accel;
    }
    return accel;
}

UnitAxialSpeedStep unit_approach_axial_speed_0092d300(const UnitAxialSpeedInputs& in) noexcept {
    UnitAxialSpeedStep step{};

    // 0092D3A6..0092D3C4: the axis is flattened, and the y direction is a literal
    // 0.0f / length (0092D3B7), so it stays zero whatever the length is. The length
    // itself comes from the CRT square root at 0092D3A6 whose argument was not read out
    // of the listing; the flattened norm below is the reading that matches the two
    // divides that follow, and it is provisional.
    const float len = std::sqrt(in.axis.x * in.axis.x + in.axis.z * in.axis.z);
    const float dir_x = in.axis.x / len;
    const float dir_y = 0.0f / len;
    const float dir_z = in.axis.z / len;

    // 0092D3CE.
    step.current_speed = dir_z * in.velocity.z + dir_y * in.velocity.y + dir_x * in.velocity.x;

    // 0092D3D8..0092D3E9: the heel scale.
    const float heel = native_abs(in.axis.y);
    float target = (1.0f - heel) * in.commanded_speed;

    // 0092D3F1..0092D42D: the acceleration selection.
    float accel = in.drive_accel;
    if (step.current_speed <= target) {
        if (step.current_speed >= 0.0f) {
            accel = in.brake_accel;
        }
    } else {
        const float magnitude = native_abs(step.current_speed);
        if (magnitude < kUnitForceCreepSpeed) {
            accel = in.brake_accel + in.brake_accel;
        } else if (step.current_speed <= 0.0f) {
            accel = in.brake_accel;
        }
    }
    step.accel_used = accel;

    // 0092D43A..0092D45B: one step of the rate limiter, written out because the native
    // form compares against the absolute gap rather than calling the shared helper.
    const float move = accel * in.dt;
    const float gap = native_abs(step.current_speed - target);
    if (move <= gap) {
        target = (target <= step.current_speed) ? (step.current_speed - move)
                                                : (step.current_speed + move);
    }
    step.target_speed = target;

    // 0092D466..0092D4A2.
    step.velocity.x = target * dir_x + (in.velocity.x - step.current_speed * dir_x);
    step.velocity.y = target * dir_y + (in.velocity.y - step.current_speed * dir_y);
    step.velocity.z = target * dir_z + (in.velocity.z - dir_z * step.current_speed);
    return step;
}

OceanVec3 unit_set_axial_speed_0092d770(const OceanVec3& velocity, const OceanVec3& axis,
                                        float speed) noexcept {
    // 0092D79A: the axis is not normalised, so this only behaves as a projection when
    // 00C32000's basis is already unit length.
    const float along = axis.z * velocity.z + axis.x * velocity.x + axis.y * velocity.y;
    OceanVec3 out{};
    out.x = (velocity.x - along * axis.x) + speed * axis.x;
    out.y = (velocity.y - along * axis.y) + axis.y * speed;
    out.z = (velocity.z - along * axis.z) + speed * axis.z;
    return out;
}

// ---------------------------------------------------------------------------
// Rudder -> yaw rate and rudder -> torque.
// ---------------------------------------------------------------------------

float unit_propeller_turn_assist_00825de0(float yaw_rate, float propeller_load,
                                          float settings_assist_limit,
                                          float settings_assist_gain) noexcept {
    if (0.0f < propeller_load) {
        const float magnitude = (yaw_rate < 0.0f) ? -yaw_rate : yaw_rate;
        if (magnitude < settings_assist_limit) {
            float boost = settings_assist_gain * (settings_assist_limit - magnitude);
            if (magnitude < boost) {
                boost = magnitude;
            }
            boost = boost * propeller_load;
            // 00825E36 tests the signed rate against 00D7A218.
            return (yaw_rate < kUnitForceZero) ? (yaw_rate - boost) : (boost + yaw_rate);
        }
    }
    return yaw_rate;
}

OceanVec3 unit_steering_torque_00937440(const UnitSteeringTorqueInputs& in) noexcept {
    // 0093746D..009374A1.
    const double along = static_cast<double>(in.velocity.z) * static_cast<double>(in.axis.z) +
                         static_cast<double>(in.velocity.x) * static_cast<double>(in.axis.x) +
                         static_cast<double>(in.velocity.y) * static_cast<double>(in.axis.y);
    float ratio = static_cast<float>(along / static_cast<double>(in.reference_speed));
    ratio = native_abs(ratio);

    // 009374C5..009374DC.
    const float heel = native_abs(in.pose_row0_y);

    // 009374E8..0093750D: the gain, then the mass squared, then the divide.
    const float gain = (1.0f - heel) * ratio * in.steering * in.settings_rudder_torque;
    const float mass_squared = in.hull_mass * in.hull_mass;

    OceanVec3 torque{};
    torque.x = (mass_squared * gain * in.pose_row2.x) / kUnitForceTorqueDivisor;
    torque.y = (gain * in.pose_row2.y * mass_squared) / kUnitForceTorqueDivisor;
    torque.z = (mass_squared * gain * in.pose_row2.z) / kUnitForceTorqueDivisor;
    return torque;
}

// ---------------------------------------------------------------------------
// The tick that issues both commands.
// ---------------------------------------------------------------------------

UnitForceCommandResult unit_apply_motion_commands_00825f20(UnitForceCommandHost& host,
                                                           float dt) noexcept {
    UnitForceCommandResult result{};

    const UnitSpeedCommandInputs inputs = host.read_speed_inputs();
    result.speed = unit_speed_command_00826985(inputs);

    // 00826B0A: BL gates both calls, not only the speed one.
    if (!result.speed.apply) {
        return result;
    }

    // 00826B0E..00826B29.
    const UnitAxialSpeedInputs axial = host.read_axial_inputs(result.speed.target_speed, dt);
    result.axial = unit_approach_axial_speed_0092d300(axial);
    host.body_set_linear_velocity(result.axial.velocity);
    result.speed_applied = true;

    // 00826B2E..00826B54.
    if (host.steering_jam()) {
        return result;
    }

    const float commanded = host.steering_command();

    // 0092E8C3..0092E8EF: the rate limiter runs on controller+80h with dt*0.5f.
    const float max_step = static_cast<float>(static_cast<double>(dt) *
                                              static_cast<double>(kUnitForceHalf));
    result.smoothed_rudder = host.step_smoothed_rudder(commanded, max_step);

    // 0092E947..0092E96B: the map, the negation at 0092E955, then the assist.
    const float mapped = host.yaw_rate_from_rudder_00811890(result.smoothed_rudder);
    result.yaw_rate = host.propeller_turn_assist(-mapped);

    host.body_set_angular_velocity_from_yaw_rate(result.yaw_rate);
    result.steering_applied = true;
    return result;
}

} // namespace bsp
