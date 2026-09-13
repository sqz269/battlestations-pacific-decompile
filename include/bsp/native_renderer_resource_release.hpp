#pragma once
#include "bsp/native_renderer_cache_clear.hpp"
#include "bsp/native_renderer_resource_restore.hpp"

namespace bsp {
// Borrow the SAME renderer, synchronization, owner and profile domains used by
// binding, cache cleanup and resource restoration. All reached profile slots
// must remain live; numeric image words select concrete source providers.
struct NativeRendererResourceReleaseContext {
    NativeRendererBindingResetContext& actual_bindings;
    NativeRendererCacheClearContext& actual_cache;
    const NativeRendererResourceRestoreProfiles& actual_resources;
};

// Complete B262C0..B26493, original ECX renderer, no stack arguments, RET.
// Enter optional guard before reading ready+1D8B; arm after that read. If ready,
// clear it before twenty texture calls, FOUR vertex(0,null) calls and index,
// depth/four color surface releases, live queries/textures/surfaces traversal,
// the native callback-free record walk, and full cache clear on renderer+34.
// Current count/base/profile loads retain native order around callbacks. No
// owner arrays or wrappers are copied, no reference or HRESULT policy added.
void release_native_renderer_resources_00b262c0(void* actual_renderer,
    NativeRendererResourceReleaseContext&);

// Actual renderer through+1D8B and all reached raw extents/profiles must remain
// valid. Texture profiles D61948/D61870/D618B0, surface D619A0 and query D62AD0
// select the existing full reset providers; renderer profile is D5F0A8.
// Guard storage is uninitialized when entry is skipped, as native; a later
// mode change must not cause an invalid read. Normal leave is disarmed first;
// exceptions use B21110, without resource rollback. Second C++ cleanup failure
// terminates. Hardware SEH, full device recreation and gameplay are unproven.
} // namespace bsp
