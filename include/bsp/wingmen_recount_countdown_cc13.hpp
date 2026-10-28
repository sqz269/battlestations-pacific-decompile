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

namespace bsp {

inline constexpr bool kWingmenRecountCountdownCc13Bound = false;
inline constexpr float kSquadronRecountPeriod3c0 = 1.0f;     // ctor seed, +3C0h
inline constexpr float kSquadronRecountCountdownSeed = -1.0f; // ctor seed, +3C4h

}  // namespace bsp
