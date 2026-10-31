#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua iteration requires MSVC Win32.
#endif

namespace bsp {

// Complete normal 006B8130/006B85B0/006B85F0, ordinary Source interfaces.
// Each reads CURRENT raw machine+4 once and calls a genuine linked C API.
// Next captures index before state and returns the actual full signed result.
// Pop reads state FIRST, then computes DWORD FFFFFFFF-count and bit-copies the
// signed target index. No signed overflow, clamp, zero skip or lua_pop macro.
// PushNil/Pop are void; incidental lower EAX is not an output contract.
//
// Caller retains genuine readable machine backing and matching linked-Source
// state/stack/code/value graph with valid index/key/capacity and synchronization.
// No model cast, default/validation, checkstack, Context/Frame, owner creation,
// cleanup, catch, pcall, restoration, retry or post-call payload observation.
// Ordinary by-value/private-home normalization is not Native tail/ECX/EDX/RET4,
// private ABI/FH3/SEH/fault/runtime or parent-builder/VM/factory/startup authority.
void push_nil_native_mission_lua_006b8130(const void* actual_machine);
std::int32_t next_native_mission_lua_006b85b0(
    const void* actual_machine, std::int32_t index);
void pop_native_mission_lua_006b85f0(
    const void* actual_machine, std::int32_t count);

} // namespace bsp
