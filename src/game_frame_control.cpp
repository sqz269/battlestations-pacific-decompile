#include "bsp/game_frame_control.hpp"

#include <utility>

namespace bsp {
namespace {
// Native request ids, as raw values so the switch stays a transcription of the
// compare chain at 004e44ea..004e4854.
constexpr std::uint32_t kRequestFrontEndRestore = 0x02u;
constexpr std::uint32_t kRequestFrontEnd = 0x04u;
constexpr std::uint32_t kRequestNoAction05 = 0x05u;
constexpr std::uint32_t kRequestNotifyE198ac = 0x06u;
constexpr std::uint32_t kRequestReset07 = 0x07u;
constexpr std::uint32_t kRequestNoAction08 = 0x08u;
constexpr std::uint32_t kRequestNotifyE198b4 = 0x09u;
constexpr std::uint32_t kRequestScenarioLoad0A = 0x0Au;
constexpr std::uint32_t kRequestScenarioLoad0B = 0x0Bu;
constexpr std::uint32_t kRequestNoAction0C = 0x0Cu;
constexpr std::uint32_t kRequestInMission = 0x0Du;
constexpr std::uint32_t kRequestRebind0E = 0x0Eu;
constexpr std::uint32_t kRequestDebrief = 0x0Fu;
constexpr std::uint32_t kRequestTeardown = 0x10u;
constexpr std::uint32_t kRequestMissionEndWait = 0x11u;
constexpr std::uint32_t kRequestResume = 0x12u;
constexpr std::uint32_t kRequestQuit = 0x13u;
constexpr std::uint32_t kRequestRestart = 0x14u;
constexpr std::uint32_t kRequestNoAction15 = 0x15u;
constexpr std::uint32_t kRequestNotifyE198b8 = 0x16u;
} // namespace

std::size_t state_request_block_index(const GameStateRequestQueue& queue,
    std::size_t offset) noexcept {
    std::size_t block = offset / kStateRequestsPerBlock;
    const std::size_t block_count = queue.blocks.size();
    if (block_count <= block) {
        block -= block_count;
    }
    return block;
}

std::uint32_t front_state_request(const GameStateRequestQueue& queue) noexcept {
    if (queue.count == 0 || queue.blocks.empty()) {
        return 0u;
    }
    const std::size_t block = state_request_block_index(queue, queue.head_offset);
    const GameStateRequestBlock* entries = queue.blocks[block].get();
    if (entries == nullptr) {
        return 0u;
    }
    return entries->entries[queue.head_offset % kStateRequestsPerBlock];
}

void pop_front_state_request(GameStateRequestQueue& queue) noexcept {
    // 004e44a4: the whole body is skipped when the queue is already empty.
    if (queue.count == 0) {
        return;
    }
    queue.head_offset += 1;
    if (queue.blocks.size() * kStateRequestsPerBlock <= queue.head_offset) {
        queue.head_offset = 0;
    }
    queue.count -= 1;
    if (queue.count == 0) {
        queue.head_offset = 0;
    }
}

void grow_state_request_map_004d20d0(GameStateRequestQueue& queue, std::size_t count) {
    std::size_t increment = queue.blocks.size() / 2;
    if (increment < 8) {
        increment = 8;
    }
    if (count < increment) {
        count = increment;
    }

    const std::size_t old_count = queue.blocks.size();
    const std::size_t head_block = old_count == 0
        ? 0
        : queue.head_offset / kStateRequestsPerBlock;
    const std::size_t new_count = old_count + count;

    // 004d2113..004d2201 moves the blocks so that the sequence starting at the
    // head block stays contiguous in the enlarged ring and the fresh slots land
    // behind it. Both native branches produce this rotation.
    std::vector<std::unique_ptr<GameStateRequestBlock>> grown(new_count);
    for (std::size_t i = 0; i < old_count; ++i) {
        const std::size_t from = (head_block + i) % old_count;
        const std::size_t to = (head_block + i) % new_count;
        grown[to] = std::move(queue.blocks[from]);
    }
    queue.blocks = std::move(grown);
    // 004d2211 adds to the block count only; the head offset is left alone.
}

void enqueue_state_request_004d3ed0(GameStateRequestQueue& queue, std::uint32_t request) {
    // 004d3ed3: grow when the write would open a new block and the map is full.
    if ((queue.head_offset + queue.count) % kStateRequestsPerBlock == 0) {
        const std::size_t needed = (queue.count + kStateRequestsPerBlock) / kStateRequestsPerBlock;
        if (queue.blocks.size() <= needed) {
            grow_state_request_map_004d20d0(queue, 1);
        }
    }

    // 004d3ef4 re-reads the head and the count after the possible growth.
    const std::size_t offset = queue.head_offset + queue.count;
    const std::size_t block = state_request_block_index(queue, offset);
    if (queue.blocks[block] == nullptr) {
        queue.blocks[block] = std::unique_ptr<GameStateRequestBlock>(new GameStateRequestBlock());
    }
    queue.blocks[block]->entries[offset % kStateRequestsPerBlock] = request;
    queue.count += 1;
}

int count_active_local_players_004c6e50(
    const LocalPlayerSlot (&slots)[kLocalPlayerSlotCount]) noexcept {
    int active = 0;
    for (std::size_t i = 0; i < kLocalPlayerSlotCount; ++i) {
        const LocalPlayerSlot& slot = slots[i];
        if (slot.present && slot.joined && !slot.inactive) {
            active += 1;
        }
    }
    return active;
}

FrameDeltaScaleResult scale_frame_delta_004c6e30(const FrameDeltaScaleInputs& inputs) noexcept {
    FrameDeltaScaleResult result{};
    float delta = inputs.raw_delta;

    // 004c6e38: mode zero skips the mission filter and the coarse clamp and
    // drops straight into the single-view path at 004c6f3d.
    bool split_view = false;
    const bool multi_view = inputs.local_player_mode != 0;
    if (multi_view) {
        split_view = inputs.local_player_mode == 2
            ? true
            : inputs.active_local_players > 1;

        if (inputs.game_state == static_cast<std::uint32_t>(GameStateId::kInMission)) {
            delta = inputs.mission_filtered_delta;
        }
        if (delta > kFrameDeltaMultiplayerMaxStep && !inputs.max_step_clamp_disabled) {
            delta = kFrameDeltaMultiplayerMaxStep;
        }
    }

    if (multi_view && split_view) {
        // 004c6f09. The scale currently ships as 1.0f, and none of the debug
        // time controls or the cinematic override reach this path.
        const float scaled = delta * kFrameDeltaSplitScreenScale;
        result.delta = scaled;
        result.step = scaled;
        result.elapsed_increment = scaled;
        result.scaled = scaled;
        return result;
    }

    // 004c6f3d. The threshold cell 00d7a270 is the double promotion of the
    // replacement value in 00ce7638, and the compare runs in double because the
    // float is loaded onto the x87 stack.
    if (static_cast<double>(delta) > kFrameDeltaSingleStepThreshold) {
        delta = kFrameDeltaSingleMaxStep;
    }

    result.delta = delta;
    result.step = delta;
    result.elapsed_increment = delta;

    // 004c6f85..004c700d. Three held actions, first match wins; the multipliers
    // are 64-bit so the product is formed in double and rounded on the store.
    float scaled = delta;
    if (inputs.fast_forward_held) {
        scaled = static_cast<float>(static_cast<double>(delta) * kFrameDeltaFastForwardScale);
    } else if (inputs.turbo_held) {
        scaled = static_cast<float>(static_cast<double>(delta) * kFrameDeltaTurboScale);
    } else if (inputs.slow_motion_held) {
        scaled = static_cast<float>(static_cast<double>(delta) * kFrameDeltaSlowMotionScale);
    }

    // 004c7013. A cinematic frame stops the simulation clock outright and the
    // step action releases exactly one fixed 1/30 s tick.
    if (inputs.cinematic) {
        scaled = 0.0f;
        if (inputs.cinematic_step_pressed) {
            scaled = kFrameDeltaCinematicStep;
        }
    }

    result.scaled = scaled;
    return result;
}

void drain_state_requests_004e4430(GameFrameControlState& state, GameFrameControlHost& host) {
    // 004e4436: an empty queue skips straight to the tail, which still runs.
    while (state.requests.count != 0) {
        state.state = front_state_request(state.requests);
        pop_front_state_request(state.requests);

        bool stop_draining = false;
        switch (state.state) {
        case kRequestQuit:
            // 004e48ca. Sets the application exit byte and returns without the
            // tail call.
            state.application_exit_requested = true;
            return;
        case kRequestFrontEnd:
            host.request_04_004e4000();
            break;
        case kRequestNoAction05:
        case kRequestNoAction08:
        case kRequestNoAction0C:
        case kRequestNoAction15:
            break;
        case kRequestNotifyE198ac:
            host.request_06_notify_00e198ac();
            break;
        case kRequestReset07:
            host.request_07_004bfc70();
            break;
        case kRequestNotifyE198b4:
            host.request_09_notify_00e198b4();
            break;
        case kRequestScenarioLoad0A:
        case kRequestScenarioLoad0B:
            host.request_0a_0b_004dfb70();
            break;
        case kRequestInMission:
            // 004e47ec. Leaves the loop with whatever is still queued.
            stop_draining = true;
            break;
        case kRequestRebind0E:
            host.request_0e_004c6b00();
            break;
        case kRequestDebrief:
            host.request_0f_004d7970();
            break;
        case kRequestTeardown:
            if (host.request_10_teardown_004e458a(state)) {
                return; // 004e48d7, one sub-path returns without the tail.
            }
            break;
        case kRequestResume:
            // 004e47f7 writes the in-mission state before running the arm.
            state.state = static_cast<std::uint32_t>(GameStateId::kInMission);
            host.request_12_resume_004cd0f0();
            break;
        case kRequestRestart:
            host.request_14_004bac20();
            break;
        case kRequestNotifyE198b8:
            host.request_16_notify_00e198b8();
            break;
        case kRequestFrontEndRestore:
            host.request_02_004d8000();
            break;
        default:
            break;
        }

        if (stop_draining) {
            break;
        }
    }

    // 004e4873. The tail runs on the state the loop left behind.
    if (state.state == static_cast<std::uint32_t>(GameStateId::kInMission)) {
        if (host.input_action_pressed(kMissionStepAction)) {
            host.request_0f_004d7970();
        }
    } else if (state.state == static_cast<std::uint32_t>(GameStateId::kMissionTeardown)) {
        state.state = host.network_session_active() ? kRequestNoAction08 : kRequestNoAction05;
    }

    host.post_drain_00a95960(state.scaled_delta);
}

bool check_mission_completion_004d7ea0(GameFrameControlState& state, GameFrameControlHost& host) {
    // 004d7ea4: a pending request wins; the poll waits for a clear queue.
    if (state.requests.count != 0) {
        return false;
    }

    state.mission_result = host.mission_result();
    if (!state.mission_result.present) {
        return false;
    }

    // 004d7ec5..004d7f14. Three zero-delta world updates with two post passes
    // between them, so the world settles without advancing mission time.
    host.world_final_tick(0.0f);
    host.world_post_tick();
    host.world_final_tick(0.0f);
    host.world_post_tick();
    host.world_final_tick(0.0f);

    host.show_mission_result_gui(state.mission_result.score);

    bool enqueued = false;
    if (state.mission_result.requests_debrief) {
        enqueue_state_request_004d3ed0(state.requests,
            static_cast<std::uint32_t>(GameStateId::kMissionDebrief));
        state.drain_suspended = true;
        enqueued = true;
    }

    host.close_mission_result();
    return enqueued;
}

void update_mission_end_wait(GameFrameControlState& state, GameFrameControlHost& host) {
    if (state.state != static_cast<std::uint32_t>(GameStateId::kMissionEndWait)) {
        return;
    }

    // 004e505b and 004e506b are both on the fall-through path, so the flag is
    // raised on entry and lowered again unless the suspend byte is set.
    state.drain_suspended = true;
    if (state.mission_end_suspended) {
        return;
    }
    state.drain_suspended = false;

    if (state.requests.count != 0) {
        return;
    }
    enqueue_state_request_004d3ed0(state.requests,
        static_cast<std::uint32_t>(GameStateId::kFrontEnd));
    host.set_front_end_pending_flag(!state.mission_flag_1ee1);
}

void run_game_frame_control(GameFrameControlState& state, FrameClock& clock,
    GameFrameControlHost& host) {
    // 004e4a5e, the first call of the frame.
    host.pre_tick_console_commands();

    // 004e4d02. The suspension latch defers the whole drain to a later frame.
    if (!state.drain_suspended) {
        drain_state_requests_004e4430(state, host);
    }

    // 004e4d45. Inputs are sampled in the order 004c6e30 reads them, so a host
    // that counts calls sees the native sequence.
    FrameDeltaScaleInputs inputs{};
    inputs.raw_delta = state.raw_delta;
    inputs.local_player_mode = state.local_player_mode;
    inputs.game_state = state.state;
    inputs.cinematic = state.cinematic;
    inputs.mission_filtered_delta = state.raw_delta;

    const bool multi_view = state.local_player_mode != 0;
    bool split_view = false;
    if (multi_view) {
        if (state.local_player_mode == 2) {
            split_view = true;
        } else {
            LocalPlayerSlot slots[kLocalPlayerSlotCount]{};
            host.copy_local_player_slots(slots);
            inputs.active_local_players = count_active_local_players_004c6e50(slots);
            split_view = inputs.active_local_players > 1;
        }
        if (state.state == static_cast<std::uint32_t>(GameStateId::kInMission)) {
            inputs.mission_filtered_delta = host.mission_time_filter_007713a0(state.raw_delta);
        }
        inputs.max_step_clamp_disabled = host.max_step_clamp_disabled();
    }
    if (!multi_view || !split_view) {
        inputs.fast_forward_held = host.input_action_held(kFastForwardAction);
        if (!inputs.fast_forward_held) {
            inputs.turbo_held = host.input_action_held(kTurboAction);
            if (!inputs.turbo_held) {
                inputs.slow_motion_held = host.input_action_held(kSlowMotionAction);
            }
        }
        if (state.cinematic) {
            inputs.cinematic_step_pressed = host.input_action_pressed(kCinematicStepAction);
        }
    }

    const FrameDeltaScaleResult scaled = scale_frame_delta_004c6e30(inputs);
    state.raw_delta = scaled.delta;
    state.step = scaled.step;
    state.elapsed += scaled.elapsed_increment;
    state.scaled_delta = scaled.scaled;

    // 004e4d4a. The frame clock accumulates the dilated value while game+64Ch
    // above accumulated the undilated one.
    clock.accumulated += state.scaled_delta;

    // 004e4d6d, with the delta slot 004c6e30 just rewrote.
    host.update_cutscene_playback_004c6b20(state.raw_delta);
    if (state.state == static_cast<std::uint32_t>(GameStateId::kInMission)) {
        host.mission_hud_update(state.scaled_delta);
    }

    // 004e4de8 and 004e4ded, both unconditional.
    host.accumulate_frame_statistics_0053c510();
    host.update_presence_context_004c0170();

    // 004e503b. Other packets own the frame counter and the network tick that
    // sit between here and the state tests.
    if (state.state == kRequestNoAction0C) {
        host.update_device_wait_screen_004db920();
    }
    update_mission_end_wait(state, host);

    // 004e5386, inside the world tick the simulation gate owns.
    if (state.state == static_cast<std::uint32_t>(GameStateId::kInMission)) {
        check_mission_completion_004d7ea0(state, host);
    }
}
}
