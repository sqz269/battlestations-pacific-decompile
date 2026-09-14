#pragma once

#include "bsp/gun_aiming.hpp"

namespace bsp {

// 007F6190, body 007F6190-007F64EB, reached through 0085AB50 (RET 8, body
// 0085AB50-0085AB9A) which bounds-checks the platform index and delegates at
// 0085AB91. 008FFF20 step 8 calls it at 00900380 with limit = pi/4.
//
// The native does NOT refuse a commanded heading that falls outside a firing
// window: it SNAPS the heading up to `limit` onto the nearest firing-window
// edge and fires along that edge, abandoning the shot only when no window's
// horizontal bounds hold the heading at all, or when the snap would exceed the
// limit. Failure is signalled by FLT_MAX (the float at 00D7A278), which
// 00900392..009003A2 tests against.
//
// The rule, from docs/TORPEDO_LAUNCH_ACCURACY.md:
//
//   find the first arc record whose HORIZONTAL bounds hold `horz` (clamped to
//     +/-pi, widened by 00D08B88 = half a degree). No flag test, no vertical
//     test.
//   none                          -> FLT_MAX
//   that record carries the fire bit -> `horz` unchanged
//   otherwise: walk backwards, then forwards, through records that carry the
//     traverse bit; at the first record on each side that also carries the fire
//     bit, take its nearer bound (min_horz or max_horz) by wrapped angular
//     distance; keep the nearer of the two candidates.
//     none, or |wrapped(candidate - horz)| > limit -> FLT_MAX
//     else                                         -> candidate
//
// Hypothesis, not a recovered symbol; the arithmetic is recovered, the name is
// descriptive. Reconstructed and build-tested, not ABI-compatible.

// 00D08B88, the half-degree widening applied to a window's horizontal bounds
// before the containment test.
inline constexpr float kGunHeadingSnapBoundEpsilonRadians = 0.008726646f;

// The sentinel 007F6190 returns when it abandons the shot: the float at
// 00D7A278. Callers compare against it rather than using a boolean.
float gun_heading_snap_failure_value() noexcept;

// True when `value` is that sentinel.
bool gun_heading_snap_failed(float value) noexcept;

// 007F6190. Returns the heading to command, or the failure sentinel.
float gun_snap_heading_to_fire_window_007f6190(const GunPlatformArcs& arcs,
                                               float horz_radians,
                                               float limit_radians) noexcept;

}  // namespace bsp
