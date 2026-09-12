#pragma once

#include "bsp/avoid_zone_boundary.hpp"
#include "bsp/avoid_zone_geometry.hpp"
#include "bsp/avoid_zone_owner.hpp"

#include <cstddef>

namespace bsp {
struct CameraAxesCrtAccess;

// C++ snapshot adapter, not another reconstruction of a native function.
// Copies native pointer order, stored bounds and every derived corner word.
// Does not rerun the approximate semantic polygon producer. In particular,
// native +20h clearance_scale is copied into the older projection's `spare`.
AvoidZonePolygon avoid_zone_polygon_snapshot(const AvoidZoneNativeStorage&);

// Connects all five 0041AEA0/0041B840 geometry calls to recovered functions.
// Construction copies valid nonnegative-count storage; sources can then be
// released. The borrowed CRT access must outlive this object. No scene load,
// allocation service, native object identity or executable registration is
// invented here. Identity values in views are snapshot-local indices.
//
// Empty polygons are retained in group order. As in the native boundary kernel,
// an empty polygon must not pass the expanded-bounds gate into its outside arm.
// A valid contained polygon must let 00416F30 select a candidate. No repair or
// exception behavior is claimed for invalid native inputs. Unknown host tokens
// and invalid public indices throw std::out_of_range at the C++ adapter boundary.
//
// Snapshot lists and views contain internal pointers, so copying/moving is
// disabled. Queries never resize their storage. See AVOID_ZONE_QUERY_BINDING.md
// for the distinction between native differential and integration fixtures.
class AvoidZoneBoundaryQueries final : public AvoidZoneBoundaryHost {
public:
    AvoidZoneBoundaryQueries(const std::vector<const AvoidZoneNativeStorage*>&,
        const CameraAxesCrtAccess&);
    AvoidZoneBoundaryQueries(const AvoidZoneBoundaryQueries&) = delete;
    AvoidZoneBoundaryQueries& operator=(const AvoidZoneBoundaryQueries&) = delete;
    AvoidZoneBoundaryQueries(AvoidZoneBoundaryQueries&&) = delete;
    AvoidZoneBoundaryQueries& operator=(AvoidZoneBoundaryQueries&&) = delete;

    const std::vector<AvoidZoneBoundaryZoneView>& zones() const noexcept;
    float distance_normal(std::size_t zone, const std::array<float, 2>& point,
        float slack, std::array<float, 2>& direction);
    std::array<float, 2> nearest_boundary_point(const std::array<float, 2>& point,
        float slack, float push);

    bool contains_point_00416b50(std::uint32_t zone,
        const std::array<float, 2>& point) override;
    void call_00416f30(std::uint32_t zone, const std::array<float, 2>& point,
        std::array<float, 2>& out_point, std::array<float, 4>& out_edge,
        std::uint8_t& out_endpoint, float push) override;
    float segment_distance_00419ab0(const std::array<float, 2>& start,
        const std::array<float, 2>& end, const std::array<float, 2>& point) override;
    std::array<float, 2> closest_segment_point_004f4b50(
        const std::array<float, 2>& start, const std::array<float, 2>& end,
        const std::array<float, 2>& point) override;
    float reciprocal_length_00419260(const std::array<float, 2>& delta) override;

private:
    struct Snapshot {
        AvoidZonePolygon polygon;
        std::vector<ShipAiPathLateralRecord> records;
        std::vector<ShipAiPathLateralRecord*> record_pointers;
        ShipAiPathLateralRecordList list{};
        std::vector<std::array<float, 2>> vertices;
    };
    const CameraAxesCrtAccess& crt_;
    std::vector<Snapshot> snapshots_;
    std::vector<AvoidZoneBoundaryZoneView> views_;
};
} // namespace bsp
