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
//         So for a unit target +70h holds the target until the first approach update
//         that sees its +5Dh set, then stays 0.
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
// update) when the target is kind 5 with +5Dh (state->simulate) set.
// Missing: NavPoint targets. Host slots carry vehicle classes, none of them kind 41h, and
// the strafe arm returns at its top without a command target, so +54h stays unset here.
// Movers: a strafe target with +5Ch clear or +5Eh/+60h set before +5Dh (OFF: no target,
// ON: still a target); a target that is not kind 2 (ON: none); a kind-2 target that is
// not kind 5, e.g. a squadron (never dropped ON); during the
// goaway break-off the arm is suspended, so ON keeps the target there until the next
// approach update; and the 007B7870 rocket target goes to 0 after the target dies.
inline constexpr bool kStrafeTargetFallback70Cc13Bound = false;

}  // namespace bsp
