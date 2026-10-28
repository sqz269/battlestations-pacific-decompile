#pragma once

// cc13-flyavoid (census S7, src/game_hosts_units.cpp, the 009C62B0 flyabove
// heading arm). The avoidance increment 009C6D42-009C6D59 adds to the dead-band
// heading A before the slew 009C6D6F:
//   009C6AA3-009C6B3A  007F0280(mode 1) with the flyabove extents (80,70,140) /
//                      (60,70,90) by flyabove+18h, weights (30,0,0)
//                      (near_field_flyover_extents_009c6aa3); out_a at esp+68h,
//                      out_b at esp+74h
//   009C6B6F           inc (esp+20h) = 0.0
//   009C6B3F-009C6B75  |out_a.x| > qword [00D7A270] (0.05f widened), else inc stays 0
//   009C6B7B-009C6B9E  X = (-out_a.x * out_b.y) * out_b.z, each FMUL at PC24, FSTP float
//   009C6BC8-009C6C0A  X > float 0.5 (00CE3800): (X - 0.5) * 2.0 (qword 00D7A280,
//                      00D7A308); -0.5 (00CE69D0) > X: (X + 0.5) * 2.0; else 0
//   009C6C10-009C6C1E  flyabove+18h clear: X *= 0.5 (00D7A280)
//   009C6C22-009C6C55  R1 = 00419010(150, 0.2, 350, 1.0, R) (00CE3808, 00CE54A0,
//                      00D1F4C8), R = esp+28h, the lead-point range stored at
//                      009C63A6 / 009C63B1 (db_flyabove_lead_range)
//   009C6C5B-009C6C77  Y = float(X * qword 00D1F6C0 (25 degrees, the float widened));
//                      esp+20h = float(R1 * Y)
//   009C6C7D-009C6CA9  flyabove+18h clear: K = 00419010(500, 0, 800, 1.0, R)
//                      (00CE397C, 00CE3950); set: K = R1 (still in ST0)
//   009C6CB2-009C6CB6  inc = float(K * esp+20h)
//   009C6D42-009C6D59  A = 00438AA0 AddWrappedAngle(A, inc), FSTP to esp+44h
// ABI: 009C62B0's inline arm, no call boundary; the outputs are 007F0280's.

#include "bsp/cc13_x87_model.hpp"
#include "bsp/dive_bomb_task.hpp"
#include "bsp/near_field_probe.hpp"

#include <cmath>

namespace bsp {

// True: the flyabove heading arm takes its own 007F0280 probe with the post-flag-step
// flyabove+18h (the image probes at 009C6B3A, after 009C659F's flag step) and adds the
// increment below to A before the slew. Requires kNearFieldProbeBound (ON). False: A
// is the dead-band heading alone, as before. The host's speed arm takes its probe
// earlier (before the flag step) and keeps it; ON adds a second 007F0280 query per
// flyabove tick in this host (counters nf_calls/nf_hits double; the probe is
// otherwise a pure read).
// Movers when ON: dive-bomber flyabove headings whenever another aircraft is inside
// the flyabove box, so the flyabove path near other planes, the roll-in latch, the
// dive start point and the release.
inline constexpr bool kDiveFlyaboveAvoidIncrementCc13Bound = false;

// 009C6B3F-009C6CB6, per op at PC24 (plain float ops round the same way; the qword
// constants are float values widened). Returns 0 when 009C6B75's JBE is taken.
inline float dive_flyabove_avoid_increment_009c6b7b(const NearFieldProbeResult& r,
                                                    bool flyabove_18,
                                                    float lead_range) noexcept {
    namespace x8 = cc13x87;
    if (!(static_cast<double>(std::fabs(r.out_a[0])) > 0.05000000074505806)) return 0.0f;
    const float t = -r.out_a[0];                                       // 009C6B84 FCHS
    float x = static_cast<float>(
        x8::x87_mul24(x8::x87_mul24(t, r.out_b[1]), r.out_b[2]));    // 009C6B94, 009C6B9A
    if (x > 0.5f) {                                                    // 009C6BCE COMISS
        x = static_cast<float>(x8::x87_mul24(x8::x87_sub24(x, 0.5), 2.0));
    } else if (-0.5f > x) {                                            // 009C6BF1 COMISS
        x = static_cast<float>(x8::x87_mul24(x8::x87_add24(x, 0.5), 2.0));
    } else {
        x = 0.0f;                                                      // 009C6C0A
    }
    if (!flyabove_18) x = static_cast<float>(x8::x87_mul24(x, 0.5));   // 009C6C18
    const float r1 = dive_bomb_interpolate_clamped_00419010(150.0f, 0.2f, 350.0f, 1.0f,
                                                            lead_range);   // 009C6C50
    const float y = static_cast<float>(x8::x87_mul24(x, 0.4363323152065277));  // 009C6C5F
    const float p = static_cast<float>(x8::x87_mul24(r1, y));         // 009C6C73
    const float k = flyabove_18
        ? r1
        : dive_bomb_interpolate_clamped_00419010(500.0f, 0.0f, 800.0f, 1.0f,
                                                 lead_range);           // 009C6CA9
    return static_cast<float>(x8::x87_mul24(k, p));                   // 009C6CB2
}

}  // namespace bsp
