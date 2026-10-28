#pragma once

// Packet cc13-strafeaimfreeze (cc9 item 5, the strafe70 residual). Names are hypotheses,
// not recovered symbols.

#include "bsp/strafe_target_fallback_70_cc13.hpp"

namespace bsp {

// kStrafeAimPointFreezeCc13Bound (OFF): the strafe approach's aim point, the target ref's
// sub+1Ch (the ref at approach+58h; 009CA680, the approach's vt[0], reads it), once the
// ref has let go of its target.
// Image:
//   009FADA0, the ref's per-update tick (009CCED0's first call, 009CCEE1): while sub+18h
//     is set it rewrites +1Ch = target+CCh x (+28h body offset) (009FAEDF 004142E0) and
//     the lead tail. At its head (009FADA6..009FAE11), with sub+18h and sub+14h set
//     (sub+14h is the target only when it answered IsKindOf(5) at the bind) and
//     [sub+14h]+5Dh set, it unregisters, stores +1Ch = target+CCh x (+28h) with no lead
//     and no re-pick, and clears +14h/+18h/+41h. Every later call sees +18h null and
//     does nothing, so +1Ch stays FROZEN.
//   009FAF90, observer slot +4h (the kill-list notify, row-15 drain or 009263C0): when
//     the notifier is sub+18h it stores +1Ch = target+CCh x (+1Ch) (009FAFC6 LEA EDI,
//     [ESI+1Ch]; MOV ECX,EDI; 009FAFCC CALL 004142E0, ECX = the source), the world point
//     pushed through the dying matrix a second time as if it were a body offset, and
//     clears +40h/+18h/+14h/+41h. It runs only when the notify reaches a ref that
//     009FADA0 has not already dropped (that drop unregisters it): a direct Kill, a wreck
//     reaching KillDepth while the strafer is in a goaway manoeuvre (009FADA0 is not
//     called there), or +5Dh and +5Eh in the same flush.
// Host OFF: st_refresh_aim_point recomputes the point from the commanded target on every
// approach update (aim_point_009fada0, the hull point with the lead), whatever the
// +70h drop latch (st_ref70_dropped) and the target's +5Dh/+5Eh say.
// ON, in st_refresh_aim_point, in this order:
//   1. st_ref70_dropped already latched: the point is left as it is (frozen).
//   2. The 009FAF90 path: the target's +5Eh mirror (scene_destroyed_005e) is set and it
//      is kind 2 (the ref holds only a kind-2 target, 009FB200): point = target.world x
//      point, then latch. LABELLED: applied at the next approach update with the
//      matrix the host has then, not at the drain.
//   3. The 009FADA0 head drop: the target is kind 2 and kind 5 (sub+14h set) and its
//      +5Dh is set (state->simulate): point = target.world x hull_aim_ref.body_offset_28,
//      no lead, then latch. LABELLED: a target that does not sample the hull has no
//      host +28h; its point is the target's position, as the OFF refresh gives it.
//   4. Otherwise the OFF refresh.
// RNG: none either way (the refresh re-picks only on a new target, the lead draws nothing).
// Movers: strafing planes between their target's death and the range test's next run;
// small against a ship shot to death (path 3: only the lead goes), possibly large where
// path 2 runs. The ON-only summary line counts each path.
// Open (out of scope): the dive-bomb and torpedo approach refs embed the same sub-object
// and its 009FAF90; whether their hosts freeze their aim points is not read here.
inline constexpr bool kStrafeAimPointFreezeCc13Bound = false;
static_assert(!kStrafeAimPointFreezeCc13Bound || kStrafeTargetFallback70Cc13Bound,
              "the freeze keys off strafe70's +70h drop latch");

}  // namespace bsp
