#pragma once

#include "bsp/native_lua_objects.hpp"

#include <cstdint>

namespace bsp {

// Complete 00B673A0 (51h bytes): ECX is the actual 14h Lua object, followed
// by an actual 8h NativeString key pointer and a four-byte argument slot whose
// low byte is the boolean (RET 8). This is an explicit C++ source interface,
// not a binary-compatible replacement for the original __thiscall ABI.
//
// There is no kind/owner validation, metadata update or early return when
// lua_checkstack fails. Each Lua primitive reloads the object's current owner
// and state; lua_settable also reads the object's current index after both
// pushes. A null key data pointer supplies an empty byte while retaining the
// key's stored length.
void native_lua_set_boolean_00b673a0(
    NativeLuaObjectStorage& object, const NativeString& key, std::uint32_t value);

} // namespace bsp
