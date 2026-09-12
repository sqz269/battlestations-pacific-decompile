#include "bsp/director_update_arms.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Permissions
// ---------------------------------------------------------------------------

DirectorPermissions controller_constructed_permissions() noexcept {
    // 0072021F / 00720222 store BL, and EBX is zeroed at 007201A2; 007202FD,
    // 00720304, 0072030B and 00720312 store the immediate 1.
    DirectorPermissions permissions;
    permissions.allow_fire = false;
    permissions.allow_move = false;
    permissions.artillery_enabled = true;
    permissions.aa_enabled = true;
    permissions.torpedo_enabled = true;
    permissions.depth_charge_enabled = true;
    return permissions;
}

bool weapon_enable(const DirectorPermissions& permissions,
                   DirectorWeaponEnable which) noexcept {
    switch (which) {
    case DirectorWeaponEnable::kArtillery:
        return permissions.artillery_enabled;
    case DirectorWeaponEnable::kAntiAircraft:
        return permissions.aa_enabled;
    case DirectorWeaponEnable::kTorpedo:
        return permissions.torpedo_enabled;
    case DirectorWeaponEnable::kDepthCharge:
        return permissions.depth_charge_enabled;
    }
    return false;
}

// ---------------------------------------------------------------------------
// The permission consumers
// ---------------------------------------------------------------------------

bool director_allows_fire_and_move(bool allow_fire, bool allow_move) noexcept {
    // 0080DC70: CMP byte [ECX+3Ch],0 / JZ fail / CMP byte [ECX+3Dh],0 / JZ fail
    // / MOV AL,1. Either byte zero falls into XOR AL,AL at 0080DC7F.
    return allow_fire && allow_move;
}

bool auto_target_enabled(bool allow_move, bool slot0_occupied,
                         int slot0_category) noexcept {
    // 009F5613: allowMove clear returns false at 009F563A.
    if (!allow_move) {
        return false;
    }
    // 009F5619: an empty slot 0 returns true at 009F5634.
    if (!slot0_occupied) {
        return true;
    }
    // 009F5627/009F562C: category 1 or 2 returns false at 009F5631.
    return !command_category_owns_fire_target(slot0_category);
}

bool store_fire_target_accepted(bool force, bool fire_target_is_primary,
                                bool fire_target_held) noexcept {
    // 00836244: a non-zero force byte jumps straight to the store.
    if (force) {
        return true;
    }
    // 00836248: fireTargetIsPrimary equal to the (zero) force byte also jumps
    // to the store, so a clear flag never blocks.
    if (!fire_target_is_primary) {
        return true;
    }
    // 00836250: a held target with the flag set returns at 00836297.
    return !fire_target_held;
}

// ---------------------------------------------------------------------------
// 008624C0
// ---------------------------------------------------------------------------

WeaponPanelSyncResult weapon_panel_sync(WeaponPanelMirror& mirror, bool force,
                                        const DirectorPermissions& permissions) noexcept {
    WeaponPanelSyncResult result;

    // 008624C6: panel+7Ch = controller+3Dh, unconditionally and before anything
    // else. allowMove is mirrored every call, never diffed.
    result.allow_move_copied = true;

    // 008624E5: view+8h -= 1. The refresh runs when `force` is set or the
    // countdown has run out, and the counter is then reloaded with 10.
    mirror.countdown -= 1;
    if (force || mirror.countdown <= 0) {
        result.refresh_all = true;
        mirror.countdown = kWeaponPanelRefreshPeriod;
    }

    // 008624F2: allowFire is rewritten when the refresh is due or the cached
    // byte differs. Setting it fills the panel's twelve enable bytes with ones;
    // clearing it zeroes them, then re-enables one class the panel object's
    // +50h subject still answers for.
    if (result.refresh_all || mirror.last_allow_fire != permissions.allow_fire) {
        mirror.last_allow_fire = permissions.allow_fire;
        result.rewrote_fire_enables = true;
        result.set_fire_enables = permissions.allow_fire;
    }

    // 00862566, 0086258B, 008625B0: each of the three is pushed through its own
    // setter only when the refresh is due or the cached byte differs.
    if (result.refresh_all || mirror.last_torpedo_enabled != permissions.torpedo_enabled) {
        mirror.last_torpedo_enabled = permissions.torpedo_enabled;
        result.torpedo_pushed = true; // 00861D70
    }
    if (result.refresh_all || mirror.last_artillery_enabled != permissions.artillery_enabled) {
        mirror.last_artillery_enabled = permissions.artillery_enabled;
        result.artillery_pushed = true; // 00861CD0
    }
    if (result.refresh_all || mirror.last_aa_enabled != permissions.aa_enabled) {
        mirror.last_aa_enabled = permissions.aa_enabled;
        result.aa_pushed = true; // 00861D20
    }
    // 008625D3 is the exception: depthChargeEnabled is diffed only, never forced
    // by the periodic refresh.
    if (mirror.last_depth_charge_enabled != permissions.depth_charge_enabled) {
        mirror.last_depth_charge_enabled = permissions.depth_charge_enabled;
        result.depth_charge_pushed = true; // 00861DC0
    }

    return result;
}

