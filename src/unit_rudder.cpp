#include "bsp/unit_rudder.hpp"
#include "bsp/dive_bomb_task.hpp"  // the two leaves moved from dive_bomb_task.cpp

#include <cmath>

namespace bsp {
namespace {
constexpr double native_pi = 3.1415927410125732421875; // 00CE3D28
constexpr double native_two_pi = 6.283185482025146484375; // 00CE3828
constexpr double native_quarter_pi = 0.785398185253143310546875; // 00CEDCD0

float wrap_native_angle(float value) noexcept {
    // 00438AB0..00438B0B / 00438B20..00438B7B. The per-iteration float
    // stores matter at the endpoints; replacing this loop with fmod changes it.
    if (static_cast<double>(value) <= -native_pi) {
        do {
            value = static_cast<float>(static_cast<double>(value) + native_two_pi);
        } while (static_cast<double>(value) <= -native_pi);
    } else {
        while (static_cast<double>(value) > native_pi) {
            value = static_cast<float>(static_cast<double>(value) - native_two_pi);
        }
    }
    return value;
}
}

float clamped_interpolate_00419010(float x0, float y0, float x1, float y1,
                                  float x) noexcept {
    if (x1 == x0) return y0; // 0041901E..00419030, ordered equality only
    float interpolated;
    // 00419033..0041905F: keep extended intermediates until the native FSTP.
    __asm {
        fld x
        fsub x0
        fld x1
        fsub x0
        fdivp st(1), st(0)
        fld y1
        fsub y0
        fmulp st(1), st(0)
        fadd y0
        fstp interpolated
    }
    const float upper = y0 > y1 ? y0 : y1;
    const float lower = y1 > y0 ? y0 : y1;
    // Native JBE branches accept unordered values; retain NaNs rather than
    // substituting std::clamp/min/max, whose ordering conventions may differ.
    if (lower > interpolated) return lower;
    if (interpolated > upper) return upper;
    return interpolated;
}

float unit_rudder_denominator_0082e890(float magnitude, UnitRudderCurveHost& host) {
    const float split = host.settings_00424c40().speed_044c;
    // FCOMIP split,magnitude / JC: unordered takes the upper segment too.
    if (magnitude <= split) {
        const auto& first = host.settings_00424c40();
        const auto& second = host.settings_00424c40();
        const auto& third = host.settings_00424c40();
        const float y1 = first.value_0448;
        const float y0 = second.value_0440;
        const float x0 = third.speed_0444;
        return clamped_interpolate_00419010(x0, y0, split, y1, magnitude);
    }
    const auto& first = host.settings_00424c40();
    const auto& second = host.settings_00424c40();
    const auto& third = host.settings_00424c40();
    const float y1 = first.value_0438;
    const float x1 = second.speed_043c;
    const float y0 = third.value_0448;
    return clamped_interpolate_00419010(split, y0, x1, y1, magnitude);
}

float unit_class_yaw_rate_0082ecb0(const ShipClassFields& ship_class, float rudder,
    float forward_speed, float turn_efficiency, UnitRudderCurveHost& host) {
    const float maximum_speed = ship_class.max_speed;
    float ratio;
    __asm {
        fld forward_speed
        fdiv maximum_speed
        fstp ratio
    }
    // 0082ECCA stores MaxRotAngle before the settings getter can run.
    const double maximum_angle = ship_class.max_rot_angle;
    const float denominator = unit_rudder_denominator_0082e890(std::fabs(ratio), host);
    float base;
    float result;
    __asm {
        fld denominator
        fdivr maximum_angle
        fstp base
        fld base
        fmul ratio
        fmul rudder
        fmul turn_efficiency
        fstp result
    }
    return result;
}

float unit_yaw_rate_00811890(float rudder, UnitRudderHost& host) {
    const bool manager_present = host.scale_manager_present();
    const auto& ship_class = host.ship_class();
    float gameplay_scale = 1.0f;
    if (manager_present && host.gameplay_scale_enabled() && host.scale_manager_enabled()) {
        gameplay_scale = host.gameplay_scale_008e6430(5);
    }
    const float efficiency = host.turn_efficiency();
    const float speed = host.forward_speed_0092d730();
    const float mapped = unit_class_yaw_rate_0082ecb0(ship_class, rudder, speed,
                                                   efficiency, host);
    if (!manager_present) return mapped;
    float result;
    __asm {
        fld mapped
        fmul gameplay_scale
        fstp result
    }
    return result;
}

float unit_current_yaw_rate_00811940(UnitRudderHost& host) {
    const float command = host.steering_command();
    return unit_yaw_rate_00811890(command, host);
}

float wrapped_angle_add_00438aa0(float left, float right) noexcept {
    float value;
    __asm {
        fld left
        fadd right
        fstp value
    }
    return wrap_native_angle(value);
}

float wrapped_angle_subtract_00438b10(float left, float right) noexcept {
    float value;
    __asm {
        fld left
        fsub right
        fstp value
    }
    return wrap_native_angle(value);
}

void unit_set_heading_target_00811960(UnitHeadingTargetState& state,
    float desired_heading, UnitHeadingTargetHost& host) {
    float heading = host.heading_virtual_0050();
    const float speed = host.forward_speed_0092d730();
    if (speed <= -1.0f) {
        heading = wrapped_angle_add_00438aa0(heading, static_cast<float>(native_pi));
    }
    float difference = wrapped_angle_subtract_00438b10(heading, desired_heading);
    if (static_cast<double>(difference) < -native_quarter_pi) {
        difference = static_cast<float>(-native_quarter_pi);
    } else if (static_cast<double>(difference) > native_quarter_pi) {
        difference = static_cast<float>(native_quarter_pi);
    }
    state.target_heading = wrapped_angle_subtract_00438b10(heading, difference);
    state.active = true;
}

void unit_shifted_update_00811ab0(UnitShiftedUpdateState& state, float seconds,
    UnitShiftedUpdateHost& host) {
    const float scale = state.scale_0340;
    if (scale > 1.0f) {
        __asm {
            fld scale
            fmul seconds
            fstp seconds
        }
    }
    copy_camera_matrix_004134f0(state.matrix_0074, state.matrix_0674);
    host.controller_update_0092f930(seconds);
    host.unit_virtual_00d8();
    const float timestamp = state.timestamp_0308;
    if (timestamp != 0.0f &&
        static_cast<double>(timestamp) < static_cast<double>(host.clock_00f876a4()) + 2.0) {
        state.field_02f8 = 0.0f;
    }
}

// The two leaves below moved verbatim from src/dive_bomb_task.cpp (bsp_game) so
// that bsp_core callers (segment2_camera_cc13, gameplay_settings_tail,
// plane_retreat_task, plane_level_bomb_task) link without bsp_game. Their
// declarations and switches stay in include/bsp/dive_bomb_task.hpp.

// 007BCC80, read whole; see the header for the listing and the sign argument.
float weapon_fall_time_007bcc80(float height_above_aim_point,
                                float unit_velocity_y) noexcept {
    // 007BCC86 COMISS 0,h / JC: the body runs when 0 < h or unordered, so a NaN
    // height computes (ON) instead of returning 0.
    const bool skip = kDiveBombPc24Cc13Bound ? (height_above_aim_point <= 0.0f)
                                             : !(height_above_aim_point > 0.0f);
    if (skip) return 0.0f;
    const float vy = unit_velocity_y -
        dive_bomb_constant::kFallTimeVerticalBias;       // 007BCCA4, 00E08E54
    // 007BCCB6 FMUL and 007BCCDA FDIV both take the QWORD at 00CF9058, and
    // 007BCCC2/007BCCCF round the discriminant and its root to float on the way.
    const double g = dive_bomb_constant::kGravity;
    if constexpr (kDiveBombPc24Cc13Bound) {
        // 007BCCB0-007BCCC2: FLD h, FADD ST0,ST0, FMUL qword g, FLD ST1 (vy),
        // FMULP ST2, FADDP: vy*vy + (2h)*g, each op rounded at PC24. g is
        // float-exact (00CF9058 = 4023 9EB8 6000 0000), so the double product
        // is exact and the cast is the one rounding.
        const float h2 = height_above_aim_point + height_above_aim_point;
        const float h2g = static_cast<float>(static_cast<double>(h2) * g);
        const float vv = vy * vy;
        const float disc = vv + h2g;
        const float root = std::sqrt(disc);                 // 007BCCCA-007BCCCF
        const float sum = root + vy;                        // 007BCCD7
        return static_cast<float>(static_cast<double>(sum) / g);   // 007BCCDA-007BCCE0
    } else {
        const float disc = static_cast<float>(
            static_cast<double>(vy) * vy + 2.0 * height_above_aim_point * g);
        const float root = std::sqrt(disc);
        return static_cast<float>((static_cast<double>(root) + vy) / g);
    }
}

// 00419010, five stack floats, RET 14h. Equal ordered endpoints return y0;
// otherwise the interpolation is clamped between the two y values whichever way
// round they are. Transcribed from docs/UNIT_RUDDER_CURVE.md's reconstruction.
float dive_bomb_interpolate_clamped_00419010(float x0, float y0, float x1, float y1,
                                             float x) noexcept {
    if (x1 == x0) {
        return y0;
    }
    const float v = ((x - x0) / (x1 - x0)) * (y1 - y0) + y0;
    const float hi = (y1 < y0) ? y0 : y1;
    const float lo = (y0 < y1) ? y0 : y1;
    // 0041908C-00419092 FCOMIP lo,v / JBE: only lo > v clamps; NaN passes on.
    if (v < lo) {
        return lo;
    }
    // 004190A9 FCOMIP v,hi / JBE 004190BC: v when v <= hi or unordered (ON).
    if constexpr (kInterpClamped00419010NanCc13Bound) {
        return (v > hi) ? hi : v;
    } else {
        return (v <= hi) ? v : hi;
    }
}
}
