#include "bsp/mission_state_frame.hpp"

namespace bsp {
namespace {

// 004e538e: the particle clock argument is the global time multiplied by the
// double 1000.0 at 00ce47a0, computed on the x87 stack and stored back as a
// float. Kept as a double multiply so the rounding matches the listing.
constexpr double kParticleClockMillisecondsScale = 1000.0;

// The step table. Order is the native call order inside 004e4a40; `callee` is
// zero for a step that has no call of its own.
constexpr MissionFrameStep kSteps[] = {
    {"session_counts_mission_start_004b6260", 0x004e4e0fu, 0x004b6260u,
        MissionFrameOwner::kSession, "bsp::session_counts_mission_004b6260"},
    {"adjust_mission_start_counters_004bcaa0", 0x004e4e25u, 0x004bcaa0u,
        MissionFrameOwner::kSession, "bsp::adjust_mission_counters_004bcaa0"},
    {"update_warning_manager_00987590", 0x004e4e67u, 0x00987590u,
        MissionFrameOwner::kScript, "bsp::update_mission_events_00987590"},
    {"run_input_effect_sets_004e4e6c", 0x004e4e6cu, 0x00000000u,
        MissionFrameOwner::kInput, "bsp::run_input_effect_lists_004e4e6c"},
    {"record_action_deadlines_004d8cd0", 0x004e4febu, 0x004d8cd0u,
        MissionFrameOwner::kUnit, "bsp::record_action_deadlines_004d8cd0"},
    {"multiplayer_tick_00778560", 0x004e5036u, 0x00778560u,
        MissionFrameOwner::kSession, "bsp::run_multiplayer_tick_00778560"},
    {"begin_game_profile_block", 0x004e50abu, 0x00be3640u,
        MissionFrameOwner::kProfiler, "bsp::profiler_begin_frame_slot_00be3640"},
    {"register_game_block_label", 0x004e50e3u, 0x0041e870u,
        MissionFrameOwner::kProfiler, ""},
    {"update_in_mission_subsystems_004c40a0", 0x004e5133u, 0x004c40a0u,
        MissionFrameOwner::kUnit, ""},
    {"begin_engine_movie_004cce50", 0x004e5147u, 0x004cce50u,
        MissionFrameOwner::kHud, ""},
    {"pause_gate_branch_004e5153", 0x004e5153u, 0x00000000u,
        MissionFrameOwner::kPure, "bsp::decide_simulation_gate_branch"},
    {"tutorial_hint_step_available", 0x004e5227u, 0x00000000u,
        MissionFrameOwner::kHud, ""},
    {"advance_tutorial_hint_0054e440", 0x004e522du, 0x0054e440u,
        MissionFrameOwner::kHud, ""},
    {"toggle_pause_menu_004db030", 0x004e5234u, 0x004db030u,
        MissionFrameOwner::kHud, ""},
    {"toggle_pause_menu_004db030", 0x004e523bu, 0x004db030u,
        MissionFrameOwner::kHud, ""},
    {"in_game_interface_active", 0x004e524cu, 0x00000000u,
        MissionFrameOwner::kHud, ""},
    {"update_in_game_interface_0068c1f0", 0x004e5252u, 0x0068c1f0u,
        MissionFrameOwner::kHud, "bsp::audio_environment_0068c1f0"},
    {"update_interface_only_004c40f0", 0x004e5259u, 0x004c40f0u,
        MissionFrameOwner::kHud, "bsp::run_interface_only_update_004c40f0"},
    {"hint_tick_cooldowns_0068ec10", 0x004e526du, 0x0068ec10u,
        MissionFrameOwner::kScript, "bsp::tick_hint_cooldowns_0068ec10"},
    {"hint_drain_queued_00692b00", 0x004e5279u, 0x00692b00u,
        MissionFrameOwner::kScript, "bsp::should_drain_queued_hint_00692b00"},
    {"hint_unit_class_00692b60", 0x004e5285u, 0x00692b60u,
        MissionFrameOwner::kScript, "bsp::unit_class_hint_00692b60"},
    {"hint_weapon_006926f0", 0x004e5291u, 0x006926f0u,
        MissionFrameOwner::kScript, "bsp::surface_weapon_hint_006926f0"},
    {"hint_environment_00692580", 0x004e529du, 0x00692580u,
        MissionFrameOwner::kScript, "bsp::environment_hint_00692580"},
    {"hint_zone_first_get_00692fd0", 0x004e52a9u, 0x00692fd0u,
        MissionFrameOwner::kScript, "bsp::scan_capture_zones_00692fd0"},
    {"hint_strategic_map_00692960", 0x004e52b5u, 0x00692960u,
        MissionFrameOwner::kScript, "bsp::should_show_strategic_map_hint_00692960"},
    {"update_bot_scheduler_00914ef0", 0x004e52cau, 0x00914ef0u,
        MissionFrameOwner::kUnit, "bsp::update_bot_scheduler_00914ef0"},
    {"update_markers_006dc1a0", 0x004e52dfu, 0x006dc1a0u,
        MissionFrameOwner::kHud, "bsp::update_markers_006dc1a0"},
    {"update_entity_manager_00481640", 0x004e52f4u, 0x00481640u,
        MissionFrameOwner::kUnit, "bsp::update_entity_manager_00481640"},
    {"update_rain_descriptor_00865ab0", 0x004e5309u, 0x00865ab0u,
        MissionFrameOwner::kRenderer, ""},
    {"update_ocean_00bbddd0", 0x004e5325u, 0x00bbddd0u,
        MissionFrameOwner::kRenderer, "bsp::run_ocean_and_effects_tick"},
    {"update_decals_00740e10", 0x004e533au, 0x00740e10u,
        MissionFrameOwner::kRenderer, "bsp::update_decal_manager_00740e10"},
    {"update_00f89b3c_0094c8f0", 0x004e534fu, 0x0094c8f0u,
        MissionFrameOwner::kRenderer, ""},
    {"update_power_ups_008eb110", 0x004e535au, 0x008eb110u,
        MissionFrameOwner::kUnit, "bsp::update_power_ups_008eb110"},
    {"update_effect_manager_00867ee0", 0x004e5377u, 0x00867ee0u,
        MissionFrameOwner::kRenderer, "bsp::run_ocean_and_effects_tick"},
    {"flush_entity_activations_00903670", 0x004e5382u, 0x00903670u,
        MissionFrameOwner::kUnit, "bsp::flush_entity_activations_00903670"},
    {"check_mission_completion_004d7ea0", 0x004e5389u, 0x004d7ea0u,
        MissionFrameOwner::kScript, "bsp::check_mission_completion_004d7ea0"},
    {"set_particle_clock_time_00b19a10", 0x004e53adu, 0x00b19a10u,
        MissionFrameOwner::kRenderer, "bsp::set_particle_clock_time_00b19a10"},
    {"update_interface_only_004c40f0", 0x004e53b6u, 0x004c40f0u,
        MissionFrameOwner::kHud, "bsp::run_interface_only_update_004c40f0"},
    {"set_foliage_shader_time_00af0450", 0x004e53d9u, 0x00af0450u,
        MissionFrameOwner::kRenderer, ""},
    {"build_foliage_visible_set_00af0c50", 0x004e540fu, 0x00af0c50u,
        MissionFrameOwner::kRenderer, "bsp::build_foliage_visible_set_00af0c50"},
    {"apply_gui_visibility_004c6c70", 0x004e5416u, 0x004c6c70u,
        MissionFrameOwner::kHud, "bsp::decide_gui_visibility_004c6c70"},
    {"interface_manager_present", 0x004e541bu, 0x00000000u,
        MissionFrameOwner::kHud, ""},
    {"update_interface_music_00685c80", 0x004e542du, 0x00685c80u,
        MissionFrameOwner::kSound, ""},
    {"update_multiplayer_interface_004d80d0", 0x004e5434u, 0x004d80d0u,
        MissionFrameOwner::kSession, ""},
    {"service_pending_menu_requests_006840f0", 0x004e5442u, 0x006840f0u,
        MissionFrameOwner::kHud, "bsp::service_pending_menu_requests_006840f0"},
    {"pump_peer_queues_00776230", 0x004e5462u, 0x00776230u,
        MissionFrameOwner::kSession, "bsp::run_menu_interface_drain_004e5434"},
    {"update_interface_only_004c40f0", 0x004e5469u, 0x004c40f0u,
        MissionFrameOwner::kHud, "bsp::run_interface_only_update_004c40f0"},
    {"service_pending_menu_requests_006840f0", 0x004e5477u, 0x006840f0u,
        MissionFrameOwner::kHud, "bsp::service_pending_menu_requests_006840f0"},
    {"apply_sound_requests_00941140", 0x004e5496u, 0x00941140u,
        MissionFrameOwner::kSound, "bsp::apply_sound_request_00941140"},
    {"update_front_end_screens_004d8620", 0x004e549du, 0x004d8620u,
        MissionFrameOwner::kHud, ""},
    {"end_game_profile_block", 0x004e54b0u, 0x00be3660u,
        MissionFrameOwner::kProfiler, "bsp::profiler_end_frame_slot_00be3660"},
    {"begin_render_profile_block", 0x004e54cau, 0x00be3640u,
        MissionFrameOwner::kProfiler, "bsp::profiler_begin_frame_slot_00be3640"},
    {"render_004ca440", 0x004e54d1u, 0x004ca440u,
        MissionFrameOwner::kRenderer, ""},
    {"end_render_profile_block", 0x004e54e4u, 0x00be3660u,
        MissionFrameOwner::kProfiler, "bsp::profiler_end_frame_slot_00be3660"},
    {"finish_render_frame_004ca1f0", 0x004e54ebu, 0x004ca1f0u,
        MissionFrameOwner::kRenderer, ""},
    {"frame_metrics_enabled", 0x004e550cu, 0x00000000u,
        MissionFrameOwner::kProfiler, ""},
    {"submit_frame_metrics_00757ce0", 0x004e551eu, 0x00757ce0u,
        MissionFrameOwner::kProfiler, ""},
};

constexpr std::size_t kStepCount = sizeof(kSteps) / sizeof(kSteps[0]);

} // namespace

// ---------------------------------------------------------------------------
// 00447060
// ---------------------------------------------------------------------------

std::size_t release_all_dynamics_00447060(GameDynamicsList& list, DynamicsReleaseHost& host)
{
    // 00447075..004470b2. The loop reloads +14h and +18h every pass and stops as
    // soon as the index is no longer below the current element count, so a
    // release that shrinks the vector cuts the walk short.
    std::size_t released = 0;
    for (std::size_t index = 0; index < list.record_handles.size(); ++index) {
        host.release_dynamics_handle_00c34f70(list.record_handles[index]);
        ++released;
    }

    // 004470b4..00447106, erase(begin, end) inlined: the move loop copies the
    // empty range [end, end) and the end pointer is set back to begin.
    list.record_handles.clear();

    // 00447109..0044712e, the same erase through 00446ef0 on the 8h range.
    list.pending_count = 0;
    return released;
}

// ---------------------------------------------------------------------------
// 004d87b0
// ---------------------------------------------------------------------------

bool mission_player_count_check_runs(const MissionPlayerCountInputs& inputs) noexcept
{
    // 004d87c1 local view mode, 004d87cd the scene record, 004d87d9 the state.
    return inputs.local_view_mode != 0 && inputs.scene_record_present
        && inputs.game_state == kMissionGameState;
}

bool mission_player_counts_sufficient(int side0, int side1, int effective_game_mode) noexcept
{
    if (effective_game_mode == kMissionFreeForAllGameMode) {
        // 004d88d8: CMP total,1 / JLE fails. More than one participant in total.
        return (side0 + side1) > 1;
    }
    // 004d88dc..004d88e2: both sides strictly positive.
    return side0 > 0 && side1 > 0;
}

MissionSideBalance count_bound_side_entries(const MissionSideSlot* slots, std::size_t count) noexcept
{
    MissionSideBalance balance{};
    if (slots == nullptr) {
        return balance;
    }
    for (std::size_t index = 0; index < count; ++index) {
        const MissionSideSlot& slot = slots[index];
        // 004d8930: the +9h exclusion byte skips the slot outright.
        if (slot.excluded_09) {
            continue;
        }
        // The native code indexes a three-slot stack array with slot+28h and
        // never bounds checks it. Out-of-range sides are dropped here instead
        // of writing past the array; that is a deliberate deviation.
        if (slot.side_28 < 0 || static_cast<std::size_t>(slot.side_28) >= kMissionSideCounterCount) {
            continue;
        }
        int& counter = balance.counts[static_cast<std::size_t>(slot.side_28)];
        // 004d8940: a negative counter is pulled up to zero on first sight.
        if (counter < 0) {
            counter = 0;
        }
        // 004d8950: the +1Bh byte marks the side seen without counting it.
        if (!slot.excluded_1b) {
            counter += 1;
        }
    }
    return balance;
}

bool side_balance_broken(const MissionSideBalance& balance) noexcept
{
    // 004d8962: exactly zero on either side. A side left at the sentinel, i.e.
    // never seen at all, does not fire.
    return balance.counts[0] == 0 || balance.counts[1] == 0;
}

MissionPlayerCountOutcome run_mission_player_count_check_004d87b0(
    const MissionPlayerCountInputs& inputs, MissionPlayerCountHost& host)
{
    if (!mission_player_count_check_runs(inputs)) {
        return MissionPlayerCountOutcome::kNotRun;
    }

    MissionPlayerCountOutcome outcome = MissionPlayerCountOutcome::kSideBalanceOnly;

    if (!inputs.session_flag_29c) {
        int side0 = 0;
        int side1 = 0;
        if (host.participant_blocks_override_active()) {
            // 004d8818..004d8848, eight 118h-stride blocks at game+748h.
            for (std::size_t block = 0; block < 8; ++block) {
                if (!host.participant_block_active_004b5530(block)) {
                    continue;
                }
                const int side = host.participant_block_side(block);
                if (side == 0) {
                    side0 += 1;
                } else if (side == 1) {
                    side1 += 1;
                }
            }
        } else {
            // 004d8850, 004bb770 fills both counts through pointer arguments.
            host.count_participants_004bb770(side0, side1);
        }

        const int mode = host.effective_game_mode_004bca50();
        if (!mission_player_counts_sufficient(side0, side1, mode)) {
            if (!host.retry_latch()) {
                // 004d88a6..004d88c5, the first failure. Nothing else runs.
                host.set_retry_deadline(
                    host.global_time_00f876a4() + kMissionPlayerCountRetryDelaySeconds);
                host.set_mission_failure_byte();
                host.set_retry_latch(true);
                return MissionPlayerCountOutcome::kFirstFailureArmed;
            }
            host.show_message_00734e50(kMissionNotEnoughPlayersKey);
            // 004d8950 in the message arm: mode 1 only, and only once the
            // deadline has been reached.
            if (inputs.local_view_mode == 1
                && host.retry_deadline() <= host.global_time_00f876a4()) {
                host.end_scene_004d7970(false);
                host.set_retry_latch(false);
                return MissionPlayerCountOutcome::kEndSceneRequested;
            }
            // The failure arm returns without reaching the balance block.
            return MissionPlayerCountOutcome::kWarningShown;
        }

        // 004d88e4: the counts passed. The latch is cleared and control falls
        // through into the balance block the other arm jumps to.
        host.set_retry_latch(false);
        outcome = MissionPlayerCountOutcome::kSufficient;
    }

    // 004d88eb..004d8990, the shared continuation.
    if (!host.side_balance_latch() && inputs.local_view_mode == 1) {
        // 004d8907: the slot count comes from the scene record, not from a
        // fixed eight, and a count of zero skips the whole walk.
        const std::size_t count = host.scene_slot_count();
        if (count > 0) {
            std::vector<MissionSideSlot> slots;
            slots.reserve(count);
            for (std::size_t index = 0; index < count; ++index) {
                slots.push_back(host.scene_slot(index));
            }
            const MissionSideBalance balance = count_bound_side_entries(slots.data(), slots.size());
            if (side_balance_broken(balance)) {
                host.report_side_empty_00982990(balance.counts[0] != 0);
                host.set_side_balance_latch();
                host.dismiss_menu_prompts_00530650();
            }
        }
    }
    return outcome;
}

// ---------------------------------------------------------------------------
// The in-mission frame
// ---------------------------------------------------------------------------

std::size_t mission_frame_step_count() noexcept
{
    return kStepCount;
}

const MissionFrameStep& mission_frame_step(std::size_t index) noexcept
{
    return kSteps[index < kStepCount ? index : kStepCount - 1];
}

MissionFrameResult run_mission_frame(MissionFrameState& state, MissionFrameHost& host)
{
    MissionFrameResult result{};
    const bool in_mission = state.state == kMissionGameState;

    // --- 004e4df4: the in-mission effect block -----------------------------
    if (in_mission) {
        if (!state.mission_start_latched) {
            // 004e4e00..004e4e2a, the one-shot at +1EE7h.
            if (host.session_counts_mission_start_004b6260() && !state.session_flag_624) {
                host.adjust_mission_start_counters_004bcaa0(true);
            }
            state.mission_start_latched = true;
        }
        // 004e4e31: a running cinematic skips the rest of the block.
        if (!state.cinematic_hidden) {
            // 004e4e3e..004e4e67, COMISS against the 0.0f at 00d7a218 with JBE,
            // so a delta of exactly zero does not run the director.
            if (state.warning_manager_present && state.scaled_delta > 0.0f) {
                host.update_warning_manager_00987590(state.scaled_delta);
            }
            result.input_entries_erased = host.run_input_effect_sets_004e4e6c(state.global_time);
        }
    }

    // --- 004e4feb: frame bookkeeping ---------------------------------------
    host.record_action_deadlines_004d8cd0();
    state.frame_counter += 1; // 004e4ff5

    // 004e5000..004e5020. The network tick is skipped only when all three hold.
    const bool skip_network_tick
        = in_mission && state.max_step_clamp_disabled && state.scaled_delta > 0.0f;
    if (!skip_network_tick) {
        host.multiplayer_tick_00778560(state.raw_delta);
    }
    // The state 0Ch and 11h arms at 004e503b and 004e504b are the spine's:
    // bsp/game_frame_control.hpp owns update_device_wait_screen_004db920 and
    // update_mission_end_wait.

    host.begin_game_profile_block(); // 004e509d..004e50ab

    // --- 004e50b0: the simulation gate -------------------------------------
    bool gate_open = !state.cinematic_hidden || state.cinematic_allow_simulation;
    if (gate_open) {
        if (!state.game_block_label_registered) {
            host.register_game_block_label(); // 004e50c6..004e5110
            state.game_block_label_registered = true;
        }
        // 004e5118, 004e5124.
        gate_open = in_mission && !state.mission_end_suspended;
    }

    if (gate_open) {
        result.simulated = true;
        host.update_in_mission_subsystems_004c40a0(); // 004e5133
        if (state.engine_movie_pending) {
            host.begin_engine_movie_004cce50(); // 004e5147
            state.engine_movie_pending = false; // 004e514c, after the call
        }

        const SimulationGateBranch branch = host.pause_gate_branch_004e5153();
        if (branch == SimulationGateBranch::kPauseMenuOpened) {
            result.paused = true;
            // 004e5213..004e522b: the hint arm needs the cinematic flag set and
            // the hint object's +8h byte set, and it falls through into the
            // second toggle, so this arm calls 004db030 twice.
            if (state.cinematic_hidden && host.tutorial_hint_step_available()) {
                host.advance_tutorial_hint_0054e440(); // 004e522d
                host.toggle_pause_menu_004db030();     // 004e5234
            }
            host.toggle_pause_menu_004db030(); // 004e523b
        } else if (branch == SimulationGateBranch::kInterfaceOnly) {
            if (host.in_game_interface_active()) {
                host.update_in_game_interface_0068c1f0(); // 004e5252
            }
            host.update_interface_only_004c40f0(); // 004e5259
        }
        // kSimulationOnly is the 004e517a jump straight to the hint passes.

        // --- 004e525e: the seven hint passes -------------------------------
        host.hint_tick_cooldowns_0068ec10(state.raw_delta);
        host.hint_drain_queued_00692b00();
        host.hint_unit_class_00692b60();
        host.hint_weapon_006926f0();
        host.hint_environment_00692580();
        host.hint_zone_first_get_00692fd0();
        host.hint_strategic_map_00692960();

        // --- 004e52ba: the world tick --------------------------------------
        host.update_bot_scheduler_00914ef0(state.scaled_delta);
        host.update_markers_006dc1a0(state.scaled_delta);
        host.update_entity_manager_00481640(state.scaled_delta);
        if (state.ocean_owner_present) { // 004e52f9
            host.update_rain_descriptor_00865ab0();
            host.update_ocean_00bbddd0(state.scaled_delta);
        }
        host.update_decals_00740e10(state.scaled_delta);
        host.update_00f89b3c_0094c8f0(state.scaled_delta);
        host.update_power_ups_008eb110();
        host.update_effect_manager_00867ee0(state.scaled_delta);
        host.flush_entity_activations_00903670();
        result.mission_completion_requested = host.check_mission_completion_004d7ea0();

        // --- 004e538e: the particle clock ----------------------------------
        const double milliseconds
            = static_cast<double>(state.global_time) * kParticleClockMillisecondsScale;
        host.set_particle_clock_time_00b19a10(static_cast<float>(milliseconds));
    } else {
        host.update_interface_only_004c40f0(); // 004e53b6, the gate fallback
    }

    // --- 004e53bb: post-simulation -----------------------------------------
    if (in_mission) {
        host.set_foliage_shader_time_00af0450(state.scene_shader_time); // 004e53d9
        host.build_foliage_visible_set_00af0c50();                      // 004e540f
    }
    host.apply_gui_visibility_004c6c70(); // 004e5416
    if (host.interface_manager_present()) {
        host.update_interface_music_00685c80(state.raw_delta); // 004e542d
    }
    host.update_multiplayer_interface_004d80d0(); // 004e5434

    // 004e5442..004e5488, the loop Ghidra drops from the pseudocode: the menu
    // state machine is run to a fixed point with no iteration bound.
    while (host.service_pending_menu_requests_006840f0()) {
        result.menu_drain_iterations += 1;
        host.pump_peer_queues_00776230();       // 004e5462
        host.update_interface_only_004c40f0();  // 004e5469
    }

    host.apply_sound_requests_00941140();      // 004e548f, 004e5496
    host.update_front_end_screens_004d8620();  // 004e549d

    // --- 004e54a2: render ---------------------------------------------------
    host.end_game_profile_block();
    // The test at 004e54b5 reads a byte both incoming paths zeroed at 004e548a,
    // so the render block is never skipped; no gate is modelled.
    host.begin_render_profile_block();
    host.render_004ca440();
    host.end_render_profile_block();
    host.finish_render_frame_004ca1f0();

    // --- 004e550c: the tail -------------------------------------------------
    if (host.frame_metrics_enabled()) {
        host.submit_frame_metrics_00757ce0(state.raw_delta);
    }
    // The 004e54fd comparison against 0x3200000 only writes the SEH try-level
    // slot; its guarded body was optimised away and is not reproduced.
    return result;
}
}
