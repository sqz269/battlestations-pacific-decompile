#pragma once
// Projection of what happens when a unit's command finishes: 0071E430, the
// stage ladder it feeds, the MT_GAMEUNIT_CLEARCMD round trip that turns a
// terminal stage into a queue advance, the standing command the controller
// falls back to, and the `command` mission-event channel that carries `start`
// and `finished` to mission Lua.
//
// Packet cc2_command_completion. docs/COMMAND_COMPLETION.md carries the
// addresses and the evidence per claim. Every descriptive name here is a
// hypothesis, not a recovered symbol.
//
// Reused, never redefined: include/bsp/command_execution.hpp owns CommandMode,
// CommandSlot, CommandParams, the controller offsets, the kCommandStage*
// values and the kCommand<class> singleton addresses, and it already projects
// 00720850 as clear_command_slot_00720850. Nothing here reimplements the queue
// removal; this header stops at the decision to remove and resumes at what the
// controller looks like afterwards.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/command_execution.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// 0071E430 BSP_WeaponDirector_EndCommand
// ---------------------------------------------------------------------------
// __thiscall(controller)(void* command, char terminal), RET 8 at 0071E469,
// 0071E486 and 0071E4B7, body 0071E430-0071E4B9.

// The stage the routine asks for. 0071E454-0071E45F is XOR EAX,EAX / CMP
// byte [ESP+0Ch],AL / SETNZ AL / ADD EAX,1, and 0071E471 is the ECX twin.
constexpr int end_command_requested_stage(bool terminal) noexcept {
    return terminal ? kCommandStageDone : kCommandStageStarted;
}

// Which of the four arms 0071E430 takes.
enum class EndCommandArm {
    // 0071E454 by way of 0071E44A: a command whose category is neither 1 nor 2
    // skips the mode test entirely.
    kQueueStageByCategory,
    // 0071E44C then 0071E454: mode 1.
    kQueueStageByMode,
    // 0071E46C then 0071E480: mode 2.
    kOverrideStageByMode,
    // 0071E489: neither mode, at least one occupied slot. Not an ending at all:
    // mode = 1, +44h = 0, +48h = 0.
    kRestartQueueHead,
    // 0071E489 with an empty queue. The routine returns having done nothing.
    kNothing,
};

// The result of one 0071E430 call, as a value.
struct EndCommandDecision {
    EndCommandArm arm{EndCommandArm::kNothing};
    // Meaningful only for the two stage arms.
    int stage{kCommandStageFresh};
};

// `has_command` is `command != 0`; `command_category` is command->vtable[0Ch]()
// and is read only when `has_command`. `occupied_slots` is 0071BE60's count of
// leading non-empty slots.
EndCommandDecision end_command_decision(bool has_command, int command_category, CommandMode mode,
                                        int occupied_slots, bool terminal) noexcept;

// The controller fields 0071E430's restart arm writes, in one value so a caller
// can apply them without a host round trip.
struct QueueHeadRestart {
    bool applies{false};
    CommandMode mode{CommandMode::kQueueHead};  // always 1 when it applies
    bool queue_accepted{false};                 // +44h = 0
    int queue_stage{kCommandStageFresh};         // +48h = 0
};

QueueHeadRestart end_command_restart(const EndCommandDecision& decision) noexcept;

// ---------------------------------------------------------------------------
// The stage ladder, 0071D810 / 0071D9E0 / 0071C130
// ---------------------------------------------------------------------------

// Both raisers return early when the stored stage already meets the request, so
// a stage never falls except through a clear.
constexpr bool stage_raise_applies(int stored_stage, int requested_stage) noexcept {
    return stored_stage < requested_stage;
}

// 0071D81E / 0071D9EE: the message is originated only at stage 2 and only when
// the session mode at *(00E188A8)+1FE4h is not 2.
constexpr bool stage_raise_sends_message(int requested_stage, int session_mode) noexcept {
    return requested_stage == kCommandStageDone && session_mode != 2;
}

// 0071C130, reached as vtable[6Ch] -> 00835BF0. A non-zero argument clears the
// queue-head pair, zero clears the override pair.
struct StageResetResult {
    bool cleared_queue_pair{false};     // +44h, +48h
    bool cleared_override_pair{false};  // +4Ch, +50h
};

StageResetResult reset_command_stage_0071c130(CommandQueueState& state, bool primary) noexcept;

// 00836920's first two arms: when a stage-1 mark becomes a real ending.
// 00836985, the queue head. `queued_count` is 0071BE60's answer.
constexpr bool queue_stage_promotes_to_done(int queue_stage, int queued_count,
                                            bool unit_player_driven) noexcept {
    return queue_stage == kCommandStageStarted && (queued_count > 1 || unit_player_driven);
}

// 00836993, the override. No grace period.
constexpr bool override_stage_promotes_to_done(int override_stage) noexcept {
    return override_stage == kCommandStageStarted;
}

