#pragma once

namespace bsp {

// Goal 4 chain 299 (g4-w4): the checkpoint Lua copies run through the native
// LuaObject bodies over the live mission state. ON: 004379C0's per-slot copy
// is 00436730 over 14h LuaObjects (number-key gate 00436A0B..00436A13, setters
// 00B66530/00B665D0/00B66580/00B66740/00B66670 and 00B673A0/00B67460/00B67400/
// 00B66790/00B67580), and 00438020's script-table copy is the native iteration
// 0043846F..004384DB calling 00436C50. OFF: the raw Lua API copy_slot.
// Evidence: src/profile_checkpoint.cpp, reports/wire-g4-c299.json.
inline constexpr bool kWireGoal4Chain299Bound = true;

} // namespace bsp
