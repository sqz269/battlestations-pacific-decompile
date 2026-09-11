// Reconstruction of the in-mission interface manager's lifecycle:
// 0068A990 (construct), 0068CC70 (Init), 0068C1F0 (the per-frame update the
// in-mission frame calls at 004E5252) and 0068BC60 / 0068B630 (teardown).
// docs/IN_MISSION_INTERFACE_RUNTIME.md carries the address-by-address evidence.
#include "bsp/in_mission_interface_runtime.hpp"

namespace bsp {
namespace {

// The level-3 collapse the update performs at four sites (0068C569, 0068C7C0,
// 0068C9ED, 0068CA68) and that 0068C0B0, 0068B3F0 and 0068B470 repeat. All four
// sites are byte-identical: the level-3 vector must be non-empty, the camera
// screen's +0Ah is raised when the global camera screen's +30h is set, both
// level-3 setters are called with the empty list, and the scene request is
// re-issued for 004B4B00()'s unit through 0068AA40.
Level3CollapseOutcome collapse_level3(InMissionInterfaceUpdateHost& host) {
    Level3CollapseOutcome outcome{};
    if (host.level3_screen_count() == 0) {
        return outcome;
    }
    if (host.camera_screen_flag_30()) {
        host.camera_screen_set_flag_0a(true);
        outcome.camera_flag_0a_set = true;
    }
    host.set_level3_screen_set(nullptr, 0);
    host.set_level3_input_contexts(nullptr, 0);
    host.push_scene_request_for_unit();
    outcome.ran = true;
    outcome.scene_request_pushed = true;
    return outcome;
}

// The spectator block, 0068C21F..0068C3E7. Runs only for the four modes
// in_mission_spectator_mode admits.
void run_spectator_block(InMissionInterfaceUpdateState& state,
    InMissionInterfaceUpdateHost& host) {
    // 0068C231..0068C26B: the local player slot asked to spectate and the camera
    // screen is not already doing it.
    if (host.local_slot_flag_19() && !host.camera_screen_spectating()
        && !host.camera_screen_flag_09() && !host.camera_screen_flag_0a()) {
        host.camera_screen_set_flag_08(false);
        host.toggle_tactical_overlay(0);
    }

    // 0068C270..0068C3B4: the walk over [[game+19CCh]+58h]. Both arms break.
    state.spectate_candidate_found = false;
    const int player_slot = host.local_player_slot();
    const std::size_t unit_count = host.spectate_unit_count();
    for (std::size_t i = 0; i < unit_count; ++i) {
        // 0068C29C..0068C2DB, the "the plane this slot flew is gone" arm.
        if (host.unit_is_kind_of(i, kUnitTypePlane) && host.unit_has_pilot(i)
            && host.unit_owner_id(i) == host.local_slot_unit_id()
            && spectate_walk_pilot_state_aborts(host.unit_pilot_state(i))) {
            host.pilot_detach(i);
            host.camera_screen_set_flag_08(false);
            host.toggle_tactical_overlay(0);
            break;
        }
        // 0068C2DD..0068C31A, the "this unit belongs to me" arm. The team read
        // at 0068C303 happens once and is compared against two values.
        if (!host.unit_dead(i) && host.unit_player_slot(i) == player_slot
            && !host.unit_is_kind_of(i, kUnitClassCommandBuilding)) {
            const int team = host.unit_team(i);
            if (team == kSpectateAnyTeam || team == player_slot) {
                state.spectate_candidate_found = true;
                if (host.camera_screen_spectating()) {
                    host.unit_bind_player(i, player_slot);
                    host.camera_screen_set_flag_08(true);
                    host.exit_free_camera_overlay();
                    // 0068C38E: a plane hands the HUD root its +9D4h instead.
                    const bool use_pilot = host.unit_is_kind_of(i, kUnitTypePlane);
                    host.hud_root_set_spectated_unit(i, use_pilot);
                }
                break;
            }
        }
    }

    // 0068C3B5..0068C3E2.
    if (host.camera_screen_flag_08() && !state.spectate_candidate_found
        && !host.camera_screen_flag_09()) {
        host.camera_screen_set_flag_08(false);
    }
}

// The free-camera toggle, 0068C3E7..0068C462.
void run_free_camera_toggle(InMissionInterfaceUpdateState& state,
    InMissionInterfaceUpdateHost& host) {
    if (!state.free_camera_requested && !host.input_action_pressed(kActionFreeCameraToggle)) {
        return;
    }
    state.free_camera_requested = false;
    if (host.screen_2b_wanted()) {
        // 0068C452: back to the scene, carrying the controlled unit as payload.
        host.push_interface_request(kInterfaceScene3d, host.has_controlled_unit());
        return;
    }
    host.hud_root_set_field_1c(0);
    host.push_interface_request(kInterfaceFreeCamera, false);
    // 0068C42F is a second, independent edge read of the same action: the arm
    // reached here through the 00E1AE80 latch passes 1, the arm reached through
    // the action passes 0.
    const bool toggle_pressed = host.input_action_pressed(kActionFreeCameraToggle);
    host.screen_2b_set_mode(!toggle_pressed);
}

// The spectate-next block, 0068C462..0068C5E4. The scene request at its end is
// the inlined copy of 0068AA40, which is why the rule is applied here rather
// than reached through `push_scene_request_for_unit`.
void run_spectate_next_block(const InGameInterfaceManager& manager, int effective_game_mode,
    InMissionInterfaceUpdateHost& host) {
    if (!host.local_slot_flag_19()) {
        return;
    }
    bool suppress = host.camera_screen_spectating();
    if (host.level3_screen_count() == 1) {
        // 0068C4BC and 0068C4E2 are two independent reads of the same element,
        // combined with byte ORs rather than a short circuit.
        const bool is_a = host.level3_screen_id(0) == kLevel3ScreenSuppressA;
        const bool is_b = host.level3_screen_id(0) == kLevel3ScreenSuppressB;
        suppress = suppress || is_a || is_b;
    }
    if (suppress) {
        return;
    }
    const bool allowed = spectate_next_mode_allows_empty_unit(effective_game_mode)
        || host.has_controlled_unit() || host.camera_screen_spectating()
        || host.screen_34_query() || host.camera_screen_flag_66();
    if (!allowed) {
        return;
    }
    if (host.level3_screen_count() == 0) {
        return;
    }
    if (host.camera_screen_flag_30()) {
        host.camera_screen_set_flag_0a(true);
    }
    host.set_level3_screen_set(nullptr, 0);
    host.set_level3_input_contexts(nullptr, 0);

    SceneRequestUnitFlags flags{};
    bool is_pending_payload = false;
    const bool present = host.query_scene_unit(flags, is_pending_payload);
    if (scene_request_accepts_unit_0068aa40(present, is_pending_payload, flags,
            manager.base.pending.interface_id)) {
        host.push_interface_request(kInterfaceScene3d, true);
    }
}

// The input block, 0068C61A..0068CAA9. Entered only while the pending interface
// is not one of the four camera modes.
void run_input_block(InMissionInterfaceUpdateHost& host) {
    // 0068C641..0068C6BB, the back-out action.
    if (!host.input_action_pressed(kActionBackOutOverlay)) {
        host.screen_3b_idle();
    } else if (host.pause_screen_flag_05() && host.pause_screen_flag_08()) {
        host.screen_3b_idle();
    } else if (host.modifier_key_allows_back_out() && !host.screen_3b_block_a()
        && !host.screen_3b_block_b() && !host.screen_45_block_a()
        && !host.screen_45_block_b()) {
        host.back_out_one_overlay_level();
    }

    // 0068C6C0..0068C6F9, the level-2 close.
    if (host.input_action_pressed(kActionCloseLevel2) && host.screen_2a_wanted()) {
        host.screen_2a_clear_wanted();
        host.collapse_overlays(true, true);
        host.input_manager_update(0.0f);
    }

    // 0068C6FE..0068C78A: six input bits are sampled before any of them is
    // tested, then the two screens take their per-frame step.
    const bool overlay_a_pressed = host.input_action_pressed(kActionOverlayA);
    const bool device_a_0b = host.device_a_flag_0b();
    const bool device_a_10 = host.device_a_flag_10();
    const bool overlay_b_pressed = host.input_action_pressed(kActionOverlayB);
    const bool device_b_0b = host.device_b_flag_0b();
    const bool device_b_10 = host.device_b_flag_10();
    host.camera_screen_frame_step();
    host.map_screen_frame_step();

    // 0068C78F..0068C7FC: the map closes itself when nothing is pending on it.
    if (host.map_screen_wanted() && !host.support_request_pending()) {
        static_cast<void>(collapse_level3(host));
    }

    // 0068C801..0068CA2C: one four-armed chain, first match wins.
    if (device_a_0b && !host.map_screen_wanted()) {
        const std::uint32_t argument = host.has_controlled_unit()
            ? host.controlled_unit_overlay_argument()
            : 0u;
        host.toggle_tactical_overlay(argument);
    } else if (device_b_0b && !host.map_screen_wanted() && !host.camera_screen_flag_09()
        && !(host.pause_screen_child_query() && host.pause_screen_flag_08())) {
        const std::uint32_t argument = host.has_controlled_unit()
            ? host.map_screen_overlay_argument()
            : 0u;
        host.toggle_tactical_overlay(argument);
    } else if ((device_a_10 || device_b_10 || host.input_action_pressed(kActionOverlayC))
        && !host.screen_3b_block_a() && !host.screen_3b_block_b()
        && !(host.pause_screen_flag_05() && host.pause_screen_flag_08())) {
        if (host.overlay_c_extra_gate() && !host.map_screen_wanted()
            && host.support_request_pending()) {
            host.set_level3_screen_set(kOverlayCLevel3Screens, 2);
            if (host.session_flag_19c4()) {
                host.set_level3_input_contexts(kOverlayCLevel3ContextsNetworked, 2);
            } else {
                host.set_level3_input_contexts(kOverlayCLevel3ContextsLocal, 5);
            }
        } else {
            static_cast<void>(collapse_level3(host));
        }
    } else if (overlay_a_pressed || overlay_b_pressed) {
        if (host.camera_screen_spectating()) {
            host.camera_screen_set_flag_0a(true);
            host.exit_free_camera_overlay();
        } else if (host.map_screen_wanted() && !host.map_screen_busy()) {
            static_cast<void>(collapse_level3(host));
        }
    }

    // 0068CA2C..0068CAA4.
    if (host.input_action_pressed(kActionCollapseLevel3) && host.map_screen_wanted()
        && !host.map_screen_busy()) {
        static_cast<void>(collapse_level3(host));
    }

    // 0068CAA9..0068CAD4.
    if (host.has_controlled_unit() && host.camera_screen_flag_66()) {
        host.camera_screen_set_flag_66(false);
    }
}

// The ambience volume and the environment name, 0068CAD9..0068CC5D. Both run on
// every frame, including the camera modes the input block skips.
void run_audio_tail(InMissionInterfaceUpdateHost& host) {
    // 0068CAD9..0068CB45. The null test on +104h is the NEG/SBB/TEST idiom, not
    // a mask test: the constant E19311h is compiler noise.
    if (host.has_ambient_sound_instance() && host.camera_present()) {
        host.refresh_camera_transform();
        host.set_ambient_volume(
            ambient_volume_from_camera_height_0068cb0c(host.camera_height()));
    }

    InMissionAudioEnvironmentInputs inputs{};
    inputs.cockpit_screen_wanted = host.cockpit_screen_wanted();
    if (inputs.cockpit_screen_wanted) {
        inputs.cockpit_screen_mode = host.cockpit_screen_mode();
    }
    bool cockpit = inputs.cockpit_screen_wanted
        && inputs.cockpit_screen_mode == kCockpitScreenMode;
    if (!cockpit) {
        inputs.second_cockpit_screen_wanted = host.second_cockpit_screen_wanted();
        if (inputs.second_cockpit_screen_wanted) {
            inputs.second_cockpit_screen_mode = host.second_cockpit_screen_mode();
        }
        cockpit = inputs.second_cockpit_screen_wanted
            && inputs.second_cockpit_screen_mode == kSecondCockpitScreenMode;
    }
    if (!cockpit) {
        // 0068CB94..0068CC3A. The camera is dereferenced without a null test
        // here, unlike the volume block above.
        host.refresh_camera_transform();
        inputs.camera_height = host.camera_height();
        inputs.water_height = host.water_height(host.camera_ground_x(), inputs.camera_height);
        inputs.has_controlled_unit = host.has_controlled_unit();
        if (inputs.has_controlled_unit) {
            inputs.controlled_unit_is_submarine = host.controlled_unit_is_submarine();
            if (inputs.controlled_unit_is_submarine) {
                host.refresh_controlled_unit_pose();
                inputs.controlled_unit_height = host.controlled_unit_height();
            }
        }
    }
    host.set_audio_environment(in_mission_audio_environment_0068cb4a(inputs));
    host.tick_profile_hints(kProfileHintsSlot);
}

}  // namespace

// ---------------------------------------------------------------------------
// 1. The constructor
// ---------------------------------------------------------------------------

bool in_game_interface_constructor_clears(std::uint16_t offset) noexcept {
    for (std::size_t i = 0; i < kInGameInterfaceConstructorClearCount; ++i) {
        if (kInGameInterfaceConstructorClears[i] == offset) {
            return true;
        }
    }
    return false;
}

void construct_in_game_interface_0068a990(InGameInterfaceManager& manager) noexcept {
    const std::size_t screen_54 = in_game_hud_screen_index_at(0x54);
    if (screen_54 < kInGameHudScreenCount) {
        manager.screen_constructed[screen_54] = false;
    }
    manager.field_ec = 0;
    manager.field_fd = false;
    manager.ambient_sound_source = 0;
    manager.ambient_sound_instance = 0;
}

// ---------------------------------------------------------------------------
// 2. Init
// ---------------------------------------------------------------------------

void init_in_game_interface_0068cc70(InGameInterfaceManager& manager,
    InMissionInterfaceInitHost& host) {
    host.load_texture_atlas(kInGameInterfaceAtlas);
    host.publish_manager_global();
    host.base_init();

    for (std::size_t order = 0; order < kInGameHudScreenCount; ++order) {
        std::size_t index = kInGameHudScreenCount;
        for (std::size_t i = 0; i < kInGameHudScreenCount; ++i) {
            if (kInGameHudScreens[i].init_order == order) {
                index = i;
                break;
            }
        }
        if (index == kInGameHudScreenCount) {
            continue;
        }
        const InGameHudScreenSlot& slot = kInGameHudScreens[index];
        const bool allocated = host.allocate_screen(index, slot.size_bytes);
        if (allocated) {
            host.construct_screen(index, slot.constructor);
        }
        manager.screen_constructed[index] = allocated;
        // The register call is not guarded by the allocation: the native code
        // loads the stored pointer and calls its virtual +10h either way.
        host.register_screen(index);
    }

    host.push_interface_request(kInterfaceScene3d, false);
    host.hud_root_set_field_1c(0);
}

// ---------------------------------------------------------------------------
// 3. The per-frame update
// ---------------------------------------------------------------------------

bool in_mission_spectator_mode(int effective_game_mode) noexcept {
    return effective_game_mode == 4 || effective_game_mode == 5 || effective_game_mode == 6
        || effective_game_mode == 7;
}

bool spectate_next_mode_allows_empty_unit(int effective_game_mode) noexcept {
    return effective_game_mode == 8 || effective_game_mode == 0 || effective_game_mode == 1
        || effective_game_mode == 2 || effective_game_mode == 3;
}

bool spectate_walk_pilot_state_aborts(int pilot_state) noexcept {
    return pilot_state == 4 || pilot_state == 5;
}

bool level3_entry_suppresses_spectate(int screen_id) noexcept {
    return screen_id == kLevel3ScreenSuppressA || screen_id == kLevel3ScreenSuppressB;
}

float ambient_volume_from_camera_height_0068cb0c(float camera_height) noexcept {
    // The product is formed on the x87 stack from two doubles and stored back as
    // a float before the clamp, so the clamp sees the rounded value.
    const float scaled = static_cast<float>(
        (static_cast<double>(camera_height) - static_cast<double>(kAmbientVolumeHeightBias))
        * static_cast<double>(kAmbientVolumeHeightScale));
    // 0068CB2C is a JBE on the comparison of 0.0f with the value, so only a
    // strictly negative value clamps to zero and a NaN passes through both
    // comparisons, exactly as it does here.
    if (scaled < 0.0f) {
        return 0.0f;
    }
    if (scaled > kAmbientVolumeMax) {
        return kAmbientVolumeMax;
    }
    return scaled;
}

bool controlled_unit_allows_air_0068cbf3(bool has_controlled_unit, bool unit_is_submarine,
    float unit_height) noexcept {
    if (!has_controlled_unit || !unit_is_submarine) {
        return true;
    }
    // 0068CC28 is COMISS/JNC on `height >= -4.0f`, so the forcing side needs a
    // strictly lower height and a NaN does not force.
    return !(unit_height < kUnderwaterUnitHeight);
}

std::string_view in_mission_audio_environment_0068cb4a(
    const InMissionAudioEnvironmentInputs& inputs) noexcept {
    if (inputs.cockpit_screen_wanted && inputs.cockpit_screen_mode == kCockpitScreenMode) {
        return "Cockpit";
    }
    if (inputs.second_cockpit_screen_wanted
        && inputs.second_cockpit_screen_mode == kSecondCockpitScreenMode) {
        return "Cockpit";
    }
    const bool above_water = inputs.camera_height > inputs.water_height;
    const bool allows_air = controlled_unit_allows_air_0068cbf3(inputs.has_controlled_unit,
        inputs.controlled_unit_is_submarine, inputs.controlled_unit_height);
    // 0068CC3A is `TEST BL,AL`, a byte AND of two already-computed flags, so
    // both terms are always evaluated.
    return (above_water && allows_air) ? "Air" : "Underwater";
}

bool scene_request_accepts_unit_0068aa40(bool unit_present, bool unit_is_pending_payload,
    const SceneRequestUnitFlags& flags, int pending_interface_id) noexcept {
    if (unit_is_pending_payload || !unit_present) {
        return false;
    }
    if (!flags.flag_5c || flags.flag_5d || flags.flag_60 || flags.flag_5e) {
        return false;
    }
    return !in_game_interface_is_camera_mode(pending_interface_id);
}

void update_in_game_interface_0068c1f0(InGameInterfaceManager& manager,
    InMissionInterfaceUpdateState& state, InMissionInterfaceUpdateHost& host) {
    const int mode = host.effective_game_mode();
    if (in_mission_spectator_mode(mode)) {
        run_spectator_block(state, host);
    }
    run_free_camera_toggle(state, host);
    run_spectate_next_block(manager, mode, host);

    // 0068C5E4..0068C615, the two movie-camera actions.
    if (host.input_action_pressed(kActionMovieCamera)) {
        host.toggle_movie_camera();
    }
    if (host.input_action_pressed(kActionMovieCameraNew)) {
        host.toggle_new_movie_camera();
    }
    if (!in_game_interface_is_camera_mode(manager.base.pending.interface_id)) {
        run_input_block(host);
    }
    run_audio_tail(host);
}

// ---------------------------------------------------------------------------
// 4. Teardown
// ---------------------------------------------------------------------------

void destruct_in_game_interface_0068b630(InGameInterfaceManager& manager,
    InMissionInterfaceTeardownHost& host) {
    host.restore_vtable(kInGameInterfaceManagerVtable);
    if (manager.ambient_sound_instance != 0) {
        host.stop_ambient_sound();
        host.release_ambient_instance();
        manager.ambient_sound_instance = 0;
    }

    const std::size_t collected = host.collect_screens();
    for (std::size_t i = 0; i < collected; ++i) {
        if (!host.screen_present(i)) {
            continue;
        }
        if (host.screen_visible(i)) {
            host.screen_hide(i);
        }
        host.screen_clear_flags(i);
        host.screen_commit_visibility(i);
    }
    for (std::size_t i = 0; i < collected; ++i) {
        if (!host.screen_present(i)) {
            continue;
        }
        host.screen_delete(i);
    }
    for (std::size_t i = 0; i < kInGameHudScreenCount; ++i) {
        manager.screen_constructed[i] = false;
    }

    host.clear_manager_global();
    host.unload_texture_atlas(kInGameInterfaceAtlas);
    host.close_menu_command_screen();
    host.free_screen_vector();

    if (manager.ambient_sound_instance != 0) {
        host.release_ambient_instance();
        manager.ambient_sound_instance = 0;
    }
    if (manager.ambient_sound_source != 0) {
        host.release_ambient_source();
        manager.ambient_sound_source = 0;
    }
    host.destroy_base();
}

void delete_in_game_interface_0068bc60(InGameInterfaceManager& manager,
    InMissionInterfaceTeardownHost& host, unsigned int flags) {
    destruct_in_game_interface_0068b630(manager, host);
    if ((flags & kScalarDeletingDestructorFreeBit) != 0u) {
        host.free_manager_memory();
    }
}

// ---------------------------------------------------------------------------
// 5. The mission exit
// ---------------------------------------------------------------------------

namespace {
const MissionExitInterfaceStep kMissionExitInterfaceSteps[kMissionExitInterfaceStepCount] = {
    {0x004DAB53u,
        "clears the front-end interface lock 00E19894 while the manager global is non-null",
        true},
    {0x004DAB82u,
        "calls the manager's virtual +00h with flags = 1, the scalar deleting destructor 0068BC60",
        true},
    {0x004DAB94u, "nulls 00E198C4 inside the same guarded block", true},
};
}  // namespace

const MissionExitInterfaceStep& mission_exit_interface_step(std::size_t index) noexcept {
    const std::size_t clamped = index < kMissionExitInterfaceStepCount
        ? index
        : kMissionExitInterfaceStepCount - 1;
    return kMissionExitInterfaceSteps[clamped];
}

}  // namespace bsp
