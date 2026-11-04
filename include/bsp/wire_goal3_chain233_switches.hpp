#pragma once

#include "bsp/wire_goal4_chain340_switches.hpp"

// Goal 3 chain 233 (game_hosts_hud.cpp): the HUD warning screen's exit-zone
// alert. ON, the screen-50h update's world-edge test 00681F40 reads the world
// bounds GGame+711Ch..+7130h from the host's own border box (the avoid-zone
// load's select_world_map_bounds_004d5ede result in GameMissionLuaHost) and the
// controlled unit's +FCh position, and the show routines 00682CB0/00682D60/
// 00682E10/00682ED0 run warning_screen_show_alert over the HUD's real warning
// widgets through a Fe5Host adapter. OFF, the test answers false and the show
// is a record, as before.
//
// Chain 233 landed OFF under the goal-3 rule, after the 2026-11-03 flip of
// every wiring switch (fa3b65aad), so it never ran. Goal 4 chain 340 turns it
// on and owns the decision: kWireGoal4Chain340Bound OFF restores the old OFF
// state. Checked before the flip: in warning_screen_update_00683020 only alert 3
// (exit zone) can be raised here (plane_stall_007c6e10 and ship_shallow_1011
// answer false, submarine_127c answers 1.0 against the 0.2 threshold). Alert
// 3's show reaches widget_set_visible_34, text_set_localised_key_00abbe50,
// widget_set_alpha_4c and, through 00682800, the GUI sound class and
// sound_create_tracked_normal with a null sample; WarningAlertFe5Binding
// overrides all of them, so no Fe5HostUnreached failure is reachable. The
// stall-only 00690FD0 hint (alert 0) is not.
namespace bsp {
inline constexpr bool kWireGoal3Chain233Bound = kWireGoal4Chain340Bound;
}
