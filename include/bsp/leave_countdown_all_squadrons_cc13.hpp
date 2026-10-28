#pragma once

// Packet cc13-leavecd (census S9, handed back from cc9-ships64).

namespace bsp {

// kLeaveCountdownAllSquadronsCc13Bound (OFF): the plane's leave-map countdown 007C6C30
// (vtable +1D8h of the plane classes, unit+6ECh/+6F0h) for every plane, not only retreating
// ones.
// Call point: 007CE040 (the plane's fixed step) calls 00953CC0 first (007CE08F); 00953CC0
// runs vtable[+1D8h](step) at 00953D98 only while unit+520h is clear (00953D7D/00953D84), so
// the countdown runs once per plane per fixed step, before the motion dispatch (the pilot
// bot and its task included). Host: PlaneBinding::out_of_action_countdown_007c6c30, which
// run_plane_fixed_step_007ce040 calls behind the +520h gate (generic_suppress_520).
// 007C6C30 (007C6C30-007C6E08, gameplay_seg47_cc13.cpp out_of_action_countdown_007c6c30):
//   +5Dh set: nothing. Inside the map (0071C4F0), or +9D4h set and 007EE2E0(sq, 1) false:
//   +6F0h = Retreat/ExitTime (tuning+34Ch), +6ECh = 1, and the leader (+9D8h == 0) zeroes
//   squadron+36Ch. Else +6F0h -= step (007C6C86..007C6CA5, one FSUB then FSTP float: one
//   op, so the C++ float subtraction is the PC24 result), +6ECh = 2, or 3 with +6F0h = -1.0
//   past Retreat/ExitDist (0059C9B0, tuning+348h) or below zero; the leader runs the HUD
//   warning 00982120 and squadron+36Ch (-= step for a local-player plane, else 1.0; reseeded
//   to tuning+350h); +6F0h below zero sends 007EE410 (-> 007CA790 -> 007F31A0, Kill(4)).
// 007EE2E0(sq, check_leader) (007EE2E0-007EE403, cc15_seg48.cpp): with the flag, the leader
//   ([sq+3D0h]) in local-player role 1 (00927F30) or with +184h set; then the squadron's
//   current command (vcall +174h): retreat 00E08F90; moveto 00E08F68 or attackmove 00E08F78
//   with a position target (+1h) outside the map; moveonpath 00E08F80 whose path's last
//   point (0071BFF0(0), 007B1350) is valid and outside the map. 007F31A0 asks it again with
//   the flag clear (007F32D1) before it sets the +361h exit latch.
// OFF: run_retreat_task_tick_009ca130 runs the countdown at its tail, for a plane with the
//   retreat task installed, with retreat_squadron_ordered as the whole predicate.
// ON: the countdown runs in the plane fixed step for every live plane. Retreat arm: the same
//   proxy (retreat task installed and retreat_squadron_ordered), so a retreating plane counts
//   as before; the other arms read the flight leader's director (current_command_0071be40,
//   active_command_descriptor_0071eb60, path_final_point_007b1350) and its role words.
//   LABELLED: a plane with no squadron record is its own one-plane squadron; squadron+36Ch
//   is kept per squadron name and only feeds the 00982120 record.
// Movers: planes outside the map whose squadron (or flight leader) holds a moveto or
// attackmove to an off-map point, or a moveonpath ending off the map, or whose leader is
// player-held (role 1) or has +184h set, now count Retreat/ExitTime down and leave the map
// (Kill(4), "exitzone") when it runs out or at once past Retreat/ExitDist. Retreating
// planes: unchanged, except that the countdown now runs before the task tick in the same
// fixed step, and a tick on which the task retires resets the timer instead of freezing it.
inline constexpr bool kLeaveCountdownAllSquadronsCc13Bound = false;

}  // namespace bsp
