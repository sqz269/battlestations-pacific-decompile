#include "bsp/dive_bomb_aimdive_tail.hpp"

#include "bsp/dive_bomb_task.hpp"  // dive_bomb_interpolate_clamped_00419010

#include <cmath>

// Packet cc9_aimdive_response. See include/bsp/dive_bomb_aimdive_tail.hpp.

namespace bsp {
namespace {

// The branch-free `v > 0 ? v : -0.0f - v` fold the listing uses at 009C5E1A
// and 009C5E91 (the -0.0 at 00D7A208).
float fold(float v) noexcept { return (v > 0.0f) ? v : (-0.0f - v); }

}  // namespace

DiveBombAimDiveTail dive_bomb_aimdive_tail_009c5db8(
    const DiveBombAimDiveTailInputs& in) noexcept {
    namespace c = dive_bomb_tail_constant;
    DiveBombAimDiveTail out;

    // 009C5E06-009C5E5D: yaw = x / |z| * 8.0, the product in x87, stored float.
    {
        const float az = fold(in.aim_local_z);
        const float ratio = in.aim_local_x / az;   // 009C5E38 FDIV, FSTP float at 009C5E4F
        out.yaw_284 = static_cast<float>(static_cast<double>(ratio) * c::kYawGain);
    }

    // 009C5E69-009C5F15: the pull-out margin, only while nose-down.
    float margin = 0.0f;                     // 009C5E49 stores 0.0 first
    float t = -0.0f - in.pitch_c64;          // 009C5E6F SUBSS from -0.0
    if (t > 0.0f) {                          // 009C5E77 COMISS, 009C5E80 JBE
        const float b = fold(in.bank_c68);
        if (static_cast<double>(b) > c::kHalfPi) {   // 009C5EB2 FCOMIP, JBE
            t = static_cast<float>(c::kPi - static_cast<double>(t));  // 009C5EC2
        }
        const float cos_t = static_cast<float>(std::cos(static_cast<double>(t)));  // 009C5ECA
        // 009C5EE0-009C5EF5: speed * (t / PitchSpd), one x87 chain.
        const double arc = static_cast<double>(in.speed_vtable38) *
            (static_cast<double>(t) / static_cast<double>(in.pitch_spd_1ac));
        const float one_minus = 1.0f - cos_t;                     // 009C5EFF
        const float drop = static_cast<float>(arc * one_minus);   // 009C5F07
        margin = static_cast<float>(static_cast<double>(drop) + c::kPullOutMarginBase);
    }
    out.pull_out_margin = margin;

    // 009C5F1C-009C5F2A: below the release floor, cut the power, full brake.
    if (in.release_alt_a8 > in.height_above_aim_14) {
        out.below_floor = true;
        out.throttle_278 = c::kFloorThrottle;   // 009C5F37
        out.air_brake_2a8 = 1.0f;               // 009C5F3F -> 009C6071
        return out;
    }

    // 009C5F4C-009C5F66: |pitch command|.
    const float a = fold(in.pitch_command);
    // 009C5F6F-009C5F9E: the power cap, max power at a level stick falling to
    // min power as |pitch| x AimPitchRatio reaches 1.
    const float x1 = in.aim_pitch_ratio_54 * a;   // FSTP float at 009C5F79
    const float power_cap = dive_bomb_interpolate_clamped_00419010(
        0.0f, in.max_power_44, 1.0f, in.min_power_48, x1);
    // 009C5FA7-009C5FCB: the brake floor, min brake at a level stick rising to
    // max brake at a full one.
    const float brake_floor = dive_bomb_interpolate_clamped_00419010(
        0.0f, in.min_brake_50, 1.0f, in.max_brake_4c, a);
    // 009C5FD4-009C6013: v = clamp((h - margin) / 120, 0.01, 1).
    float v = static_cast<float>(
        (static_cast<double>(in.height_above_aim_14) - static_cast<double>(margin)) /
        c::kPowerHeightBand);
    if (c::kPowerRampMin > v) {
        v = c::kPowerRampMin;
    } else if (v > c::kPowerRampMax) {
        v = c::kPowerRampMax;
    }
    // 009C6016-009C602A: throttle = min(v, power cap).
    out.throttle_278 = (v > power_cap) ? power_cap : v;
    // 009C6030-009C604E: air brake = max(brake floor, 1 - v). 009C6044 is JA.
    const float one_minus_v = 1.0f - v;
    out.air_brake_2a8 = (brake_floor > one_minus_v) ? brake_floor : one_minus_v;
    return out;
}

float tuning_min_control_multiplier_007e41df(float control_range_min,
                                             float control_range_max,
                                             float stall_range_max,
                                             float level_flight) noexcept {
    namespace k = flyabove_speed_constant;
    // 007E413B-007E415D: min + (max - min) * 0.85, stored float.
    const float lerp = static_cast<float>(
        static_cast<double>(control_range_min) +
        (static_cast<double>(control_range_max) - control_range_min) * k::kControlRangeLerp);
    const float stall = static_cast<float>(stall_range_max * k::kStallRangeScale);  // 007E416D
    const float level = static_cast<float>(level_flight * k::kLevelFlightScale);    // 007E417D
    float m = (lerp > stall) ? lerp : stall;      // 007E4189-007E4197
    m = (m > level) ? m : level;                  // 007E41A7-007E41B1, JA keeps m
    return (m > level_flight) ? level_flight : m; // 007E41CB-007E41D5
}

float plane_min_control_speed_007c4810(float tuning_28c, float stall_spd_184) noexcept {
    // 007C4819 FLD [EAX+28Ch], 007C481F FMUL [ESI+184h], 007C4826 FSTP float.
    return static_cast<float>(static_cast<double>(tuning_28c) * stall_spd_184);
}

float dive_bomb_flyabove_desired_speed_009c6f97(float approach_a4,
                                                float min_control_speed,
                                                float slot_entry_108) noexcept {
    // 009C6FA8-009C6FC5: (double)approach+A4h - 007C4810, stored float.
    const float margin = static_cast<float>(
        static_cast<double>(approach_a4) - static_cast<double>(min_control_speed));
    double d;
    if (slot_entry_108 > 0.0f) {                 // 009C6FC2 COMISS, 009C6FC9 JBE
        d = static_cast<double>(slot_entry_108) * flyabove_speed_constant::kPositiveGain;
    } else {
        d = static_cast<double>(margin) * slot_entry_108;   // 009C6FDB
    }
    // 009C6FE1 FADD approach+A4h, 009C6FFB FSTP float.
    return static_cast<float>(d + approach_a4);
}

DiveBombAimGlideThrottle dive_bomb_aimglide_throttle_009c55e5(
    float planar_to_aim_10, float planar_to_impact_14,
    float max_power_44, float min_power_48) noexcept {
    namespace k = dive_bomb_glide_throttle_constant;
    DiveBombAimGlideThrottle out;
    // 009C534B-009C5353: FLD [ESP+10h], FDIV [ESP+14h], FSTP dword. A zero
    // divisor is not guarded in the image either.
    out.ratio_24 = planar_to_aim_10 / planar_to_impact_14;
    const float t = dive_bomb_interpolate_clamped_00419010(
        k::kRatioLo, min_power_48, k::kRatioHi, max_power_44, out.ratio_24);
    out.throttle_278 = 0.0f > t ? 0.0f : (t > 1.0f ? 1.0f : t);
    const float b = -0.0f - t;
    out.air_brake_2a8 = 0.0f > b ? 0.0f : (b > 1.0f ? 1.0f : b);
    return out;
}

DiveBombAimGlidePitch dive_bomb_aimglide_pitch_009c5484(
    const DiveBombAimGlidePitchInputs& in) noexcept {
    namespace k = dive_bomb_glide_pitch_constant;
    DiveBombAimGlidePitch out;
    // 009C5497-009C54AD: the difference, stored dword, sign bit cleared.
    const float diff = in.planar_to_aim_10 - in.planar_to_impact_14;
    const float d = std::fabs(diff);
    // 009C54B5 FLD1; FCOMIP; JBE: 1.0 > d takes 1.0.
    out.distance_d = k::kMinDistance > d ? k::kMinDistance : d;
    // 009C54CD-009C54DF: (C - H) / D, stored dword; 009C54E7 atan, stored dword.
    const float q = (in.release_ceiling_1c - in.height_above_aim_20) / out.distance_d;
    const float a = static_cast<float>(std::atan(static_cast<double>(q)));
    // 009C550A-009C5522: [ESP+28h] > climb angle takes the climb angle.
    out.glide_angle = a > in.climb_angle_1ec ? in.climb_angle_1ec : a;
    out.p1 = dive_bomb_interpolate_clamped_00419010(
        k::kRatioLo, k::kSteepest, k::kRatioHi, out.glide_angle, in.ratio_24);
    const float x_hi = static_cast<float>(
        static_cast<double>(in.release_ceiling_1c) * k::kCeilingHi);
    const float x_lo = static_cast<float>(
        static_cast<double>(in.release_ceiling_1c) * k::kCeilingLo);
    out.p2 = dive_bomb_interpolate_clamped_00419010(
        x_lo, 0.0f, x_hi, k::kSteepest, in.height_above_aim_20);
    out.pitch_target_2bc = out.p1 > out.p2 ? out.p1 : out.p2;
    return out;
}

DiveBombAimGlideSteer dive_bomb_aimglide_steer_009c53d0(float miss_1c,
                                                        float signed_error) noexcept {
    namespace k = dive_bomb_glide_steer_constant;
    DiveBombAimGlideSteer out;
    // 009C53D8 COMISS xmm0(140), [ESP+1Ch]; 009C53E5 JBE: 140 <= miss goes on
    // to the heading arm.
    if (!(k::kYawArmMiss > miss_1c)) {
        return out;
    }
    out.yaw_arm = true;
    out.yaw_284 = static_cast<float>(static_cast<double>(signed_error) * k::kYawGain);
    return out;
}

}  // namespace bsp
