#include "bsp/plane_control_rate.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"

namespace bsp {
namespace {

// 007DAB79..007DAB8A, and again at 007DAB99 and 007DAD0F: MSVC's fabs here is
// `-0.0f - x` under a sign test rather than a bit mask, the -0.0f coming from
// 00D7A208. The two agree on every finite input and on infinities; they differ
// only in which NaN payload survives, which nothing downstream reads.
float absolute(float value) noexcept {
    return std::fabs(value);
}

// 007DABEA..007DAC06, transcribed branch for branch rather than reduced to a
// sign comparison. Reduced, it reads "the floor applies unless delta and
// current are strictly the same sign", but the original is a four-branch chain
// and two of its tests are against zero in the opposite operand order, which is
// what decides the unordered cases.
bool floor_applies(float delta, float current) noexcept {
    if (!(delta < 0.0f)) {          // 007DABED JC 007DABF9, not taken
        if (!(0.0f < current)) {    // 007DABF7 JNC 007DAC08
            return true;
        }
    }
    if (0.0f < delta) {             // 007DABFC JC 007DAC59
        return false;
    }
    if (current < 0.0f) {           // 007DAC06 JC 007DAC59
        return false;
    }
    return true;                    // falls into 007DAC08
}

}  // namespace

float plane_control_axis_step_007da710(const PlaneRotationFactors& factors,
                                       const PlaneControlAxisState& axis,
                                       bool floor_from_deflection,
                                       float step) noexcept {
    // 007DAB52..007DAB75. The subtraction is `target - current`, formed by an
    // FSUB of the axis field from a duplicated target, which is also what
    // leaves each target on the x87 stack for the clamp to reuse later.
    const float delta = axis.target - axis.current;

    // 007DABB2..007DABE0. The square is formed as delta*delta, so A's term does
    // not depend on the sign; B's does, through the fabs at 007DAB99.
    float rate = (factors.a * delta * delta + factors.b * absolute(delta) + factors.c) *
                 absolute(axis.accel);

    if (!floor_from_deflection) {
        // 007DAC48..007DAC53. COMISS then JA, so the floor is taken on
        // `!(rate > idle_floor)` - equality keeps the floor's value, which is
        // the same number either way.
        if (!(rate > factors.idle_floor)) {
            rate = factors.idle_floor;
        }
    } else if (floor_applies(delta, axis.current)) {
        // 007DAC08..007DAC40. The deflection is made positive by masking the
        // sign bit out of the float's integer image (AND ECX,7FFFFFFFh at
        // 007DAC12) rather than by arithmetic, then scaled by the 1.5 at
        // 00CE3D78 and compared with FCOMIP/JA.
        const float deflection_floor = factors.crossing_gain * absolute(axis.current);
        if (!(rate > deflection_floor)) {
            rate = deflection_floor;
        }
    }

    // 007DAC9B..007DACC1. XORPS XMM0,XMM0 at 007DAB40 puts zero in the left
    // comparand and nothing rewrites it before the compare, so this is
    // sign(delta) and not sign(-delta): an axis moves toward its target.
    float sign = 0.0f;
    if (delta < 0.0f) {
        sign = -1.0f;
    } else if (delta > 0.0f) {
        sign = 1.0f;
    }

    // 007DACC1..007DACD8. The sign is FILD-ed as an integer, so the step is a
    // constant rate rather than anything proportional to the error - the error
    // only enters through `rate`.
    const float candidate = axis.current + sign * rate * step;

    // 007DAC59..007DAC9E build the two bounds by comparing the axis against its
    // own target, and 007DACE0..007DAD01 applies them in this order. Written as
    // the original's three-way branch, not as std::clamp, because the order
    // decides the result when the bounds are inverted or unordered.
    const float upper = (axis.current > axis.target) ? axis.current : axis.target;
    const float lower = (axis.target > axis.current) ? axis.current : axis.target;
    if (lower > candidate) {
        return lower;
    }
    if (candidate > upper) {
        return upper;
    }
    return candidate;
}


PlaneControlTargets plane_control_targets_007da710(const PlaneControlClass& cls,
                                                   const PlaneControlUnitState& unit,
                                                   float mode_factor_f1,
                                                   float mode_factor_f2,
                                                   bool slide_and_coupling_flag) noexcept {
    PlaneControlTargets out;

    // ---- roll, frame=-76, live at 007DAA78 (or 007DA8E5 when the flag is 0) --
    // 007DA732..007DA74C: the base is unit+838h plus the latched roll.
    float roll = unit.roll_base_838 + unit.latched_roll;
    // 007DA750..007DA7B5: only flight state 6 engages the bank-angle factor.
    // Free flight is 7, so this is 1.0 in the air and the branch is here for
    // faithfulness rather than for effect.
    float bank_factor = 1.0f;
    if (unit.flight_state_900 == 6) {
        bank_factor = clamped_interpolate_00419010(
            0.0f, 1.0f, 0.10471976f, -0.1f, std::fabs(unit.bank_angle_c68));
    }
    // 007DA7C2..007DA7DA.
    roll = cls.roll_spd * mode_factor_f1 * bank_factor * roll;
    // 007DA7B9 / 007DA7DE: the out-of-action ramp and the spin term. This host
    // does not model unit+5Dh, unit+C36h/C37h or the lost-drag timer's use here,
    // so an undamaged plane takes neither, and the flag is carried in the input
    // so that a host which does model them has somewhere to say so rather than
    // this being silently absent.
    //
    // 007DA8D9..007DA8E5: zeroed on the ground. The sense is `mode == 1`, not
    // `!= 1` - the Ghidra plate comment had it inverted for one packet.
    if (unit.controller_mode_fc == 1) {
        roll = 0.0f;
    }

    // ---- yaw, frame=-8, live at 007DAA97 -----------------------------------
    // 007DA926..007DA953: the raw product, with unit+BC4h in it.
    const float yaw_raw = cls.yaw_spd * unit.latched_yaw * unit.yaw_scale_bc4;
    // 007DA957..007DA95F.
    float yaw = yaw_raw * mode_factor_f2;

    // 007DA9F5: one flag gates both the slide term and the yaw-roll coupling.
    if (slide_and_coupling_flag) {
        // 007DAA3B..007DAA58, SUBTRACTED (007DAA56 FSUBP, DE E9).
        yaw -= cls.slide_ratio * cls.yaw_spd *
               std::sin(unit.bank_angle_c68) * std::cos(unit.pitch_angle_c64);
        // 007DAA5C..007DAA78, added to the ROLL target - the coupling a
        // coordinated turn needs, and it uses the raw yaw product rather than
        // the mode-scaled one.
        roll += cls.yaw_roll_ratio * mode_factor_f1 * yaw_raw;
    }
    // 007DAA85..007DAA97: negated on the way to its slot, against the -0.0f at
    // 00D7A208. A sign error here turns every aircraft the wrong way while every
    // downstream number still looks validated.
    const float yaw_target = -yaw;

    // ---- pitch, frame=-72, live at 007DA912 / 007DA922 ---------------------
    float pitch = cls.pitch_spd * mode_factor_f1 * unit.latched_pitch;
    if (unit.latched_pitch < 0.0f) {          // 007DA8F9 / 007DA916
        pitch *= cls.negative_pitch_ratio;
    }

    // The order is the one the axes sit in: ctl+48h pitch, +4Ch yaw, +50h roll.
    out.target[0] = pitch;
    out.target[1] = yaw_target;
    out.target[2] = roll;

    // 007DAA7F / 007DAAA1 / 007DAAB3. Symmetric; only the magnitude is used
    // downstream, so the yaw term's negation does not survive into the rate.
    out.accel[0] = cls.pitch_accel * mode_factor_f1;
    out.accel[1] = cls.yaw_accel * -mode_factor_f2;
    out.accel[2] = cls.roll_accel * mode_factor_f1;
    return out;
}

}  // namespace bsp
