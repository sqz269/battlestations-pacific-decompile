#pragma once

#include <cstdint>

namespace bsp {

// Ordinary Source projection of the low EAX bits observed at 006B8040's
// direct 00886F4C caller. This does not recover an Original signed/wide return
// type, high EDX, unknown indirect-call ABI, ECX/EDX inputs or RET4 convention.
//
// Caller supplies the genuine raw machine (CURRENT Lua-state pointer at +4),
// its matching live linked-Source Lua state/stack/code/index domain, and the
// actual mutable 0109EEA4 feature word. That word's ADDRESS reaches the existing
// CRT converter; its value is not sampled here. Retain these objects, CPU/mode
// truth and FP control/status/error/nonlocal domain for the entire call.
//
// Real lua_tonumber's ST0 flows directly into the existing BF7420 entry without
// a C++ floating temporary, F32/F64 snapshot, cast or FP-control policy. No
// owner, VM setup, validation, cleanup, stack recovery or caller grant is added.
std::uint32_t to_integer_bits_native_mission_lua_006b8040(
    const void* actual_machine, std::int32_t index,
    const volatile std::uint32_t& actual_conversion_word_0109eea4);

} // namespace bsp
