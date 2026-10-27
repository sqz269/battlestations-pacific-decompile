#pragma once

// PC24 wave switches for the plane-task / air-ops lane (cc13 pc24w1, packets
// pc24w1-sweep3-*). The game's main thread runs the x87 with 24-bit precision
// control, so every FADD/FSUB/FMUL/FDIV result is rounded to a 24-bit
// significand (docs/CC13_FPU_PRECISION.md). The bodies below evaluated multi-op
// chains in double and rounded once; the ON arms round per op through the
// cc13x87 helpers in the image's operation order. Each OFF arm is the previous
// code, unchanged.

namespace bsp {

// kPc24PlaneTasksCc13Bound (OFF): per-op PC24 arithmetic in
//   pc24w1-sweep3-1 (cc13-pc24w1-sweep3-1):
//     009C4F80 dive_bomb_aim_heading_009c4f80 (dive_bomb_task.cpp)
//     009C62B0 flyabove tick: dive_bomb_flyabove_altitude_009c6e10 and
//              dive_bomb_flyabove_bank_009c6857 (dive_bomb_task.cpp)
//     009AC190 kamikaze_gotowards_tick_009ac190 (kamikaze_task_cc13.cpp)
//     009AC910 kamikaze_aim_tick_009ac910 (kamikaze_task_cc13.cpp)
//     0042B2F0 planar_length (kamikaze_task_cc13.cpp)
//     00419510 normalize_00419510 (plane_follow_law.cpp)
//     009BFEE0 plane_follow_geometry_009bfee0 with its Phase A
//              plane_follow_phase_a_009c0251 (plane_follow_law.cpp)
//   pc24w1-sweep3-2 (depth charge; sites listed atop depth_charge_task_cc13.cpp):
//     009A3300, 009A5F50, 009A65F0, 009A3770, 009A3CF0, 009A4010, 009A4940
//   pc24w1-sweep3-3 (sites listed in each .cpp):
//     0041BC20 avoid_zone_sample (game_hosts_avoid_zones.cpp)
//     009B5C80 level_bomb_aim_tick and 009B6670 level_bomb_prepare_tick
//              (plane_level_bomb_task.cpp); dogfight_task.cpp stays under
//              kDogfightPc24Cc13Bound / kFollowWaitFloatHelpersCc13Bound
//   pc24w1-sweep3-7 (sites listed atop each .cpp):
//     009FC260 (leg_altitude_manager_cc13.cpp), 00414C60 and 009BD150
//     (moveonpath_task_cc13.cpp), 007BB6E0 (pilot_plan_slots.cpp), 0099D0A0
//     and 0099D300's 0099E490/0099D756/0099E344 (plane_ai_control.cpp),
//     007F23A0 (plane_formation.cpp; its game_hosts_units.cpp copy is
//     pc24w1-sweep3-5's), 00809E10 (rocket_task_cc13.cpp), 0080A000's
//     0080A024/0080A15B (rocket_homing_cc13.cpp)
//   Other copies of the same image routines are narrowed by their own
//   packets, not here: 0042B2F0 in plane_wanderer.cpp (pc24w1-sweep3-4) and
//   game_hosts_units.cpp avoid_len (pc24w1-sweep3-5); 009AC190's bearing in
//   plane_attitude_angles.cpp (pc24w1-sweep1-3, kPc24FlightRenderCc13Bound).
//   00419510 also has the bound naked copy camera_vector_normalize_00419510
//   (system_camera_axes.cpp) and the C++ copies in plane_pose_commit.cpp and
//   world_ocean.cpp.
//   Not here: 009AC0B0, 009ADE00 and 009AE9D0 round per op under
//   kKamikazeX87FloatCc13Bound, and 009C6857's cross-track sum under
//   kDiveBombPc24Cc13Bound; those sites flip with their own switches.
// Expected movers when ON: plane task waypoints and approach timing, kamikaze
// turn-around and pitch targets, the kamikaze lock window, the dive-bomb aim
// bearing, the flyabove dive reference and roll-in latch, and the
// formation-follow steer point (attack release points).
inline constexpr bool kPc24PlaneTasksCc13Bound = false;

}  // namespace bsp
