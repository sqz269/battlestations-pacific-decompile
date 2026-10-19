#pragma once
// Packet cc13-seg67g: the no-band commitment threshold of 009E04E0 and the
// relative-bearing maximum it shares with the avoidance step
// (bsp/ship_ai_clearance_profile.hpp, docs/CC13_SEG67D.md notes G and H).
//
// 009E110F..009E112E, reached when a tick leaves stand-on or give-way torpedo
// tracks but no real band ([ESP+30h] == 0):
//   SUB EAX,ECX            ; give_way - stand_on
//   FILD [ESP+48h]
//   FMUL qword [00D7A348]  ; 0.25 (009E1122)
//   FCOMIP ratio, ...; JB  ; ratio < diff*0.25, or unordered -> astern band
// The host formed `(give_way - stand_on) * 4.0f`, a threshold 16 times the
// image's.
//
// 00D2185C, read at 009E0775 and 009E0C96, holds 0x40402038 (3.0019664764f).
// The host constant 3.0019646f is 0x40402030, 8 ulp low.
//
// True: ship_ai_commitment_band_009e10a9 takes the image's quarter factor as a
// double multiply of the integer difference, and the comparison keeps the JB
// arm (astern unless ratio >= diff*0.25, so NaN goes astern); both bearing
// tests use the image bits. False: the previous host forms. The product is
// exact at any precision control (a small integer times 0.25).
#include "bsp/ship_ai_clearance_profile.hpp"

namespace bsp {

inline constexpr bool kShipAiCommitmentBandQuarterBound = true;  // ON: JM08/USN21 moved by the ahead band (SHIP_AI 300)

// [00D7A348], a double, read by FMUL qword at 009E1122.
inline constexpr double kShipAiCommitmentBandQuarter = 0.25; // 00D7A348

inline constexpr float kShipAiContactBearingMaxImage = 3.0019664764404296875f; // 00D2185C
static_assert(kShipAiContactBearingMaxImage == 0x1.80407p+1f);                 // 0x40402038

inline constexpr float kShipAiContactBearingMaxActive =
    kShipAiCommitmentBandQuarterBound ? kShipAiContactBearingMaxImage : kShipAiContactBearingMax;

} // namespace bsp
