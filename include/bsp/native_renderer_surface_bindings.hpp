#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"
#include <cstdint>

namespace bsp {
struct NativeSurfaceOwnerStorage;

// Complete 00B23D80: native ECX renderer, stack slot/wrapper, RET8.
// Borrow actual renderer storage (through +1BA3h), actual 34h surface wrappers,
// and the current real IDirect3DDevice9 pointer at renderer+1A10h.
// A null wrapper at slot zero reads the default wrapper at renderer+197Ch.
// Only a nonnull supplied wrapper increments the DWORD at +1BA0h, including
// when its COM pointer is null or SetRenderTarget returns a failed HRESULT.
void bind_native_renderer_color_surface_00b23d80(void* actual_renderer,
    std::uint32_t slot, const NativeSurfaceOwnerStorage* actual_wrapper,
    NativeRendererSynchronizationGlobals&);

// Complete 00B21690: native ECX renderer, stack wrapper, RET4.
// Borrow actual renderer storage through +1BCFh. Always call the actual device
// SetDepthStencilSurface, then increment +1BCCh only for a nonnull wrapper.
void bind_native_renderer_depth_surface_00b21690(void* actual_renderer,
    const NativeSurfaceOwnerStorage* actual_wrapper,
    NativeRendererSynchronizationGlobals&);

// Both are new C++ interfaces, with no recovered HRESULT return contract.
// They preserve optional entry/cleanup scope, current mode checks, captured
// COM arguments, native counter wrapping, and absence of wrapper ownership.
// Native skipped-entry guards remain uninitialized; callers must not create
// an enabled cleanup from such storage by changing mode asynchronously.
} // namespace bsp
