#pragma once

// 009ED6B0's station-keeping arm, 009EDA28..009EE57B, read whole by packet
// cc9_station_keeping (docs/STATION_KEEPING.md). Semantic C++ for MSVC Win32,
// not a binary replacement; names are hypotheses. The arm runs when blk+3A5h is
// set and blk+3A6h clear (009EDA34 / 009EDA41), i.e. after the follow update's
// station request 009DA3B0, and leaves through 009EE57B JMP 009EF206 with AL = 0
// ([ESP+37h] cleared at 009EDA2F). Every offset below is relative to blk =
// brain+8h. EDI = 2 and EBP = 1 on entry (009ED7EC, 009ED7F7).

#include "bsp/ship_ai_follow_land.hpp"
#include "bsp/ship_ai_states.hpp"

namespace bsp {

// What the arm reads besides the request block.
struct ShipAiStationKeepingInputs {
    float reference_speed_3c4{1.0f};  // blk+3C4h
    float retardation_508{1.0f};      // [[blk+3FCh]+538h]+508h, the class Retardation
    float goal_x_1dc{0.0f};           // blk+1DCh, the station point
    float goal_z_1e0{0.0f};           // blk+1E0h
    float hull_x_184{0.0f};           // blk+184h
    float hull_z_188{0.0f};           // blk+188h
    float unit_heading{0.0f};         // [[unit]+50h]() at 009EDAFC
    float unit_length_9c8{0.0f};      // unit+9C8h
    float unit_width_9cc{0.0f};       // unit+9CCh
    float turn_radius{0.0f};          // 0082E850(class) at 009EDDBF
};

// The fields the arm reads and writes.
struct ShipAiStationKeepingState {
    ShipAiThrottleDirection direction_35c{ShipAiThrottleDirection::Stopped};
    float timer_360{0.0f};
    float direction_value_374{0.0f};
    int direction_counter_384{0};
    bool aligned_388{false};
    bool reversing_389{false};
    bool close_38a{false};
    float throttle_39c{0.0f};         // blk+39Ch, the request's +10h slot
    float heading_target_324{0.0f};
    float distance_32c{0.0f};
};

// The arm's intermediate values, for diagnostics.
struct ShipAiStationKeepingTrace {
    float along{0.0f};        // [ESP+3Ch] before the radius shift
    float across{0.0f};       // [ESP+58h]
    float heading_error{0.0f};// [ESP+50h]
    float steer_limit{0.0f};  // [ESP+18h] after 009EDF25
    float braking{0.0f};      // [ESP+1Ch], 0.5 * ref * ref / Retardation
};

ShipAiStationKeepingTrace ship_ai_station_keeping_arm_009eda28(
    const ShipAiStationRequest& request, const ShipAiStationKeepingInputs& in,
    ShipAiStationKeepingState& state) noexcept;

} // namespace bsp
