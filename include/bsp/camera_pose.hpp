#pragma once
#include "bsp/camera_look_at.hpp"

namespace bsp {
// Required adapter to the same actual owner represented by CameraState. The
// table/slot readers perform only the indicated raw integer load: no floating
// point operations, callbacks, retained inputs or mutations. They must preserve
// the FP environment, including a live x87 stack value in the base position setter.
// Tokens are actual table/entry identities; never substitute a cached override.
// Invocation uses the selected entry and passes the live input by reference.
struct CameraPoseAccess {
    void* context;
    std::uintptr_t (*load_vtable)(void*, CameraState&) noexcept;
    std::uintptr_t (*load_slot)(void*, std::uintptr_t table, std::uint32_t offset) noexcept;
    void (*invoke_position_30)(void*, std::uintptr_t entry, CameraState&, const CameraAxis&);
    void (*invoke_world_matrix_34)(void*, std::uintptr_t entry, CameraState&, const CameraMatrix&);
};

// Native ECX=this, stack position, tail JMP current virtual+34 with &this+F0.
// New typed ABI. Writes actual world[12..14] through ordered x87 loads/stores;
// resolves +34 after loading input.z and before storing world.z.
void set_transform_world_position_00b6dae0(CameraState&, const CameraAxis&, const CameraPoseAccess&);
// Native ECX=camera, stack position, RET4. Clears actual camera cache bits,
// calls the base setter above, then refreshes direction/target on the same state.
void set_camera_world_position_00b71400(CameraState&, const CameraAxis&, const CameraPoseAccess&);
// Native ECX=camera, stack eye then target, RET8 (saved pseudocode misses target).
// Selects actual virtual+30 before clearing flags. The eye and target remain live
// across that callback and subsequent CRT dispatches. Captures the final vtable
// before inverse, then loads its +34 entry after inverse. No early input snapshots
// are substituted for those live reads across callbacks.
// Both adapters and CRT bindings are checked before any input read/output change.
void set_camera_look_at_00b700e0(CameraState&, const CameraAxis& eye, const CameraAxis& target,
    const CameraPoseAccess&, const CameraAxesCrtAccess&);
// Bound variant captures actual D7A24C for up.y at B7016F, then passes its
// address to the builder's later independent B64232 reload. Legacy overload
// uses the installed positive-one word; no extra FLD/FSTP conversions.
void set_camera_look_at_00b700e0(CameraState&, const CameraAxis& eye, const CameraAxis& target,
    const CameraPoseAccess&, const CameraAxesCrtAccess&, const volatile std::uint32_t& one_00d7a24c);
}
