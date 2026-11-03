#pragma once

// Goal 3 chain 233 (game_hosts_hud.cpp): the HUD warning screen's exit-zone
// alert. ON, the screen-50h update's world-edge test 00681F40 reads the world
// bounds GGame+711Ch..+7130h from the host's own border box (the avoid-zone
// load's select_world_map_bounds_004d5ede result in GameMissionLuaHost) and the
// controlled unit's +FCh position, and the show routines 00682CB0/00682D60/
// 00682E10/00682ED0 run warning_screen_show_alert over the HUD's real warning
// widgets through a Fe5Host adapter. OFF, the test answers false and the show
// is a record, as before.
namespace bsp {
inline constexpr bool kWireGoal3Chain233Bound = false;
}
