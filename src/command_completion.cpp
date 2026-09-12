// Packet cc2_command_completion. docs/COMMAND_COMPLETION.md carries the
// evidence; the comments here name the site each rule came from.
#include "bsp/command_completion.hpp"

#include <algorithm>

namespace bsp {
namespace {

// 0071BE60, and the identical inline loop at 0071E489-0071E49E: count leading
// occupied slots, stopping at the first empty one, at most ten.
int occupied_slot_count(const CommandQueueState& state) noexcept {
    int count = 0;
    for (std::size_t i = 0; i < kDirectorCommandSlotCount; ++i) {
        if (state.slots[i].command == 0) {
            break;
        }
        ++count;
    }
    return count;
}

// 0071E43D-0071E44A. Only categories 1 and 2 reach the mode test; the base
// answers 0 and follow answers 3 (docs/COMMAND_CLASSES.md).
constexpr bool category_defers_to_mode(int category) noexcept {
    return category == 1 || category == 2;
}

}  // namespace

// ---------------------------------------------------------------------------
// 0071E430
// ---------------------------------------------------------------------------

EndCommandDecision end_command_decision(bool has_command, int command_category, CommandMode mode,
                                        int occupied_slots, bool terminal) noexcept {
    EndCommandDecision decision;
    const int stage = end_command_requested_stage(terminal);

    // 0071E437 TEST ECX,ECX / JZ 0071E44C: a null command skips the category
    // call and falls into the mode test.
    if (has_command && !category_defers_to_mode(command_category)) {
        // 0071E44A JNZ 0071E454.
        decision.arm = EndCommandArm::kQueueStageByCategory;
        decision.stage = stage;
        return decision;
    }

    // 0071E44C MOV EAX,[ESI+30h].
    if (mode == CommandMode::kQueueHead) {
        decision.arm = EndCommandArm::kQueueStageByMode;
        decision.stage = stage;
        return decision;
    }
    if (mode == CommandMode::kOverride) {
        decision.arm = EndCommandArm::kOverrideStageByMode;
        decision.stage = stage;
        return decision;
    }

    // 0071E489: neither mode. 0071E4A0 TEST EAX,EAX / JLE skips the writes.
    decision.arm = occupied_slots > 0 ? EndCommandArm::kRestartQueueHead : EndCommandArm::kNothing;
    return decision;
}

QueueHeadRestart end_command_restart(const EndCommandDecision& decision) noexcept {
    QueueHeadRestart restart;
    // 0071E4A4 MOV [ESI+30h],1 / 0071E4AB MOV byte [ESI+44h],0 /
    // 0071E4AF MOV [ESI+48h],0.
    restart.applies = decision.arm == EndCommandArm::kRestartQueueHead;
    restart.mode = CommandMode::kQueueHead;
    restart.queue_accepted = false;
    restart.queue_stage = kCommandStageFresh;
    return restart;
}

EndCommandTrace run_end_command_0071e430(CommandQueueState& state, std::uint32_t command,
                                         bool terminal, CommandCompletionHost& host) {
    EndCommandTrace trace;

    int category = 0;
    const bool has_command = command != 0;
    if (has_command) {
        // 0071E440 CALL EDX, the vtable[0Ch] category getter.
        category = host.command_category(command);
        trace.read_category = true;
    }

    const EndCommandDecision decision = end_command_decision(
        has_command, category, state.mode, occupied_slot_count(state), terminal);
    trace.arm = decision.arm;
    trace.requested_stage = decision.stage;

    switch (decision.arm) {
    case EndCommandArm::kQueueStageByCategory:
    case EndCommandArm::kQueueStageByMode:
        // 0071E463 CALL 0071D810.
        host.raise_queue_stage(decision.stage);
        trace.raised_queue_stage = true;
        break;
    case EndCommandArm::kOverrideStageByMode:
        // 0071E480 CALL 0071D9E0.
        host.raise_override_stage(decision.stage);
        trace.raised_override_stage = true;
        break;
    case EndCommandArm::kRestartQueueHead: {
        const QueueHeadRestart restart = end_command_restart(decision);
        state.mode = restart.mode;
        state.queue_accepted = restart.queue_accepted;
        state.queue_stage = restart.queue_stage;
        trace.restart = restart;
        break;
    }
    case EndCommandArm::kNothing:
        break;
    }

    return trace;
}

// ---------------------------------------------------------------------------
// The stage ladder
// ---------------------------------------------------------------------------

StageResetResult reset_command_stage_0071c130(CommandQueueState& state, bool primary) noexcept {
    StageResetResult result;
    if (primary) {
        // 0071C138 MOV byte [ECX+44h],AL / 0071C13B MOV [ECX+48h],EAX, EAX = 0.
        state.queue_accepted = false;
        state.queue_stage = kCommandStageFresh;
        result.cleared_queue_pair = true;
        return result;
    }
    // 0071C141 MOV byte [ECX+4Ch],AL / 0071C144 MOV [ECX+50h],EAX.
    state.override_accepted = false;
    state.override_stage = kCommandStageFresh;
    result.cleared_override_pair = true;
    return result;
}

// ---------------------------------------------------------------------------
// MT_GAMEUNIT_CLEARCMD
// ---------------------------------------------------------------------------

ClearCommandMessage clear_command_message_for_queue_stage_done() noexcept {
    // 0071D852 CALL 0071C730 with (1, 0).
    ClearCommandMessage message;
    message.arm = 1;
    message.index = 0;
    return message;
}

ClearCommandMessage clear_command_message_for_override_stage_done() noexcept {
    // 0071D9E0's twin: 0071C730(0, 0).
    ClearCommandMessage message;
    message.arm = 0;
    message.index = 0;
    return message;
}

ClearCommandMessage clear_command_message_for_slot(int slot_index) noexcept {
    // 0071D900: byte [ESP+..] = 1 at +20h, the argument at +24h.
    ClearCommandMessage message;
    message.arm = 1;
    message.index = slot_index;
    return message;
}

ClearCommandMessage clear_command_message_for_all_slots() noexcept {
    // 0071D880: +20h = 1, +24h = 0FFFFFFFFh.
    ClearCommandMessage message;
    message.arm = 1;
    message.index = kClearCommandAllSlots;
    return message;
}

ClearCommandAction clear_command_action(const ClearCommandMessage& message) noexcept {
    // 00721B99 CMP byte [ESI+20h],0 / 00721B9F JZ 00721BC5.
    if (message.arm == 0) {
        return ClearCommandAction::kClearOverride;
    }
    // 00721BA1 MOV ESI,[ESI+24h] / TEST ESI,ESI / 00721BA6 JGE 00721BB6.
    if (message.index < 0) {
        return ClearCommandAction::kClearAllSlots;
    }
    return ClearCommandAction::kClearSlot;
}

// ---------------------------------------------------------------------------
// The idle tail
// ---------------------------------------------------------------------------

StandingCommand standing_command_after_queue_empty(bool controller_belongs_to_another,
                                                   bool unit_player_driven,
                                                   float commanded_speed) noexcept {
    // 00836E13 CALL 007788B0, tested first.
    if (controller_belongs_to_another) {
        return StandingCommand::kFollow;
    }
    // 00836E45 CMP byte [EAX+184h],0 / JNZ 00836E7A takes cruise directly.
    // 00836E59 COMISS XMM0,[00D7A218] = 0.0f / 00836E60 JNC 00836E7A: CF is set
    // both when the speed is below zero and when the compare is unordered, so a
    // NaN speed reaches stop, which is why this is `>=` and not `!(< 0)`.
    if (unit_player_driven || commanded_speed >= 0.0f) {
        return StandingCommand::kCruise;
    }
    return StandingCommand::kStop;
}

std::uint32_t standing_command_address(StandingCommand which) noexcept {
    switch (which) {
    case StandingCommand::kFollow:
        return kCommandFollow;
    case StandingCommand::kCruise:
        return kCommandCruise;
    case StandingCommand::kStop:
        return kCommandStop;
    }
    return 0;
}

IdleTailTrace run_idle_tail_00836dc9(const CommandQueueState& state, int queued_count,
                                     bool no_command_flag, std::uint32_t unit,
                                     bool unit_player_driven, float commanded_speed,
                                     CommandCompletionHost& host) {
    IdleTailTrace trace;
    if (!idle_tail_runs(state.queue_stage, queued_count, no_command_flag)) {
        return trace;
    }
    trace.ran = true;
    if (!idle_tail_issues(state.queue_stage, queued_count)) {
        return trace;
    }

    // 00836E0B: vtable[6Ch](1) = 00835BF0, which expires the commanded speed
    // the branch below then reads.
    host.on_command_changed(true);

    const bool belongs_to_another = host.controller_belongs_to_another();
    const StandingCommand which =
        standing_command_after_queue_empty(belongs_to_another, unit_player_driven, commanded_speed);

    // 00836E28 CALL 007788D0 only on the follow branch.
    trace.target = which == StandingCommand::kFollow ? host.controlling_entity() : unit;
    trace.command = which;
    trace.command_address = standing_command_address(which);
    trace.issued = true;

    // 00836E92 CALL 0071ECF0.
    host.issue_command(trace.command_address, trace.target);
    return trace;
}

// ---------------------------------------------------------------------------
// The `command` mission-event channel
// ---------------------------------------------------------------------------

namespace {

template <typename T>
bool filter_accepts(const std::vector<T>& filter, const T& value) {
    // 0097C8A0 builds every container empty; an empty container cannot reject,
    // so it is the accept-all case.
    if (filter.empty()) {
        return true;
    }
    return std::find(filter.begin(), filter.end(), value) != filter.end();
}

}  // namespace

bool command_event_matches(const CommandEventSubscription& subscription,
                           const CommandEventParameters& parameters) noexcept {
    // 00968AF0: four filter->vtable[0] calls on the containers at +0Ch, +1Ch,
    // +2Ch and +3Ch against params[0..3], short-circuiting on the first false.
    return filter_accepts(subscription.unit_ids, parameters.unit_id) &&
           filter_accepts(subscription.target_ids, parameters.target_id) &&
           filter_accepts(subscription.command_names, parameters.command_name) &&
           filter_accepts(subscription.statuses, parameters.status);
}

std::vector<std::string> command_event_callbacks(
    const std::vector<CommandEventSubscription>& channel,
    const CommandEventParameters& parameters) {
    // 0097B8C0 walks the std::list in order and appends subscription+4h on a
    // true answer.
    std::vector<std::string> callbacks;
    for (const CommandEventSubscription& subscription : channel) {
        if (command_event_matches(subscription, parameters)) {
            callbacks.push_back(subscription.callback);
        }
    }
    return callbacks;
}

// ---------------------------------------------------------------------------
// The shared finish block
// ---------------------------------------------------------------------------

FinishCommandTrace run_finish_command(CommandQueueState& state, std::uint32_t entity,
                                      std::uint32_t command_class, bool raise_stage_first,
                                      CommandCompletionHost& host) {
    FinishCommandTrace trace;

    if (raise_stage_first) {
        // 00836D12 CALL 0071D810 with 2, ahead of the report.
        host.raise_queue_stage(kCommandStageDone);
        trace.raised_stage_first = true;
    }

    // 00836D20 CALL 0041E870 with 00D09FD8.
    host.assign_status_text(kCommandEventStatusFinished);
    // 00836D3E CALL 004F1830.
    const std::uint32_t command_target = host.make_command_target(command_class);
    // 00836D51 CALL 00984300 on [00F8A0C4].
    host.report_command_event(entity, command_target, command_class, kCommandEventStatusFinished);
    trace.reported = true;

    // The state steps then call 0071E430 with terminal = 1; the director step's
    // moveonpath arm has already raised the stage and does not.
    if (!raise_stage_first) {
        trace.end = run_end_command_0071e430(state, command_class, true, host);
    }
    return trace;
}

}  // namespace bsp
