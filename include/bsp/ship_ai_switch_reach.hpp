#pragma once

// Reach counters for the cc13 ship-AI precision and NaN switches (packet
// cc9-sweep2). In either setting each counter counts how often the switch's ON
// and OFF arms would give a different answer at that evaluation, so a pair
// that never moves can be told apart from one that never reaches its arm.
// "summary mission ship ai switch reach" in game_hosts_ship_ai.cpp prints them
// and resets them. Instrumentation only: nothing reads them back.

#include <cmath>
#include <cstdint>
#include <cstring>

namespace bsp {

struct ShipAiSwitchReach {
    // 009EDA28 (kShipAiStationKeepingNanArmsCc13Bound /
    // kShipAiStationKeepingPc24Cc13Bound).
    unsigned long long station_calls{0};
    unsigned long long station_nan_operands{0};  // NaN unit+9C8h or NaN across
    unsigned long long station_nan_differs{0};   // state differs, NaN switch flipped
    unsigned long long station_pc24_differs{0};  // state differs, PC24 switch flipped
    // 009E04E0 (kShipAiThrottleProfileNanArmsCc13Bound): NaN operands at the
    // switched compares.
    unsigned long long throttle_nan_operands{0};
    // 00415510 / 00415550 / 00415620 and 009EE62B
    // (kShipAiNavigationNanArmsCc13Bound): NaN operands at the switched compares.
    unsigned long long navigation_nan_operands{0};
    // 009E9EB0 / 009E4A60 (kShipAiFireStepPc24Cc13Bound): switched sites
    // evaluated, and those where the per-op and the single-rounding forms give
    // a different value (or, for a compare, a different outcome).
    unsigned long long fire_pc24_sites{0};
    unsigned long long fire_pc24_differs{0};
    // 009D3BB4-009D3BC8 torpedo home sector (kTorpedoHomeSectorScaleCc13Bound,
    // packet cc9-sweep2b): replans that compute a home sector, and those where
    // the stored +64h or the walk's start differ between the two arms.
    unsigned long long torpedo_home_calls{0};
    unsigned long long torpedo_home_differs{0};
    int torpedo_home_bound{-1};  // the build's setting, -1 until reached
    // Second batch (packet cc9-sweep2b). Each *_differs counts the evaluations
    // where the switch's two arms would give a different result.
    // 009E5E00 kEngagementTargetKindTestCc13Bound: non-zero +0B20h targets read,
    // and those that are not kind 1Ch.
    unsigned long long engage_target_reads{0};
    unsigned long long engage_target_differs{0};
    // 007DA406/007DA47C kWaterRateFactorsNanOrderCc13Bound: water-arm calls, and
    // those whose up value bits or throttle gate differ.
    unsigned long long water_rate_calls{0};
    unsigned long long water_rate_differs{0};
    int water_rate_bound{-1};  // the build's setting, -1 until reached
    // 0080E000 kUnitAiOrderResetFieldsCc13Bound: steps, and those whose record
    // differs after the step.
    unsigned long long order_step_calls{0};
    unsigned long long order_step_differs{0};
    // 00791020 / 0079A3B0 / 007911E0 movie trio: begins of a use-current key
    // holding a modifier; empty-track updates with a non-zero clock; finishes
    // where the stale-flag read changes the suppression.
    unsigned long long movie_begin_modifier_differs{0};
    unsigned long long movie_empty_clock_differs{0};
    unsigned long long movie_stale_flag_differs{0};
    // 0092D730 kSubmarineSeabedSpeedPc24Cc13Bound: scans, and those whose speed
    // bits differ between per-op float and the double chain.
    unsigned long long seabed_speed_calls{0};
    unsigned long long seabed_speed_differs{0};
    // 009D9E6C kShipAiPathNodeDirectionUnorderedCc13Bound (and, through the
    // walks' OFF arm, kShipAiPathSideWalkNanCc13Bound): undecided two-link
    // nodes evaluated, and those where the image's rule and the C++ `<` differ.
    unsigned long long path_dir_evals{0};
    unsigned long long path_dir_differs{0};
    // 00811D10 kUnitAiOrderCopyPc24Cc13Bound: order-record copies, and those
    // where the image's FLD/FSTP copy and the member-wise copy differ.
    unsigned long long order_copy_calls{0};
    unsigned long long order_copy_differs{0};
};

inline ShipAiSwitchReach& ship_ai_switch_reach() noexcept {
    static ShipAiSwitchReach reach{};
    return reach;
}

inline void ship_ai_reach_note_nan(unsigned long long& counter, float v) noexcept {
    if (std::isnan(v)) ++counter;
}

inline bool ship_ai_reach_same_bits(float a, float b) noexcept {
    std::uint32_t x;
    std::uint32_t y;
    std::memcpy(&x, &a, sizeof x);
    std::memcpy(&y, &b, sizeof y);
    return x == y;
}

inline void ship_ai_reach_note_fire(float per_op, float once) noexcept {
    ShipAiSwitchReach& r = ship_ai_switch_reach();
    ++r.fire_pc24_sites;
    if (!ship_ai_reach_same_bits(per_op, once)) ++r.fire_pc24_differs;
}

inline void ship_ai_reach_note_fire(bool per_op, bool once) noexcept {
    ShipAiSwitchReach& r = ship_ai_switch_reach();
    ++r.fire_pc24_sites;
    if (per_op != once) ++r.fire_pc24_differs;
}

} // namespace bsp
