#include "bsp/simulation_gate.hpp"

#include <algorithm>

namespace bsp {

HudLayerVisibility hud_layer_visibility_004cd0f0(
    bool hud_hidden, int game_state, bool pending_teardown) noexcept {
    // 004CD142..004CD165. The shared byte is cleared only when the routine is
    // un-hiding the HUD and the game is neither in nor entering teardown; every
    // other combination leaves the effect layers hidden.
    const bool leaving_hidden = !hud_hidden
        && game_state != static_cast<int>(GameStateId::kMissionTeardown) && !pending_teardown;
    HudLayerVisibility visibility{};
    visibility.in_game_gui_hidden = hud_hidden; // 004CD1AD passes game+634h itself
    visibility.effects_hidden = !leaving_hidden;
    return visibility;
}

CinematicModeFlags cinematic_flags_004cd0f0(bool hud_hidden, bool allow_simulation) noexcept {
    // 004CD26C: `game+635h = -(game+634h != 0) & p3`, a mask AND, so +635h can
    // only be set while +634h is.
    CinematicModeFlags flags{};
    flags.hud_hidden = hud_hidden;
    flags.simulate_while_hidden = hud_hidden && allow_simulation;
    return flags;
}

bool simulation_gate_open(const SimulationGateEntry& entry) noexcept {
    // 004E50B0: `cmp byte [esi+634h],0 / je` then `cmp byte [esi+635h],0 / je
    // 004E53B4`, so a hidden HUD only stops the block when +635h is clear.
    if (entry.cinematic.hud_hidden && !entry.cinematic.simulate_while_hidden) return false;
    // 004E5118 compares against EBX, loaded with 0Dh at 004E4FFB and untouched
    // in between; 004E5124 is the suspend byte.
    if (entry.game_state != static_cast<int>(GameStateId::kInMission)) return false;
    return !entry.simulation_suspended;
}

bool is_base_interface_0068a140(int interface_id) noexcept {
    return interface_id == kInterfaceShipyard || interface_id == kInterfaceAirbase;
}

int effective_game_mode_004bca50(
    int raw_mode, bool mode_forced, bool multiplayer_session) noexcept {
    if (!mode_forced && !multiplayer_session && raw_mode != 9
        && raw_mode != kDefaultSinglePlayerGameMode) {
        return kDefaultSinglePlayerGameMode;
    }
    return raw_mode;
}

bool multiplayer_spectator_active_004bfe50(
    int effective_mode, bool spectator_enabled, int spectator_state) noexcept {
    const bool mode_allowed = std::find(kSpectatorGameModes.begin(), kSpectatorGameModes.end(),
                                  effective_mode)
        != kSpectatorGameModes.end();
    // 004BFE93..004BFEBC, two short-circuiting accessor calls on the same object.
    return mode_allowed && spectator_enabled && spectator_state == 1;
}

int top_populated_level_004f7740(
    const std::array<std::size_t, kPriorityLevelCount>& level_sizes) noexcept {
    // The native walk runs from the highest address down and stops at the first
    // non-empty vector; index 0 of this array is level 1, at 00E18CFC.
    for (std::size_t i = kPriorityLevelCount; i > 0; --i) {
        if (level_sizes[i - 1] != 0) return static_cast<int>(i);
    }
    return 0;
}

namespace {
// 004E5180..004E51DB. Each failure below falls through to the same shared pause
// test that the not-pressed case reaches, so the chain never opens the pause
// menu by itself. Its one effect is the early exit at 004E51DB.
bool alternate_chain_suppresses_pause(const PauseDecisionInputs& inputs) noexcept {
    if (!inputs.alternate_pause_pressed || inputs.hud_hidden) return false;
    // 004E519B: `cmp eax,edi` with EDI = 1 and JG, so the base-screen test is
    // consulted only when the top level is 1 or lower.
    const bool level_ok = inputs.top_populated_level > 1 || inputs.base_interface_active;
    const bool spectator_ok = !inputs.spectator_gate_enabled || inputs.spectator_flag;
    return level_ok && spectator_ok && !inputs.multiplayer_spectator_active;
}
}

SimulationGateBranch decide_simulation_gate_branch(const PauseDecisionInputs& inputs) noexcept {
    // 004E5153..004E517A. Only inside a network session, and only when the
    // selected slot exists: a non-positive counter jumps clear of both branches.
    if (inputs.multiplayer_session && inputs.active_slot_present
        && inputs.active_slot_counter <= 0) {
        return SimulationGateBranch::kSimulationOnly;
    }

    if (alternate_chain_suppresses_pause(inputs)) return SimulationGateBranch::kInterfaceOnly;

    // 004E51DD..004E5211.
    if (!inputs.pause_pressed) return SimulationGateBranch::kInterfaceOnly;
    if (inputs.scene_console_open || inputs.scene_modal_188h || inputs.scene_modal_218h) {
        return SimulationGateBranch::kInterfaceOnly;
    }
    return SimulationGateBranch::kPauseMenuOpened;
}

std::string_view audio_environment_0068c1f0(const AudioEnvironmentInputs& inputs) noexcept {
    if (inputs.cockpit_interface_active) return kAudioEnvironmentCockpit;
    // 0068CC42: `fcompi` then a bitwise AND of the two byte results, not a
    // short circuit, so both terms are always evaluated in the native body.
    const bool above_water = inputs.water_height < inputs.camera_height;
    return (above_water && !inputs.device_allows_underwater) ? kAudioEnvironmentAir
                                                             : kAudioEnvironmentUnderwater;
}

SimulationGateBranch run_simulation_gate(
    SimulationGateState& state, SimulationGateHost& host, const SimulationGateEntry& entry) {
    if (!simulation_gate_open(entry)) {
        // 004E53B4. The same callee as the interface-only branch, reached from
        // a different site, and the simulation below it never runs.
        host.update_interface_only();
        return SimulationGateBranch::kGateClosed;
    }

    // 004E5131 and 004E5138. Both run before the pause decision, so a frame
    // that skips both branches still advances these.
    host.update_in_mission_subsystems();
    if (state.engine_movie_requested) {
        host.begin_engine_movie();
        state.engine_movie_requested = false; // 004E514C
    }

    PauseDecisionInputs inputs{};
    inputs.multiplayer_session = host.multiplayer_session();
    if (inputs.multiplayer_session) {
        inputs.active_slot_present = host.active_local_player_slot(inputs.active_slot_counter);
    }
    if (inputs.multiplayer_session && inputs.active_slot_present
        && inputs.active_slot_counter <= 0) {
        return SimulationGateBranch::kSimulationOnly;
    }

    inputs.alternate_pause_pressed = host.input_action_pressed(kAlternatePauseAction);
    if (inputs.alternate_pause_pressed) {
        inputs.hud_hidden = host.hud_hidden();
        if (!inputs.hud_hidden) {
            inputs.top_populated_level = host.top_populated_level();
            if (inputs.top_populated_level <= 1) {
                inputs.base_interface_active = host.base_interface_active();
            }
            if (inputs.top_populated_level > 1 || inputs.base_interface_active) {
                inputs.spectator_gate_enabled = host.spectator_gate_enabled();
                if (inputs.spectator_gate_enabled) inputs.spectator_flag = host.spectator_flag();
                if (!inputs.spectator_gate_enabled || inputs.spectator_flag) {
                    inputs.multiplayer_spectator_active = host.multiplayer_spectator_active();
                }
            }
        }
    }

    // 004E51DD onward, reached only when the chain above did not divert to
    // 004E5242. The three scene fields short-circuit, so each is queried only
    // while the ones before it are clear.
    if (!alternate_chain_suppresses_pause(inputs)) {
        inputs.pause_pressed = host.input_action_pressed(kPauseAction);
    }
    if (inputs.pause_pressed) {
        inputs.scene_console_open = host.scene_console_open();
        if (!inputs.scene_console_open) inputs.scene_modal_188h = host.scene_modal_188h();
        if (!inputs.scene_console_open && !inputs.scene_modal_188h) {
            inputs.scene_modal_218h = host.scene_modal_218h();
        }
    }

    const SimulationGateBranch branch = decide_simulation_gate_branch(inputs);
    if (branch == SimulationGateBranch::kPauseMenuOpened) {
        // 004E5213..004E5240. The HUD byte is read a second time here, after
        // the action tests, and the free-camera object only matters while it is
        // already raised.
        if (host.hud_hidden() && host.free_camera_active()) {
            host.build_pause_unit_list();
            host.toggle_pause_menu();
        }
        host.toggle_pause_menu();
        return branch;
    }

    // 004E5242..004E525C.
    if (host.in_mission_interface_ready()) host.update_in_mission_interface();
    host.update_interface_only();
    return branch;
}
}
