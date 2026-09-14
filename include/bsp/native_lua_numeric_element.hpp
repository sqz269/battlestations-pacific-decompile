#pragma once

#include "bsp/native_lua_objects.hpp"

namespace bsp {
// B66580: original thiscall ECX is the actual 14h Lua object. The signed
// integer key and float value occupy the two public stack words (RET 8).
// The source fastcall EDX word is unused; pass nullptr. Lua calls use the
// linked 5.1.1 C ABI, with explicit bridges in the body. No private game
// Lua ABI, C++ exception interception, or automatic object cleanup.
void __fastcall set_native_lua_numeric_element_00b66580(
    NativeLuaObjectStorage* actual_object,
    void* unused_source_edx,
    std::int32_t key,
    float value);
} // namespace bsp
