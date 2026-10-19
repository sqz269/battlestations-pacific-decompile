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
// True: each of those seams tests C25h/C20h after the station and takes the
// arm instead of the law, as the image does. False: unchanged.
// The arm draws no random number (00BD2F10 untouched).
namespace bsp {
inline constexpr bool kFollowReleaseSeamsBound = false;
}  // namespace bsp
