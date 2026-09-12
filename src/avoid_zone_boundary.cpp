#include "bsp/avoid_zone_boundary.hpp"

#include <limits>

namespace bsp {
float avoid_zone_distance_normal_0041aea0(AvoidZoneBoundaryHost& host,
    const AvoidZoneBoundaryZoneView& zone, const std::array<float, 2>& point,
    float slack, std::array<float, 2>& out_direction) {
    float best = (std::numeric_limits<float>::max)(); // 00D7A248: 7F7FFFFF.
    out_direction = {0.0f, 0.0f};                  // 0041AED9 / 0041AEE1.
    const float minimum_x = zone.bounds[0] - slack;
    const float maximum_x = zone.bounds[2] + slack;
    const float minimum_z = zone.bounds[1] - slack;
    const float maximum_z = zone.bounds[3] + slack;
    if (!(minimum_x <= point[0] && point[0] < maximum_x &&
          minimum_z <= point[1] && point[1] < maximum_z)) {
        return best;                              // 0041AFF8-0041B004.
    }

    if (host.contains_point_00416b50(zone.native_identity, point)) { // 0041AF62.
        std::array<float, 2> projected{};
        std::array<float, 4> ignored_edge{};
        std::uint8_t ignored_endpoint{};
        host.call_00416f30(zone.native_identity, point, projected,
            ignored_edge, ignored_endpoint, 1.0f); // 0041AF87; RET 14h.
        const float dx = projected[0] - point[0];
        const float dz = projected[1] - point[1];
        out_direction = {dx, dz};
        const float reciprocal = host.reciprocal_length_00419260(out_direction); // 0041AFBD.
        out_direction = {reciprocal * dx, reciprocal * dz};
        return 0.0f;                              // 0041AFED.
    }

    const auto& vertices = *zone.vertices;
    auto previous = vertices.back();              // 0041B012: no empty guard.
    for (const auto& current : vertices) {
        const float distance = host.segment_distance_00419ab0(previous, current, point); // 0041B069.
        if (distance < best) {                    // 0041B07A / JBE: unordered skips.
            const auto nearest = host.closest_segment_point_004f4b50(previous, current, point); // 0041B096.
            const float dx = point[0] - nearest[0];
            const float dz = point[1] - nearest[1];
            out_direction = {dx, dz};
            const float reciprocal = host.reciprocal_length_00419260(out_direction); // 0041B0BC.
            out_direction = {reciprocal * dx, reciprocal * dz};
            best = distance;
        }
        previous = current;                       // 0041B0ED / 0041B0F3.
    }
    return best;                                  // 0041B107.
}

std::array<float, 2> avoid_zone_nearest_boundary_point_0041b840(
    AvoidZoneBoundaryHost& host, const std::vector<AvoidZoneBoundaryZoneView>& zones,
    const std::array<float, 2>& point, float slack, float push) {
    auto result = point;
    float best = (std::numeric_limits<float>::max)();
    for (const auto& zone : zones) {
        std::array<float, 2> direction{};
        const float distance = avoid_zone_distance_normal_0041aea0(
            host, zone, point, slack, direction);  // 0041B893; ST0 spilled to float.
        if (distance < best) {
            if (distance <= 0.0f) {                // 0041B8B1 / JNC 0041B941.
                return point;
            }
            const float extended_distance = distance + push; // 0041B8BA / FSTP.
            const float offset_x = extended_distance * direction[0];
            const float offset_z = extended_distance * direction[1];
            result = {point[0] - offset_x, point[1] - offset_z};
            best = distance;
        }
    }
    return result;
}
} // namespace bsp
