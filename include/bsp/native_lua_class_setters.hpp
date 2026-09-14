#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {
// Complete native bodies; ECX actual14h target, two stack arguments, RET8.
// No kind gates or owner validation. Each Lua primitive reloads target owner;
// final settable reads the current target index after both pushes.
void native_lua_set_integer_00b67460(NativeLuaObjectStorage& target,
    const NativeString& key, std::int32_t value);
// C-string null value pushes nil; a null key follows Lua's nil-key error path.
void native_lua_set_cstring_00b66790(NativeLuaObjectStorage& target,
    const char* key, const char* value);
// Uses TARGET owner's state with VALUE's current index; does not consult
// value.owner, copy/register an object, or normalize a relative index.
void native_lua_set_object_00b675d0(NativeLuaObjectStorage& target,
    const NativeString& key, const NativeLuaObjectStorage& value);
} // namespace bsp
