#include "bsp/bot_task_states.hpp"

#include <cmath>

// Reconstruction of the bot task state objects. docs/BOT_TASK_STATES.md carries the
// evidence and the coverage table; every name is a hypothesis, not a recovered symbol.

namespace bsp {
namespace {

// The nine depth-charge state objects, task-relative, from the registration order of
// 009A3090 (approach-relative, plus the 3F8h approach base) and confirmed against the
// pointer comparisons of 009A5870.
constexpr int kDepthChargeStateOffsets[static_cast<int>(DepthChargeState::kCount)] = {
    0x508,  // moveto,    009A30A8 registers approach+110h
    0x544,  // follow,    009A30BB registers approach+14Ch
    0x5DC,  // done,      009A30CE registers approach+1E4h
    0x67C,  // attackrun, 009A30E1 registers approach+284h
    0x6A0,  // goaway,    009A30F4 registers approach+2A8h
    0x6C4,  // aim,       009A3107 registers approach+2CCh
    0x6E4,  // prepare,   009A311A registers approach+2ECh
    0x784,  // leave,     009A312D registers approach+38Ch
    0x7A0,  // turnto,    009A3140 registers approach+3A8h after ADD ESI,0x3A8
};

// The literal strings at 00D1F608, 00D1F5F0, 00D1F5DC, 00D1F5C4, 00D1F5B0, 00D1F5A0,
// 00D1F58C, 00D1F578 and 00D1F564. Debug text only; nothing looks a state up by name.
const char* const kDepthChargeStateNames[static_cast<int>(DepthChargeState::kCount)] = {
    "moveto (DepthCharge)", "follow (DepthCharge)", "DepthCharge/done",
    "DepthCharge/attackrun", "DepthCharge/goaway",  "DepthCharge/aim",
    "DepthCharge/prepare",  "DepthCharge/leave",    "DepthCharge/turnto",
};

int state_index(DepthChargeState state) {
    const int index = static_cast<int>(state);
    if (index < 0 || index >= static_cast<int>(DepthChargeState::kCount)) {
        return -1;
    }
    return index;
}

// 009A5870 step 2: the task is engaged when its +46Dh byte is set, or when the pilot
// control mode is 2 and a target is latched at task+48Ch.
bool is_engaged(const DepthChargeTransitionInputs& in) {
    return in.flag_46D || (in.control_mode == 2 && in.has_latched_target);
}

// 009A5870 steps 4 and 5: the approach state follows unit+9D8h on every tick.
DepthChargeState approach_state(const DepthChargeTransitionInputs& in) {
    return in.unit_has_no_follow_target ? DepthChargeState::kMoveTo : DepthChargeState::kFollow;
}

}  // namespace

int depth_charge_state_offset(DepthChargeState state) {
    const int index = state_index(state);
    return index < 0 ? 0 : kDepthChargeStateOffsets[index];
}

const char* depth_charge_state_name(DepthChargeState state) {
    const int index = state_index(state);
    return index < 0 ? "" : kDepthChargeStateNames[index];
}

// 009A5420, from docs/BOT_TASKS.md: exactly the seven pointers that are not moveto or
// follow. docs/BOT_TASK_STATES.md repeats the list from the transition rule.
bool depth_charge_is_attack_state(DepthChargeState state) {
    switch (state) {
        case DepthChargeState::kDone:
        case DepthChargeState::kAttackRun:
        case DepthChargeState::kGoAway:
        case DepthChargeState::kAim:
        case DepthChargeState::kPrepare:
        case DepthChargeState::kLeave:
        case DepthChargeState::kTurnTo:
            return true;
        default:
            return false;
    }
}

// 009A57D0, complete: 009A57D0-009A586C.
DepthChargeState depth_charge_entry_state(const DepthChargeTransitionInputs& in) {
    if (in.control_mode == 0) {
        return DepthChargeState::kPrepare;  // 009A57D0 head
    }
    if (!in.flag_46C) {
        return DepthChargeState::kDone;  // the 009A5810 branch
    }
    if (!in.flag_46D) {
        return DepthChargeState::kAttackRun;  // 009A5878-relative branch, task+67Ch
    }
    return DepthChargeState::kTurnTo;  // the 009A5864 tail, task+7A0h
}

// 009A5870, complete: 009A5870-009A5AEE.
DepthChargeState depth_charge_next_state(const DepthChargeTransitionInputs& in) {
    const bool attacking = depth_charge_is_attack_state(in.current);
    const bool engaged = is_engaged(in);

    if (!attacking) {
        return engaged ? depth_charge_entry_state(in) : approach_state(in);
    }
    if (!engaged) {
        return approach_state(in);  // the abort back to the approach states
    }
    if (in.control_mode == 0) {
        return DepthChargeState::kPrepare;
    }
    if (in.current == DepthChargeState::kDone) {
        return DepthChargeState::kUnchanged;  // done is terminal for this rule
    }
    if (in.should_break_off) {
        return DepthChargeState::kDone;  // task->vtable[+1Ch], 009A65F0
    }

    switch (in.current) {
        case DepthChargeState::kPrepare:
            return depth_charge_entry_state(in);
        case DepthChargeState::kAttackRun:
            return in.flag_46D ? DepthChargeState::kTurnTo : DepthChargeState::kUnchanged;
        case DepthChargeState::kAim:
            if (!in.approach_has_ordnance) {
                return DepthChargeState::kLeave;
            }
            return in.aim_pull_out ? DepthChargeState::kGoAway : DepthChargeState::kUnchanged;
        case DepthChargeState::kLeave:
            // The native test is FCOM-style: countdown <= 0 and countdown != 0.
            return (in.leave_countdown < 0.0f) ? DepthChargeState::kGoAway
                                               : DepthChargeState::kUnchanged;
        case DepthChargeState::kTurnTo:
            if (in.turn_to_aim_ready) {
                return DepthChargeState::kAim;
            }
            return in.turn_to_give_up ? DepthChargeState::kGoAway : DepthChargeState::kUnchanged;
        case DepthChargeState::kGoAway:
            if (!in.go_away_finished) {
                return DepthChargeState::kUnchanged;
            }
            return in.flag_46C ? DepthChargeState::kTurnTo : DepthChargeState::kDone;
        default:
            return DepthChargeState::kUnchanged;
    }
}

// 009A3788 and 009D07D8: the countdown is compared against dt, not against zero.
bool attack_run_should_reroll(float countdown, float dt) { return !(dt < countdown); }

float attack_run_next_countdown(float period, float countdown, float dt) {
    if (!attack_run_should_reroll(countdown, dt)) {
        return countdown - dt;
    }
    return (period - dt) + countdown;
}

// 009A3FA0, complete: three instructions.
bool depth_charge_in_release_window(float run_in_param, float release_base, float release_bias) {
    return run_in_param < release_bias + release_base;
}

// 009A4591-009A45A3: the arm request only goes up inside a band below the aim altitude.
bool depth_charge_in_drop_band(float altitude, float release_base, float release_bias,
                               double band_00ce4d70) {
    const double margin = static_cast<double>(altitude) -
                          static_cast<double>(release_bias + release_base);
    return margin < band_00ce4d70;
}

// 009A45B8-009A4619, complete. Every term is a separate conditional jump in the listing.
bool depth_charge_should_release(const DepthChargeReleaseInputs& in) {
    if (!in.has_latched_target) {
        return false;  // 009A45BE TEST EAX,EAX
    }
    if (!in.target_engageable) {
        return false;  // 009A45C6 CMP byte ptr [EAX+5Dh],0
    }
    if (!in.has_ordnance) {
        return false;  // 009A45D0 CMP byte ptr [EDI+74h],0
    }
    if (!(in.release_cooldown < 0.0f)) {
        return false;  // 009A45DD COMISS XMM0(0.0f), [EBX+1Ch]
    }
    if (!in.in_release_window) {
        return false;  // 009A45E5 CALL 009A3FA0
    }
    const double ceiling = static_cast<double>(in.aim_altitude) + in.altitude_margin;
    return static_cast<double>(in.unit_altitude) < ceiling;  // 009A4615 FCOMIP
}

// 009A48D0 and 009D2570, complete.
bool prepare_done_exit_drops(float drop_gate, double threshold_00d7a218) {
    return static_cast<double>(drop_gate) >= threshold_00d7a218;
}

// 009C18C0 step 1: the planar separation is compared against a double constant before
// the square root is taken at all.
bool move_to_arrived(float dx, float dz, double threshold_00ce3820) {
    const double planar = static_cast<double>(dx) * dx + static_cast<double>(dz) * dz;
    return planar <= threshold_00ce3820;
}

float move_to_desired_speed(float dx, float dz, double threshold_00ce3820) {
    if (move_to_arrived(dx, dz, threshold_00ce3820)) {
        return 0.0f;
    }
    return std::sqrt(dx * dx + dz * dz);
}

// 009C18C0 step 5: the far range is an offset above the target, floored by the near range.
float move_to_target_altitude(float far_range, float target_altitude, float near_range) {
    const float candidate = far_range + target_altitude;
    return candidate < near_range ? near_range : candidate;
}

// 009A6777-009A67A3: four pointer comparisons, all of them exclusions.
bool manual_release_allowed(DepthChargeState current) {
    switch (current) {
        case DepthChargeState::kAim:
        case DepthChargeState::kAttackRun:
        case DepthChargeState::kTurnTo:
        case DepthChargeState::kPrepare:
            return false;
        default:
            return true;
    }
}

// 009A56C0, complete.
void set_state(BotTaskStateHost& host, BotTaskStateContext& ctx, void* next) {
    void* current = ctx.state;
    if (next == current) {
        return;  // 009A56D0 JE
    }
    if (current != nullptr) {
        host.exit_state(current);  // 009A56DB, vtable +8h
    }
    ctx.state = next;              // 009A56DD
    host.enter_state(next);        // 009A56EA, vtable +4h
}

// 009A66C0, complete: 009A66C0-009A67BA. The body has no Ghidra function and was read
// from the PE listing.
void depth_charge_tick(BotTaskStateHost& host, BotTaskStateContext& ctx,
                       const DepthChargeTransitionInputs& in, float dt, int rounds_remaining,
                       void* const state_objects[static_cast<int>(DepthChargeState::kCount)]) {
    // Step 1 sets task->+470h = 0FFh; step 6 copies it into task->+2E4h. Both are task
    // fields this projection does not own, so only the ordering is kept.
    host.update_approach(ctx.approach, dt);  // 009A66DF

    void* const move_to = state_objects[static_cast<int>(DepthChargeState::kMoveTo)];
    host.refresh_move_to_ranges(move_to, 0.0f, 0.0f, 0.0f);  // 009A671A, constants unread

    const DepthChargeState next = depth_charge_next_state(in);  // 009A6729
    if (next != DepthChargeState::kUnchanged) {
        set_state(host, ctx, state_objects[static_cast<int>(next)]);
    }

    if (ctx.state != nullptr) {
        host.tick_state(ctx.state, dt);  // 009A6741, vtable +Ch
    }

    // Step 7, the manual-release passthrough.
    if (rounds_remaining <= 0 || ctx.unit == nullptr) {
        return;  // 009A6756 JLE, 009A6760 JE
    }
    if (!host.manual_release_requested(ctx.unit)) {
        return;  // 009A6773 TEST AL,AL
    }
    const DepthChargeState current = (next == DepthChargeState::kUnchanged) ? in.current : next;
    if (!manual_release_allowed(current)) {
        return;
    }
    host.request_ordnance_release(ctx.unit);  // 009A67AB
    host.consume_round(ctx.approach);         // 009A67B0 decrements task->+424h, not approach+2Ch
}

// 009A3770 and 009D07B0, complete for both.
AttackRunTickResult attack_run_tick(BotTaskStateHost& host, BotTaskStateContext& ctx,
                                    const AttackRunTickInputs& in, float dt) {
    AttackRunTickResult out;
    out.countdown = attack_run_next_countdown(in.period, in.countdown, dt);
    out.heading_offset = in.heading_offset;
    if (attack_run_should_reroll(in.countdown, dt)) {
        out.heading_offset = host.sample_heading_offset(ctx.unit, in.variant);
    }

    const float heading = host.add_wrapped_angle(in.run_in_heading, out.heading_offset);
    host.write_command_float(ctx.command_block, pilot_command_off::kDesiredHeading, heading);
    host.write_command_word(ctx.command_block, pilot_command_off::kHeadingMode, 2);

    host.command_altitude_and_throttle(ctx.approach, in.altitude, 0.0f, 0.0f, in.throttle);

    host.write_command_float(ctx.command_block, pilot_command_off::kFloat278, in.command_278);
    host.write_command_byte(ctx.command_block, pilot_command_off::kFlag27C, 1);
    host.write_command_word(ctx.command_block, pilot_command_off::kWord2A8, 0);
    host.write_command_byte(ctx.command_block, pilot_command_off::kFlag2AC, 1);
    host.write_command_word(ctx.command_block, pilot_command_off::kSpeedValid, 0);
    return out;
}

// 009A4588-009A4667, the release tail of the aim tick. coverage: partial.
DepthChargeAimTickResult depth_charge_aim_tick_release(BotTaskStateHost& host,
                                                       BotTaskStateContext& ctx,
                                                       const DepthChargeReleaseInputs& release,
                                                       float pull_out_metric,
                                                       double pull_out_threshold_00ce380c,
                                                       double drop_band_00ce4d70,
                                                       float release_cooldown, float dt) {
    DepthChargeAimTickResult out;
    out.release_cooldown = release_cooldown - dt;  // the head of 009A4010

    // 009A4588: the flag the transition rule reads to leave aim for goaway.
    out.pull_out = pull_out_threshold_00ce380c < static_cast<double>(pull_out_metric);

    const bool in_band = depth_charge_in_drop_band(release.unit_altitude, 0.0f, 0.0f,
                                                   drop_band_00ce4d70);
    if (!in_band) {
        host.set_arm_request(ctx.approach, false);  // 009A465D
        return out;
    }
    host.set_arm_request(ctx.approach, true);  // 009A45A9
    host.set_weapon_selector(ctx.approach, 3); // 009A45AF
    out.armed = true;

    DepthChargeReleaseInputs gated = release;
    gated.release_cooldown = out.release_cooldown;
    if (!depth_charge_should_release(gated)) {
        return out;
    }
    host.request_ordnance_release(ctx.unit);  // 009A4620
    host.consume_round(ctx.approach);         // 009A462D
    // 009A4646: the next release delay is a draw between 00CE3860 and 00CE6448 scaled
    // by 00CE65D0. The three constants were not resolved to tuning rows.
    out.release_cooldown = host.random_between(0.0f, 0.0f);
    out.released = true;
    return out;
}

// 009A48D0 and 009D2570, complete.
bool prepare_done_exit(BotTaskStateHost& host, BotTaskStateContext& ctx, float drop_gate,
                       double threshold_00d7a218) {
    if (!prepare_done_exit_drops(drop_gate, threshold_00d7a218)) {
        return false;
    }
    host.request_ordnance_release(ctx.unit);  // 009A48EA, 009D258A
    host.consume_round(ctx.approach);         // 009A48F4, 009D2592
    return true;
}

// 009C18C0, the branches this packet transcribed. coverage: partial.
void move_to_tick(BotTaskStateHost& host, BotTaskStateContext& ctx, bool has_target, float dx,
                  float dz, double arrive_threshold_00ce3820) {
    const float speed =
        has_target ? move_to_desired_speed(dx, dz, arrive_threshold_00ce3820) : 0.0f;
    host.set_desired_speed(ctx.state, speed);  // the vtable +1Ch call

    if (has_target) {
        return;  // the altitude and throttle branch is not reproduced
    }
    // The no-target branch, complete.
    host.write_command_word(ctx.command_block, pilot_command_off::kWord2C4, 0);
    host.write_command_word(ctx.command_block, pilot_command_off::kHeadingMode, 1);
    host.write_command_word(ctx.command_block, pilot_command_off::kWord2BC, 0);
    host.write_command_word(ctx.command_block, pilot_command_off::kMode2D0, 2);
}

}  // namespace bsp
