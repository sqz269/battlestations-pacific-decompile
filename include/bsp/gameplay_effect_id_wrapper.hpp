#pragma once

#include "bsp/gameplay_effect_acquisition.hpp"

namespace bsp {
// Ordinary C++ adaptation of complete 00870CD0..00870CF9 (42 bytes).
// Capture the actual output address and ID, obtain the genuine canonical
// manager unconditionally, then load the ACTUAL borrowed flag word. Forward
// its full width to 008700E0 and return the captured output address regardless
// of the lower return value. Do not preclear out or bypass the getter for ID0.
//
// Requires a live void* output object, a live uint32 flag object, and the
// application's genuine compatible acquisition context and dependencies.
// A successful getter must provide a live canonical GameplayEffectManager.
// Borrowed object/reference lifetimes and addresses stay valid across calls.
// Providers may mutate the valid flag storage before its post-getter load.
// Output/flag must not alias private wrapper locals, reference bindings,
// context metadata, or incompatible C++ objects. No raw-parent-slot cast or
// private manager/context is supplied by this API.
//
// Preserve genuine Lua/string/component/lifetime/error obligations. Provider
// failure propagates with completed effects; no catch, retry or rollback.
// This new ABI does not reproduce native ECX/EDX/RET4, private-frame aliases,
// FH3/SEH/fault behavior, profile vtables, or production Damageable binding.
void** acquire_gameplay_effect_by_id_00870cd0(void*& out, std::int32_t id,
    const volatile std::uint32_t& actual_flag_word,
    GameplayEffectAcquisitionContext& context);
} // namespace bsp
