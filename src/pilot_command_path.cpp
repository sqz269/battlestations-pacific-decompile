#include "bsp/pilot_command_path.hpp"

#include <cmath>

// docs/PILOT_COMMAND_PATH.md is the evidence for every rule below. Read-only Ghidra analysis,
// packet cc2_pilot_command_path; the descriptive names are hypotheses, the Lua key strings and
// the two GUI tokens are recovered.

namespace bsp {
namespace {

// 00D7A260 and 00D7A24C, the bipolar bounds; 0.0f is an immediate in 0099BC00.
constexpr float kAxisLow = -1.0f;
constexpr float kAxisHigh = 1.0f;
constexpr float kUnipolarLow = 0.0f;

}  // namespace

const char* pilot_cmd_axis_name_007d5d20(PilotCmdAxis axis) noexcept {
    switch (axis) {
        case PilotCmdAxis::kYaw:
            return "yawInput";  // 00D05D94, key site 007D69D6 for the pointer at 007D69BA
        case PilotCmdAxis::kPitch:
            return "pitchInput";  // 00D05D7C, key site 007D6A44
        case PilotCmdAxis::kRoll:
            return "rollInput";  // 00D05D88, key site 007D6A0D
        case PilotCmdAxis::kPower:
            return "pwrInput";  // 00D05DA0, key site 007D699F
        case PilotCmdAxis::kAirBrake:
            return "airBrakeInput";  // 00D05D6C, key site 007D6A7B
    }
    return "";
}

const char* pilot_cmd_latched_name_007d5d20(PilotCmdAxis axis) noexcept {
    switch (axis) {
        case PilotCmdAxis::kYaw:
            return "yawF";  // 00D05D30 on unit+BB0h, key site 007D6BB2
        case PilotCmdAxis::kPitch:
            return "pitchF";  // 00D05D14 on unit+BB4h, key site 007D6C57
        case PilotCmdAxis::kRoll:
            return "rollF";  // 00D05D38 on unit+BB8h, key site 007D6B7B
        case PilotCmdAxis::kPower:
            return "pwrF";  // 00D05D1C on unit+BBCh, key site 007D6C20
        case PilotCmdAxis::kAirBrake:
            return "airBrakeF";  // 00D05D24 on unit+BC0h, key site 007D6BE9
    }
    return "";
}

bool pilot_cmd_axis_is_unipolar_0099bc00(PilotCmdAxis axis) noexcept {
    // 0099BC1C-0099BC5D clamps the power command to [0, 1] and 0099BD4E-... the air brake;
    // the three attitude arms use the -1.0f lower bound from 00D7A260.
    return axis == PilotCmdAxis::kPower || axis == PilotCmdAxis::kAirBrake;
}

float pilot_cmd_slew_slot_0099bb40(const UnitPlanSlot& slot, float rate, float dt) noexcept {
    // 0099BB43 FLD [ECX+4] and 0099BB49 FLD [ECX]: the difference is pending - committed.
    const float step = rate * dt;  // 0099BB61 FLD [ESP+0Ch]; FMUL [ESP+10h]
    const float difference = slot.pending - slot.committed;
    // 0099BB78 AND EAX,7FFFFFFFh is the absolute value; 0099BB89 FCOMI, 0099BB8D JBE.
    if (std::fabs(difference) < step) {
        return slot.pending;  // 0099BB8F-0099BB9B
    }
    if (difference < 0.0f) {
        return slot.committed - step;  // the FMUL by -1.0f arm
    }
    // The native tail multiplies the step by (int)(0.0f < difference), so a difference of
    // exactly zero that also failed the |d| < step test returns the committed value unchanged.
    return slot.committed + (difference > 0.0f ? step : 0.0f);
}

float pilot_cmd_clamp_unipolar_0099bc00(float value) noexcept {
    // if (0.0 <= v) { if (1.0 < v) v = 1.0; } else v = 0.0;
    if (kUnipolarLow <= value) {
        return (kAxisHigh < value) ? kAxisHigh : value;
    }
    return kUnipolarLow;
}

float pilot_cmd_clamp_bipolar_0099bc00(float value) noexcept {
    // if (-1.0 <= v) { if (1.0 < v) v = 1.0; } else v = -1.0;
    if (kAxisLow <= value) {
        return (kAxisHigh < value) ? kAxisHigh : value;
    }
    return kAxisLow;
}

PilotCommandBlock pilot_cmd_evaluate_plan_slots_0099bc00(const PilotPlanSlots& slots, float rate,
                                                        float dt) noexcept {
    PilotCommandBlock command{};
    // The slot order is the seeding order in 0099B450 and the walk order in 0099BC00:
    // +274h power, +280h yaw, +28Ch roll, +298h pitch, +2A4h air brake. PilotPlanSlots keeps
    // the native offsets under the older member names, so `roll` is the +280h (yaw) triple and
    // `yaw` is the +28Ch (roll) triple.
    command.throttle = pilot_cmd_clamp_unipolar_0099bc00(
        pilot_cmd_slew_slot_0099bb40(slots.throttle, rate, dt));  // 0099BC17, store 0099BC5D
    command.roll = pilot_cmd_clamp_bipolar_0099bc00(
        pilot_cmd_slew_slot_0099bb40(slots.roll, rate, dt));  // 0099BC65, store 0099BCB0
    command.yaw = pilot_cmd_clamp_bipolar_0099bc00(
        pilot_cmd_slew_slot_0099bb40(slots.yaw, rate, dt));  // 0099BCB7, store 0099BD02
    command.pitch = pilot_cmd_clamp_bipolar_0099bc00(
        pilot_cmd_slew_slot_0099bb40(slots.pitch, rate, dt));  // 0099BD0A
    command.fifth = pilot_cmd_clamp_unipolar_0099bc00(
        pilot_cmd_slew_slot_0099bb40(slots.fifth, rate, dt));  // 0099BD5A
    return command;
}

PilotCommandBlock pilot_cmd_evaluate_plan_slots_0099bee0(const PilotPlanSlots& slots,
                                                        const PilotCmdBytes& bytes, float rate,
                                                        float dt) noexcept {
    PilotCommandBlock command = pilot_cmd_evaluate_plan_slots_0099bc00(slots, rate, dt);
    // 0099BF07-0099BF1F. Only out+14h is carried into the control block: 007B8C90 copies the
    // sixth dword whole, the quantiser tests unit+A10h at 007BB7B9 and 007BB7ED, and nothing
    // in the image references unit+A11h or unit+A12h.
    command.boost_request = bytes.low != 0;
    return command;
}

bool pilot_cmd_should_zero_pitch_007d1331(float angle_radians) noexcept {
    // 007D131D MOVSS XMM0,[00CE3990]; 007D1325 COMISS XMM0,[ESI+C68h]; 007D132C JBE.
    // The store runs only when the constant is strictly greater, so an unordered compare skips.
    return kPilotCmdPitchZeroAngle > angle_radians;
}

bool pilot_cmd_local_input_enabled_007bb9a0(const PilotCmdLocalInputGate& gate) noexcept {
    if (!gate.enable_byte_c0c) {
        return false;  // 007BB9AA JZ 007BBA09
    }
    // 007BB9B4 COMISS against 00D7A218 = 0.0f, 007BB9BB JA: only a value above zero refuses,
    // so an unordered compare falls through the way the native code does.
    if (gate.gate_float_aa0 > 0.0f) {
        return false;
    }
    if (gate.gate_block_present && gate.gate_block_value != kAxisHigh) {
        return false;  // 007BB9D0 UCOMISS / LAHF / TEST AH,44h / JP: continue only on equality
    }
    if (gate.controller_present && gate.controller_blocks) {
        return false;  // 007BB9EE JNZ 007BBA09
    }
    if (gate.global_block_00604a20) {
        return false;  // 007BB9F7 JNZ 007BBA05
    }
    if (gate.unit_suppressed) {
        return false;  // 007BB9FC JNZ 007BBA05
    }
    return true;  // 007BB9FE MOV EAX,1
}

PilotCmdApplyResult pilot_cmd_apply_state_message_007d1360(
    const PilotCmdStateMessage& message) noexcept {
    PilotCmdApplyResult out{};
    // 007D166D-007D169A: the control block, in memory order +9E4h..+9F8h. PlaneControlInput
    // keeps the native offsets under the older member names.
    out.control.roll = message.axes[0];      // +9E4h yawInput
    out.control.pitch = message.axes[1];     // +9E8h pitchInput
    out.control.yaw = message.axes[2];       // +9ECh rollInput
    out.control.throttle = message.axes[3];  // +9F0h pwrInput
    out.control.aux = message.axes[4];       // +9F4h airBrakeInput
    out.control.byte_f8 = static_cast<std::uint8_t>(message.byte_word & 0xFFu);
    // 007D16A3-007D16D4: the same six dwords into the command block. unit+A14h is never set,
    // so the quantiser does not re-run over them.
    out.command.roll = message.axes[0];
    out.command.pitch = message.axes[1];
    out.command.yaw = message.axes[2];
    out.command.throttle = message.axes[3];
    out.command.fifth = message.axes[4];
    out.command.boost_request = (message.byte_word & 0xFFu) != 0u;
    out.command.pending = false;
    return out;
}

PilotCmdStateMessage pilot_cmd_build_state_message_007bdd30(const PlaneControlInput& control,
                                                           std::uint32_t byte_word) noexcept {
    // 007BDEAF-007BDED6 reads the six dwords at [&unit+9E4h]+0h..+14h, so the message carries
    // the control block, not the command block.
    PilotCmdStateMessage message{};
    message.axes[0] = control.roll;
    message.axes[1] = control.pitch;
    message.axes[2] = control.yaw;
    message.axes[3] = control.throttle;
    message.axes[4] = control.aux;
    message.byte_word = byte_word;
    return message;
}

bool run_pilot_bot_command_tick_0099acd0(PilotCmdBotTickHost& host, float dt) {
    if (!host.unit_present()) {
        return false;  // 0099ACD9 JZ 0099B19F
    }
    void* const head_before = host.first_task();  // 0099AE72-0099AE79
    host.release_stale_tasks(head_before);        // 0099AE7E, 0099A4C0
    void* const task = host.first_task();         // 0099AE89-0099AE90
    if (task != head_before) {
        host.on_task_head_changed(task);  // 0099AE96-0099AEBD
    }
    if (task == nullptr) {
        return false;  // 0099AEC4 JZ 0099B11C
    }
    if (host.task_step_flag_clear(task)) {
        host.set_think_timer(kPilotCmdForceThinkTimer);  // 0099AEE5-0099AEED
    }
    const float timer = host.think_timer() - dt;  // 0099AEF2-0099AF07
    host.set_think_timer(timer);
    if (timer > 0.0f) {
        return false;  // 0099AF10 JB 0099B11A
    }
    host.update_task(task, dt);  // 0099AF1C, 009998A0 BSP_PilotBot_Update
    if (host.unit_step_flag_set()) {
        host.set_think_timer(kPilotCmdSuppressTimer);  // 0099AF3D-0099AF4A
        return false;
    }
    // 0099AFCC-0099B001: the first task whose vtable[+50h] is not 1 decides the mode.
    const int mode = host.task_control_mode(task);
    host.publish_task_mode(mode);                    // 0099B00C, unit+DF8h
    host.set_step_flag(host.peer_step_flag(mode));   // 0099B055, 007BB150
    // 0099B05A-0099B09A: the stack buffer, zeroed, with the power command at 1.0f.
    PilotPlanSlots slots = host.plan_slots(task);
    PilotCommandBlock command =
        pilot_cmd_evaluate_plan_slots_0099bee0(slots, host.plan_bytes(task), kPilotCmdSlewRate, dt);
    command = host.repair_command(task, command);  // 0099B0AC, 0099BF30
    host.set_pilot_command_block(command);         // 0099B0B9, 007B8C90
    host.run_task_tail(task);                      // 0099B0BE-0099B113
    return true;
}

PilotCmdScreenOutcome run_pilot_screen_update_00519bb0(PilotCmdScreenUpdateHost& host, float dt) {
    if (host.global_gate_blocked()) {
        return PilotCmdScreenOutcome::kBlockedByGlobalGate;  // 00519BD9 / 00519BE5
    }
    host.set_role_text(host.unit_is_local_player_pilot() ? kPilotCmdRoleTextPlayer
                                                         : kPilotCmdRoleTextBot);
    if (!host.local_input_enabled()) {
        host.hand_off_to_spectator();  // 00519CA3-00519CDB
        return PilotCmdScreenOutcome::kHandedOffToSpectator;
    }
    host.update_hud_panels(dt);     // 00519CE8, 005191B0
    host.build_player_command(dt);  // 00519CF7, 00519520 -> 007B8C90
    host.poll_unit_parts();         // 00519CFE, 00519020
    host.update_tail(0.5f);         // 00519D1B and 00519D29, the 00CE3800 constant
    return PilotCmdScreenOutcome::kCommandIssued;
}

}  // namespace bsp
