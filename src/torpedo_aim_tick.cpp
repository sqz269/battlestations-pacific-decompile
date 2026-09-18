// 009D15F0-009D2377, the torpedo bot's `aim` state tick.
// Evidence, frame table and uncertainty: docs/TORPEDO_AIM_TICK.md.
//
// Every stack slot below is named F=<offset>, the offset from the deepest
// prologue ESP.  The mapping was proven by a CFG fixpoint over the function's
// own graph: 870/870 instructions reached, 0 depth conflicts at any join.

#include "bsp/torpedo_aim_tick.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"

namespace bsp {
namespace {

using namespace torpedo_aim;

// 009D17E8-009D188B and 009D1858-009D18A6 both compile a two-sided clamp as
// "if (lo > v) v = lo; else if (v > hi) v = hi".  With lo <= hi that is a
// clamp; the helper keeps the listing's order so a NaN takes the same arm.
float clamp_listing(float v, float lo, float hi) noexcept {
    if (!(lo <= v)) return lo;      // 009D17F2 JBE / 009D1862 JBE
    if (v <= hi) return v;          // 009D187F JBE / 009D18A1 JBE
    return hi;
}

// The bit-level absolute value the tick uses at 009D1746, 009D18EC, 009D1AB2,
// 009D1BAA and 009D1FBF: MOVSS to memory, AND 7FFFFFFFh, reload.
float abs_bits(float v) noexcept {
    return std::fabs(v);
}

}  // namespace

// ---------------------------------------------------------------------------
float torpedo_aim_altitude_floor_009d1613(bool over_land_a9, float alt_74,
                                          float alt_78,
                                          float ground_height) noexcept {
    // 009D1613 CMP byte [EDI+A9h]; 009D1635/009D163F pick the constant.
    float floor_value = over_land_a9 ? kAltFloorOverLand : kAltFloorOverSea;
    // 009D1647 FLD [EDI+78h]; 009D1650 FADD [EDI+74h]; 009D1653 -> F=28h.
    const float pair = alt_78 + alt_74;
    // 009D165B/009D165F/009D1663: JBE skips, so the sum wins only when strictly
    // greater.  009D1673 stores the winner back into F=28h.
    if (pair > floor_value) floor_value = pair;
    // 009D16D6 FLD F=24h (the 00903860 out-slot); 009D16DA FADD 5.0 -> F=30h.
    const float ground_plus = ground_height + kGroundClearance;
    // 009D16E4/009D16EC/009D16F0: same shape, 009D16F8 stores into F=28h.
    if (ground_plus > floor_value) floor_value = ground_plus;
    return floor_value;
}

// ---------------------------------------------------------------------------
float torpedo_aim_release_threshold_009d1ded(float unit_altitude,
                                             float altitude_floor,
                                             float turn_magnitude) noexcept {
    // 009D1D54 FLD [EDI+100h]; 009D1D5A FSUB F=28h; 009D1D5E -> F=48h.
    float margin = unit_altitude - altitude_floor;
    // 009D1D62 FLD F=18h; 009D1D66 FMUL 1.6; 009D1D6C FLD1; 009D1D6E FSUBRP.
    float turn_factor = 1.0f - turn_magnitude * kTurnFactorSlope;
    // 009D1D78/009D1D7E/009D1D82: max(margin, -0.5).
    if (!(kAltMarginFloor <= margin)) margin = kAltMarginFloor;
    // 009D1D9E/009D1DA4/009D1DA8: max(turn_factor, 0.1).
    if (!(kTurnFactorFloor <= turn_factor)) turn_factor = kTurnFactorFloor;
    // 009D1DBA FLD F=48h; 009D1DD2 FMUL F=40h; 009D1DED FSTP F=34h.
    return margin * turn_factor;
}

// ---------------------------------------------------------------------------
bool torpedo_aim_complete_009d22ff(float release_threshold, float range,
                                   float turn_magnitude, float time_to_target,
                                   float commanded_speed) noexcept {
    // 009D2320 FLD F=48h (the speed); 009D2324 FMUL double 0.5; 009D232A back
    // to float, then pushed as the fourth argument at 009D2332.
    const float half_speed =
        static_cast<float>(static_cast<double>(commanded_speed) * kRampHalf);
    // 009D2336 FLD1 -> x1, 009D233C FLDZ -> x0 and y0, 009D2318 -> x = F=0Ch.
    const float ramp = clamped_interpolate_00419010(0.0f, 0.0f, 1.0f,
                                                    half_speed, time_to_target);
    // 009D234A FSUBR double F=40h: (double)range - ramp, stored back to float.
    const float remaining = range - ramp;
    // 009D2356 FLD double F=34h; 009D235A FCOMIP ST0,ST1; 009D235E JA sets.
    if (release_threshold > remaining) return true;
    // 009D2360 FLD F=0Ch; 009D2364 FLD F=18h; 009D2368 FCOMIP; 009D236C JBE
    // skips, so the byte is set when the turn magnitude is strictly greater.
    return turn_magnitude > time_to_target;
}

// ---------------------------------------------------------------------------
bool torpedo_aim_solution_009d2021(float target_aspect, float approach_84,
                                   float commanded_speed,
                                   float range) noexcept {
    // 009D1FA3 FCOS on F=2Ch; 009D1FBF the bit-level absolute value.
    const float aspect = abs_bits(std::cos(target_aspect));
    // 009D1FED: x0 = 0.5, y0 = 1.0, x1 = 1.0, y1 = approach+84h, x = |cos|.
    const float shaped = clamped_interpolate_00419010(kAspectX0, kAspectY0,
                                                      kAspectX1, approach_84,
                                                      aspect);
    // 009D1FF2 FMUL F=20h (the speed); 009D2002 FADD 200.0; 009D2008 compare.
    const float envelope = shaped * commanded_speed;
    return static_cast<float>(static_cast<double>(envelope) +
                              kSolutionBias) > range;
}

// ---------------------------------------------------------------------------
TorpedoAimTickResult torpedo_aim_tick_full_009d15f0(TorpedoAimTickHost& host,
                                                    const TorpedoAimTickState& in,
                                                    float dt) noexcept {
    TorpedoAimTickResult out;

    // --- 009D15F0-009D1673, the entry block: dominates every later read. ---
    float f14_range = in.range_90;                       // 009D1604
    float f0c_time = host.time_to_target_009d1500();     // 009D160A/009D160F
    const float f1c_bearing = in.bearing_94;             // 009D1622
    float f18 = in.turn_offset_5c;                       // 009D162D
    const float f20_heading = host.unit_heading_vtable50();  // 009D1679

    // --- 009D1683-009D16F8, the bearing error and the altitude floor. ---
    float f10_turn = wrapped_angle_subtract_00438b10(f1c_bearing, f20_heading);
    host.refresh_world_pose_00414db0(in.pose_dirty_c8);  // 009D16B4
    float f2c = 0.0f;                                    // 009D16A7/009D16AA
    const float ground = host.ground_height_00903860();  // 009D16D1
    const float f28_floor = torpedo_aim_altitude_floor_009d1613(
        in.over_land_a9, in.alt_floor_74, in.alt_margin_78, ground);
    out.altitude_floor_f28 = f28_floor;

    // --- 009D16FE-009D174C, the target's aspect angle into F=2Ch. ---
    if (host.has_target_cc() &&
        host.target_is_kind_vtable5c(kTargetKindProbe)) {   // 009D1709/009D1718
        const float th = host.target_heading_vtable50();     // 009D1721
        // 009D1732 pushes F=2Ch first, 009D172E pushes F=20h second, so the
        // first argument is the target heading.
        f2c = abs_bits(wrapped_angle_subtract_00438b10(th, f20_heading));
    }

    // --- 009D1750-009D178A, the 0.9 tighten for two unit kinds. ---
    if (host.unit_is_kind_vtable5c(kTightenKindA) ||
        host.unit_is_kind_vtable5c(kTightenKindB)) {         // 009D175F/009D176E
        // 009D177E is DC C9 (FMUL ST(1),ST(0)), so both slots are scaled.
        f14_range = f14_range * kTightenScale;               // 009D1782
        f0c_time = kTightenScale * f0c_time;                 // 009D178A
    }

    // --- 009D178E-009D18C9, the sector turn, only when +5Ch != 0. ---
    float f1c_command = 0.0f;
    if (f18 != 0.0f) {                                       // 009D1794 UCOMISS
        const float gain = clamped_interpolate_00419010(
            kSectorGainX0, kSectorGainY0, kSectorGainX1, kSectorGainY1,
            f0c_time);                                       // 009D17D7
        f18 = gain * f18;                                    // 009D17DC
        const float shaped = clamp_listing(f18, kSectorClampLo, kSectorClampHi);
        const float lead = clamped_interpolate_00419010(
            kLeadGainX0, 0.0f, kLeadGainX1, 1.0f, f0c_time); // 009D182C
        f10_turn = wrapped_angle_add_00438aa0(f10_turn, lead * shaped);
        f10_turn = clamp_listing(f10_turn, kTurnClampLo, kTurnClampHi);
        f1c_command = wrapped_angle_add_00438aa0(f20_heading, f10_turn);
    }

    // --- 009D18CD-009D18F5, the speed switch and |turn|. ---
    const float speed = torpedo_commanded_speed_009d3c99(
        in.elapsed_134, in.speed_late_7c, in.speed_early_80);  // 009D18F9
    float f18_turn_mag = abs_bits(f10_turn);                   // 009D18F5

    // --- 009D190A-009D19A0, the turn-radius escape. ---
    const float capped =
        (f18_turn_mag > kTurnSinCap) ? kTurnSinCap : f18_turn_mag;  // 009D191E
    const float f40_sin = std::sin(capped);                    // 009D193A
    // 009D1962 compares +26Ch against +268h and 009D1966 JBE picks +268h, so
    // the slot ends up with the larger of the two turn radii.
    float f34 = (in.turn_radius_26c <= in.turn_radius_268) ? in.turn_radius_268
                                                           : in.turn_radius_26c;
    const float turn_room = static_cast<float>(
        static_cast<double>(f34 * f40_sin) +
        (static_cast<double>(speed) + kTurnRadiusBias));        // 009D1994
    bool skipped_to_join = false;
    if (turn_room > f14_range) {                               // 009D19A0
        host.update_run_time_009d1360();                       // 009D19A4
        out.run_time_updated = true;
        skipped_to_join = true;                                // 009D19A9
    }

    // --- 009D19AE-009D1B64, the sector probe arm. ---
    if (!skipped_to_join) {
        const float extent = kProbeTimeScale * f0c_time;       // 009D19C1
        if (in.sector_probe_enabled_aa) {                      // 009D19C5
            const float cone = clamped_interpolate_00419010(
                kProbeGateX0, kProbeGateY0, kProbeGateX1, kProbeGateY1,
                f0c_time);                                     // 009D1A01
            if (cone > f18_turn_mag) {                         // 009D1A0C
                // 009D1A20 is D8 C9 (FMUL ST(0),ST(1)), 00CEFFA0 = 0.7.
                const float lateral = 0.7f * extent;
                const float capped_lateral =
                    (lateral <= 150.0f) ? lateral : 150.0f;    // 009D1A34
                const TorpedoAimSectorProbe probe =
                    host.sector_probe_009d1a94(extent, capped_lateral,
                                               1.5f * extent);  // 009D1A94
                float f48 = 0.0f;
                if (probe.hit) {                                // 009D1AC9
                    float v = -probe.lateral * probe.scale_a *
                              probe.scale_b;                    // 009D1ADD
                    // 009D1AE1-009D1B1D: the 0.03 / -0.03 / 0.97 band.
                    if (v > 0.03f) {
                        v = (v - 0.03f) / 0.97f;
                    } else if (v < -0.03f) {
                        v = (v + 0.03f) / 0.97f;
                    } else {
                        v = 0.0f;
                    }
                    const float shape = clamped_interpolate_00419010(
                        kProbeGateX0, 1.0f, 2.5f, 0.25f, f0c_time);  // 009D1B4D
                    f48 = shape * (v * 1.39626f);               // 009D1B60
                }
                // 009D1B7A and 009D1B95: fold into the command, then |error|.
                const float commanded =
                    wrapped_angle_add_00438aa0(f1c_command, f48);
                f10_turn = wrapped_angle_subtract_00438b10(commanded,
                                                           f20_heading);
                f18_turn_mag = abs_bits(f10_turn);              // 009D1BB0
            }
        }
    }

    // --- 009D1BB8-009D1D39, the join, the throttle and the heading. ---
    const float commanded_heading =
        wrapped_angle_add_00438aa0(f20_heading, f10_turn);      // 009D1BCC
    const float bank_fold = clamped_interpolate_00419010(
        kBankFoldX0, kBankFoldY0, kBankFoldX1, kBankFoldY1,
        in.unit_bank_c64);                                      // 009D1C0B
    const float time_fold = clamped_interpolate_00419010(
        kTimeFoldX0, kTimeFoldY0, kTimeFoldX1, 1.0f, f0c_time); // 009D1C42
    host.refresh_world_pose_00414db0(in.pose_dirty_c8);         // 009D1C79
    const float speed_term = static_cast<float>(
        static_cast<double>(in.cruise_speed_25c) * kSpeedFoldScale);  // 009D1C57
    float throttle = (time_fold * speed_term) * bank_fold;      // 009D1C6D
    const float alt_fold = clamped_interpolate_00419010(
        in.alt_fold_a4, kAltFoldY0,
        static_cast<float>(static_cast<double>(in.alt_fold_a4) * kAltFoldX1Scale),
        kAltFoldY1, in.unit_altitude);                          // 009D1CCA
    throttle = alt_fold * throttle;                             // 009D1CCF
    if (throttle > kThrottleCap) throttle = kThrottleCap;       // 009D1CE5
    out.roll_limit_2e8 = kRollLimit;                            // 009D1D02
    out.commanded_heading_2c0 = commanded_heading;              // 009D1D16
    out.heading_mode_2cc = 2;                                   // 009D1D1E
    out.commanded_throttle_2c8 = throttle;                      // 009D1D2E

    // --- 009D1D44-009D1DED, the release threshold. ---
    host.refresh_world_pose_00414db0(in.pose_dirty_c8);         // 009D1D4F
    f34 = torpedo_aim_release_threshold_009d1ded(in.unit_altitude, f28_floor,
                                                 f18_turn_mag);

    // --- 009D1DD6-009D1EE5, the commanded pitch. ---
    float denom = 3.0f;                                         // 009D1DC4/1DF7
    if (0.0f - in.unit_bank_c64 > 0.0f) {                       // 009D1DE6
        const float sel = (host.unit_is_kind_vtable5c(kTightenKindA) ||
                           host.unit_is_kind_vtable5c(kTightenKindB))
                              ? 2.5f : 1.5f;                    // 009D1E1D/1E27
        denom = static_cast<float>(
            static_cast<double>((0.0f - in.unit_bank_c64) / in.pitch_div_1ac *
                                sel) + 3.0);                    // 009D1E49
    }
    float pitch_den = f14_range - kPitchRangeBias;              // 009D1E5A
    const float scaled = in.pitch_scale_188 * denom;            // 009D1E6A
    if (scaled > pitch_den) pitch_den = scaled;                 // 009D1E7E
    float pitch = -f34 / pitch_den;                             // 009D1E9A
    pitch = clamp_listing(pitch, kPitchClampLo, kPitchClampHi); // 009D1EAE
    out.commanded_altitude_2bc = pitch;                         // 009D1EDD
    out.altitude_mode_2d0 = 1;                                  // 009D1EE5

    // --- 009D1EEF-009D1F6B, the ordnance fade and the fixed command bits. ---
    if (in.has_ordnance_132 && kProbeGateX0 > f0c_time) {       // 009D1EF2/1F03
        out.fade_264 = clamped_interpolate_00419010(1.0f, 1.0f, kProbeGateX0,
                                                    0.0f, f0c_time);  // 009D1F2E
        out.fade_264_written = true;
    }
    out.flag_278 = 1.0f;                                        // 009D1F4A
    out.flag_27c = 1;                                           // 009D1F55
    out.flag_2a8 = 0.0f;                                        // 009D1F5C
    out.flag_2ac = 1;                                           // 009D1F64
    out.flag_2d8 = 0;                                           // 009D1F6B

    // --- 009D1F75-009D2027, the aim-solution byte and the timer. ---
    out.aim_solution_130 = torpedo_aim_solution_009d2021(
        f2c, in.aspect_scale_84, speed, f14_range);
    host.accumulate_timer_009fa3a0(dt);                         // 009D2027

    // --- 009D202C-009D2233, the five-flag release chain. ---
    const float aspect = abs_bits(std::cos(f2c));
    const float envelope =
        clamped_interpolate_00419010(kAspectX0, kAspectY0, kAspectX1,
                                     in.aspect_scale_84, aspect) * speed;
    out.gate_state_24 = !in.state_flag_24;                      // 009D2038
    out.gate_lead = (envelope > f14_range) &&
                    (static_cast<float>(static_cast<double>(f14_range) +
                                        kLeadSlack) > in.fall_lead_a0);  // 009D2052
    host.refresh_world_pose_00414db0(in.pose_dirty_c8);         // 009D2077
    const float alt_gate = clamped_interpolate_00419010(
        kAltGateX0, kAltGateY0, kAltGateX1, kAltGateY1, f0c_time);  // 009D20B4
    out.gate_altitude = alt_gate > in.unit_altitude;            // 009D20C4
    const float bank_signed = (in.unit_bank_rate_c68 > 0.0f)
                                  ? in.unit_bank_rate_c68
                                  : (0.0f - in.unit_bank_rate_c68);  // 009D20E4
    // 009D210A compares 0.8 against |+C68h| and 009D210E JBE jumps to the
    // clear, so the bank flag is only computed while the roll rate is small.
    out.gate_bank = false;
    if (kBankRateBias > bank_signed) {                          // 009D210E
        float limit = static_cast<float>(
            static_cast<double>(f14_range) * kBankRateScale) - kBankRateBias;
        if (!(limit <= kBankRateCap)) limit = kBankRateCap;     // 009D213C
        out.gate_bank = in.unit_bank_c64 > limit;               // 009D2165
    }
    const float cone_deg =
        (kConeSplit > f0c_time)
            ? clamped_interpolate_00419010(kConeNearX0, kConeNearY0, kConeSplit,
                                           kConeNearY1, f0c_time)   // 009D21AC
            : clamped_interpolate_00419010(kConeSplit, kConeNearY1, kConeFarX1,
                                           kConeFarY1, f0c_time);   // 009D21F2
    out.gate_cone = static_cast<float>(static_cast<double>(cone_deg) *
                                       kDegToRadNum / kDegToRadDen) >
                    f18_turn_mag;                               // 009D2209
    // 009D2215-009D2231: each of the four byte flags jumps to 009D228B when
    // clear, and 009D2231 TEST BL,BL jumps to the tail.  So the ground arm at
    // 009D2239 needs all five, while the countdown at 009D228F is re-tested at
    // 009D228B and therefore needs only the cone flag.
    if (out.gate_state_24 && out.gate_lead && out.gate_altitude &&
        out.gate_bank && out.gate_cone) {
        if (kGroundArmHeight > host.ground_height_00903860()) { // 009D2272
            host.arm_release_timer_009d2287();                  // 009D2287
        }
    }
    if (out.gate_cone && kCountdownGate > f0c_time) {           // 009D229D
        out.clear_slot_1c_40 = true;                            // 009D22AB
        out.countdown_a4 = (kCountdownSplit > f0c_time) ? 3 : 0xFF;
    }

    // --- 009D22D9-009D236E, the aim-complete test. ---
    const float final_speed = torpedo_commanded_speed_009d3c99(
        in.elapsed_134, in.speed_late_7c, in.speed_early_80);   // 009D22DC
    out.release_threshold_f34 = f34;
    out.range_f14 = f14_range;
    out.turn_magnitude_f18 = f18_turn_mag;
    out.time_to_target_f0c = f0c_time;
    out.ramp = clamped_interpolate_00419010(
        0.0f, 0.0f, 1.0f,
        static_cast<float>(static_cast<double>(final_speed) * kRampHalf),
        f0c_time);
    out.clause_range = f34 > (f14_range - out.ramp);
    out.clause_turn = f18_turn_mag > f0c_time;
    out.aim_complete_2c = torpedo_aim_complete_009d22ff(
        f34, f14_range, f18_turn_mag, f0c_time, final_speed);
    return out;
}

}  // namespace bsp
