#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {
// Full00B67F10..00B68091[386]. Original ECX object, stack output, EAX output,
// RET4. Read indices1..4, write bytes2/1/0/3, and release each temporary before
// the next lookup. Actual Lua numeric conversion, without a float32 spill.
// Each channel uses x87 FISTP32 with truncation, stores its low byte, then
// restores the incoming control word. No clamping, kind check, default alpha,
// protected Lua error boundary, or added exception cleanup. The output must
// supply four writable bytes; the owner/stack/tracking domain must be valid.
// This is a Win32 source interface, not an original binary ABI replacement.
std::uint8_t* read_native_lua_color_00b67f10(
    NativeLuaObjectStorage& input, std::uint8_t* output);
} // namespace bsp
