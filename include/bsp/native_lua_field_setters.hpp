#pragma once

#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Complete 00B67530: ECX is the actual 14h Lua object, followed by an actual
// 8h NativeString key and a pointer (RET 8). The C++ signature is a source
// interface, not a binary-compatible replacement for the original ABI.
// Neither setter changes the tracked-object records; lua_settable consumes
// only the temporary key and value it pushed onto the same Lua stack.
void native_lua_set_lightuserdata_00b67530(
    NativeLuaObjectStorage& object, const NativeString& key, void* value);

// Complete 00B67580: ECX object, stack NativeString key (RET 4).
void native_lua_set_new_table_00b67580(
    NativeLuaObjectStorage& object, const NativeString& key);

} // namespace bsp
