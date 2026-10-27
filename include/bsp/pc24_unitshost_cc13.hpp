#pragma once

// PC24 wave switch for the units host (cc13 pc24w1, packets pc24w1-units-*):
// the bodies inlined into GameUnitsHost's motion step (game_hosts_units.cpp).
// The game's main thread runs the x87 with 24-bit precision control, so every
// FADD/FSUB/FMUL/FDIV/FSQRT result is rounded to a 24-bit significand
// (docs/CC13_FPU_PRECISION.md). The host bodies evaluated multi-op chains in
// double and rounded once; the ON arms round per op through the cc13x87
// helpers in the image's operation order. Each OFF arm is the previous code,
// unchanged. None of these bodies runs under DynWorkerPrecisionScope.

#include "bsp/cc13_x87_model.hpp"

namespace bsp {

// kPc24UnitsHostCc13Bound (OFF): per-op PC24 arithmetic in
//   units-A (cc13-pc24w1-units-A), the dogfight host bodies:
//     004142E0 df_local (the host's d . row / |row| stand-in for the vector
//              times unit+110h)
//     007B4E90 pattern df_heading_of (pi/2 - atan2, +2pi wrap; 009A8873-009A889A)
//     009A87F0 df_maneuver_enter_009a87f0 (009A89BD-009A89F5)
//     009A8B20 df_maneuver_tick_009a8b20 (009A8CB3-009A8CC7 ceiling; the speed
//              is the vtable[38h] stand-in)
//     009A80E0 df_avoid_turn_tick_009a80e0 (009A82CD-009A8312)
//     00954650 df_predict_00954650 with 0042B2F0, 004F9B30, 00952340, 0085C3F0
//     009FC7C0 df_set_lead_bound's dist (009FC931-009FC9F6, 0042B2F0) and
//              df_fine_aim_009f9fc0 (009FCB25 length; the +68h world rebuild is
//              a host stand-in)
//     009AAA80 df_early_edge_009aaa80 (009AAB96-009AAC31; speed is the
//              vtable[38h] stand-in)
//     009A76E0 df_arm_body_009ab1c0 (tan_len 00414C60 at 009A77F4; target
//              speed stand-in)
// Expected movers when ON: dogfight lead points and fine-aim angles, the arc
// predictor's arc/straight verdicts, maneuver headings and pitch, the avoid
// turn's bank decay, and early-edge target pickups.
inline constexpr bool kPc24UnitsHostCc13Bound = false;

namespace pc24_unitshost {

// 0042B2F0: (y*y + x*x) + z*z, stored as a float, sqrt when above 1e-10
// [00CE3820], else 0; the result is stored as a float.
inline float length3_0042b2f0(float x, float y, float z) noexcept {
    using namespace bsp::cc13x87;
    const float s = static_cast<float>(
        x87_add24(x87_add24(x87_mul24(y, y), x87_mul24(x, x)), x87_mul24(z, z)));
    if (!(static_cast<double>(s) > 1e-10)) return 0.0f;
    return static_cast<float>(x87_sqrt24(s));
}

// 00414C60: x*x + y*y, the same cutoff and stores.
inline float length2_00414c60(float x, float y) noexcept {
    using namespace bsp::cc13x87;
    const float s = static_cast<float>(x87_add24(x87_mul24(x, x), x87_mul24(y, y)));
    if (!(static_cast<double>(s) > 1e-10)) return 0.0f;
    return static_cast<float>(x87_sqrt24(s));
}

// (a0*b0 + a1*b1) + a2*b2, each op rounded, the sum stored as a float.
inline float dot3(const float a[3], const float b[3]) noexcept {
    using namespace bsp::cc13x87;
    return static_cast<float>(x87_add24(x87_add24(x87_mul24(a[0], b[0]), x87_mul24(a[1], b[1])),
                                        x87_mul24(a[2], b[2])));
}

// 004F9B30: out = a x b, out_i = a_j*b_k - a_k*b_j, each component stored.
inline void cross_004f9b30(const float a[3], const float b[3], float out[3]) noexcept {
    using namespace bsp::cc13x87;
    out[0] = static_cast<float>(x87_sub24(x87_mul24(a[1], b[2]), x87_mul24(a[2], b[1])));
    out[1] = static_cast<float>(x87_sub24(x87_mul24(a[2], b[0]), x87_mul24(a[0], b[2])));
    out[2] = static_cast<float>(x87_sub24(x87_mul24(a[0], b[1]), x87_mul24(a[1], b[0])));
}

}  // namespace pc24_unitshost

}  // namespace bsp
