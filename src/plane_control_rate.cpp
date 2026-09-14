#include "bsp/plane_control_rate.hpp"

#include <cmath>

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

}  // namespace bsp
