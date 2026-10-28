#pragma once

// cc13-tranche2 S22 (census row units:27820 "near-field probe scans all live aircraft
// directly, no 3 s list refresh lag").
//
// 007F0280's candidate walk (ECX = squadron, ESI = the probing plane), mode byte [ESP+138h]:
//   007F038D..007F03AB  mode set and [plane+C50h] non-null: walk the object's +30h vector
//                       (007F03B1 EBP = [C50h+30h] .. [C50h+34h]); keep a member answering
//                       vtable[5Ch](0Fh) (007F03D8). No live test, no distance test.
//   007F0481..          otherwise (mode clear, or no C50h object): the squadron's +3D0h
//                       members for +3CCh, less the plane itself (007F049A).
// The +30h vector is written only by 007E11D0 (drop at or beyond R = 1080, then append
// every world entity answering IsKindOf(6) or (0Fh) within R), and 007E11D0 runs only from
// 007E2010, the object's tick slot +0Ch (vtable 00D06B88, slot 3 at 00D06B94), when the
// clock +7Ch reaches the period +88h = 3.0 (then +7Ch -= 3.0). The object is a tick
// element: it advances with the fixed step, not with the pilot's think. Its constructor
// 007E1E20 seeds +7Ch = U(0, 3) + 3 from 00BD2F10 with ECX = 1 (stream 1, 007E1F94), so the
// first tick refreshes and the phase after it is the draw; that draw is already bound
// (kNeighbourClockDrawsBound, keyed `name#nb7c`), this packet takes no new draw. The
// object exists only when [game+1FE4h] is 0 or 1 and unit+54h != [[game]+5FCh]+908h
// (007D61C0..007D621F), the gate 009A184A applies too (kSceneDefaultDummyAi stands in).
//
// True:
//  * the units host advances every plane's +30h copy (nb_near_30 / nb_clock_30) once per
//    fixed step and refreshes it on the 3 s clock, instead of inside 007DF4F0's think;
//  * 007F0280 mode 1 walks nb_near_30 (kind 0Fh members still in the world), and a plane
//    with no C50h object takes the squadron arm, as 007F03AB's JZ does.
// LABELLED: the pass runs at the start of the units' fixed step; "removed by its observer"
// is a slot no longer in the world (wrecks stay listed); the probe keeps its reach
// prefilter. False: the probe scans every live aircraft each call and 007DF4F0 refreshes
// the list on its think.
// Movers when ON: near-field avoidance offsets (the level-bomb run offset, the dive
// flyabove avoidance - S7's kDiveFlyaboveAvoidIncrementCc13Bound reads this probe - and
// the other mode-1 callers) see falling wrecks until removal and miss a plane that came
// within R since the last refresh; 007DF4F0's vehicle avoidance refreshes on the fixed
// step clock.
//
// CORRECTION (rev7 S22 FIX + NITs):
//  * Mover: 007DF360 target_reachable_007df360 (game_hosts_units.cpp, under
//    kTorpedoLineOfSightBound, ON on main) walks the same nb_near_30, as the image's
//    007DF360 walks [C50h+30h]. With this switch ON that list refreshes every fixed step,
//    so torpedo +A9h reachability answers move when it flips.
//  * Mover: with kStreamSet2SharedDrawsCc13Bound also ON, nb_seed_clocks' stream-1 draws
//    happen at the first fixed step for every C50h plane (not at that plane's first
//    think), which reorders the shared stream.
//  * Mover, the default case (rev7 S22 FIX 2): with this switch ON alone (and
//    BSP_GUNNERY_RNG_STREAMS unset), nb_seed_clocks -> stream_set2_draw_00bd2f10<false>
//    -> release_altitude_draw_00bd2f10 draws from the shared, unkeyed host state
//    db_release_rng, which every release_altitude_draw_00bd2f10 caller shares (it is not
//    gunnery's stream 1). The first seed moves from a plane's first neighbour think to
//    the first fixed step, for every in-world C50h plane, including planes that never
//    think neighbours, so db_release_rng draws are reordered and added.
//  * Label wording: 007E2010 is a plane sub-node (007E1E20 -> 00876020 Reparent, +18h = 2,
//    under unit+310h), so it runs in wave 2, after wave 1's pose step and before the wave-3
//    consumers. There is no step lead; the only difference is that the host pass measures
//    before the pose move, the existing kPlanePoseFirstBound substitution. The pass now
//    runs before S19's squadron seed, as the plane sub-nodes run before the group-3
//    squadron node.
//  * LABELLED split: the image's single +7Ch clock refreshes +30h, +50h and +70h together,
//    and 007E2010's early return clears all three. Under ON only the +30h copy moves to
//    the fixed step; nb_clock_70 (friendly_in_line_clock_007e2010, with +80h) and the +50h
//    list stay on the think.
//  * The OFF arm of 007F0280's walk is the pre-S22 loop verbatim (no candidate copy).

namespace bsp {

inline constexpr bool kNearFieldProbeCachedListCc13Bound = false;

}  // namespace bsp
