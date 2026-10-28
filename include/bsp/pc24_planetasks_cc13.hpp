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
//   pc24w2-dc (wave-2 census): 009A3390's fly-over distance U(1.1, 1.3) * radius +
//     FlyAboveDist * +24h (009A34AF..009A34C6), inlined in
//     depth_charge_task_construct_009a5240 (depth_charge_task_cc13.cpp)
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
//     Other 00414C60 copies, not switched: bsp::length_2d_00414c60 in
//     vector_helpers.cpp (its length_kernel is a naked-asm replica of the
//     image's x87 sequence, so it rounds per op at the caller's precision)
//     and length_2d_00414c60 in plane_water_law.cpp:34 (float ops)
//   pc24w1-sweep3-4 (sites listed at each body):
//     007BE060 plane_wanderer_fixed_step_007be060, with length3_0042b2f0
//              ((y*y + x*x) + z*z and the 1e-10 floor) and 00439950
//              plane_wanderer_random_direction_00439950 (plane_wanderer.cpp)
//     009C8F40 retreat_refresh_target_009c8f40 and 009C9FB0
//              retreat_arm_009c9fb0 (plane_retreat_task.cpp)
//     009BA020 tight_turn_tick_009ba020 fragment, 2.0 - e / 10 degrees
//              (strafe_task.cpp)
//   pc24w1-sweep3-5 (game_hosts_units.cpp; sites listed at each body):
//     007D7DA0 leader_turn_rate (x, y, z and z + (cb * c0) * yaw_spd)
//     009BEE30 follow law: the 009BFD70 latch d / length / dot, the latched
//              k and steer point, the 009BF13D sight point
//     0042B2F0 avoid_len (length_0042b2f0_pc24, with the 1e-10 floor)
//     007DF4F0 vehicle avoidance: closing dots, (d - rr) / s, t, box top,
//              gap, 1.5 / PitchSpd, zz and the box-corner sums
//     0099CAB0 band shaper fade and BC4h value
//     0099F1C0 terrain avoidance: C7Ch length, p0, c0, inverted k,
//              (1 - w) * f.y, fan run lengths and band edges, the 009A1420
//              water fan offset
//     006BEE40 carrier holder: the 0042D0D0 offset rows
//     Not narrowed: 004142E0 follow_to_body (a substitution; the image uses
//     the stored inverse at unit+110h)
//   pc24w1-sweep3-6 (game_hosts_units.cpp air ops; sites listed at each body):
//     006CDF70 landing blocks width * 0.8 + half width; 007C5AC0 site timer;
//     006CFE90 elevator nose distance; 006D0390 elevator lane top and bound;
//     007C5F60 launch-spot slide (006CF730's chain); 006C3B10 corridor zp,
//     ceiling and overshoot; 006C3E50 landing radius (with 007C6760's mix);
//     006C6020 landing path length; 006C3F80 landing spacing floor, sum and x
//   cc13-pc24bear (game_hosts_units.cpp update_dive_bomb_approach): the planar
//     pairs' float differences, sums of squares and bearings at 009C7B34..
//     009C7BD9 (approach+BCh, +C0h), 009C5950..009C5ADB (+5Ch, [ESP+18h]) and
//     009C5207..009C52F8 (+14h); 009C8B14's 3D range is not narrowed
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
// formation-follow steer point (attack release points); from sweep3-4, the
// wanderer drift, retreat target and countdown and tight-turn limit; from sweep3-5, the
// leader turn rate, vehicle and terrain avoidance and carrier holder offsets;
// from sweep3-6, carrier landing approach, spacing and launch-spot placement.
inline constexpr bool kPc24PlaneTasksCc13Bound = false;

}  // namespace bsp