// ---------------------------------------------------------------------------
// MT_GAMEUNIT_CLEARCMD, message kind 5Dh, vtable 00CFD9D8
// ---------------------------------------------------------------------------
// 0071C730(isQueue, index), 0071D880 and 0071D900 build the same eight fields;
// only +20h and +24h differ between them.

inline constexpr int kSessionMessageClearCommand = 0x5d;
inline constexpr std::size_t kClearCommandMsgOffArm = 0x20;    // 0071C730, 0071D900
inline constexpr std::size_t kClearCommandMsgOffIndex = 0x24;  // 0071C730, 0071D900
inline constexpr std::int32_t kClearCommandAllSlots = -1;      // 0071D880

struct ClearCommandMessage {
    int kind{kSessionMessageClearCommand};
    // +20h. 0 addresses the override record, non-zero the queue.
    std::uint8_t arm{1};
    // +24h. Negative means every slot.
    std::int32_t index{0};
};

// 0071D810 at 0071D852: 0071C730(1, 0).
ClearCommandMessage clear_command_message_for_queue_stage_done() noexcept;
// 0071D9E0: 0071C730(0, 0).
ClearCommandMessage clear_command_message_for_override_stage_done() noexcept;
// 0071D900, called from 0071E550 at 0071E5AA with count - 1.
ClearCommandMessage clear_command_message_for_slot(int slot_index) noexcept;
// 0071D880 BSP_WeaponDirector_SendClearCommands.
ClearCommandMessage clear_command_message_for_all_slots() noexcept;

// 00721A40's 5Dh arm, 00721B9A-00721BCA.
enum class ClearCommandAction {
    kClearOverride,   // 00721BC5, 0071E610
    kClearAllSlots,   // 00721BA8, 00720CA0
    kClearSlot,       // 00721BB7, 00720850
};

ClearCommandAction clear_command_action(const ClearCommandMessage& message) noexcept;

// ---------------------------------------------------------------------------
// 00836920's idle tail, LAB_00836DC9
// ---------------------------------------------------------------------------

// 00836DCE-00836DE5: the tail runs at all only under this test.
// `no_command_flag` is the bool 00836941 computes, read at 00836DE0 as
// byte [ESP+0Bh].
constexpr bool idle_tail_runs(int queue_stage, int queued_count, bool no_command_flag) noexcept {
    return !((queue_stage != kCommandStageDone || queued_count > 1) && !no_command_flag);
}

// 00836DF4 SETZ CL against stage 2, 00836DFA CMP EAX,ECX / JG: a finished head
// does not count towards the queue.
constexpr bool idle_tail_issues(int queue_stage, int queued_count) noexcept {
    return queued_count <= (queue_stage == kCommandStageDone ? 1 : 0);
}

// What the tail issues. The target is the controlling entity for kFollow and
// the unit itself for the other two.
enum class StandingCommand {
    kFollow,  // 00E08F60, 00836E28
    kCruise,  // 00E08F70
    kStop,    // 00E08F88
};

// 00836E13-00836E78. `commanded_speed` is *(unit+73Ch)+28h, read at 00836E54
// after 00835BF0 has expired it; the test is COMISS against 00D7A218 = 0.0f.
StandingCommand standing_command_after_queue_empty(bool controller_belongs_to_another,
                                                   bool unit_player_driven,
                                                   float commanded_speed) noexcept;

// The singleton address the choice names.
std::uint32_t standing_command_address(StandingCommand which) noexcept;

// ---------------------------------------------------------------------------
// The attackmove target gate, 005457C0
// ---------------------------------------------------------------------------
// __thiscall(unit)(int side), RET 4. `unit+54h != side && side != 2`. The step
// raises stage 2 when this answers false, so it reads "still hostile".
constexpr bool attack_target_still_hostile(int unit_side, int target_side) noexcept {
    return unit_side != target_side && target_side != 2;
}

// ---------------------------------------------------------------------------
// The `command` mission-event channel, 00984300
// ---------------------------------------------------------------------------

// The only two status verbs in the image. 00CFDB1C and 00D09FD8.
inline constexpr char kCommandEventStatusStart[] = "start";
inline constexpr char kCommandEventStatusFinished[] = "finished";

// The four parameters 00984300 boxes, in the order 00968AF0 tests them.
struct CommandEventParameters {
    std::uint16_t unit_id{0};       // argument 1's +174h, 0 for a null entity
    std::uint16_t target_id{0};     // 00521EA0(argument 2)'s +174h, 0 when unresolved
    std::string command_name;       // argument 3->vtable[4]()
    std::string status;             // argument 4
};

// One parsed Lua `command` event block: the four filter containers 0097C8A0
// builds at +0Ch, +1Ch, +2Ch and +3Ch, plus the callback name at +4h. An empty
// container accepts everything.
struct CommandEventSubscription {
    std::string callback;
    std::vector<std::uint16_t> unit_ids;
    std::vector<std::uint16_t> target_ids;
    std::vector<std::string> command_names;
    std::vector<std::string> statuses;
};

