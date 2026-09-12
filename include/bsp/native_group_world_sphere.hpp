#pragma once
#include "bsp/native_group_owner.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Borrow the same CRT dispatch/error state and live native double D7A280.
// No constant defaults and no replacement sqrt policy. The numeric helpers
// preserve the caller's x87 control word and original spill/compare schedule.
struct NativeGroupSphereNumericAccess {
    const CameraAxesCrtAccess* crt;
    const volatile double* half_00d7a280;
};

// Complete B8E980: original ECX output XYZ, EDX input XYZ, stack length output,
// EAX output XYZ, RET4. C++ adds the explicit numeric access in a second stack
// argument (RET8). Store length before reloading source; aliasing is retained.
float* __fastcall normalize_native_group_sphere_delta_00b8e980(float* output,
    const float* input, float* length, const NativeGroupSphereNumericAccess*);
// Complete B8E9F0: original ECX destination sphere, stack source, EAX same
// destination, RET4. C++ adds the numeric access in EDX. No finite-only or
// fixed-rounding-mode shortcut: both FCOMI branches include unordered behavior.
float* __fastcall merge_native_group_sphere_00b8e9f0(float* destination,
    const NativeGroupSphereNumericAccess*, const float* source);

struct NativeGroupWorldSphereRuntime {
    NativeNodeDestructionRuntime& nodes;
    NativeGroupSphereNumericAccess numeric;
    void* context;
    const float* (*sphere_virtual48)(NativeGroupWorldSphereRuntime&,
        SceneNodeAttachment&);
};

// Complete B8EBE0 over the SAME live Group owner and +178/+17C descriptor.
// Existing +178 elements are CameraTransform companions for actual native node
// identities; this is the repository's established C++ projection, not a raw
// native pointer ABI. No hierarchy walk, second array or sphere cache is used.
// Capture begin/end once, skip unqualified children only until the first seed,
// then merge ALL subsequent children. Read each current element/callback anew.
// Callbacks must keep the captured allocation and all reached owners alive;
// element/count/cache mutation and reentry are allowed, concurrent edits are not.
// Empty radius comes from owner's existing live CE4970 constant binding.
// B8F100's static/ cached/ dynamic dispatch composition belongs to the caller;
// this routine always recomputes and ORs the SAME +138 with 0x30 on completion.
void aggregate_native_group_world_sphere_00b8ebe0(NativeGroupWorldSphereRuntime&,
    NativeGroupOwner&);
} // namespace bsp
