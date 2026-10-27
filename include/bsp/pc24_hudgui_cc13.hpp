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
//
// pc24w1-hudgui-2 sites:
//   src/award_trackers.cpp      00692FD0 zone distance^2 and radius^2 (00693189, 00693252)
//   src/game_hosts_units.cpp    0064B870 player_helm_issue: the lever quantization
//                               (0064BA99/0064BAA4, 0064BAD7/0064BADD); the same
//                               routine's integration copy in src/hud_warning_screen.cpp
//                               (integrated_controls_0064b870) is pc24w1-hudgui-1's
//   src/gui_framebox.cpp        00ACF9B0 sub2/scaled_sub2/uv_border/uv_border_y (their
//                               inline asm runs at the host's precision) and the uv
//                               middles (00ACFBC2, 00ACFC4B)
//   src/gui_render_order.cpp    00AA3E00 0.5/size + 0.5 and 0.5/size + 0.375
//   src/gui_screens_3.cpp       0054E020 the pause title and button y (0054E1D0..E1E2,
//                               0054E206..E216)
//   src/world_entities.cpp      006DC1A0 the tinted highlight alpha (006DC552/006DC561)
//   src/world_map_bounds.cpp    004D5BD0's layout 004C71C0 (004C72BC..004C7448) and
//                               008AECD0's (dx*dx + 0*0) + dz*dz (008AEEDF..008AEEEB)
// Expected movers when ON: HUD and GUI layout (frame boxes, hint panel, camera
// placement), marker alpha, the world border zone corners and the border push, the
// capture-zone landing/capture flags at the radius edge, and the player helm levers.
// 0043B370's gauge chain already has its own switch, kShipScreenGaugeProductRoundCc13Bound.
namespace bsp {
inline constexpr bool kPc24HudGuiCc13Bound = false;
}  // namespace bsp
