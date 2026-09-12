// Reconstruction of the bomb-family tick and the torpedo swim model.
// Evidence: docs/BOMB_FAMILY_TICK.md and docs/TORPEDO_TICK.md.
#include "bsp/bomb_torpedo_tick.hpp"

#include <cmath>

namespace bsp {
namespace {

// Every bomb-family field that also exists on the bullet sits 1A0h higher,
// because the shot interface moved from +170h to +310h. These check the pairs
// this packet read against the offsets docs/PROJECTILE_IMPACT.md established.
static_assert(kBombOffFlightTime == kProjectileOffFlightTime + kBombRecordShotInterfaceShift,
              "flight time shift");
static_assert(kBombOffSweepExtra == kProjectileOffSweepExtra + kBombRecordShotInterfaceShift,
              "sweep extra shift");
static_assert(kBombOffSnapshotCurrent == kProjectileOffSnapshotCurrent + kBombRecordShotInterfaceShift,
              "current snapshot shift");
static_assert(kBombOffSnapshotPrevious == kProjectileOffSnapshotPrevious + kBombRecordShotInterfaceShift,
              "previous snapshot shift");
static_assert(kBombOffOwner == kProjectileOffOwner + kBombRecordShotInterfaceShift, "owner shift");
// The base fields ahead of the insertion point keep their bullet offsets.
static_assert(kBombOffPoseValid == kProjectileOffPoseValid, "pose flag offset");
static_assert(kBombOffWorldTranslation == kProjectileOffWorldTranslation, "world translation offset");
// The swim depth the Lua binding writes is inside the torpedo tail this packet
// read, between the two drag coefficients and the depth gains.
static_assert(kTorpedoOffLateralDrag < kTorpedoInstanceOffSwimDepth, "swim depth placement");
static_assert(kTorpedoInstanceOffSwimDepth < kTorpedoOffDepthGain, "swim depth placement");

constexpr float kGravity = static_cast<float>(kProjectileGravity); // 00CF9058
constexpr float kPi = 3.14159265358979323846f;                     // 00CE3D28 / 00CE3D20

float dot3(const BombVector3& a, const BombVector3& b) noexcept {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float length3(const BombVector3& v) noexcept {
    return std::sqrt(dot3(v, v));
}

// 006E15CD: the reciprocal is taken only for a strictly positive length, and is
// zero otherwise, so a stationary model keeps its forward row.
float inverse_length_or_zero(const BombVector3& v) noexcept {
    const float len = length3(v);
    return len > 0.0f ? 1.0f / len : 0.0f;
}

// 00856EA5 and 008570F9: the clamp is written as two compares, so a NaN delta
// falls through unchanged. std::fmin/fmax would differ there; the explicit
// compares match the listing.
float clamp_symmetric(float value, float limit) noexcept {
    float out = value;
    if (out > limit) {
        out = limit;
    }
    if (out < -limit) {
        out = -limit;
    }
    return out;
}

// 00438B10 BSP_Math_SubtractWrappedAngle, the difference folded into (-pi, pi].
float subtract_wrapped_angle(float a, float b) noexcept {
    float d = a - b;
    while (d > kPi) {
        d -= 2.0f * kPi;
    }
    while (d <= -kPi) {
        d += 2.0f * kPi;
    }
    return d;
}

} // namespace

float bomb_scaled_step(float step, float time_scale) noexcept {
    return time_scale * step;
}

BombWaterMode bomb_water_mode(bool in_water) noexcept {
    return in_water ? BombWaterMode::Water : BombWaterMode::Air;
}

BombWaterMode torpedo_water_mode(bool in_water, float world_y, float swim_depth) noexcept {
    // 00855F1A: 2 * swimDepth is compared against the world height, and the
    // deeper answer short-circuits the in-water byte entirely.
    if (swim_depth + swim_depth > world_y) {
        return BombWaterMode::TorpedoBelowTwiceSwimDepth;
    }
    return bomb_water_mode(in_water);
}

BombVector3 bomb_air_velocity(const BombVector3& velocity,
                              const BombVector3& acceleration,
                              bool no_gravity,
                              float dt) noexcept {
    BombVector3 out = velocity;
    if (!no_gravity) {
        out.y -= kGravity * dt;
        out.x += acceleration.x * dt;
        out.y += acceleration.y * dt;
        out.z += acceleration.z * dt;
    }
    return out;
}

float bomb_pitch_blend(float forward_y, const BombVector3& velocity) noexcept {
    const float inv = inverse_length_or_zero(velocity);
    return forward_y * kBombPitchBlendKeep + inv * velocity.y * kBombPitchBlendAdd;
}

BombVector3 depth_charge_dive_velocity(const BombVector3& velocity,
                                       float drag_coefficient,
                                       float dt) noexcept {
    const float k = -drag_coefficient;
    BombVector3 out;
    out.x = velocity.x + k * velocity.x * dt;
    out.y = velocity.y + (k * velocity.y - kGravity) * dt;
    out.z = velocity.z + k * velocity.z * dt;
    return out;
}

float depth_charge_terminal_sink_speed(float drag_coefficient) noexcept {
    return drag_coefficient > 0.0f ? -kGravity / drag_coefficient : 0.0f;
}

bool depth_charge_should_detonate(float world_y, float detonation_depth) noexcept {
    return world_y < -std::fabs(detonation_depth);
}

bool depth_charge_scan_enabled(float world_y) noexcept {
    return world_y < kDepthChargeScanDepth;
}

bool depth_charge_in_proximity(const BombVector3& delta) noexcept {
    return dot3(delta, delta) <= kDepthChargeProximityRadiusSquared;
}

float rocket_axial_speed(float axial_speed, float acceleration, float v_max, float dt) noexcept {
    if (v_max <= axial_speed) {
        return axial_speed;
    }
    const float stepped = acceleration * dt + axial_speed;
    return stepped > v_max ? v_max : stepped;
}

BombVector3 rocket_thrust_velocity(const BombVector3& velocity,
                                   const BombVector3& world_forward,
                                   float axial_speed,
                                   float acceleration,
                                   float v_max,
                                   float dt) noexcept {
    const float delta = rocket_axial_speed(axial_speed, acceleration, v_max, dt) - axial_speed;
    BombVector3 out = velocity;
    out.x += world_forward.x * delta;
    out.y += world_forward.y * delta;
    out.z += world_forward.z * delta;
    return out;
}

float rocket_pre_ignition_slice(float elapsed, float ignition_delay, float dt) noexcept {
    const float remaining = ignition_delay - elapsed;
    return remaining > dt ? dt : remaining;
}

bool rocket_ignites_this_step(float elapsed, float ignition_delay, float dt) noexcept {
    const float pre = rocket_pre_ignition_slice(elapsed, ignition_delay, dt);
    const float post = dt - pre;
    const float credit = elapsed <= post ? elapsed : post;
    // 0080A0D0: a non-zero post-ignition credit lights the motor, and so does a
    // zero pre-ignition slice, which is the already-expired case.
    return credit != 0.0f || pre == 0.0f;
}

float torpedo_level_forward_y(float forward_y, float dt) noexcept {
    return forward_y * (1.0f - kTorpedoLevelRate * dt);
}

BombVector3 torpedo_swim_velocity(const BombVector3& velocity,
                                  const BombVector3& forward,
                                  float axial_drag,
                                  float lateral_drag,
                                  float dt) noexcept {
    const float along = dot3(velocity, forward);
    const BombVector3 axial{forward.x * along, forward.y * along, forward.z * along};
    const BombVector3 lateral{velocity.x - axial.x, velocity.y - axial.y, velocity.z - axial.z};
    const float axial_keep = 1.0f - axial_drag * dt;
    const float lateral_keep = 1.0f - lateral_drag * dt;
    BombVector3 out;
    out.x = axial.x * axial_keep + lateral.x * lateral_keep;
    out.y = axial.y * axial_keep + lateral.y * lateral_keep;
    out.z = axial.z * axial_keep + lateral.z * lateral_keep;
    return out;
}

float torpedo_clamp_vertical_speed(float vertical_velocity) noexcept {
    if (vertical_velocity >= 0.0f) {
        return vertical_velocity > kTorpedoMaxRiseSpeed ? kTorpedoMaxRiseSpeed : vertical_velocity;
    }
    return kTorpedoMaxSinkSpeed > vertical_velocity ? kTorpedoMaxSinkSpeed : vertical_velocity;
}

float torpedo_depth_velocity(float vertical_velocity,
                             float world_y,
                             float swim_depth,
                             float surface_height,
                             float depth_gain,
                             float surface_gain,
                             float dt) noexcept {
    const float error = (world_y - swim_depth) * depth_gain;
    const float acceleration = -(error - surface_gain * surface_height);
    return torpedo_clamp_vertical_speed(vertical_velocity + acceleration * dt);
}

float torpedo_homing_turn(float own_angle, float target_angle, float turn_speed, float dt) noexcept {
    return clamp_symmetric(subtract_wrapped_angle(own_angle, target_angle), turn_speed * dt);
}

float torpedo_heading_rate(float heading_turn_degrees, float modifier) noexcept {
    return heading_turn_degrees * kPi / 180.0f * modifier;
}

float torpedo_heading_turn(float current_yaw, float commanded_yaw, float rate, float dt) noexcept {
    const float error = -subtract_wrapped_angle(current_yaw, commanded_yaw);
    return clamp_symmetric(error, rate) * dt;
}

bool torpedo_prefers_candidate(float current_distance_squared,
                               float candidate_distance_squared) noexcept {
    return current_distance_squared > candidate_distance_squared;
}

bool torpedo_scan_due(float countdown, float dt) noexcept {
    return !(dt < countdown);
}

float torpedo_advance_scan_countdown(float countdown, float interval, float dt) noexcept {
    return torpedo_scan_due(countdown, dt) ? countdown + interval - dt : countdown - dt;
}

float torpedo_cached_range(float water_travel_speed, float fly_time) noexcept {
    return water_travel_speed * fly_time * kTorpedoRangeFactor;
}

float torpedo_terminal_fall_speed(float max_fall) noexcept {
    return std::sqrt((max_fall + max_fall) * kGravity);
}

bool water_entry_destroys(float impact_speed,
                          float vertical_speed,
                          float max_water_hit_velocity,
                          float terminal_fall_speed) noexcept {
    return impact_speed > max_water_hit_velocity || vertical_speed < -terminal_fall_speed;
}

BombStepSnapshot bomb_commit_snapshot(const BombStepSnapshot& snapshot,
                                      const BombVector3& world_translation) noexcept {
    BombStepSnapshot out;
    out.previous = snapshot.current;
    out.current = snapshot.cached;
    out.cached = world_translation;
    return out;
}

BombSweepFlags bomb_sweep_flags(int world_mode) noexcept {
    if (world_mode == 2) {
        return BombSweepFlags{false, false};
    }
    return BombSweepFlags{true, true};
}

bool bomb_sweep_armed(float arming_delay) noexcept {
    return arming_delay < 0.0f;
}

bool bomb_family_tick_006e1300(BombTickState& state,
                               const BombFlightClass& flight_class,
                               BombTickHost& host,
                               float step) {
    // 006E130B: a negative launch delay counts up and nothing else runs.
    if (state.launch_delay < 0.0f) {
        state.launch_delay += step;
        return true;
    }
    // 006E1331.
    if (!state.active) {
        return true;
    }

    const float dt = bomb_scaled_step(step, flight_class.time_scale);

    // 006E1361 then 006E138B.
    if (host.query_water_mode() == BombWaterMode::Air) {
        host.advance_in_air(dt);
    } else {
        host.advance_in_water(dt);
    }

    // 006E138D.
    if (state.sweep_enabled) {
        if (!bomb_sweep_armed(state.arming_delay)) {
            state.arming_delay -= dt;
        } else {
            BombSweepFlags flags = host.query_sweep_flags();
            if (host.world_mode() == 2) {
                flags = bomb_sweep_flags(2);
            }
            if (!state.pose_valid) {
                host.refresh_world_pose();
                state.pose_valid = true;
            }
            host.sweep_step_segment(flags);
        }
    }

    // 006E1464: the expiry test does not run in world mode 2.
    if (host.world_mode() != 2) {
        state.flight_time += dt;
        if (state.flight_time > flight_class.fly_time) {
            if (flight_class.has_timeout_effect) {
                host.spawn_timeout_effect();
            }
            host.expire_projectile();
            host.release_projectile();
            return false;
        }
    }

    // 006E14EA.
    host.advance_owner_grace(dt);
    return true;
}

bool torpedo_steer_00856bb0(TorpedoSwimState& state,
                            const TorpedoSwimClass& swim_class,
                            TorpedoSwimHost& host,
                            float dt) {
    bool homed = false;
    // 00856BC0: a class with no horizontal homing speed never scans.
    if (swim_class.homing_horz_turn_speed > 0.0f) {
        const bool scan = torpedo_scan_due(state.scan_countdown, dt);
        state.scan_countdown = torpedo_advance_scan_countdown(state.scan_countdown,
                                                              state.scan_interval, dt);
        if (scan) {
            host.bind_target(host.acquire_nearest_target());
        }

        void* target = host.current_target();
        if (target != nullptr) {
            if (!state.pose_valid) {
                host.refresh_world_pose();
                state.pose_valid = true;
            }
            const BombMatrix4x3 world = host.world_matrix();
            float own_pitch = 0.0f;
            float own_yaw = 0.0f;
            host.direction_to_pitch_yaw(world.forward, own_pitch, own_yaw);

            float target_pitch = 0.0f;
            float target_yaw = 0.0f;
            host.direction_to_pitch_yaw(host.direction_to_target(target), target_pitch, target_yaw);
            // 00856E25: the commanded heading becomes the target bearing even
            // when the turn below is skipped.
            state.commanded_heading = target_yaw;

            if (swim_class.homing_vert_turn_speed > 0.0f && host.target_is_homing_kind(target) &&
                !host.target_suppresses_homing(target)) {
                // 00856ED3: the pitch turn, about the hull's own right axis.
                host.rotate_about_right(torpedo_homing_turn(own_pitch, target_pitch,
                                                            swim_class.homing_vert_turn_speed, dt));
                // 00856F51: the second turn is about the up axis, but the
                // listing feeds it the negated pitch error, not a yaw error.
                // Both 00438B10 calls read the same two stack slots, E+0Ch and
                // E+8h. See docs/TORPEDO_TICK.md; this is reproduced as read.
                host.rotate_about_up(-torpedo_homing_turn(own_pitch, target_pitch,
                                                          swim_class.homing_horz_turn_speed, dt));
                homed = true;
            } else {
                // 00856F6C: the fallback rotates by the hull's own pitch, which
                // levels it, and then falls through to the two controllers.
                host.rotate_about_right(own_pitch);
            }
        }
    }

    if (homed) {
        return true;
    }

    // 00856F83: the depth controller.
    if (!state.pose_valid) {
        host.refresh_world_pose();
        state.pose_valid = true;
    }
    const BombMatrix4x3 world = host.world_matrix();
    const float surface = host.sample_water_height(world.translation.x, world.translation.z);
    state.velocity.y = torpedo_depth_velocity(state.velocity.y, world.translation.y,
                                              state.swim_depth, surface, state.depth_gain,
                                              state.surface_gain, dt);

    // 00857061: the heading controller.
    const float yaw = std::atan2(world.forward.x, world.forward.z);
    const float rate = torpedo_heading_rate(swim_class.heading_turn_degrees,
                                            host.gameplay_modifier(kTorpedoHeadingModifierKey));
    host.rotate_about_up(torpedo_heading_turn(yaw, state.commanded_heading, rate, dt));
    return false;
}

void torpedo_swim_00857480(TorpedoSwimState& state,
                           const TorpedoSwimClass& swim_class,
                           TorpedoSwimHost& host,
                           float dt) {
    // 0085748A.
    state.run_time += dt;

    BombMatrix4x3 local = host.local_matrix();
    // 008574A0: the sentinel seeds the commanded heading once.
    if (state.commanded_heading == kTorpedoHeadingUnset) {
        if (!state.pose_valid) {
            host.refresh_world_pose();
            state.pose_valid = true;
        }
        float pitch = 0.0f;
        float yaw = 0.0f;
        host.direction_to_pitch_yaw(host.world_matrix().forward, pitch, yaw);
        state.commanded_heading = yaw;
    }

    // 008574F0: level the hull, then write it back.
    local.forward.y = torpedo_level_forward_y(local.forward.y, dt);
    host.orthonormalize(local);
    host.set_local_matrix(local);

    // 00857531.
    torpedo_steer_00856bb0(state, swim_class, host, dt);

    // 00857536: the matrix is re-read because the steering rotated it.
    const BombMatrix4x3 steered = host.local_matrix();
    state.velocity = torpedo_swim_velocity(state.velocity, steered.forward, state.axial_drag,
                                           state.lateral_drag, dt);
    // The wake and sound tail from 00857679 is a contract, not reconstructed.
}

} // namespace bsp
