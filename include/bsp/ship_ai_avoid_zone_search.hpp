#pragma once

#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/avoid_zone_geometry.hpp"
#include "bsp/ship_ai_avoidance_request.hpp"

namespace bsp {
// Searcher+18h/+1Ch. The list's layer is distinct from cached query key+14h.
// 009E4401/07 initializes these words to zero; 00419FA0 changes the list key.
// The caller owns this list and must clear it before its allocator disappears.
struct ShipAiAvoidZoneSegmentList {
    AvoidZoneSelectedSegment* head{};
    std::int32_t layer{};
};

class ShipAiAvoidZoneSearchAccess {
public:
    virtual ~ShipAiAvoidZoneSearchAccess() = default;
    // Every actual singleton lookup in004224C0 is preserved. The returned
    // semantic and native groups must describe the same stable ordered zones.
    // The semantic table must contain a group: native004120D0 reads slot zero.
    virtual const AvoidZoneTable& manager_004218e0() = 0;
    virtual AvoidZoneClearanceGroupView native_group(const AvoidZoneLayerGroup&) = 0;
    virtual const AvoidZoneAllocationAccess& allocation() const noexcept = 0;
};

// Complete normal bodies: ECX=list, stack key/box respectively, RET4.
void ship_ai_avoid_list_set_layer_00419fa0(ShipAiAvoidZoneSegmentList&,
    std::int32_t layer, const AvoidZoneAllocationAccess&);
void ship_ai_avoid_list_refill_004224c0(ShipAiAvoidZoneSegmentList&,
    const ShipAiAvoidZoneSearcher&, ShipAiAvoidZoneSearchAccess&);

// Complete009D7050..009D724E: ECX=searcher, stack query, RET4.
// Existing searcher fields plus the separate actual list owner form its20h
// record. Queries are disjoint stack records at009DA6E0 and009DC2E0.
// Preserves raw key equality, half-open cache containment, x87 spills and
// unordered max/floor branches. False/true is a new C++ diagnostic for reused/
// refreshed cache; original return-register contents are not an API contract.
// Initial box words must be supplied by the owner;009E4330 leaves them alone.
bool ship_ai_avoid_query_refresh_009d7050(ShipAiAvoidZoneSearcher&,
    ShipAiAvoidZoneSegmentList&, const ShipAiAvoidZoneQuery&, ShipAiAvoidZoneSearchAccess&);
} // namespace bsp
