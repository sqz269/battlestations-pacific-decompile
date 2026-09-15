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
// before the polynomial, and the three are symmetric: pitch is
// `class+1C0h * modeFactor` (007DAA7F, the FMUL direction settled from the bytes
// D8 C9), yaw is `class+1C4h * -modeFactor` (007DAAA1), roll is
// `class+1BCh * modeFactor` (007DAAB3). Only the magnitude is used.
//
// An earlier revision of this comment gave roll as the PITCH product times
// class+1BCh and called the asymmetry surprising. That was an arithmetic slip:
// 007DAA9D's FSTP stores and pops, so ST0 is the mode factor again, not the
// product. There is no asymmetry to explain.
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


// ---------------------------------------------------------------------------
// The three targets 007DA710 drives the axes toward
// ---------------------------------------------------------------------------
//
// Recovered in docs/PLANE_CONTROL_TARGETS.md (packet cc7_plane_control_targets),
// which carries the address-by-address evidence. What follows is that doc's
// result, with the gated terms this host cannot yet supply marked in the inputs
// rather than dropped silently.
//
// Everything the rule reads off the plane is a field of the **unit**, reached
// through ctl+8h at 007DA72D - not of the flight controller. Only the three
// axes the law writes are controller-relative. An earlier revision of
// docs/PLANE_CONTROL_RATE_LAW.md had the whole latched block under a `ctl+`
// heading, which is wrong by one indirection.

// Class descriptor fields, all from the plane row of vehicleclasses.lua.
struct PlaneControlClass {
    float roll_spd = 0.0f;              // +1A8h RollSpd
    float pitch_spd = 0.0f;             // +1ACh PitchSpd
    float yaw_spd = 0.0f;               // +1B0h YawSpd
    float yaw_roll_ratio = 0.0f;        // +1B4h YawRollRatio
    float slide_ratio = 0.0f;           // +1B8h SlideRatio
    float roll_accel = 0.0f;            // +1BCh RollAccel
    float pitch_accel = 0.0f;           // +1C0h PitchAccel
    float yaw_accel = 0.0f;             // +1C4h YawAccel
    float negative_pitch_ratio = 0.0f;  // +1D8h NegativePitchRatio
};

// The unit state the target expressions read.
struct PlaneControlUnitState {
    float latched_yaw = 0.0f;    // unit+BB0h, latched from +9E4h by 007B9770
    float latched_pitch = 0.0f;  // unit+BB4h, from +9E8h
    float latched_roll = 0.0f;   // unit+BB8h, from +9ECh
    // unit+BC4h. BSP_Plane_ReadPropertyBag sets it to 1.0f and the rate law
    // maintains it there, relaxing toward 1.0 by 0.5*step; it multiplies into
    // the yaw raw product at 007DA94B.
    float yaw_scale_bc4 = 1.0f;
    // unit+838h, added to the latched roll at 007DA738 to form the roll base.
    // NOT identified: no reader in this repository names it for a plane, and
    // docs/PLANE_CONTROL_TARGETS.md leaves its provenance open. Zero here is a
    // stand-in, not a recovered default.
    float roll_base_838 = 0.0f;
    // unit+C64h pitch angle and unit+C68h bank angle, the two the slide term
    // and the bank-angle roll factor read through FCOS/FSIN. This host does not
    // maintain them, so they are zero and the slide term vanishes; that is a
    // modelling gap, recorded rather than papered over.
    float pitch_angle_c64 = 0.0f;
    float bank_angle_c68 = 0.0f;
    // unit+900h. 6 is the state that engages the bank-angle roll factor; free
    // flight is 7, so the factor is 1.0 in the air.
    int flight_state_900 = 7;
    // ctl+FCh, the controller mode - the one controller field here. 1 is on the
    // ground and zeroes the roll target at 007DA8E5. 007DC841 zeroes the field
    // every step, so free flight is 0.
    int controller_mode_fc = 0;
    // unit+5Dh out of action, and the spin block it gates (unit+C36h present,
    // unit+C37h sign, unit+C3Ch the lost-drag timer). Unmodelled here: an
    // undamaged plane takes none of it.
    bool out_of_action_5d = false;
};

// The three targets, and the three acceleration terms the rate law pairs with
// them. `mode_factor_f1` and `mode_factor_f2` are 007DA380's two float outputs;
// in free flight both are BSP_PlaneFlight_ControlAuthority's return
// (docs/PLANE_CONTROL_AUTHORITY.md), which is why one value fills both.
struct PlaneControlTargets {
    float target[3] = {0.0f, 0.0f, 0.0f};  // pitch, yaw, roll - the ctl+48h/4Ch/50h order
    float accel[3] = {0.0f, 0.0f, 0.0f};
};

PlaneControlTargets plane_control_targets_007da710(const PlaneControlClass& cls,
                                                   const PlaneControlUnitState& unit,
                                                   float mode_factor_f1,
                                                   float mode_factor_f2,
                                                   bool slide_and_coupling_flag) noexcept;

}  // namespace bsp
