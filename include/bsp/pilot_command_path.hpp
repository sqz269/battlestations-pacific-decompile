#pragma once
#include <cstdint>

#include "bsp/pilot_controls.hpp"  // PilotCommandBlock, PilotPlanSlots, pilot_command_off
#include "bsp/plane_flight.hpp"    // PlaneControlInput
#include "bsp/unit_orders.hpp"     // UnitPlanSlot

// Projection of the two producers of the plane's pilot command block unit+9FCh and of the
// paths that write the control block unit+9E4h behind the quantiser: the pilot bot's think
// tick 0099ACD0, the pilot HUD screen's update 00519BB0, the local-input gate 007BB9A0, the
// plan-slot slew 0099BB40 / 0099BC00 / 0099BEE0, the band repair 0099BF30, and the control
// state message pair 007C2880 (send) / 007D1360 (apply).
//
// docs/PILOT_COMMAND_PATH.md carries the evidence, the coverage table and the corrections.
// docs/PILOT_CONTROLS.md is the contract for 007B8C90, 007BB920 and the quantiser 007BB6E0;
// docs/PLANE_FLIGHT.md for the latch 007B9770 and the rate law 007DA710.
//
// Every name here is a hypothesis, not a recovered symbol, EXCEPT the five axis names and the
// five latched names, which are the game's own Lua keys read out of 007D5D20 (see
// kPilotCmdAxisName below). Nothing in this header is a binary-compatible layout: the offset
// constants are the native ones, the structs are not.
//
// Contracts named but not reconstructed: the callers of 0099ACD0 and 00519BB0 (both
// virtual-only); 0099B940's band table; 0099CFF40, 00999F50, 0099A4C0, 007B9140, 0042A7E0,
// 007D7A80, 007D8330, 00779FC0, 00779F80, 00637620, 005484F0, 007BA7B0.

