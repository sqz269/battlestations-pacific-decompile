#pragma once

// The avoid-zone free-bearing query 009DC2E0 and the outline walk 00416270 it
// uses. docs/SHIP_AI_OPEN_ITEMS.md section 14 has the read; every name is a
// hypothesis, not a recovered symbol. New C++ interfaces over the host's
// selected-segment list, not drop-in replacements of the native ABI.

#include <array>
#include <cstdint>

#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/ship_ai_avoidance_request.hpp"
#include "bsp/ship_ai_sector_scan.hpp"

namespace bsp {

struct CameraAxesCrtAccess;

// 00416270, `__stdcall(seg, &origin, radius, forward, monotone, &out)`, RET 18h,
// body 00416270-004166D5. From the hit segment, walk the outline forward (+10h
// next, `forward`) or backward (+14h previous) until the walk leaves the circle
// of `radius` about `origin`:
//   * a segment whose normal n = (e.z - s.z, -(e.x - s.x)) faces away from the
//     origin ((origin - s) . n < 0) answers its near end (s forward, e backward);
//   * a far end (e forward, s backward) outside the circle answers 004F3BA0's
//     crossing: the single one, or of two the one nearer that far end, or the
//     far end itself when the solver finds none;
//   * without `monotone`, a far end nearer the origin than the previous one
//     answers the near end;
//   * the end of the chain answers the far end.
// Always true for a non-null segment; false (out untouched) for null.
bool avoid_zone_outline_walk_00416270(const AvoidZoneSelectedSegment* segment,
                                      const std::array<float, 2>& origin, float radius,
                                      bool forward, bool monotone, std::array<float, 2>& out,
                                      const CameraAxesCrtAccess& crt);

// What 009DC2E0 reaches outside itself.
class ShipAiFreeBearingHost {
public:
    virtual ~ShipAiFreeBearingHost() = default;
    // 009DC3B2, 009D7050(ECX = searcher)(&query): refresh the searcher's cache
    // and selected list for the box about the origin.
    virtual void refresh_query_009d7050(const ShipAiAvoidZoneQuery& query) = 0;
    // searcher+18h, the selected list's head after the refresh.
    virtual AvoidZoneSelectedSegment* selected_head() = 0;
    virtual const CameraAxesCrtAccess& crt() = 0;
};

// What one call did, for the census.
struct ShipAiFreeBearingOutcome {
    bool refreshed{false};
    bool list_empty{false};
    bool ahead_hit{false};      // 004158E0 found a crossing along the query direction
    int corner_side{0};         // 1: the forward-walk corner won, 2: the backward one
    bool lateral_run{false};    // the clearance pass ran
    bool lateral_turn{false};   // it rotated the bearing
    bool short_leg{false};      // the free leg was under 10 m: answered false
};

// 009DC2E0, `char __thiscall(searcher)(query*)`, RET 4, body 009DC2E0-009DCEA2,
// read whole (docs/SHIP_AI_OPEN_ITEMS.md section 14). `enabled` is the
// searcher's +0h byte and `layer_key` the query's +20h word. Writes
// `query.bearing` (+1Ch) and answers whether the caller should steer by it.
bool ship_ai_free_bearing_009dc2e0(bool enabled, ShipAiSectorFreeBearingQuery& query,
                                   std::int32_t layer_key, ShipAiFreeBearingHost& host,
                                   ShipAiFreeBearingOutcome* outcome = nullptr);

}  // namespace bsp
