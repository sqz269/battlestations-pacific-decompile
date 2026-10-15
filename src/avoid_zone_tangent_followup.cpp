#include "bsp/avoid_zone_tangent_followup.hpp"
#include "bsp/unit_rudder.hpp"

#include <limits>

namespace bsp {
namespace {

void add_weighted_direction(std::array<float, 2>& sum,
    const std::array<float, 2>& direction, float weight) noexcept {
    float nx = direction[0];
    float nz = direction[1];
    float sx = sum[0];
    float sz = sum[1];
    float weighted_x;
    float weighted_z;
    // 0041B614-0041B635: round both products to float before either addition.
    // 0041B618 is D8 C9 (ST0 *= ST1), not the ambiguous DC C9 form.
    __asm {
        fld weight
        fld nx
        fmul st(0), st(1)
        fstp weighted_x
        fmul nz
        fstp weighted_z
        fld sx
        fadd weighted_x
        fstp sx
        fld sz
        fadd weighted_z
        fstp sz
    }
    sum[0] = sx;
    sum[1] = sz;
}

} // namespace

bool avoid_zone_group_any_segment_hit_0041b4b0(const AvoidZoneLayerGroup& group,
    const std::array<float, 2>& toward, const std::array<float, 2>& from) noexcept {
    return avoid_zone_group_segment_hit_004179d0(group, toward, from).hit; // 0041B4CC
}

float avoid_zone_group_weighted_boundary_direction_0041b580(
    AvoidZoneBoundaryHost& host, const std::vector<AvoidZoneBoundaryZoneView>& zones,
    const std::array<float, 2>& point, float slack, float full_weight_distance,
    std::array<float, 2>& out) {
    float minimum = std::numeric_limits<float>::max(); // 00D7A248: FF FF 7F 7F
    out[0] = 0.0f;
    out[1] = 0.0f;
    for (const auto& zone : zones) {
        std::array<float, 2> direction;
        const float distance = avoid_zone_distance_normal_0041aea0(
            host, zone, point, slack, direction); // 0041B5D0; FSTP at 0041B5D5
        const float weight = clamped_interpolate_00419010(
            full_weight_distance, 1.0f, slack, 0.0f, distance); // 0041B5FF
        if (weight > 0.0f) { // 0041B612 JBE also skips unordered
            add_weighted_direction(out, direction, weight);
        }
        // 0041B648 JA skips only ordered distance > minimum. A NaN replaces
        // the minimum, and a subsequent ordered value replaces that NaN.
        if (!(distance > minimum)) minimum = distance;
    }
    return minimum;
}

Cc13AvoidZoneOptionalVector& avoid_zone_group_slack_tangent_0041b7e0(
    AvoidZoneBoundaryHost& host, Cc13AvoidCrtHost& crt,
    const std::vector<AvoidZoneBoundaryZoneView>& zones, Cc13AvoidZoneOptionalVector& out,
    const std::array<float, 2>& point, float slack) {
    out.valid = 0; // 0041B7EC; retain x, z and padding
    Cc13AvoidZoneOptionalVector candidate;
    for (const auto& zone : zones) {
        cc13_avoid_zone_tangent_0041b290(host, crt, zone, candidate, point, slack); // 0041B815
        cc13_avoid_merge_tangent_0041b700(out, candidate); // 0041B81E
    }
    return out;
}

} // namespace bsp
