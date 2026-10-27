#pragma once

// follow_to_body per op (packet cc13-followbody). Header-only.

namespace bsp {

// kFollowToBodyPc24Cc13Bound (OFF): GameUnitsHost::Impl::follow_to_body
// (src/game_hosts_units.cpp) stands in for 004142E0 (a point times the inverse
// the image caches at unit+110h) and 0042D0D0 with flag 0 (a direction times
// the same matrix, no normalize). OFF projects onto the pose rows divided by
// their length, in double, rounded once. ON builds the inverse as 00B63D50 does
// (each row over its SQUARED length, translation -t times it) and applies it as
// the two routines do, every x87 op rounded at PC24 and every listing store a
// float. The two agree in exact arithmetic only for unit-length rows.
// Labelled: the image reads the cached +110h (rebuilt when byte +10Ch is clear);
// ON rebuilds it from the current u.world on every call.
// Callers (all in game_hosts_units.cpp, so all move together): gunfire_detect
// 0099EC40 (shooter frame l, velocity v, own frame o), run_follow_hold_arm
// 009BEE56 and the follow law (station, sight, steer points; 009BEE92,
// 009BF57F), follow_sight_tick 009C211D, vehicle_avoidance 007DF4F0,
// friendly_in_line_eval 007DEDB0, the dogfight threat cone 009A85B0 and give-up
// ratio 009A7B98, the takeoff run step 009CE2C0's ship obstacle factor, and
// motion_step 00825F20's "wing trace2" log line (log text only).
// Movers when ON: gunfire avoidance flags and dodge direction, formation
// station keeping and steer points, sight-follow targets, vehicle avoidance
// probes, friendly line-of-fire tests, dogfight threat and give-up decisions,
// the takeoff run's obstacle factor near ships.
inline constexpr bool kFollowToBodyPc24Cc13Bound = false;

}  // namespace bsp
