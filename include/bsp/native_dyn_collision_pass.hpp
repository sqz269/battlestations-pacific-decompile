#pragma once
#include "bsp/dyn_profile_scopes.hpp"
#include "bsp/native_game_physics_lifetime.hpp"
namespace bsp {
// Concrete default services use the existing allocator, scheduler, profiler
// and manifold removal. Timestamp site IDs permit deterministic validation;
// the default is actual RDTSC. The same owners must outlive each invocation.
struct NativeDynCollisionPassCalls:NativeGamePhysicsLifetimeCalls {
    virtual void* allocate_00bf55be(std::uint32_t site,std::uint32_t bytes,const AvoidZoneDynHullMemory&);
    virtual void free_00bf6989(std::uint32_t site,void*,const AvoidZoneDynHullMemory&);
    virtual void run_batch_00c33140(void* manager,void* const* tasks,std::int32_t count,const AvoidZoneDynHullMemory&);
    virtual std::uint64_t read_timestamp(std::uint32_t site) noexcept;
};
struct NativeDynCollisionPassContext {
    const AvoidZoneDynHullMemory& memory; // same scene/body allocator
    const DynProfileScopeContext& profile; // actual profile cell and its allocator
    void* const* engine_slot_0109e9fc; // actual engine publication cell
    NativeDynCollisionPassCalls& calls;
};
// Complete normal raw-record bodies. New C++ interfaces, not native ABI thunks.
// Valid native body/manifold/scene/task/callback records, successful allocation,
// positive task capacity for nonempty work and no concurrent scene mutation
// are required. Original FH3 registration/unwind/OOM behavior remains separate.
void refresh_native_dyn_manifold_00c4b9b0(void* manifold); // EDX / RET
void update_native_dyn_manifolds_00c549d0(void* pool,const NativeDynCollisionPassContext&); // stack pool / RET4
void dispatch_native_dyn_contact_events_00c35480(void* scene); // stack scene / RET4
void run_native_dyn_collision_pass_00c57070(void* scene,const NativeDynCollisionPassContext&); // stack scene / RET4
} // namespace bsp
