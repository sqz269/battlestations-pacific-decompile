#pragma once

#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_renderer_index_binding.hpp"
#include "bsp/native_renderer_texture_binding.hpp"
#include "bsp/native_renderer_vertex_layout_binding.hpp"

namespace bsp {

// Actual logical D61D6C (at least twelve original-token DWORDs) and physical
// D61E34/D61E7C (at least eight DWORDs each) share the full lifetime domain.
// Canonical companions borrow each raw owner's SAME +04 atomic and execute
// its CURRENT terminal, including complete logical destruction and pool return.
struct NativeRendererVertexBindingContext {
    NativeLogicalVertexOwnerContext& actual_logical_owner;
};

// Complete B48CF0 ten-byte tail getter: current logical+58 -> current physical
// profile -> slot+1C B4B9F0 -> borrowed real COM buffer at physical+28.
// Numeric original profiles/slots are selectors, never host call addresses.
void* get_native_logical_vertex_buffer_00b48cf0(const void* actual_logical,
    const NativeLogicalBufferDeviceRestoreProfiles&);

// Complete B23710, native ECX destination pointer cell, EDX source pointer
// cell, EAX same destination, RET. Capture source before old; on change publish
// and increment incoming before decrementing old and resolving only at zero.
void* assign_native_renderer_vertex_owner_00b23710(void* actual_destination_cell,
    const void* actual_source_cell, NativeRenderActualOwners&);

// Complete B24840, native ECX renderer, stack stream index/logical owner, RET8.
// Requires actual renderer through +1BB7, logical74h/pool78h storage and actual
// declaration stride+CC. Optional guard precedes identity. Distinct identities
// with equal current buffers/cached stride/cached offset still replace owners
// but skip COM. Otherwise publish ownership, recompute original incoming values,
// write offset then stride, call current SetStreamSource and increment+1BB4.
void bind_native_renderer_vertex_stream_00b24840(void* actual_renderer,
    std::uint32_t stream_index, void* actual_logical,
    NativeRendererVertexBindingContext&);

struct NativeRendererBindingResetContext {
    NativeRendererVertexBindingContext& actual_vertex;
    NativeRendererTextureBindingContext& actual_texture;
    NativeRendererIndexBindingContext& actual_index;
    NativeRendererVertexLayoutBindingContext& actual_layout;
    // D5F0A8 through slot+138 inclusive (79 DWORDs). Current renderer profile
    // is checked anew for each virtual call; this is the observed full profile.
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
};

// Complete B24BF0 (453 bytes), native ECX renderer, RET. All contexts share
// actual renderer/global/owner domains. Clear20 current virtual+130 textures,
// call current virtual+134 with (0,null) FOUR times, then virtual+138(null,0).
// Clear pixel+176C and vertex+1770 cache BEFORE their optional COM unbinds;
// counters increment only on return. Three native EH states reuse one guard.
// Current virtual+E0(null) layout, four actual color clears, current device
// depth+9C(null). No depth counter increment. Requires actual default surface.
void reset_native_renderer_bindings_00b24bf0(void* actual_renderer,
    NativeRendererBindingResetContext&);

// Complete within the evidenced actual-profile domain; no HRESULT gate,
// rollback, null repair, synthesized owner or recreation stand-in. Raw pointers
// remain valid at native accesses, including unsigned wrapped stream addresses.
// Guard storage starts uninitialized and is reused by reset: newly enabling a
// never-entered guard remains outside the native valid domain. Normal leave is
// disarmed; exceptional cleanup is B21110, terminating a second C++ exception.
// These new source interfaces are not original game ABI replacements. Whole
// B29670 recreation and the runtime producer of logical base+4C remain unproved.
} // namespace bsp
