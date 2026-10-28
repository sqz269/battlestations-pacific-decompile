#pragma once

// PC24 wave switches for the mission / Lua lane (cc13 pc24w1, packets
// pc24w1-mission-*). The game's main thread runs the x87 with 24-bit precision
// control, so every FADD/FSUB/FMUL/FDIV/FSQRT result is rounded to a 24-bit
// significand (docs/CC13_FPU_PRECISION.md). The bodies below evaluated multi-op
// chains in double and rounded once; the ON arms round per op through the
// cc13x87 helpers in the image's operation order. Each OFF arm is the previous
// code, unchanged.

namespace bsp {

// kPc24MissionLuaCc13Bound (OFF): per-op PC24 arithmetic in
//   pc24w1-mission-1 (cc13-pc24w1-mission-1):
//     00977990 scan_proximity_00977990_bound (game_hosts_mission_frame.cpp):
//              d2 = (dx*dx + dy*dy) + dz*dz.
//     006F5CC0 building_pads (game_hosts_script_orders.cpp): the 00419440
//              normalize of the pad facing (squares, sqrt, 1/len, scale).
//     007340A0 countdown_time_left and 00735100 run_countdown_update:
//              dur - (clock - start).
//     00735100 narrative display time n*char_wait + wait_base, and the fade
//              alpha steps (1/fade_in)*dt + alpha and alpha - (1/fade_out)*dt.
//     0042D2E0 basis_degrees_0042d2e0: (wrapped / pi) * 180.
//     00948CC0 row_lateral / row_gap / normalize_row (lua_spawn_new.cpp).
//     0094A140 spawn_candidate_frame_0094a140: ((RotY(a)*T(0,0,d))*RotY(-a))*R
//              through 00413920 / 00B646E0 replicas, each element stored as
//              float. OFF kept R's basis; ON carries the RotY(a)*RotY(-a)
//              rounding into it, as the image does.
//     005CAAF0 mission_tree_load_progress (mission_tree_data.cpp): n*0.02 + 0.45.
//   Census miss 00875BB0 (game_hosts_mission_frame.cpp hull-attitude trace) is
//   a diagnostic log line with no image arithmetic; it carries a lint allow.
//   Other copies of the normalize/planar helpers (00419510, 0042B2F0) are under
//   kPc24PlaneTasksCc13Bound.
//   Expected movers when ON: Lua proximity trigger hits, building pad facing
//   (ship AI landing), countdown / narrative timing and their callbacks,
//   camera-state degrees, spawn placement positions and frames, and the
//   loading-screen progress value.
// ON (cc9-recon2): flipped with kPc24LuaSpawnCc13Bound; 18 WINS-row pairs on afa0394fc.
inline constexpr bool kPc24MissionLuaCc13Bound = true;

}  // namespace bsp
