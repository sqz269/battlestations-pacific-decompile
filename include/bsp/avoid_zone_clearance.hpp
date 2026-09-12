#pragma once

#include "bsp/avoid_zone_owner.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
struct TrackedCriticalSection;

// Complete instruction schedules, including the existing native parameter
// solver's relative denominator test and unordered rejection. Names are
// hypotheses; docs/AVOID_ZONE_CLEARANCE.md records original ABI and evidence.
// ECX=a0, EDX=a1, stack b0,b1,out; AL success, RET0Ch. Output is untouched
// on failure; all endpoint inputs are captured before a successful output.
bool __fastcall native_segment_crossing_004f3730(const float* a0, const float* a1,
    const float* b0, const float* b1, float* output) noexcept;
// ECX=min, EDX=max, stack b0,b1; AL success, RET8. Retains x87 spills and
// native unordered branches rather than replacing them with generic SAT.
bool __fastcall native_box_meets_segment_0085c910(const float* minimum,
    const float* maximum, const float* b0, const float* b1) noexcept;

// Actual 20h-byte allocation produced by00415190/00417630. The links describe
// contiguous runs of selected polygon edges, not a spatial search tree.
struct AvoidZoneSelectedSegment {
    std::array<float, 2> start;              // +00h
    std::array<float, 2> end;                // +08h
    AvoidZoneSelectedSegment* next;         // +10h: next edge in the run
    AvoidZoneSelectedSegment* previous;     // +14h
    AvoidZoneSelectedSegment* next_run;     // +18h: next run/group continuation
    std::uint8_t closes_run;                // +1Ch: bypass cyclic next link
    std::uint8_t untouched_padding[3];
};
static_assert(sizeof(AvoidZoneSelectedSegment) == 0x20);
static_assert(offsetof(AvoidZoneSelectedSegment, next) == 0x10);
static_assert(offsetof(AvoidZoneSelectedSegment, previous) == 0x14);
static_assert(offsetof(AvoidZoneSelectedSegment, next_run) == 0x18);
static_assert(offsetof(AvoidZoneSelectedSegment, closes_run) == 0x1c);

// Borrow the actual native group's +4h zone-pointer array and +8h count.
// This is a view, not the 14h group object or AvoidZoneLayerGroup's vector.
struct AvoidZoneClearanceGroupView {
    const AvoidZoneNativeStorage* const* zones;
    std::uint32_t count;
};

AvoidZoneSelectedSegment& avoid_zone_segment_initialize_00415190(
    AvoidZoneSelectedSegment&, const std::array<float, 2>& start,
    const std::array<float, 2>& end) noexcept;

// Complete normal-flow edge selection and run links. Zone bounds reject only
// strict separation; edge bounds must overlap strictly, then native SAT must
// accept. Edges are last->first, first->second, etc. A bounds-overlapping zone
// must have a positive count/readable corners: native reads count-1 first.
// Allocations use allocate_record_00bf681b(20h), which must return or throw.
// Native constructor allocation failures leak previously unreturned nodes;
// no new allocation-failure recovery policy is inserted into these routines.
AvoidZoneSelectedSegment* avoid_zone_select_segments_00417630(
    const AvoidZoneNativeStorage&, const std::array<float, 4>& bounds,
    const AvoidZoneAllocationAccess&);
AvoidZoneSelectedSegment* avoid_zone_group_select_segments_00417a40(
    const AvoidZoneClearanceGroupView&, const std::array<float, 4>& bounds,
    const AvoidZoneAllocationAccess&);

// ECX originally addresses the list's head word. Full free loop recovered from
// bytes004158C8-D1 after Ghidra's false no-return _free. Sets head to null.
void avoid_zone_selected_segments_clear_004158a0(AvoidZoneSelectedSegment*& head,
    const AvoidZoneAllocationAccess&) noexcept;

// Complete traversal; returns the last segment to shorten output, hence the
// nearest crossing to start. Empty head leaves output untouched; nonempty head
// first copies end to output even when there is no hit. Input/output may alias.
AvoidZoneSelectedSegment* avoid_zone_selected_segments_hit_004158e0(
    AvoidZoneSelectedSegment* head, const std::array<float, 2>& start,
    const std::array<float, 2>& end, std::array<float, 2>& output) noexcept;
// Ordered minimum of actual00419AB0 distances; empty/all-NaN yields FLT_MAX.
float avoid_zone_selected_segments_distance_00419fc0(
    const AvoidZoneSelectedSegment* head, const std::array<float, 2>& query,
    const CameraAxesCrtAccess&);

class AvoidZoneClearanceAccess {
public:
    virtual ~AvoidZoneClearanceAccess() = default;
    // Required actual manager+4h value. Null is a native state; implementations
    // must not synthesize null to omit a live lock. Borrowed pointer survives
    // the full operation. Uses the canonical native 1Ch tracked-section layout.
    virtual TrackedCriticalSection* manager_critical_section_004218e0() = 0;
    // Called AFTER entering the above gate, corresponding to the second
    // singleton lookup then004120D0(zone.layer). Resolve from actual groups;
    // a truly null group or empty group is represented by count zero. The view
    // remains valid until the gate is released. No default/empty-list fallback.
    virtual AvoidZoneClearanceGroupView selected_group_004120d0(
        std::uint32_t layer) = 0;
};

// Complete00423190 normal flow through explicit actual manager/allocator/CRT.
// Native ECX=zone, stack record, RET4. Positive cache returns before all manager
// access; zero/negative/NaN recompute. Lock precedes selection and list creation;
// cache store precedes list destruction and unlock. C++ exceptions release the
// returned list and acquired gate; native SEH ABI/hardware faults are unmodelled.
// New source interface, not a drop-in replacement; no game-validation claim.
void avoid_zone_ensure_corner_clearance_00423190(const AvoidZoneNativeStorage&,
    ShipAiPathLateralRecord&, AvoidZoneClearanceAccess&,
    const AvoidZoneAllocationAccess&, const CameraAxesCrtAccess&);
} // namespace bsp
