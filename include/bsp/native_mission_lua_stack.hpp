#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua stack access requires MSVC Win32.
#endif

namespace bsp {

// Complete raw-machine leaves 006B7E70/80/90/A0, ordinary Source interfaces.
// GetTop loads CURRENT machine+4 and returns genuine lua_gettop unchanged.
// Indexed leaves capture index BEFORE one CURRENT state load, then call the
// corresponding real linked C API once. No cross-call state cache or post-read.
// No LuaMachine cast, validation/default, checkstack, index mapping, Frame,
// owner creation, cleanup, catch, pcall, stack restoration or retry.
//
// Caller retains genuine raw machine backing and the reached live linked-Lua
// state/stack/value/code graph, with that operation's valid index/capacity and
// synchronization domain. Original VM bytes/numeric pointers establish none
// of this. Native registers, tail-jump/RET4, physical argument-home aliasing,
// private ABI, faults and runtime/parent-builder/factory/startup are unproved.
std::int32_t get_top_native_mission_lua_006b7e70(const void* actual_machine);
void set_top_native_mission_lua_006b7e80(
    const void* actual_machine, std::int32_t index);
void push_value_native_mission_lua_006b7e90(
    const void* actual_machine, std::int32_t index);
void remove_native_mission_lua_006b7ea0(
    const void* actual_machine, std::int32_t index);

} // namespace bsp
