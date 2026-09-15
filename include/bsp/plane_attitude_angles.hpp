#pragma once

#include "bsp/plane_advance_pose.hpp"

namespace bsp {

// The three attitude angles a plane keeps at `unit+C64h`, `+C68h` and `+C6Ch`,
// and the bearing a pilot task puts in `plan+2C0h`.
//
// Recovered in `docs/PLANE_ATTITUDE_ANGLES.md` and
// `docs/PILOT_TASK_HEADING_ARM.md`. These three scalars are read all over the
// plane flight and AI code - the control targets read `sin(bank)` and
// `cos(pitch)`, the yaw law reads `cos(bank)`, `|bank|` and the heading - so
// getting a sign wrong here turns every aircraft the wrong way while every
// downstream number still looks as validated as if it were right.

struct PlaneAttitudeAngles {
    float pitch = 0.0f;    // unit+C64h, [-pi/2, +pi/2], positive = nose up
    float heading = 0.0f;  // unit+C6Ch, (-pi, pi]
    float bank = 0.0f;     // unit+C68h, (-pi, pi]
    // False when the forward axis is too near vertical for a heading to exist:
    // the native's two guards, |fwd x up|^2 <= 1.0842e-10 (00CE3820) and
    // |fwd x up| <= 0.001f (00D7A23C), both branch past the heading and bank
    // stores and leave the previous values in place. The pitch is always
    // written.
    bool heading_and_bank_written = false;
};

// `007C1900`-`007C1ACA`, the whole derivation, from the plane's live world pose.
//
// It is reproduced through the same primitives the native calls rather than
// reduced to closed form. The pitch and heading do have closed forms - the doc
// proves `heading == atan2(fwd.x, fwd.z)` - but the **bank** falls out of a
// two-rotation chain (lay the forward axis flat by the pitch, then rotate the
// heading out with a RotationY) whose residual is read straight off the matrix,
// and deriving that by hand is exactly the kind of step where a sign goes
// missing. `ops` supplies the three recovered matrix primitives.
PlaneAttitudeAngles plane_attitude_angles_007c1900(const AdvanceMatrix& world_pose,
                                                   AdvanceMatrixOps& ops) noexcept;

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
