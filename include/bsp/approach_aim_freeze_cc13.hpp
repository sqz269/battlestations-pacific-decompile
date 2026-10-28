#pragma once

// Packet cc13-aimfreeze3: the target ref's death freeze for the dive-bomb, level-bomb and
// torpedo approaches. All three embed the one sub-object (vtable 00D21CB4): dive and level
// bomb at approach+30h (009C7A9F, 009B7CA1), torpedo at approach+B4h (009D34BB). Its
// per-tick 009FADA0 and its observer slot +4h 009FAF90 are shared; there are no per-task
// copies.
//
// Image.
//   009FADAE..009FAE18, the death head: with sub+18h, sub+14h and [sub+14h]+5Dh set, the
//   observer is unregistered (006952A0), the pose refreshed, sub+1Ch = M(+CCh) x sub+28h
//   with NO projtime lead (the tail 009FAF05 is not reached), and sub+14h, sub+18h and
//   sub+41h are cleared (sub+40h is not). The point is frozen from then on: the per-tick
//   body needs sub+18h, so there is no re-pick and no lead.
//   009FAF90, the observer slot: when the kill list's drain notifies before any tick saw
//   +5Dh, sub+1Ch = M x sub+1Ch (the world point transformed a second time) and sub+40h,
//   sub+18h, sub+14h and sub+41h are cleared.
//   Each approach then tests sub+18h right after its 009FADA0 call and leaves early:
//     dive    009C7B03 CMP [ESI+48h],0 / JZ 009C7E91: +D0h = 0, return
//     level   009B7CF3 CMP [ESI+48h],0 / JZ 009B80E9: +CDh = 0, return
//     torpedo 009D34E3 CMP [ESI+CCh],0 / JZ 009D3E21: +131h = 0, +134h += dt, return
//   and approach->vtable[0] keeps answering the frozen sub+1Ch to the task's states (dive
//   flyabove 009C6342, aimdive 009C59CD / 009C5DF1, aimglide 009C5278, goaway 009C47D0;
//   level goaway 009B591B; torpedo goaway 009D0670).
//
// Host (OFF). GameUnitsHost::Impl::aim_point_009fada0 recomputes M(live pose) x the body
// offset and adds the lead at every call, with no +5Dh test, until the squadron retest
// (009F8412) clears the unit's target; the getters then fall back to the unit's own
// position (dive) or have no point (torpedo goaway).
//
// ON. aim_point_freeze_009fada0 wraps aim_point_009fada0 for the callers below only; the
// wrapped function itself is unchanged for every caller (strafe's st_refresh_aim_point
// keeps its own flag, kStrafeAimPointFreezeCc13Bound). The latch is the ref's own
// `frozen` / `tracking` pair (sub+18h cleared), with the frozen point in world_point_1c;
// hull_aim_world_point's re-construction on a new target_plus_one resets it.
//   +5Dh (state->simulate) first seen on the bound ref: 009FADAE's freeze, no lead.
//   +5Eh (scene_destroyed_005e) already set at that first sight: 009FAF90's double
//   transform of the last point returned. LABELLED, reach unverified: the host observes
//   the flags once per call, so the notify order is inferred from which flag it sees first,
//   and the matrix is the target's pose at that call, not at the notify.
//   LABELLED: the host keeps one ref per shooter rather than one per approach, so the latch
//   is tagged with the task that set it and answers only that task's callers.
//
// Expected movers when ON: after a target's +5Dh, dive and torpedo in-range bytes drop at
// once (dive +D0h, torpedo +131h) and the rest of each approach body stops; dive flyabove /
// aimdive / aimglide / goaway-turn heights and bearings and the torpedo goaway steer to the
// frozen death point instead of the live sinking hull plus lead, and keep it after the
// target is cleared; the level-bomb lb_aim (break-off distance, goaway retreat pitch) holds
// the death point. Size set by the window between +5Dh and the squadron retest's clear.
// Census: "summary aim freeze" (ON only).

namespace bsp {

// Dive bomb, approach+30h: update_dive_bomb_approach's 009C7B03 early-out and the
// approach->vtable[0] readers (flyabove, aimdive error and tick, aimglide, goaway turn).
inline constexpr bool kDiveAimPointFreezeCc13Bound = false;
// Level bomb, approach+30h: lb_approach_update_009b7c90's lb_aim. has_target is unchanged
// (df_slot_live already drops the target on +5Dh).
inline constexpr bool kLevelBombAimPointFreezeCc13Bound = false;
// Torpedo, approach+B4h: approach_target_point answers false once latched (the 009D3E21
// arm in torpedo_approach_update.cpp), and the goaway reads the frozen point.
inline constexpr bool kTorpedoAimPointFreezeCc13Bound = false;

}  // namespace bsp
