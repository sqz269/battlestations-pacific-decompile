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

}  // namespace bsp
