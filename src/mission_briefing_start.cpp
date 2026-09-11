#include "bsp/mission_briefing_start.hpp"

namespace bsp {
namespace {

// 0058BED0 and 005C56E2: the low byte of the second side block's enabled dword,
// which the loading-screen configuration carries as its third field.
bool second_side_enabled(const MissionRecord& record) noexcept
{
    return static_cast<std::uint8_t>(record.sides[1].enabled & 0xFFu) != 0;
}

// The host-method list of mission_briefing_host_steps. Call sites are the
// native addresses of the CALL or of the store the method stands for.
constexpr MissionBriefingHostStep kHostSteps[] = {
    // 005922F0, the play action of the mission-detail page.
    {0x0059230D, 0x004E1CA0, "MissionBriefingPlayHost", "flush_award_tracker", "session"},
    {0x00592314, 0x00690CD0, "MissionBriefingPlayHost", "flush_award_tracker", "session"},
    {0x0059231B, 0x005806A0, "MissionBriefingPlayHost", "(the selected record)", "pure"},
    {0x0059232F, 0x005830A0, "MissionBriefingPlayHost", "suspend_page_for_movie", "gui"},
    {0x0059233C, 0x00A85C00, "MissionBriefingPlayHost", "stop_front_end_audio", "audio"},
    {0x0059234C, 0x00000000, "MissionBriefingPlayHost", "apply_pending_interface", "gui"},
    {0x0059235B, 0x00000000, "MissionBriefingPlayHost", "arm_movie_surface", "gui"},
    {0x00592361, 0x004F83B0, "MissionBriefingPlayHost", "commit_movie_visibility", "gui"},
    {0x0059236D, 0x00000000, "MissionBriefingPlayHost", "enter_movie_surface", "gui"},
    {0x00592383, 0x004F8A20, "MissionBriefingPlayHost", "play_mission_movie", "movie"},
    {0x0059238E, 0x00000000, "MissionBriefingPlayHost", "mark_movie_active", "movie"},
    {0x0059239C, 0x004F8970, "MissionBriefingPlayHost", "set_movie_completion", "movie"},
    {0x005923B3, 0x0058BDF0, "MissionBriefingPlayHost", "start_selected_mission", "pure"},
    {0x0059244C, 0x0054B530, "MissionBriefingPlayHost", "clear_command_bar", "gui"},

    // 0058BDF0, the start itself.
    {0x0058BE40, 0x0041DD40, "MissionStartHost", "set_current_mission_key", "game state"},
    {0x0058BE70, 0x004E2770, "MissionStartHost", "set_pending_scene", "scene graph"},
    {0x0058BEDE, 0x0057D060, "MissionStartHost", "publish_loading_config", "gui"},
    {0x0058BEFA, 0x0041DD40, "MissionStartHost", "set_mission_key_mirror", "game state"},
    {0x0058BF21, 0x00626930, "MissionStartHost", "reset_mission_stats", "pure"},
    {0x0058BF48, 0x00000000, "MissionStartHost", "main_menu_flag_5c", "gui"},
    {0x0058BF52, 0x00000000, "MissionStartHost", "chosen_difficulty", "game state"},
    {0x0058BF37, 0x00000000, "MissionStartHost", "set_effective_difficulty", "game state"},
    {0x0058BF79, 0x007F8D60, "MissionStartHost", "checkpoint_differs", "profile"},
    {0x0058BF89, 0x00437C70, "MissionStartHost", "write_checkpoint", "profile"},
    {0x0058BF8E, 0x00439020, "MissionStartHost", "request_mission_start", "game state"},
    {0x0058BFA0, 0x00A92C40, "MissionStartHost", "reset_front_end_timer", "input"},
    {0x0058BFA5, 0x00000000, "MissionStartHost", "metrics_enabled", "metrics"},
    {0x0058BFB4, 0x00000000, "MissionStartHost", "non_campaign_session", "game state"},
    {0x0058BFCB, 0x00000000, "MissionStartHost", "current_page", "gui"},
    {0x0058BFDE, 0x00753810, "MissionStartHost", "report_mission_start_metrics", "metrics"},
    {0x0058BFEF, 0x004D2A80, "MissionStartHost", "finish_start", "gui"},

    // 00626930, run from inside the start.
    {0x0062694D, 0x0041DD40, "MissionStatsResetHost", "set_stats_mission_name", "scoring"},
    {0x00626985, 0x0041DD40, "MissionStatsResetHost", "set_stats_debriefing_text", "scoring"},
    {0x006269A9, 0x004954F0, "MissionStatsResetHost", "clear_stats_container", "scoring"},
    {0x00626A81, 0x005F6190, "MissionStatsResetHost", "clear_stats_container", "scoring"},
    {0x00626A9A, 0x006226F0, "MissionStatsResetHost", "clear_stats_container", "scoring"},
};

} // namespace

// ---------------------------------------------------------------------------
// 00626930
// ---------------------------------------------------------------------------

void run_mission_stats_reset_00626930(
    const MissionRecordData& record, MissionStatsResetHost& host)
{
    // 00626938: LEA ESI,[EDI+8]; the guard against self-assignment compares the
    // source with the destination global, which never holds for a record.
    host.set_stats_mission_name(record.screen.title);
    // 00626970: LEA ESI,[EDI+38h].
    host.set_stats_debriefing_text(record.extra.debriefing_text);
    for (const MissionStatsContainer& container : kMissionStatsContainers) {
        host.clear_stats_container(container);
    }
}

// ---------------------------------------------------------------------------
// 00580940
// ---------------------------------------------------------------------------

MainMenuMissionSelection publish_main_menu_mission_selection_00580940(
    const MainMenuSelectionInputs& inputs) noexcept
{
    MainMenuMissionSelection out;
    // 00580945: the whole body is inside `if (game+1FE4h == 0)`.
    if (inputs.non_campaign_session) {
        return out;
    }
    out.published = true;
    out.group_index = inputs.selected_group;   // 00580960
    out.mission_index = inputs.mission_index;  // 00580982
    out.sub_selection = -1;                    // 00580987

    if (inputs.selected_group == 0) {          // 00580994
        out.page = MainMenuPage::Page08;
        return out;
    }
    if (inputs.group_completed) {              // 005809B6
        out.page = MainMenuPage::SinglePlayer;
        return out;
    }
    // 005809D7 and 005809DC pick the pair; 005809EF and 00580A0B are the
    // NEG/SBB/ADD form of `base - (side_index != 0)`, so groups 3 and 4 take
    // the 6/7 pair and everything else the 4/5 pair.
    const int base = (inputs.selected_group == 3 || inputs.selected_group == 4) ? 7 : 5;
    const int page = base - (inputs.side_index != 0 ? 1 : 0);
    out.page = static_cast<MainMenuPage>(page);
    return out;
}

const MissionRecordData* selected_mission_005806a0(const MissionTreeTables& tables,
    std::uint32_t group_index, std::uint32_t mission_index) noexcept
{
    // 005806B3 and 005806E6: both indices are bound-checked against the
    // container extents before the element address is formed.
    if (group_index >= tables.groups.size()) {
        return nullptr;
    }
    const MissionGroupData& group = tables.groups[group_index];
    if (mission_index >= group.missions.size()) {
        return nullptr;
    }
    return &group.missions[mission_index];
}

std::size_t mission_side_index_005c27e0(const MissionRecordData& record) noexcept
{
    return mission_side_index(record.screen);
}

bool group_completed_005c3be0(const MissionGroupData& group,
    const std::vector<bool>& mission_completed) noexcept
{
    for (std::size_t i = 0; i < group.missions.size(); ++i) {
        if (i >= mission_completed.size() || !mission_completed[i]) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// 005922F0 and 0058D9D0
// ---------------------------------------------------------------------------

bool run_briefing_play_005922f0(
    const MissionRecordData& record, MissionBriefingPlayHost& host)
{
    host.flush_award_tracker();

    // 00592325: the test is on the size dword of the MovieName string at +58h.
    if (record.extra.movie_name.empty()) {
        host.start_selected_mission();
        host.clear_command_bar();
        return false;
    }

    host.suspend_page_for_movie();
    host.stop_front_end_audio();
    host.apply_pending_interface();
    host.arm_movie_surface();
    host.commit_movie_visibility();
    host.enter_movie_surface();
    host.play_mission_movie(record.extra.movie_name);
    host.mark_movie_active();
    host.set_movie_completion(kMissionMovieCompletionAddress);
    return true;
}

void run_briefing_movie_finished_0058d9d0(
    const MissionRecordData& record, MissionStartHost& host)
{
    // 0058D9D0 loads ECX with [00E198AC]+58h and falls into 0058BDF0, which
    // never reads it.
    run_start_selected_mission_0058bdf0(record, host);
}

// ---------------------------------------------------------------------------
// 0058BDF0
// ---------------------------------------------------------------------------

MissionStartOutcome run_start_selected_mission_0058bdf0(
    const MissionRecordData& record, MissionStartHost& host)
{
    MissionStartOutcome outcome;
    const std::size_t side = mission_side_index_005c27e0(record);

    host.set_current_mission_key(record.screen.name);
    // 005C5682's twin: the override name is null here as well.
    host.set_pending_scene(mission_scene_name(record.screen), std::string_view{});
    host.publish_loading_config(record.screen.sides[side].loading_text,
        record.screen.title, second_side_enabled(record.screen));
    host.set_mission_key_mirror(record.screen.name);
    host.reset_mission_stats(record);

    // 0058BF26: the record's own difficulty wins unless it is the "ask the
    // player" value, and then only when the main-menu flag is clear. When the
    // flag is set and the record asks, game+6ACh keeps whatever it held - the
    // one place this routine differs from 005C5600's launch arm.
    if (record.screen.difficulty == kMissionDifficultyFromPlayer) {
        if (!host.main_menu_flag_5c()) {
            outcome.effective_difficulty = host.chosen_difficulty();
            host.set_effective_difficulty(outcome.effective_difficulty);
            outcome.difficulty_written = true;
        }
    } else {
        outcome.effective_difficulty = static_cast<std::int32_t>(record.screen.difficulty);
        host.set_effective_difficulty(outcome.effective_difficulty);
        outcome.difficulty_written = true;
    }

    // 0058BF67: the same flag guards the checkpoint write.
    if (!host.main_menu_flag_5c() && host.checkpoint_differs(record)) {
        host.write_checkpoint();
        outcome.checkpoint_written = true;
    }

    host.request_mission_start();
    host.reset_front_end_timer();

    if (host.metrics_enabled() && !host.non_campaign_session()) {
        outcome.metrics_code = host.current_page() == MainMenuPage::Page08
            ? kMissionStartMetricsCodePage08
            : kMissionStartMetricsCodeDefault;
        host.report_mission_start_metrics(outcome.metrics_code, host.main_menu_flag_5c());
        outcome.metrics_reported = true;
    }

    host.finish_start();
    return outcome;
}

// ---------------------------------------------------------------------------
// 0051B7B0
// ---------------------------------------------------------------------------

void run_briefing_activate_play_0051b7b0(bool start_now, BriefingActivatePlayHost& host)
{
    const void* const active = host.active_widget();
    // The three tests are independent: the native code does not stop at the
    // first match, so a table with a repeated handle would store twice.
    for (std::size_t i = 0; i < kBriefingDifficultyWidgetCount; ++i) {
        if (host.difficulty_widget(i) == active) {
            host.set_difficulty_pair(static_cast<std::int32_t>(i));
        }
    }
    host.clear_active_page();
    host.refresh_widget_visibility();
    host.clear_item_disabled_byte();
    if (start_now) {
        host.request_mission_start();
    } else {
        host.rebuild_help_line();
    }
    host.reset_front_end_timer();
}

// ---------------------------------------------------------------------------

const MissionBriefingHostStep* mission_briefing_host_steps(std::size_t& count) noexcept
{
    count = sizeof(kHostSteps) / sizeof(kHostSteps[0]);
    return kHostSteps;
}
}
