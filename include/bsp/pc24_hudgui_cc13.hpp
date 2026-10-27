#pragma once

// PC24 lane hudgui (cc13 PC24 wave 1, packets pc24w1-hudgui-1 and -2). The HUD and helm
// input bodies run on the main thread, whose x87 runs at 24-bit precision control: every
// FADD/FSUB/FMUL/FDIV rounds to a 24-bit significand (with the extended exponent) before
// the next op. ON evaluates each listed chain per op through the cc13x87 helpers
// (include/bsp/cc13_x87_model.hpp); OFF keeps the earlier double chains rounded once.
//
// pc24w1-hudgui-1 sites:
//   src/hud_ship_screen.cpp     0064ABD0 flash decay factor, 0064A9F0 stick angle and ease,
//                               0064AAD0 rudder dial value and offset
//   src/hud_warning_screen.cpp  0064B870 thrust/turn integration, 0051F050 binocular yaw and
//                               pitch steps, 00526A40 pick ray end and lock radius
//   src/game_hosts_hud_world.cpp 005C0F20 direction wedge (005C1864/005C186C)
// Copies not narrowed by this packet: 0064B870 in src/game_hosts_units.cpp
// (player_helm_issue_0064b870, packet pc24w1-hudgui-2); 0051F050's helm_aim_axes in
// src/game_hosts_hud.cpp is a host inverse of the step, not an image chain.
// 0043B370's gauge chain already has its own switch, kShipScreenGaugeProductRoundCc13Bound.
namespace bsp {
inline constexpr bool kPc24HudGuiCc13Bound = false;
}  // namespace bsp
