// Projection of the unit command controller's execution engine.
// Packet cc2_director_commands; docs/COMMAND_EXECUTION.md, docs/COMMAND_CLASSES.md.
//
// Coverage: 00720CD0, 00720CA0, 00720850, 0071E610, 0071E7F0, 008358D0 are
// complete. 00835C70 covers the arms outside the cruise latch, which is
// docs/CRUISE_COMMAND.md's contract (00835AC0) and is left to the host. The
// moveonpath path build in 0071F600 and the whole of 00836920 are contracts;
// the completion rules 00836A6C, 00836A8E and 00836ADC are projected as pure
// functions because their constants are complete.
#include "bsp/command_execution.hpp"

namespace bsp {
namespace {

// 00720850's empty-slot reinitialiser: command 0, both flags and the id clear,
// the position set to the 00F87574 default, trailing 0.
void reset_slot(CommandSlot& slot, CommandExecutionHost& host) {
    slot.command = 0;
    slot.params.has_target_entity = false;
    slot.params.has_position = false;
    slot.params.target_entity_id = 0;
    slot.params.target_entity = 0;
    host.default_command_position(slot.params.position_x, slot.params.position_y,
                                  slot.params.position_z);
    slot.params.trailing = 0;
}

// The snap both clears share: 00720850's `count < 2` arm and 0071E610. The
// record keeps the target's world position and forgets the entity.
void snap_to_target(CommandSlot& slot, std::uint32_t target, CommandExecutionHost& host) {
    host.refresh_target_pose(target);
    slot.params.has_target_entity = false;
    slot.params.has_position = true;
    slot.params.target_entity_id = 0;
    slot.params.target_entity = 0;
    host.read_target_position(target, slot.params.position_x, slot.params.position_y,
                              slot.params.position_z);
    slot.params.trailing = 0;
}

bool is_movement_only_command(std::uint32_t command) {
    return command == kCommandStop || command == kCommandCruise || command == kCommandLeave ||
           command == kCommandDisband;
}

}  // namespace

std::uint32_t command_queue_current_command(const CommandQueueState& state) noexcept {
    if (state.mode == CommandMode::kQueueHead) {
        return state.slots[0].command;
    }
    if (state.mode == CommandMode::kOverride) {
        return state.override_slot.command;
    }
    return 0;
}

int command_queue_count(const CommandQueueState& state,
                        const int (&path_point_counts)[kDirectorCommandSlotCount]) noexcept {
    int index = 0;
    while (index < kDirectorCommandSlotCount) {
        const std::uint32_t command = state.slots[index].command;
        if (command == 0) {
            return index;
        }
        int step = 1;
        if (command == kCommandMoveOnPath && path_point_counts[index] > 0) {
            step = path_point_counts[index];
        }
        index += step;
    }
    return index;
}

bool command_slot_has_active_order(const CommandQueueState& state, int index,
                                   int path_point_count) noexcept {
    if (index < 0 || index >= kDirectorCommandSlotCount) {
        return false;
    }
    const std::uint32_t command = state.slots[index].command;
    if (command == 0 || is_movement_only_command(command)) {
        return false;
    }
    if (command != kCommandMoveOnPath) {
        return true;
    }
    return state.path_objects[index] != 0 && path_point_count > 0;
}

bool command_arrival_reached(float unit_x, float unit_z, float target_x,
                             float target_z) noexcept {
    const float dx = target_x - unit_x;
    const float dz = target_z - unit_z;
    const float squared = dx * dx + dz * dz;
    return static_cast<double>(squared) < kCommandArrivalRadiusSquared;
}

bool command_arrival_gate_open(int queued_count, int queue_stage, int head_category,
                               int last_category) noexcept {
    if (queued_count <= 1 || queue_stage >= kCommandStageStarted) {
        return false;
    }
    if (head_category == 1 || head_category == 2) {
        return false;
    }
    return last_category == 1 || last_category == 2;
}

bool stop_command_complete(int queued_count, bool unit_flag_184h, float unit_speed) noexcept {
    if (queued_count > 1 || unit_flag_184h) {
        return true;
    }
    return !(unit_speed < kStopCompletionSpeed);
}

bool follow_command_continues(bool unit_in_group, std::uint32_t group_leader, std::uint32_t unit,
                              std::uint32_t command_target, int queued_count) noexcept {
    if (!unit_in_group || group_leader == 0 || group_leader == unit) {
        return false;
    }
    if (command_target != group_leader) {
        return false;
    }
    return queued_count < 2;
}

void clear_command_slot_00720850(CommandQueueState& state, int index,
                                 CommandExecutionHost& host) {
    if (index < 0 || index >= kDirectorCommandSlotCount) {
        return;
    }

    // 00720880..007208B1: the trace, with "EmptyCommand" for a null slot.
    const std::uint32_t slot_command = state.slots[index].command;
    const char* name = slot_command == 0 ? "EmptyCommand" : host.command_name(slot_command);
    host.trace_clear_primary_command(name, index, static_cast<int>(state.mode),
                                     host.session_trace_value());

    // 007208C0..007208E5: lift an idle mode, or give up when nothing is queued.
    if (state.mode == CommandMode::kIdle) {
        if (state.override_slot.command != 0) {
            state.mode = CommandMode::kOverride;
        } else if (state.slots[0].command != 0) {
            state.mode = CommandMode::kQueueHead;
        } else {
            return;
        }
    }

    if (index != 0) {
        // A queued command, not the current one: drop it and close the gap.
        if (state.path_objects[index] != 0) {
            host.destroy_path_object(state.path_objects[index]);
            state.path_objects[index] = 0;
        }
        for (int i = index; i + 1 < kDirectorCommandSlotCount; ++i) {
            state.slots[i] = state.slots[i + 1];
            state.path_objects[i] = state.path_objects[i + 1];
        }
        reset_slot(state.slots[kDirectorCommandSlotCount - 1], host);
        state.path_objects[kDirectorCommandSlotCount - 1] = host.create_path_object();
        return;
    }

    // 00720888 onwards: the current command completes or is replaced.
    if (state.mode == CommandMode::kOverride) {
        clear_override_command_0071e610(state, host);
    }
    state.mode = CommandMode::kQueueHead;

    int occupied = 0;
    while (occupied < kDirectorCommandSlotCount && state.slots[occupied].command != 0) {
        ++occupied;
    }

    const std::uint32_t target = host.resolve_target(state.slots[0].params);
    if (target != 0) {
        host.unregister_target_observer(target);
    }

    // 00720936..: the slot-0 record becomes the previous-command record.
    state.previous = state.slots[0];

    if (occupied < 2) {
        if (target != 0) {
            snap_to_target(state.slots[0], target, host);
        }
        state.slots[0].command = 0;
        state.mode = CommandMode::kIdle;
    } else {
        for (int i = 0; i + 1 < occupied; ++i) {
            state.slots[i] = state.slots[i + 1];
        }
        reset_slot(state.slots[occupied - 1], host);
    }

    if (occupied > 0) {
        if (state.path_objects[0] != 0) {
            host.destroy_path_object(state.path_objects[0]);
            state.path_objects[0] = 0;
        }
        for (int i = 0; i + 1 < kDirectorCommandSlotCount; ++i) {
            state.path_objects[i] = state.path_objects[i + 1];
        }
        state.path_objects[kDirectorCommandSlotCount - 1] = host.create_path_object();
    }

    host.on_command_changed(true);  // 00720B56, vtable[6Ch](1)
}

void clear_all_command_slots_00720ca0(CommandQueueState& state, CommandExecutionHost& host) {
    for (int index = kDirectorCommandSlotCount - 1; index >= 0; --index) {
        if (state.slots[index].command != 0) {
            clear_command_slot_00720850(state, index, host);
        }
    }
}

bool issue_target_command_00720cd0(CommandQueueState& state, std::uint32_t entity,
                                   std::uint16_t entity_id, CommandExecutionHost& host) {
    clear_all_command_slots_00720ca0(state, host);

    CommandParams params{};
    params.has_target_entity = entity != 0;
    params.has_position = false;  // 00720D2A zeroes the whole word first
    params.target_entity_id = entity != 0 ? entity_id : static_cast<std::uint16_t>(0);
    params.target_entity = entity;
    host.default_command_position(params.position_x, params.position_y, params.position_z);
    params.trailing = 0;

    return host.set_command(kCommandFollow, params);  // 00720D61 pushes 00E08F60
}

void clear_override_command_0071e610(CommandQueueState& state, CommandExecutionHost& host) {
    const bool engaged = state.mode == CommandMode::kOverride ||
                         state.override_slot.params.has_target_entity ||
                         state.override_slot.command != 0;
    if (!engaged) {
        return;
    }
    state.mode = CommandMode::kQueueHead;
    const std::uint32_t target = host.resolve_target(state.override_slot.params);
    if (target != 0) {
        snap_to_target(state.override_slot, target, host);
        host.unregister_target_observer(target);
    }
    state.override_slot.command = 0;
    host.on_command_changed(false);  // 0071E6B5, vtable[6Ch](0)
}

void set_override_command_0071e7f0(CommandQueueState& state, std::uint32_t command,
                                   const CommandParams& params, CommandExecutionHost& host) {
    // 0071E7FF..0071E814: the same command with the same target is a no-op.
    if (state.mode == CommandMode::kOverride && state.override_slot.command == command &&
        host.resolve_target(state.override_slot.params) == host.resolve_target(params)) {
        return;
    }
    if (!host.command_accepts_target(command, params)) {
        return;
    }

    // 0071E837: an attackmove head only tolerates an override on the same object.
    const std::uint32_t head = state.slots[0].command;
    if (head == kCommandAttackMove) {
        if (host.resolve_target(params) != host.resolve_target(state.slots[0].params)) {
            return;
        }
    } else if (head != 0) {
        const int category = host.command_category(head);
        if (category == 1 || category == 2) {
            return;
        }
    }

    const std::uint32_t old_target = host.resolve_target(state.override_slot.params);
    if (old_target != 0) {
        host.unregister_target_observer(old_target);
    }

    state.override_slot.command = command;
    state.override_slot.params = params;

    const std::uint32_t new_target = host.resolve_target(params);
    if (new_target != 0) {
        host.register_target_observer(new_target);
    }

    state.mode = CommandMode::kOverride;
    state.override_accepted = false;                // 0071E907, byte +4Ch
    state.override_stage = kCommandStageFresh;      // dword +50h
    host.on_command_changed(false);                 // 0071E90E, vtable[6Ch](0)
}

bool set_command_008358d0(std::uint32_t command, const CommandParams& params,
                          CommandExecutionHost& host) {
    if (!host.push_command_slot(command, params)) {  // 008358DF
        return false;
    }
    const int category = host.command_category(command);  // 008358F9
    if (category == 1 || category == 2) {
        const std::uint32_t target = host.resolve_target(params);  // 00835907
        const int mode = host.session_mode();
        if (target != 0 && (mode == 0 || mode == 1)) {
            host.set_fire_target(target, true);  // 00835930
        }
    }
    return true;
}

bool begin_current_command_00835c70(CommandQueueState& state, bool use_queue_head,
                                    CommandExecutionHost& host) {
    const std::uint32_t command =
        use_queue_head ? state.slots[0].command : state.override_slot.command;
    const CommandParams& params = use_queue_head ? state.slots[0].params
                                                 : state.override_slot.params;

    // 00835C81..00835CF9: invalidate the first two path objects.
    if (state.path_objects[0] != 0) {
        host.invalidate_path_object(state.path_objects[0]);
    }
    if (state.path_objects[1] != 0) {
        host.invalidate_path_object(state.path_objects[1]);
    }

    // 00835D0A: a unit flagged at +184h refuses cruise and stop outright.
    if (host.unit_flag_184h() && (command == kCommandCruise || command == kCommandStop)) {
        return true;
    }

    if (!host.begin_command_base(use_queue_head)) {  // 00835D33
        return false;
    }

    bool accepted = true;
    if (host.session_mode() != 2) {
        if (command == kCommandFollow) {
            // 00835D5C: without a shared group, the answer is the ownership test.
            const std::uint32_t target = host.resolve_target(params);
            if (target == 0 || host.group_leader() == 0) {
                accepted = host.controller_belongs_to_another();
            }
        } else if (command == kCommandCruise || command == kCommandStop) {
            host.raise_queue_stage(kCommandStageStarted);  // 00835E12
            // 00835E58: 00835AC0, the cruise latch, is docs/CRUISE_COMMAND.md's.
        } else if (command == kCommandAttackMove) {
            const std::uint32_t fire_target = host.fire_target();
            if (host.attack_move_gate() || fire_target == 0) {  // 00835DC2
                const std::uint32_t target = host.resolve_target(params);
                if (fire_target != target) {
                    host.set_fire_target(target, use_queue_head);  // 00835E07
                }
            }
        }
    }

    if (command != 0) {
        if (use_queue_head) {
            state.queue_accepted = accepted;
        } else {
            state.override_accepted = accepted;
        }
    }
    return accepted;
}

}  // namespace bsp
