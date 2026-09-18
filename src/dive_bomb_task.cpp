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

    // The approach fallback both the !attacking and the attacking arm share,
    // 009C8419-009C845E and 009C8489-009C84E8.
    const DiveBombState approach_state =
        in.unit_lacks_follow_target ? DiveBombState::kMoveTo : DiveBombState::kFollow;

    if (!attacking) {
        out.next = engaged ? dive_bomb_entry_state_009c8310(in.entry) : approach_state;
    } else if (!engaged) {
        out.next = approach_state;  // 009C8483, the abort back to the approach
    } else if (in.engaged.control_mode_370 == 0) {
        out.next = DiveBombState::kPrepare;  // 009C84F5
    } else if (in.current != DiveBombState::kDone) {
        if (in.should_break_off) {
            out.next = DiveBombState::kDone;  // 009C8514
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
                    if (in.aimglide_out_of_bombs) {
                        out.next = DiveBombState::kGoAway;  // 009C8694
                    } else if (in.aimglide_pull_out_76c) {
                        out.next = DiveBombState::kGoAway;  // 009C86B4
                    }
                    break;
                case DiveBombState::kGoAway:
                    if (in.goaway_complete) {
                        // 009C86D9: with bombs left, climb back over the target.
                        out.next = in.entry.has_bomb_ordnance_4c9
                                       ? DiveBombState::kFlyAbove
                                       : DiveBombState::kDone;
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
    if (!in.latched) {
        threshold = in.in_range_distance;
    } else if (!in.control_flag_369 || !in.global_e17bf2) {
        threshold = in.in_range_distance +
                    static_cast<float>(dive_bomb_constant::kMoveToRangeBias);
    } else {
        return true;  // 009C8C2F falls straight to the `1`
    }
    return !(threshold <= in.planar_distance);
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
    const float x1 = in.begin_altitude_ac + in.extra_range_50;
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
    if (!(in.release_range_d4 + in.extra_range_50 > in.slant_range)) {
        return false;  // 009C5B18
    }
    if (!(in.unit_attitude_c64 > dive_bomb_constant::kAbortRollFloor)) {
        return false;  // 009C5B2C
    }
    const float bound =
        in.slant_range * static_cast<float>(dive_bomb_constant::kAbortRangeSlope) +
        static_cast<float>(dive_bomb_constant::kAbortRangeBias);
    return bound > in.aim_point_distance;  // 009C5B3E
}

// 009C5693-009C57A6. coverage: partial; the four frame slots are host inputs.
DiveBombAimGlideReleaseResult dive_bomb_aimglide_release_009c5777(
    const DiveBombAimGlideReleaseInputs& in) noexcept {
    DiveBombAimGlideReleaseResult out;
    out.travel_accumulator_20 = in.travel_accumulator_20;

    // 009C569B: the flight path has to be shallower than 30 degrees.
    if (!(dive_bomb_constant::kGlideDiveAngleLimit > in.dive_angle)) {
        return out;
    }
    // 009C56AE: height_above + 50.0 must still clear the limit.
    if (!(static_cast<double>(in.height_above) +
              dive_bomb_constant::kGlideHeightMargin >
          static_cast<double>(in.height_limit))) {
        return out;
    }
    // 009C56C2-009C56FE: the lateral offset inside 120.0.
    const float lateral = fold_abs(in.lateral_a - in.lateral_b);
    if (!(dive_bomb_constant::kGlideLateralLimit > static_cast<double>(lateral))) {
        return out;
    }
    // 009C5704-009C5755: the lead the bomb still has to cover, against the
    // 5.0 margin and then against the 3.0 scale on its negation.
    const float lead = in.lateral_b - std::cos(in.dive_angle) * in.lateral_a;
    const double near_edge = static_cast<double>(in.travel_accumulator_20) +
                             dive_bomb_constant::kGlideLeadMargin;
    if (!(static_cast<double>(lead) + static_cast<double>(in.travel_accumulator_20) >
          near_edge)) {
        return out;
    }
    if (!(-(static_cast<double>(lead) +
            static_cast<double>(in.travel_accumulator_20)) *
              dive_bomb_constant::kGlideLeadScale >
          near_edge)) {
        return out;
    }

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

    // 009C4512-009C4524, before any branch: the desired-speed command and the
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

// 009C7EA0-009C7EF2, __fastcall(state) -> bool.
bool dive_bomb_turndown_complete_009c7ea0(float attitude_c64,
                                          float attitude_c68) noexcept {
    float folded = attitude_c68;
    if (folded <= dive_bomb_constant::kZero) {
        folded = dive_bomb_constant::kNegativeZero - folded;
    }
    if (attitude_c64 >= dive_bomb_constant::kTurnDownRollGate &&
        (attitude_c64 >= dive_bomb_constant::kMinusOne ||
         folded <= dive_bomb_constant::kTurnDownPitchGate)) {
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
