#pragma once

namespace bsp {

// 007DA710 BSP_PlaneFlight_ControlRateLaw, `void __thiscall(this, float step)`,
// RET 4. `this` is the plane controller; the three body angular velocity
// components it drives are `this+48h` pitch, `this+4Ch` yaw, `this+50h` roll -
// the same offsets `include/bsp/plane_angular_velocity.hpp` records as
// `kAngularBody`, so this law IS what fills the body angular velocity that
// 007D9C80 then rotates into world.
//
// The function runs the identical rule three times, once per axis: the same
// polynomial globals are read at 007DABB2/007DAD4E/007DAED0 and the same clamp
// shape appears at 007DACE0, 007DAE62 and in the third block. What differs per
// axis is the target, the current value and one acceleration term.
//
// The full derivation, address by address, is docs/PLANE_CONTROL_RATE_LAW.md.
// Two things in it are worth repeating here because getting either wrong
// produces code that compiles, runs, and is silently wrong:
//
// * **The clamp is not a magnitude bound.** It is the interval between the
//   current value and the target (007DAC59..007DAC9E build min and max of that
//   pair), so the law can never overshoot the target in one step and never
//   moves away from where it already is. An earlier revision of the doc
//   published `clamp(..., -bound, +bound)`; there is no such bound anywhere in
//   the function.
// * **The polynomial coefficients are not in the executable.** They are read
//   from three globals at 00F872FC/00F87300/00F87304 which are past `.data`'s
//   file-backed extent - zero-filled at load - and written at runtime by a
//   single `REP MOVSD` at 007EAAE1 that copies 78 dwords from the game tuning
//   singleton. Reading them statically gives 0.0 and means nothing.
//   `Dynamics/RotationFactors/{A,B,C}` in planeglobals.lua is where the values
//   live: see PlaneRotationFactors below.
//
// The rule, per axis:
//
//   delta = target - current                                007DAB52..007DAB75
//   rate  = (A*delta^2 + B*|delta| + C) * |accel|            007DABB2..007DABE0
//   if (!flag)                     rate = max(rate, 0.15)    007DAC48..007DAC53
//   else if (!same_sign(delta, current))
//                                  rate = max(rate, 1.5*|current|)
//                                                           007DABEA..007DAC28
//   cand  = current + sign(delta) * rate * step              007DAC9B..007DACD4
//   new   = clamp(cand, min(current, target), max(current, target))
//                                                           007DACE0..007DAD01
//
// `flag` is the third output of 007DA380
// BSP_PlaneFlight_ControllerModeFactors, which switches on the controller mode
// at `ctl+FCh`. That field is zeroed every step at 007DC841, so **a plane in
// free flight always takes mode 0**, whose arm (007DA6E6, ten instructions)
// sets the flag to 1. The 0.15 idle floor therefore never applies in the air;
// only the sign-guarded floor does. Callers that are modelling free flight
// should pass `flag = true`.

// Dynamics/RotationFactors, mirrored into 00F872FC/00F87300/00F87304 by the
// REP MOVSD at 007EAAE1. The defaults are this installation's
// scripts/datatables/planeglobals.lua. That file is NOT part of the untouched
// install - 29 of the 38 files in scripts/datatables carry the install date and
// this one does not - so treat these as this installation's values rather than
// as provably retail. One thing argues they were never retuned: the
// reverse-crossing floor compiled into the executable is 1.5*|current|, the
// same coefficient as B applied to the deflection instead of the error, and a
// modder changing B would have no reason to match a constant inside the binary.
struct PlaneRotationFactors {
    float a = 0.5f;  // Dynamics/RotationFactors/A, class block +21Ch
    float b = 1.5f;  // Dynamics/RotationFactors/B, class block +220h
    float c = 0.1f;  // Dynamics/RotationFactors/C, class block +224h

    // 00CE3D30 = 3F19999Ah, in .rdata and therefore a real compiled constant.
    float idle_floor = 0.15f;
    // 00CE3D78 = 1.5 as a double, likewise .rdata.
    float crossing_gain = 1.5f;
};

// One axis's inputs. `accel` is the per-axis acceleration term the law forms
// before the polynomial - pitch is `class+1C0h * modeFactor` (007DAA7F, the
// FMUL direction settled from the bytes D8 C9), yaw is `class+1C4h *
// -modeFactor` (007DAAA1), roll is the pitch product times `class+1BCh`
// (007DAAB3). Only its magnitude is used.
struct PlaneControlAxisState {
    float current = 0.0f;
    float target = 0.0f;
    float accel = 0.0f;
};

// One axis of 007DA710's rate law. Returns the value the function would store
// to `this+48h`, `this+4Ch` or `this+50h`.
//
// `floor_from_deflection` is 007DA380's byte output: true selects the
// sign-guarded `crossing_gain * |current|` floor, false the flat `idle_floor`.
// Free flight is always true (see the header comment above).
//
// The branch structure is reproduced rather than simplified, so the NaN
// behaviour of the original survives: an x87 `FCOMIP` on an unordered pair sets
// CF, which sends both `JBE`s in the clamp down their taken arms.
float plane_control_axis_step_007da710(const PlaneRotationFactors& factors,
                                       const PlaneControlAxisState& axis,
                                       bool floor_from_deflection,
                                       float step) noexcept;

}  // namespace bsp