namespace bsp {

// ---------------------------------------------------------------------------
// The recovered axis order. `PilotCommandBlock` (include/bsp/pilot_controls.hpp) and
// `PlaneControlInput` (include/bsp/plane_flight.hpp) have the right offsets under the wrong
// names: their `roll` member is the native +9FCh / +9E4h slot, which 007D5D20 reads as
// `yawInput`, and their `yaw` member is +A04h / +9ECh, which it reads as `rollInput`. The
// enumerator order below is the native memory order of the control block, not the slot order.
// ---------------------------------------------------------------------------
enum class PilotCmdAxis {
    kYaw = 0,       // +9E4h yawInput      / command +9FCh, PilotCommandBlock::roll
    kPitch = 1,     // +9E8h pitchInput    / command +A00h, PilotCommandBlock::pitch
    kRoll = 2,      // +9ECh rollInput     / command +A04h, PilotCommandBlock::yaw
    kPower = 3,     // +9F0h pwrInput      / command +A08h, PilotCommandBlock::throttle
    kAirBrake = 4,  // +9F4h airBrakeInput / command +A0Ch, PilotCommandBlock::fifth
};

// The Lua key 007D5D20 reads each axis under. Recovered strings, not hypotheses.
const char* pilot_cmd_axis_name_007d5d20(PilotCmdAxis axis) noexcept;
// The `dynamics` group name of the latched copy at unit+BB0h..+BC0h.
const char* pilot_cmd_latched_name_007d5d20(PilotCmdAxis axis) noexcept;

// true for the two unipolar axes: 0099BC00 clamps power and the air brake to [0, 1] and the
// three attitude axes to [-1, 1].
bool pilot_cmd_axis_is_unipolar_0099bc00(PilotCmdAxis axis) noexcept;

// ---------------------------------------------------------------------------
// Offsets. The control-block and command-block displacements live in
// pilot_controls.hpp's pilot_command_off; these are the ones this packet added.
// ---------------------------------------------------------------------------
namespace pilot_cmd_path_off {
// The plan-slot triples on the bot object 0099BEE0 takes as `this` (task+4h). Stride 0Ch,
// base +274h, in the order 0099B450 seeds them.
inline constexpr int kPilotCmdPlanSlotBase = 0x274;    // 0099BEF7 LEA ECX,[ESI+274h]
inline constexpr int kPilotCmdPlanSlotStride = 0x0C;   // 0099BC56 / 0099BCA9 / 0099BCFB
inline constexpr int kPilotCmdPlanSlotPower = 0x274;   // 0099B466, from unit+9F0h
inline constexpr int kPilotCmdPlanSlotYaw = 0x280;     // 0099B486, from unit+9E4h
inline constexpr int kPilotCmdPlanSlotRoll = 0x28C;    // 0099B4A4, from unit+9ECh
inline constexpr int kPilotCmdPlanSlotPitch = 0x298;   // 0099B4C2, from unit+9E8h
inline constexpr int kPilotCmdPlanSlotBrake = 0x2A4;   // 0099B4E0, from unit+9F4h
inline constexpr int kPilotCmdPlanUnit = 0x2F0;        // 0099B450, 0099BF3x
inline constexpr int kPilotCmdPlanThrottleFloorGate = 0x258;  // 0099BF30's bot+258h
inline constexpr int kPilotCmdPlanByteHigh = 0x2DC;    // 0099BEE0 -> out+16h
inline constexpr int kPilotCmdPlanByteLow = 0x2E4;     // 0099BEE0 -> out+14h
inline constexpr int kPilotCmdPlanByteMid = 0x2E5;     // 0099BEE0 -> out+15h

// The bot object 0099ACD0 takes as `this`.
inline constexpr int kPilotCmdBotUnit = 0x50;        // 0099ACD6, 0099B0B1
inline constexpr int kPilotCmdBotTaskVector = 0x58;  // 0099AE72
inline constexpr int kPilotCmdBotTaskCount = 0x5C;   // 0099AE6C
inline constexpr int kPilotCmdBotThinkTimer = 0x74;  // 0099AEF2, decremented by dt

// The unit fields the two paths gate on.
inline constexpr int kPilotCmdUnitPilotSlot = 0x1B0;     // role slot 1, 00927F30(unit, 1)
inline constexpr int kPilotCmdUnitLocalGate = 0xC0C;     // 007BB9A3, cleared at 007D5D52
inline constexpr int kPilotCmdUnitGateFloat = 0xAA0;     // 007BB9B4, must be <= 0.0f
inline constexpr int kPilotCmdUnitGateBlock = 0xDEC;     // 007BB9BD, +44h byte / +48h float
inline constexpr int kPilotCmdUnitController = 0x9D4;    // 007BB9DD, +3B0h byte
inline constexpr int kPilotCmdUnitSuppress = 0x5D;       // 007BB9F9, 007D1305
inline constexpr int kPilotCmdUnitStepRecord = 0x9C0;    // eight bytes per double-buffer index
inline constexpr int kPilotCmdUnitTaskBudget = 0xC58;    // 0099AF53, 0099AFB6, 0099AFC2
inline constexpr int kPilotCmdUnitTaskMode = 0xDF8;      // 0099B00C
inline constexpr int kPilotCmdUnitStepFlag = 0xDF0;      // 007BB150's destination
inline constexpr int kPilotCmdUnitPitchZeroAngle = 0xC68;  // 007D1325, 0099BF30's sign
inline constexpr int kPilotCmdUnitPitchZeroLatch = 0xC3A;  // 007D12FC, set at 007D1314

// The control-state message, kind 0C1h. 007BDD30 fills it, 007D1360 applies it.
inline constexpr int kPilotCmdMessageAxes = 0x38;   // +38h..+4Ch, six dwords
inline constexpr int kPilotCmdMessageRecord = 0x1C;  // 007BDD30's param_3, the 0x84 record
inline constexpr int kPilotCmdApplyByteA = 0x9E0;   // 007D174D sets, 007D1AA8 clears
inline constexpr int kPilotCmdApplyByteB = 0x9E1;   // 007D1642 = host flag, 007D1724 = 1
}  // namespace pilot_cmd_path_off

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
inline constexpr float kPilotCmdSlewRate = 4.0f;        // 00CE3D34, 0099ACD0's second argument
inline constexpr float kPilotCmdSuppressTimer = 3.0f;   // 00CE3854, 0099AF3D
inline constexpr float kPilotCmdForceThinkTimer = -1.0f;  // 00D7A260, 0099AEB5 / 0099AEE5
inline constexpr float kPilotCmdRepairOvershoot = 1.1f;   // 00CE6448; 00D06BB0 is its negative
inline constexpr float kPilotCmdThrottleFloor = 0.01f;    // 00D7A238, 0099BF30
inline constexpr float kPilotCmdPitchZeroAngle = 0.1745329f;  // 00CE3990, ten degrees
inline constexpr int kPilotCmdPilotRoleSlot = 1;        // 00519BF0's 00927F30 argument
inline constexpr int kPilotCmdMessageKind = 0xC1;       // 007BDD30's ConstructBase argument
inline constexpr int kPilotCmdLeaveInterfaceId = 0x24;  // 00519CC5, 004CC460's request
inline constexpr int kPilotCmdHostNetworkMode = 1;      // 007D1374, game+1FE4h
inline constexpr int kPilotCmdClientNetworkMode = 2;    // 007D0BC1, game+1FE4h

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 0099BB40, the plan-slot slew limiter. Moves `slot.current` toward `slot.desired` by
// `rate * dt` and returns the result; it does not write back.
float pilot_cmd_slew_slot_0099bb40(const UnitPlanSlot& slot, float rate, float dt) noexcept;

// 0099BC00's two clamps. The native form is `if (low <= v) { if (high < v) v = high; }
// else v = low;`, so an unordered compare yields the low bound; the projection keeps that.
float pilot_cmd_clamp_unipolar_0099bc00(float value) noexcept;
float pilot_cmd_clamp_bipolar_0099bc00(float value) noexcept;

// 0099BC00 whole: slew every slot at `rate` over `dt` and clamp it into the command block.
// The fields written are PilotCommandBlock's, under its own (native-offset) member names.
PilotCommandBlock pilot_cmd_evaluate_plan_slots_0099bc00(const PilotPlanSlots& slots, float rate,
                                                         float dt) noexcept;

// 0099BEE0: 0099BC00 plus the three byte copies. Only `low` survives into the control block -
// `mid` and `high` land at unit+A11h and unit+A12h, which have no reader in the image.
struct PilotCmdBytes {
    std::uint8_t low{0};   // bot+2E4h -> out+14h -> unit+A10h -> the byte unit+9F8h
    std::uint8_t mid{0};   // bot+2E5h -> out+15h -> unit+A11h, dead
    std::uint8_t high{0};  // bot+2DCh -> out+16h -> unit+A12h, dead
};
PilotCommandBlock pilot_cmd_evaluate_plan_slots_0099bee0(const PilotPlanSlots& slots,
                                                         const PilotCmdBytes& bytes, float rate,
                                                         float dt) noexcept;

// 007D1331: zero the pitch input when the angle at unit+C68h is under ten degrees.
bool pilot_cmd_should_zero_pitch_007d1331(float angle_radians) noexcept;

// 007BB9A0, the local-input gate. One field per native test, in the native order.
struct PilotCmdLocalInputGate {
    bool enable_byte_c0c{false};     // 007BB9A3, unit+C0Ch
    float gate_float_aa0{0.0f};      // 007BB9B4, must not be above 0.0f
    bool gate_block_present{false};  // 007BB9C6, [unit+DECh]+44h
    float gate_block_value{0.0f};    // 007BB9CB, [unit+DECh]+48h, must equal 1.0f when present
    bool controller_present{false};  // 007BB9DD, unit+9D4h
    bool controller_blocks{false};   // 007BB9E7, [unit+9D4h]+3B0h
    bool global_block_00604a20{false};  // 007BB9F0
    bool unit_suppressed{false};        // 007BB9F9, byte unit+5Dh
};
bool pilot_cmd_local_input_enabled_007bb9a0(const PilotCmdLocalInputGate& gate) noexcept;

// 007D1360's restore. The same six dwords reach both blocks and unit+A14h is never set, so the
// apply bypasses the quantiser.
struct PilotCmdStateMessage {
    float axes[5]{0.0f, 0.0f, 0.0f, 0.0f, 0.0f};  // msg+38h..+48h
    std::uint32_t byte_word{0};                    // msg+4Ch, reaches unit+9F8h and unit+A10h
};
struct PilotCmdApplyResult {
    PlaneControlInput control{};  // unit+9E4h..+9F8h
    PilotCommandBlock command{};  // unit+9FCh..+A10h; `pending` stays false
};
PilotCmdApplyResult pilot_cmd_apply_state_message_007d1360(
    const PilotCmdStateMessage& message) noexcept;

// 007C2880/007BDEAF: the send takes the message's six dwords from the control block, which is
// what makes the apply's source the control block and not the command block.
PilotCmdStateMessage pilot_cmd_build_state_message_007bdd30(const PlaneControlInput& control,
                                                            std::uint32_t byte_word) noexcept;

// ---------------------------------------------------------------------------
// 0099ACD0, the pilot bot's think tick. One method per native call site in the
// command-production window 0099AE6C-0099B1A3; the maintenance pass before it is unread.
// ---------------------------------------------------------------------------
class PilotCmdBotTickHost {
public:
    virtual ~PilotCmdBotTickHost() = default;
    virtual bool unit_present() = 0;                 // 0099ACD9, bot+50h
    virtual void* first_task() = 0;                  // 0099AE72 / 0099AE89, bot+58h[0]
    virtual void release_stale_tasks(void* head) = 0;  // 0099AE7E, 0099A4C0(bot, head)
    virtual void on_task_head_changed(void* head) = 0;  // 0099AE96-0099AEBD
    // 0099AEDB, [[task+274h] + idx*8 + 9C2h]. `idx` is NOT a parameter and must
    // not be chosen by the host: it is the global word at 00F876B8, which
    // 009998B7 reads here and 0099D316 reads again inside the planner. Both
    // sides scale it by 8. A host that supplies its own index desynchronises
    // this early-out from the planner's own gate, which would look like a
    // sporadically skipped plan rather than an error.
    // docs/PILOT_BOT_TASK_OBJECT.md.
    virtual bool task_step_flag_clear(void* task) = 0;
    virtual void set_think_timer(float value) = 0;    // 0099AEBD / 0099AEED / 0099AF45, bot+74h
    virtual float think_timer() = 0;                  // 0099AEF2
    virtual void update_task(void* task, float dt) = 0;  // 0099AF1C, 009998A0
    virtual bool unit_step_flag_set() = 0;            // 0099AF33, [unit+idx*8+9C2h]
    virtual int task_control_mode(void* task) = 0;    // 0099AFE8, task vtable[+50h]
    virtual void publish_task_mode(int mode) = 0;     // 0099B00C, unit+DF8h
    virtual std::uint8_t peer_step_flag(int mode) = 0;  // 0099B042, the mode-1 override
    virtual void set_step_flag(std::uint8_t value) = 0;  // 0099B055, 007BB150
    virtual PilotPlanSlots plan_slots(void* task) = 0;   // the task+278h triples
    virtual PilotCmdBytes plan_bytes(void* task) = 0;    // bot+2DCh / +2E4h / +2E5h
    virtual PilotCommandBlock repair_command(void* task, PilotCommandBlock command) = 0;
                                                      // 0099B0AC, 0099BF30
    virtual void set_pilot_command_block(const PilotCommandBlock& command) = 0;
                                                      // 0099B0B9, 007B8C90
    virtual void run_task_tail(void* task) = 0;       // 0099B0BE-0099B113
};

// Returns true when the command reached 007B8C90 this step. The three refusals are a missing
// task (0099AEC2), a still-positive think timer (0099AF10) and the step flag (0099AF3B).
bool run_pilot_bot_command_tick_0099acd0(PilotCmdBotTickHost& host, float dt);

// ---------------------------------------------------------------------------
// 00519BB0, the pilot HUD screen's update. One method per native call site.
// ---------------------------------------------------------------------------
enum class PilotCmdScreenOutcome {
    kBlockedByGlobalGate,  // 00519BD9 / 00519BE5
    kHandedOffToSpectator,  // 00519CA3-00519CDB, 007BB9A0 refused
    kCommandIssued,         // 00519CE8-00519D29
};

class PilotCmdScreenUpdateHost {
public:
    virtual ~PilotCmdScreenUpdateHost() = default;
    virtual bool global_gate_blocked() = 0;  // 00519BD1 / 00519BDF, [00E188A8]+61Fh / +620h
    virtual bool unit_is_local_player_pilot() = 0;  // 00519BF0, 00927F30(unit, 1)
    virtual void set_role_text(const char* token) = 0;  // 00519C20 / 00519C64, 00AA7E00
    virtual bool local_input_enabled() = 0;  // 00519C9A, 007BB9A0
    virtual void hand_off_to_spectator() = 0;  // 00519CB6 / 00519CC7, 00647300 + 004CC460
    virtual void update_hud_panels(float dt) = 0;  // 00519CE8, 005191B0
    virtual void build_player_command(float dt) = 0;  // 00519CF7, 00519520
    virtual void poll_unit_parts() = 0;      // 00519CFE, 00519020
    virtual void update_tail(float half) = 0;  // 00519D1B + 00519D29, 00637620 + 005484F0
};

PilotCmdScreenOutcome run_pilot_screen_update_00519bb0(PilotCmdScreenUpdateHost& host, float dt);

// The two GUI tokens 00519BB0 writes into `role_Text` (00CEC768). Recovered strings.
inline constexpr const char* kPilotCmdRoleTextPlayer = "ingame.pilotplayer";  // 00CEC754
inline constexpr const char* kPilotCmdRoleTextBot = "ingame.pilotai";         // 00CEC744

}  // namespace bsp
