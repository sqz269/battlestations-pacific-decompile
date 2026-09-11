#include "bsp/unit_motion.hpp"
#include "bsp/camera_affine.hpp"

#include <cmath>

namespace bsp {

CameraMatrix& pose_world_matrix_0042d7e0(PoseRefreshView& pose) {
    if (pose.world_valid_c8 == 0) refresh_pose_00414db0(pose);
    return pose.world_cc;
}

float* transform_point_staged_00414d10(float* output,
    const std::array<float, 3>& input, const CameraMatrix& matrix) {
    std::array<float, 3> scratch;
    transform_point_004142e0(input, matrix, scratch);
    output[0] = scratch[0];
    output[1] = scratch[1];
    output[2] = scratch[2];
    return output;
}

const char kUnitAudioParamRpm[] = "rpm";         // 00D08688
const char kUnitAudioParamGeneric[] = "param00"; // 00D09998

// ---------------------------------------------------------------------------
// Shared scalar primitives.
// ---------------------------------------------------------------------------

float unit_step_towards_0042ac60(float state, float target, float max_step) noexcept {
    // 0042AC61..0042ACB6. The difference is stored back to a float and its sign bit is
    // cleared with an integer AND, so the magnitude is the float magnitude.
    const float difference = state - target;
    const float distance = std::fabs(difference);
    if (max_step > distance) {
        return target;
    }
    // FCOMIP at 0042ACA6 compares target with state; equal takes the subtracting arm.
    if (target > state) {
        return state + max_step;
    }
    return state - max_step;
}

float unit_clamp_00415690(float value, float low, float high) noexcept {
    // 00415693..004156CA. Both comparisons are strict, so an equal bound is a no-op.
    if (low > value) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

// ---------------------------------------------------------------------------
// Speed sources.
// ---------------------------------------------------------------------------

float unit_reference_speed_0080fc30(float base_speed, float gameplay_scale) noexcept {
    // 0080FC58 and 0080FC72: one x87 multiply, rounded once on the store.
    return static_cast<float>(static_cast<double>(base_speed) *
                              static_cast<double>(gameplay_scale));
}

float unit_forward_speed_0092d730(const UnitBodyAxisSpeedInputs& in) noexcept {
    // 0092D74E..00920765: (v.y*a.y + v.x*a.x) + v.z*a.z, all on the x87 stack, stored
    // to a float only at the end.
    const double yy = static_cast<double>(in.velocity[1]) * static_cast<double>(in.axis[1]);
    const double xx = static_cast<double>(in.velocity[0]) * static_cast<double>(in.axis[0]);
    const double zz = static_cast<double>(in.velocity[2]) * static_cast<double>(in.axis[2]);
    return static_cast<float>((yy + xx) + zz);
}

UnitHealthRead unit_health_00923be0(bool gate_5d, float virtual_health) noexcept {
    // 00923BE4..00923C1E.
    UnitHealthRead out;
    if (gate_5d) {
        out.value = 0.0f;
        return out;
    }
    if (virtual_health < 0.0f) {
        out.value = 0.0f;
        out.wrote_zero_to_164 = true;
        return out;
    }
    out.value = virtual_health;
    return out;
}

// ---------------------------------------------------------------------------
// 008252C0.
// ---------------------------------------------------------------------------

std::size_t unit_audio_settings_block_008252de(bool is_secondary_trait,
                                               bool is_tertiary_trait) noexcept {
    if (is_secondary_trait) {
        return kUnitAudioSettingsSecondary;
    }
    if (is_tertiary_trait) {
        return kUnitAudioSettingsTertiary;
    }
    return kUnitAudioSettingsDefault;
}

float unit_audio_rpm_step_0082530b(float settings_rate, float scaled_delta) noexcept {
    // FLD [block+8]; FMUL delta; stored to a float before the call.
    return static_cast<float>(static_cast<double>(settings_rate) *
                              static_cast<double>(scaled_delta));
}

float unit_audio_rpm_target_00825311(bool gate_5d, float throttle) noexcept {
    if (gate_5d) {
        return 0.0f; // FLDZ at 0082532D
    }
    return std::fabs(throttle); // AND 7FFFFFFFh at 0082534F
}

float unit_audio_param00_008253c6(bool gate_5d, float reference_speed,
                                  float forward_speed) noexcept {
    if (gate_5d) {
        return 0.0f; // FLDZ at 008253CC
    }
    // 008253EA..00825406: (forward - 0) * (1 / (reference - 0)) * (1 - 0) + 0. The
    // reciprocal is taken first, exactly as the listing schedules it, and nothing
    // guards a zero reference speed.
    const double reciprocal = 1.0 / static_cast<double>(reference_speed);
    return static_cast<float>(static_cast<double>(forward_speed) * reciprocal);
}

void unit_update_engine_audio_008252c0(UnitEngineAudioState& state,
                                       UnitEngineAudioHost& host, float scaled_delta) {
    // 008252C5: the settings object is fetched before the gate is tested.
    if (!state.enabled) {
        return; // 008252CA
    }

    const bool secondary = host.is_kind_of(kUnitAudioTraitSecondary);
    const bool tertiary = secondary ? false : host.is_kind_of(kUnitAudioTraitTertiary);
    const std::size_t block = unit_audio_settings_block_008252de(secondary, tertiary);

    const float step = unit_audio_rpm_step_0082530b(host.settings_rate(block), scaled_delta);
    const float target = unit_audio_rpm_target_00825311(state.gate_5d, state.throttle);
    state.rpm_smoothed = unit_step_towards_0042ac60(state.rpm_smoothed, target, step);

    if (state.has_emitter_a) {
        host.set_emitter_parameter(kUnitOffRpmEmitterA, kUnitAudioParamRpm,
                                   state.rpm_smoothed);
    }
    if (state.has_emitter_b) {
        host.set_emitter_parameter(kUnitOffRpmEmitterB, kUnitAudioParamRpm,
                                   state.rpm_smoothed);
    }
    if (state.has_emitter_c) {
        float value = 0.0f;
        if (!state.gate_5d) {
            // The two calls happen only on the ungated arm, in this order.
            const float reference = host.reference_speed();
            const float forward = host.forward_speed();
            value = unit_audio_param00_008253c6(false, reference, forward);
        }
        host.set_emitter_parameter(kUnitOffRpmEmitterC, kUnitAudioParamGeneric, value);
    }
    if (state.has_stop_target && state.gate_5d) {
        host.stop_audio_target(0.0f); // 0082543B
    }
}

// ---------------------------------------------------------------------------
// 00956600.
// ---------------------------------------------------------------------------

bool unit_damage_scan_runs_00956704(float health, float previous_health) noexcept {
    // FCOMIP previous vs health, JBE skips: the body runs only when health < previous.
    return health < previous_health;
}

bool unit_damage_record_fires_0095674c(float health, float previous_health,
                                       float threshold) noexcept {
    // 0095674F: JC skips when threshold < health, so the record needs health <= threshold.
    // 0095678A: JBE skips when previous <= threshold, so it needs threshold < previous.
    return health <= threshold && threshold < previous_health;
}

float unit_step_fade_009569f8(float fade, float target, float scaled_delta) noexcept {
    if (target > fade) {
        // 00956A2D: cur + delta * 0.5, one x87 store.
        float value = static_cast<float>(static_cast<double>(fade) +
                                         static_cast<double>(scaled_delta) *
                                             kUnitFadeRatePerSecond);
        if (0.0f > value) {
            return 0.0f;
        }
        if (value > target) {
            value = target;
        }
        return value;
    }
    if (fade > target) {
        const float value = static_cast<float>(static_cast<double>(fade) -
                                               static_cast<double>(scaled_delta) *
                                                   kUnitFadeRatePerSecond);
        // 00956A98 calls 00415690 with the target as the low bound and 1.0f as the high.
        return unit_clamp_00415690(value, target, kUnitFadeCeiling);
    }
    return fade; // 00956A9F, the equal case is left alone.
}

float unit_visibility_factor_00956aa3(int game_mode, bool mission_reveal_byte,
                                      bool global_intensity_override,
                                      bool local_intensity_override, float fade) noexcept {
    if (game_mode == kUnitFadeThresholdGameMode) {
        if (global_intensity_override || local_intensity_override) {
            return 1.0f;
        }
        return fade > kUnitFadeVisibleThreshold ? 1.0f : 0.0f;
    }
    if (mission_reveal_byte) {
        return 1.0f;
    }
    if (global_intensity_override || local_intensity_override) {
        return 1.0f;
    }
    return fade;
}

void unit_update_timers_00956600(UnitTimerState& state, PoseRefreshView& actual_pose,
    UnitTimerHost& host, float scaled_delta) {
    // 00956626: unbounded accumulator.
    state.age = state.age + scaled_delta;

    // 0095662C: only stepped while strictly positive, then floored at zero.
    if (state.clamped_countdown > 0.0f) {
        const float remaining = state.clamped_countdown - scaled_delta;
        state.clamped_countdown = remaining < 0.0f ? 0.0f : remaining;
    }

    // 0095666F: unconditional and allowed to go negative.
    state.damage_scan_timer = state.damage_scan_timer - scaled_delta;
    if (state.damage_scan_timer < 0.0f && host.damage_record_count() != 0) {
        state.damage_scan_timer = kUnitDamageScanPeriod; // 009566BD
        const std::size_t record_count = host.damage_record_count();
        const std::size_t anchor_count = host.descriptor_anchor_count();
        const float health = host.health();
        if (unit_damage_scan_runs_00956704(health, state.damage_scan_mark)) {
            for (std::size_t i = 0; i < record_count; ++i) {
                const UnitDamageRecord record = host.damage_record(i);
                if (!unit_damage_record_fires_0095674c(health, state.damage_scan_mark,
                                                       record.health_threshold)) {
                    continue;
                }
                // 009567A3..0095682D: one stack XYZ snapshot for both consumers.
                std::array<float, 3> world_point;
                if (record.anchor_index >= 0 &&
                    static_cast<std::size_t>(record.anchor_index) < anchor_count) {
                    // Native resolves the source pointer before refreshing the unit.
                    const auto& anchor = host.descriptor_anchor(
                        static_cast<std::size_t>(record.anchor_index));
                    const CameraMatrix& world = pose_world_matrix_0042d7e0(actual_pose);
                    transform_point_staged_00414d10(world_point.data(), anchor, world);
                } else {
                    if (actual_pose.world_valid_c8 == 0) refresh_pose_00414db0(actual_pose);
                    world_point[0] = actual_pose.world_cc[12]; // actual unit+FCh
                    world_point[1] = actual_pose.world_cc[13]; // actual unit+100h
                    world_point[2] = actual_pose.world_cc[14]; // actual unit+104h
                }
                if (record.announce_id >= 0) {
                    host.announce_damage_record(i, world_point);
                }
                if (record.has_effect) {
                    host.spawn_damage_effect(i, world_point);
                }
            }
            state.damage_scan_mark = health; // 0095693B
        }
    }

    // 00956946..009569F6: the animation chain behind the +70Ch byte.
    if (state.animation_gate && host.animation_target_ready()) {
        if (state.animation_extra) {
            host.animation_pre_step_00b78670();
        }
        host.animation_step();
    }

    state.fade = unit_step_fade_009569f8(state.fade, state.fade_target, scaled_delta);

    if (state.has_scene_node) {
        const int mode = host.effective_game_mode();
        const bool reveal = mode == kUnitFadeThresholdGameMode ? false
                                                               : host.mission_reveal_byte();
        const float value = unit_visibility_factor_00956aa3(
            mode, reveal, host.global_intensity_override(), state.local_intensity_override,
            state.fade);
        host.set_visibility_factor(value);
    }
}

// ---------------------------------------------------------------------------
// 00834E90.
// ---------------------------------------------------------------------------

float unit_steering_angle_target_00834ed7(float steering_source) noexcept {
    // 00834EF5..00834F05: (s - bias) then subtract blend * (s - bias).
    const double biased = static_cast<double>(steering_source) - kUnitSteeringBias;
    return static_cast<float>(biased - kUnitSteeringBlend * biased);
}

float unit_propeller_base_rate_008350f2(float throttle, float steering, float load,
                                        float class_gain, float class_idle) noexcept {
    // 008350F8..00835116: gain * load * 1.2 + idle, one store.
    const double weighted = static_cast<double>(class_gain) * static_cast<double>(load);
    const float loaded = static_cast<float>(weighted * kUnitPropellerLoadScale +
                                            static_cast<double>(class_idle));
    // 0083512D: throttle * 4, stored to a float before the clamp.
    const float scaled = static_cast<float>(static_cast<double>(throttle) *
                                            kUnitPropellerThrottleGain);
    float clamped = scaled;
    if (kUnitPropellerRateFloor > clamped) {
        clamped = kUnitPropellerRateFloor;
    } else if (clamped > kUnitPropellerRateCeiling) {
        clamped = kUnitPropellerRateCeiling;
    }
    // 00835159..00835175: (1 - |throttle|) * clamped * loaded * steering.
    const double magnitude = 1.0 - static_cast<double>(std::fabs(throttle));
    return static_cast<float>(((magnitude * static_cast<double>(clamped)) *
                               static_cast<double>(loaded)) *
                              static_cast<double>(steering));
}

float unit_propeller_side_term_008351f5(float base_rate, bool node_local_x_positive) noexcept {
    if (node_local_x_positive) {
        return base_rate;
    }
    return kUnitPropellerMirror - base_rate; // SUBSS against -0.0f
}

float unit_propeller_target_rate_00835220(float class_gain, float throttle,
                                          float side_term) noexcept {
    return static_cast<float>(static_cast<double>(class_gain) *
                                  static_cast<double>(throttle) +
                              static_cast<double>(side_term));
}

float unit_propeller_rate_slew_00835203(float scaled_delta) noexcept {
    return static_cast<float>(static_cast<double>(scaled_delta) * kUnitPropellerRateSlew);
}

float unit_propeller_spin_delta_0083541f(std::size_t index, float rate,
                                         float scaled_delta) noexcept {
    // 0083541F: (index & 1) * 2 - 1, converted with FILD.
    const int sign = static_cast<int>(index & 1u) * 2 - 1;
    const float product = static_cast<float>(static_cast<double>(sign) *
                                             static_cast<double>(scaled_delta) *
                                             static_cast<double>(rate));
    return -product; // FCHS at 0083544C, after the value was rounded to a float.
}

bool unit_cavitation_active_008351c9(float rate) noexcept {
    return kUnitCavitationRateThreshold <= std::fabs(rate);
}

void unit_update_propellers_00834e90(UnitPropellerState& state,
                                     const UnitPropellerClassBlock& class_block,
                                     UnitPropellerHost& host, float scaled_delta) {
    // 00834EB7: the controller speed is sampled and the value discarded at 00834EBC.
    (void)host.sample_controller_speed();

    // 00834EBE and 008350BD read the same +61h byte; it swaps both inputs.
    const float throttle =
        state.use_alternate_inputs ? state.throttle_alternate : state.throttle;
    const float steering =
        state.use_alternate_inputs ? state.steering_alternate : state.steering;

    // 00834ED7..00834F12: the shared steering angle steps at exactly the frame delta.
    state.steering_angle = unit_step_towards_0042ac60(
        state.steering_angle, unit_steering_angle_target_00834ed7(steering), scaled_delta);

    for (std::size_t i = 0; i < kUnitSteeringNodeCount; ++i) {
        if (!state.has_steering_node[i]) {
            continue;
        }
        float local_position[3] = {0.0f, 0.0f, 0.0f};
        host.steering_node_local_position(i, local_position);
        host.set_steering_node_transform(i, state.steering_angle, local_position);
    }

    const float base_rate = unit_propeller_base_rate_008350f2(
        throttle, steering, state.load, class_block.propeller_gain,
        class_block.propeller_idle);
    const float slew = unit_propeller_rate_slew_00835203(scaled_delta);

    for (std::size_t i = 0; i < kUnitPropellerCount; ++i) {
        if (!state.has_propeller_node[i]) {
            continue; // 0083517E
        }
        const bool starboard = host.propeller_node_local_x(i) > 0.0f;
        const bool was_active = unit_cavitation_active_008351c9(state.rates[i]);
        const float side_term = unit_propeller_side_term_008351f5(base_rate, starboard);
        const float target = unit_propeller_target_rate_00835220(class_block.propeller_gain,
                                                                 throttle, side_term);
        state.rates[i] = unit_step_towards_0042ac60(state.rates[i], target, slew);
        const bool is_active = unit_cavitation_active_008351c9(state.rates[i]);

        if (was_active) {
            if (!is_active && state.cavitation_live[i]) {
                host.stop_cavitation_effect(i);
                state.cavitation_live[i] = false;
            }
        } else if (is_active && class_block.has_cavitation_effect) {
            host.start_cavitation_effect(i);
            state.cavitation_live[i] = true;
        }

        host.spin_propeller_node(
            i, unit_propeller_spin_delta_0083541f(i, state.rates[i], scaled_delta));
    }

    // 0083553E..0083556F, in this order.
    host.sub_update_00834820(scaled_delta);
    host.sub_update_00834cc0(scaled_delta);
    host.sub_update_00834a70(scaled_delta);
}

} // namespace bsp
