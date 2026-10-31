#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua scalar pushes require MSVC Win32.
#endif

namespace bsp {

// Complete game-owned 006B80C0/006B80E0/006B8120, ordinary Source APIs.
// Boolean: CURRENT state FIRST, normalize the low BYTE to full int0/1.
// String: capture pointer FIRST, CURRENT state, real lua_pushstring once;
// actual NULL->pushnil / strlen behavior belongs to that genuine C API.
// Number: actual initialized4B HOME FLD32 FIRST, CURRENT state, FSTP64 into
// the actual argument area, one real cdecl lua_pushnumber. No implicit C++
// widening, FP helper while ST0 is live, extra round or control-word policy.
//
// Caller retains real machine backing and matching linked-Source VM/stack/code,
// capacity, FP-stack/environment, HOME/CString extent/lifetime and serialization.
// No model cast, default/guard/checkstack, Context/Frame, owner creation,
// cleanup, catch/pcall, stack restoration, retry or post-call payload read.
// Normal Source argument cleanup is not Lua-stack rollback. Original private
// HOME/register/tail/ABI/FH3/SEH/fault/runtime and parent/VM/startup are unproved.
void push_boolean_native_mission_lua_006b80c0(
    const void* actual_machine, std::uint8_t value);
void push_number_native_mission_lua_006b80e0(
    const void* actual_machine, const void* actual_float32_home);
void push_string_native_mission_lua_006b8120(
    const void* actual_machine, const char* text);

} // namespace bsp
