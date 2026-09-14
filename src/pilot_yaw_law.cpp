#include "bsp/pilot_yaw_law.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"

namespace bsp {
namespace {

float clamp_unit(float v) noexcept {
    if (v < -1.0f) return -1.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

}  // namespace

float pilot_yaw_desired_0099ea3e(const PilotYawTuning& tuning,
                                 const PilotYawInputs& in) noexcept {
    // 0099DDD0 stores 0 into the error slot before the gate, so every path that
    // skips the heading block runs with a zero error rather than a stale one.
    float error = 0.0f;

    // 0099DDBE / 0099DE8A. Only the value 2 enables the heading term.
    if (in.heading_mode_2cc == 2) {
        // 0099DEAF / 0099DEB8: the difference is taken through 00438B10, which
        // wraps into (-pi, pi], so a heading hold across the +-pi seam turns the
        // short way round rather than the long way.
        error = wrapped_angle_subtract_00438b10(in.desired_heading, in.current_heading);
        // 0099DEBD, the frame's 1 / max(unit+340h * 0.4, 1.0). A zero scale_340
        // makes this the identity.
        float denom = in.scale_340 * 0.4f;
        if (denom < 1.0f) {
            denom = 1.0f;
        }
        error /= denom;

        // 0099DEC5..0099DEFC: a deadband of SoftHdgZone, subtracted rather than
        // snapped, so the response is continuous across it instead of stepping.
        const float zone = tuning.soft_hdg_zone;
        if (error >= zone) {
            error -= zone;
        } else if (error <= -zone) {
            error += zone;
        } else {
            error = 0.0f;
        }

        // 0099DF0F..0099DF24: a soft limit whose width comes from the class's own
        // turn authority. Inside it the error is scaled by SoftHdgMul; outside,
        // the same scaling is applied as a constant offset, which keeps the two
        // pieces continuous at |error| == limit.
        const float limit = (in.turn_roll_spd + in.pitch_spd) * tuning.soft_hdg_limit;
        const float mul = tuning.soft_hdg_mul;
        if (limit > std::fabs(error)) {
            error = mul * error;
        } else if (error > 0.0f) {
            error -= (1.0f - mul) * limit;
        } else {
            error += (1.0f - mul) * limit;
        }
    }

    // 0099DFFB..0099E027: the heading term's weight falls from 1 to 0 as the
    // plane banks over, because a banked plane turns with its elevator rather
    // than its rudder.
    const float g = clamped_interpolate_00419010(
        tuning.yaw_turn_roll_range_1, 1.0f,
        tuning.yaw_turn_roll_range_2, 0.0f, std::fabs(in.bank));

    // 0099E820..0099E884.
    float base = 0.0f;
    if (g > 0.0f) {
        const float denom = in.yaw_spd * std::cos(in.bank) * tuning.yaw_ctrl_set_time_mul;
        base = clamp_unit(denom != 0.0f ? error / denom : 0.0f) * g;
    }

    // 0099E88E..0099EA3E. The turn term is the mirror image: it uses sin(bank)
    // where the base uses cos(bank), and it blends in as the base blends out.
    float yaw = base;
    if (in.turn_numerator > 0.0f) {
        const float sin_bank = std::sin(in.bank);
        const float denom = in.yaw_spd * sin_bank * tuning.yaw_ctrl_set_time_mul;
        const float turn = clamp_unit(denom != 0.0f ? in.turn_numerator / denom : 0.0f);
        float t = clamped_interpolate_00419010(
            tuning.yaw_turn_roll_range_1, 0.0f,
            tuning.yaw_turn_roll_range_2, 3.0f, std::fabs(in.bank));
        if (t > 1.0f) {
            t = 1.0f;
        }
        yaw = t * turn + (1.0f - t) * base;
    }
    return clamp_unit(yaw);
}

}  // namespace bsp
