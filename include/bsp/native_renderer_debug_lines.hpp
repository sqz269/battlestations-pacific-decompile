#pragma once

#include "bsp/native_renderer_vertex_binding.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

struct NativeRendererDebugLinesContext {
    NativeRendererVertexBindingContext& actual_vertex;
    NativeRendererIndexBindingContext& actual_index;
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    // Borrow D5F0A8 through +138 inclusive. Numeric original slots select
    // substantive source providers; they are never called as host pointers.
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    const volatile float& actual_one_00d7a24c;
};

// Complete B28D00..B290C5 (966 bytes). Original ECX renderer, plain RET;
// source EDX adds this borrowed context. Initial DWORD+1D04==0 returns before
// touching context, which may be null only on that path. Otherwise borrow the
// same raw renderer, actual synchronization/owner domains and real COM device
// used by both binding contexts. No projected state cache is involved.
//
// Preserve ordered state/shader calls, three freshly loaded transforms in one
// reused64-byte local, current +134/+138 dispatch, then optional line-list draw.
// DWORD+1D90 or byte+1D8A skips only the draw. HRESULTs never gate subsequent
// work. Header+1D00 has stride14h, signed count+4/capacity+8: negative capacity
// invokes full B22940(0), then current positive count is drained and zeroed.
//
// No outer guard/unwind handler, rollback, cache restoration or count repair.
// Exceptions preserve prior effects and prevent unvisited calls/record drain.
// Accessed raw storage/profiles must remain valid across callbacks. Active
// original-renderer execution, original caller ABI/private stack aliases,
// asynchronous hardware-fault unwind and game rendering remain unproved.
void __fastcall draw_native_renderer_debug_lines_00b28d00(void* actual_renderer,
    const NativeRendererDebugLinesContext*);

} // namespace bsp
