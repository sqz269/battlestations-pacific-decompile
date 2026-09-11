#include "bsp/mission_load_path.hpp"

#include <algorithm>

namespace bsp {
namespace {

// 00439020, in push order at 00439026 and 00439033.
constexpr std::uint32_t kMissionStartRequests[2]
    = {kMissionStartInterfaceRequest, kMissionSceneLoadRequest};

} // namespace

std::size_t scene_slot_record_offset(std::size_t index) noexcept
{
    // 004bb169..004bb1c3 spells the eight addresses out rather than indexing.
    return kSceneSlotRecordBase + index * kSceneSlotRecordStride;
}

std::size_t participant_record_offset(std::size_t index) noexcept
{
    // 004bb446 then 004bb458: ECX+748h stepped by 118h.
    return kParticipantRecordBase + index * kSceneSlotRecordStride;
}

// 004c6933..004c6967. This is NOT the rule 004dfb70 uses at 004e087b: the
// relaxed arm here tests `slot == 9` and keeps it, where 004e0896 tests the same
// value and falls into the store of 8. Slot 9 therefore survives into
// game+2028h while the load's own local copy of it becomes 8.
static std::int32_t select_scene_record_script_slot(
    std::int32_t script_slot, bool script_slot_forced, std::int32_t session_mode) noexcept
{
    if (script_slot_forced) {
        return script_slot;
    }
    if (session_mode != 0) {
        return script_slot;
    }
    if (script_slot == 9 || script_slot == 8) {
        return script_slot;
    }
    return 8;
}

SelectSceneRecordResult run_select_scene_record_004c6890(
    const std::vector<SceneRecord>& records, std::int32_t index,
    const SelectSceneRecordInputs& inputs)
{
    SelectSceneRecordResult result;

    // 004c6897..004c68a9. The clamp compares `count < index`, so index == count
    // is accepted; the walk below then runs off the end of the list and the
    // native takes the tail's record instead of failing.
    const std::int32_t count = static_cast<std::int32_t>(records.size());
    if (count == 0 || index < 0 || count < index) {
        index = 0;
    }
    result.selected_index = index; // game+60Ch

    // 004c68b6..004c68e0. The walk stops early at the tail and keeps it.
    const SceneRecord* record = nullptr;
    if (!records.empty()) {
        const std::size_t position
            = std::min<std::size_t>(static_cast<std::size_t>(index), records.size() - 1);
        record = &records[position];
    }

    // 004c68e3: the override string at game+600h/604h is emptied whether or not
    // a record was found.
    result.have_record = record != nullptr;
    if (record == nullptr) {
        return result; // 004c6911, the early return
    }

    // 004c691c. Only the low byte of the mission id reaches game+2015h.
    result.mission_id_byte = static_cast<std::uint8_t>(record->mission_id & 0xff);

    result.resolved_script_slot = select_scene_record_script_slot(
        inputs.script_slot, inputs.script_slot_forced, inputs.session_mode);

    // 004c696d..004c69ed. Session mode 1 and the 00f8a2fc vtable +198h predicate
    // gate a pair of publishing calls; the reconstruction records that they ran
    // rather than modelling the leaderboard object.
    result.leaderboard_published = inputs.session_mode == 1;

    // 004c69f8..004c6aa8. The loop bound is the record's own side-block count,
    // not eight, and the native does not clamp it; the record layout caps it at
    // eight because the ninth block would overlap the scene path at +90Ch.
    const std::size_t supplied
        = std::min<std::size_t>(record->side_blocks.size(), kSceneSlotRecordCount);
    for (std::size_t i = 0; i < supplied; ++i) {
        SceneSlotRecord& slot = result.slots[i];
        slot.in_use = true;                            // 004c6a44
        slot.selector = record->side_blocks[i].selector;   // 004c6a52, block +4h
        slot.secondary = record->side_blocks[i].secondary; // 004c6a63, block +8h
        slot.name.clear();        // 004c6a72, strcpy of the literal at 00ce3a0c
        slot.second_name.clear(); // 004c6a84
    }
    result.slots_filled = supplied;

    // 004c6ab0..004c6adb. Every remaining slot has only its in-use byte cleared;
    // the rest of the 118h record keeps whatever the previous mission left.
    for (std::size_t i = supplied; i < kSceneSlotRecordCount; ++i) {
        result.slots[i].in_use = false;
    }

    // 004c6add..004c6af2, 004bc890(game, game+614h, 0). The raw slot, not the
    // resolved one.
    result.network_slot_refresh = inputs.session_mode != 0;
    return result;
}

bool run_build_scene_record_004e1d70(std::vector<SceneRecord>& records,
    const std::string& scene_path, SceneRecordBuildArm arm,
    const std::string& weather_override, SetPendingSceneHost& host)
{
    // 004e1da9..004e1e0c. The block name is the prefix at 00ce7f0c concatenated
    // with the short name; derive_scene_short_name is the same routine 004dfb70
    // keys its numbered blocks on.
    host.enter_file_block(derive_scene_short_name(scene_path));

    bool created = false;
    if (arm == SceneRecordBuildArm::ReuseExisting) {
        // 004e1e9e..004e1f2c. The duplicate test walks every existing record and
        // compares its +910h path against the argument with a plain byte loop;
        // a match returns 0 without touching the list.
        for (const SceneRecord& existing : records) {
            if (existing.scene_path == scene_path) {
                host.leave_file_block();
                return false;
            }
        }
        SceneRecord record;
        record.scene_path = scene_path;
        // 004e1fbd then 004e20c2: the VFS probe, and the header pass only when
        // the file is there.
        record.file_present = host.scene_file_exists(scene_path);
        if (record.file_present) {
            host.load_scene_header_pass(scene_path, weather_override, record);
        }
        records.push_back(record); // 004e20df, the hand-built 0xc node
        created = true;
    } else {
        // 004e2143..004e21cd. No duplicate test, no VFS probe, and the header
        // pass runs unconditionally.
        SceneRecord record;
        record.scene_path = scene_path;
        host.load_scene_header_pass(scene_path, weather_override, record);
        records.push_back(record); // 004e21c1, the container push_back 004c2c80
        host.notify_award_tracker(); // 004e21c6 then 004e21cd
        created = true;
    }

    host.leave_file_block(); // 004e21de
    return created;
}

SetPendingSceneResult run_set_pending_scene_004e2770(std::vector<SceneRecord>& records,
    const std::string& scene_path, const std::string& weather_override,
    const SelectSceneRecordInputs& inputs, SetPendingSceneHost& host)
{
    SetPendingSceneResult result;

    // 004e277a, 004bf930 with ECX = game+5F0h: pop every node, destroy the
    // record through its virtual +0h with 1, free the node.
    for (SceneRecord& record : records) {
        host.destroy_scene_record(record);
    }
    result.records_destroyed = records.size();
    records.clear();

    // 004e278b and 004e2795: game+5FCh is nulled and the override string at
    // game+600h/604h is emptied before the record is built.

    // 004e27c2. The arm argument is a literal 0 at 004e27bd.
    result.record_created = run_build_scene_record_004e1d70(
        records, scene_path, SceneRecordBuildArm::Create, weather_override, host);

    // 004e27cb, 004c6890(game, 0).
    result.selection = run_select_scene_record_004c6890(records, 0, inputs);
    return result;
}

const std::uint32_t* mission_start_state_requests_00439020(std::size_t& count) noexcept
{
    count = 2;
    return kMissionStartRequests;
}

MissionTreeStartDecision decide_mission_tree_start_005c5600(
    const MissionTreeSelection& selection) noexcept
{
    MissionTreeStartDecision decision;

    // 005c562d..005c5633: XOR EAX,EAX / CMP byte [record+0B8h],BL / SETZ AL.
    decision.side_index = selection.side_flag == 0 ? 1 : 0;

    // 005c573a: CMP dword [EBP+4],EBX with EBP = record + 0B8h + side*154h.
    if (selection.briefing_present) {
        decision.show_briefing = true;
        decision.stored_mode = 0; // game+6ACh is not written on this arm
        return decision;
    }

    // 005c5769..005c5793. Mode 3 means "take the value already in game+6B0h".
    decision.stored_mode
        = selection.mode == kMissionModeInherit ? selection.inherited_mode : selection.mode;
    decision.queue_mission_start = true;
    return decision;
}

const char* mission_load_stage_name(MissionLoadStage stage) noexcept
{
    switch (stage) {
    case MissionLoadStage::Idle: return "Idle";
    case MissionLoadStage::SceneSelected: return "SceneSelected";
    case MissionLoadStage::Requested: return "Requested";
    case MissionLoadStage::Loading: return "Loading";
    case MissionLoadStage::SceneReady: return "SceneReady";
    case MissionLoadStage::InMission: return "InMission";
    }
    return "Idle";
}

MissionLoadStage mission_load_stage_for_state(std::uint32_t game_state,
    bool requests_pending, bool have_pending_scene) noexcept
{
    if (game_state == kGameStateSceneReady) {
        return MissionLoadStage::SceneReady;
    }
    if (game_state == static_cast<std::uint32_t>(GameStateId::kInMission)) {
        return MissionLoadStage::InMission;
    }
    if (requests_pending) {
        return MissionLoadStage::Requested;
    }
    if (have_pending_scene) {
        return MissionLoadStage::SceneSelected;
    }
    return MissionLoadStage::Idle;
}

MissionTreeStartDecision start_mission_from_tree(MissionLoadPathState& state,
    const MissionTreeSelection& selection, const SelectSceneRecordInputs& inputs,
    SetPendingSceneHost& host)
{
    // 005c5684. The mission tree pushes a null override, so the weather
    // descriptor is chosen by the reader from Weather.lua rather than forced.
    const SetPendingSceneResult pending = run_set_pending_scene_004e2770(
        state.records, selection.scene_path, std::string(), inputs, host);
    state.selection = pending.selection;
    state.have_pending_scene = pending.selection.have_record;

    const MissionTreeStartDecision decision = decide_mission_tree_start_005c5600(selection);
    if (decision.queue_mission_start) {
        std::size_t count = 0;
        const std::uint32_t* requests = mission_start_state_requests_00439020(count);
        for (std::size_t i = 0; i < count; ++i) {
            state.requests.push_back(requests[i]);
        }
    }
    return decision;
}

MissionLoadFrameResult advance_mission_load_path(MissionLoadPathState& state,
    MissionLoadPathFrame& frame)
{
    MissionLoadFrameResult result;

    // 004e4430 re-reads the queue count after every dispatch, so 6 and 0Ah land
    // in the same pass. Only the two requests this path owns are consumed here;
    // anything else is left for the frame-control driver.
    while (!state.requests.empty()) {
        const std::uint32_t request = state.requests.front();
        if (request == kMissionStartInterfaceRequest) {
            state.requests.erase(state.requests.begin());
            continue; // the interface change belongs to the front-end owner
        }
        if (request != kMissionSceneLoadRequest && request != kMissionSceneReloadRequest) {
            break;
        }
        state.requests.erase(state.requests.begin());
        if (frame.scene_load == nullptr || frame.scene_load_host == nullptr) {
            break;
        }
        // The drain writes the dequeued request into game+5D4h before dispatch,
        // which is where 004dfdd5 reads it to pick the block label.
        frame.scene_load->state = request;
        run_mission_scene_load(*frame.scene_load, *frame.scene_load_host);
        result.ran_scene_load = true;
        state.have_pending_scene = false;
    }

    const std::uint32_t game_state
        = frame.scene_load != nullptr ? frame.scene_load->state : 0u;

    // 004e4a4d dispatches 004db920 only while game+5D4h is 0Ch, and only on a
    // frame that did not just run the load: 004dfb70 returns with 0Ch set and
    // the frame loop reaches the state handler on the next pass.
    if (!result.ran_scene_load && game_state == kGameStateSceneReady
        && frame.device_wait_host != nullptr) {
        result.ran_device_wait = true;
        const bool entered = run_mission_device_wait(
            frame.device_wait_inputs, state.device_wait_latch, *frame.device_wait_host);
        if (entered && frame.state_entry != nullptr && frame.state_entry_host != nullptr
            && frame.audio_settings != nullptr) {
            // 004dba96 tail-calls 004da6c0, which is what writes 0Dh.
            run_mission_state_entry(
                *frame.state_entry, *frame.audio_settings, *frame.state_entry_host);
            if (frame.scene_load != nullptr) {
                frame.scene_load->state = frame.state_entry->game_state;
            }
            result.entered_mission
                = frame.state_entry->game_state
                == static_cast<std::uint32_t>(GameStateId::kInMission);
        }
    }

    const std::uint32_t final_state
        = frame.scene_load != nullptr ? frame.scene_load->state : 0u;
    result.stage = mission_load_stage_for_state(
        final_state, !state.requests.empty(), state.have_pending_scene);
    return result;
}

const char* mission_load_owner_name(MissionLoadOwner owner) noexcept
{
    switch (owner) {
    case MissionLoadOwner::PureLogic: return "pure";
    case MissionLoadOwner::Vfs: return "vfs";
    case MissionLoadOwner::Renderer: return "renderer";
    case MissionLoadOwner::SceneGraph: return "scene-graph";
    case MissionLoadOwner::World: return "world";
    case MissionLoadOwner::Lua: return "lua";
    case MissionLoadOwner::Audio: return "audio";
    case MissionLoadOwner::Input: return "input";
    case MissionLoadOwner::Gui: return "gui";
    case MissionLoadOwner::Session: return "session";
    }
    return "pure";
}

namespace {

// Call order of every host method the load needs, from the mission-tree request
// to the first frame of the mission. The order is the order the listings reach
// the call sites; the owner column says whether the rebuilt executable can
// satisfy the method from reconstructed logic alone.
constexpr MissionLoadHostStep kHostSteps[] = {
    // --- selection, 005c5600 and 004e2770 -----------------------------------
    {0x005c3870, "MissionTreeScreen", "selected_mission_record", MissionLoadOwner::Gui,
        "the record 005c5600 reads every field below out of"},
    {0x004bf930, "SetPendingSceneHost", "destroy_scene_record", MissionLoadOwner::PureLogic,
        "record virtual +0h with 1, then the node is freed"},
    {0x00be0a30, "SetPendingSceneHost", "enter_file_block", MissionLoadOwner::Vfs,
        "BSP_VFS_EnterFileBlock, name = 00ce7f0c prefix + short name"},
    {0x0109ceec, "SetPendingSceneHost", "scene_file_exists", MissionLoadOwner::Vfs,
        "provider table virtual +8h; reuse arm only"},
    {0x0046df00, "SetPendingSceneHost", "load_scene_header_pass", MissionLoadOwner::SceneGraph,
        "SceneFilePass::Header; fills the record this path then selects"},
    {0x0068ea00, "SetPendingSceneHost", "notify_award_tracker", MissionLoadOwner::Session,
        "004e1ca0 singleton, create arm only"},
    {0x00bdcb30, "SetPendingSceneHost", "leave_file_block", MissionLoadOwner::Vfs, ""},
    {0x004c6890, "-", "run_select_scene_record_004c6890", MissionLoadOwner::PureLogic,
        "reconstructed here; fills the eight slot records"},
    {0x0057d060, "MissionTreeScreen", "loading_screen_set_mission_text", MissionLoadOwner::Gui,
        "record+8h, the side block hints at +64h and the byte at +20Ch"},
    {0x00439020, "-", "mission_start_state_requests_00439020", MissionLoadOwner::PureLogic,
        "queues 6 then 0Ah through 004d7920"},

    // --- the load, 004dfb70, in listing order -------------------------------
    {0x004dfba3, "MissionSceneLoadHost", "pending_time_scale", MissionLoadOwner::Audio, ""},
    {0x00a7a440, "MissionSceneLoadHost", "apply_audio_time_scale", MissionLoadOwner::Audio, ""},
    {0x004cd0f0, "MissionSceneLoadHost", "set_cinematic_mode", MissionLoadOwner::PureLogic,
        "already reconstructed by the simulation-gate packet"},
    {0x004dfbf0, "MissionSceneLoadHost", "reset_frame_pacing_globals", MissionLoadOwner::PureLogic,
        ""},
    {0x0076da60, "MissionSceneLoadHost", "session_reset", MissionLoadOwner::Session, ""},
    {0x004bb160, "MissionSceneLoadHost", "reset_single_player_slots", MissionLoadOwner::PureLogic,
        "points game+18CCh at the eight records at game+1008h; 004bb440 claims one"},
    {0x004cec60, "MissionSceneLoadHost", "reset_network_slots", MissionLoadOwner::Session, ""},
    {0x00e198ac, "MissionSceneLoadHost", "release_main_menu_manager", MissionLoadOwner::Gui, ""},
    {0x00e198b4, "MissionSceneLoadHost", "release_secondary_menu_manager", MissionLoadOwner::Gui,
        ""},
    {0x006878f0, "MissionSceneLoadHost", "detach_network_menu_manager", MissionLoadOwner::Session,
        ""},
    {0x00be0a30, "MissionSceneLoadHost", "enter_file_block", MissionLoadOwner::Vfs,
        "loading_screen_scene or loading_screen_reload"},
    {0x0057d0c0, "MissionSceneLoadHost", "publish_loading_screen_config", MissionLoadOwner::Gui,
        ""},
    {0x0057cb60, "MissionSceneLoadHost", "begin_loading_screen", MissionLoadOwner::Gui,
        "mode 1, LoadingImage; starts the render worker that animates the screen"},
    {0x004dfe83, "MissionSceneLoadHost", "release_mission_result", MissionLoadOwner::PureLogic, ""},
    {0x004dc6a0, "MissionSceneLoadHost", "global_subsystems", MissionLoadOwner::World,
        "game+21D0h, game+21E4h, 00F88C30, game+21E0h"},
    {0x00874640, "MissionSceneLoadHost", "reset_render_scene", MissionLoadOwner::Renderer, ""},
    {0x006ad600, "MissionSceneLoadHost", "reset_effect_atlas", MissionLoadOwner::Renderer, ""},
    {0x0046df00, "MissionSceneLoadHost", "load_scene_file", MissionLoadOwner::SceneGraph,
        "SceneFilePass::Header again, this time against the live database"},
    {0x004de610, "MissionSceneLoadHost", "construct_world", MissionLoadOwner::World,
        "game+19CCh and game+21D4h, the ocean and the sky"},
    {0x00951560, "MissionSceneLoadHost", "reset_shader_globals", MissionLoadOwner::Renderer, ""},
    {0x005e2f00, "MissionSceneLoadHost", "sync_lobby_settings_from_lua", MissionLoadOwner::Lua, ""},
    {0x004f2800, "MissionSceneLoadHost", "resolve_named_scene_objects", MissionLoadOwner::SceneGraph,
        "the 26-class registration table"},
    {0x0095ba60, "MissionSceneLoadHost", "reset_slot_cameras", MissionLoadOwner::Renderer, ""},
    {0x004d4df0, "MissionSceneLoadHost", "load_scene_contents", MissionLoadOwner::SceneGraph,
        "the Registration and Instantiate passes; this is where the entities appear"},
    {0x007fa2d0, "MissionSceneLoadHost", "activate_slot", MissionLoadOwner::Session, ""},
    {0x004c3840, "MissionSceneLoadHost", "assign_party_player_slots", MissionLoadOwner::Session, ""},
    {0x0068a990, "MissionSceneLoadHost", "create_hud_manager", MissionLoadOwner::Gui,
        "operator new(0x108) into 00E198C4"},
    {0x004c9680, "MissionSceneLoadHost", "reset_hud_layout", MissionLoadOwner::Gui, ""},
    {0x004c1ac0, "MissionSceneLoadHost", "select_front_end_layout", MissionLoadOwner::Gui, ""},
    {0x008053c0, "MissionSceneLoadHost", "prime_view", MissionLoadOwner::Renderer, ""},
    {0x008073c0, "MissionSceneLoadHost", "bind_local_view", MissionLoadOwner::Renderer, ""},
    {0x00aa0d30, "MissionSceneLoadHost", "register_locale_table", MissionLoadOwner::PureLogic,
        "names split off record+980h"},
    {0x00aa06d0, "MissionSceneLoadHost", "reload_locale_tables", MissionLoadOwner::Vfs, ""},
    {0x004218e0, "MissionSceneLoadHost", "reset_objective_list", MissionLoadOwner::PureLogic, ""},
    {0x00a92c40, "MissionSceneLoadHost", "input_update", MissionLoadOwner::Input, ""},
    {0x0075b430, "MissionSceneLoadHost", "dispatch_session_ready_event", MissionLoadOwner::Session,
        "session mode 2 only"},
    {0x0077f5e0, "MissionSceneLoadHost", "commit_scene_ready", MissionLoadOwner::Session,
        "immediately before game+5D4h = 0Ch at 004e086b"},
    {0x004d30f0, "MissionSceneLoadHost", "rebuild_scripted_name_list", MissionLoadOwner::Lua, ""},
    {0x008860b0, "MissionSceneLoadHost", "run_mission_script", MissionLoadOwner::Lua,
        "Scripts/missions/<name>.lua, name from record+928h + slot*8"},
    {0x0045f520, "MissionSceneLoadHost", "lua_call_entry_point_a", MissionLoadOwner::Lua,
        "luaPrecacheUnits then luaStageInitMulti"},
    {0x0045f440, "MissionSceneLoadHost", "lua_call_entry_point_b", MissionLoadOwner::Lua,
        "luaStageInit, or luaEngineMovieInit on raw slot 9"},
    {0x004e0a9b, "MissionSceneLoadHost", "precache_units", MissionLoadOwner::SceneGraph,
        "the eleven calls between 004e0a9b and 004e0acd"},
    {0x00f8d394, "MissionSceneLoadHost", "renderer_scene_ready", MissionLoadOwner::Renderer,
        "vtable +E4h, inside the 6_ block"},
    {0x007065e0, "MissionSceneLoadHost", "load_warning_bank", MissionLoadOwner::Audio, ""},
    {0x00f8a2fc, "MissionSceneLoadHost", "publish_mission_id", MissionLoadOwner::Session,
        "+48h from record+1098h"},
    {0x0057c250, "MissionSceneLoadHost", "end_loading_screen", MissionLoadOwner::Gui, ""},
    {0x004c9ca0, "MissionSceneLoadHost", "apply_in_game_interface", MissionLoadOwner::Gui,
        "with 1; 004da6c0 calls the same routine with 0"},

    // --- state 0Ch, 004db920 ------------------------------------------------
    {0x00a91020, "MissionDeviceWaitHost", "any_dynamic_device_button_down", MissionLoadOwner::Input,
        ""},
    {0x00425d10, "MissionDeviceWaitHost", "menu_command_screen_busy", MissionLoadOwner::Gui, ""},
    {0x00f8abe8, "MissionDeviceWaitHost", "gui_root_busy", MissionLoadOwner::Gui, ""},
    {0x00a92c40, "MissionDeviceWaitHost", "update_input_manager", MissionLoadOwner::Input, ""},
    {0x0075b430, "MissionDeviceWaitHost", "dispatch_session_event", MissionLoadOwner::Session, ""},

    // --- state 0Dh, 004da6c0 ------------------------------------------------
    {0x00447060, "MissionStateEntryHost", "release_deferred_dynamics", MissionLoadOwner::World, ""},
    {0x004da71e, "MissionStateEntryHost", "mark_local_slot_ready", MissionLoadOwner::PureLogic, ""},
    {0x00a7a440, "MissionStateEntryHost", "set_audio_environment_level", MissionLoadOwner::Audio,
        ""},
    {0x004c9ca0, "MissionStateEntryHost", "apply_in_game_interface", MissionLoadOwner::Gui,
        "with 0"},
    {0x004d87b0, "MissionStateEntryHost", "check_multiplayer_player_count",
        MissionLoadOwner::Session, "can move game+5D4h away from 0Dh"},
    {0x004cd0f0, "MissionStateEntryHost", "set_cinematic_mode", MissionLoadOwner::PureLogic,
        "opens the simulation gate"},
};

} // namespace

const MissionLoadHostStep* mission_load_host_steps(std::size_t& count) noexcept
{
    count = sizeof(kHostSteps) / sizeof(kHostSteps[0]);
    return kHostSteps;
}

std::size_t mission_load_external_step_count() noexcept
{
    std::size_t count = 0;
    const MissionLoadHostStep* steps = mission_load_host_steps(count);
    std::size_t external = 0;
    for (std::size_t i = 0; i < count; ++i) {
        if (steps[i].owner != MissionLoadOwner::PureLogic) {
            ++external;
        }
    }
    return external;
}

} // namespace bsp
