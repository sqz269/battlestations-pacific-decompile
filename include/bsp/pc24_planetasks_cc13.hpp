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
//     009AC190 kamikaze_gotowards_tick_009ac190 (kamikaze_task_cc13.cpp)
//     009AC910 kamikaze_aim_tick_009ac910 (kamikaze_task_cc13.cpp)
//     0042B2F0 planar_length (kamikaze_task_cc13.cpp)
//     00419510 normalize_00419510 (plane_follow_law.cpp)
//     009BFEE0 plane_follow_geometry_009bfee0 with its Phase A
//              plane_follow_phase_a_009c0251 (plane_follow_law.cpp)
//   Other copies of the same image routines are narrowed by their own
//   packets, not here: 0042B2F0 in plane_wanderer.cpp (pc24w1-sweep3-4) and
//   game_hosts_units.cpp avoid_len (pc24w1-sweep3-5); 009AC190's bearing in
//   plane_attitude_angles.cpp (pc24w1-sweep1-3, kPc24FlightRenderCc13Bound).
//   00419510 also has the bound naked copy camera_vector_normalize_00419510
//   (system_camera_axes.cpp) and the C++ copies in plane_pose_commit.cpp and
//   world_ocean.cpp.
// Expected movers when ON: plane task waypoints and approach timing, kamikaze
// turn-around and pitch targets, the kamikaze lock window, the dive-bomb aim
// bearing and the formation-follow steer point (attack release points).
inline constexpr bool kPc24PlaneTasksCc13Bound = false;

}  // namespace bsp
