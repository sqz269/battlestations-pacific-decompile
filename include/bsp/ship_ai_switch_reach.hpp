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
