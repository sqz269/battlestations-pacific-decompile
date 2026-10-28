#pragma once

// cc13-tranche2 S18 (census row units:19882 "target-not-dead stands for 007EE8F0").
// 009F8160, the squadron auto-target object's tick, re-asks 007EE8F0 for a command of
// category 1 or 2 before its dogfight leash arm:
//   009F82DA..009F82EC  class vtable[0Ch] == 1 or 2, else straight to 009F8319;
//   009F82EE            no resolved target (ESI = 00521EA0's answer) -> 009F8412;
//   009F82F6..009F830C  007EE8F0(ECX = squadron [EBX+8h]; class EBP, target ESI,
//                       00922DC0(target) and 007B58D0(squadron));
//   009F8313            refused -> 009F8412, the leash arm (009F8319) is not reached.
// The leash arm needs class 00E08F58 (dogfight), so the arm that matters is 007EE8F0's
// dogfight arm 007EEAEC..007EEB31 behind its side prologue 007EE8F6..007EE932
// (bsp::attack_command_applies): the target is airborne (00922B10: +5Dh clear and a
// plane or plane squadron), the slot-0 plane's PilotFires byte +C24h is set, it is not
// IsKindOf(16h), and 0047B850 (IsKindOf(10h) || IsKindOf(16h)) answers 0; the sides
// differ and the target is not neutral.
//
// True: the host's leash gate asks bsp::attack_command_applies with those inputs, read
// on the squadron's flight leader. LABELLED: the leader stands for the squadron's side
// (+54h) and for its slot-0 plane [+3D0h], as the leash block already takes the
// leader's director for the squadron's; the dead test stays as the stand-in for
// 00521EA0 resolving no object. False: "the target is not dead" alone (the old
// substitution).
// Movers when ON: a dogfight whose target stopped being an airborne enemy (+5Dh set,
// a landed or same-side target) or whose leader has no PilotFires skips the leash
// arm, as in the image; expected rare.
//
// CORRECTION (rev7 FIX 1): the refusal at 009F8313 also skips the 009F83AD..009F8407
// tail (slot 0 / 0080DC70 / 009F7FE0 -> 0071D9E0(2) + 009F7810, then 009F840C's
// +3Eh -> +3Dh), which the host gates on the same answer for every category 1/2
// class. The classes reaching 009F82DA are artillery 00E08F10, strafe 00E08F40,
// attackmove 00E08F78 and dogfight 00E08F58, so the test now runs for all four:
// strafe's arm 007EEB94..007EEBC7 (the 00922DC0 surface byte, or IsKindOf(41h);
// +C24h; 0047B850), attackmove's side prologue alone, and artillery, which no arm
// names (007EEBD1 JNZ 007EEBF6, XOR AL,AL): always refused under ON. More movers
// when ON: every artillery free attack and any refused strafe/attackmove/dogfight
// skips the tail's 0071D9E0(2) override stage and 009F7810 timer.

namespace bsp {

// ON (cc9-sweep3b, 2026-10-28): census differs on all six short rows (USN01 7433, BSM09
// 28526, BSM06 h1_i 253, LOMP10 8412, PRCP01 23289, USN12 4664 of the gate answers), yet
// the pairs (ab36d2284) are gameplay identical with the same end times on all six.
inline constexpr bool kFreeAttackAcceptTestCc13Bound = true;

}  // namespace bsp
