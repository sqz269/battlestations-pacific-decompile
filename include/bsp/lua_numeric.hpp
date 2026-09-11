#pragma once

#include "bsp/gui_lua_reader.hpp"

namespace bsp {
// Original ECX=LuaObject, RET. Both accessors spill lua_tonumber to float32
// and reload it before returning/converting. New host-reference interface.
float lua_object_number_00b66270(GuiLuaHost&, GuiLuaRef);
// Native0109EEA4 is tested AFTER the Lua call. The required live mode aliases
// that choice; SSE2 returns CVTTSD2SI bits, x87 returns low32 of the existing
// complete BF7456 kernel. No saturation, zero fallback, or C++ out-of-range cast.
std::int32_t lua_object_integer_00b66290(GuiLuaHost&, GuiLuaRef,
    const bool& crt_sse2_conversion);
} // namespace bsp
