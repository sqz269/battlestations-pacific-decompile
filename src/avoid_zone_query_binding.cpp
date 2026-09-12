#include "bsp/avoid_zone_query_binding.hpp"

#include "bsp/avoid_zone_offset.hpp"
#include "bsp/avoid_zone_segment_math.hpp"
#include "bsp/native_vector2_math.hpp"

#include <cstring>

namespace bsp {
AvoidZonePolygon avoid_zone_polygon_snapshot(const AvoidZoneNativeStorage& source) {
    AvoidZonePolygon result;
    std::memcpy(&result.layer, &source.layer, sizeof(result.layer));
    result.bounds_min = {source.min_x, source.min_z};
    result.bounds_max = {source.max_x, source.max_z};
    result.corners.reserve(static_cast<std::size_t>(source.corners.count));
    for (std::int32_t i = 0; i < source.corners.count; ++i) {
        const auto& record = *source.corners.records[i];
        AvoidZoneCorner corner;
        corner.position = {record.x, record.z};
        corner.edge_direction = {record.outgoing_x, record.outgoing_z};
        corner.normal = {record.offset_dir_x, record.offset_dir_z};
        corner.edge_length = record.outgoing_length;
        corner.turn_angle = record.signed_turn;
        corner.spare = record.clearance_scale;
        result.corners.push_back(corner);
    }
    return result;
}

AvoidZoneBoundaryQueries::AvoidZoneBoundaryQueries(
    const std::vector<const AvoidZoneNativeStorage*>& sources,
    const CameraAxesCrtAccess& crt) : crt_(crt) {
    // Finalize all vector sizes before publishing any pointer into a snapshot.
    snapshots_.resize(sources.size());
    views_.reserve(sources.size());
    for (std::size_t i = 0; i < sources.size(); ++i) {
        const auto& source = *sources[i];
        auto& snapshot = snapshots_[i];
        snapshot.polygon = avoid_zone_polygon_snapshot(source);
        const auto count = static_cast<std::size_t>(source.corners.count);
        snapshot.records.reserve(count);
        snapshot.record_pointers.reserve(count);
        snapshot.vertices.reserve(count);
        for (std::size_t j = 0; j < count; ++j) {
            const auto& record = *source.corners.records[j];
            snapshot.records.push_back(record);
            snapshot.vertices.push_back({record.x, record.z});
        }
        for (auto& record : snapshot.records) {
            snapshot.record_pointers.push_back(&record);
        }
        snapshot.list = {snapshot.record_pointers.data(), source.corners.count};
        views_.push_back({static_cast<std::uint32_t>(i),
            {source.min_x, source.min_z, source.max_x, source.max_z},
            &snapshot.vertices});
    }
}

const std::vector<AvoidZoneBoundaryZoneView>& AvoidZoneBoundaryQueries::zones() const noexcept {
    return views_;
}

float AvoidZoneBoundaryQueries::distance_normal(std::size_t zone,
    const std::array<float, 2>& point, float slack, std::array<float, 2>& direction) {
    return avoid_zone_distance_normal_0041aea0(*this, views_.at(zone), point, slack, direction);
}

std::array<float, 2> AvoidZoneBoundaryQueries::nearest_boundary_point(
    const std::array<float, 2>& point, float slack, float push) {
    return avoid_zone_nearest_boundary_point_0041b840(*this, views_, point, slack, push);
}

bool AvoidZoneBoundaryQueries::contains_point_00416b50(std::uint32_t zone,
    const std::array<float, 2>& point) {
    return avoid_zone_contains_point_00416b50(snapshots_.at(zone).polygon, point);
}

void AvoidZoneBoundaryQueries::call_00416f30(std::uint32_t zone,
    const std::array<float, 2>& point, std::array<float, 2>& out_point,
    std::array<float, 4>& out_edge, std::uint8_t& out_endpoint, float push) {
    avoid_zone_closest_offset_point_00416f30(snapshots_.at(zone).list,
        point, out_point, out_edge, out_endpoint, push);
}

float AvoidZoneBoundaryQueries::segment_distance_00419ab0(
    const std::array<float, 2>& start, const std::array<float, 2>& end,
    const std::array<float, 2>& point) {
    return avoid_zone_segment_distance_00419ab0(start, end, point, crt_);
}

std::array<float, 2> AvoidZoneBoundaryQueries::closest_segment_point_004f4b50(
    const std::array<float, 2>& start, const std::array<float, 2>& end,
    const std::array<float, 2>& point) {
    const std::array<float, 4> endpoints{start[0], start[1], end[0], end[1]};
    std::array<float, 2> output;
    avoid_zone_segment_closest_point_004f4b50(endpoints, output, point, crt_);
    return output;
}

float AvoidZoneBoundaryQueries::reciprocal_length_00419260(
    const std::array<float, 2>& delta) {
    return native_vector2_reciprocal_length_00419260(delta.data(), &crt_);
}
} // namespace bsp
