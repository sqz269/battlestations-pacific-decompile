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
//   004F3970 tangent chord / 009E3C00 follower point (ship_ai_path_follower.cpp; the
//            other 009E3C00 copy, ship_ai_path_point.cpp, has no caller and is not switched)
//   009D15F0 torpedo_aim_tick_full (torpedo_aim_tick.cpp)
//   0074F930 unit_leak_tick / 00937440 unit_steering_torque (unit_forces.cpp; 00937440 is
//            not live while kTorpedoBoatRudderTorqueBound is ON)
// Sites (pc24w1-sweep2-4):
//   009F4DA0 formation throttle ceiling, 009E6170 traffic record range test, 007470B0 /
//            00745BE0 land fort fire rolls (game_hosts_ship_ai.cpp)
//   00852860 attack-move altitude gate, 009F3240 approach throttle
//            (ship_ai_attackmove_substates.cpp)
//   009DE5B0 arm final step squares and reach (ship_ai_arm_final_step.cpp)
//   0095EB40 firepower rating excess and contribution (ship_ai_bearing_rating.cpp)
//   009ED6B0 arm tail normalize 004192E0 (game_hosts_ship_ai.cpp); turn lead, look-ahead
//            radius (ship_ai_navigation.cpp); stopping distance, setback radicand,
//            astern limit (ship_ai_navigation_arm_tail.cpp)
// Sites (pc24w1-sweep2-5):
//   00A1C960 nearest group, 00A1A660 sphere test, 00A2C150/1C0/230 command target
//            distance (band_009f_cc13.cpp); 0074A990 landing time (building_pads.cpp)
//   008FFF20 torpedo run end degrees and friendly crossing threshold (gun_bot_remainder.cpp)
//   00852B90 submarine surface limit (sensor_tables.cpp)
//   0092E8C0 righting rate, 00825F20 target speed and boost refill (ship_motion.cpp)
//   008252C0 engine audio, 00834E90 steering target and propeller rates (unit_motion.cpp)
//   009D29E0 torpedo done follow blend (torpedo_task_arm.cpp)
// Movers: ship headings, throttle and path picks, submarine depth, torpedo run times and hits.
inline constexpr bool kPc24ShipAiCc13Bound = false;

}  // namespace bsp
