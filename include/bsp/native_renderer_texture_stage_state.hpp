#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"
#include <cstdint>

namespace bsp {

// Complete B24510..B24600, native ECX renderer and stack stage/state/value,
// RET0C. Actual validity/value banks start at +45C/+480 with ACh stride.
// All DWORD index/address arithmetic wraps; there is no stage remapping.
// Guard entry precedes the validity read; cleanup is armed only AFTER it.
// Invalid/changed entries publish validity then value, call the current
// device/table+10C, and increment current +1BA8 only after that call returns.
// HRESULT is ignored. Current synchronization mode is read before disarming
// normal cleanup and before either saved guard field is read. Unwind invokes
// the full actual guard destructor. A skipped entry leaves the guard record
// uninitialized, without repair if the mode later changes.
// Borrow actual renderer/COM/global storage. This is a new C++ interface,
// not original-caller binary ABI, a semantic state cache, or game validation.
void set_native_renderer_texture_stage_state_00b24510(void* actual_renderer,
    std::uint32_t stage, std::uint32_t state, std::uint32_t value,
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc);

} // namespace bsp
