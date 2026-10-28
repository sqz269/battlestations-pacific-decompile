#pragma once

// Packet cc13-strafe70 (follow-up to census S6, cc13-flag5d).

namespace bsp {

// kStrafeTargetFallback70Cc13Bound (OFF): the strafe task's target pair, approach+54h else
// approach+70h (approach = task+3F8h, so task+44Ch / task+468h).
// Writers, all from the one target the task is built with (009CA4A0's argument, the
// command target):
//   +54h  009CA5C6 zeroes it, 009CA62C stores the target when vt[5Ch](41h) answers true
//         (kUnitKindQueryNavPoint, a position order's NavPoint), else 0. No other writer.
//   +70h  the target ref's sub+18h (the ref sits at approach+58h). 009FB200 (009CA5CD)
//         stores the target through 009FB190 when vt[5Ch](2) answers true (commandable),
//         with sub+14h = the target when vt[5Ch](5) answers true (unit); its 009FADA0
//         call (009FB364) and every later one (009CCED0's first call, 009CCEE1) clear
//         sub+14h and sub+18h at 009FAE0A/009FAE11 once [sub+14h]+5Dh is set. 009FB5E0
//         (SetTarget) also writes it, but only for the depth charge and dogfight tasks.
//         009FB190 registers sub+18h with the target's observer list (00694A60, EDX = the
//         ref, so its primary table 00D21CB4). Slot +4h, 009FAF90, runs when the target
//         goes through the kill list: the row-15 drain (009274DE..00927510, 00696330)
//         sets +5Dh/+5Eh/+5Fh and notifies +4h, as does the direct removal 009263C0
//         (the same flags, then 00925C40 -> 00696330); 009FAF90 moves +1Ch by the dying
//         target's matrix and clears sub+18h and sub+14h (009FAFF4/009FAFF7), whatever
//         state the task is in, the goaway included. The damage destroy hook 00926390
//         notifies slot +8h instead, 0042B120 (RET 4), so a target destroyed by damage
//         alone is dropped by the next 009FADA0. 009FB670 (slot 1 of the second
//         sub-object, table entry 7) clears sub+14h only (009FB687) for a notifier at
//         sub+14h, which no strafe path registers (009FB190 registers sub+18h only,
//         docs/CC13_SEG68.md section 3), so it does not run for this ref.
//         So for a unit target +70h holds the target until it is Killed (kill list) or
//         until the first approach update that sees its +5Dh set, then stays 0.
//         009FADA0 call sites on the strafe ref: 009FB364 (construct), 009CCEE1
//         (009CCED0, from the arm at 009CD182) and 009CCED0(0) from the cruise profile
//         009CD020 at 009CD0BB (that call is not hosted, see kStrafeProfileActivationSeedBound;
//         harmless here, the latch only needs one 009FADA0 run after +5Dh).
// Strafe reads of the pair:
//   009CCF1C..009CCF3B  range test: none means out of range (host: rin.has_target)
//   009CB659..009CB6A4  goaway pitch re-plan: none skips the climb/drop pitch, else the
//                       target's +100h (host: st_goaway_pitch_009cb650)
//   009CC6ED, 009CC7EB  the rule 009CC690, the attack mode 2 arms (host: ri.strafe_target_44c
//                       / ri.ref_target_468)
//   009CA80A            +70h alone, into 007B7870 (host: the kOpportunisticRocketBound call
//                       in st_gotowards_enter)
//   009CC898            009CC850 IsCommandStillValid, not on the units host strafe path
// OFF: the commanded target with the host's liveness (df_slot_live: +5Ch set and
// +5Dh/+5Eh/+60h clear), and the rocket call gets the commanded target at any liveness.
// ON: +54h = the target if kind 41h; +70h = the target if kind 2, dropped for good by a
// latch the host sets where the image runs 009FADA0 (the install and the 009CD182 approach
// update) when the target is kind 5 with +5Dh (state->simulate) set; and the 009FAF90
// drop, modelled by testing the host's +5Eh mirror (scene_destroyed_005e) at every +70h
// reader (st_target_70), which latches it. The host sets +5Eh in its kill-list drain (the
// labelled 009274A1..0092751F substitution at each Kill site), so a reader sees the drop
// from the drain on, as in the image, where no reader runs between the notify and the
// +5Eh store; the latch keeps it when the garrison revive substitution later stores
// all-false flags over +5Eh. Not modelled: 009FAF90's
// +1Ch freeze (the host's aim point follows st_target, labelled at st_refresh_aim_point),
// and slots without scene flags (scene_flags_available false), whose +5Eh stays 0.
// LABELLED, the revive window: the latch is set only when a +70h reader sees +5Eh. During
// the goaway the only reader is the pitch re-plan, so a garrison member that is Killed
// and revived in place (game_hosts_ship_ai.cpp, the 006F3660 substitution, which stores
// all-false flags) within one goaway is never latched and stays a target; in the image the
// revive is a new object and +70h stays 0. Setting the latch from the revive would mean
// walking every strafer's target, so it is not done.
// LABELLED, the install edge: a strafe installed on a kind-2 target that is not kind 5 and
// is already Killed latches at the first reader here, while the image keeps +70h (009FB190
// registers after the notify and nothing re-notifies; 009FADA0 tests kind 5 only).
// Practically unreachable: commands are not issued at Killed targets.
// Pair the flip with S6's kTargetDisabledByte5dCc13Bound: st_ref70_drop_009fada0 reads
// +5Dh through S6's binding (state->simulate) whatever that switch is set to.
// 009CC390 also reads task+44Ch and +468h (009CC3A8, 009CC3B2), but it is dead: undefined
// code with no references.
// Missing: NavPoint targets. Host slots carry vehicle classes, none of them kind 41h, and
// the strafe arm returns at its top without a command target, so +54h stays unset here.
// Movers: a strafe target with +5Ch clear or +5Eh/+60h set before +5Dh (OFF: no target,
// ON: still a target); a target that is not kind 2 (ON: none); a kind-2 target that is
// not kind 5, e.g. a squadron, is dropped only when it is Killed (kill list); during the
// goaway break-off the arm is suspended, so ON keeps a target killed by damage until the
// next approach update, but drops a Killed one at once; and the 007B7870 rocket target
// goes to 0 after the target dies.
inline constexpr bool kStrafeTargetFallback70Cc13Bound = false;

}  // namespace bsp
