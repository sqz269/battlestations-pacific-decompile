// The turn clearance blk+37Ch (009EF910) and the 65-bin throttle cost profile
// blk+4h (009E04E0 through 009D67F0 / 009D56F0).
//
// Packet cc_ai_clearance_profile. Ghidra was read-only. Every routine carries
// the address it comes from, the original ABI and its coverage; names are
// hypotheses, not recovered symbols. See docs/SHIP_AI_CLEARANCE_PROFILE.md for
// the evidence, the coverage table and the uncertainties.

#include "bsp/ship_ai_clearance_profile.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp" // clamped_interpolate_00419010,
                               // wrapped_angle_add_00438aa0,
                               // wrapped_angle_subtract_00438b10

namespace bsp {
namespace {

// 00415550 BSP_Math_MaxFloatByRef, __fastcall(const float* a, const float* b),
// RET 0. 009E0867 passes ECX = &1.0f and EDX = &(distance / speed).
float max_by_ref_00415550(float a, float b) noexcept { return (a > b) ? a : b; }

} // namespace

// ---------------------------------------------------------------------------
// 009EF910
// ---------------------------------------------------------------------------

// 009EFAAF (`COMISS XMM0,[00D7A218]` with XMM0 = the heading error, `JBE` to the
// blk+18Ch arm) and 009EFBDD (`XORPS XMM0,XMM0; COMISS XMM0,[ESP+2Ch]`, `JBE`
// to the same arm). Ahead the port shoulder is taken at or below zero, astern at
// or above zero.
bool ship_ai_clearance_uses_port_shoulder_009efaaf(bool astern_mode,
                                                   float heading_error) noexcept {
    return astern_mode ? (heading_error >= 0.0f) : (heading_error <= 0.0f);
}

// 009EFB03..009EFC8D. The adding arm is 009EFB14 / 009EFB33 / 009EFB4E and the
// subtracting arm 009EFC40 / 009EFC5F / 009EFC7A; both meet at the 009EFC81
// `FCOMIP ST0,ST1` whose carry flag leaves BL clear. The adding arm reaches that
// point with ST0 = the wrapped difference and ST1 = zero (009EFB53 `FLDZ` then
// `FXCH`), the subtracting arm with ST0 = zero and ST1 = the difference
// (009EFC7F `FLDZ`), which is why the two tests read as mirrors.
ShipAiClearanceSweep ship_ai_clearance_sweep_009efb03(bool port_arm,
                                                      float hull_heading,
                                                      float heading_target,
                                                      float sweep_half_angle) noexcept {
    // 009EFA91 `FLD [ESI+1B4h]; FSUBR double ptr [00CE3830]`.
    const float offset = kShipAiClearanceHalfPiWide - sweep_half_angle;

    ShipAiClearanceSweep out;
    if (port_arm) {
        out.from = wrapped_angle_add_00438aa0(hull_heading, offset);
        out.to = wrapped_angle_add_00438aa0(heading_target, kShipAiClearanceQuarterTurn);
        const float difference = wrapped_angle_subtract_00438b10(out.to, out.from);
        out.pending = !(difference >= 0.0f);
    } else {
        out.from = wrapped_angle_subtract_00438b10(hull_heading, offset);
        out.to = wrapped_angle_subtract_00438b10(heading_target, kShipAiClearanceQuarterTurn);
        const float difference = wrapped_angle_subtract_00438b10(out.to, out.from);
        out.pending = !(0.0f >= difference);
    }
    return out;
}

// 009EFEED..009EFF44. The squared distance is built as dz*dz then dx*dx
// (009EFEF5 `FLD ST1`, 009EFEF7 `FMULP ST2`, 009EFEF9 `FMUL ST0`), compared
// against the squared hull radius held in [ESP+34h], and only then square
// rooted (009EFF0F `CALL 00BF7030`).
float ship_ai_clearance_from_closest_point_009efeed(float pivot_x, float pivot_z,
                                                    float closest_x, float closest_z,
                                                    float hull_radius,
                                                    float current) noexcept {
    const float dx = closest_x - pivot_x;
    const float dz = closest_z - pivot_z;
    const float squared = dz * dz + dx * dx;
    if (!(squared > hull_radius * hull_radius)) {
        return current; // 009EFF0D JBE: the footprint is inside the hull circle
    }
    const float candidate = std::sqrt(squared) - hull_radius;
    if (current > candidate) { // 009EFF30 FCOMIP with ST0 = blk+37Ch
        return candidate;
    }
    return current;
}

// 009F0022..009F0072. 00419010 is `float __stdcall(x0, y0, x1, y1, x)`, RET 14h;
// the five pushes at 009F0060..009F004C read in slot order as
// (1.0, 1.0, 2.0, 0.0, ratio).
float ship_ai_clearance_faded_error_009f0022(float heading_error,
                                             float path_length,
                                             float class_length_unit) noexcept {
    const float ratio = path_length / class_length_unit;
    const float scale = clamped_interpolate_00419010(1.0f, 1.0f,
                                                     kShipAiClearancePathFadeSpans, 0.0f,
                                                     ratio);
    return heading_error * scale;
}

void ship_ai_refresh_turn_clearance_009ef910(ShipAiClearanceBlock& blk,
                                             const ShipAiClearanceGeometry& geometry,
                                             const ShipAiClearanceSettings& settings,
                                             float seconds,
                                             ShipAiClearanceHost& host) {
    // 009EF91D..009EF93D. The countdown is stored before the test, so a frame
    // that returns early still consumed its delta.
    blk.refresh_timer_374 -= seconds;
    if (blk.refresh_timer_374 > 0.0f) {
        return;
    }

    // 009EF943..009EF96F. The sentinel goes in before anything can lower it.
    blk.refresh_timer_374 = settings.refresh_period_1d4;
    blk.outcome_370 = ShipAiClearanceOutcome::Clear;
    blk.clearance_37c = kShipAiClearanceSentinel;

    // 009EF977..009EF9A8. Astern, the hull heading is turned through half a
    // circle so the rest of the routine can treat it as a forward heading.
    const bool astern = (blk.steering_mode_35c == 2);
    float hull_heading = host.hull_heading_vtable50();
    if (astern) {
        hull_heading = wrapped_angle_add_00438aa0(hull_heading, kShipAiObstacleHalfTurn);
    }

    // 009EF9C0..009EF9EB.
    const float heading_error =
        wrapped_angle_subtract_00438b10(blk.heading_target_324, hull_heading);
    if (kShipAiClearanceDeadband > std::fabs(heading_error)) {
        return;
    }

    // 009EF9F1..009EF9FF.
    const int neighbour_count = host.neighbour_count_604();
    const bool have_work = (neighbour_count > 0) || blk.static_zone_present_a3c;

    // 009EFA05..009EFA17.
    bool category_enabled[kShipAiClearanceCategoryCount] = {false, false, false};
    if (have_work) {
        for (int i = 0; i < kShipAiClearanceCategoryCount; ++i) {
            category_enabled[i] = host.obstacle_category_enabled_009ec770(i);
        }
    }

    bool reached_static_label = true; // 009EFFB1
    bool static_hit = false;          // [ESP+13h], cleared at 009EFC97

    if (have_work) {
        // 009EFA19..009EFC33. The pivot is the shoulder point on the inside of
        // the turn and the two directions bound the wedge the clearance is
        // measured in.
        const bool port_shoulder =
            ship_ai_clearance_uses_port_shoulder_009efaaf(astern, heading_error);
        const bool port_arm = astern ? !port_shoulder : port_shoulder;

        const float pivot_x = port_shoulder ? geometry.shoulder_port_x
                                            : geometry.shoulder_stbd_x;
        const float pivot_z = port_shoulder ? geometry.shoulder_port_z
                                            : geometry.shoulder_stbd_z;

        // 009EFA31 / 009EFB71: the hull's forward axis, reversed astern.
        const float dir_a_x = astern ? -geometry.forward_x : geometry.forward_x;
        const float dir_a_z = astern ? -geometry.forward_z : geometry.forward_z;

        // 009EFA1F..009EFAA0 and 009EFB5C..009EFBD1, the same expression in both
        // modes: the commanded heading turned into a world direction and
        // negated.
        float angle = kShipAiClearanceHalfPiWide - blk.heading_target_324;
        if (angle < 0.0f) {
            angle += kShipAiClearanceFullTurn;
        }
        const float dir_b_x = -std::cos(angle);
        const float dir_b_z = -std::sin(angle);

        const ShipAiClearanceSweep sweep =
            ship_ai_clearance_sweep_009efb03(port_arm, hull_heading,
                                             blk.heading_target_324,
                                             geometry.sweep_half_angle);

        // 009EFCA0..009EFD4F. Four conjuncts guard the static test; the
        // 009EFCA6 comparison is against BL, which is zero on this path.
        if (sweep.pending && blk.avoidance_enabled_3f4 &&
            host.avoidance_globally_enabled_0080e160_242() &&
            blk.static_zone_present_a3c) {
            static_hit = host.static_zone_blocks_009d57e0(pivot_x, pivot_z,
                                                          geometry.hull_radius,
                                                          sweep.from, sweep.to);
            if (static_hit) {
                blk.clearance_37c = 0.0f; // 009EFD10
            } else {
                // 009EFD4F. 00415D70 answers FLT_MAX (00D7A248) when no segment
                // survives the wedge, which replaces the 9999.0f sentinel with a
                // larger one; both ramp the danger level to zero.
                blk.clearance_37c =
                    host.static_zone_clearance_00415d70(pivot_x, pivot_z,
                                                        geometry.hull_radius,
                                                        dir_a_x, dir_a_z,
                                                        dir_b_x, dir_b_z);
            }
        }

        int hit_index = -1; // 009EFD5B OR ECX,0xFFFFFFFF
        if (neighbour_count != 0 && sweep.pending) {
            // 009EFD7E..009EFDD6. The last neighbour that blocks the sweep wins;
            // the loop does not break.
            for (int i = 0; i < neighbour_count; ++i) {
                if (!host.neighbour_owner_present_14(i)) {
                    continue;
                }
                if (host.neighbour_owner_gone_5e(i)) {
                    continue;
                }
                const int category = host.neighbour_owner_category_54(i);
                if (category < 0 || category >= kShipAiClearanceCategoryCount) {
                    continue;
                }
                if (!category_enabled[category]) {
                    continue;
                }
                if (host.neighbour_blocks_sweep_009dd010(i, pivot_x, pivot_z,
                                                         geometry.hull_radius,
                                                         sweep.from, sweep.to)) {
                    hit_index = i;
                }
            }

            if (!static_hit && hit_index < 0) {
                // 009EFE03..009EFF54. No owner or category filter here: every
                // neighbour in the list is measured.
                for (int i = 0; i < neighbour_count; ++i) {
                    float sx = 0.0f;
                    float sz = 0.0f;
                    host.neighbour_support_point_009d8860(i, dir_a_x, dir_a_z, sx, sz);
                    if (!((sx - pivot_x) * dir_a_x + (sz - pivot_z) * dir_a_z > 0.0f)) {
                        continue; // 009EFE59
                    }
                    host.neighbour_support_point_009d8860(i, dir_b_x, dir_b_z, sx, sz);
                    if (!((sx - pivot_x) * dir_b_x + (sz - pivot_z) * dir_b_z > 0.0f)) {
                        continue; // 009EFEB3
                    }
                    float cx = 0.0f;
                    float cz = 0.0f;
                    host.neighbour_closest_point_009d8a30(i, pivot_x, pivot_z, cx, cz);
                    blk.clearance_37c =
                        ship_ai_clearance_from_closest_point_009efeed(pivot_x, pivot_z,
                                                                      cx, cz,
                                                                      geometry.hull_radius,
                                                                      blk.clearance_37c);
                }
            } else {
                blk.clearance_37c = 0.0f; // 009EFF60
            }

            if (hit_index >= 0) {
                reached_static_label = false; // 009EFF6D falls through
                const float speed = std::fabs(host.neighbour_speed_0092d730(hit_index));
                blk.outcome_370 = (speed > kShipAiStallNeighbourSpeed)
                                      ? ShipAiClearanceOutcome::BlockedMoving  // 009EFFA5
                                      : ShipAiClearanceOutcome::BlockedStopped; // 009EFFB8
            }
        }
    }

    // 009EFFB1..009EFFB8.
    if (reached_static_label && static_hit) {
        blk.outcome_370 = ShipAiClearanceOutcome::BlockedStopped;
    }

    // 009EFFC2.
    if (blk.outcome_370 != ShipAiClearanceOutcome::Clear) {
        // 009F00D2. The hold is raised, never lowered.
        if (kShipAiObstacleHold > blk.hold_354) {
            blk.hold_354 = kShipAiObstacleHold;
        }
        return;
    }

    // 009EFFCF..009EFFF2.
    bool committed = false;
    if (blk.steering_mode_35c == 1) {
        committed = !blk.committed_ahead_364;
    } else if (blk.steering_mode_35c == 2) {
        committed = blk.committed_ahead_364;
    }

    // 009EFFF2..009F0072.
    float error = heading_error;
    if (host.path_fade_applies_00778890()) {
        error = ship_ai_clearance_faded_error_009f0022(error, blk.path_length_330,
                                                       host.class_length_unit_00811a30());
    }

    // 009F0076..009F00BD.
    const float gate = committed ? settings.error_gate_committed_214
                                 : settings.error_gate_free_218;
    if (std::fabs(error) > gate) {
        blk.outcome_370 = ShipAiClearanceOutcome::HeadingErrorLarge;
    }
}

// ---------------------------------------------------------------------------
// 009D56F0 and 009D67F0
// ---------------------------------------------------------------------------

// 009D56F0..009D570A for the low end and 009D5723..009D5729 for the high end.
// The 2.0 (00D7A308) and the 16.0 (00CED9F8) stay on the x87 stack between the
// two conversions, so both ends use the same expression. 00BF7420 truncates.
int ship_ai_throttle_band_index_009d56f0(float value) noexcept {
    const double scaled = (static_cast<double>(value) + kShipAiThrottleProfileOrigin) *
                          kShipAiThrottleProfileScale;
    return static_cast<int>(scaled);
}

void ship_ai_add_throttle_band_009d56f0(ShipAiThrottleProfile& profile,
                                        float low, float high,
                                        std::int8_t cost) noexcept {
    const int last = kShipAiThrottleProfileBins - 1; // 0x40 at 009D5719

    int first_bin = ship_ai_throttle_band_index_009d56f0(low);
    if (first_bin < 0) {
        first_bin = 0;
    } else if (first_bin > last) {
        first_bin = last;
    }

    int last_bin = ship_ai_throttle_band_index_009d56f0(high);
    if (last_bin < 0) {
        last_bin = 0;
    } else if (last_bin > last) {
        last_bin = last;
    }

    if (first_bin > last_bin) {
        return; // 009D5742 JG
    }

    // 009D5748. Any band at all takes the profile out of bypass, which is the
    // state the control-block constructor 009E4330 leaves it in (009E435F).
    profile.bypass_41 = false;

    // 009D5750 `ADD byte ptr [ESI+EDI],CL`: an 8-bit wrapping add, and the cost
    // is signed.
    for (int bin = first_bin; bin <= last_bin; ++bin) {
        const std::uint8_t sum = static_cast<std::uint8_t>(
            profile.bin[static_cast<std::size_t>(bin)] + static_cast<std::uint8_t>(cost));
        profile.bin[static_cast<std::size_t>(bin)] = sum;
    }
}

// 009D67F0-009D680C. `ADD ECX,0x4` at 009D67FD is what moves `this` from the
// control block to the profile, and the cost is the literal 1 pushed at
// 009D67F4.
void ship_ai_mark_throttle_band_009d67f0(ShipAiThrottleProfile& profile,
                                         float low, float high) noexcept {
    ship_ai_add_throttle_band_009d56f0(profile, low, high, 1);
}

// ---------------------------------------------------------------------------
// 009E04E0
// ---------------------------------------------------------------------------

ShipAiContactHorizon ship_ai_contact_horizon_009e07d1(float lateral,
                                                      float hull_half_width,
                                                      float heading_error,
                                                      const ShipAiContactTrack& track) noexcept {
    // 009E0521 computes the divided half width once per step; the division is
    // repeated here so the rule stays self-contained.
    const float half_width = hull_half_width / kShipAiContactHalfWidthDivisor;
    const float sine = std::sin(heading_error); // 009E07D5 FSIN

    // 009E07FB..009E0837.
    float distance = (std::fabs(lateral) - half_width) / std::fabs(sine);
    distance -= track.lead_time_04;
    if (distance < 0.0f) {
        distance = 0.0f; // 009E0843
    }

    ShipAiContactHorizon out;
    out.distance = distance;
    out.usable = (distance < track.max_horizon_0c); // 009E0856 JBE skips the track
    if (out.usable) {
        // 009E085C and 009E087F.
        out.seconds = max_by_ref_00415550(kShipAiContactMinHorizon,
                                          distance / track.speed_18);
    }
    return out;
}

// 009E0953..009E09E4 (upper) and 009E0A30..009E0AF0 (lower). Both solve
// x^2 + b*x + c = 0 with b built from the reachable speed after `horizon_seconds`
// of acceleration and c from the distance; the halving constant is +0.5
// (00D7A280) at the upper end and -0.5 (00CEC9E0) at the lower.
float ship_ai_contact_speed_root_009e0953(float average, float distance,
                                          float own_speed, float acceleration,
                                          float horizon_seconds,
                                          bool upper_end) noexcept {
    if (upper_end) {
        if (!(average > 0.0f)) {
            return average; // 009E093F
        }
        if (!(average > own_speed)) {
            return average; // 009E094D
        }
        const float b = (own_speed + acceleration * horizon_seconds) * -2.0f;
        const float discriminant =
            b * b - ((distance + distance) * acceleration + own_speed * own_speed) * 4.0f;
        if (!(discriminant >= 0.0f)) {
            return average; // 009E099F
        }
        const float root = std::sqrt(discriminant);
        const float lower_solution = (-b - root) * 0.5f;
        const float upper_solution = 0.5f * (root + -b);
        return (average < lower_solution) ? lower_solution : upper_solution;
    }

    if (!(average < 0.0f)) {
        return average;
    }
    if (!(average < own_speed)) {
        return average;
    }
    const float b = (acceleration * horizon_seconds - own_speed) * -2.0f;
    const float discriminant =
        b * b - (own_speed * own_speed - (distance + distance) * acceleration) * 4.0f;
    if (!(discriminant >= 0.0f)) {
        return average;
    }
    const float root = std::sqrt(discriminant);
    const float lower_solution = (-b - root) * -0.5f;
    const float upper_solution = -0.5f * (root + -b);
    return (lower_solution < average) ? lower_solution : upper_solution;
}

ShipAiThrottleWindow ship_ai_contact_throttle_window_009e0923(float along_distance,
                                                              float margin,
                                                              float horizon_seconds,
                                                              float own_speed,
                                                              float acceleration,
                                                              float reference_speed) noexcept {
    // 009E091B and 009E0A2E-ish: the crossing interval is the along-track
    // distance plus and minus the combined half lengths.
    const float far_distance = margin + along_distance;
    const float near_distance = along_distance - margin;

    float high = far_distance / horizon_seconds; // 009E092F
    high = ship_ai_contact_speed_root_009e0953(high, far_distance, own_speed,
                                               acceleration, horizon_seconds, true);
    float low = near_distance / horizon_seconds;
    low = ship_ai_contact_speed_root_009e0953(low, near_distance, own_speed,
                                              acceleration, horizon_seconds, false);

    ShipAiThrottleWindow out;
    out.high = high / reference_speed; // 009E0A24-ish
    out.low = low / reference_speed;   // 009E0B37
    return out;
}

ShipAiContactBandOutcome ship_ai_apply_contact_band_009e0b37(ShipAiThrottleProfile& profile,
                                                             ShipAiThrottleWindow window,
                                                             ShipAiContactCounts& counts,
                                                             bool& stand_on) noexcept {
    if (!(window.high > kShipAiThrottleAxisLow)) {
        return ShipAiContactBandOutcome::OffAxis; // 009E0B4C
    }
    if (!(kShipAiThrottleAxisHigh > window.low)) {
        return ShipAiContactBandOutcome::OffAxis; // 009E0B63
    }

    if (!(window.high > kShipAiContactMinHorizon)) {
        // 009E0BF9. The window fits under full ahead, so it is a real band.
        counts.banded_84 += 1;
        if (window.low >= kShipAiThrottleWindowAsternEdge) {
            ship_ai_mark_throttle_band_009d67f0(profile, window.low, window.high);
            return ShipAiContactBandOutcome::Banded; // 009E0C45
        }
        // 009E0C19. The low end runs off the astern side, so the band is widened
        // to the end of the axis.
        ship_ai_mark_throttle_band_009d67f0(profile, kShipAiThrottleAxisLow, window.high);
        if (window.high > kShipAiThrottleWindowSettled) {
            return ShipAiContactBandOutcome::BandedContinue; // 009E0C2C falls through
        }
        return ShipAiContactBandOutcome::Banded;
    }

    stand_on = false; // 009E0B7A XOR BL,BL
    if (window.low >= kShipAiThrottleWindowAsternEdge) {
        // 009E0BD1. The window reaches past full ahead; everything from the low
        // end upwards is blocked.
        counts.banded_84 += 1;
        ship_ai_mark_throttle_band_009d67f0(profile, window.low, kShipAiThrottleAxisHigh);
        return ShipAiContactBandOutcome::Banded;
    }

    // 009E0B81..009E0BCC. The window covers the whole axis, so no throttle
    // avoids this contact and the counters decide what the hull does instead.
    if ((window.low + 0.5f) * -2.0f <= window.high - 1.0f) {
        counts.stand_on_74 += 1;
        stand_on = true;
    } else {
        counts.give_way_80 += 1;
    }
    return ShipAiContactBandOutcome::NotThrottleable;
}

ShipAiAvoidanceStep ship_ai_contact_avoidance_step_009e0c7c(float heading_error,
                                                            float lateral_side,
                                                            float gap,
                                                            float width,
                                                            bool stand_on,
                                                            float dir_x,
                                                            float dir_z) noexcept {
    // 009E0657..009E0679: the contact's left normal, the step the routine
    // prefers.
    const ShipAiAvoidanceStep normal{-dir_z, dir_x};
    const float magnitude = std::fabs(heading_error);

    // 009E0C86. Past a quarter turn the bearing arms run; inside it the arm at
    // 009E0E42 runs. Both have the same shape, with opposite signs on the
    // course step (00D7A308 is +2.0, 00D19660 is -2.0).
    const bool wide = !(magnitude <= kShipAiClearanceHalfPiWide);
    if (wide && magnitude > kShipAiContactBearingMax) {
        // 009E0CA2. Nearly reciprocal: the side alone decides.
        return (lateral_side < 0.0f) ? normal
                                     : ShipAiAvoidanceStep{-normal.x, -normal.z};
    }
    if (!wide && magnitude < kShipAiContactBearingMin) {
        // 009E0E42's inner arm, the same test with the same answer.
        return (lateral_side < 0.0f) ? normal
                                     : ShipAiAvoidanceStep{-normal.x, -normal.z};
    }

    // 009E0D07 / 009E0DB4 and their counterparts inside the quarter turn: the
    // lateral gap has to land inside a band whose low end opens up when the hull
    // is standing on.
    const float course_sign = wide ? -2.0f : 2.0f;
    const float reach = (heading_error >= 0.0f) ? (gap + lateral_side)
                                                : (gap - lateral_side);
    const float low = stand_on ? -(kShipAiAvoidBandPad + width) : 0.0f;
    if (low <= reach && reach < width + kShipAiAvoidBandPad) {
        return normal; // 009E0D48
    }
    return ShipAiAvoidanceStep{dir_x * course_sign, course_sign * dir_z}; // 009E0D71
}

ShipAiThrottleWindow ship_ai_commitment_band_009e10a9(const ShipAiContactCounts& counts,
                                                      float throttle_ratio,
                                                      float snapped_ratio) noexcept {
    const ShipAiThrottleWindow ahead{kShipAiThrottleAxisLow, kShipAiCommitAheadEdge};
    const ShipAiThrottleWindow astern{kShipAiCommitAsternEdge, kShipAiThrottleAxisHigh};

    if (counts.banded_84 == 0) {
        // 009E110F..009E112E. Without a band the counters pick the side: each
        // give-way track that could not be banded pushes the threshold up by
        // four and each stand-on track pulls it down by four.
        const float threshold =
            static_cast<float>(counts.give_way_80 - counts.stand_on_74) * 4.0f;
        return (threshold <= throttle_ratio) ? ahead : astern;
    }
    // 009E10F2. With a band, the side is whichever side the snapped throttle
    // already sits on.
    return (snapped_ratio >= 0.0f) ? ahead : astern;
}

void ship_ai_build_throttle_profile_009e04e0(ShipAiThrottleProfileBlock& blk,
                                             const ShipAiThrottleProfileInputs& inputs,
                                             float seconds,
                                             ShipAiThrottleProfileHost& host) {
    // 009E04FE..009E050B. The heading is fetched and discarded; the call is the
    // point at which the unit's pose can refresh.
    static_cast<void>(host.hull_heading_vtable50());

    // 009E0562..009E059C.
    blk.avoid_x_34c = 0.0f;
    blk.avoid_z_350 = 0.0f;
    if (blk.hold_354 >= 0.0f) {
        blk.hold_354 -= seconds;
    }

    // 009E05A2.
    if (host.track_count_400() < 1) {
        return;
    }

    ShipAiContactCounts counts;

    for (int index = 0; index < host.track_count_400(); ++index) {
        ShipAiContactTrack& track = host.track_at(index);

        // 009E05C7..009E05F9.
        track.lifetime_14 -= seconds;
        const bool expired = (track.lifetime_14 <= 0.0f && track.lifetime_14 != 0.0f);
        if (expired || track.retired_64 || !host.track_has_source(index)) {
            host.destroy_track(index);
            index -= 1; // 009E1075 SUB EAX,1
            continue;
        }

        // 009E0606..009E0622.
        if (!host.track_range_within_target_488(index)) {
            continue;
        }
        if (!host.avoidance_active_009da1d0()) {
            continue;
        }

        // 009E0631.
        if (!host.refresh_track_009dc060(index)) {
            continue;
        }

        // 009E0636..009E0653.
        const float heading_error =
            wrapped_angle_subtract_00438b10(host.hull_heading_vtable50(), track.heading_20);
        const float magnitude = std::fabs(heading_error);

        // 009E0683..009E06C2. The lateral term uses the contact's own left
        // normal, so a positive value means the hull is to the contact's left.
        const float rel_x = track.pos_x_2c - inputs.position_x;
        const float rel_z = track.pos_z_30 - inputs.position_z;
        const float lateral_side = rel_x * -track.dir_z_28 + rel_z * track.dir_x_24;

        // 009E068C..009E06FF. 00BF9940's domain clamp (0.0 above 1.0, pi below
        // -1.0) is the arc-cosine pattern; the routine feeds it the heading error
        // rather than a cosine.
        float arc = 0.0f;
        if (!(heading_error > 1.0f)) {
            arc = (heading_error >= -1.0f) ? std::acos(heading_error)
                                           : kShipAiObstacleHalfTurn;
        }

        // 009E0705..009E074B.
        const float gap =
            (1.0f - std::fabs(arc)) * host.class_length_three_quarters_00811a30();

        // 009E074F..009E0763.
        const float width = inputs.hull_half_width + kShipAiContactWidthPad;

        bool stand_on = false;
        bool run_avoidance = false;

        if (magnitude <= kShipAiContactBearingMin || magnitude >= kShipAiContactBearingMax) {
            // 009E0C52..009E0C76. A contact on nearly the same or the exactly
            // opposite course cannot be banded; it only steers.
            run_avoidance = (std::fabs(lateral_side) < width + kShipAiContactWidthPad);
        } else {
            // 009E0785..009E07CC.
            const float lateral =
                rel_x * inputs.normal_x + inputs.normal_z * rel_z;
            if ((lateral > 0.0f && heading_error > 0.0f) ||
                (lateral < 0.0f && heading_error < 0.0f)) {
                track.retired_64 = true; // 009E07C8
                continue;
            }

            const ShipAiContactHorizon horizon =
                ship_ai_contact_horizon_009e07d1(lateral, inputs.hull_half_width,
                                                 heading_error, track);
            if (!horizon.usable) {
                continue;
            }

            // 009E0888..009E0913.
            const float sine = std::sin(heading_error);
            const float cosine = std::cos(heading_error);
            const float along = std::fabs(lateral / sine) * cosine +
                                rel_x * inputs.forward_x + inputs.forward_z * rel_z;
            const float margin = inputs.hull_beam * kShipAiContactBeamFraction +
                                 std::fabs(track.half_length_00 / sine) +
                                 kShipAiContactLengthPad;

            const ShipAiThrottleWindow window =
                ship_ai_contact_throttle_window_009e0923(along, margin, horizon.seconds,
                                                         inputs.own_speed,
                                                         inputs.acceleration,
                                                         inputs.reference_speed_3c4);

            const ShipAiContactBandOutcome outcome =
                ship_ai_apply_contact_band_009e0b37(blk.profile, window, counts, stand_on);
            switch (outcome) {
            case ShipAiContactBandOutcome::OffAxis:
            case ShipAiContactBandOutcome::Banded:
                continue;
            case ShipAiContactBandOutcome::BandedContinue:
            case ShipAiContactBandOutcome::NotThrottleable:
                run_avoidance = true;
                break;
            }
        }

        if (run_avoidance) {
            const ShipAiAvoidanceStep step =
                ship_ai_contact_avoidance_step_009e0c7c(heading_error, lateral_side, gap,
                                                        width, stand_on,
                                                        track.dir_x_24, track.dir_z_28);
            blk.avoid_x_34c += step.x;
            blk.avoid_z_350 += step.z;
        }
    }

    // 009E109C..009E10AD.
    if (counts.stand_on_74 == 0 && counts.give_way_80 == 0) {
        return;
    }

    // 009E10B8.
    float ratio = inputs.own_speed / inputs.reference_speed_3c4;

    // 009E10C6..009E10E2. The snap only runs when at least one real band exists;
    // it reads the bins this routine just wrote.
    float snapped = ratio;
    if (counts.banded_84 != 0) {
        snapped = ship_ai_apply_throttle_profile_009d6b40(blk.profile, ratio, -1.0f, 1.0f);
        ratio = snapped;
    }

    const ShipAiThrottleWindow commitment =
        ship_ai_commitment_band_009e10a9(counts, ratio, snapped);
    ship_ai_add_throttle_band_009d56f0(blk.profile, commitment.low, commitment.high,
                                       kShipAiCommitBandCost);
}

} // namespace bsp
