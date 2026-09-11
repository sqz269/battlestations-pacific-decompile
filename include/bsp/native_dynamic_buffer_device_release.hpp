#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Complete 00B237D0..00B238C2. Borrows the actual renderer, actual wrapper
// storage and real COM interfaces. Uses the existing actual guard providers.
// New C++ interface; original is ECX renderer / RET. No ownership is added.
// The native uninitialized-guard case (entry disabled, exit enabled) remains
// outside the defined caller domain; do not toggle mode that way during calls.
void release_native_dynamic_buffers_for_reset_00b237d0(
    void* actual_renderer, NativeRendererSynchronizationGlobals& actual_globals);

} // namespace bsp
