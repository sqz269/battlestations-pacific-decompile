// The torpedo bot task arm and its release rules. Evidence in
// docs/TORPEDO_TASK_ARM.md; every address in a comment is a listing address.

#include "bsp/torpedo_task_arm.hpp"

#include <cmath>

namespace bsp {
namespace {

// BSP_Math_InterpolateClamped 00419010, as the two release cones use it:
// the segment (x0,y0)-(x1,y1) evaluated at x and clamped to the endpoints.
float interpolate_clamped(float x0, float y0, float x1, float y1, float x) noexcept {
    if (x1 == x0) return y0;
    const float t = (x - x0) / (x1 - x0);
    if (t <= 0.0f) return y0;
    if (t >= 1.0f) return y1;
    return y0 + (y1 - y0) * t;
}

}  // namespace

// 009D31D0. `param_2` is the state pointer; five compares against task+710h,
// +618h, +6D8h, +6B4h and +740h, returning 1 for any of them.
bool torpedo_in_attack_state_009d31d0(TorpedoState state) noexcept {
    switch (state) {
        case TorpedoState::kAim:
        case TorpedoState::kDone:
        case TorpedoState::kGoAway:
        case TorpedoState::kAttackRun:
        case TorpedoState::kPrepare:
            return true;
        default:
            return false;
    }
}

// 009D3210. The early `task->+529h != 0` short-circuits every other test, the
// same shape as the depth charge's `task->+46Dh != 0 || ...`.
bool torpedo_engaged_009d3210(const TorpedoEngagedInputs& in) noexcept {
    if (in.aim_flag_529) return true;
    if (!in.has_engage_target_4c4) return false;          // 009D3222
    if (in.pilot_control_mode_370 == 2) return true;      // 009D3236
    if (!in.unit_has_no_follow_target) return false;      // 009D3245
    // 009D325B: task->+484h * [00D05AC8] must exceed task->+488h.
    if (in.engage_range_484 * in.engage_range_scale <= in.engage_range_limit_488) {
        return false;
    }
    return true;
}

// 009D3F60, the entry chooser. The depth charge's 009A57D0 picks `turnto` in
// the last arm; the torpedo has no `turnto`, so it picks `aim`.
TorpedoState torpedo_entry_state_009d3f60(const TorpedoEntryInputs& in) noexcept {
    if (in.pilot_control_mode_370 == 0) return TorpedoState::kPrepare;   // 009D3F6E
    // 009D3FA9: the extra clause the depth charge does not have.
    if (!in.attack_flag_52a && (!in.control_flag_369 || !in.global_e17bf2)) {
        return TorpedoState::kDone;                                      // 009D3FC7
    }
    if (!in.aim_flag_529) return TorpedoState::kAttackRun;               // 009D4004
    return TorpedoState::kAim;                                           // 009D3FDE
}

// 009D4030. Returns kNone when the rule leaves the state alone.
TorpedoState torpedo_next_state_009d4030(const TorpedoTransitionInputs& in) noexcept {
    const bool attacking = torpedo_in_attack_state_009d31d0(in.current);
    const bool engaged = torpedo_engaged_009d3210(in.engaged);

    // 009D41D1 and the tail at 009D40x: the two approach states.
    const TorpedoState approach_state =
        in.unit_has_no_follow_target ? TorpedoState::kMoveTo : TorpedoState::kFollow;

    if (!attacking) {
        // 009D41C4: not in an attack state.
        if (engaged) return torpedo_entry_state_009d3f60(in.entry);  // 009D41D1
        return approach_state;
    }

    if (!engaged) return approach_state;  // 009D4053, the abort back to approach

    // 009D4084: the pilot control block is not yet flying the run.
    if (in.entry.pilot_control_mode_370 == 0) return TorpedoState::kPrepare;

    if (in.current == TorpedoState::kDone) return TorpedoState::kNone;  // 009D4097

    // 009D40A6: the break-off test, guarded by the in-attack-state test again.
    if (torpedo_in_attack_state_009d31d0(in.current) && in.should_break_off) {
        return TorpedoState::kDone;
    }

    switch (in.current) {
        case TorpedoState::kPrepare:
            // 009D41D1: prepare re-runs the entry chooser every tick.
            return torpedo_entry_state_009d3f60(in.entry);
        case TorpedoState::kAttackRun:
            // 009D40D5: the depth charge goes to `turnto` here.
            return in.entry.aim_flag_529 ? TorpedoState::kAim : TorpedoState::kNone;
        case TorpedoState::kAim:
            // 009D40F1: stay while the attack flag holds and 009D31B0 refuses.
            if (in.entry.attack_flag_52a && !in.aim_hold_009d31b0) {
                return TorpedoState::kNone;
            }
            return TorpedoState::kGoAway;
        case TorpedoState::kGoAway:
            // 009D4132: the depth charge picks `turnto` instead of `aim`.
            if (!in.goaway_done_009d3150) return TorpedoState::kNone;
            return in.entry.attack_flag_52a ? TorpedoState::kAim : TorpedoState::kDone;
        default:
            return TorpedoState::kNone;
    }
}

// 009D49A0, task vtable slot +24h.
TorpedoArmResult torpedo_arm_drop_009d49a0(const TorpedoArmInputs& in) noexcept {
    TorpedoArmResult out;
    if (!in.attack_flag_52a) {
        out.consumed = true;  // 009D49DA returns 1
        return out;
    }
    if (in.current == TorpedoState::kPrepare) {
        // 009D49C2: prepare->+98h = [00CE3D34]. This is the only writer that
        // raises the countdown; everything else sets it to -1.0f.
        out.armed = true;
        out.prepare_timer = in.arm_countdown_00ce3d34;
        out.consumed = true;
        return out;
    }
    if (!torpedo_engaged_009d3210(in.engaged)) {
        // 009D49B6: falls through to the base slot +24h, 0099B6A0.
        out.fell_through_to_0099b6a0 = true;
        out.consumed = false;  // the base's return decides; unread, taken as 0
        return out;
    }
    out.consumed = true;
    return out;
}

// 009D25A0.
bool torpedo_release_gate_009d25a0(const TorpedoReleaseGateInputs& in) noexcept {
    if (in.force) return true;  // 009D25B3
    // 009D26C1: the time metric must be inside the cone's far end.
    if (!(in.time_to_target < torpedo_constant::kHelperConeFarTime)) return false;
    const float allowed = interpolate_clamped(
        torpedo_constant::kHelperConeNearTime, torpedo_constant::kHelperConeNearAngle,
        torpedo_constant::kHelperConeFarTime, torpedo_constant::kHelperConeFarAngle,
        in.time_to_target);
    if (!(in.bearing_error < allowed)) return false;         // 009D26DC
    // 009D26EC: unit->+C68h must be under 35 degrees, and not equal to it.
    if (in.unit_bank_c68 > torpedo_constant::kHelperMaxBank) return false;
    if (in.unit_bank_c68 == torpedo_constant::kHelperMaxBank) return false;
    return true;
}

// 009D2570, the exit slot.
bool torpedo_exit_drop_009d2570(float drop_timer_98) noexcept {
    return drop_timer_98 >= torpedo_constant::kExitDropThreshold;
}

// 009D2720.
TorpedoDoneTickResult torpedo_done_prepare_tick_009d2720(
    const TorpedoDoneTickInputs& in) noexcept {
    TorpedoDoneTickResult out;
    out.drop_timer_98 = in.drop_timer_98;

    if (in.drop_timer_98 > 0.0f) {
        // 009D2759-009D2794: the committed branch. Weapon select 3, throttle
        // 0.3, and the countdown steps down by dt.
        out.wrote_committed_command = true;
        float timer = in.drop_timer_98 - in.dt;
        if (!(timer > 0.0f)) {
            // 009D27A2: 009D25A0(state, false). The countdown expired, so the
            // gated helper decides.
            const TorpedoReleaseGateInputs gate{false, in.time_to_target,
                                                in.bearing_error, in.unit_bank_c68};
            if (torpedo_release_gate_009d25a0(gate)) {
                out.released = true;
                out.consumed_round = true;
                out.outcome = TorpedoDoneTickOutcome::kReleaseTimeout;
            }
            // 009D27B3: armed or not, the countdown is cleared here.
            out.drop_timer_98 = torpedo_constant::kDisarmed;
            return out;
        }
        out.drop_timer_98 = timer;

        // 009D27C9: with the countdown still running, steer or release.
        if (!in.has_target) {
            out.outcome = TorpedoDoneTickOutcome::kIdle;
            return out;
        }
        if (in.time_to_target > torpedo_constant::kAbortTimeToTarget
            || in.bearing_error > torpedo_constant::kAbortBearing) {
            // 009D29CB: the solution decayed past 2.5 s or 80 degrees. The
            // native drops anyway and does not clear the countdown.
            out.released = true;
            out.consumed_round = true;
            out.outcome = TorpedoDoneTickOutcome::kReleaseAbort;
            return out;
        }
        if (torpedo_constant::kCloseTimeToTarget > in.time_to_target) {
            // 009D28DD: inside 1.5 s the bearing cone decides.
            const float allowed = interpolate_clamped(
                torpedo_constant::kConeNearTime, torpedo_constant::kConeNearAngle,
                torpedo_constant::kConeFarTime, torpedo_constant::kConeFarAngle,
                in.time_to_target);
            if (allowed > in.bearing_error
                && torpedo_constant::kMaxBankAtRelease > in.unit_bank_c68) {
                // 009D2938.
                out.released = true;
                out.consumed_round = true;
                out.drop_timer_98 = torpedo_constant::kDisarmed;  // 009D294E
                out.outcome = TorpedoDoneTickOutcome::kReleaseInCone;
                return out;
            }
        }
        // 009D295F: hold the run in. cmd->+2BCh = 0, cmd->+2D0h = 2.
        if (torpedo_constant::kDisarmed + 2.0f > out.drop_timer_98) {
            // 009D297F compares 1.0f against the countdown.
            out.outcome = TorpedoDoneTickOutcome::kHoldHeading;  // 009D2990
        } else {
            out.outcome = TorpedoDoneTickOutcome::kSteerToTarget;  // 009D29B7
        }
        return out;
    }

    // 009D29E0: the countdown is disarmed. This branch never releases and
    // never writes +98h; it produces the blend value at state->+8Ch.
    out.outcome = TorpedoDoneTickOutcome::kIdle;
    return out;
}

// 009D4886/009D4890: the speed select. The comparison is a double against
// [00CF3F20] = 15.0, and the jbe arm takes approach->+7Ch.
float torpedo_arm_speed_009d4886(float speed_switch_134, float speed_low_7c,
                                 float speed_high_80) noexcept {
    return (speed_switch_134 >= torpedo_constant::kSpeedSwitchThreshold) ? speed_low_7c
                                                                        : speed_high_80;
}

// 009D48AC-009D48CF: both range arguments of 009BDE80 are the same sum.
float torpedo_arm_move_to_range_009d48ac(float range_base_78,
                                         float range_spread_74) noexcept {
    return range_base_78 + range_spread_74;
}

// 009D48F8-009D495B.
bool torpedo_manual_passthrough_009d4956(int rounds_pending_424, bool has_unit,
                                         bool device_requests_release,
                                         TorpedoState current) noexcept {
    if (rounds_pending_424 <= 0) return false;      // 009D490B
    if (!has_unit) return false;                    // 009D4915
    if (!device_requests_release) return false;     // 009D492A
    // 009D492C-009D494E: aim, attackrun and prepare own their own release.
    if (current == TorpedoState::kAim) return false;
    if (current == TorpedoState::kAttackRun) return false;
    if (current == TorpedoState::kPrepare) return false;
    return true;
}

// 009D4850, in the native order.
TorpedoArmTickResult torpedo_task_arm_009d4850(TorpedoTaskHost& host,
                                               TorpedoTaskContext& ctx,
                                               float dt) {
    TorpedoArmTickResult out;
    out.state_before = ctx.current;
    out.state_after = ctx.current;

    // 009D4865: task->+49Ch = 0FFh, the per-tick plan scratch.
    host.set_plan_step_scratch(ctx.task, 0xFF);

    // 009D486F: the approach update.
    host.update_torpedo_approach_009d3420(ctx.approach, dt);

    // 009D4874-009D48CF: refresh the moveto ranges with the class speed.
    const TorpedoTransitionInputs inputs = host.read_transition_inputs(ctx.task);
    out.move_to_speed = host.approach_speed_for_arm(ctx.approach);
    out.move_to_range = host.approach_move_to_range_for_arm(ctx.approach);
    host.refresh_move_to_ranges(host.move_to_state(ctx.task), out.move_to_range,
                                out.move_to_range, out.move_to_speed);

    // 009D48DE: the transition rule, before the state's own tick.
    const TorpedoState next = torpedo_next_state_009d4030(inputs);
    if (next != TorpedoState::kNone && next != ctx.current) {
        host.set_state(ctx.task, next);
        ctx.current = next;
        out.state_after = next;
        out.transitioned = true;
    }

    // 009D48E7-009D48F6: state->vtable[0Ch](dt) on task->+310h.
    if (ctx.current == TorpedoState::kDone || ctx.current == TorpedoState::kPrepare) {
        void* const state = (ctx.current == TorpedoState::kPrepare) ? ctx.prepare_state
                                                                   : ctx.state;
        host.follow_base_tick_009c1fd0(state, dt);  // 009D2731
        TorpedoDoneTickInputs tick;
        tick.dt = dt;
        tick.drop_timer_98 = host.read_drop_timer(state);
        tick.has_target = host.pilot_control_has_target(ctx.pilot_control_block);
        tick.time_to_target = host.time_to_target_009d1500(ctx.approach);
        tick.bearing_error = host.bearing_error_to_target(ctx.approach, ctx.unit);
        tick.unit_bank_c68 = host.unit_bank_c68(ctx.unit);
        const TorpedoDoneTickResult r = torpedo_done_prepare_tick_009d2720(tick);
        out.done_tick = r;
        out.ran_done_tick = true;
        if (r.wrote_committed_command) {
            // 009D2761: approach->+A4h = 3, the weapon selector.
            host.set_weapon_selector(ctx.approach, 3);
            host.clear_approach_1c_field40(ctx.approach);
            host.write_command_float(ctx.command_block, pilot_command_off::kInterp2C8,
                                     torpedo_constant::kCommittedThrottle);
        }
        if (r.outcome == TorpedoDoneTickOutcome::kHoldHeading) {
            host.write_command_float(ctx.command_block, pilot_command_off::kWord2BC, 0.0f);
            host.write_command_word(ctx.command_block, pilot_command_off::kMode2D0, 2);
            host.write_command_float(ctx.command_block, pilot_command_off::kWord2C4, 0.0f);
            host.write_command_word(ctx.command_block, pilot_command_off::kHeadingMode, 1);
        } else if (r.outcome == TorpedoDoneTickOutcome::kSteerToTarget) {
            host.write_command_float(ctx.command_block, pilot_command_off::kWord2BC, 0.0f);
            host.write_command_word(ctx.command_block, pilot_command_off::kMode2D0, 2);
            host.steer_toward_target_009f9e40(ctx.approach);
        } else if (r.outcome == TorpedoDoneTickOutcome::kIdle && tick.drop_timer_98 <= 0.0f) {
            host.approach_committed_hook_009d1360(ctx.approach);
        }
        host.write_drop_timer(state, r.drop_timer_98);
        if (r.released) {
            host.request_ordnance_release(ctx.unit);
            ++out.releases;
        }
        if (r.consumed_round) host.consume_round(ctx.approach);
    } else {
        host.tick_state(ctx.state, dt);
    }

    // 009D48FF-009D4905: task->+2E4h = task->+49Ch.
    host.commit_plan_step_result(ctx.task);

    // 009D4956: the manual-release passthrough.
    const int pending = host.rounds_pending(ctx.task);
    if (torpedo_manual_passthrough_009d4956(pending, ctx.unit != nullptr,
                                            host.manual_release_requested(ctx.unit),
                                            ctx.current)) {
        host.request_ordnance_release(ctx.unit);
        host.spend_pending_round(ctx.task);
        out.manual_release = true;
        ++out.releases;
    }
    return out;
}

// ---------------------------------------------------------------------------
// 009D0D90-009D0E3A, the goaway enter. Evidence: docs/TORPEDO_GOAWAY_RELEASE.md.
// ---------------------------------------------------------------------------
TorpedoGoAwayState torpedo_goaway_enter_009d0d90(
    const TorpedoGoAwayEnterInputs& in) noexcept {
    TorpedoGoAwayState out;
    // 009D0D90-009D0DBC: the low bit of [00F876B0] alternates the break-off
    // side. The AND 80000001h plus the JNS fixup is the signed remainder idiom,
    // so the test is on the magnitude's low bit.
    out.break_off_side_2c = in.side_bit ? torpedo_goaway::kSideLeft
                                        : torpedo_goaway::kSideRight;
    // 009D0DD1: the base distance is the tuning singleton's +438h.
    float distance = in.safe_distance_438;
    // 009D0DDF-009D0E0F: a target that answers vtable[5Ch](5) raises it to its
    // own extent when that is larger. 009D0E07 JA keeps the base when the base
    // is strictly greater, so an equal extent replaces it, harmlessly.
    if (in.has_extent_target && !(distance > in.target_extent)) {
        distance = in.target_extent;
    }
    // 009D0E2C: BSP_Random_UniformFloatRange(1.0, 1.15), then 009D0E31 FMUL.
    out.break_off_distance_24 = in.distance_jitter * distance;
    return out;
}

// ---------------------------------------------------------------------------
// 009D3150-009D31A5, the goaway-done predicate.
// ---------------------------------------------------------------------------
bool torpedo_goaway_complete_009d3150(
    const TorpedoGoAwayCompleteInputs& in) noexcept {
    float distance = in.break_off_distance_24;   // 009D3151
    // 009D315C CMP byte [EAX+369h]; 009D316A CMP byte [00E17BF2]. Either clear
    // and the scaling block is skipped entirely.
    if (in.control_flag_369 && in.global_e17bf2) {
        // 009D3173: no ordnance left and the predicate returns false outright.
        if (!in.has_ordnance_132) return false;   // 009D317C XOR AL,AL
        distance = distance * torpedo_goaway::kOrderedScale;  // 009D3183
    }
    // 009D318F FLD approach+90h; 009D3195 FCOMIP ST0,ST1 compares the range
    // against the distance and 009D3199 JBE clears, so the test is strict.
    return in.range_90 > distance;
}

}  // namespace bsp
