#pragma once

#include "bsp/avoid_zone_geometry.hpp"
#include "bsp/cc13_avoid1.hpp"

namespace bsp {

// Complete semantic projections; names are hypotheses, not recovered symbols.
// Existing geometry/boundary/tangent implementations supply the callee contracts.
// These interfaces are not native object overlays or binary ABI replacements.
// Evidence: docs/OCT15_AVOID_TANGENT.md; reports/oct15_avoid_tangent.json.

// 0041B4B0-0041B4D6: ECX group, stack toward/from, AL boolean, RET 8.
// Calls 004179D0 and discards its point, zone and edge outputs.
bool avoid_zone_group_any_segment_hit_0041b4b0(const AvoidZoneLayerGroup& group,
    const std::array<float, 2>& toward, const std::array<float, 2>& from) noexcept;

// 0041B580-0041B677: ECX group; stack point/slack/full_weight_distance/out;
// ST0 minimum distance, RET 10h. Clears out, then sums each zone's 0041AEA0
// direction times 00419010(full_weight_distance,1,slack,0,distance), but only
// for ordered positive weights. Does not normalize the sum. Empty groups return
// FLT_MAX and zero out. Minimum replacement includes equality AND unordered
// comparisons. Equal interpolation endpoints retain 00419010's constant-one arm.
// Borrowed snapshots must remain stable during host calls; point and out must
// not alias. Bounds, polygon validity and required bindings follow
// avoid_zone_distance_normal_0041aea0. x87 product/add float stores are retained.
float avoid_zone_group_weighted_boundary_direction_0041b580(
    AvoidZoneBoundaryHost& host, const std::vector<AvoidZoneBoundaryZoneView>& zones,
    const std::array<float, 2>& point, float slack, float full_weight_distance,
    std::array<float, 2>& out);

// 0041B7E0-0041B83E: ECX group; stack out/point/slack; EAX out, RET 0Ch.
// Clears only out.valid, then folds 0041B290 results through 0041B700 in zone
// order. No valid contribution leaves x/z and padding untouched; padding is
// never written. Out and point must not alias; snapshots stay stable across
// host calls. Existing per-zone tangent host/CRT requirements still apply.
Cc13AvoidZoneOptionalVector& avoid_zone_group_slack_tangent_0041b7e0(
    AvoidZoneBoundaryHost& host, Cc13AvoidCrtHost& crt,
    const std::vector<AvoidZoneBoundaryZoneView>& zones, Cc13AvoidZoneOptionalVector& out,
    const std::array<float, 2>& point, float slack);

} // namespace bsp
