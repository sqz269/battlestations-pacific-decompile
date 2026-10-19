#pragma once

// Packet cc13-follow (audit of the 009C1FD0 release arm, 2026-10-18).
//
// 009C1FFD/009C2006: with unit+C25h set or unit+C20h > 0, the follow tick
// 009C1FD0 takes 009C200F-009C2063 (seven command stores, RET 4) and skips the
// follow law 009BFEE0/009BEE30. GameUnitsHost::Impl::kFollowReleaseArmBound
// applies that arm in run_follow_tick_009c1fd0 and the moveto follow arm only.
// Five other host renditions of 009C1FD0 run the station and the law with no
// C25h/C20h test:
//   - the torpedo prepare/done seam 009D2731 (follow_base_tick_009c1fd0);
//   - the dive-bomb done tick 009C7278;
//   - the dive-bomb follow state (task+52Ch, vtable 00D20AB8 slot +0Ch);
//   - the dogfight follow state;
//   - the strafe follow/prepare tick (st_follow_tick_009c1fd0).
// st_follow_tick_009c1fd0 is shared, so the switch reaches three more tasks
// (eight host paths in all), each of which reaches 009C1FD0 in the image too:
//   - kamikaze follow/prepare (KzHost::tick_formation_state), the plain
//     follow vtable;
//   - drop-kamikaze follow, and prepare/done through 009AD1F0
//     (DkzHost::tick_formation_state). 009AD1F0 calls 009C1FD0 (009AD371)
//     only when +A0h is clear (009AD1F3) and +98h <= 0 or unordered (009AD218
//     JBE). The host makes that call only on the same arm:
//     drop_kamikaze_follow_tick_009ad1f0's runs_follow_base
//     (src/kamikaze_task_cc13.cpp, the S::kPrepare/S::kDone case). So the
//     arm cannot fire on a DKZ tick the image never takes into 009C1FD0;
//   - depth-charge follow and prepare/done through 009A4940 (CALL 009C1FD0 at
//     009A4950), via the depth-charge adapter's tick_formation_state.
// True: each of those seams tests C25h/C20h after the station and takes the
// arm instead of the law, as the image does. False: unchanged.
// The arm draws no random number (00BD2F10 untouched).
//
// NOT MODELLED, recorded only (switch or no switch): none of these seams runs
// 009C1FD0's post-law tail: the +85h trail arm at 009C207C and the +74h sight
// search at 009C211D (run_follow_tick_009c1fd0 has both). The torpedo seam and
// the dogfight follow do not write cmd+26Ch = 2 (009C1FE2); the strafe-shared,
// dive-bomb done and dive-bomb follow seams do.
namespace bsp {
inline constexpr bool kFollowReleaseSeamsBound = true;   // ON: OCEAN_WAVE_FIELD 98 (cc9-physics23)
}  // namespace bsp
