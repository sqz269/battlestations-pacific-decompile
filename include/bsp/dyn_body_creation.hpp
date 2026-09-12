#pragma once

#include "bsp/avoid_zone_dyn_hull.hpp"
#include "bsp/ship_hull_shapes.hpp"
#include <windows.h>

namespace bsp {
// Native storage, not replacements for the existing semantic DynBody/MotionState.
// Unspecified bytes remain unchanged. The documented functions require MSVC Win32.
struct alignas(4) DynBodyStorage { unsigned char bytes[0x88]; };
struct alignas(4) DynMotionStorage { unsigned char bytes[0xc8]; };
struct alignas(4) DynConvexShapeStorage { unsigned char bytes[0x218]; };
struct alignas(4) DynSapProxyStorage { unsigned char bytes[0x50]; };

// Exact 38h native allocator block at 0109ECF0. Registry and vtable are borrowed;
// pages and page-pointer vector belong to this allocator. Never copy this object.
struct DynConvexShapePoolStorage {
    const void* vtable;
    void* previous;
    void* next;
    CRITICAL_SECTION critical_section;
    std::uint32_t lock_depth;
    void** pages;
    std::uint32_t page_count, page_capacity, first_free_page;
};
static_assert(sizeof(DynConvexShapePoolStorage) == 0x38);

// A real native world (48Ch) and scene (E8h) must already own their pools and
// runtime dependencies. scene+AC must be a Dyn::SAPBroadPhaseManager2 (248h).
// Its complete vtable and the ConvexMeshShape vtable must be supplied by the
// owning runtime, with callable Win32 methods. No replacement vtable is invented.
// The shape vtable's lifetime operations must use this same convex allocator.
struct DynBodyCreationContext {
    AvoidZoneDynHullMemory memory;
    DynConvexShapePoolStorage* convex_pool;
    const void* convex_shape_vtable;
};

// Native complete pool constructors. Storage sizes: body 124h, motion 1A4h,
// endpoint/pair 34h, proxy B4h. Each owns 1,000 slots plus a page-pointer vector.
void dyn_body_pool_construct_00409170(void*, const AvoidZoneDynHullMemory&);
void dyn_motion_pool_construct_00409450(void*, const AvoidZoneDynHullMemory&);
void dyn_endpoint_pool_construct_0040b4a0(void*, const AvoidZoneDynHullMemory&);
void dyn_pair_pool_construct_0040ac90(void*, const AvoidZoneDynHullMemory&);
void dyn_proxy_pool_construct_0040b750(void*, const AvoidZoneDynHullMemory&);
void dyn_sap_manager_construct_00c36f10(void* manager, const void* complete_vtable,
    const AvoidZoneDynHullMemory&);
// Only the three pool constructors and final shared-motion allocation/stores
// from 00C41AD0; not a world constructor. Does not initialize world settings,
// scene, collision dispatch or thread tasks. Fresh, uninitialized pools only.
void dyn_world_body_pool_fragment_00c41ad0(void* world, const AvoidZoneDynHullMemory&);

void dyn_convex_pool_construct_00407c70(DynConvexShapePoolStorage&,
    void*& allocator_registry_head, const void* allocator_vtable,
    const AvoidZoneDynHullMemory&);
void dyn_convex_pool_page_construct_004085f0(void* page, std::uint32_t page_index);
DynConvexShapeStorage* dyn_convex_shape_allocate_00407ed0(
    DynConvexShapePoolStorage&, const AvoidZoneDynHullMemory&);
// Returns a slot only; does not unlink or destroy a live shape.
void dyn_convex_shape_free_00408040(DynConvexShapePoolStorage&,
    DynConvexShapeStorage&);

void dyn_body_storage_init_00c43ca0(DynBodyStorage&, const DynBodyDescriptor&);
void dyn_sap_proxy_init_00c4cda0(DynSapProxyStorage&, DynBodyStorage&,
    const DynAabb&, bool is_static, const AvoidZoneDynHullMemory&);
DynSapProxyStorage* dyn_sap_create_proxy_00c54aa0(void* manager,
    DynBodyStorage&, const DynAabb&, bool is_static, const AvoidZoneDynHullMemory&);
void dyn_body_register_broadphase_00c50470(DynBodyStorage&, void* scene,
    const AvoidZoneDynHullMemory&);
void dyn_body_recompute_bounds_00c55fc0(DynBodyStorage&);
void dyn_convex_shape_refresh_bounds_00c57c40(DynConvexShapeStorage&);
void dyn_convex_shape_construct_00c57f50(DynConvexShapeStorage&, DynBodyStorage&,
    const DynShapeDescriptor&, const void* complete_vtable);

// Complete type-4 branch and its post-switch continuation. Other native shape
// constructors are outside this API, and rejected before mutating storage.
void dyn_body_attach_convex_shape_00c5c940(DynBodyStorage&,
    const DynShapeDescriptor&, const DynBodyCreationContext&);

// Both static and dynamic allocation branches of 00C5D580, restricted to convex
// descriptors. Shapes is the native descriptor+78/+7C vector made explicit;
// desc.shape_count must equal count. Hull data is borrowed by each shape, so the
// owner's retained handle must outlive the body. Owning world/scene construction,
// SAP endpoint insertion, collision stepping and body destruction are separate.
DynBodyStorage* dyn_world_create_convex_body_00c5d580(void* native_world,
    const DynBodyDescriptor&, const DynShapeDescriptor* const* shapes,
    std::uint32_t count, const DynBodyCreationContext&);
} // namespace bsp
