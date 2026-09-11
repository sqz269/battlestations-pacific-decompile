#include "bsp/mission_state_entry.hpp"

#include "bsp/game_settings.hpp"

namespace bsp {

bool mission_entry_uses_player_count_arm(
    std::int32_t local_view_mode, bool session_flag_29c) noexcept
{
    // 004db93c: CMP [EBP+1FE4h],0 / JZ takes the device arm.
    // 004db94c: CMP byte [EDI+29Ch],0 / JZ takes the player-count arm.
    return local_view_mode != 0 && !session_flag_29c;
}

bool device_wait_button_edge(DeviceWaitLatch& latch, bool any_button_down) noexcept
{
    // 004db962..004db977: the edge is computed from the stored byte, then the
    // stored byte is overwritten with the new sample on every path.
    const bool edge = !latch.any_button_down && any_button_down;
    latch.any_button_down = any_button_down;
    return edge;
}

bool device_wait_screens_idle(bool menu_screen_busy, bool gui_root_busy) noexcept
{
    return !menu_screen_busy && !gui_root_busy;
}

std::size_t count_unbound_player_slots(
    const MissionEntryPlayerSlot (&slots)[kLocalPlayerSlotCount]) noexcept
{
    // 004db9e0..004dba2d, two passes of four with stride 4.
    std::size_t count = 0;
    for (const MissionEntryPlayerSlot& slot : slots) {
        if (!slot.excluded && !slot.device_bound) {
            ++count;
        }
    }
    return count;
}

bool player_count_arm_may_enter(
    std::size_t unbound_slots, std::int32_t game_field_624) noexcept
{
    // 004dba2f: CMP EDX,1 / JL falls through to the event build.
    // 004dba34: CMP [EBP+624h],ESI (ESI is zero here) / JZ returns.
    return unbound_slots < 1 || game_field_624 != 0;
}

MissionOneShots arm_mission_one_shots(std::int32_t local_view_mode) noexcept
{
    MissionOneShots one_shots{};
    one_shots.scene_ended_by_abort = false; // 004da6fd
    one_shots.end_scene_body_done = false; // 004da6ec
    one_shots.session_dropped = false; // 004da6f7
    one_shots.not_enough_players = false; // 004da703
    one_shots.session_was_networked = local_view_mode != 0; // 004da6f2/004da709
    return one_shots;
}

bool run_mission_state_entry(MissionStateEntryState& state,
    const AudioSettings& audio_settings, MissionStateEntryHost& host)
{
    host.enter_scope(kMissionEntryScopeLabel); // 004da6c9

    // 004da6d7: MOVSS, so the store is the float 0.0f, not an integer zero.
    state.scaled_frame_delta = 0.0f;
    host.release_deferred_dynamics(); // 004da6df

    state.one_shots = arm_mission_one_shots(state.local_view_mode);

    if (state.local_view_mode == 0) {
        // 004da70f..004da71e. Only the slot game+18ECh selects is promoted.
        host.mark_local_slot_ready(state.local_slot_index, kLocalSlotReadyValue);
    }

    // 00f889a0 is settings base 00f88980 +20h, written indirectly by settings
    // reset/load. Its zero-initialized image bytes do not make it a constant.
    host.set_audio_environment_level(audio_settings.master_20, kAudioEnvironmentBusMask); // 004da734

    state.game_state = static_cast<std::uint32_t>(GameStateId::kInMission); // 004da73c
    host.apply_in_game_interface(false); // 004da746

    if (state.local_view_mode != 0) {
        host.check_multiplayer_player_count(); // 004da755
        state.game_state = host.game_state(); // 004da75a re-reads game+5D4h
        if (state.game_state != static_cast<std::uint32_t>(GameStateId::kInMission)) {
            return false; // 004da761, the early return
        }
    }

    host.set_cinematic_mode(kMissionEntryCinematicHide, kMissionEntryCinematicAllowSimulation,
        kMissionEntryCinematicThirdArgument); // 004da769
    state.hud_suppressed = false; // 004da76e
    return true;
}

bool run_mission_device_wait(const MissionDeviceWaitInputs& inputs, DeviceWaitLatch& latch,
    MissionDeviceWaitHost& host)
{
    if (!mission_entry_uses_player_count_arm(inputs.local_view_mode, inputs.session_flag_29c)) {
        const bool edge = device_wait_button_edge(latch, host.any_dynamic_device_button_down());
        if (!edge) {
            return false; // 004db97d
        }
        if (!device_wait_screens_idle(host.menu_command_screen_busy(), host.gui_root_busy())) {
            return false; // 004db98f / 004db9a1
        }
        host.update_input_manager(kDeviceWaitInputDelta); // 004db9b4
        host.enter_mission_state(); // 004db9b9 jumps to 004dba94
        return true;
    }

    // 004db9be: only mode 1 has the player-count arm; any other non-zero mode
    // leaves without touching the SEH frame.
    if (inputs.local_view_mode != 1) {
        return false;
    }
    if (!player_count_arm_may_enter(inputs.unbound_player_slots, inputs.game_field_624)) {
        return false; // 004dba3a
    }
    host.dispatch_session_event(kMissionEntrySessionEventTag); // 004dba42..004dba8f
    host.enter_mission_state(); // 004dba96
    return true;
}

}
