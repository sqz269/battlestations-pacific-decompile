// The unit wake trail: the ring a follower's station is measured along.
//
// Packet cc8_ship_follow. Evidence: docs/SHIP_UNIT_GROUP_FOLLOW.md sections 5b
// and 5c, which read 00810190 BSP_UnitWake_AppendSample (303 instructions) and
// 00810630 BSP_UnitWake_SampleAtDistance (198) whole from the listing. Every
// name here is a hypothesis, not a recovered symbol.
//
// The object is `entity+0BD0h`. 00811150 adds that offset and tail-calls
// 00810630, and 00811180 reads the same buffer with the entity as `this`, which
// is why docs/SHIP_AI_FORMATION.md quotes the ring at entity+0BD8h and the head
// index at entity+0F98h: 0BD0h + 8h and 0BD0h + 3C8h.
//
//   wake+008h   40 samples of 18h bytes          ShipAiWakeSample, ship_ai_formation.hpp
//   wake+3C8h   the head index                   head
//   wake+3CCh   a byte flag                      leg_grew
//   wake+3D0h   a residual offset, three floats  residual
//
// The trail is NOT one sample per tick. 00810190 returns at once unless the ship
// is more than 4 m from the head sample, rewrites the head sample in place, and
// only promotes it to a new slot once that leg exceeds 50 m. Forty slots is
// therefore about 2 km of trail.

#ifndef BSP_SHIP_AI_WAKE_TRAIL_HPP
#define BSP_SHIP_AI_WAKE_TRAIL_HPP

#include <cstdint>

#include "bsp/ship_ai_formation.hpp"

namespace bsp {

// 00CE6454, a float, compared against the SQUARED horizontal distance from the
// head sample at 0081022B: the ship must move 4 m before anything happens.
inline constexpr float kShipAiWakeAppendMinDistSq = 16.0f;

// 00CE3938, a DOUBLE, at 00810585. The head advances to a new slot only once
// the current leg is longer than this.
inline constexpr float kShipAiWakeAdvanceLeg = 50.0f;

// 00D09438, a DOUBLE, at 008104EF. Closer than this to the sample two back and
// the last leg is merged instead, moving the head backwards.
inline constexpr float kShipAiWakeMergeDist = 55.0f;

// 00D7A348, a DOUBLE, at 0081028D: the residual decays by a quarter of the
// distance travelled since the head sample.
inline constexpr float kShipAiWakeResidualDecay = 0.25f;

// 00D7A24C, a float, at 00810798: the interpolation parameter when the walk has
// consumed the whole ring without reaching `along`.
inline constexpr float kShipAiWakeWalkExhausted = 1.0f;

// The ring, `entity+0BD0h`. The three counters at the end are this host's
// bookkeeping and have no address in the image.
struct ShipAiWakeTrail {
    ShipAiWakeSample samples[kShipAiWakeSampleCount]{};  // wake+008h
    std::int32_t head{0};                                // wake+3C8h
    // wake+3CCh. Set at 0081048C on the arm the listing reaches when the leg to
    // the previous sample has SHRUNK - `FCOMIP` at 00810480 compares that
    // sample's stored length against the new one and `JBE` leaves the arm for
    // "stored <= new". While it is set the head cannot advance (00810599) and
    // the merge test runs instead, which is how a ship that curves back on its
    // own track collapses the last leg rather than laying down another.
    bool leg_shrank{false};
    float residual[3]{0.0f, 0.0f, 0.0f};                 // wake+3D0h

    std::uint32_t appends{0};   // calls that got past the 4 m gate
    std::uint32_t advances{0};  // 008105BE, the head moved to a new slot
    std::uint32_t merges{0};    // 0081056B, the head moved back
    // How many slots the ship has actually laid down, capped at the ring. Host
    // bookkeeping with no address in the image: the image's ring starts zeroed
    // and its first leg therefore runs from the origin to wherever the ship is,
    // which is a real several-kilometre segment sitting in the ring until the
    // head laps it. Only the reported trail length uses this; no branch does.
    std::uint32_t written{0};
};

// 00810190 BSP_UnitWake_AppendSample,
// __thiscall(wake)(const float world_pos[3], float heading, float yaw_rate),
// RET 0Ch. `heading` is the pose heading the motion tick has just written to
// unit+1050h (00826C56, atan2 over world row 2) and `yaw_rate` the tick's own,
// which is the only thing 00826C75 feeds (docs/SHIP_AI_RUDDER_HOP.md).
void ship_ai_wake_append_00810190(ShipAiWakeTrail& trail,
                                  const float world_pos[3],
                                  float heading,
                                  float yaw_rate) noexcept;

// 00810630 BSP_UnitWake_SampleAtDistance,
// __thiscall(wake)(float along, float* pos, float* dir, float* yaw), RET 10h.
// `along` metres back from the head sample; a negative or zero `along`
// extrapolates forward from the head sample's own heading and leaves the yaw
// rate UNWRITTEN, which the answer reports through `yaw_rate_written` rather
// than reproducing the image's stale stack slot (00810658 reuses that argument's
// own slot as an angle scratch).
ShipAiWakePoint ship_ai_wake_sample_at_distance_00810630(const ShipAiWakeTrail& trail,
                                                         float along) noexcept;

// How much trail exists behind the head, in metres: the sum of the legs over the
// slots this ship has actually laid down. Not a native routine - the host reports
// it so a run can show the trail filling, and it stops at `written` so the
// ring's opening leg from the origin is not counted as trail. 00810630's own walk
// has no such stop; it would traverse that leg like any other.
float ship_ai_wake_trail_length(const ShipAiWakeTrail& trail) noexcept;

}  // namespace bsp

#endif  // BSP_SHIP_AI_WAKE_TRAIL_HPP
