#pragma once
#include "bsp/native_game_dynamics.hpp"
#include "bsp/native_dyn_sap_lifetime.hpp"
#include <cstdint>

namespace bsp {
struct NativeGamePhysicsLifetimeCalls : NativeDynSapLifetimeCalls {
    virtual ~NativeGamePhysicsLifetimeCalls()=default;
    virtual void* physics_allocate_00bf55be(std::uint32_t,const AvoidZoneDynHullMemory&);
    virtual void physics_free_00bf65ac(void*,const AvoidZoneDynHullMemory&);
    virtual void physics_free_00bf6989(void*,const AvoidZoneDynHullMemory&);
    virtual void physics_free_00bf9dc8(void*,const AvoidZoneDynHullMemory&);
    // Profile SBO text is produced by NativeLegacySboString's concrete heap,
    // independently of the engine-context allocation callbacks.
    virtual void physics_free_profile_text_00bf65ac(void*);
    virtual void physics_delete_section(void*);
    // Attachments still use their actual complete callable tables. Numeric
    // original-image vtables are not callable source tables.
    virtual void physics_virtual_scalar(void*,std::uint32_t byte_slot,std::uint32_t flags);
    // Concrete SAPBroadPhaseManager2 produced by the existing scene constructor.
    virtual void physics_broadphase_remove(void*,void*,const AvoidZoneDynHullMemory&,NativeDynSapLifetimeProgress&);
    virtual void physics_delete_broadphase_004043d0(void*,std::uint32_t,const AvoidZoneDynHullMemory&,NativeDynSapLifetimeProgress&);
    virtual void physics_destroy_tasks_00c40ff0(void*,const AvoidZoneDynHullMemory&);
};
struct NativeGamePhysicsLifetimeContext {
    // Borrow the SAME construction contexts, publication cell and allocators.
    // The world/scene and engine allocations may use distinct memory services.
    const NativeGameDynamicsContext& dynamics;
    NativeGamePhysicsLifetimeCalls& calls;
};
struct NativeGamePhysicsLifetimeProgress {
    std::uint32_t native_site{};
    void* header{};
    void* world{};
    void* cursor{};
    void* captured{};
    NativeDynSapLifetimeProgress sap;
};
struct NativeGamePhysicsLifetimeOperation final:NativeGamePhysicsLifetimeProgress {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    NativeGamePhysicsLifetimeContext* context{};
    NativeGamePhysicsLifetimeOperation()=default;
    ~NativeGamePhysicsLifetimeOperation();
    NativeGamePhysicsLifetimeOperation(const NativeGamePhysicsLifetimeOperation&)=delete;
    NativeGamePhysicsLifetimeOperation& operator=(const NativeGamePhysicsLifetimeOperation&)=delete;
    // Only after the caller resolves partial ownership. No retry or rollback.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Complete172B C37D30: native ESI body,EDX manifold,RET. Swap found pointer
// with last and decrement count; retain the native unsigned-underflow branch.
// Valid removal requires a present manifold and a nonempty pointer vector.
void remove_native_dyn_manifold_reference_00c37d30(void*,void*,const AvoidZoneDynHullMemory&,NativeGamePhysicsLifetimeCalls&,NativeGamePhysicsLifetimeProgress&);
// Complete130B/147B: native stack body,RET4. Detach both manifold endpoints,
// unlink active slots and return them to their actual pool free chains.
void clear_native_dyn_body_manifolds_00c43aa0(void*,const AvoidZoneDynHullMemory&,NativeGamePhysicsLifetimeCalls&,NativeGamePhysicsLifetimeProgress&);
void destroy_native_dyn_body_storage_00c43c00(void*,const AvoidZoneDynHullMemory&,NativeGamePhysicsLifetimeCalls&,NativeGamePhysicsLifetimeProgress&);
// Complete444B: native EBX world,RET. Clear two active body lists, release
// attached chains via actual virtual slot4, then recycle body/motion slots.
void clear_native_dyn_world_bodies_00c4daa0(void*,const AvoidZoneDynHullMemory&,NativeGamePhysicsLifetimeCalls&,NativeGamePhysicsLifetimeProgress&);
// Complete64B/63B. Native EAX manifold pool / ESI bucket owner; both RET.
void destroy_native_dyn_manifold_pool_00406f20(void*,const AvoidZoneDynHullMemory&,NativeGamePhysicsLifetimeCalls&,NativeGamePhysicsLifetimeProgress&);
void destroy_native_dyn_bucket_storage_00407210(void*,const AvoidZoneDynHullMemory&,NativeGamePhysicsLifetimeCalls&,NativeGamePhysicsLifetimeProgress&);
// Complete195B/369B. Native stack scene/world,RET4. Borrow their actual
// construction allocator, concrete SAP lifetime and callable attachment tables.
void destroy_native_dyn_scene_00c32250(void*,const AvoidZoneDynHullMemory&,NativeGamePhysicsLifetimeCalls&,NativeGamePhysicsLifetimeProgress&);
void destroy_native_dyn_world_00c4dc60(void*,const AvoidZoneDynHullMemory&,NativeGamePhysicsLifetimeCalls&,NativeGamePhysicsLifetimeProgress&);
// Complete103B profile-node recursive release,ECX node/RET. Frees node itself,
// its children/vector and heap SBO string. Complete114B engine destructor,
// stack engine/RET4, frees profile wrapper/tasks/list, not engine itself.
void delete_native_dyn_profile_node_00c35400(void*,const AvoidZoneDynHullMemory&,NativeGamePhysicsLifetimeCalls&,NativeGamePhysicsLifetimeProgress&);
void destroy_native_dyn_engine_00c421b0(void*,const AvoidZoneDynHullMemory&,NativeGamePhysicsLifetimeCalls&,NativeGamePhysicsLifetimeProgress&);
// Complete93B C4DDE0,ECX captured list header/RET. Reload list bounds after
// each world. Then destroy/free the CURRENT engine publication and clear it.
// Does not clear profile publication E9F8 or captured list entries/count.
// Normal source schedule only: native FH3/SEH, fault recovery, concurrency,
// original register ABI and application/gameplay admission remain unproved.
void destroy_native_game_physics_00c4dde0(void*,NativeGamePhysicsLifetimeContext&,NativeGamePhysicsLifetimeOperation&);
} // namespace bsp
