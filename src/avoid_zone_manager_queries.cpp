#include "bsp/avoid_zone_manager_queries.hpp"

#include "bsp/avoid_zone_offset.hpp"
#include "bsp/geometry_helpers.hpp"

#include <stdexcept>

namespace bsp {
namespace {
void require_manager_group(const AvoidZoneTable& table) {
    if (table.groups.empty()) {
        throw std::invalid_argument("avoid-zone manager requires native slot 0");
    }
}
} // namespace

std::array<float, 2> avoid_zone_point_offset_00417580(
    const AvoidZoneNativeStorage& native, const AvoidZonePolygon& polygon,
    const std::array<float, 2>& point, float margin, bool test_containment) {
    if (test_containment) { //00417583/90.
        const std::array<float, 4> bounds{
            native.min_x, native.min_z, native.max_x, native.max_z};
        if (!contains_point_00414f50(bounds, point.data()) || //00417596.
            !avoid_zone_contains_point_00416b50(polygon, point)) { //004175A2.
            return point; //004175EB..00417600.
        }
    }
    std::array<float, 2> output{};
    std::array<float, 4> ignored_edge{};
    std::uint8_t endpoint = 0xff;
    avoid_zone_closest_offset_point_00416f30(native.corners, point, output,
        ignored_edge, endpoint, margin); //004175C5, margin unchanged.
    if (endpoint == 0xff) {
        // Native output locals have no defined value when no candidate wins.
        throw std::domain_error("avoid-zone offset has no closest candidate");
    }
    return output; //004175CA..004175E8.
}

AvoidZoneManagerZone avoid_zone_manager_containing_point_00417e40(
    const AvoidZoneTable& table, const std::array<float, 2>& point,
    std::int32_t layer) {
    require_manager_group(table);
    const auto group = avoid_zone_group_for_layer_00412120(table, layer); //00417E4A.
    const auto zone = avoid_zone_first_containing_004178f0(
        table.groups[static_cast<std::size_t>(group)], point); //00417E51.
    return zone < 0 ? AvoidZoneManagerZone{} : AvoidZoneManagerZone{group, zone};
}

bool avoid_zone_manager_segment_hit_00417e90(const AvoidZoneTable& table,
    std::int32_t layer, const std::array<float, 2>& toward,
    const std::array<float, 2>& from, AvoidZoneManagerZone& out_zone,
    std::int32_t& out_edge) {
    require_manager_group(table);
    const auto group = avoid_zone_group_for_layer_004120d0(table, layer); //00417ECE.
    const auto hit = avoid_zone_group_segment_hit_004179d0(
        table.groups[static_cast<std::size_t>(group)], toward, from); //00417ED5.
    if (hit.hit) {
        out_edge = hit.edge_index; //004179D0 writes through caller's edge slot.
        out_zone = {group, hit.zone_index}; //00417EE6; untouched on a miss.
    }
    return hit.hit; //AL survives TEST/MOV/ADD; RET14h at00417EEB.
}

bool avoid_zone_manager_segment_hit_point_00417ef0(const AvoidZoneTable& table,
    std::int32_t layer, const std::array<float, 2>& toward,
    const std::array<float, 2>& from, std::array<float, 2>& out_point) {
    require_manager_group(table);
    const auto group = avoid_zone_group_for_layer_004120d0(table, layer); //00417F2E.
    const auto hit = avoid_zone_group_segment_hit_004179d0(
        table.groups[static_cast<std::size_t>(group)], toward, from); //00417F35.
    if (hit.hit) out_point = hit.point; //00417F3E..00417F51.
    return hit.hit; //AL survives TEST/MOV/ADD; RET10h at00417F59.
}
} // namespace bsp
