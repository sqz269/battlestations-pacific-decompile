#pragma once

namespace bsp {
// Goal 2, chain 35 (src/game_hosts_hud.cpp): 0068AB80 collapse_in_game_overlays
// at the two HUD collapse sites (0068ACA0's 0068AD43 and 0068C1F0's 0068C6E7),
// fed with the menu host's screen-set state: the level-3 vector 00E18D18 and
// registry slot 4Ch's requested byte +4h.
inline constexpr bool kWireGoal2Chain35Bound = true;
}
