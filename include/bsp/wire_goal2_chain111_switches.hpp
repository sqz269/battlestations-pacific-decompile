#pragma once

namespace bsp {
// Goal 2, chain 111 (g2-w1): segment 59's 0090C5D0
// midway_save_folder_preexisted_0090c5d0, the predicate 004E4000 calls at
// 004E42F2 before its first-main-menu award block, bound in
// src/game_hosts_menu.cpp's MenuShellHost::award_gate_open over the process's
// actual native string pool (0041E870, 004261A0, 00419CC0, 00BD1510). ON runs
// the body's CreateDirectoryA probe of <CSIDL_PERSONAL>\Battlestations-Midway\save,
// as the game does; that call creates the last folder when only its parent
// exists. OFF keeps the stand-in's constant false and constructs nothing.
inline constexpr bool kWireGoal2Chain111Bound = false;
}
