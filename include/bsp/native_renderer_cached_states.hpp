#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"
#include <cstdint>

namespace bsp {

// Complete B24460..B24509, native ECX renderer, stack state/value, RET8.
// Reads the actual validity byte before arming cleanup. For invalid/changed
// entries publish validity then value, call current device/table+E4, and only
// after return increment current counter+1BA0. HRESULT is ignored.
void set_native_renderer_render_state_00b24460(void* actual_renderer,
    std::uint32_t state, std::uint32_t value,
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc);

// Complete B24610..B24705, native ECX renderer, stack sampler/state/value,
// RET0C. Actual validity/value banks begin at+11CC/+11DC with72-byte stride.
// Unsigned samplers>=16 map to wrapped sampler+F1 for the actual COM call.
// Current device/table+114 follows cache publication; current+1BA4 increments
// only after that call returns. No enum, index or HRESULT policy is added.
void set_native_renderer_sampler_state_00b24610(void* actual_renderer,
    std::uint32_t sampler, std::uint32_t state, std::uint32_t value,
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc);

// Complete B26170..B262BF, native ECX renderer, no stack args, plain RET.
// Invoke the exact19 ordered render defaults, then seven ordered sampler
// defaults for each of20 banks, through the complete providers above.
void initialize_native_renderer_default_states_00b26170(void* actual_renderer,
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc);

// All three borrow the application's actual renderer, current device/COM table
// and synchronization globals. Raw byte/DWORD address arithmetic wraps at32bits;
// every reached access must remain valid. Optional guard entry occurs before
// cache checks; current mode is reread on normal/unwind leave. A skipped entry
// leaves its native-shaped guard record uninitialized, with no default repair.
// These are new C++ interfaces, not original-caller ABI or game validation.

} // namespace bsp
