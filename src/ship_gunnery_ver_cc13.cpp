// Packet cc13-ver21. See include/bsp/ship_gunnery_ver_cc13.hpp and
// docs/CC13_VER_21.md. Pure fragments; draws are arguments.
#include "bsp/ship_gunnery_ver_cc13.hpp"

#include <algorithm>
#include <cmath>

namespace bsp {

RangingVec3 artillery_ranging_offset(float length, float bearing) noexcept {
    return RangingVec3{std::sin(bearing) * length, 0.0f, std::cos(bearing) * length};
}

ArtilleryRangingRecord artillery_ranging_record_construct_00864880(
    const ArtilleryRangingRow& row, float distance, float error_u, float bearing_u,
    const RangingVec3& target_position, const RangingVec3& target_velocity,
    float think_time, float now) noexcept {
    ArtilleryRangingRecord rec{};
    // 008648F0..00864906: both clocks are the frame clock 00F876A4.
    rec.t_update = now;
    rec.t_shot = now;
    // err0 = min(MaxErrorRadius, ErrorRangeMul * distance); the caller drew
    // error_u over [err0 * MinErrorMul, err0] (00864A12).
    (void)row;
    (void)distance;
    rec.error_length = error_u;
    rec.bearing = bearing_u;
    rec.offset = artillery_ranging_offset(rec.error_length, rec.bearing);
    // 00864ACA..00864B02: +50h..+58h = position - velocity * think_time.
    const RangingVec3 step{target_velocity.x * think_time, target_velocity.y * think_time,
                           target_velocity.z * think_time};
    rec.last_target = RangingVec3{target_position.x - step.x, target_position.y - step.y,
                                  target_position.z - step.z};
    // 00864B20..00864B8A: +60h..+68h = position + velocity * think_time, +64h = 0.
    rec.predicted = RangingVec3{target_position.x + step.x, 0.0f, target_position.z + step.z};
    rec.fired_since = false;
    return rec;
}

bool artillery_ranging_record_update_error_00862cd0(ArtilleryRangingRecord& rec,
    const ArtilleryRangingRow& row, float distance, float frac, float now,
    float miss_length, float angle_u) noexcept {
    if (rec.fired_since) {
        // 00862DA2..00862E6B. The approach factor is an x87 interpolation
        // (max - min) * (frac - 0) / (1 - 0) + min that is never stored.
        rec.t_shot = now;
        const float dev = frac * frac * row.deviation_mul;   // stored, 00862DFA
        const double approach =
            static_cast<double>(row.approach_mul_max - row.approach_mul_min) * frac
            + row.approach_mul_min;
        rec.fired_since = false;
        rec.error_length = static_cast<float>(rec.error_length * approach
                                              + static_cast<double>(miss_length) * dev);
    } else {
        // 00862D72..00862D8A: JBE straight to the extrapolation tail.
        if (static_cast<double>(now) - rec.t_shot <= row.start_time) return false;
        rec.error_length = static_cast<float>(
            static_cast<double>(row.max_error_radius) * frac
                * ((static_cast<double>(now) - rec.t_update) / row.full_time)
            + rec.error_length);
    }
    // 00862E86..00862F17: cap at min(MaxErrorRadius, ErrorRangeMul * distance).
    const float cap = std::min(row.max_error_radius, row.error_range_mul * distance);
    if (rec.error_length > cap) rec.error_length = cap;
    // 00862F46: the draw is made; 00862F58 adds it to +6Ch; 00862F62 FSTP ST0
    // throws the sum away. The bearing stays as 00864880 drew it.
    (void)angle_u;
    // 00862F64..00862FA0: +74h..+7Ch rebuilt from the new length, old bearing.
    rec.offset = artillery_ranging_offset(rec.error_length, rec.bearing);
    return true;
}

ArtilleryLeadPushOut artillery_lead_push_out_006df833(float distance, float class_speed,
    float time_scale, const RangingVec3& target_velocity) noexcept {
    ArtilleryLeadPushOut out{};
    // 006DF876..006DF899: s = distance * g / speed^2, stored as a float.
    const float s = static_cast<float>(static_cast<double>(distance) * kShipGunneryGravity
                                       / (static_cast<double>(class_speed) * class_speed));
    // 006DF8A7: 1.0 < s takes pi/4; otherwise asin(s) * 0.5 (0042CF10).
    out.pitch = s > 1.0f ? kShipGunneryQuarterPi
                         : static_cast<float>(std::asin(static_cast<double>(s)) * 0.5);
    // 006DF8D2..006DF911: time = distance / (speed * cos(pitch) * time_scale).
    const float horizontal = static_cast<float>(
        static_cast<double>(class_speed) * std::cos(static_cast<double>(out.pitch)) * time_scale);
    out.flight_time = distance / horizontal;
    // 006DF915..006DF937: (vx * t, 0.0 * t, vz * t).
    out.push_out = RangingVec3{target_velocity.x * out.flight_time,
                               static_cast<float>(0.0 * out.flight_time),
                               target_velocity.z * out.flight_time};
    return out;
}

float artillery_launch_vertical_bias_006e85d2(int class_sub_type) noexcept {
    if (class_sub_type < 4 || class_sub_type > 7) return 0.0f;
    return static_cast<float>(kArtilleryLaunchBiasStep * kShipGunneryGravity);
}

float projectile_scaled_step_006e675a(float time_scale, float step) noexcept {
    return time_scale * step;
}

float artillery_launch_bias_shortfall(float class_speed, float pitch) noexcept {
    const double bias = static_cast<double>(artillery_launch_vertical_bias_006e85d2(4));
    return static_cast<float>(class_speed * std::cos(static_cast<double>(pitch)) * 2.0 * bias
                              / kShipGunneryGravity);
}

}  // namespace bsp
