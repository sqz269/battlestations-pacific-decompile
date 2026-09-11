#include "bsp/mission_scene_load.hpp"
#include "bsp/global_subsystems.hpp"

#include <algorithm>
#include <cctype>

namespace bsp {
namespace {

bool equals_ignore_case(const std::string& text, std::size_t offset, const char* literal) noexcept
{
    std::size_t index = 0;
    for (; literal[index] != '\0'; ++index) {
        if (offset + index >= text.size()) {
            return false;
        }
        const unsigned char lhs
            = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(text[offset + index])));
        const unsigned char rhs
            = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(literal[index])));
        if (lhs != rhs) {
            return false;
        }
    }
    return offset + index == text.size();
}

}

const char* mission_scene_load_block_label(std::uint32_t request) noexcept
{
    // 004dfdd5: the state compare picks the label before the block is opened.
    if (request == kMissionSceneLoadRequest) {
        return "loading_screen_scene"; // 00CE7EF0
    }
    if (request == kMissionSceneReloadRequest) {
        return "loading_screen_reload"; // 00CE7ED8
    }
    return "this_will_not_work"; // 00CE7EC4
}

const char* mission_load_phase_prefix(MissionLoadPhase phase) noexcept
{
    switch (phase) {
    case MissionLoadPhase::WorldConstruction:
        return "1_"; // 00CE7EC0
    case MissionLoadPhase::MissionScript:
        return "3_"; // 00CE7E90
    case MissionLoadPhase::StageInit:
        return "4_"; // 00CE4EC8
    case MissionLoadPhase::EngineMovie:
        return "5_"; // 00CE7E78
    case MissionLoadPhase::RendererHandoff:
        return "6_"; // 00CE7E74
    }
    return "";
}

std::string mission_load_phase_block_name(MissionLoadPhase phase, const std::string& short_name)
{
    // BSP_NativeString_Concat (004261a0) with the prefix on the left.
    return std::string(mission_load_phase_prefix(phase)) + short_name;
}

std::string derive_scene_short_name(const std::string& scene_path)
{
    std::string work = scene_path;
    // 004cd85a: BSP_String_ReplaceSubstrings with "\" and "/".
    std::replace(work.begin(), work.end(), '\\', '/');

    // 004cd8a1: rfind('/'); the tail after the separator becomes the name.
    std::string name = work;
    const std::size_t separator = work.find_last_of('/');
    if (separator != std::string::npos) {
        name = work.substr(separator + 1);
    }

    // 004cd97f: the first '_' is only used to start the search for the second
    // one. Truncation happens at the second underscore, when there is one.
    const std::size_t first = name.find('_');
    if (first != std::string::npos) {
        const std::size_t second = name.find('_', first + 1);
        if (second != std::string::npos) {
            name = name.substr(0, second);
        }
    }

    // 004cda65: the trailing four characters are compared with ".scn" through
    // _stricmp and dropped on a match. The native substring clamps, so a name
    // shorter than four characters simply fails the compare.
    if (name.size() >= 4 && equals_ignore_case(name, name.size() - 4, ".scn")) {
        name = name.substr(0, name.size() - 4);
    }
    return name;
}

std::string mission_script_path(const std::string& script_name)
{
    // 008860b0 concatenates in this order: directory, name, extension.
    return std::string(kMissionScriptDirectory) + script_name + kMissionScriptExtension;
}

std::int32_t normalise_mission_script_slot(
    std::int32_t script_slot, bool script_slot_forced, std::int32_t session_mode) noexcept
{
    // 004e087b, transcribed from the listing rather than simplified: the
    // "slot == 9 || slot != 8" term is redundant with the outer disjunction but
    // is what the compare chain evaluates.
    const bool relaxed = !script_slot_forced && session_mode == 0;
    if ((relaxed && (script_slot == 9 || script_slot != 8)) || script_slot == 9) {
        return 8;
    }
    return script_slot;
}

bool mission_uses_engine_movie(std::int32_t script_slot) noexcept
{
    // 004e0a50..004e0a75. Both guard arms reach the same "== 9" test.
    return script_slot == 9;
}

const char* mission_side_suffix(std::int32_t side_selector) noexcept
{
    return side_selector != 0 ? "_Jp" : "_Ally";
}

std::string warnings_bank_name(const std::string& language, std::int32_t side_selector)
{
    return std::string(kWarningsBankPrefix) + language + mission_side_suffix(side_selector);
}