// ---------------------------------------------------------------------------
// 0071F290's arms
// ---------------------------------------------------------------------------

bool session_is_live(const SessionLiveFlags& flags) noexcept {
    // 0071F29F JZ out, 0071F2A9 JNZ out, 0071F2B3 JNZ out, 0071F2BD JNZ out.
    return flags.flag_5c && !flags.flag_5d && !flags.flag_60 && !flags.flag_5e;
}

int promote_mode_to_queue_head(int mode, bool slot0_occupied) noexcept {
    // 0071F317 CMP [ESI+30h],0 / JNZ past; 0071F31D CMP [ESI+54h],0 / JZ past;
    // 0071F323 MOV [ESI+30h],1.
    if (mode == 0 && slot0_occupied) {
        return 1;
    }
    return mode;
}

bool begin_command_arm_runs(const BeginCommandArm& arm) noexcept {
    // 0071F32A / 0071F34F: a set accepted byte skips the arm. 0071F330 /
    // 0071F355: so does a missing command.
    return !arm.already_accepted && arm.command_present;
}

CommandControllerUpdateTrace run_command_controller_update(
    const CommandControllerUpdateState& state, float frame_delta,
    CommandControllerUpdateHost& host) {
    CommandControllerUpdateTrace trace;
    trace.auto_target_hold = state.auto_target_hold;
    trace.mode = state.mode;

    // Arm 1, 0071F294..0071F2C1. Every failure jumps to 0071F3A0, the epilogue,
    // so nothing else in the routine runs.
    if (!state.session_present || !session_is_live(state.session_flags)) {
        return trace;
    }
    trace.session_gate_passed = true;

    // Arm 2, 0071F2C7..0071F2F7. Path object 0 only; the other nine pointers in
    // the +1A4h array are not touched here.
    if (state.path_object0_present) {
        host.reset_path_vector();
        trace.path_vector_reset = true;
    }

    // Arm 3, 0071F2F8..0071F316. docs/DIRECTOR_TARGET_GATE.md owns the rule;
    // update_auto_target_hold is its implementation.
    const float next_hold = update_auto_target_hold(state.auto_target_hold, frame_delta);
    // The JB at 0071F30A is taken on below *and* unordered, so a NaN hold is
    // left alone exactly as a negative one is.
    trace.hold_decremented = state.auto_target_hold >= 0.0f;
    trace.auto_target_hold = next_hold;

    // Arm 4, 0071F317..0071F329.
    const int mode = promote_mode_to_queue_head(state.mode, state.slot0_occupied);
    trace.mode_promoted = mode != state.mode;
    trace.mode = mode;

    // Arm 5, 0071F32A..0071F34E, the override.
    BeginCommandArm override_arm;
    override_arm.already_accepted = state.override_accepted;
    override_arm.command_present = state.override_command_present;
    if (begin_command_arm_runs(override_arm)) {
        trace.override_begin_attempted = true;
        if (!host.begin_command(kBeginCommandOverride)) {
            host.raise_override_stage(kCommandStageTerminal);
            trace.override_terminated = true;
        }
    }

    // Arm 6, 0071F34F..0071F372, the queue head. Note the presence test is the
    // same [+54h] slot-0 pointer arm 4 read, not a separate field.
    BeginCommandArm queue_arm;
    queue_arm.already_accepted = state.queue_accepted;
    queue_arm.command_present = state.slot0_occupied;
    if (begin_command_arm_runs(queue_arm)) {
        trace.queue_begin_attempted = true;
        if (!host.begin_command(kBeginCommandQueueHead)) {
            host.raise_queue_stage(kCommandStageTerminal);
            trace.queue_terminated = true;
        }
    }

    // Arm 7, 0071F373..0071F39F. Session mode 2 returns before both steps.
    if (state.session_mode == kSessionModeNoSimulation) {
        return trace;
    }
    if (state.auto_target_present) {
        host.step_auto_target(frame_delta);
        trace.auto_target_stepped = true;
    }
    host.step_commands();
    trace.commands_stepped = true;
    return trace;
}

} // namespace bsp
