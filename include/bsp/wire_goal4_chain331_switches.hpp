#pragma once

namespace bsp {
// Goal 4 chain 331: the Lua LoadMessageMap binding (008C61C0) loads the mission
// message map into the MessageManager [game+21DCh] through 00706760
// (008C62F6..008C6308). OFF keeps the binding's empty message_map_load.
inline constexpr bool kWireGoal4Chain331Bound = true;
} // namespace bsp
