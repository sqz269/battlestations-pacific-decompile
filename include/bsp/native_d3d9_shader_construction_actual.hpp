#pragma once
#include "bsp/native_d3d9_shader_construction.hpp"

namespace bsp {
// Borrow the same application's actual AA8/AA4/AA0, FEDC and F8D394 cells.
// This context adds no owner, manager, allocator, diagnostic operation or
// admission-time publication load. All referenced storage outlives the call.
struct NativeD3d9ShaderConstructionActualContext {
    NativeStringRawPoolContext& strings;
    NativeResourceSupportStorage* volatile& actual_support_0108fedc;
    void* const volatile& actual_renderer_00f8d394;
};

// Complete raw registry bodies, shared with the retained legacy overloads.
// ECX actual header/renderer, one stacked argument, RET4 in the original.
// New C++ signatures have explicit parameters. No registry owns its pointees.
// Allocation/copy/free failure preserves native partial effects, without an
// operation allocation, rollback, or recovery of native leaked allocations.
void reserve_native_vertex_shader_registry_00b22dd0(void*, std::int32_t);
void reserve_native_pixel_shader_registry_00b22e30(void*, std::int32_t);
void register_native_vertex_shader_00b289a0(void*, void*);
void register_native_pixel_shader_00b289f0(void*, void*);

// B5E720/B5E7E0: exact native ECX/RET tail to BD30F0. Restore CEB130
// only; acquired COM+08, refcount+04 and opaque+0C remain untouched.
void __fastcall destroy_native_pixel_shader_base_00b5e720(void*) noexcept;
void __fastcall destroy_native_vertex_shader_base_00b5e7e0(void*) noexcept;
// B3F4C0 ECX points to {borrowed COM, length, pooled data}. Capture current
// +08, then current +04+1; get the real current pool and return. Ignore COM,
// leave the temporary unchanged, and propagate getter failure.
void destroy_native_buffer_diagnostic_record_00b3f4c0(void*, NativeStringRawPoolContext&);

// Original ECX actual fresh10h owner, stacked COM argument, EAX owner, RET4.
// These C++ overloads require an additional explicit context, not binary ABI
// compatibility. Real current COM vslot4 AddRef and current renderer registry.
// Pixel registers after both normal pool returns; vertex registers before
// either name construction. Native states arm after resize/copy and disarm
// before normal return. C++ dependency exceptions run only armed FH3 actions:
// second temporary, first string, base. They do not Release COM, unregister,
// free the caller's owner, or clean allocations whose states were not armed.
// A cleanup exception during C++ unwind terminates. Native FH3/SEH and full
// effect-compiler/caller ownership composition remain unproved.
NativeD3d9ShaderStorage* construct_native_pixel_shader_00b5f9b0(void*, void*,
    NativeD3d9ShaderConstructionActualContext&);
NativeD3d9ShaderStorage* construct_native_vertex_shader_00b5faf0(void*, void*,
    NativeD3d9ShaderConstructionActualContext&);

namespace detail {
// Internal shared-body entry for the established retained-operation interface.
NativeD3d9ShaderStorage* construct_native_shader_retained(void*, void*,
    NativeD3d9ShaderConstructionContext&, NativeD3d9ShaderConstructionOperation&, bool pixel);
}
} // namespace bsp
