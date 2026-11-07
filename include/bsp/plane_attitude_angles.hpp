#pragma once

#include "bsp/plane_advance_pose.hpp"

namespace bsp {

// The bearing a pilot task puts in `plan+2C0h`, measured as the plane's own
// heading `unit+C6Ch` is. The angles themselves (`unit+C64h`, `+C68h`, `+C6Ch`)
// are 007C18B0's, `plane_update_pose_derived_state_007c18b0` in
// `src/cc13_seg47b.cpp` (goal 5 chain 657 retired the 007C1900 fragment copy).
//
// Recovered in `docs/PLANE_ATTITUDE_ANGLES.md` and
// `docs/PILOT_TASK_HEADING_ARM.md`. These three scalars are read all over the
// plane flight and AI code - the control targets read `sin(bank)` and
// `cos(pitch)`, the yaw law reads `cos(bank)`, `|bank|` and the heading - so
// getting a sign wrong here turns every aircraft the wrong way while every
// downstream number still looks as validated as if it were right.

// `009AC190`'s bearing, `Kamikaze/gotowards`'s state tick:
//
//   wrap2pi( pi/2 - atan2(target.z - self.z, target.x - self.x) )
//
// which is the bearing to the target measured the same way `unit+C6Ch` measures
// the plane's own heading, so the planner's `SubtractWrappedAngle(plan+2C0h,
// heading)` is a like-for-like difference.
//
// Two things the recovery establishes that a plausible reimplementation would
// miss. The y components are **not** used - it is a ground-plane bearing. And
// the target position the native reads is a copy **latched once at task
// construction** (`009FB32A`-`009FB345`) rather than the entity's live position:
// nothing in the tick path re-reads the entity, so this arm does not re-derive a
// moving ship's bearing per tick.
float plane_bearing_to_target_009ac190(const float self_position[3],
                                       const float target_position[3]) noexcept;

}  // namespace bsp
