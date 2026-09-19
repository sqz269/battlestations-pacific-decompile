// The dive-bomb bot task (kind 8). Evidence in docs/DIVE_BOMB_TASK.md.
//
// Every rule below is a transcription of a listing range named in its comment.
// Where a range was read only for its gates and its constants the comment says
// `coverage: partial` and the rule models exactly what was read.

#include "bsp/dive_bomb_task.hpp"

#include <cmath>

namespace bsp {
namespace {

// The abs-fold every gate in this class uses: `movss xmm, [00D7A208]` then
// `subss xmm, x`, i.e. (-0.0f) - x. Identical to fabs for every finite x, and
// kept in this shape so the sign of a -0.0f input matches the image.
inline float fold_abs(float x) noexcept {
    return (x > 0.0f) ? x : (dive_bomb_constant::kNegativeZero - x);
}

// 00438AA0 BSP_Math_AddWrappedAngle: add and wrap into [0, 2pi).
//
// DEFECT, found by packet cc8_dive_flyover and NOT fixed here because this
// helper is shared by hunks other workers hold (2026-09-19 arbitration). The
// image wraps into (-pi, pi], not [0, 2pi): 00438AB0 loads the double -pi at
// 00CE3D18 and 00438AB6's FCOMI with 00438AB8's `72` JC sends every sum above
// -pi to 00438ADF, which loads the +pi at 00CE3D28 and subtracts the 2pi at
// 00CE3828 while the sum exceeds it; the other arm adds 2pi while the sum is at
// or below -pi. src/unit_rudder.cpp's wrap_native_angle has it right and cites
// the same ranges. Inert for the commands this file produces, because the only
// consumer (game_hosts_units.cpp, the PilotBotHeadingTerm at the unit arm)
// takes wrapped_angle_subtract_00438b10 of the result against the aircraft's
// own heading, and that subtraction is invariant to a 2pi offset.
inline float wrapped_angle_add_00438aa0(float base, float delta) noexcept {
    const float two_pi = static_cast<float>(dive_bomb_constant::kTwoPi);
    float v = base + delta;
    while (v >= two_pi) v -= two_pi;
    while (v < 0.0f) v += two_pi;
    return v;
}

// 00438B10 BSP_Math_SubtractWrappedAngle: subtract and wrap into (-pi, pi].
// 00438B20..00438B7B is the same loop pair as the add above, and this one is
// transcribed from it rather than from the sibling helper.
inline float wrapped_angle_subtract_00438b10(float left, float right) noexcept {
    const double pi = 3.1415927410125732;       // 00CE3D28 / 00CE3D18 negated
    const double two_pi = 6.2831854820251465;   // 00CE3828
    float v = left - right;
    if (static_cast<double>(v) <= -pi) {
        do {
            v = static_cast<float>(static_cast<double>(v) + two_pi);
        } while (static_cast<double>(v) <= -pi);
    } else {
        while (static_cast<double>(v) > pi) {
            v = static_cast<float>(static_cast<double>(v) - two_pi);
        }
    }
    return v;
}

}  // namespace

// 009C7910-009C796B, __thiscall(task, state) -> bool, RET 4. Eight LEA/CMP
// pairs in the order aimdive, aimglide, done, flyabove, goaway, prepare,
// attackrun, turndown; anything else falls to `XOR AL,AL` at 009C7964.
bool dive_bomb_is_attacking_009c7910(DiveBombState state) noexcept {
    switch (state) {
        case DiveBombState::kAimDive:
        case DiveBombState::kAimGlide:
        case DiveBombState::kDone:
        case DiveBombState::kFlyAbove:
        case DiveBombState::kGoAway:
        case DiveBombState::kPrepare:
        case DiveBombState::kAttackRun:
        case DiveBombState::kTurnDown:
            return true;
        default:
            return false;
    }
}

// 009C83F8-009C8417. `CMP byte ptr [ESI+4C8h],0` / `JNZ`, then
// `CMP dword ptr [EAX+370h],2` / `JNZ` and `CMP dword ptr [ESI+440h],0` / `JNZ`.
bool dive_bomb_engaged_009c83f8(const DiveBombEngagedInputs& in) noexcept {
    if (in.in_range_latch_4c8) {
        return true;
    }
    return in.control_mode_370 == 2 && in.has_latched_target_440;
}

// 009C8310-009C83D2, __fastcall(task). Four arms in this order; each tail-calls
// the state's enter slot after the usual exit/store/enter triple.
DiveBombState dive_bomb_entry_state_009c8310(const DiveBombEntryInputs& in) noexcept {
    if (in.control_mode_370 == 0) {
        return DiveBombState::kPrepare;  // 009C8319
    }
    if (!in.has_bomb_ordnance_4c9 &&
        (!in.control_flag_369 || !in.global_e17bf2)) {
        return DiveBombState::kDone;  // 009C834F-009C8371
    }
    if (in.in_range_latch_4c8) {
        return DiveBombState::kFlyAbove;  // 009C8379-009C8388
    }
    return DiveBombState::kAttackRun;  // 009C83AE
}

// 009C83E0-009C8785, __fastcall(task), RET 4.
DiveBombTransitionResult dive_bomb_next_state_009c83e0(
    const DiveBombTransitionInputs& in) noexcept {
    DiveBombTransitionResult out;
    out.next = in.current;

    const bool attacking = dive_bomb_is_attacking_009c7910(in.current);
    const bool engaged = dive_bomb_engaged_009c83f8(in.engaged);

    // The approach fallback both the !attacking and the attacking arm share.
    // CORRECTED, packet cc8_dive_approach, from docs/BOMBER_AFTER_TASK.md 10.1,
    // which read 009C83E0-009C8783 whole: the two `!engaged` edges are
    // 009C8419-009C845E (the attacking half) and 009C873E-009C8783 (the
    // non-attacking half). The second range named here, 009C8489-009C84E8, is
    // inside the ENGAGED continuation and is not an approach edge at all.
    const DiveBombState approach_state =
        in.unit_lacks_follow_target ? DiveBombState::kMoveTo : DiveBombState::kFollow;

    if (!attacking) {
        out.next = engaged ? dive_bomb_entry_state_009c8310(in.entry) : approach_state;
    } else if (!engaged) {
        // CORRECTED: 009C8419-009C845E. 009C8483 is the ENGAGED continuation
        // (`LEA EBX,[ESI+664h] / CMP EDI,EBX / JZ ret`), not the abort.
        out.next = approach_state;  // 009C8419-009C845E
    } else if (in.engaged.control_mode_370 == 0) {
        // CORRECTED: the test is 009C8461 and the store 009C8470. 009C84F5 is
        // inside the engaged continuation.
        out.next = DiveBombState::kPrepare;  // 009C8470
    } else if (in.current != DiveBombState::kDone) {
        // 009C8483 `LEA EBX,[ESI+664h] / CMP EDI,EBX / JZ ret`: a task already
        // in done returns unchanged, which is what parks a spent bomber there.
        if (in.should_break_off) {           // 009C8495 CALL [[ESI]+1Ch]
            // CORRECTED: 009C849E. 009C8514 is not an instruction boundary at
            // all -- 009C8515 is `MOV EAX,[ESI+798h]`, inside the flyabove arm.
            out.next = DiveBombState::kDone;  // 009C849E, +664h is done
        } else {
            switch (in.current) {
                case DiveBombState::kPrepare:
                    // 009C8536: re-run the entry chooser from prepare.
                    out.next = dive_bomb_entry_state_009c8310(in.entry);
                    break;
                case DiveBombState::kAttackRun:
                    // 009C8545: the only edge out of attackrun.
                    if (in.engaged.in_range_latch_4c8) {
                        out.next = DiveBombState::kFlyAbove;
                    }
                    break;
                case DiveBombState::kFlyAbove:
                    if (in.flyabove_ready_791) {
                        if (!in.flyabove_can_dive_790) {
                            out.next = DiveBombState::kAimGlide;  // 009C8557
                        } else {
                            // 009C8563-009C85C9: roll into the dive, picking a
                            // side from +798h, then from the attitude, then
                            // from the coin at 009C85A3.
                            int side = in.flyabove_turn_side_798;
                            if (side == 0) {
                                if (in.unit_bank_c68 > in.bank_high_00ce398c) {
                                    side = 1;
                                } else if (in.unit_bank_c68 < in.bank_low_00d1fbc0) {
                                    side = -1;
                                } else {
                                    side = in.random_turn_side;
                                }
                            }
                            out.wrote_turn_direction = true;
                            out.turn_side = side;
                            out.next = DiveBombState::kTurnDown;
                        }
                    } else if (in.flyabove_leave_792) {
                        out.next = DiveBombState::kGoAway;  // 009C85F5
                    }
                    break;
                case DiveBombState::kTurnDown:
                    if (in.turndown_complete) {
                        out.next = DiveBombState::kAimDive;  // 009C8620
                    }
                    break;
                case DiveBombState::kAimDive:
                    if (!in.aimdive_alive_74d) {
                        out.next = DiveBombState::kAimGlide;  // 009C8650
                    } else if (!in.entry.has_bomb_ordnance_4c9) {
                        out.next = DiveBombState::kGoAway;  // 009C8664, through +738h
                    } else if (in.aimdive_pull_out_74c) {
                        out.next = DiveBombState::kGoAway;  // 009C8677
                    }
                    break;
                case DiveBombState::kAimGlide:
                    // CORRECTED addresses, packet cc8_dive_approach, read from
                    // the listing at 009C868B-009C86D0. The arm is entered by
                    // `LEA ECX,[ESI+754h] / CMP EAX,ECX / JNE 009C86D3`, and
                    // BOTH exits store +704h goaway. Neither 009C8694 nor
                    // 009C86B4 is an instruction boundary.
                    if (in.aimglide_out_of_bombs) {   // 009C8695 CALL 009C7850
                        out.next = DiveBombState::kGoAway;  // 009C869E
                    } else if (in.aimglide_pull_out_76c) {  // 009C86B2, byte +76Ch
                        out.next = DiveBombState::kGoAway;  // 009C86BF
                    }
                    // With both false, 009C86B9 `JE 009C84E7` leaves the state
                    // alone: the aimglide is TERMINAL unless one of these two
                    // fires. Confirms packet cc8_dive_flyover's finding from
                    // the listing rather than from the run.
                    break;
                case DiveBombState::kGoAway:
                    // 009C86D3 `LEA ECX,[ESI+704h]` / 009C86D9 `CMP EAX,ECX` is
                    // the GOAWAY arm's own state test, not an aimglide edge.
                    if (in.goaway_complete) {   // 009C86E1 CALL 009C7F00
                        // 009C86EE `CMP byte [ESI+4C9h],0` splits on the
                        // ordnance byte. The two destinations are PROVED by the
                        // registers, not assumed: 009C86F9 pushes EDI, loaded
                        // `LEA EDI,[ESI+778h]` at 009C84ED = flyabove, and
                        // 009C8705 pushes EBX, loaded `LEA EBX,[ESI+664h]` at
                        // 009C8483 = done.
                        out.next = in.entry.has_bomb_ordnance_4c9
                                       ? DiveBombState::kFlyAbove   // 009C86F9
                                       : DiveBombState::kDone;      // 009C8705
                    }
                    break;
                default:
                    break;
            }
        }
    }

    out.changed = out.next != in.current;
    if (!out.changed) {
        out.wrote_turn_direction = false;
        out.turn_side = 0;
    }
    return out;
}

// 009C7BFB-009C7C31. The hysteresis on approach+D0h.
bool dive_bomb_in_range_latch_009c7c31(const DiveBombRangeLatchInputs& in) noexcept {
    float threshold;
    bool latch;
    if (!in.latched) {
        threshold = in.in_range_distance;
        latch = !(threshold <= in.planar_distance);
    } else if (!in.control_flag_369 || !in.global_e17bf2) {
        threshold = in.in_range_distance +
                    static_cast<float>(dive_bomb_constant::kMoveToRangeBias);
        latch = !(threshold <= in.planar_distance);
    } else {
        // 009C8C2F falls straight to the `1`, and 009C7C57's JNZ then skips the
        // spent-member arm on the same two conditions, so this returns here.
        return true;
    }
    // 009C7C3E: a bomber that still has bombs is done.
    if (in.has_bomb_ordnance_d1) return latch;
    // 009C7C44-009C7C57: the same guard as the hold arm above, in the same
    // order - skip only when BOTH the control flag and the global are set.
    if (in.control_flag_369 && in.global_e17bf2) return latch;
    // 009C7C63 and the null squadron: no leader to measure against.
    if (in.is_flight_leader || !in.leader_known) return latch;
    // 009C7CCB `FLD [ESI+B8h]` / `FCOMPI` / `JBE 009C7CFC`: AL is 0 when the
    // in-range distance is at or below the leader-to-aim-point range, and
    // 009C7CFE ANDs it into the latch.
    return latch && !(in.in_range_distance <= in.leader_to_aim_point);
}

// 009C7A94-009C7AB4. `FLD [ctl+39Ch]`, `FLD [approach+A8h]`,
// `FMUL qword [00D7A270]`, `FSUBP` and the min against the raw value.
float dive_bomb_decay_dive_altitude_009c7a94(float dive_altitude_a8,
                                             float control_alt_39c) noexcept {
    const float decayed =
        control_alt_39c -
        dive_altitude_a8 * static_cast<float>(dive_bomb_constant::kDiveAltitudeDecay);
    return (dive_altitude_a8 < decayed) ? dive_altitude_a8 : decayed;
}

// 007BCC80, read whole; see the header for the listing and the sign argument.
float weapon_fall_time_007bcc80(float height_above_aim_point,
                                float unit_velocity_y) noexcept {
    if (!(height_above_aim_point > 0.0f)) return 0.0f;   // 007BCC86
    const float vy = unit_velocity_y -
        dive_bomb_constant::kFallTimeVerticalBias;       // 007BCCA4, 00E08E54
    // 007BCCB6 FMUL and 007BCCDA FDIV both take the QWORD at 00CF9058, and
    // 007BCCC2/007BCCCF round the discriminant and its root to float on the way.
    const double g = dive_bomb_constant::kGravity;
    const float disc = static_cast<float>(
        static_cast<double>(vy) * vy + 2.0 * height_above_aim_point * g);
    const float root = std::sqrt(disc);
    return static_cast<float>((static_cast<double>(root) + vy) / g);
}

// 009C7D71-009C7E33, the diving arm of the approach update.
DiveBombImpactPoint dive_bomb_impact_point_009c7d71(
    const DiveBombImpactPointInputs& in) noexcept {
    DiveBombImpactPoint out;
    const float h = in.unit_position[1] - in.aim_point_y;   // 009C7B49, negated
    out.fall_time = weapon_fall_time_007bcc80(h, in.unit_velocity[1]) +
        static_cast<float>(dive_bomb_constant::kImpactPointFallTimeBias);
    out.point[0] = in.unit_position[0] + out.fall_time * in.unit_velocity[0];
    // 009C7DF1 adds the 0.0f at 00D7A258 and 009C7E33 then overwrites +DCh with
    // the aim point's own Y, so only that second store survives the tick.
    out.point[1] = in.aim_point_y;
    out.point[2] = in.unit_position[2] + out.fall_time * in.unit_velocity[2];
    return out;
}

// 00419010, five stack floats, RET 14h. Equal ordered endpoints return y0;
// otherwise the interpolation is clamped between the two y values whichever way
// round they are. Transcribed from docs/UNIT_RUDDER_CURVE.md's reconstruction.
float dive_bomb_interpolate_clamped_00419010(float x0, float y0, float x1, float y1,
                                             float x) noexcept {
    if (x1 == x0) {
        return y0;
    }
    const float v = ((x - x0) / (x1 - x0)) * (y1 - y0) + y0;
    const float hi = (y1 < y0) ? y0 : y1;
    const float lo = (y0 < y1) ? y0 : y1;
    if (v < lo) {
        return lo;
    }
    return (v <= hi) ? v : hi;
}

// 009C59BA-009C5C9B.
DiveBombAimError dive_bomb_aim_error_009c5c9b(const DiveBombAimErrorInputs& in) noexcept {
    DiveBombAimError out;
    // 009C5BEE-009C5BF7 and 009C5C27-009C5C38: one x window for both calls.
    const float x0 = in.dive_altitude_a8 +
                     static_cast<float>(dive_bomb_constant::kMoveToRangeBias);
    const float x1 = in.begin_altitude_ac + in.aim_point_height_50;
    // 009C5C49: the lead, 0 at the floor up to (approach+14h)->+5Ch high up.
    out.lead = dive_bomb_interpolate_clamped_00419010(x0, 0.0f, x1, in.lead_at_high_5c,
                                                      in.height_above_target);
    // 009C5C4E: FSUBR, so the memory operand is the minuend.
    out.along_track = std::cos(in.bearing_error) * in.planar_distance - out.lead;
    // 009C5C92: the gain, 1.0 at the floor up to (approach+14h)->+60h high up.
    out.gain = dive_bomb_interpolate_clamped_00419010(x0, 1.0f, x1, in.gain_at_high_60,
                                                      in.height_above_target);
    // 009C5C97.
    out.error = out.gain * out.along_track;
    return out;
}

// 009C608C-009C6154. Three gates in order, then the round, then the pull-out.
DiveBombAimDiveReleaseResult dive_bomb_aimdive_release_009c60f1(
    const DiveBombAimDiveReleaseInputs& in) noexcept {
    DiveBombAimDiveReleaseResult out;
    out.rearm_timer_1c = in.rearm_timer_1c;

    // 009C60AF `FCOMPI`: release only once the aircraft is below the drop floor.
    const bool below_floor = in.dive_altitude_a8 > in.altitude;
    // 009C60BB `COMISS xmm0(0.0), [esi+1Ch]` / `JB`: the re-arm must have run out.
    const bool rearmed = !(0.0f < in.rearm_timer_1c);
    // 009C60DE-009C60EC: |error| under the 25.0 double at 00CE3880.
    const bool on_aim = static_cast<double>(fold_abs(in.aim_error)) <
                        dive_bomb_constant::kAimDiveReleaseErrorLimit;

    if (below_floor && rearmed && on_aim) {
        out.released = true;        // 009C60F1, one 007BBBA0
        out.consumed_round = true;  // 009C60FB, approach+2Ch -= 1
        out.rearm_timer_1c = in.rearm_draw;  // 009C6119
    }

    // 009C6131-009C6154, run on every path: the pull-out edge the transition
    // rule reads at 009C8677.
    out.pull_out =
        in.dive_altitude_a8 *
            static_cast<float>(dive_bomb_constant::kPullOutAltitudeFraction) >
        in.altitude;
    return out;
}

// 009C5AFD-009C5B48.
bool dive_bomb_dive_abort_009c5b43(const DiveBombDiveAbortInputs& in) noexcept {
    // 009C5AFD FLD [ESP+14h] is the height above the aim point, not a range;
    // 009C5B0E/009C5B12 compare approach+D4h + approach+50h against it.
    if (!(in.release_range_d4 + in.aim_point_height_50 > in.height_above_target_14)) {
        return false;  // 009C5B18
    }
    if (!(in.unit_attitude_c64 > dive_bomb_constant::kAbortRollFloor)) {
        return false;  // 009C5B2C
    }
    // 009C5B2E FLD ST1 copies that same height, so the slope multiplies the
    // height and 009C5B3C compares the sum against ST1, the [ESP+1Ch] range.
    const float bound =
        in.height_above_target_14 *
            static_cast<float>(dive_bomb_constant::kAbortRangeSlope) +
        static_cast<float>(dive_bomb_constant::kAbortRangeBias);
    return bound > in.aim_point_distance;  // 009C5B3E
}

// 009C5689-009C57BB. coverage: complete; every gate slot's producer is traced,
// see the header. The five frame slots remain host inputs.
DiveBombAimGlideReleaseResult dive_bomb_aimglide_release_009c5777(
    const DiveBombAimGlideReleaseInputs& in) noexcept {
    DiveBombAimGlideReleaseResult out;
    out.travel_accumulator_20 = in.travel_accumulator_20;
    out.rearm_timer_1c = in.rearm_timer_1c;

    // 009C5689 COMISS XMM2,[ESI+1Ch] with XMM2 zeroed at 009C562D; 009C568D JBE
    // bails unless the rearm countdown has gone negative. ADDED, packet
    // cc8_dive_glide: the host began the gate chain at 009C569B and so had no
    // cooldown between salvoes at all.
    if (!(0.0f > in.rearm_timer_1c)) {
        return out;
    }
    out.gate_reached = 1;
    // 009C569B: the bearing to the aim point has to be within 30 degrees of the
    // aim heading. NOT a flight-path angle; see kGlideDiveAngleLimit.
    if (!(dive_bomb_constant::kGlideDiveAngleLimit > in.bearing_error_18)) {
        return out;
    }
    out.gate_reached = 2;
    // 009C56A6-009C56B8, CORRECTED: the aircraft must be BELOW the glide release
    // ceiling, not above an altitude. 009C56A6 loads [ESP+20h] first and
    // 009C56AA loads [ESP+1Ch] second, so 009C56B4's FCOMIP compares
    // `ceiling + 50.0` against the height above the aim point and 009C56B8's JBE
    // bails when the ceiling is the smaller. See the header for both producers.
    if (!(static_cast<double>(in.glide_release_ceiling) +
              dive_bomb_constant::kGlideHeightMargin >
          static_cast<double>(in.height_above_aim_point))) {
        return out;
    }
    out.gate_reached = 3;
    // 009C56BE-009C56FE: |throw - range| inside 120.0. 009C56E4's `SUBSS XMM4`
    // is the negate arm of an abs: XMM4 is loaded once in the whole body, at
    // 009C561B from the -0.0f at 00D7A208, and that load dominates 009C56E4.
    const float lateral = fold_abs(in.lateral_a - in.lateral_b);
    if (!(dive_bomb_constant::kGlideLateralLimit > static_cast<double>(lateral))) {
        return out;
    }
    out.gate_reached = 4;
    // 009C5704-009C5755, BOTH ARMS CORRECTED. 009C572F FADDP and 009C5733 FADD
    // build `lead + travel + 5.0` in ST1 while ST0 keeps the bare travel.
    // 009C5715 FLD [ESP+10h], 009C5719 FLD [ESP+20h] (the cosine 009C5708 just
    // took of the bearing error), 009C571D FMUL [ESP+14h], 009C5725 FSUB: the
    // lead is `range - cos(bearing error) * throw`, the along-track shortfall
    // of the predicted impact point against the target. It goes NEGATIVE when
    // the throw overruns, and the window below is `-4*travel - 5.0 < lead <
    // -5.0` - a deliberate overshoot of 5 m up to 4*travel + 5 m, which is what
    // walks a stick of bombs through the target.
    const float lead =
        in.lateral_b - std::cos(in.bearing_error_18) * in.lateral_a;
    const double lead_travel_margin =
        static_cast<double>(lead) + static_cast<double>(in.travel_accumulator_20) +
        dive_bomb_constant::kGlideLeadMargin;
    // 009C5743 FCOMI ST0,ST1 compares the travel with that sum and 009C5745
    // `76` JBE bails unless the travel is the greater - that is, unless
    // `lead < -5.0`. This was written `lead > 5.0`: the lead's sign backwards.
    out.lead = lead;
    if (!(static_cast<double>(in.travel_accumulator_20) > lead_travel_margin)) {
        return out;
    }
    out.gate_reached = 5;
    // 009C5747 FCHS negates the TRAVEL the FCOMI left on the stack, 009C5749
    // scales it by the 3.0 at 00D7A2B0, and 009C5751/009C5755 `76` JBE bail
    // unless the sum is the greater. This negated `lead + travel` instead.
    //
    // The pair is satisfiable exactly when `-4*travel - 5.0 < lead < -5.0`,
    // which needs a positive travel accumulator; the two as they stood were
    // mutually exclusive at the zero this host supplied, so the salvo at
    // 009C5777 could never fire at any altitude.
    if (!(lead_travel_margin >
          -static_cast<double>(in.travel_accumulator_20) *
              dive_bomb_constant::kGlideLeadScale)) {
        return out;
    }
    out.gate_reached = 6;

    // 009C575D-009C5784: the salvo. min(rounds, cap), then one request each.
    int count = in.rounds_available;
    if (count >= in.rounds_cap) {
        count = in.rounds_cap;
    }
    if (count > 0) {
        out.released = true;
        out.rounds_released = count;
    }
    // 009C5786-009C57BB runs whether or not the salvo did.
    out.rearm_timer_1c = in.rearm_draw;
    out.travel_accumulator_20 =
        in.travel_accumulator_20 +
        in.drift_rate_a4 * static_cast<float>(dive_bomb_constant::kGlideTravelScale);
    return out;
}

// 009C8200-009C8255, __fastcall(task) -> bool, RET 0.
DiveBombArmResult dive_bomb_arm_drop_009c8200(const DiveBombArmInputs& in) noexcept {
    DiveBombArmResult out;
    out.consumed = true;  // every path leaves AL = 1
    if (!in.has_bomb_ordnance_4c9 || !in.has_unit || !in.device_requests_release) {
        return out;  // 009C8252
    }
    out.queued_round = true;  // 009C822C, task+424h += 1
    if (in.current == DiveBombState::kPrepare) {
        out.armed = true;  // 009C8248, prepare+98h = 5.0f
        out.prepare_timer = dive_bomb_constant::kArmCountdown;
    }
    return out;
}

// 009C7800-009C7849, __thiscall(turndown_state, int sign).
float dive_bomb_turn_direction_009c7800(int sign, float magnitude_draw) noexcept {
    const float side = (sign >= 0) ? dive_bomb_constant::kPlusOne
                                   : dive_bomb_constant::kMinusOne;
    return magnitude_draw * side;
}

// 009C673F-009C67B0.
bool dive_bomb_flyabove_roll_in_009c67b0(float bearing_error,
                                         float clamped_slot) noexcept {
    // 009C6796 stores the folded error; 009C67A3 FCOMIP against the 1.6 and
    // 009C67A7 JA set the flag when the error is the larger.
    if (static_cast<double>(fold_abs(bearing_error)) > kFlyAboveRollInBearing) {
        return true;
    }
    // 009C67A9 COMISS 0, slot with 009C67AE JC skipping when the slot is
    // positive. The slot is already max(x, 0), so this is x <= 0.
    return !(clamped_slot > 0.0f);
}

// 009C67C7-009C680E.
bool dive_bomb_flyabove_can_dive_009c680e(float height_above_target,
                                          float release_range_d4) noexcept {
    // 009C67EA FLD [approach+D4h], 009C67F0 FXCH, 009C67F2 FCOMIP ST0,ST1 with
    // ST0 = the height and ST1 = the range, 009C67F6 JBE to the XOR EAX,EAX.
    return height_above_target > release_range_d4;
}

// 009C4220-009C447D, the attackrun tick.
DiveBombAttackRunResult dive_bomb_attackrun_tick_009c4220(
    const DiveBombAttackRunInputs& in) noexcept {
    DiveBombAttackRunResult out;
    out.lateral_offset_20 = in.lateral_offset_20;

    // 009C424A FCOMI(dt, state+1Ch) then 009C424F JC: the countdown arm runs
    // while dt is below the timer, and the re-roll arm otherwise.
    if (in.dt < in.reroll_timer_1c) {
        out.reroll_timer_1c = in.reroll_timer_1c - in.dt;  // 009C4333
    } else {
        // 009C4255-009C427B: the period is added to whatever is left, so the
        // phase carries rather than resetting.
        out.reroll_timer_1c = (in.reroll_period_18 - in.dt) + in.reroll_timer_1c;
        out.rerolled = true;
        // 009C42BD-009C42D9: the new lateral offset, negated and scaled by the
        // 30 degrees at 00CEC730.
        out.lateral_offset_20 = -in.sampler_result *
            static_cast<float>(dive_bomb_attackrun_constant::kLateralOffsetScale);
    }

    // 009C42DC-009C4305, run on both arms: the heading is the bearing to the
    // target plus the lateral offset, with mode 2.
    out.commanded_heading_2c0 = wrapped_angle_add_00438aa0(
        in.target_bearing_c0, out.lateral_offset_20);

    // 009C4311-009C4342: the distance, clamped at 2000.
    out.clamped_distance =
        (static_cast<double>(in.planar_distance_bc) <
         dive_bomb_attackrun_constant::kDistanceClamp)
            ? in.planar_distance_bc
            : static_cast<float>(dive_bomb_attackrun_constant::kDistanceClamp);

    // 009C435B-009C438B: the height margin under the 1400 ceiling, floored at 50.
    const float margin =
        static_cast<float>(dive_bomb_attackrun_constant::kMarginCeiling) - in.altitude;
    out.height_margin =
        (static_cast<double>(margin) >= dive_bomb_attackrun_constant::kMarginFloor)
            ? margin
            : static_cast<float>(dive_bomb_attackrun_constant::kMarginFloor);

    // 009C4397-009C43CD: the throttle, interpolated on the margin over the
    // clamped distance. Flat 0.4 below a tenth, full at just over a third.
    out.throttle_ratio = (out.clamped_distance != 0.0f)
        ? out.height_margin / out.clamped_distance : 0.0f;
    out.descent_scale = dive_bomb_interpolate_clamped_00419010(
        dive_bomb_attackrun_constant::kDescentScaleRatioLow,
        dive_bomb_attackrun_constant::kDescentScaleAtLow,
        dive_bomb_attackrun_constant::kDescentScaleRatioHigh,
        dive_bomb_attackrun_constant::kDescentScaleAtHigh,
        out.throttle_ratio);

    // 009C43ED-009C4401: the altitude base handed to 009FBA50.
    out.commanded_altitude_base = in.begin_altitude_ac + in.aim_point_height_50;
    return out;
}

// 009C4530-009C4575. 00BF857A is the CRT x87 helper with ST(1) = x and
// ST(0) = y, so the call is fmod(bank, 2pi); the pair of compares that follows
// pulls the result into (-pi, pi].
float dive_bomb_wrap_signed_pi_009c4551(float angle) noexcept {
    float a = static_cast<float>(
        std::fmod(static_cast<double>(angle), dive_bomb_constant::kTwoPi));
    // 009C4557 FCOMIP(-pi, a) then 009C4561 JC: the add runs when -pi >= a.
    if (!(dive_bomb_turndown_constant::kWrapLow < static_cast<double>(a))) {
        a = static_cast<float>(static_cast<double>(a) + dive_bomb_constant::kTwoPi);
    } else if (!(static_cast<double>(a) <= dive_bomb_turndown_constant::kWrapHigh)) {
        // 009C456B FCOMI(a, pi) then 009C456D JBE: the subtract runs when a > pi.
        a = static_cast<float>(static_cast<double>(a) - dive_bomb_constant::kTwoPi);
    }
    return a;
}

// 009C44F0-009C4736, the turndown tick.
DiveBombTurnDownResult dive_bomb_turndown_tick_009c44f0(
    const DiveBombTurnDownInputs& in) noexcept {
    DiveBombTurnDownResult out;

    // 009C4512-009C4524, before any branch: the level-flight speed command and the
    // one-shot the planner spends. docs/PILOT_THROTTLE_CUT_RAISER.md shows the
    // pair (+2B4h, +2D8h) is one command; nothing here touches +278h/+27Ch.
    out.speed_2b4 = in.desired_speed;

    const float wrapped = dive_bomb_wrap_signed_pi_009c4551(in.bank_c68);
    out.folded_bank = fold_abs(wrapped);  // 009C457D-009C45A5
    // 009C45BD FSUBP with ST1 = pi: the angle still to roll through to inverted.
    const float to_inverted =
        dive_bomb_turndown_constant::kPi - out.folded_bank;
    // 009C45C7-009C45DD: negatives are floored at zero.
    out.angle_to_inverted = (to_inverted >= 0.0f) ? to_inverted : 0.0f;

    if (!in.rolled_latch_1c) {  // 009C45A9 CMP / JNZ
        // 009C45B9 FCOMIP(0.8, |bank|) then 009C45BB JBE: the roll arm runs
        // only while the bank is still under 45.8 degrees.
        if (static_cast<double>(out.folded_bank) <
            dive_bomb_turndown_constant::kRollHandOver) {
            // 009C45EA-009C460F. The interpolation eases the roll off inside
            // 30 degrees of inverted; at these banks it clamps at 1.0, so the
            // command is the full signed magnitude 009C7800 drew.
            const float gain = dive_bomb_interpolate_clamped_00419010(
                dive_bomb_turndown_constant::kEaseOffAngle, 1.0f, 0.0f, 0.0f,
                out.angle_to_inverted);
            out.wrote_roll = true;
            out.roll_290 = gain * in.roll_command_18;
        } else {
            // 009C4637: hand the roll axis back, +2C4h = pi and mode 1.
            out.released_roll = true;
        }
        // 009C4654 COMISS then 009C465B JBE: latch once past 150 degrees.
        if (out.folded_bank > dive_bomb_turndown_constant::kLatchBank) {
            out.latch_1c_set = true;
        }
        // 009C4660-009C4687, run on both arms. 009C4687 JBE sends the 20 degree
        // band and above to the altitude arm.
        if (fold_abs(in.pitch_c64) < dive_bomb_turndown_constant::kPitchHoldBand) {
            out.wrote_pitch = true;   // 009C4689: +29Ch = 0, +2A0h = 1, +2D0h = 0
            out.pitch_29c = 0.0f;
        } else {
            out.wrote_altitude_hold = true;  // 009C46AA: +2BCh = 0, +2D0h = 2
        }
        return out;
    }

    // 009C46C9-009C4736, once latched: release the roll and pull the nose down.
    out.released_roll = true;
    out.wrote_pitch = true;
    out.pitch_29c = dive_bomb_interpolate_clamped_00419010(
        dive_bomb_turndown_constant::kEaseOffAngle, 0.0f,
        dive_bomb_turndown_constant::kFullPitchAngle, 1.0f,
        out.angle_to_inverted);
    return out;
}

// 009C7F00-009C7FD6, the goaway completion rule, read whole.
bool dive_bomb_goaway_complete_009c7f00(
    const DiveBombGoAwayCompleteInputs& in) noexcept {
    // 009C7F03-009C7F16: the distance term.
    double term = static_cast<double>(in.travel_20) *
                  dive_bomb_constant::kGoAwayDistanceScale;
    // 009C7F51-009C7F7A: all three flags set scales it a second time by the same
    // 0.9 that 009C7F09 left on the x87 stack; 009C7F72 returns false outright
    // when the first two are set and the ordnance flag is not.
    if (in.control_flag_369 && in.global_e17bf2) {
        if (!in.has_bomb_ordnance_d1) {
            return false;   // 009C7F74 XOR AL,AL
        }
        term *= dive_bomb_constant::kGoAwayDistanceScale;
    }
    // 009C7F23-009C7F46: the ceiling, the smaller of the two.
    const float sum = in.begin_altitude_ac + in.aim_point_height_50;
    const float ceiling = (in.cruise_altitude_398 <= sum) ? in.cruise_altitude_398 : sum;
    // 009C7F8D `76` JBE: the range has to have opened past the term.
    if (!(static_cast<double>(in.planar_distance_bc) > term)) {
        return false;
    }
    // 009C7FB2/009C7FC2 `76` JBE: and the aircraft has to have climbed back to
    // within the 100.0 at 00D7A220 of that ceiling. This half was missing.
    return static_cast<double>(in.altitude) >
           static_cast<double>(ceiling) - dive_bomb_constant::kMoveToRangeBias;
}

// 009C4B44-009C4C06, the goaway tick's climb-out.
DiveBombGoAwayCommand dive_bomb_goaway_climb_009c4b44(
    const DiveBombGoAwayInputs& in) noexcept {
    DiveBombGoAwayCommand out;
    // 009C4BB3: InterpolateClamped(60.0, climb angle, 300.0, 0.0, altitude) -
    // the full climb angle below 60 m, easing to level by 300 m.
    const float eased = dive_bomb_interpolate_clamped_00419010(
        dive_bomb_goaway_constant::kClimbFullAltitude, in.climb_angle_1ec,
        dive_bomb_goaway_constant::kClimbEaseAltitude, 0.0f, in.altitude);
    // 009C4B61 is a second curve over the same 300.0 upper endpoint whose y1 and
    // interpolant were not traced; 009C4BC4/009C4BC8 `77` JA take the larger of
    // the two. SUBSTITUTION, labelled: the same curve stands in for it, so the
    // max is the curve itself and the command is never weaker than the image's.
    out.pitch_target_2bc = eased;
    out.pitch_mode_2d0 = 1;      // 009C4BE8, EBX
    out.bank_target_2c4 = 0.0f;  // 009C4BFE, the XORPS zero
    out.heading_mode_2cc = 1;    // 009C4C06, EBX
    out.air_brake_mode_2d8 = 0;  // 009C4CA7 / 009C4CE7
    return out;
}

// 009C542C-009C5450, the aimglide tick's heading arm.
DiveBombAimGlideCommand dive_bomb_aimglide_command_009c542c(
    const DiveBombAimGlideCommandInputs& in) noexcept {
    DiveBombAimGlideCommand out;
    out.wrote_heading = true;
    out.heading_2c0 = in.heading_to_aim_point;  // 009C5435 / 009C5442
    out.heading_mode_2cc = 2;                   // 009C53DD / 009C5450
    return out;
}

// 009C6DCD-009C6DEF, the flyabove tick's heading arm.
DiveBombFlyAboveCommand dive_bomb_flyabove_command_009c6dcd(
    const DiveBombFlyAboveCommandInputs& in) noexcept {
    DiveBombFlyAboveCommand out;
    // 009C6DCD CMP byte ptr [ESI+1Ch],0 and 009C6DDA `75` JNZ: a non-zero flag
    // jumps past both stores to 009C6DF5.
    if (in.suppress_heading_1c) {
        return out;
    }
    out.wrote_heading = true;
    out.heading_2c0 = in.heading_to_aim_point;  // 009C6DE7
    out.heading_mode_2cc = 2;                   // 009C6DD5 / 009C6DEF
    return out;
}

// 009C6E10-009C6F91, the flyabove tick's altitude arm. Packet cc8_dive_entry;
// the walk and every branch byte are in include/bsp/dive_bomb_task.hpp.
DiveBombFlyAboveAltitudeCommand dive_bomb_flyabove_altitude_009c6e10(
    const DiveBombFlyAboveAltitudeInputs& in) noexcept {
    DiveBombFlyAboveAltitudeCommand out;
    const float base = in.begin_altitude_ac + in.aim_point_height_50;  // 009C6E48
    // 009C6491 TEST EAX,EAX with 009C64A6 `74` JZ: the control block's ordered
    // cruise altitude when there is one, the approach's own base otherwise.
    float c = in.has_control_block_0c ? in.cruise_altitude_398 : base;
    // 009C655F-009C6589. The compare is against 1.1 * R and the store is the
    // bare R, so the clamp lands 10 per cent under where it triggers.
    const float release_limit = in.new_release_mul_40 * in.release_altitude_a8;
    if (static_cast<double>(c) >
        static_cast<double>(release_limit) *
            dive_bomb_flyabove_constant::kNewReleaseMargin) {
        c = release_limit;  // 009C6580-009C6589
    }
    out.limit_c = c;
    // 009C6E1C-009C6E44: the error is measured against approach+ACh, NOT
    // against the base the target uses - the two differ by approach+50h.
    const float reference_height =
        (in.begin_altitude_ac <= c) ? in.begin_altitude_ac : c;  // 009C6E26 JBE
    out.height_error = in.height_above_aim_b - reference_height;
    // 009C6E55-009C6E71.
    out.target_altitude = (base <= c) ? base : c;
    // 009C6E77-009C6EA1: 0.15 * C against approach+B0h, the smaller.
    const float scaled = static_cast<float>(
        static_cast<double>(c) * dive_bomb_flyabove_constant::kDeadBandScale);
    out.dead_band = (scaled <= in.alt_span_b0) ? scaled : in.alt_span_b0;
    // 009C6EBD / 009C6F1B: the FLD1 against A picks the divisor.
    const float divisor =
        (dive_bomb_flyabove_constant::kClimbReferenceCap <= in.planar_distance)
            ? in.planar_distance
            : dive_bomb_flyabove_constant::kClimbReferenceCap;
    // 009C6EAA COMISS 0, err with 009C6EAF `76` JBE: the fall-through is err < 0.
    if (out.height_error < 0.0f) {
        const float v = static_cast<float>(
            -static_cast<double>(out.height_error) *
            dive_bomb_flyabove_constant::kClimbGain /
            static_cast<double>(divisor));
        // 009C6EF5 `77` JA keeps the cap when the quotient is the larger.
        out.reference =
            (v > dive_bomb_flyabove_constant::kClimbReferenceCap)
                ? dive_bomb_flyabove_constant::kClimbReferenceCap : v;
        return out;
    }
    // 009C6F15 `76` JBE: inside the dead band the arm writes a bare zero pitch
    // and never reaches 009FB800. XMM0 is the zero 009C6EA7 put there.
    if (out.height_error <= out.dead_band) {
        out.level_arm = true;
        return out;
    }
    const float v = static_cast<float>(
        (static_cast<double>(in.planar_distance) +
         static_cast<double>(in.planar_distance)) /
        static_cast<double>(divisor));  // 009C6F33 FADD ST0,ST0 then 009C6F3B FDIV
    // 009C6F4D FCOMIP against the 0.8 at 00CE3D40, 009C6F51 `76` JBE.
    out.reference = (static_cast<double>(v) <=
                     dive_bomb_flyabove_constant::kLeaveSpanScale)
                        ? v : dive_bomb_flyabove_constant::kDiveReferenceCap;
    return out;
}

// 009C658D-009C65FD, the span both flyabove flags key on. The threshold is
// built from the HEIGHT and the span is the RANGE less that threshold: the two
// arguments are different quantities and 009C65DB's FSUBP is what says so.
DiveBombFlyAboveSpan dive_bomb_flyabove_span_009c65fd(
    float height_above_aim_point, float planar_range) noexcept {
    DiveBombFlyAboveSpan out;
    // 009C658D FLD [00D7A220], 009C659B FCOMIP, 009C65A9 JBE: the floor takes
    // the height when 100.0 is the smaller, and the 100.0 at 00CE3D08 otherwise.
    out.floored_height =
        (static_cast<double>(dive_bomb_constant::kMoveToRangeBias) <=
         static_cast<double>(height_above_aim_point))
            ? height_above_aim_point
            : dive_bomb_flyabove_constant::kHeightFloor;
    // 009C65C1-009C65D1.
    out.threshold = static_cast<float>(
        static_cast<double>(out.floored_height) *
            dive_bomb_flyabove_constant::kHeightScale +
        dive_bomb_flyabove_constant::kHeightBias);
    // 009C65D5-009C65FD: the difference, floored at zero. CORRECTION, packet
    // cc8_dive_heading: the minuend is R, the PLANAR RANGE at 009C63A6, not the
    // height. The x87 stack, walked forward from 009C64EE with every push and
    // pop accounted (local/f.ps1, zero join conflicts over the whole body):
    //   009C64EE FLD [ESP+28h]  ST0=R
    //   009C64F4 FLD [ESP+38h]  ST0=B ST1=R
    //   009C64F8 FLD [ESP+3Ch]  ST0=X ST1=B ST2=R   (all four BL paths rejoin
    //                                                at 009C6532 with this)
    //   009C6578 FCOMPI ST(1) pop / 009C657A FSTP ST(0)  -> ST0=B ST1=R
    //   009C659B FCOMPI ST(1) pop / 009C659D FSTP ST(0)  -> ST0=R   B is gone
    //   009C65D1 FSTP [ESP+10h]  ST0=R   ([ESP+10h] = threshold)
    //   009C65D5 FLD  [ESP+10h]  ST0=S ST1=R
    //   009C65D9 FLD  ST(0)      ST0=S ST1=S ST2=R
    //   009C65DB FSUBP ST(2)     ST2 = R - S
    // [ESP+28h] is written once, at 009C63A6 (the 00BF7030 sqrt at 009C6399),
    // and never overwritten in the 949-instruction body.
    //
    // The earlier reading took the minuend as the height. That is where the
    // "flyabove leaves at B <= 666.7 m" in docs/HANDOFF_DIVE_BOMB_ENTRY.md came
    // from: max(B - (0.7B + 200), 0) is zero at B = 666.7. No such number is in
    // the image - the arm is a range-to-go test against a glide slope.
    const float difference = planar_range - out.threshold;
    out.span = (difference > 0.0f) ? difference : 0.0f;
    return out;
}

// 009C66D5-009C66E7.
bool dive_bomb_flyabove_leave_009c66e3(float bearing_error,
                                       const DiveBombFlyAboveSpan& span,
                                       float attack_distance_b4) noexcept {
    // 009C6615-009C6621: the far endpoint. FSUBRP leaves (+B4h * 0.8) - S.
    const float far_endpoint = static_cast<float>(
        static_cast<double>(attack_distance_b4) *
            dive_bomb_flyabove_constant::kLeaveSpanScale -
        static_cast<double>(span.threshold));
    // 009C663E: InterpolateClamped(0, 20 deg, far, pi, span).
    const float tolerance = dive_bomb_interpolate_clamped_00419010(
        0.0f, dive_bomb_flyabove_constant::kLeaveToleranceLow, far_endpoint,
        dive_bomb_flyabove_constant::kLeaveTolerancePi, span.span);
    // 009C6453's operand is the folded bearing error; 009C66DD FCOMIP and
    // 009C66E1 `76` JBE leave only the strictly-greater case setting +1Ah.
    return fold_abs(bearing_error) > tolerance;
}

// 009C664B-009C6674.
float dive_bomb_flyabove_span_dead_band_009c6674(float span) noexcept {
    return dive_bomb_interpolate_clamped_00419010(
        0.0f, dive_bomb_flyabove_constant::kSpanDeadBandLow,
        dive_bomb_flyabove_constant::kSpanDeadBandSpan, 0.0f, span);
}

// 009C64EE-009C6530, BL. Header carries the branch table and the evidence that
// 009C64EC's kind 14h is the recon plane; this is the transcription.
bool dive_bomb_flyabove_bank_arm_009c6530(bool is_recon_plane,
                                          float release_range_d4,
                                          float limit_c,
                                          float attack_distance_b4,
                                          float lead_range_r,
                                          float height_above_aim_b) noexcept {
    // 009C64F2 TEST AL,AL with 009C64FC `75` JNE -> 009C6530 XOR BL,BL.
    if (is_recon_plane) return false;
    // 009C650E FCOMI with ST0 = approach+D4h and ST1 = C, 009C6510 `77` JA.
    if (release_range_d4 > limit_c) return false;
    // 009C6518 FCOMPI ST(4) with ST0 = approach+B4h and ST4 = R, then
    // 009C651A `76` JBE -> 009C652A MOV BL,1.
    if (!(attack_distance_b4 > lead_range_r)) return true;
    // 009C651C FXCH ST(2) puts B in ST0 and approach+D4h in ST2, 009C651E
    // FCOMI ST0,ST2, 009C6522 `72` JB -> 009C6530 XOR BL,BL.
    return !(height_above_aim_b < release_range_d4);
}

// 009C6857-009C6923, the bank arm: the roll-in latch and the dead-band width.
DiveBombFlyAboveBank dive_bomb_flyabove_bank_009c6857(
    const DiveBombFlyAboveBankInputs& in) noexcept {
    DiveBombFlyAboveBank out;
    out.latched_1c = in.latched_1c;
    out.dead_band_t = in.span_dead_band;
    // 009C67BF TEST BL,BL with 009C67C1 `0F84` JZ: without BL the body leaves
    // for the dead band at 009C6A37 and T stays as 009C6674 left it.
    if (!in.bank_arm_bl) {
        return out;
    }
    // 009C6861 CMP [ESI+1Ch],0 with 009C6865 `0F85` JNZ: once latched the arm
    // is skipped whole. T is then irrelevant, because 009C6923's `0F84` JZ is
    // not taken either and the dead band at 009C6A37 never runs.
    if (in.latched_1c) {
        return out;
    }
    const double eabs = static_cast<double>(in.bearing_error_abs);
    const double range = static_cast<double>(in.lead_range_r);
    // 009C6847-009C6853, base[ESP+30h]: the turn circle the roll-in must fit.
    const float turn_circle = static_cast<float>(
        static_cast<double>(in.turn_circle_radius) *
        dive_bomb_flyabove_constant::kTurnCircleMul);
    // 009C6857-009C687D. FSIN, then (sin + 0.5 * sin), then * R, stored as a
    // float at 009C687D and reloaded at 009C6885.
    out.cross_track = static_cast<float>(
        std::sin(eabs) * (1.0 + dive_bomb_flyabove_constant::kRollInSinHalf) *
        range);
    // 009C6889 FCOMIP ST0,ST1 with ST0 = cross-track and ST1 = the turn circle,
    // 009C688D `76` JBE: only a strictly wider offset leaves.
    if (out.cross_track > turn_circle) {
        out.clear_roll_in_19 = true;  // 009C688F
        out.dead_band_t = 0.0f;       // 009C6893, the XORPS zero of 009C67BC
        return out;
    }
    out.weapon_select_3 = true;  // 009C68A0 approach+CCh = 3
    // 009C68AA-009C68C8. FCOS, * R, less 120 m, stored as a float.
    out.along_track = static_cast<float>(std::cos(eabs) * range);
    const float margin = static_cast<float>(
        std::cos(eabs) * range - dive_bomb_flyabove_constant::kRollInAlongTrack);
    // 009C68D2 FCOMIP with ST0 = margin and ST1 = the FLDZ zero, 009C68D4 `76`
    // JBE -> 009C6919, the latch.
    if (!(margin > 0.0f)) {
        out.latched_1c = true;
        return out;
    }
    out.clear_roll_in_19 = true;  // 009C68E1
    // 009C68D6-009C6911. The argument window the `SUB ESP,14h` at 009C68DA
    // opens: arg5 = R (the FXCH at 009C68D8 brings it to ST0), arg4 = 10 deg,
    // arg3 = classDesc+268h UNMULTIPLIED, arg2 = 100 deg, arg1 = the FLDZ zero
    // that survives the 009C68D2 pop.
    out.dead_band_t = dive_bomb_interpolate_clamped_00419010(
        0.0f, dive_bomb_flyabove_constant::kBankDeadBandNear,
        in.turn_circle_radius, dive_bomb_flyabove_constant::kBankDeadBandFar,
        in.lead_range_r);
    return out;
}

// 009C6A37-009C6A7F.
float dive_bomb_flyabove_dead_band_009c6a37(float bearing_error,
                                            float half_width_t) noexcept {
    // 009C6A43 COMISS XMM2,XMM0 with XMM0 zero and 009C6A46 `76` JBE: a
    // non-positive T skips the arm and base[ESP+44h] keeps the raw bearing, so
    // the caller's delta is the untouched bearing error.
    if (!(half_width_t > 0.0f)) {
        return bearing_error;
    }
    // 009C6A48 COMISS XMM1,XMM0 with 009C6A4B `76` JBE.
    if (bearing_error > 0.0f) {
        // 009C6A4D-009C6A51 E - T; 009C6A59 FLD then 009C6A5D FLDZ put the zero
        // in ST0, so 009C6A73's FCOMIP compares 0 with E - T and 009C6A77 `77`
        // JA keeps the XORPS zero when the difference went negative.
        const float shrunk = bearing_error - half_width_t;
        return (0.0f > shrunk) ? 0.0f : shrunk;
    }
    // 009C6A61-009C6A65 T + E; 009C6A6D FLDZ then 009C6A6F FLD put the sum in
    // ST0, so the same JA keeps the zero when the sum went positive.
    const float shrunk = half_width_t + bearing_error;
    return (shrunk > 0.0f) ? 0.0f : shrunk;
}

// 009C6D6F-009C6DC8.
float dive_bomb_flyabove_slew_009c6d6f(float heading_c, float desired_a,
                                       float limit_l) noexcept {
    // 009C6D6F SubtractWrappedAngle(A, C) -> base[ESP+1Ch].
    const float delta = wrapped_angle_subtract_00438b10(desired_a, heading_c);
    // 009C6D78-009C6DB0: 009C6D7E FCHS builds -L, 009C6D8C FCOMPI compares -L
    // with delta and 009C6D8E `76` JBE lets it through, then 009C6D9C compares
    // delta with L and 009C6DA0 `76` JBE lets it through.
    float clamped = delta;
    if (!(-limit_l <= clamped)) {
        clamped = -limit_l;
    } else if (clamped > limit_l) {
        clamped = limit_l;
    }
    // 009C6DC8 AddWrappedAngle(C, clamped) -> base[ESP+44h] -> cmd+2C0h.
    return wrapped_angle_add_00438aa0(heading_c, clamped);
}

// 009C4F80-009C5177, the heading the two aim states steer on. Header carries
// the argument; this is the transcription.
float dive_bomb_aim_heading_009c4f80(
    const DiveBombAimHeadingInputs& in) noexcept {
    // 009C4F90-009C4FB1, the same abs-fold the rest of the class uses.
    const float folded_bank = fold_abs(in.bank_c68);

    // 009C4FBF COMISS then 009C4FC6 `76` JBE: pitch at or below -40 degrees
    // takes the body-axis arm at 009C5010.
    if (in.pitch_c64 > dive_bomb_constant::kAimHeadingSteepPitch) {
        // 009C4FD5 FLD qword [00CE3830], 009C4FDB FLD |bank|, 009C4FDF FCOMIP
        // ST0,ST1 with ST0 = |bank| and ST1 = pi/2, 009C4FE3 `76` JBE: only a
        // bank strictly past pi/2 falls through to the add.
        if (static_cast<double>(folded_bank) >
            dive_bomb_constant::kAimHeadingInvertedBank) {
            // 009C4FE9 pushes the pi at 00D7A264 as the SECOND argument and
            // 009C4FF6 the heading as the first: add(heading, pi).
            return wrapped_angle_add_00438aa0(
                in.heading_c6c, dive_bomb_constant::kAimHeadingHalfTurn);
        }
        // 009C516C, the arm that returns the raw heading untouched.
        return in.heading_c6c;
    }

    // 009C5053-009C505B: atan2 of the transformed vector's z over its x, i.e.
    // of the body +Y row. 009C5068 FSUBR makes it pi/2 - atan2, and
    // 009C5072-009C507C add 2pi to a negative - the same bearing form
    // 009C7B8A builds for the target.
    float bearing = static_cast<float>(
        dive_bomb_constant::kHalfPi -
        std::atan2(static_cast<double>(in.body_up_z),
                   static_cast<double>(in.body_up_x)));
    if (bearing < 0.0f) {
        bearing += static_cast<float>(dive_bomb_constant::kTwoPi);
    }
    return bearing;
}

// 009C5C9F-009C5DB2, the aimdive tick's steering.
//
// PARTIAL, and the partial part is the roll's interpolant. The image draws TWO
// bearing errors, from two calls to approach->vtable[0] at 009C594C and
// 009C5988: one against the latched target the constructor parked at
// approach+D8h/+DCh/+E0h (009C4065-009C407D), one against the aircraft's own
// position at unit+FCh/+100h/+104h. The default roll arm interpolates the
// first; the wider arm 009C5D24-009C5D31 selects the second when the error is
// positive and a folded angle is inside the 60 degrees at 00D05AAC. This host
// re-reads the commanded target each tick and keeps ONE bearing, so it passes
// that one and takes the default band. The wide arm is named, not bound.
DiveBombAimDiveSteerResult dive_bomb_aimdive_steer_009c5c9f(
    const DiveBombAimDiveSteerInputs& in) noexcept {
    DiveBombAimDiveSteerResult out;

    // 009C5BD4 COMISS pose+C64h against 00CEC728 with 009C5BDB `0f 87` JA
    // straight to 009C5CEF. Shallower than 30 degrees nose-down (and that
    // includes every nose-up attitude) is a full -1.0 with no aim error
    // computed, so [ESP+5Ch] keeps the planar distance the second sqrt left
    // there and it is that, not the error, that the later reads see.
    out.steep_gate_fired = in.pitch_c64 > dive_bomb_constant::kAimDiveSteepGateAngle;
    if (out.steep_gate_fired) {
        out.pitch_29c = -1.0f;                        // 009C5CEF, 00D7A260
        out.error_slot_5c = in.planar_distance_slot_5c;
    } else {
        out.error_slot_5c = in.aim_error;             // 009C5C9B
        // 009C5C9F FLDZ, 009C5CA5 FCOMI ST0,ST1, 009C5CA9 JBE: the sign of the
        // aim error picks the gain, and each arm clamps at its own end of the
        // stick.
        if (in.aim_error > 0.0f) {
            const float demand = in.aim_error * in.pitch_gain_positive_64;
            // 009C5CB6 FLD1, 009C5CB8 FCOMIP, 009C5CBC JBE.
            out.pitch_29c = (demand < 1.0f) ? demand : 1.0f;
        } else {
            const float demand = in.aim_error * in.pitch_gain_negative_68;
            // 009C5CD7 FLD [00D7A260], 009C5CE1 FCOMIP, 009C5CE5 JBE.
            out.pitch_29c = (demand > -1.0f) ? demand : -1.0f;
        }
    }

    // 009C5D0E COMISS against the 0.0f at 00D7A218 with 009C5D22 `76` JBE, then
    // 009C5D2C COMISS the 60 degrees at 00D05AAC against |pose+C68h| with
    // 009C5D31 `76` JBE. The wide band is taken only while the aircraft is still
    // inside 60 degrees of bank AND short of its aim point; banked over or past
    // it, the tighter band applies.
    // 009C5D08 reloads [ESP+5Ch], so the gated arm tests the planar distance.
    out.used_wide_band = out.error_slot_5c > 0.0f &&
        fold_abs(in.bank_c68) < dive_bomb_constant::kAimDiveRollBandAngle;
    const float band = out.used_wide_band
        ? dive_bomb_constant::kAimDiveRollBandWide   // 009C5D48 / 009C5D58
        : dive_bomb_constant::kAimDiveRollBand;      // 009C5D75 / 009C5D85
    // 009C5D8E: InterpolateClamped(-band, 1.0, band, -1.0, x). Falling, like
    // every other roll map in this bot: a positive bearing error gives a
    // negative stick. The x the two arms load is not the same slot: 009C5D33
    // takes [ESP+18h] and 009C5D60 takes [ESP+24h].
    out.roll_290 = dive_bomb_interpolate_clamped_00419010(
        -band, 1.0f, band, -1.0f,
        out.used_wide_band ? in.bearing_error_wide_18 : in.bearing_error);
    return out;
}

// 009C3FFB-009C4045, the tail of the approach constructor 009C3EA0.
float dive_bomb_dive_entry_height_009c4045(float dive_altitude_a8,
                                           float begin_altitude_ac) noexcept {
    // 009C400F FLD ST0 / 009C4011 FADDP ST2,ST0 / 009C4013 FXCH / 009C4015 FMUL:
    // the sum of the two altitudes, halved.
    const float mean = static_cast<float>(
        (static_cast<double>(begin_altitude_ac) +
         static_cast<double>(dive_altitude_a8)) *
        dive_bomb_constant::kDiveEntryHeightMean);
    // 009C401F FADD, on the copy of +A8h the FXCH left behind.
    const float margin = static_cast<float>(
        static_cast<double>(dive_altitude_a8) +
        dive_bomb_constant::kDiveEntryHeightMargin);
    // 009C4031 FCOMIP then 009C4035 JBE: the larger of the two wins.
    return (margin > mean) ? margin : mean;
}

// 009C7EA0-009C7EF2, __fastcall(state) -> bool.
bool dive_bomb_turndown_complete_009c7ea0(float attitude_c64,
                                          float attitude_c68) noexcept {
    float folded = attitude_c68;
    if (folded <= dive_bomb_constant::kZero) {
        folded = dive_bomb_constant::kNegativeZero - folded;
    }
    if (attitude_c64 >= dive_bomb_constant::kTurnDownPitchComplete &&
        (attitude_c64 >= dive_bomb_constant::kMinusOne ||
         folded <= dive_bomb_constant::kTurnDownInvertedBank)) {
        return false;
    }
    return true;
}

// 009C880C-009C8822: `FLD [ESP+14h]`, `FST [ESP+4]`, `FSUB qword [00D7A220]`.
float dive_bomb_arm_move_to_near_009c8814(float range_near_4a4) noexcept {
    return range_near_4a4 - static_cast<float>(dive_bomb_constant::kMoveToRangeBias);
}

// 009C8794-009C87D7: aimdive, aimglide, flyabove, turndown.
bool dive_bomb_arm_is_diving_009c87d2(DiveBombState state) noexcept {
    return state == DiveBombState::kAimDive || state == DiveBombState::kAimGlide ||
           state == DiveBombState::kFlyAbove || state == DiveBombState::kTurnDown;
}

// 009C884E-009C88C9. Six states refuse the passthrough: aimdive, aimglide,
// attackrun, flyabove, turndown and prepare.
bool dive_bomb_manual_passthrough_009c88c4(int rounds_pending_424, bool has_unit,
                                           bool device_requests_release,
                                           DiveBombState current) noexcept {
    if (rounds_pending_424 <= 0 || !has_unit || !device_requests_release) {
        return false;
    }
    switch (current) {
        case DiveBombState::kAimDive:
        case DiveBombState::kAimGlide:
        case DiveBombState::kAttackRun:
        case DiveBombState::kFlyAbove:
        case DiveBombState::kTurnDown:
        case DiveBombState::kPrepare:
            return false;
        default:
            return true;
    }
}

// 009C8920-009C8A8A, task vtable slot +54h.
DiveBombCruiseProfileResult dive_bomb_cruise_profile_009c8920(
    const DiveBombCruiseProfileInputs& in) noexcept {
    DiveBombCruiseProfileResult out;
    out.attack_distance_4b0 = in.current_attack_distance_4b0;
    if (!in.unit_lacks_follow_target) {
        return out;  // step 2: a following aircraft keeps its own profile
    }
    if (!in.overridden_38d && in.control_alt_380 < 0.0f && !in.one_shot_3a9) {
        out.wrote_394 = true;
        out.altitude_394 = in.cruising_altitude;
        out.dirty_3ad = true;
    }
    if (!in.overridden_38c && in.control_alt_37c < 0.0f && !in.one_shot_3aa) {
        out.wrote_398 = true;
        out.altitude_398 = in.begin_altitude;
        out.dirty_3ad = true;
    }
    // Step 6: divebomb and torpedo only.
    if (!in.overridden_38e && in.control_alt_384 < 0.0f && !in.one_shot_3ab) {
        out.wrote_39c = true;
        out.altitude_39c = dive_bomb_constant::kCruisingAltitudeThird;
        out.dirty_3ad = true;
    }
    // Step 7: the attack-distance clamp.
    // 009C8A1B-009C8A5E: task+4B0h = max(task+4B0h, tuning+4C4h * task+41Ch).
    // The divebomb clamp target is +4B0h, not the +43Ch docs/BOT_TASKS.md gives
    // for the depth charge.
    const float wanted = in.attack_distance * in.speed_ratio_41c;
    if (wanted > out.attack_distance_4b0) {
        out.attack_distance_4b0 = wanted;
    }
    // 009C8A7C: the profile ends by running 009C7A80(approach, 0.0f, false).
    out.reran_approach_update = true;
    return out;
}

// 009C8A90-009C8B9E, task vtable slot +1Ch.
bool dive_bomb_should_break_off_009c8a90(const DiveBombBreakOffInputs& in) noexcept {
    if (!in.base_0099c230) {
        return false;
    }
    if (!in.has_latched_target || in.target_dead_5d) {
        return true;
    }
    if ((!in.control_flag_369 || !in.global_e17bf2) && !in.has_bomb_ordnance_4c9) {
        if (in.safe_distance * in.speed_ratio_41c <= in.distance_to_target) {
            return true;
        }
    }
    return false;
}

// 009C8790-009C88D5, the per-tick arm, in the native order.
DiveBombArmTickResult dive_bomb_task_arm_009c8790(DiveBombTaskHost& host,
                                                  DiveBombTaskContext& ctx,
                                                  float dt) {
    DiveBombArmTickResult out;
    out.state_before = ctx.current;

    // Step 1, 009C87A3: the plan-step scratch.
    host.set_plan_step_scratch(ctx.task, 0xFF);

    // Step 2, 009C87D2 then 009C87EA: the diving predicate and the approach
    // update, which is where approach+D0h and approach+D1h come from.
    out.was_diving = dive_bomb_arm_is_diving_009c87d2(ctx.current);
    host.update_dive_bomb_approach_009c7a80(ctx.approach, dt, out.was_diving);

    // Step 3, 009C87EF-009C8825: refresh the moveto ranges.
    const float near_raw = host.arm_move_to_range(ctx.task);
    out.move_to_near = dive_bomb_arm_move_to_near_009c8814(near_raw);
    out.move_to_far = near_raw;
    out.move_to_speed = host.arm_move_to_speed(ctx.task);
    host.refresh_move_to_ranges(host.move_to_state(ctx.task), out.move_to_near,
                                out.move_to_far, out.move_to_speed);

    // Step 4, 009C8834: the transition rule, before the state's own tick.
    const DiveBombTransitionInputs inputs = host.read_transition_inputs(ctx.task);
    const DiveBombTransitionResult trans = dive_bomb_next_state_009c83e0(inputs);
    if (trans.wrote_turn_direction) {
        host.write_turn_direction(
            ctx.task,
            dive_bomb_turn_direction_009c7800(
                trans.turn_side,
                host.uniform_between(dive_bomb_constant::kTurnMagnitudeLow,
                                     dive_bomb_constant::kTurnMagnitudeHigh)));
    }
    if (trans.changed) {
        host.set_dive_bomb_state(ctx.task, trans.next);
        ctx.current = trans.next;
        out.transitioned = true;
    }
    out.state_after = ctx.current;

    // Step 5, 009C884C: the current state's tick, state->vtable[+Ch].
    host.tick_state(ctx.state, dt);
    if (ctx.current == DiveBombState::kAimDive) {
        out.ran_aimdive = true;
        out.aimdive = dive_bomb_aimdive_release_009c60f1(
            host.read_aimdive_inputs(ctx.state, dt));
        host.apply_aimdive_result(ctx.state, out.aimdive);
        if (out.aimdive.released) {
            host.request_ordnance_release(ctx.unit);  // 009C60F1
            host.spend_round(ctx.approach);
            out.releases += 1;
        }
    } else if (ctx.current == DiveBombState::kAimGlide) {
        out.ran_aimglide = true;
        DiveBombAimGlideReleaseInputs glide = host.read_aimglide_inputs(ctx.state, dt);
        glide.rounds_available = host.rounds_remaining_007c1db0(ctx.unit);
        out.aimglide = dive_bomb_aimglide_release_009c5777(glide);
        host.apply_aimglide_result(ctx.state, out.aimglide);
        for (int i = 0; i < out.aimglide.rounds_released; ++i) {
            host.request_ordnance_release(ctx.unit);  // 009C5777, in the loop
            host.spend_round(ctx.approach);
            out.releases += 1;
        }
    }

    // Step 6, 009C885B: commit the plan step.
    host.commit_plan_step_result(ctx.task);

    // Step 7, 009C884E-009C88C9: the manual-release passthrough.
    const int pending = host.rounds_pending(ctx.task);
    if (pending > 0 && ctx.unit != nullptr) {
        const bool requested = host.manual_release_requested(ctx.unit);
        if (dive_bomb_manual_passthrough_009c88c4(pending, true, requested,
                                                  ctx.current)) {
            host.request_ordnance_release(ctx.unit);  // 009C88C4
            host.spend_pending_round(ctx.task);
            out.manual_release = true;
            out.releases += 1;
        }
    }
    return out;
}

}  // namespace bsp
