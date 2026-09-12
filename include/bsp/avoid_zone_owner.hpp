#pragma once

#include "bsp/ship_ai_lateral_record.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
struct PoseRefreshView;

// Complete 24h native zone storage produced by 0041CCD0. Names are hypotheses.
// Capacity comparisons are unsigned. Count reuses the existing signed-index
// accessor view; construction/clipping treat its bits as unsigned counts.
struct AvoidZoneNativeStorage {
    ShipAiPathLateralRecordList corners;
    std::uint32_t capacity;
    std::uint32_t layer;
    void* associated_entity;
    float min_x, min_z, max_x, max_z;
};
static_assert(sizeof(AvoidZoneNativeStorage) == 0x24);
static_assert(offsetof(AvoidZoneNativeStorage, corners) == 0);
static_assert(offsetof(AvoidZoneNativeStorage, capacity) == 8);
static_assert(offsetof(AvoidZoneNativeStorage, layer) == 0x0c);
static_assert(offsetof(AvoidZoneNativeStorage, associated_entity) == 0x10);
static_assert(offsetof(AvoidZoneNativeStorage, min_x) == 0x14);
static_assert(offsetof(AvoidZoneNativeStorage, max_z) == 0x20);

// Required real allocation services. Both allocators implement the native
// malloc/new-handler/retry/throw contract, never a null/default substitute.
// All sizes are Win32 bytes. Deallocation must match the supplied allocator.
struct AvoidZoneAllocationAccess {
    void* context;
    void* (*allocate_record_00bf681b)(void*, std::uint32_t bytes);
    void* (*allocate_array_00bf55be)(void*, std::uint32_t bytes);
    void (*free_record_00bf65ac)(void*, void*) noexcept;
    void (*free_array_00bf6989)(void*, void*) noexcept;
};

using AvoidZoneScenePoint = std::array<float, 3>;
struct AvoidZoneScenePointSlots {
    const AvoidZoneScenePoint* const* begin; // native scene path +08h
    const AvoidZoneScenePoint* const* end;   // native scene path +0Ch
};

// Borrow actual scene fields and its +14h entity's pose. Implementations must
// expose live slots, including repairs made by invalid_parameter, and actual
// parent identities. No scene storage, identity matrix or dispatch is invented.
class AvoidZoneScenePathAccess {
public:
    virtual ~AvoidZoneScenePathAccess() = default;
    virtual const AvoidZoneScenePointSlots& point_slots() const = 0;
    virtual PoseRefreshView& scene_pose() = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    // Body read: follow entity+3Ch chain, starting at immediate parent, with
    // signed generations-1 remaining. Constructor calls three times with 1.
    virtual void* call_00923810(std::int32_t generations) = 0;
    // Actual entity-specific callee is unresolved: retain address/slot name.
    virtual std::uint8_t call_parent_vtable_5c(void* parent, std::uint32_t code) = 0;
};

// Complete 007AF800 point transformation via canonical 00414DB0/B62D10.
// Original ECX scene path; stack output, unsigned index; EAX output; RET8.
// Validation can return; re-read slots afterward. No index repair or w guard.
AvoidZoneScenePoint& avoid_zone_scene_world_point_007af800(
    AvoidZoneScenePathAccess&, AvoidZoneScenePoint& output, std::uint32_t index);

// Complete normal flow of 004167C0, ECX zone, RET: frees pointed-to records
// and zeros count; preserves pointer allocation, capacity, bounds and identity.
void avoid_zone_clear_records_004167c0(
    AvoidZoneNativeStorage&, const AvoidZoneAllocationAccess&) noexcept;

// Complete normal flow and temporary-array lifetime of 0041A540, ECX zone,
// stack minXYZ,maxXYZ, RET8. Rejects touching original bounds, clips against
// x-min,x-max,z-min,z-max planes expanded by100, rebuilds if each pass has>=3.
// Does not recompute bounds. Native empty-overlap input reads before a null
// temporary array; callers must supply readable native state (no new repair).
void avoid_zone_clip_world_bounds_0041a540(AvoidZoneNativeStorage&,
    const AvoidZoneScenePoint& minimum, const AvoidZoneScenePoint& maximum,
    const AvoidZoneAllocationAccess&);

// Complete body 0041CCD0..0041D0B1 through explicit real scene, allocator and
// CRT boundaries. Original ECX storage; stack scene path,layer; EAX zone; RET8.
// Requires fresh storage: overwrites pointer/count/capacity without releasing.
// Nonempty input seeds bounds; EMPTY input leaves caller's bound bits intact
// before invoking clip. Exact world-owner fields: minimum={711Ch,7120h,7130h},
// maximum={7128h,712Ch,7124h}; the native z endpoints cross the XYZ groups.
// Native SEH frame ABI is not reproduced; C++ exception unwinding releases the
// pointer array only (00412DC0), including the native retained-record leak on
// exceptional construction. This interface is not a drop-in binary replacement.
AvoidZoneNativeStorage& avoid_zone_construct_from_scene_path_0041ccd0(
    AvoidZoneNativeStorage&, AvoidZoneScenePathAccess&, std::uint32_t layer,
    const AvoidZoneScenePoint& minimum, const AvoidZoneScenePoint& maximum,
    const AvoidZoneAllocationAccess&, const CameraAxesCrtAccess&);

// Explicit C++ ownership cleanup, not a claimed native destructor entrypoint.
// Use after successful construction; releases records and pointer allocation.
void avoid_zone_release_owned_storage(
    AvoidZoneNativeStorage&, const AvoidZoneAllocationAccess&) noexcept;
} // namespace bsp
