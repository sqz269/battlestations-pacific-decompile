#pragma once

#include "bsp/avoid_zone_geometry.hpp"
#include "bsp/avoid_zone_owner.hpp"

#include <array>
#include <cstdint>

namespace bsp {
// Stable C++ identity in a caller-owned table. A miss is {-1,-1}. Keep group
// and zone ordering fixed while path nodes retain these indices; map them to
// native zone storage in the same order. These are not native pointer values.
struct AvoidZoneManagerZone {
    std::int32_t group_index{-1};
    std::int32_t zone_index{-1};
    explicit operator bool() const noexcept {
        return group_index >= 0 && zone_index >= 0;
    }
};

// Complete wrapper00417580..00417600; native ECX zone, stack out,point,margin,
// byte test_containment, EAX out, RET10h. When requested, first test the stored
// half-open bounds (00414F50), then polygon containment (00416B50). An outside
// point is copied unchanged; otherwise00416F30 receives margin unchanged.
// Positive margin follows interpolated native corner offset_dir, without
// normalization or sign reversal. Producer0041A200 makes those directions
// outward for a valid simple ring; arbitrary records provide no such guarantee.
//
// polygon must be a matching avoid_zone_polygon_snapshot of native: bounds,
// corner order and positions must agree. Native storage/records remain alive.
// The projection requires00416F30 to select a candidate whenever called.
// Failure throws domain_error at the C++ boundary, where the native copies
// uninitialized stack data. NaN margins still propagate after a valid selection.
std::array<float, 2> avoid_zone_point_offset_00417580(
    const AvoidZoneNativeStorage& native, const AvoidZonePolygon& polygon,
    const std::array<float, 2>& point, float margin,
    bool test_containment = true);

// Complete00417E40..00417E58; ECX manager, stack point,layer, EAX zone or null,
// RET8. Select with existing00412120, then first containing zone004178F0.
// All manager entries require a nonempty table as native slot0 is unconditional;
// empty C++ tables throw invalid_argument instead of inventing a clear result.
AvoidZoneManagerZone avoid_zone_manager_containing_point_00417e40(
    const AvoidZoneTable&, const std::array<float, 2>& point,
    std::int32_t layer);

// Complete00417E90..00417EED; ECX manager, stack layer,toward,from,out_zone,
// out_edge, AL bool, RET14h. Select with existing004120D0, then004179D0.
// The crossing nearest toward wins. Both outputs remain untouched on a miss.
bool avoid_zone_manager_segment_hit_00417e90(const AvoidZoneTable&,
    std::int32_t layer, const std::array<float, 2>& toward,
    const std::array<float, 2>& from, AvoidZoneManagerZone& out_zone,
    std::int32_t& out_edge);

// Complete00417EF0..00417F5B; ECX manager, stack layer,toward,from,out_point,
// AL bool, RET10h. Same selection and crossing; only copies point on a hit.
bool avoid_zone_manager_segment_hit_point_00417ef0(const AvoidZoneTable&,
    std::int32_t layer, const std::array<float, 2>& toward,
    const std::array<float, 2>& from, std::array<float, 2>& out_point);

// Reuse avoid_zone_group_for_layer_004120d0 from avoid_zone_geometry.hpp.
// Its complete native body ends00412111 (RET4 starts0041210F), and returns
// exact key, else last lower key, else slot0. No second implementation here.
// Wrappers retain the documented semantic geometry dependencies. This is a
// new C++ interface, not original ABI replacement or gameplay validation.
} // namespace bsp
