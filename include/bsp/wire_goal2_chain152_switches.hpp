#pragma once

namespace bsp {
// Goal 2 chain 152 (g2-w4): segment 57, 008DF5D0 (the per-position push onto an
// objective's +20h list) wired into the mission Lua Objectives_Add / AddUnit target
// walk in game_hosts_lua.cpp. OFF keeps the host's rule that position targets add
// nothing.
inline constexpr bool kWireGoal2Chain152Bound = true;
}
