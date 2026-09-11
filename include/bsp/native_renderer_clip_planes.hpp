#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Full B23E50..B23EF6: ECX renderer, stack index/input float4, RET8.
// Actual renderer+190C+DWORD(index<<4) cache; unchecked x86 index wrapping.
// Four ordered x87 FLD/FSTP pairs preserve floating status/quieting and alias
// behavior. Native guard cleanup is armed only after the first two stores.
// SetClipPlane uses the original input pointer and current device/table+DC;
// its HRESULT is ignored. This new interface borrows actual global storage.
void set_native_renderer_clip_plane_00b23e50(void* actual_renderer,
    std::uint32_t index, const void* actual_four_coefficients,
    NativeRendererSynchronizationGlobals&);

// Full B25040..B2507E: ECX renderer, stack float4, RET4. Set at current
// active+19EC, increment its current value after return, then set state98
// to DWORD((1 << (current_active &31))-1). No additional direct pending+19F0
// store is added; unchecked child cache aliasing and callbacks remain visible.
void append_native_renderer_clip_plane_00b25040(void* actual_renderer,
    const void* actual_four_coefficients, NativeRendererSynchronizationGlobals&);

// Full B25080..B250AE: ECX renderer, RET. Set state98 using current pending
// +19F0, then reload pending AFTER that call and copy it to active+19EC.
void restore_native_renderer_pending_clip_planes_00b25080(
    void* actual_renderer, NativeRendererSynchronizationGlobals&);

// Original skipped-entry guards remain uninitialized. A later mode change
// requiring their cleanup is outside the native valid domain. No new bounds,
// device ownership, floating-point mode, original ABI or game claim is added.

} // namespace bsp
