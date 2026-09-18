// The range-curve object the ship AI's standoff choice samples: 00954940 (the
// clear), 00955A40 (the sample), 009523C0 (the peak) and 00952530 (the last
// range with a positive sample).
//
// Packet cc8_ship_ai_approach_curves, worker agent/cc8-ship-approach-curves.
// Project C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra
// was READ-ONLY for this packet. Every descriptive name here is a hypothesis,
// not a recovered symbol. docs/SHIP_AI_APPROACH_CURVES.md carries the evidence
// address by address; reports/ship_ai_approach_curves.json carries the rows.
//
// What the object is. It is a bare array of 60 floats, no header and no
// vtable: 00954940 stores zero into 0x3C dwords from `this` (00954940 whole),
// 00955A40 indexes `[ECX + i*4]` with a bound of 0x3B and reads `[ECX+0ECh]`
// for the saturated tail, and 009523C0 walks `param_1[0]..param_1[0x3B]`. The
// two instances live at nested+12C0h and nested+13B0h, 0F0h = 60*4 apart, and
// nested+14A0h (the traffic list) follows the second.
//
// What fills it: 0095F080, the 60-sample range profile of
// docs/SHIP_AI_BEARING_RATING.md, which calls the expected-damage estimate
// 0095EB40 sixty times stepping the query range by the 50.0 double at
// 00CE3938. Sample i is therefore the rating at 50*(i+1) metres, which is the
// same 50.0 constant 00955A40 divides by. 009F1BC0 owns both instances:
// 009F2F11 fills nested+12C0h from the OWN unit with prefer_long_range = 1 and
// 009F2FB1 fills nested+13B0h from the TARGET with prefer_long_range = 0
// (docs/SHIP_AI_BEARING_RATING.md, "0095F080, the 60-sample range profile").
//
// So the sampled quantity is expected damage over a 20 s window, and the two
// curves are "what we can do to them at range x" and "what they can do to us
// at range x". The scan at 009E71A5 minimises
//   max(1, them(x)) * (nested+1284h / us(x)) * interp(0, 2, 1, 1, us(x)/peak)
// over x = 50, 75, ... 3000.

#ifndef BSP_SHIP_AI_APPROACH_CURVES_HPP
#define BSP_SHIP_AI_APPROACH_CURVES_HPP

#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------

// 00954940, `float* __fastcall(float* this)`, body 00954940-00954951.
inline constexpr std::uint32_t kShipAiApproachCurveClearAddress = 0x00954940u;
// 00955A40, `float __thiscall(float* this, float x)`, RET 4 at 00955A66,
// 00955A8A and 00955AC2; body 00955A40-00955AC4.
inline constexpr std::uint32_t kShipAiApproachCurveSampleAddress = 0x00955A40u;
// 009523C0, `float __fastcall(float* this)`, body 009523C0-00952525.
inline constexpr std::uint32_t kShipAiApproachCurvePeakAddress = 0x009523C0u;
// 00952530, `float __fastcall(float* this)`, body 00952530-009525B4.
inline constexpr std::uint32_t kShipAiApproachCurveEffectiveRangeAddress = 0x00952530u;

// ---------------------------------------------------------------------------
// Shape
// ---------------------------------------------------------------------------

// 0x3C dwords (00954940's `MOV ECX,3Ch` fill), the 0x3B bound of 00955A40 and
// the `iVar2 < 0x3C` tail of 009523C0 all agree on sixty.
inline constexpr int kShipAiApproachCurveSamples = 60;

// 00CE3938, the double 50.0 (bytes 00 00 00 00 00 00 49 40). 00955A40 both
// subtracts it and divides by it, so sample i sits at x = 50*(i+1) and the
// curve spans 50 m to 3000 m. 0095F080 steps its query range by the same
// double at 0095F140.
inline constexpr double kShipAiApproachCurveStep = 50.0;

// The object itself. No constructor of its own runs in the image: 009E5530
// (009E55C3, 009E55CE) and 009E6E80 (009E6E8F) both reach it through 00954940,
// which is a plain zero fill.
struct ShipAiApproachRangeCurve {
    float samples[kShipAiApproachCurveSamples];
};

static_assert(sizeof(ShipAiApproachRangeCurve) == 0xF0,
              "nested+12C0h and nested+13B0h are 0F0h apart");

// ---------------------------------------------------------------------------
// The four routines, as pure rules
// ---------------------------------------------------------------------------

// 00954940 whole: `for (i = 3Ch; i; --i) *p++ = 0;`, returning `this`. Callers
// treat it as the curve's constructor.
void ship_ai_approach_curve_clear_00954940(ShipAiApproachRangeCurve& curve) noexcept;

// 00955A40 whole. `t = x - 50.0` in x87 double, stored back to the argument
// slot as a float (00955A4F); `t <= 0` returns samples[0] (00955A63, FLD
// [ECX]); otherwise `t = t / 50.0` stored as a float (00955A6F), `i = (int)t`
// by CVTTSS2SI (00955A73), `i >= 0x3B` returns samples[59] (00955A83, FLD
// [ECX+0ECh]); otherwise the clamped interpolation 00419010(0, samples[i], 1,
// samples[i+1], t - i) with the fraction formed by FISUB and stored as a float
// (00955A94, 00955A98).
float ship_ai_approach_curve_sample_00955a40(const ShipAiApproachRangeCurve& curve,
                                             float x) noexcept;

// 009523C0 whole: the largest of the sixty samples, seeded with samples[0]
// (a running max in an 8-wide unrolled loop over 1..56 plus a 57..59 tail).
// The scan at 009E71EF uses it as the denominator that normalises the own
// curve, so it is the unit's best expected damage at any range.
float ship_ai_approach_curve_peak_009523c0(const ShipAiApproachRangeCurve& curve) noexcept;

// 00952530 whole: walks DOWN from index 59 in blocks of six looking for the
// first sample greater than 0.0 and returns `(index + 1) * 50.0`, which is
// that sample's range; with no positive sample the index reaches -1 and the
// result is 0.0. It is the longest range at which the curve still bites.
float ship_ai_approach_curve_effective_range_00952530(
    const ShipAiApproachRangeCurve& curve) noexcept;

}  // namespace bsp

#endif  // BSP_SHIP_AI_APPROACH_CURVES_HPP