// 00968AF0 BSP_EventSubscription_MatchFourParameters, vtable 00D1B770 slot +0Ch.
// A conjunction over the four containers.
bool command_event_matches(const CommandEventSubscription& subscription,
                           const CommandEventParameters& parameters) noexcept;

// 0097B8C0 BSP_WarningChannel_Evaluate over a whole channel: the callback names
// of every matching subscription, in list order.
std::vector<std::string> command_event_callbacks(
    const std::vector<CommandEventSubscription>& channel,
    const CommandEventParameters& parameters);

// ---------------------------------------------------------------------------
// Host: one virtual per native call site of the completion sequence
// ---------------------------------------------------------------------------
// No default implementations. The session, the mission event reporter, the Lua
// host and the command classes are contracts.
struct CommandCompletionHost {
    virtual ~CommandCompletionHost() = default;

    // 0071E440: command->vtable[0Ch](), the category.
    virtual int command_category(std::uint32_t command) = 0;

    // 0071E463: 0071D810 BSP_WeaponDirector_RaiseCommandStage,
    // __thiscall(controller)(int stage), RET 4.
    virtual void raise_queue_stage(int stage) = 0;

    // 0071E480: 0071D9E0 BSP_WeaponDirector_RaiseOverrideStage, same shape.
    virtual void raise_override_stage(int stage) = 0;

    // 0071D852 / 0071D961: 0071C730 then 0077C2A0 BSP_Session_RouteMessage
    // (msg, 7, 0). The message is originated only under
    // stage_raise_sends_message.
    virtual void route_clear_command_message(const ClearCommandMessage& message) = 0;

    // 00836D20: 0041E870 BSP_NativeString_Assign with the status literal.
    virtual void assign_status_text(const char* literal) = 0;

    // 00836D3E: 004F1830, build the command-target descriptor the report takes.
    virtual std::uint32_t make_command_target(std::uint32_t command) = 0;

    // 00836D51, 0071F79B, 0071F8C0, 009E595C, 009E5C35, 0084E431:
    // 00984300 BSP_MissionEvents_ReportCommand on [00F8A0C4],
    // __thiscall(reporter)(entity, target, class, status), RET 10h.
    virtual void report_command_event(std::uint32_t entity, std::uint32_t command_target,
                                      std::uint32_t command_class, const char* status) = 0;

    // 0071F7E8: 00984800 BSP_MissionEvents_ReportTarget, start only.
    virtual void report_target_event(std::uint32_t entity, std::uint32_t target) = 0;

    // 00836E13: 007788B0 BSP_Entity_ControllerBelongsToAnother.
    virtual bool controller_belongs_to_another() = 0;

    // 00836E28: 007788D0 BSP_Entity_ControllingEntity.
    virtual std::uint32_t controlling_entity() = 0;

    // 00836E92: 0071ECF0 BSP_WeaponDirector_IssueCommand(class, target).
    virtual void issue_command(std::uint32_t command_class, std::uint32_t target) = 0;

    // 00836E0B: this->vtable[6Ch](1) = 00835BF0, which calls 0071C130 first.
    virtual void on_command_changed(bool primary) = 0;
};

// ---------------------------------------------------------------------------
// The sequences
// ---------------------------------------------------------------------------

// What one 0071E430 call did.
struct EndCommandTrace {
    EndCommandArm arm{EndCommandArm::kNothing};
    bool read_category{false};
    int requested_stage{kCommandStageFresh};
    bool raised_queue_stage{false};
    bool raised_override_stage{false};
    QueueHeadRestart restart{};
};

// 0071E430 whole, over the controller state and the host.
EndCommandTrace run_end_command_0071e430(CommandQueueState& state, std::uint32_t command,
                                         bool terminal, CommandCompletionHost& host);

// The block every `finished` producer shares: assign the status text, build the
// command target, report the event, then end the command. 00836D12-00836D5C is
// the worked example (it raises the stage itself first, which is why
// `raise_stage_first` exists); 009E58EF-009E5997 is the same block without it.
struct FinishCommandTrace {
    bool raised_stage_first{false};
    bool reported{false};
    EndCommandTrace end{};
};

FinishCommandTrace run_finish_command(CommandQueueState& state, std::uint32_t entity,
                                      std::uint32_t command_class, bool raise_stage_first,
                                      CommandCompletionHost& host);

// LAB_00836DC9 whole. Returns the standing command it issued, or nothing when
// the tail did not run or did not issue.
struct IdleTailTrace {
    bool ran{false};
    bool issued{false};
    StandingCommand command{StandingCommand::kStop};
    std::uint32_t command_address{0};
    std::uint32_t target{0};
};

IdleTailTrace run_idle_tail_00836dc9(const CommandQueueState& state, int queued_count,
                                     bool no_command_flag, std::uint32_t unit,
                                     bool unit_player_driven, float commanded_speed,
                                     CommandCompletionHost& host);

}  // namespace bsp
