#pragma once

#include "bsp/dyn_scene_runtime.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
// Full global owner at 0109EA48. The constructor does not touch padding_04.
// Storage must stay at one address from construction through final CS deletion.
struct alignas(8) DynGeneralConvexIntersectStorage {
    const void* vtable_00;
    std::uint32_t padding_04;
    double support_directions_08[26][3];
    CRITICAL_SECTION critical_section_278;
};
static_assert(offsetof(DynGeneralConvexIntersectStorage, support_directions_08) == 8);
static_assert(offsetof(DynGeneralConvexIntersectStorage, critical_section_278) == 0x278);
static_assert(sizeof(DynGeneralConvexIntersectStorage) == 0x290);

// Each static PE object is exactly one actual vtable pointer. The vtables have
// one virtual collision/ray entry and adjacent RTTI, not lifecycle slots.
struct DynStaticDispatchObjectStorage { const void* vtable; };
static_assert(sizeof(DynStaticDispatchObjectStorage) == 4);

// Complete actual callable tables, supplied by their owning runtime. This
// module does not manufacture callback tables or substitute collision methods.
struct DynDispatchVtables {
    const void* box_box_00d7a184;             // vslot0 00C49A30
    const void* terrain_convex_mesh_00d7a18c; // vslot0 00C53630
    const void* sphere_sphere_00d7a1cc;       // vslot0 00C518D0
    const void* box_sphere_00d7a1d4;          // vslot0 00C48330
    const void* general_convex_00d7a1a8;      // vslot0 00C535E0
    const void* convex_ray_00d7a1f4;          // vslot0 00C44780
    const void* box_ray_00d7a1fc;             // vslot0 00C50DD0
    const void* sphere_ray_00d7a204;          // vslot0 00C50740
};

// New aggregate, not a claimed contiguous native global layout. Value-initialized
// storage represents the zero BSS before CRT startup. Seven PE data words must
// then be bound before scene construction. No automatic CS destruction: native
// teardown is registered explicitly and runs after the last borrowing scene.
struct DynDispatchGlobalsStorage {
    DynStaticDispatchObjectStorage box_box{};
    DynStaticDispatchObjectStorage terrain_convex_mesh{};
    DynStaticDispatchObjectStorage sphere_sphere{};
    DynStaticDispatchObjectStorage box_sphere{};
    DynStaticDispatchObjectStorage convex_ray{};
    DynStaticDispatchObjectStorage box_ray{};
    DynStaticDispatchObjectStorage sphere_ray{};
    DynGeneralConvexIntersectStorage general_convex{};

    DynDispatchGlobalsStorage() = default;
    DynDispatchGlobalsStorage(const DynDispatchGlobalsStorage&) = delete;
    DynDispatchGlobalsStorage& operator=(const DynDispatchGlobalsStorage&) = delete;
};

// PE data producer, not an invented native initializer: copies the actual seven
// initialized words at E17434/38/40/44/48/E8/EC. Does not initialize general_convex.
void bind_dyn_dispatch_static_objects(DynDispatchGlobalsStorage&, const DynDispatchVtables&);
DynSceneDispatchObjects dyn_scene_dispatch_objects(DynDispatchGlobalsStorage&) noexcept;

// Complete 00C48FD0..00C49222, native cdecl/no args/EAX global owner/RET. This
// typed interface supplies the actual global storage, table and recovered CRT
// state. Publishes vtable, initializes the real CS with spin10000 (BOOL ignored),
// writes 78 ternary doubles in native order, then runs all26 x87 normalizations.
// Preserves extended ST0 across the existing recovered CRT entry. No new sqrt.
DynGeneralConvexIntersectStorage* dyn_general_convex_construct_00c48fd0(
    DynGeneralConvexIntersectStorage&, const void* complete_vtable,
    const CameraAxesCrtAccess&);

// Complete 00CD91D0..00CD91DB, cdecl/no args/RET. DeleteCriticalSection only;
// no vtable/direction clear or deallocation. Exactly once after last use.
void dyn_general_convex_destroy_00cd91d0(DynGeneralConvexIntersectStorage&) noexcept;

using DynDispatchAtexit = int (__cdecl *)(void (__cdecl *)());
// Bind the process's sole actual owner before registration; it must outlive
// every registered callback. Rebinding to another owner is an invalid boundary.
void bind_static_dyn_dispatch_globals_0109ea48(DynDispatchGlobalsStorage&);
// Complete 00CC8950..00CC8960. Construct first, then register CD91D0 cleanup
// with actual CRT atexit. Returns registration status unchanged; failure does
// not undo construction. This function does not bind the seven PE data words.
int initialize_static_dyn_dispatch_00cc8950(const void* complete_vtable,
    const CameraAxesCrtAccess&, DynDispatchAtexit);
void __cdecl destroy_static_dyn_dispatch_00cd91d0();
// All entries are new C++ interfaces, not native global/exception ABI or game
// validation claims. No collision method or task execution is reconstructed here.
} // namespace bsp
