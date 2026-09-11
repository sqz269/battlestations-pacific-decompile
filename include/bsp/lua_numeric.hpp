#pragma once

#include "bsp/gui_lua_reader.hpp"

namespace bsp {
// Original ECX=LuaObject, RET. Both accessors spill lua_tonumber to float32
// and reload it before returning/converting. New host-reference interface.
float lua_object_number_00b66270(GuiLuaHost&, GuiLuaRef);
// Conversion tails shared by live Lua objects and evaluated-value readers.
float lua_number_float32_00b66270(double number) noexcept;
std::int32_t lua_number_integer_00b66290(double number,
    const bool& crt_sse2_conversion) noexcept;
//00BD5816 uses this instruction directly for a FloatIndex key. It does not
// consult0109EEA4 and does not run the x87 integer-conversion kernel.
std::int32_t lua_float_index_00bd5790(float number) noexcept;
// Native0109EEA4 is tested AFTER the Lua call. The required live mode aliases
// that choice; SSE2 returns CVTTSD2SI bits, x87 returns low32 of the existing
// complete BF7456 kernel. No saturation, zero fallback, or C++ out-of-range cast.
std::int32_t lua_object_integer_00b66290(GuiLuaHost&, GuiLuaRef,
    const bool& crt_sse2_conversion);
} // namespace bsp
