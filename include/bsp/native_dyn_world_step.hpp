#pragma once
#include "bsp/native_dyn_collision_pass.hpp"
#include "bsp/native_dyn_contact_groups.hpp"
#include "bsp/native_dyn_world_integration.hpp"
namespace bsp {
// All defaults invoke the existing native-storage implementations. Borrow the
// same scene/body allocator, profile/engine cells, group owner and CRT service.
struct NativeDynWorldStepCalls:NativeDynCollisionPassCalls {
    virtual void integrate_velocities_00c41550(void* world,float dt);
    virtual void collision_pass_00c57070(void* scene,const NativeDynCollisionPassContext&);
    virtual void create_groups_00c4b610(void* manager,const NativeDynContactGroupContext&);
    virtual void integrate_positions_00c5b1b0(void* world,float dt,const CameraAxesCrtAccess&);
    virtual void sleep_groups_00c4b550(void* manager,const NativeDynContactGroupContext&);
    virtual DynProfileScopeStorage* enter_profile_00c57020(DynProfileScopeStorage&,
        std::uint32_t id,const char* name,const DynProfileScopeContext&);
};
struct NativeDynWorldStepContext {
    const NativeDynCollisionPassContext& collision;
    const NativeDynContactGroupContext& groups;
    const CameraAxesCrtAccess& crt;
    NativeDynWorldStepCalls& calls;
};
// Complete normal raw-record bodies. Explicit source interfaces; native FH3,
// faults, allocation failures, concurrent mutation and ABI entry compatibility
// remain separate. Valid owners, callback tables and positive solver capacity
// for nonempty work are required. The world descriptor controls solver mode,
// maximum substeps, step duration, and reporting; native timing is preserved.
void reset_native_dyn_profile_tree_00c321b0(void* node); // ECX / RET
void flush_native_dyn_pending_bodies_00c4d980(void* world,const NativeDynWorldStepContext&); // ECX / RET
void run_native_dyn_world_substep_00c5bb30(void* world,float dt,const NativeDynWorldStepContext&); // stack world,dt / RET8
void simulate_native_dyn_world_00c5c540(void* world,float dt,const NativeDynWorldStepContext&); // ECX,stack dt / RET4
} // namespace bsp
