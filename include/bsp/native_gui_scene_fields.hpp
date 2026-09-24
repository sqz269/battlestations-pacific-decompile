#pragma once
#include "bsp/native_render_context.hpp"
#include "bsp/native_texture_surface_getter.hpp"

namespace bsp {
struct NativeGuiSceneFieldsContext {
    NativeRenderActualOwnerRegistry& owners;
    NativeTextureSurfaceReferenceIncrement const volatile& increment_00ce221c;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
};
// Complete59B B723F0. Native ECX actual24h scene, stacked actual resource,
// RET4. Capture requested caller word FIRST, then current scene+1C. If unequal,
// publish requested, current increment(requested+4), current decrement(old+4),
// and only atzero resolve the captured old owner's canonical companion and
// dispatch its CURRENT0. Verify it borrows the SAME actual+4 atomic. No final
// field reload/clear; callback mutations remain. No admission or rollback.
void set_native_gui_scene_resource_00b723f0(void* actual_scene,
    const volatile std::uint32_t& actual_requested_argument,
    NativeGuiSceneFieldsContext&);
// Complete33B B721F0. Native ECX actual24h scene, stacked actual node, RET4.
// Capture scene+C into node+3C, zero node+40, reread CURRENT scene+C; if nonnull
// write that head+40=node; then publish scene+C=node. Exact payload aliases
// permitted when each reached access is valid; no logical root/node projection.
void prepend_native_gui_scene_node_00b721f0(void* actual_scene,
    void* actual_node) noexcept;
// Existing B72110/B72220 remain separate providers. These leaves do not admit
// attachment propagation, registry registration or AC59A0. New source ABI;
// native private caller frames, concurrent mutation and gameplay excluded.
} // namespace bsp
