#pragma once

#include "bsp/dyn_body_creation.hpp"

namespace bsp {
// Native storage consumed by dyn_body_creation and dyn_collision_pass. No copy
// operation or destructor is supplied: allocated children have owning lifetimes.
struct alignas(4) DynSceneStorage { unsigned char bytes[0xe8]; };
struct alignas(4) DynManifoldContainerStorage { unsigned char bytes[0x1f0]; };

// Actual complete dispatcher objects, not vtables or arrays of substitute calls.
// The seven small native objects contain their one-word vtable pointer; the
// shared GeneralConvexIntersect object also owns support directions and a lock.
struct DynSceneDispatchObjects {
    void* box_box;              // object 00E17434, vtable 00D7A184
    void* terrain_convex_mesh;  // object 00E17438, vtable 00D7A18C
    void* sphere_sphere;        // object 00E17440, vtable 00D7A1CC
    void* box_sphere;           // object 00E17444, vtable 00D7A1D4
    void* general_convex;       // object 0109EA48, producer 00C48FD0
    void* convex_ray;           // object 00E17448, vtable 00D7A1F4
    void* box_ray;              // object 00E174E8, vtable 00D7A1FC
    void* sphere_ray;           // object 00E174EC, vtable 00D7A204
};

struct DynSceneRuntimeContext {
    AvoidZoneDynHullMemory memory;
    // Native global 0109E9FC: initialized engine, whose +10 is a real thread
    // pool and whose thread-pool +4 is its worker count. Read after pool setup.
    void* const* engine_slot;
    // Native global 0109E9F4: constructor sets this to dispatch.general_convex.
    void** general_convex_slot;
    DynSceneDispatchObjects dispatch;
    const void* sap_manager_vtable;     // complete Dyn::SAPBroadPhaseManager2
    const void* intersect_task_vtable;  // complete CollisionSystem::IntersectTask2
};

// 0040A1E0..0040A2DF: stack pool, EAX result, RET4. Constructs the first 1D4h
// bytes of a fresh container: 1,000 E0h slots, free chain and two sentinels.
// The caller constructs its owner pointer and CRITICAL_SECTION suffix.
void dyn_manifold_pool_construct_0040a1e0(
    DynManifoldContainerStorage&, const AvoidZoneDynHullMemory&);

// 00C38070..00C38461: stack scene/world, EAX scene, RET8. Complete normal
// successful-allocation storage behavior for fresh scene storage. C++ interface
// has explicit globals/allocator/vtables; it is not a native ABI replacement.
// Scene owns its SAP manager, manifold container, task arrays and event buffer;
// it borrows world, engine and dispatch objects. They must stay valid throughout
// use. Full engine/global-dispatch startup, collision execution and scene/world
// destruction are separate responsibilities; no dummy runtime is constructed.
DynSceneStorage* dyn_scene_construct_00c38070(DynSceneStorage&, void* world,
    const DynSceneRuntimeContext&);
} // namespace bsp
