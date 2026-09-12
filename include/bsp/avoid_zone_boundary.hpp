#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace bsp {
// Complete control-flow projections of 0041AEA0 and 0041B840 through required
// callee bindings. Semantic C++ interfaces, not native object layouts or ABI
// replacements. Descriptive names are hypotheses. Evidence and limits:
// docs/AVOID_ZONE_BOUNDARY.md and reports/avoid_zone_boundary.json.

// A borrowed snapshot of one polygon. These are values supplied by an adapter,
// not an overlay on game memory: native zone +00h is a pointer array, +04h its
// count, and +14h..+20h hold the four bounds. Vertices keep native pointer order.
// vertices must be valid and nonempty when the outside arm is entered;
// 0041AEA0 itself dereferences the last vertex before checking for an empty ring.
// Host calls must not mutate the vertex vector or the caller's zone collection:
// native loops reload their counts, while these snapshot loops assume stability.
struct AvoidZoneBoundaryZoneView {
    std::uint32_t native_identity{0};
    std::array<float, 4> bounds{}; // minimum X/Z, maximum X/Z; 0041CCD0 producer.
    const std::vector<std::array<float, 2>>* vertices{nullptr};
};

struct AvoidZoneBoundaryHost {
    virtual ~AvoidZoneBoundaryHost() = default;

    // 0041AF62: body 00416B50-00416CC4 checks the unexpanded half-open
    // bounds, then odd horizontal-ray crossings. AL is the boolean result.
    virtual bool contains_point_00416b50(std::uint32_t zone,
        const std::array<float, 2>& point) = 0;

    // 0041AF87: body read whole. Selects the closest clamped edge projection,
    // then adds the interpolated vertex +10h/+14h offsets times push. The
    // edge and endpoint flag are real native outputs even though this caller
    // does not consume them. Detailed producer policy stays with its packet.
    virtual void call_00416f30(std::uint32_t zone,
        const std::array<float, 2>& point, std::array<float, 2>& out_point,
        std::array<float, 4>& out_edge, std::uint8_t& out_endpoint, float push) = 0;

    // 0041B069: ECX=start, EDX=end, stack point, RET 4, ST0 distance.
    // Preserve the native segment-parameter and short-length behavior; a
    // generic standard-library segment distance is not a verified binding.
    virtual float segment_distance_00419ab0(const std::array<float, 2>& start,
        const std::array<float, 2>& end, const std::array<float, 2>& point) = 0;

    // 0041B096: ECX=(start,end), stack out/point, RET 8, EAX=out.
    // The body clamps the 004F3630 parameter to [0,1] and interpolates.
    virtual std::array<float, 2> closest_segment_point_004f4b50(
        const std::array<float, 2>& start, const std::array<float, 2>& end,
        const std::array<float, 2>& point) = 0;

    // 0041AFBD and 0041B0BC. Existing implementation available in
    // bsp/native_vector2_math.hpp with required CameraAxesCrtAccess binding.
    // Returns zero for exactly zero length, reciprocal otherwise.
    virtual float reciprocal_length_00419260(const std::array<float, 2>& delta) = 0;
};

// Original: __thiscall(zone)(point, slack, out_direction), ST0, RET 0Ch.
// Clears direction first. Rejects outside [min-slack,max+slack), returning
// FLT_MAX. Inside returns 0 and normalize(call_00416f30(...,1)-point).
// Outside returns the smallest edge distance and normalize(point-closest).
// Strict comparisons keep the first edge on a tie. Point/direction must not
// alias, and host calls must preserve the borrowed polygon snapshot. The inside
// host binding must populate its outputs for a valid polygon.
float avoid_zone_distance_normal_0041aea0(AvoidZoneBoundaryHost& host,
    const AvoidZoneBoundaryZoneView& zone, const std::array<float, 2>& point,
    float slack, std::array<float, 2>& out_direction);

// Original: __thiscall(group)(out,point,slack,push), EAX=out, RET 10h.
// Visits zones in order. For a new positive minimum: point-normal*(d+push).
// A nonpositive new minimum returns point immediately. No candidate and empty
// groups also return point. Positive push moves an outside point past the
// selected boundary toward the polygon interior; it is not an outward push.
// zones and every borrowed vertex vector remain stable across all host calls.
std::array<float, 2> avoid_zone_nearest_boundary_point_0041b840(
    AvoidZoneBoundaryHost& host, const std::vector<AvoidZoneBoundaryZoneView>& zones,
    const std::array<float, 2>& point, float slack, float push);
} // namespace bsp
