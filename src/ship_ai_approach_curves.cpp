// Packet cc8_ship_ai_approach_curves. Evidence: docs/SHIP_AI_APPROACH_CURVES.md.
//
// Every expression below is transcribed from the listings of 00954940,
// 00955A40, 009523C0 and 00952530, not from the decompiler output. The
// decompiler printed both operands of 00955A40's `x - 50.0` and `/ 50.0` as
// the same `_DAT_00ce3938` and warned about overlapping globals; the listing
// (00955A45 FLD double ptr [0x00ce3938], 00955A4B FSUB ST1,ST0, 00955A6B
// FDIVRP ST2,ST0) shows one double constant used twice, so the expression
// really is `(x - 50.0) / 50.0` and the index is `x/50 - 1`.
//
// The `static_cast<float>` calls sit exactly where the image stores to memory:
// 00955A4F (the difference), 00955A6F (the quotient, which CVTTSS2SI then
// truncates as a float, not as a double) and 00955A98 (the fraction). Keeping
// those rounds is what makes the index agree with the image at a sample
// boundary.

#include "bsp/ship_ai_approach_curves.hpp"

#include "bsp/unit_rudder.hpp"

namespace bsp {

void ship_ai_approach_curve_clear_00954940(ShipAiApproachRangeCurve& curve) noexcept
{
    // 00954940: MOV ECX,3Ch then REP STOSD of zero over the object.
    for (int i = 0; i < kShipAiApproachCurveSamples; ++i) {
        curve.samples[i] = 0.0f;
    }
}

float ship_ai_approach_curve_sample_00955a40(const ShipAiApproachRangeCurve& curve,
                                             float x) noexcept
{
    // 00955A41..00955A4F: x87 loads the argument, subtracts the 50.0 double at
    // 00CE3938 and stores the difference back over the argument slot as float.
    const float shifted = static_cast<float>(static_cast<double>(x)
                                             - kShipAiApproachCurveStep);

    // 00955A57 FLDZ, 00955A59 FCOMI ST0,ST1 (0.0 against the difference),
    // 00955A5B JC: only a strictly positive difference takes the scan; the
    // fall-through returns samples[0] (00955A63, FLD float ptr [ECX]).
    if (!(shifted > 0.0f)) {
        return curve.samples[0];
    }

    // 00955A6B FDIVRP, 00955A6F FSTP float ptr [ESP+8]: the quotient is
    // rounded to float before CVTTSS2SI reads it at 00955A73.
    const float scaled = static_cast<float>(static_cast<double>(shifted)
                                            / kShipAiApproachCurveStep);
    const int index = static_cast<int>(scaled); // CVTTSS2SI, truncation

    // 00955A79 CMP EAX,3Bh, 00955A7F JL: an index of 0x3B or more saturates on
    // the last sample, read as [ECX+0ECh] = samples[59] at 00955A83.
    if (index >= kShipAiApproachCurveSamples - 1) {
        return curve.samples[kShipAiApproachCurveSamples - 1];
    }

    // 00955A94 FISUB dword ptr [ESP+14h] against the saved index, 00955A98
    // FSTP float: the fraction is a float before 00419010 sees it. The call at
    // 00955ABC passes (0, samples[i], 1, samples[i+1], fraction).
    const float fraction = static_cast<float>(static_cast<double>(scaled)
                                              - static_cast<double>(index));
    return clamped_interpolate_00419010(0.0f, curve.samples[index], 1.0f,
                                        curve.samples[index + 1], fraction);
}

float ship_ai_approach_curve_peak_009523c0(const ShipAiApproachRangeCurve& curve) noexcept
{
    // 009523C0: the running maximum is seeded with samples[0] and the compiler
    // unrolled the walk eight wide over 1..56 with a 57..59 tail. Every test is
    // `if (best < sample) best = sample;`, so a curve of all-equal samples
    // keeps the first and NaNs would keep the seed; the image never tests for
    // emptiness.
    float best = curve.samples[0];
    for (int i = 1; i < kShipAiApproachCurveSamples; ++i) {
        if (best < curve.samples[i]) {
            best = curve.samples[i];
        }
    }
    return best;
}

float ship_ai_approach_curve_effective_range_00952530(
    const ShipAiApproachRangeCurve& curve) noexcept
{
    // 00952530: the walk starts at index 0x3B with the pointer at samples[57]
    // (param_1 + 0E4h) and tests [2], [1], [0], [-1], [-2], [-3] in turn,
    // subtracting 0..5 from the index and breaking on the first sample greater
    // than 0.0. With no hit the index drops by six and the pointer by six, and
    // the loop ends once the index reaches -1 (`while (-1 < iVar2)`), which is
    // exactly ten passes and never reads below samples[0].
    int index = kShipAiApproachCurveSamples - 1;
    while (index >= 0) {
        if (curve.samples[index] > 0.0f) {
            break;
        }
        --index;
    }
    if (index < 0) {
        index = -1;
    }

    // 009525A1: the answer is `(index + 1) * 50.0`, the range of that sample.
    return static_cast<float>(static_cast<double>(index + 1)
                              * kShipAiApproachCurveStep);
}

}  // namespace bsp
