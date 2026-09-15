#pragma once

#include "bsp/native_lua_objects.hpp"

namespace bsp {
// B674C0: original ECX is the actual 14h Lua object; original stack words
// point to the current 8h key/value string headers; native exit is RET 8.
// EDX supplies the borrowed address of the actual 0108FF2C zero byte.
// This Win32 source interface preserves the public slot addresses, but the
// linked Lua 5.1.1 C API is not the game's private register ABI.
void __fastcall set_native_lua_string_field_00b674c0(
    NativeLuaObjectStorage* actual_object,
    const char* actual_empty_0108ff2c,
    const void* actual_key_header,
    const void* actual_value_header);
} // namespace bsp
