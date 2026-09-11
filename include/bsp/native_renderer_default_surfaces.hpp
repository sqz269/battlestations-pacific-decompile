#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"
#include "bsp/native_surface_owner.hpp"
#include <cstdint>

namespace bsp {

struct NativeRendererDefaultSurfacesContext {
    NativeSurfaceOwnerContext& surface_owner;
    NativeRendererSynchronizationGlobals& synchronization;
    const std::uint32_t (&original_surface_profile_00d619a0)[2];
};
static_assert(sizeof(NativeRendererDefaultSurfacesContext) == 12);

// Complete 00B238D0 [562 bytes]: original ECX actual renderer, RET, no recovered
// HRESULT. This new ECX/EDX interface borrows raw renderer fields +197C/+198C,
// current device +1A10, and the full depth binder's synchronization/counter state.
// The bound static canonical pool must be surface_owner.actual_surface_pool_0108db00.
// Reached final-zero dispatches require actual D619A0 with original slots
// BD30E0/B3F5B0; the second profile read remains current. No other profile domain
// is admitted. Both surface constructors receive flags=0 and kind=0.
// Current output and allocation spill cells survive external calls. Only failed
// construction returns its current raw slot; later failures do not undo field
// publication or release COM output temporaries. This does not reproduce the
// original stack layout, native SEH ABI, or an incidental EAX return contract.
void __fastcall capture_native_renderer_default_surfaces_00b238d0(
    void* actual_renderer, const NativeRendererDefaultSurfacesContext* context);

} // namespace bsp
