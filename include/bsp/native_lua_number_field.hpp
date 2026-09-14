#pragma once

#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Complete 00B67400..00B67455. Original ECX = actual LuaObject, stack =
// NativeString pointer then float32, RET8. This is a source interface, not a
// binary replacement. Operates on the actual Lua 5.1.1 stack; kind04 is ignored.
// Reads key length/data after checkstack, substitutes the native empty literal
// for null data without changing length, widens through x87 FLD/FSTP, and
// reloads owner/state for every library call. Final owner, index, state reads
// retain their native order. No Lua error interception or ownership changes.
// Evidence and exception/FPU boundaries: docs/NATIVE_LUA_NUMBER_FIELD.md.
void native_lua_set_number_field_00b67400(NativeLuaObjectStorage&,
    const NativeString& key, float value);

} // namespace bsp
