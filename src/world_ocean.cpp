#include "bsp/world_ocean.hpp"

#include <cmath>

namespace bsp {
namespace {

OceanVec3 subtract(const OceanVec3& a, const OceanVec3& b) noexcept {
    return OceanVec3{a.x - b.x, a.y - b.y, a.z - b.z};
}

} // namespace

OceanVec3 cross_004f9b30(const OceanVec3& a, const OceanVec3& b) noexcept {
    // out.x = a[1]*b[2] - a[2]*b[1]
    // out.y = b[0]*a[2] - a[0]*b[2]
    // out.z = b[1]*a[0] - a[1]*b[0]
    OceanVec3 out;
    out.x = a.y * b.z - a.z * b.y;
    out.y = b.x * a.z - a.x * b.z;
    out.z = b.y * a.x - a.y * b.x;
    return out;
}

float length_00419440(const OceanVec3& v) noexcept {
    // 00419456..00419489. Each square is stored back to a float, so xx, yy and
    // zz are rounded individually. The partial sum xx+yy stays on the x87 stack
    // unrounded and only the total is stored, which the double below models.
    // The square root is the CRT helper at 00BF7030 applied to that float.
    const float xx = v.x * v.x;
    const float yy = v.y * v.y;
    const float zz = v.z * v.z;
    const float sum = static_cast<float>((static_cast<double>(xx) + yy) + zz);
    return static_cast<float>(std::sqrt(static_cast<double>(sum)));
}

OceanVec3 normalize_00419510(const OceanVec3& v) noexcept {
    const float len = length_00419440(v);
    float inverse = 0.0f;
    if (len > 0.0f) {
        inverse = 1.0f / len;
    }
    // The stores keep the native operand order: x and z scale by the inverse on
    // the left, y on the right. Float multiplication is commutative, so this
    // only documents the listing.
    OceanVec3 out;
    out.x = inverse * v.x;
    out.y = v.y * inverse;
    out.z = inverse * v.z;
    return out;
}

bool ocean_orthonormalize_00bbd310(OceanState& ocean, const OceanVec3& reference_axis) noexcept {
    if (!ocean.enabled) {
        // 00BBD31F: the node at ocean+30h is hidden with 00B6DA70(0.0f, 0) and
        // nothing else runs. No frame field is touched.
        return false;
    }

    const OceanVec3 first = cross_004f9b30(reference_axis, ocean.frame.up);
    const OceanVec3 second = cross_004f9b30(first, reference_axis);
    ocean.frame.up = normalize_00419510(second);

    const OceanVec3 third = cross_004f9b30(reference_axis, ocean.frame.up);
    ocean.frame.right = normalize_00419510(third);

    // 00BBD3D9 measures the up axis after it was normalised, so the length is
    // 1.0f unless the cross products collapsed and normalize returned zero.
    if (length_00419440(ocean.frame.up) < kOceanFrameDegenerateEpsilon) {
        ocean.frame.up = OceanVec3{0.0f, kOceanUnit, 0.0f};
        ocean.frame.right = OceanVec3{kOceanUnit, 0.0f, 0.0f};
    }

    ocean.frame.scale = kOceanUnit;
    ocean.frame.visible_layer_count = 0;
    return true;
}

OceanVec3 shore_wave_scroll_step_00bbec06(
    const OceanVec3& direction, float speed, float delta) noexcept {
    // Three passes over the triple in the listing: multiply by the delta, then
    // by the speed, then by the constant. Each pass rounds to a float.
    OceanVec3 step;
    step.x = direction.x * delta;
    step.y = direction.y * delta;
    step.z = direction.z * delta;

    step.x = step.x * speed;
    step.y = step.y * speed;
    step.z = step.z * speed;

    step.x = step.x * kShoreWaveScrollScale;
    step.y = step.y * kShoreWaveScrollScale;
    step.z = step.z * kShoreWaveScrollScale;
    return step;
}

void advance_shore_wave_scroll_00bbec06(
    OceanVec3& scroll, const OceanVec3& direction, float speed, float delta) noexcept {
    const OceanVec3 step = shore_wave_scroll_step_00bbec06(direction, speed, delta);
    scroll.x = step.x + scroll.x;
    scroll.y = scroll.y + step.y;
    scroll.z = scroll.z + step.z;
}

bool advance_effect_sample_00867d00(
    EffectSampleState& state, float delta, const OceanVec3& sampled_position) noexcept {
    // 00867D2C: the age accumulates before any gate.
    state.age = delta + state.age;

    if (!state.track_velocity && !state.track_displacement) {
        return false;
    }

    const float elapsed = state.sample_timer + delta;
    if (!(state.sample_interval < elapsed)) {
        state.sample_timer = elapsed;
        return false;
    }

    state.previous_position = state.current_position;
    state.current_position = sampled_position;

    if (state.sample_count > 1 && delta > 0.0f) {
        if (state.track_velocity) {
            // The native code recomputes sample_timer + delta here rather than
            // reusing the earlier sum, which is the same value.
            const float span = state.sample_timer + delta;
            const OceanVec3 travel = subtract(state.current_position, state.previous_position);
            state.velocity.x = travel.x / span;
            state.velocity.y = travel.y / span;
            state.velocity.z = travel.z / span;
        }
        if (state.track_displacement) {
            state.displacement = subtract(state.current_position, state.previous_position);
        }
    }

    state.sample_timer = 0.0f;
    return true;
}

bool session_counts_mission_004b6260(bool session_object_present, bool session_flag_29c) noexcept {
    return session_object_present && !session_flag_29c;
}

void adjust_mission_counters_004bcaa0(MissionCounterState& counters, bool add) noexcept {
    if (add) {
        counters.active = counters.active + 1;
        counters.total = counters.total + 1;
        return;
    }
    // 004BCAD6 compares against 1 with an unsigned branch, so zero stays zero.
    counters.active = counters.active >= 1 ? counters.active - 1 : 0;
}

void arm_mission_start_latch_004e4e00(
    MissionStartLatch& latch, const WorldOceanTickInputs& inputs, WorldOceanHost& host) {
    // 004E4DF4 gates the whole block on the in-mission state.
    if (inputs.game_state != 0x0D) {
        return;
    }
    if (latch.counted) {
        return;
    }
    if (host.network_session_counts_mission() && !host.mission_start_suppressed()) {
        host.record_mission_started();
    }
    // 004E4E2A sets the latch on every path that reaches it, including the one
    // where the session predicate failed.
    latch.counted = true;
}

void run_ocean_and_effects_tick(const WorldOceanTickInputs& inputs, WorldOceanHost& host) {
    if (host.ocean_manager_present()) {
        void* const descriptor = host.weather_effect_descriptor();
        host.update_rain_spawner(descriptor, inputs.scaled_delta);
        host.update_ocean(inputs.scaled_delta);
    }

    host.updates_between_ocean_and_effects(inputs.scaled_delta);

    void* const manager = host.effect_manager();
    host.update_effects(manager, inputs.scaled_delta);
}

} // namespace bsp
