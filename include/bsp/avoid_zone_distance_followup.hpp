#pragma once

#include "bsp/avoid_zone_geometry.hpp"
#include "bsp/avoid_zone_owner.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
// Complete 004F49F0..004F4AD0. Native ECX=a0, EDX=a1; stack b0,b1;
// ST0 distance, RET8. Reuses actual 004F3730 and all four 00419AB0 calls,
// including their ignored solver-failure/pointer-bit behavior. Each distance
// is rounded to binary32. Each pair minimum and the final group selection
// choose the second operand on equality or unordered comparison. A crossing
// returns positive zero before consulting CRT. Required CRT access is borrowed
// unchanged. The private double substitution in game_hosts_ship_ai.cpp remains
// separate; it is not called or exposed by this implementation.
float segment_pair_distance_004f49f0(const std::array<float, 2>& a0,
    const std::array<float, 2>& a1, const std::array<float, 2>& b0,
    const std::array<float, 2>& b1, const CameraAxesCrtAccess&);

// Complete caller body 004171F0..004172DE through the implementation above.
// Native ECX=zone; stack start,end pointers; ST0 result; RET8. Copies
// both query endpoints, visits (last,first) first, and keeps only strict
// binary32 improvements over FLT_MAX. Unordered candidates never win.
// This measures boundary edges; a segment inside the polygon can have a
// positive result. Native loads records[count-1] BEFORE testing the loop:
// count must be positive, and the pointer array and records must be readable
// and stable across every call. No empty-polygon repair is introduced.
float avoid_zone_segment_boundary_distance_004171f0(
    const ShipAiPathLateralRecordList& zone,
    const std::array<float, 2>& segment_start,
    const std::array<float, 2>& segment_end, const CameraAxesCrtAccess&);

// Output fields written by 004174A0 and its 00416F30 call. The observed native
// extent ends at +1Ah; trailing C++ alignment is not native-size evidence.
struct AvoidZoneExpandedBoundaryQuery {
    std::array<float, 2> point;       // +00h: closest boundary point
    std::array<float, 4> edge;        // +08h: start.x,start.z,end.x,end.z
    std::uint8_t contained;           // +18h: unexpanded 00416B50 result
    std::uint8_t within_bounds;       // +19h: expanded AABB admission
    std::uint8_t endpoint;            // +1Ah: 00416F30 clamp flag
};
static_assert(offsetof(AvoidZoneExpandedBoundaryQuery, point) == 0x00);
static_assert(offsetof(AvoidZoneExpandedBoundaryQuery, edge) == 0x08);
static_assert(offsetof(AvoidZoneExpandedBoundaryQuery, contained) == 0x18);
static_assert(offsetof(AvoidZoneExpandedBoundaryQuery, within_bounds) == 0x19);
static_assert(offsetof(AvoidZoneExpandedBoundaryQuery, endpoint) == 0x1a);

// Complete 004174A0..0041757B; native ECX=zone; stack point,margin,out;
// AL admission result, RET0Ch. Each expanded bound has its native x87 ->
// binary32 spill. Bounds are min-inclusive/max-exclusive; NaNs reject.
// A miss writes ONLY out.within_bounds=0. A hit writes it to 1, evaluates
// unexpanded containment, and calls 00416F30 with push=+0.0. Empty/no-candidate
// geometry therefore keeps the caller's point/edge/endpoint bits unchanged.
// A margin expands the box only: admission does not test boundary distance.
// polygon must be the matching avoid_zone_polygon_snapshot of native; count
// is nonnegative, records are readable, storage remains stable, and point/output
// must not alias native, polygon or each other.
// Reuses the semantic 00416B50 and instruction-backed 00416F30 APIs. These
// functions expose new C++ interfaces; original ABI, full floating-point
// exception equivalence and gameplay are unverified. Names are hypotheses.
bool avoid_zone_expanded_boundary_query_004174a0(
    const AvoidZoneNativeStorage& native, const AvoidZonePolygon& polygon,
    const std::array<float, 2>& point, float margin,
    AvoidZoneExpandedBoundaryQuery& output) noexcept;
} // namespace bsp
