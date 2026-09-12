#pragma once

#include "bsp/ship_ai_lateral_record.hpp"

#include <array>
#include <cstdint>

namespace bsp {
// Complete 00416F30, original ECX=zone; stack point, out_point, out_edge,
// out_endpoint, push; no return value; RET 14h at 004171B2. Body ends 004171B4.
// Selects the nearest unshifted clamped edge projection, then adds the linear
// interpolation of the existing corner offset directions, multiplied by push.
// Edge order starts (last, first); strict distance comparison keeps the first
// on ties. out_edge is (start.x, start.z, end.x, end.z), and out_endpoint is 1
// for either clamp (including exact t=0/1), 0 for an interior projection.
// Empty input, or no squared distance below FLT_MAX, leaves ALL outputs alone.
// There is no zero-edge repair: t divides by outgoing_length even when zero;
// unordered t reaches interpolation, whose NaN distance cannot win selection.
// A NaN push still offsets a selected candidate, producing NaN coordinates.
//
// Valid nonnegative count and readable pointers/records are required. The list
// and records must stay stable, and outputs must not alias inputs or each other.
// Zero, one and two records are not rejected. The producer-backed native record
// is reused; no new overlay or generic segment-normal policy is introduced.
// This semantic C++ interface is not a native ABI replacement. x87 arithmetic
// and binary32 spill boundaries are retained; exception ABI and gameplay remain
// unverified. Descriptive names are hypotheses. See docs/AVOID_ZONE_OFFSET.md.
void avoid_zone_closest_offset_point_00416f30(
    const ShipAiPathLateralRecordList& zone, const std::array<float, 2>& point,
    std::array<float, 2>& out_point, std::array<float, 4>& out_edge,
    std::uint8_t& out_endpoint, float push) noexcept;
} // namespace bsp
