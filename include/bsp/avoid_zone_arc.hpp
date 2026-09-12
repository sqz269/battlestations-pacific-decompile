#pragma once
#include "bsp/avoid_zone_clearance.hpp"

namespace bsp {
// Complete native schedules; names are hypotheses. Explicit existing CRT sqrt
// binding, and host _CIatan2 boundary as in geometry_helpers.cpp. The latter is
// not an independent reconstruction of the original VS2005 CRT.
// Native ECX=center, EDX=start, stack radius/end/output; EAX count, RET0Ch.
// Returns0..2 points in segment-parameter order. Exact tangency and endpoint
// intersections are excluded. Radius is not repaired. Unwritten output words
// retain their bits. Original reads/spills/aliases and ignored solver result
// are preserved. If the solver declines to write, the perpendicular parameter
// is ambient native stack data; no deterministic result is promised for that
// path (including finite delta with squared-length overflow). See the doc.
int avoid_zone_circle_segment_004f3ba0(const std::array<float,2>& center,
    const std::array<float,2>& start, float radius, const std::array<float,2>& end,
    std::array<std::array<float,2>,2>& points, const CameraAxesCrtAccess&);

// Native ECX=&head, stack center/radius/start_bearing/&end_bearing; RET10h.
// Traverses actual20h selected runs: closes_run or null next selects next_run.
// Returns the last segment that wrote end_bearing. Empty/no-hit leaves it
// untouched. Both intersections within a segment compare to the bearing delta
// captured BEFORE that segment's candidate loop; no extra nearest-hit policy.
// Uses the native unbounded angle-wrap routines; nonterminating angle inputs
// retain that limitation. List/center/output/CRT storage must remain readable.
AvoidZoneSelectedSegment* avoid_zone_selected_segments_arc_00415970(
    AvoidZoneSelectedSegment* head, const std::array<float,2>& center,
    float radius, float start_bearing, float& end_bearing,
    const CameraAxesCrtAccess&);
} // namespace bsp
