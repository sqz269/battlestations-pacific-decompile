// In-game HUD screens of segment 25 and GUI widgets of segment 79.
// docs/CC13_HUD_AND_WIDGETS.md carries the evidence, ABIs and uncertainty.

#include "bsp/hud_widgets.hpp"

#include <cctype>
#include <cstdlib>

namespace bsp {

namespace {

// _stricmp for the ASCII localisation keys the scoreboard compares.
bool equals_ignore_case(std::string_view a, std::string_view b) noexcept {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

// _atol on the selected row's name: the enter names every row after its slot.
int selected_slot(std::uintptr_t listbox, HudScoringHost& host) {
    return static_cast<int>(std::atol(host.selected_row_name(listbox).c_str()));
}

// The gate shared by the update (0060E4D9..0060E51A) and the selection listener
// (0060E101..0060E17B): a human row, a selected signed-in user (state 2), and
// the platform virtual +19Ch answering false.
bool gamer_card_allowed(const HudScoringPlayer& player, HudScoringHost& host) {
    return !player.ai && host.xenon_has_selected_user_00a3e510() &&
           host.xenon_selected_sign_in_state_00a3ead0() == 2 && !host.platform_virtual_19c();
}

}  // namespace

void hud_scoring_hide_ping_icons_0060ef50(std::uintptr_t connection_group, HudScoringHost& host) {
    for (std::string_view icon : kHudScoringPingIcons) {
        host.set_visible(host.find_child_00aa7e00(connection_group, icon), false);
    }
}

void hud_scoring_layout_0060f210(HudScoringScreenState& screen, HudScoringHost& host) {
    screen.page_44 = host.load_page_00aa5840("GUI_scoring");
    screen.header_group_10 = host.find_child_00aa7e00(screen.page_44, "Header_Group");
    host.set_visible(screen.header_group_10, false);
    screen.listbox_18 = host.find_child_00aa7e00(screen.page_44, "Main_Listbox");
    host.listbox_set_listener_00a9ac40(screen.listbox_18);
    host.listbox_set_input_filter_00a9ac70(screen.listbox_18);
    // 0060F3DA..0060F3F9: the template row is a clone of the listbox's own
    // Player_Group; the clone gets the widget listener at screen+0Ch.
    screen.row_template_14 =
        host.clone_subtree_00aab4c0(host.find_child_00aa7e00(screen.listbox_18, "Player_Group"));
    host.set_listener_fields_00aa6bc0(screen.row_template_14, true);
    host.set_row_flag_78(screen.row_template_14);
    host.set_visible(screen.row_template_14, false);
    hud_scoring_hide_ping_icons_0060ef50(
        host.find_child_00aa7e00(screen.row_template_14, "Connection_Group"), host);
}

void hud_scoring_register_0060d740(HudScoringScreenState& screen, HudScoringHost& host) {
    host.front_end_register_004f71d0();
    hud_scoring_layout_0060f210(screen, host);  // through vtable +14h
    host.set_visible(screen.page_44, false);
}

void hud_scoring_close_0060dd10(HudScoringScreenState& screen, HudScoringHost& host) {
    screen.requested_04 = false;
    if (host.game_online_1fe4()) {
        host.pop_request_interface_006881f0();
    } else {
        host.raise_manager_screen_d8();
    }
    host.input_manager_update_00a92c40();
}

void hud_scoring_exit_0060d980(HudScoringScreenState& screen, HudScoringHost& host) {
    host.listbox_clear_rows_00a9bec0(screen.listbox_18);
    host.command_bar_exit_and_commit();
    if (screen.team0_header_3c != 0) {
        host.destroy_widget(screen.team0_header_3c);
        screen.team0_header_3c = 0;
    }
    if (screen.team1_header_40 != 0) {
        host.destroy_widget(screen.team1_header_40);
        screen.team1_header_40 = 0;
    }
}

HudListboxInput hud_scoring_listbox_input_0060dbc0(HudScoringHost& host) {
    HudListboxInput input;
    input.up = host.action_edge_004d92b0(0x46);
    input.down = host.action_edge_004d92b0(0x47);
    input.select = host.action_edge_004d92b0(0x4A);
    float axis = 0.0f;
    if (host.pad_axis_10(axis)) {
        input.up = input.up || axis < -0.5f;     // 00CE69D0
        if (host.pad_axis_10(axis)) {            // the axis is read a second time
            input.down = input.down || axis > 0.5f;  // 00CE3800
        }
    }
    return input;
}

void hud_scoring_on_selection_changed_0060e0e0(HudScoringScreenState& screen,
    std::uintptr_t listbox, HudScoringHost& host) {
    if (listbox != screen.listbox_18) {
        return;
    }
    const HudScoringPlayer player = host.player(selected_slot(screen.listbox_18, host));
    std::array<MainMenuCommandArgument, 5> commands{{
        {0, "", 1}, {0, "", 1}, {0, "", 1}, {kHudScoringBackCommand, "globals.back", 2}, {0, "", 1}}};
    if (gamer_card_allowed(player, host)) {
        commands[0] = {kHudScoringGamerCardCommand, "globals.showgamercard", 1};
    }
    host.command_bar_rebuild_0054b530(commands);
}

void hud_scoring_button_alpha_0060d9f0(std::uintptr_t widget, bool selected_d4,
    HudScoringHost& host) {
    if (host.widget_kind(widget) == 3) {
        host.set_alpha(widget, selected_d4 ? 0.5f : 1.0f);  // 00CE3800 / 00D7A24C
    }
}

void hud_scoring_on_widget_activated_0060de90(HudScoringScreenState& screen,
    std::uintptr_t widget, std::uint8_t name_first_byte, std::uint32_t row_index_d8,
    HudScoringHost& host) {
    const int kind = host.widget_kind(widget);
    if (kind == 3) {
        host.set_alpha(widget, 1.0f);
        if (name_first_byte == kHudScoringBackCommand) {
            hud_scoring_close_0060dd10(screen, host);
            return;
        }
        if (name_first_byte != kHudScoringGamerCardCommand) {
            return;
        }
        const HudScoringPlayer player = host.player(selected_slot(screen.listbox_18, host));
        // 0060DF27..0060DF4E: this path asks 004B44F0 (signed into Live), not
        // the selected-user sign-in state the update and the listener use.
        if (!player.ai && host.xenon_signed_into_live_004b44f0() && !host.platform_virtual_19c()) {
            host.show_gamer_card(player);
        }
        return;
    }
    if (kind == 2) {
        host.listbox_select_index_00a9c7c0(screen.listbox_18, row_index_d8);
    }
}

namespace {

// One team's rows in the update (0060E55E..0060E8D6 for team 1, 0060E8DB..
// 0060EC8C for team 0): refresh the points text, sum the team total and drive
// the mic icon.
int refresh_team_rows(const std::vector<HudScoreRow>& rows, HudScoringHost& host) {
    int total = 0;
    for (const HudScoreRow& row : rows) {
        const HudScoringPlayer player = host.player(row.slot);
        if (player.score_index == -1) {
            continue;
        }
        const int score = host.score_by_index(player.score_index);
        host.set_text(host.find_child_00aa7e00(row.row_widget, "Player_point_Text"),
            std::to_string(score), true);
        total += score;
        const std::uintptr_t mic = host.find_child_00aa7e00(row.row_widget, "mic_Icon");
        if (host.player_recently_spoke_007f74b0(row.slot)) {
            const HudScoringPlayer local = host.player(host.local_player_slot());
            host.set_visible(mic, !host.voice_muted_00778740(local, player));
        } else {
            host.set_visible(mic, false);
        }
    }
    return total;
}

void refresh_team_header(std::uintptr_t header, int state, int total, HudScoringHost& host) {
    host.select_state(host.find_child_00aa7e00(header, "USN_Icon"), state, 0, 1.0f);
    host.set_text(host.find_child_00aa7e00(header, "USN_point_Text"), std::to_string(total), true);
}

}  // namespace

void hud_scoring_update_0060e440(HudScoringScreenState& screen, HudScoringHost& host) {
    if (host.action_edge_004d92b0(0x4A) || host.action_edge_004d92b0(0x4B)) {
        hud_scoring_close_0060dd10(screen, host);
    }
    if (host.action_edge_004d92b0(0x4F)) {
        const HudScoringPlayer player = host.player(selected_slot(screen.listbox_18, host));
        // Every failed test leaves the update without refreshing the rows
        // (the JNEs at 0060E4DD..0060E51A go to the epilogue 0060EF31).
        if (!gamer_card_allowed(player, host)) {
            return;
        }
        host.show_gamer_card(player);
    }
    const int team1_total = refresh_team_rows(screen.team1_rows_2c, host);
    const int team0_total = refresh_team_rows(screen.team0_rows_1c, host);
    if (screen.team1_header_40 != 0) {
        refresh_team_header(screen.team1_header_40, 1, team1_total, host);
    }
    if (screen.team0_header_3c != 0) {
        refresh_team_header(screen.team0_header_3c, 0, team0_total, host);
    }
}

namespace {

// 006101CF..00610366: the record one occupied slot contributes, or false.
bool build_score_row(int slot, HudScoringHost& host, HudScoreRow& row, int& team) {
    const HudScoringPlayer player = host.player(slot);
    if (!player.active) {
        return false;
    }
    if (!player.ai) {
        row.name = player.name;
        row.clan = player.clan;
    } else {
        if (!player.ai_listed) {
            return false;
        }
        row.name = std::string(kHudScoringAiLevelNames.at(static_cast<std::size_t>(player.ai_level)));
        row.clan = " ";  // 00CE3A90
    }
    row.score = host.score_by_slot(slot);
    row.valid = true;
    row.slot = slot;
    team = player.team;
    return true;
}

// 006105EB..00610B2B (team 0) and 00610DC3..00611334 (team 1): one listbox
// row per record, positioned down the y cursor.
void insert_team_rows(HudScoringScreenState& screen, std::vector<HudScoreRow>& rows, float& y,
    HudScoringHost& host) {
    for (HudScoreRow& record : rows) {
        if (!record.valid) {
            break;
        }
        // An empty name becomes the 24-byte block holding "FE.multi_lobby_normalai".
        const std::string name = record.name.empty() ? std::string("FE.multi_lobby_normalai")
                                                     : record.name;
        const std::uintptr_t row = host.clone_subtree_00aab4c0(screen.row_template_14);
        host.set_name(row, std::to_string(record.slot));
        host.set_row_field_d8(row, host.listbox_field_104(screen.listbox_18));
        host.listbox_append_row_00a9d750(screen.listbox_18, row);
        host.listbox_select_row_00a9c740(screen.listbox_18, row);
        host.set_visible(row, true);
        record.row_widget = row;
        float position[3]{};
        host.get_resolved_position(row, position);
        const float placed[3]{position[0] + 0.0f, position[1] + y, position[2] + 0.0f};
        host.set_resolved_position(row, placed);
        y = static_cast<float>(y + kHudScoringRowStep);
        host.set_text(host.find_child_00aa7e00(row, "Player_point_Text"),
            std::to_string(record.score), true);
        const HudScoringPlayer player = host.player(record.slot);
        const bool localise = player.ai || equals_ignore_case(name, "globals.newplayer");
        host.set_text(host.find_child_00aa7e00(row, "Player_name_Text"), name, localise);
        host.set_text(host.find_child_00aa7e00(row, "Player_clan_Text"), record.clan, true);
    }
}

// 006103C1..006105E4 (team 0) and 00610B87..00610DB8 (team 1).
void place_team_header(HudScoringScreenState& screen, std::uintptr_t& header, float y_offset,
    int state, int total, std::string_view title_key, HudScoringHost& host) {
    if (header == 0) {
        header = host.clone_subtree_00aab4c0(screen.header_group_10);
    }
    host.set_visible(screen.header_group_10, false);
    host.set_visible(header, true);
    float position[3]{};
    host.get_resolved_position(header, position);
    const float placed[3]{position[0] + 0.0f, position[1] + y_offset, position[2] + 0.0f};
    host.set_resolved_position(header, placed);
    refresh_team_header(header, state, total, host);
    host.set_text_key(host.find_child_00aa7e00(header, "USN_Text"), title_key);
}

}  // namespace

void hud_scoring_enter_00610070(HudScoringScreenState& screen, HudScoringHost& host) {
    screen.team1_header_40 = 0;
    screen.team0_header_3c = 0;
    host.listbox_clear_rows_00a9bec0(screen.listbox_18);
    host.set_visible(screen.listbox_18, true);
    host.listbox_virtual_60(screen.listbox_18, true);
    host.listbox_set_spacing(screen.listbox_18, kHudScoringListboxSpacing);
    if (!host.game_online_1fe4() && !host.game_flag_218c()) {
        return;
    }
    screen.team0_rows_1c.clear();  // 0060F960 over [begin, end)
    screen.team1_rows_2c.clear();
    int team0_total = 0;
    int team1_total = 0;
    for (int slot = 0; slot < 8; ++slot) {  // game+18CCh..+18ECh
        HudScoreRow row;
        int team = -1;
        if (!build_score_row(slot, host, row, team)) {
            continue;
        }
        if (team == 0) {
            screen.team0_rows_1c.push_back(row);  // 0060FFC0
            team0_total += row.score;
        } else if (team == 1) {
            screen.team1_rows_2c.push_back(row);
            team1_total += row.score;
        }
    }
    // The y cursor is a float at [esp+1Ch]; each step adds a double and stores
    // back to float (00610441, 00610888, 00610BB3, 00610C16, 00611063).
    float y = 0.0f;
    if (!screen.team0_rows_1c.empty()) {
        place_team_header(screen, screen.team0_header_3c, 0.0f, 0, team0_total,
            "ingame.score_usa", host);
        y = static_cast<float>(0.0f + kHudScoringRowStep);
        insert_team_rows(screen, screen.team0_rows_1c, y, host);
    }
    if (!screen.team1_rows_2c.empty()) {
        y = static_cast<float>(y + kHudScoringTeamGap);
        place_team_header(screen, screen.team1_header_40, y, 1, team1_total,
            "ingame.score_jap", host);
        y = static_cast<float>(y + kHudScoringRowStep);
        insert_team_rows(screen, screen.team1_rows_2c, y, host);
    }
}

void please_wait_hide_0060d3e0(PleaseWaitScreenState& screen, PleaseWaitHost& host) {
    host.stop_render_worker_00aa2b80();
    if (screen.applied_05) {
        host.screen_exit();
    }
    screen.requested_04 = false;
    screen.applied_05 = false;
    host.commit_visibility_004f83b0();
}

void please_wait_show_0060d6e0(PleaseWaitScreenState& screen, bool background_visible,
    PleaseWaitHost& host) {
    screen.requested_04 = true;
    screen.applied_05 = true;
    host.commit_visibility_004f83b0();
    host.screen_enter();
    host.set_visible(screen.background_0c, background_visible);
    // 0060D710..0060D71B: (descriptor = page, 0060D650, context = page, 25).
    host.start_render_worker_00aa4040(screen.page_08, screen.page_08, kPleaseWaitWorkerRate);
}

void please_wait_worker_frame_0060d650(std::uintptr_t page, PleaseWaitHost& host) {
    host.gui_update_pages_00aa3f70({page}, kPleaseWaitGuiStep);
}

void please_wait_render_frame_0060d570(const PleaseWaitScreenState* screen, PleaseWaitHost& host) {
    if (screen == nullptr || !screen->applied_05) {
        return;
    }
    host.gui_update_pages_00aa3f70({screen->page_08, host.front_end_root_page()}, kPleaseWaitGuiStep);
    // 0060D60A..0060D626: the begin-frame result is not tested.
    host.game_render_frame();
}

void plane_spawn_on_object_released_0060c7d0(PlaneSpawnScreenState& screen,
    std::uintptr_t object, PlaneSpawnHost& host) {
    if (screen.holder_24 != 0 && host.holder_entity_7c(screen.holder_24) == object) {
        screen.holder_24 = 0;
    }
    if (screen.plane_20 == object) {
        screen.plane_20 = 0;
    }
    if (screen.observed_1c == object) {
        screen.observed_1c = 0;
    }
}

void plane_spawn_exit_0060c880(PlaneSpawnScreenState& screen, PlaneSpawnHost& host) {
    // The three releases appear twice in a row (0060C883..0060C8F5); the second
    // pass finds every field already cleared.
    for (int pass = 0; pass < 2; ++pass) {
        if (screen.plane_20 != 0) {
            host.unregister_pair_006952a0(screen.plane_20);
            screen.plane_20 = 0;
        }
        if (screen.holder_24 != 0) {
            host.unregister_pair_006952a0(host.holder_entity_7c(screen.holder_24));
            screen.holder_24 = 0;
        }
        if (screen.observed_1c != 0) {
            host.unregister_pair_006952a0(screen.observed_1c);
            screen.observed_1c = 0;
        }
    }
    host.manager_c4_00681ec0();
}

void plane_spawn_update_0060c830(PlaneSpawnScreenState& screen, PlaneSpawnHost& host) {
    if (screen.plane_20 == 0) {
        host.hud_root_force_select_unit_006485a0();
        return;
    }
    if (host.pilot_local_input_enabled_007bb9a0(screen.plane_20)) {
        host.hud_root_set_spectated_unit_00647300(host.plane_field_9d4(screen.plane_20));
    }
}

std::uint32_t plane_spawn_target_id_0060ca20(const PlaneSpawnTargetView& plane) noexcept {
    if (plane.flag_904) {
        return plane.has_bf4 ? plane.bf4_entity_id : 0;
    }
    // ucomiss + LAHF/TEST AH,44h/JNP: only an ordered equal compare returns 0.
    const bool unset = plane.field_538_198 == kPlaneSpawnTargetUnset;
    if (!unset && plane.has_9d4) {
        return plane.field_9d4_404;
    }
    return 0;
}
}  // namespace bsp
