#pragma once

#include "bsp/point_effect_owner.hpp"
#include "bsp/pose_refresh.hpp"

namespace bsp {
struct NativeModelGroupSelectionAccess;

// All references designate fields of the SAME existing unit. The descriptor,
// model and group-state cells are shared with its health/parts initializer.
struct NativeUnitGroupStateView {
    PoseRefreshView& pose;
    void* volatile& descriptor_354;
    volatile std::int32_t& group_state_358;
    void* volatile& model_360;
};

class NativeUnitGroupStateFields {
public:
    virtual ~NativeUnitGroupStateFields() = default;
    // Pure, nonthrowing live field access. No mutation, allocation, caching,
    // callback, default value, null fallback or floating-point environment change.
    virtual std::int32_t descriptor_effect_40(void* actual_descriptor) noexcept = 0;
    virtual void* world_announcement_manager_21d0(void* actual_world) noexcept = 0;
};

struct NativeUnitGroupStateContext {
    void* volatile& world_00e188a8;
    NativeUnitGroupStateFields& fields;
    PointAnnouncementConstruction& announcements;
    const NativeModelGroupSelectionAccess& model_groups;
};

// Complete00876EC0..00876F83[196]. Native ECX=unit, stack signed state,
// RET4. Source interface differs. Only an increase may announce; refresh pose
// before capturing coordinates, reload descriptor/world at their native points,
// and use canonical49C940 and710BB0. Capture model BEFORE state358 store,
// then dispatch that captured nonnull model even for unchanged/lower state.
void set_native_unit_group_state_00876ec0(NativeUnitGroupStateView,
    std::int32_t state, NativeUnitGroupStateContext&);
} // namespace bsp
