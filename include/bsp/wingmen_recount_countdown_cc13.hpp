#pragma once

// cc13-tranche2 S19 (census row units:9896 "wingmen count recounted at the call, not on
// 007EE790 countdown"). 007B95E0, the leader turn-roll test, reads squadron+3BCh, which
// only 007EE790 writes: +3BCh = 0, then +1 for each member 1..[+3CCh)-1 whose
// [member + [00F876B8]*8 + 9C4h] is non-negative (007EE79B..007EE7DC). Its one caller
// is the squadron tick 007F3BA0, at the countdown 007F3C3F..007F3C5B:
//   step < [+3C4h]:  [+3C4h] -= step                       (007F3D24..007F3D26)
//   otherwise:       [+3C4h] = ([+3C0h] - step) + [+3C4h]  (007F3C47..007F3C55)
//                    then 007EE790(squadron)               (007F3C5B)
// The constructor seeds the period +3C0h with 1.0f and the countdown with -1.0f, so the
// first step recounts (docs/TICK_ELEMENT_OVERRIDES.md item 3); no other writer of +3C0h
// is recorded. Between recounts 007B95E0 answers from the held count.
//
// True: the units host keeps +3C4h and +3BCh per squadron, advances the countdown once
// per fixed step and recounts on expiry with the rule it already uses at the call
// (member_follows_007bcc20). LABELLED: the step runs at the start of the units' fixed
// step, ahead of the planes' ticks (the squadron element's position among the job
// waves is not modelled); the per-member wait value +9C4h stays the host's freshness
// approximation. False: the count is taken at each 007B95E0 call (the old
// substitution). Movers when ON: leader roll timing while wingmen join or leave a
// follow state, up to one period (1 s) late.
//
// CORRECTION (rev4 FIX 1): the constructor does not seed +3C4h with -1.0. 007F2D30
// FLD1 -> [ESP+4] (hi 1.0), 007F2D46 FLDZ -> [ESP] (lo 0), ECX = EDI = 1, 007F2D5B CALL
// 00BD2F10, 007F2D60 FCHS, 007F2D6A FSTP [ESI+3C4h]: the seed is -U(0, 1) on stream 1
// (+3C0h = 1.0 and +3BCh = 0 stand). kSquadronRecountCountdownSeed is only the value
// before the host takes that draw. ON draws it when the units host first sees the
// squadron, at the start of the next fixed step: a STREAM-ORDER MOVER (one stream-1 draw
// per squadron, not at the constructor's place in the sequence).
// CORRECTION (rev4 FIX 2): "ahead of the planes' ticks" is wrong. The squadron node
// registers in tick group 3 (007F2C89 PUSH 3 -> 00875890), units in group 1, and wave 3
// runs groups 0-4 in order, so the countdown runs after every plane's step and
// 007B95E0 reads the previous step's recount. ON runs it at the end of the units' fixed
// step; a squadron created during a step is first ticked the next step. An unordered
// compare (a NaN) takes the t -= step arm, as 007F3C41's JC does.
// LABELLED (rev4 NIT 1): a squadron added between steps by a script or Lua creator
// (scene_contents :2010, script_orders :1108/1219/1376) ticks one step earlier than in
// the image, where 00874C90 splices the node in after wave 3. Scene-load squadrons are
// unaffected.
// LABELLED (rev4 NIT 2): the seed reuses gunnery's Draw::death_mode keyed (leader, 0),
// so under BSP_GUNNERY_RNG_STREAMS=1 it shares a key with the leader's own death-mode
// draw. The default shared stream 1 is unaffected; no dedicated draw kind was added
// (the Draw enum is a cc9 file).

namespace bsp {

// ON since cc9-nondet2 (2026-10-28), jointly with the other six stream-set switches (docs/USN21_NONDETERMINISM.md 8).
inline constexpr bool kWingmenRecountCountdownCc13Bound = true;
inline constexpr float kSquadronRecountPeriod3c0 = 1.0f;     // ctor seed, +3C0h
inline constexpr float kSquadronRecountCountdownSeed = -1.0f; // ctor seed, +3C4h

}  // namespace bsp