std::string authentic_warnings_bank_name(std::int32_t side_selector)
{
    return std::string(kWarningsAuthenticEnglishBank) + mission_side_suffix(side_selector);
}

bool language_entry_matches(const std::string& current, const std::string& entry) noexcept
{
    // 004e10c0: the empty cases are decided before _stricmp is reached.
    if (current.empty()) {
        return entry.empty();
    }
    if (entry.empty()) {
        return false;
    }
    return equals_ignore_case(entry, 0, current.c_str());
}

std::vector<std::string> split_locale_table_list(const std::string& list)
{
    std::vector<std::string> names;
    if (list.empty()) {
        return names;
    }
    std::size_t start = 0;
    while (true) {
        const std::size_t separator = list.find(kLocaleTableListSeparator, start);
        if (separator == std::string::npos) {
            names.push_back(list.substr(start));
            break;
        }
        names.push_back(list.substr(start, separator - start));
        start = separator + 1;
    }
    return names;
}

void run_mission_scene_load(MissionSceneLoadState& state, MissionSceneLoadHost& host)
{
    const std::uint32_t request = state.state;

    // 004dfb9d..004dfc0e.
    state.scene_reload_latch = false;
    const float pending = host.pending_time_scale();
    if (pending != 0.0f) {
        host.clear_pending_time_scale();
        host.apply_audio_time_scale(pending);
    }
    host.set_cinematic_mode(1, 0, 1);
    host.reset_frame_pacing_globals();
    host.session_reset();

    // 004dfc13. game+1FE4h picks the participant table that is rebuilt.
    if (state.session_mode == 0) {
        host.reset_single_player_slots();
        state.local_slot = 0; // 004dfd77
    } else {
        host.reset_network_slots();
    }

    // 004dfd90..004dfdd0.
    host.release_main_menu_manager();
    if (state.session_mode == 0) {
        host.release_secondary_menu_manager();
    } else {
        host.detach_network_menu_manager();
    }

    // 004dfe3f..004dfe7e. The block is opened around the bring-up only; the
    // screen itself outlives it and is torn down at the end of the load.
    host.enter_file_block(mission_scene_load_block_label(request));
    host.publish_loading_screen_config();
    host.begin_loading_screen(LoadingScreenMode::LoadingImage);
    host.leave_file_block();

    host.release_mission_result(); // 004dfe83
    auto global = host.global_subsystems();
    construct_global_subsystems_004dc6a0(global.state, global.host, global.context); // 004dfebb

    const std::string short_name = derive_scene_short_name(state.scene_path);

    // 004dff7a. This block is a heap FileBlock released through its virtual
    // destructor at 004e0370, not a scoped stack object.
    host.enter_file_block(
        mission_load_phase_block_name(MissionLoadPhase::WorldConstruction, short_name));
    state.scene_resident = false; // 004e0150
    host.reset_render_scene();
    host.reset_effect_atlas();
    host.load_scene_file(state.scene_path, state.scene_override);
    host.construct_world();
    host.reset_shader_globals();
    if (host.lua_global_exists("thisTable")) {
        host.lua_clear_global("thisTable");
    }
    host.lua_reset_state();
    host.lua_declare_global("recon");
    host.resolve_named_scene_objects();
    state.hud_suppressed = false; // 004e0360
    host.leave_file_block();

    // 004e0378..004e0463.
    host.reset_slot_cameras();
    host.load_scene_contents();
    for (int slot = 0; slot < 8; ++slot) {
        if (host.slot_present(slot)) {
            host.activate_slot(slot);
        }
    }
    if (state.session_mode == 1) {
        host.assign_party_player_slots();
    }

    // 004e0463..004e05c0.
    host.create_hud_manager();
    host.enter_file_block(kGvSpaceInitBlock);
    host.reset_hud_layout();
    host.select_front_end_layout(3);
    host.hud_manager_init();
    host.hud_manager_start();
    host.leave_file_block();
    host.publish_side_to_renderer(state.side_selector);
    for (int index = 0; index < 3; ++index) {
        host.prime_view(index);
    }
    if (state.local_slot >= 0 && state.local_slot < 8) {
        host.bind_local_view();
        state.hud_suppressed = false; // 004e05a2
    }

    // 004e05c8..004e0750. The list is only walked when the record supplied one.
    if (!state.locale_table_list.empty()) {
        for (const std::string& name : split_locale_table_list(state.locale_table_list)) {
            host.register_locale_table(name);
        }
        host.reload_locale_tables();
    }

    // 004e0750..004e0870.
    state.objective_counter = 0;
    state.objective_timer = 0.0f;
    host.reset_objective_list();
    host.set_input_capture(true);
    host.input_update(0.0f);
    host.input_update(0.0f);
    host.set_input_capture(false);
    if (state.session_mode == 2) {
        host.dispatch_session_ready_event();
    } else if (state.session_mode != 0) {
        host.mark_local_slot_ready();
    }
    host.commit_scene_ready();
    state.scene_resident = true; // 004e0865
    state.state = kGameStateSceneReady; // 004e086b

    // 004e0870. The script name comes from the normalised slot; the arm below
    // is chosen from the raw slot.
    state.resolved_script_slot
        = normalise_mission_script_slot(state.script_slot, state.script_slot_forced, state.session_mode);
    host.rebuild_scripted_name_list();

    if (!state.script_name.empty()) {
        host.enter_file_block(
            mission_load_phase_block_name(MissionLoadPhase::MissionScript, short_name));
        host.run_mission_script(mission_script_path(state.script_name));
        host.leave_file_block();

        if (mission_uses_engine_movie(state.script_slot)) {
            if (host.lua_entry_point_exists(kLuaEngineMovieInit)) {
                host.enter_file_block(
                    mission_load_phase_block_name(MissionLoadPhase::EngineMovie, short_name));
                host.lua_call_entry_point_b(kLuaEngineMovieInit);
                host.leave_file_block();
            }
        } else {
            host.push_script_reentry_guard(1); // 004e0a75
            host.lua_call_entry_point_a(kLuaPrecacheUnits);
            host.precache_units();
            if (host.lua_script_mode_enabled()) {
                host.lua_collect_garbage();
            }
            host.enter_file_block(
                mission_load_phase_block_name(MissionLoadPhase::StageInit, short_name));
            host.lua_call_entry_point_a(kLuaStageInitMulti);
            host.lua_call_entry_point_b(kLuaStageInit);
            host.leave_file_block();
            host.push_script_reentry_guard(-1); // 004e0e03
        }
    }

    // 004e0e09..004e0f60.
    host.enter_file_block(
        mission_load_phase_block_name(MissionLoadPhase::RendererHandoff, short_name));
    host.renderer_scene_ready();
    host.leave_file_block();
    if (state.session_mode != 2 && host.lua_script_mode_enabled()) {
        host.lua_collect_garbage();
    }

    // 004e0fad. Eviction pass, once per side and spoken-language index: walk the
    // installed languages from the back and, for every entry that is NOT the
    // current one, select it, load its bank and immediately replace it with the
    // placeholder. The current language is skipped and restored afterwards.
    if (host.spoken_language_setting() == 2) {
        const int language_index = host.spoken_language_index();
        if (!host.warnings_loaded_for_side(state.side_selector, language_index)) {
            const std::string current = host.current_language_name();
            const std::vector<std::string> installed = host.installed_languages();
            for (std::size_t index = installed.size(); index-- > 0;) {
                const std::string& entry = installed[index];
                if (language_entry_matches(current, entry)) {
                    continue;
                }
                host.select_voice_language(entry);
                host.enter_file_block(warnings_bank_name(entry, state.side_selector));
                host.load_warning_bank(1);
                host.leave_file_block();
                host.enter_file_block(kWarningsPlaceholderBank);
                host.load_warning_bank(0);
                host.leave_file_block();
            }
            host.mark_warnings_loaded_for_side(state.side_selector, language_index);
            host.select_voice_language(current);
        }
    }

    // 004e1374. Both arms reach this: the bank for the language actually in use.
    {
        const bool authentic = host.content_installed(kAuthenticEnglishContentId);
        const std::string bank = authentic
            ? authentic_warnings_bank_name(state.side_selector)
            : warnings_bank_name(host.current_language_name(), state.side_selector);
        host.enter_file_block(bank);
        host.load_warning_bank(1);
        host.leave_file_block();
    }

    if (state.session_mode == 1 || state.script_slot_forced) {
        host.notify_slots_ready();
    }

    // 004e183d..004e18a2.
    host.publish_mission_id(state.mission_id);
    host.clear_render_gate();
    host.end_loading_screen();
    host.gui_set_enabled(false);
    host.apply_in_game_interface();
    host.input_update(0.0f);
    host.reset_render_scene();
    host.set_cinematic_mode(1, 0, 1);
}
}
