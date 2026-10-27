#pragma once

// PC24 wave switches, lane "ship AI" (cc13 pc24w1-sweep2-1). Header-only; later waves of the
// same lane reuse the switch.

namespace bsp {

// kPc24ShipAiCc13Bound: the main thread runs the x87 at 24-bit precision control, so every
// FADD/FSUB/FMUL/FDIV result rounds to 24 bits (extended exponent). OFF keeps the bodies'
// double chains that round once; ON narrows each live chain per x87 op with the
// include/bsp/cc13_x87_model.hpp helpers. Sites (pc24w1-sweep2-1):
//   009DF2D0 ship_ai_follow_update_formation_point and published speed, 0070EFD0
//            ship_ai_formation_reshape_slot (ship_ai_formation.cpp)
//   009EAE20 / 009EAFC0 neighbour near / avoid box refresh (ship_ai_neighbour_box.cpp)
//   009D9550 ship_ai_path_prune (ship_ai_path_search.cpp)
//   004F3970 tangent chord / 009E3C00 follower point (ship_ai_path_follower.cpp)
//   009D15F0 torpedo_aim_tick_full (torpedo_aim_tick.cpp)
//   0074F930 unit_leak_tick / 00937440 unit_steering_torque (unit_forces.cpp; 00937440 is
//            not live while kTorpedoBoatRudderTorqueBound is ON)
// Movers: ship headings, throttle and path picks, submarine depth, torpedo run times and hits.
inline constexpr bool kPc24ShipAiCc13Bound = false;

}  // namespace bsp
