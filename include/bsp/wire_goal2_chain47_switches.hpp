#pragma once

namespace bsp {
// Goal 2, chain 47, one arm in src/game_hosts_hud_world.cpp:
// - 005BD590 constructs each minimap icon entry (005C173A in 005C0F20) from the
//   minimap_units_white_Group template IsKindOf picks and the red/blue/grey
//   colour 005BE240 copied from those groups' item_ship_Icon, through
//   MinimapIconEntryBinding; OFF keeps the clone of the colour group's
//   item_ship_Icon chosen by the markers rule 00639990.
inline constexpr bool kWireGoal2Chain47Bound = false;
}
