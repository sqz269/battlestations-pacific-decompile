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
void gui_grid_constructed_00aca0b0(GuiGridState& grid, GuiGridHost& host) {
    grid.visual_group_118 = host.create_widget_00aa6640(2, "Visual_Group");
    host.append_child_00aaa5a0(grid.visual_group_118);
}

void gui_grid_loaded_00aca170(const GuiGridState& grid, GuiGridHost& host) {
    if (grid.width_20 == 0.0f || grid.height_24 == 0.0f) {
        return;
    }
    if (const std::uintptr_t vertical = host.find_child_00aaa2b0("_Vertical_Scrollbar")) {
        host.set_local_xy_00aa7d00(vertical, grid.width_20, 0.0f);
    }
    if (const std::uintptr_t horizontal = host.find_child_00aaa2b0("_Horizontal_Scrollbar")) {
        host.set_local_xy_00aa7d00(horizontal, 0.0f, grid.height_24);
    }
}

void gui_grid_layout_00aca290(GuiGridState& grid, GuiGridHost& host) {
    float x = 0.0f;
    float y = 0.0f;
    for (const GuiGridColumn& column : grid.columns_ec) {
        y = 0.0f;  // 00ACA30D: every column restarts at the top
        for (std::uintptr_t cell : column.cells) {
            if (cell != 0) {
                host.set_local_xy_00aa7d00(cell, x, y);
            }
            y = y + grid.cell_h_108;
        }
        x = x + column.width;
    }
    // The content height is the last column's (the cursor is not maximised).
    grid.content_w_11c = x;
    grid.content_h_120 = y;
    const std::uintptr_t horizontal = host.find_child_00aaa2b0("_Horizontal_Scrollbar");
    const std::uintptr_t vertical = host.find_child_00aaa2b0("_Vertical_Scrollbar");
    if (horizontal != 0) {
        host.scrollbar_set_maximum_00ac7de0(horizontal, grid.content_w_11c - grid.width_20);
    }
    if (vertical != 0) {
        host.scrollbar_set_maximum_00ac7de0(vertical, grid.content_h_120 - grid.height_24);
    }
}

void gui_grid_sync_scrollbars_00aca540(const GuiGridState& grid, GuiGridHost& host) {
    if (const std::uintptr_t horizontal = host.find_child_00aaa2b0("_Horizontal_Scrollbar")) {
        host.scrollbar_set_value_00ac7d00(horizontal, -0.0f - grid.scroll_x_110);
    }
    if (const std::uintptr_t vertical = host.find_child_00aaa2b0("_Vertical_Scrollbar")) {
        host.scrollbar_set_value_00ac7d00(vertical, -0.0f - grid.scroll_y_114);
    }
}

bool gui_grid_is_kind_of_00acb140(const std::array<std::uint32_t, 3>& chain, std::uint32_t id) noexcept {
    for (std::uint32_t entry : chain) {
        if (entry == id) {
            return true;
        }
    }
    return false;
}
void gui_sound_set_visible_00acdf10(GuiSoundWidgetState& sound, bool visible, GuiSoundWidgetHost& host) {
    host.base_set_visible_00aa8530(visible);
    if (!visible) {
        if (sound.sound_f0 != 0) {
            host.sound_stop(sound.sound_f0, false);
            host.release_sound_0054d510(sound.sound_f0);
            sound.sound_f0 = 0;
        }
        return;
    }
    if (sound.sound_f0 != 0) {
        return;  // already playing
    }
    const std::int64_t type = host.find_sound_type_00a7b0a0("Normal");
    sound.sound_f0 = host.create_tracked_sound_00a7e490(sound.sample_ec, sound.sound_class_f4, type);
}

bool gui_sound_update_should_hide_00acd2c0(const GuiSoundWidgetState& sound, GuiSoundWidgetHost& host) {
    return sound.sound_f0 != 0 && host.sound_finished(sound.sound_f0);
}

void gui_sound_bind_resources_00acddb0(GuiSoundWidgetState& sound, GuiSoundWidgetHost& host) {
    const std::uintptr_t sample = host.acquire_sample_00a83fd0(sound.sound_efx_f8);
    if (sound.sample_ec != sample) {
        const std::uintptr_t old = sound.sample_ec;
        sound.sample_ec = sample;
        if (sample != 0) {
            host.retain(sample);
        }
        if (old != 0) {
            host.release(old);
        }
    }
    if (sample != 0) {
        host.release(sample);  // the acquire's temporary reference
    }
    sound.sound_class_f4 = host.find_sound_class_00a7acf0(sound.category_100);
}
namespace {

std::uintptr_t& powerup_widget(HudPowerupScreenState& screen, std::uint16_t field) {
    return screen.widgets.at(field / 4);
}

std::uintptr_t powerup_parent(HudPowerupScreenState& screen, std::uint16_t field) {
    if (field == 0x24) {
        return screen.support_page_24;
    }
    if (field == 0xB0) {
        return screen.info_page_b0;
    }
    return powerup_widget(screen, field);
}

}  // namespace

void hud_powerup_layout_00614600(HudPowerupScreenState& screen, HudPowerupHost& host) {
    screen.support_page_24 = host.load_page_00aa5840(kHudPowerupSupportPage);
    screen.info_page_b0 = host.load_page_00aa5840(kHudPowerupInfoPage);
    std::uintptr_t previous = 0;
    for (const HudPowerupBinding& row : kHudPowerupBindings) {
        const std::uintptr_t parent =
            row.parent_field == 0 ? previous : powerup_parent(screen, row.parent_field);
        previous = host.find_child_00aa7e00(parent, row.name);
        if (row.field != 0) {
            powerup_widget(screen, row.field) = previous;
        }
    }
}

void hud_powerup_register_00614170(HudPowerupScreenState& screen, HudPowerupHost& host) {
    host.front_end_register_004f71d0();
    hud_powerup_layout_00614600(screen, host);  // vtable +14h
    host.set_visible(powerup_widget(screen, 0xB8), false);
    host.set_visible(host.find_child_00aa7e00(screen.support_page_24, "map_Group"), false);
    float position[3]{};
    host.get_resolved_position(powerup_widget(screen, 0xE4), position);
    screen.right_circle_23c[1] = position[1];
    screen.right_circle_23c[2] = position[2];
    screen.right_circle_23c[0] = position[0] - screen.field_234;  // x87 fsub, stored as float
    host.get_resolved_position(screen.info_page_b0, screen.info_page_f4);
    screen.mode_20 = 0;
    screen.field_228 = 0;
    screen.list_1a4.clear();  // 004685A0 erase(begin, end)
    screen.field_1f0 = 0;
    screen.field_1ec = 0;
    screen.field_1f4 = 0;
    screen.flag_1f9 = false;
    screen.flag_22c = false;
    screen.field_1fc = 0.0f;
    screen.field_1e8 = 0.0f;
    screen.field_230 = 0.0f;
}

// ---------------------------------------------------------------------------
// Powerup-manager enter 0061B850, exit 006169F0, update 0061BEA0, carousel 00619C50 and the second
// list's selection 0061A6C0 (cc13-hud2). Evidence: the listings, read whole; the support-manager
// write table is in docs/CC13_HUD_AND_WIDGETS.md.
// ---------------------------------------------------------------------------

namespace {

constexpr std::array<float, 4> kPowerupFullUv{0.0f, 0.0f, 1.0f, 1.0f};

std::uint32_t powerup_list_size(const std::vector<std::uintptr_t>& list) {
    return static_cast<std::uint32_t>(list.size());
}

// The bounds-checked element read the image open-codes before each use (00BF6713 when out of range).
std::uintptr_t powerup_entry_at(const std::vector<std::uintptr_t>& list, std::uint32_t index) {
    return list.at(index);
}

// 0061BB5F..0061BBA3 (and the copies in update and 0061A6C0): label width and panel extent.
void powerup_relayout_label(HudPowerupScreenState& screen, HudPowerupScreenHost& host) {
    const std::array<float, 3> extent = host.group_extent_00aa6740(powerup_widget(screen, 0xEC));
    screen.field_234 = static_cast<float>(static_cast<double>(screen.label_width_254) + kPowerupLabelPad);
    screen.field_238 = extent[1];
}

float powerup_label_width(HudPowerupScreenState& screen, HudPowerupScreenHost& host) {
    return static_cast<float>(host.text_width_114(powerup_widget(screen, 0xD4)) / kPowerupLabelScale);
}

// 0061C5C9..0061C615 / 0061A910..0061A95D: widen only; the image then hides HUD+C0's +D4h (push 0).
void powerup_widen_label(HudPowerupScreenState& screen, HudPowerupScreenHost& host) {
    const float width = powerup_label_width(screen, host);
    if (width > screen.label_width_254) {
        screen.label_width_254 = width;
        host.set_visible(host.hud_c0_widget(0xD4), false);
    }
}

// The common tail of 00619C50 (from 00619DE5) and 0061A6C0 (from 0061A998) once the selected
// definition is known. `insert_when_present` is 0061A6C0's inverted cache test (00611C80-style
// != end plus texture != 0) against 00619C50's == end (00611C50).
void powerup_show_selected(HudPowerupScreenHost& host, std::uint16_t c0_vector, std::uint32_t index,
    bool second_list) {
    const auto definition = [&] {
        return host.entry_definition_4(powerup_entry_at(host.hud_c0_entries(c0_vector), index));
    };
    if (host.definition_kind_8(definition()) == 1) {
        host.set_visible(host.support_widget(0xF0), false);
        host.set_visible(host.support_widget(0xF4), true);
        host.set_visible(host.support_widget(0x1CC), true);
        host.set_visible(host.hud_c0_widget(0xB8), false);
        const HudPowerupTeamEntry team = host.team_entry(definition(), host.local_team_28());
        const std::uintptr_t art = host.support_widget(0x158);
        if (team.icon_name_1c != 0) {
            const std::uintptr_t texture = host.resolve_icon_texture(team.unit_class_20, team.icon_name_1c);
            const std::array<float, 4> uv = host.resolve_icon_uv(team.unit_class_20, team.icon_name_1c);
            host.set_state_texture_00ab2690(art, 3, texture, uv);
            host.widget_set_state_88(art, 0, 0, 1.0f);
            host.widget_set_state_88(art, 3, 0, 1.0f);
        } else {
            host.widget_set_state_88(art, 0, 0, 1.0f);
        }
        if (team.unit_class_20 != 0) {
            const std::int32_t key = host.class_key_70(team.unit_class_20);
            const bool present = host.support_icon_map_contains_00614060(key);
            const bool insert = second_list ? (present && host.class_texture_d4(team.unit_class_20) != 0) : !present;
            if (insert) {
                const std::uint32_t state = host.icon_add_state_00ab64c0(host.support_widget(0x1CC),
                    host.class_texture_d4(team.unit_class_20), kPowerupFullUv);
                host.support_icon_map_store_006171b0(key, state);           // the SM+42Ch write
            }
        }
        if (team.unit_class_20 != 0 &&
            host.support_icon_map_contains_00614060(host.class_key_70(team.unit_class_20))) {
            host.widget_set_state_88(host.support_widget(0x1CC),
                host.support_icon_map_read_006171b0(host.class_key_70(team.unit_class_20)), 0, 1.0f);
            return;
        }
        host.widget_set_state_88(host.support_widget(0x1CC), 0, 0, 1.0f);
        return;
    }
    host.set_visible(host.support_widget(0xF0), true);
    host.set_visible(host.support_widget(0xF4), false);
    host.set_visible(host.support_widget(0x1CC), false);
    host.set_visible(host.hud_c0_widget(0xB8), true);
    if (host.definition_name_is_00425850(definition(), kPowerupRadarSweep)) {
        host.set_visible(host.hud_c0_widget(0xBC), true);
        return;
    }
    host.set_visible(host.hud_c0_widget(0xBC), false);
    host.set_visible(host.hud_c0_widget(0xC0), true);
    // 0061A2D5..0061A3F6: the category icons show when the definition's bit is clear.
    host.set_visible(host.hud_c0_widget(0xC4), (~host.definition_flags_88(definition()) & 1u) != 0);
    host.set_visible(host.hud_c0_widget(0xC8), (~(host.definition_flags_88(definition()) >> 1) & 1u) != 0);
    host.set_visible(host.hud_c0_widget(0xCC), (~(host.definition_flags_88(definition()) >> 2) & 1u) != 0);
    host.set_visible(host.hud_c0_widget(0xD0), (~(host.definition_flags_88(definition()) >> 3) & 1u) != 0);
}

}  // namespace

void hud_powerup_enter_0061b850(HudPowerupScreenState& screen, HudPowerupScreenHost& host) {
    host.slot51_00529a30();                                              // 0061B86F..0061B879
    host.set_resolved_position_00aa8240(host.support_widget(0x1C4), screen.home_14);
    host.set_visible(screen.support_page_24, true);
    host.set_visible(host.support_widget(0x12C), false);                // 0061B8A5..0061B8F2
    host.set_visible(host.support_widget(0x130), false);
    host.set_visible(host.support_widget(0x134), false);
    host.support_delete_slot_clones_00652a60();
    host.support_rebind_slots_0065d220();
    host.play_support_sound(HudSupportSound::Open);
    host.set_visible(host.support_widget(0x1B0), true);                 // 0061B926..0061B959
    host.set_visible(host.support_widget(0x1C4), true);
    host.widget_set_size_58(host.support_widget(0x1C4), kPowerupHomeSize);
    host.set_visible(host.support_widget(0x1C0), false);                // 0061B994..0061B9DF
    host.set_visible(host.support_widget(0x178), true);
    host.set_visible(host.support_widget(0x1C8), false);
    host.bind_support_widgets_00614320();
    screen.flag_200 = true;                                              // 0061B9EE..0061BA21
    screen.flag_22c = true;
    host.set_visible(powerup_widget(screen, 0xD4), false);
    screen.first_ready_20c = false;
    screen.field_230 = 0.0f;
    host.set_visible(powerup_widget(screen, 0xD4), false);
    host.set_visible(powerup_widget(screen, 0xE8), true);
    host.set_visible(powerup_widget(screen, 0xEC), true);
    host.set_visible(powerup_widget(screen, 0xE4), true);
    // 0061BA50..0061BA8F: the info page 0.3 to the left of its registered position.
    const float info[3] = {static_cast<float>(static_cast<double>(screen.info_page_f4[0]) - kPowerupInfoOffsetEnter),
        screen.info_page_f4[1], screen.info_page_f4[2]};
    host.set_resolved_position_00aa8240(screen.info_page_b0, info);
    host.get_resolved_position(powerup_widget(screen, 0xE4), screen.right_circle_248);
    screen.carousel_owner_110 = host.screen_address() + 8;               // 0061BABA..0061BAC3
    screen.carousel_slot_100 = 0;
    screen.carousel_selected_104 = 0;
    host.rebuild_lists_0061b0f0();
    if (!screen.entries_144.empty()) {                                   // 0061BADA..0061BBD1
        screen.mode_20 = 0;
        const std::uintptr_t definition = host.entry_definition_4(powerup_entry_at(host.hud_c0_entries(0x144), 0));
        host.text_set_source_00abaed0(powerup_widget(screen, 0xD4),
            host.team_entry(definition, host.local_team_28()).text_source, true);
        screen.label_width_254 = powerup_label_width(screen, host);
        powerup_relayout_label(screen, host);
        screen.count_204 = static_cast<std::int32_t>(screen.entries_144.size());
    } else {                                                             // 0061BBD6..0061BCC0
        screen.mode_20 = 1;
        screen.field_1f0 = 0;
        screen.field_1ec = 0;
        screen.field_1f4 = 0;
        const std::uintptr_t definition = host.entry_definition_4(powerup_entry_at(host.hud_c0_entries(0x184), 0));
        host.text_set_source_00abaed0(host.hud_c0_widget(0xD4),
            host.team_entry(definition, host.local_team_28()).text_source, true);
        screen.label_width_254 = powerup_label_width(screen, host);
        powerup_relayout_label(screen, host);
        for (std::uint16_t field = 0x48; field <= 0x54; field += 4) host.set_visible(powerup_widget(screen, field), false);
    }
    screen.fade_1e4 = false;                                             // 0061BCC2..0061BD2A
    screen.field_1e8 = 0.0f;
    screen.list_1a4.clear();
    // 0061BD30..0061BE70: the first-use hint once, the permanent hint afterwards.
    if (host.hint_shown_0068e600(kPowerupHintFirstUse)) {
        host.trigger_hint_00690fd0(kPowerupHintPermanent, 0);
    } else {
        host.trigger_hint_00690fd0(kPowerupHintFirstUse, 0);
    }
    host.slot51_00528f70();                                              // 0061BE75..0061BE80
}

void hud_powerup_exit_006169f0(HudPowerupScreenState& screen, HudPowerupScreenHost& host) {
    host.set_visible(host.hud_slot_widget(0xC4, 0x64), false);           // 006169F3..00616A0D
    host.hud_refresh_0054e610();
    host.set_visible(screen.support_page_24, false);
    host.set_visible(powerup_widget(screen, 0xD4), false);
    host.set_visible(host.support_widget(0xF0), true);                  // 00616A3B..00616A89
    host.set_visible(host.support_widget(0xF4), true);
    host.set_visible(host.support_widget(0x1CC), true);
    host.set_visible(host.hud_c0_widget(0xB8), false);
    host.set_visible(host.support_widget(0x12C), true);                 // 00616AA5..00616AF3
    host.set_visible(host.support_widget(0x130), true);
    host.set_visible(host.support_widget(0x134), true);
    host.play_support_sound(HudSupportSound::Close);
    host.set_visible(screen.info_page_b0, true);
    float position[3]{};
    host.get_resolved_position(powerup_widget(screen, 0xE8), position);  // 00616B14..00616B2B
    host.set_resolved_position_00aa8240(powerup_widget(screen, 0xE4), position);
    host.set_visible(powerup_widget(screen, 0xE8), false);
    host.set_visible(powerup_widget(screen, 0xEC), false);
    // 00616B4E..00616BA7: FSUB of a double constant chosen by the support screen's refresh result.
    const double offset = host.support_refresh_0066ffa0() ? kPowerupInfoOffsetExitOn : kPowerupInfoOffsetExitOff;
    const float info[3] = {static_cast<float>(static_cast<double>(screen.info_page_f4[0]) - offset),
        screen.info_page_f4[1], screen.info_page_f4[2]};
    host.set_resolved_position_00aa8240(screen.info_page_b0, info);
    // 00616BAC..00616BD9: deleting destructors on +28h..+34h and +58h..+64h; the pointers stay.
    for (std::uint16_t k = 0; k < 4; ++k) {
        if (powerup_widget(screen, static_cast<std::uint16_t>(0x28 + 4 * k)) != 0) {
            host.delete_virtual_4(powerup_widget(screen, static_cast<std::uint16_t>(0x28 + 4 * k)));
        }
        if (powerup_widget(screen, static_cast<std::uint16_t>(0x58 + 4 * k)) != 0) {
            host.delete_virtual_4(powerup_widget(screen, static_cast<std::uint16_t>(0x58 + 4 * k)));
        }
    }
    host.tree_erase_00615c90(0x1B4);                                     // 00616BDB..00616C03
    screen.tree_1b4.clear();
    host.tree_erase_00615c90(0x1CC);                                     // 00616C06..00616C2E
    screen.tree_1cc.clear();
    host.support_rebind_slots_0065d220();
    host.slot51_00529060();
    host.slot51_00529a30();                                              // tail jump 00616C64
}

void hud_powerup_update_0061bea0(HudPowerupScreenState& screen, HudPowerupScreenHost& host, float dt) {
    // 0061BEBC..0061BF8F: a blocking hint suspends the screen.
    bool blocking = false;
    if (!host.active_hint_name_0068e5b0().empty()) blocking = host.hint_blocking_00690b00(host.active_hint_name_0068e5b0());
    if (blocking) return;
    if (host.page_busy_121(screen.support_page_24) || host.page_busy_121(screen.info_page_b0)) return;
    // 0061BFB8..0061BFEE: tile_Icon size (+230h * +234h, +238h).
    host.widget_set_size_58(powerup_widget(screen, 0xEC),
        {static_cast<float>(static_cast<double>(screen.field_230) * screen.field_234), screen.field_238});
    // 0061BFF0..0061C056: right_circle_Icon at the tile's right edge.
    const std::array<float, 3> extent = host.group_extent_00aa6740(powerup_widget(screen, 0xEC));
    const float right[3] = {static_cast<float>(static_cast<double>(extent[0]) + screen.right_circle_248[0]),
        screen.right_circle_248[1], screen.right_circle_248[2]};
    host.set_resolved_position_00aa8240(powerup_widget(screen, 0xE4), right);
    // 0061C05B..0061C0E0: text_Text centred half its width left of the circle, y/z kept.
    float text_z[3]{};
    float text_y[3]{};
    float circle[3]{};
    host.get_resolved_position(powerup_widget(screen, 0xD4), text_z);
    host.get_resolved_position(powerup_widget(screen, 0xD4), text_y);
    host.get_resolved_position(powerup_widget(screen, 0xE4), circle);
    const float text[3] = {static_cast<float>(static_cast<double>(circle[0]) -
                               static_cast<double>(screen.label_width_254) * 0.5),
        text_y[1], text_z[2]};
    host.set_resolved_position_00aa8240(powerup_widget(screen, 0xD4), text);
    if (!screen.flag_22c) host.set_visible(powerup_widget(screen, 0xD4), true);
    host.set_visible(host.hud_slot_widget(0xC4, 0x64), true);           // 0061C0FD..0061C113
    host.slot50_message_00682bd0(host.xbox_prompts_f88a30() ? kPowerupMessageXbox : kPowerupMessagePc);
    // 0061C15A..0061C1E1: scroll arrows when a list overflows its four slots.
    host.support_show_100_104(screen.entries_144.size() > 4 && !screen.carousel_scrolling_13f);
    host.support_show_f8_fc(screen.entries_184.size() > 4 && !screen.flag_1f9);
    // 0061C1E6..0061C3A4: re-raise the permanent hint unless it is the active one or a hint blocks.
    bool raise = false;
    if (host.names_unequal_00449af0(host.active_hint_name_0068e5b0(), kPowerupHintPermanent)) {
        raise = !host.hint_blocking_00690b00(host.active_hint_name_0068e5b0());
    } else if (host.active_hint_name_0068e5b0().empty()) {
        raise = !host.hint_blocking_00690b00(host.active_hint_name_0068e5b0());
    }
    if (raise) host.trigger_hint_00690fd0(kPowerupHintPermanent, 0);
    // 0061C3A4..0061C42C: a changed first list resets the carousel and starts its intro.
    if (screen.count_204 != static_cast<std::int32_t>(screen.entries_144.size())) {
        for (std::uint16_t field = 0x78; field <= 0x84; field += 4) host.set_visible(powerup_widget(screen, field), false);
        screen.carousel_slot_100 = 0;
        screen.carousel_selected_104 = 0;
        host.set_visible(powerup_widget(screen, 0x78), true);
        screen.count_204 = static_cast<std::int32_t>(screen.entries_144.size());
        screen.field_208 = 1.0f;
        screen.first_ready_20c = true;
    }
    if (screen.first_ready_20c) {                                        // 0061C433..0061C468
        host.first_intro_00611ae0(dt);
        host.carousel_tick_00617470();
        return;
    }
    host.tick_00611770(dt);                                              // 0061C46B..0061C489
    host.second_fade_0061aff0(dt);
    host.tick_00612b70(dt);
    if (screen.count_204 != static_cast<std::int32_t>(screen.entries_144.size())) {   // 0061C48E..0061C4F4
        host.set_visible(powerup_widget(screen, static_cast<std::uint16_t>(0x78 + 4 * screen.carousel_slot_100)), false);
        screen.carousel_slot_100 = 0;
        screen.carousel_selected_104 = 0;
        host.set_visible(powerup_widget(screen, 0x78), true);
        screen.count_204 = static_cast<std::int32_t>(screen.entries_144.size());
    }
    if (!screen.entries_184.empty() && screen.mode_20 == 0 && host.action_pressed_004c43c0(kPowerupActionToSecond) &&
        !screen.carousel_scrolling_13f) {                                // 0061C4FA..0061C655
        for (std::uint16_t field = 0x78; field <= 0x84; field += 4) host.set_visible(powerup_widget(screen, field), false);
        screen.mode_20 = 1;
        const std::uintptr_t definition =
            host.entry_definition_4(powerup_entry_at(host.hud_c0_entries(0x184), static_cast<std::uint32_t>(screen.field_1f4)));
        host.text_set_source_00abaed0(host.hud_c0_widget(0xD4),
            host.team_entry(definition, host.local_team_28()).text_source, true);
        powerup_widen_label(screen, host);
        powerup_relayout_label(screen, host);
        host.play_support_sound(HudSupportSound::LeftRight);
    } else if (!screen.entries_144.empty() && screen.mode_20 == 1 &&
               host.action_pressed_004c43c0(kPowerupActionToFirst) && !screen.flag_1f9) {   // 0061C657..0061C6CD
        screen.mode_20 = 0;
        screen.field_228 = 0;
        host.carousel_refresh_label_00618980();
        for (std::uint16_t field = 0x88; field <= 0x94; field += 4) host.set_visible(powerup_widget(screen, field), false);
        host.play_support_sound(HudSupportSound::LeftRight);
    }
    host.second_refresh_rows_00619ae0();                                 // 0061C6D2
    if (screen.entries_144.empty()) screen.mode_20 = 1;                  // 0061C6D9..0061C6F3
    if (screen.mode_20 == 0) {
        hud_powerup_carousel_update_00619c50(screen, host, dt);
    } else {
        hud_powerup_update_selection_0061a6c0(screen, host);
    }
    // 0061C71C..0061C77F: the use input. In the first list it calls the gameplay power-up manager.
    if (!screen.entries_144.empty() && host.input_owner_flag_b_or_10()) {
        if (screen.mode_20 == 0) {
            host.powerup_use_008eb6e0(powerup_entry_at(screen.entries_144, screen.carousel_selected_104));
        } else {
            host.second_refresh_rows_00619ae0();
        }
    } else if (screen.mode_20 != 0) {
        host.second_refresh_rows_00619ae0();
    }
    screen.flag_200 = false;                                             // 0061C784
}

void hud_powerup_carousel_update_00619c50(HudPowerupScreenState& screen, HudPowerupScreenHost& host, float dt) {
    if (screen.carousel_scrolling_13f) {                                 // 00619C56..00619C74
        host.carousel_animate_006113a0(dt);
        host.carousel_tick_00617470();
        return;
    }
    host.carousel_tick_00617470();
    const std::uint32_t size = powerup_list_size(screen.carousel_entries_11c);
    std::uint32_t& selected = screen.carousel_selected_104;
    std::int32_t& slot = screen.carousel_slot_100;
    if (host.action_pressed_004c43c0(kPowerupActionPrev)) {              // 00619C7D..00619D28
        if (size <= 4 && selected == 0) return;                          // an empty vector counts as <= 4
        host.play_support_sound(HudSupportSound::UnitToUnit);
        screen.carousel_moved_13d = true;
        host.set_visible(host.hud_c0_widget(static_cast<std::uint16_t>(0x78 + 4 * slot)), false);
        selected = (size + selected - 1) % size;                         // unsigned DIV
        if (slot > 0) {
            --slot;
        } else {
            screen.carousel_wrap_13c = true;
            host.carousel_start_scroll_006179d0();
            screen.carousel_scroll_13e = true;
        }
        host.carousel_refresh_label_00618980();
    } else if (host.action_pressed_004c43c0(kPowerupActionNext)) {       // 00619D2D..00619DDB
        if (size <= 4 && selected == size - 1) return;
        host.play_support_sound(HudSupportSound::UnitToUnit);
        host.set_visible(host.hud_c0_widget(static_cast<std::uint16_t>(0x78 + 4 * slot)), false);
        screen.carousel_moved_13d = true;
        selected = (selected + 1) % size;
        if (slot < 3) {
            ++slot;
        } else {
            screen.carousel_wrap_13c = false;
            host.carousel_start_scroll_006179d0();
            screen.carousel_scroll_13e = true;
        }
        host.carousel_refresh_label_00618980();
    }
    // 00619DE5..00619E37: the selected definition's name into the support screen's +178h text.
    const std::uintptr_t definition = host.entry_definition_4(powerup_entry_at(host.hud_c0_entries(0x144), selected));
    host.text_set_source_00abaed0(host.support_widget(0x178), definition + 0x14, true);
    powerup_show_selected(host, 0x144, selected, false);
}

void hud_powerup_update_selection_0061a6c0(HudPowerupScreenState& screen, HudPowerupScreenHost& host) {
    const std::uint32_t size = powerup_list_size(screen.entries_184);
    auto& selected = reinterpret_cast<std::uint32_t&>(screen.field_1f4);
    bool moved = false;
    if (host.action_pressed_004c43c0(kPowerupActionPrev) && !screen.flag_1f9 && !screen.fade_1e4) {   // 0061A6C9..0061A7C7
        if (screen.field_1f0 == 0 && size <= 4) return;
        host.play_support_sound(HudSupportSound::UnitToUnit);
        if (screen.field_1f0 == 0) {
            if (screen.field_1ec == 0) screen.field_1ec = static_cast<std::int32_t>(size);   // 0059AE20
            screen.field_1ec -= 1;
            screen.second_dir_1f8 = false;
            host.second_start_scroll_006191a0();
        } else {
            screen.field_1f0 -= 1;
        }
        selected = (size + selected - 1) % size;
        moved = true;
    } else if (host.action_pressed_004c43c0(kPowerupActionNext) && !screen.flag_1f9 && !screen.fade_1e4) {   // 0061A7CC..0061A8B7
        if (static_cast<std::uint32_t>(screen.field_1f0) >= size - 1) return;   // unsigned JAE
        host.play_support_sound(HudSupportSound::UnitToUnit);
        if (screen.field_1f0 == 3) {
            if (size > 4) {
                screen.field_1ec += 1;
                if (screen.field_1ec == static_cast<std::int32_t>(size)) screen.field_1ec = 0;
            }
            screen.second_dir_1f8 = true;
            host.second_start_scroll_006191a0();
        } else {
            screen.field_1f0 += 1;
        }
        selected = (selected + 1) % size;
        moved = true;
    }
    if (moved) {                                                         // 0061A8C4..0061A993
        const std::uintptr_t definition = host.entry_definition_4(powerup_entry_at(host.hud_c0_entries(0x184), selected));
        host.text_set_source_00abaed0(host.hud_c0_widget(0xD4),
            host.team_entry(definition, host.local_team_28()).text_source, true);
        powerup_widen_label(screen, host);
        const std::array<float, 3> extent = host.group_extent_00aa6740(powerup_widget(screen, 0xEC));
        screen.field_234 = static_cast<float>(static_cast<double>(screen.label_width_254) + kPowerupLabelPad);
        screen.field_238 = extent[1];
        host.second_refresh_rows_00619ae0();
    }
    // 0061A998..0061AA00: the name text comes from this screen's own +184h list.
    if (selected >= size) return;
    host.text_set_source_00abaed0(host.support_widget(0x178),
        host.entry_definition_4(powerup_entry_at(screen.entries_184, selected)) + 0x14, true);
    powerup_show_selected(host, 0x184, selected, true);
}

Vec3f plane_spawn_camera_target_0060cb60(const PlaneSpawnCameraState& camera,
    PlaneSpawnCameraHost& host) {
    if (camera.home_384 != 0 && host.is_kind_of(camera.home_384, 9)) {
        return host.home_kind9_position(camera.home_384);
    }
    const PlaneSpawnPlaneView plane = host.plane(camera.plane_388);
    if (!plane.current_5c && plane.state_900 == 1) {
        const std::uintptr_t block = host.air_ops_block_006bcd20(camera.home_384);
        if (block == 0) {
            return {};
        }
        return host.parked_position(block, camera.plane_388);
    }
    if (plane.parent_3c == camera.home_384 || camera.home_384 == 0) {
        return plane.local_a4;
    }
    return host.transform_into_home(camera.home_384, camera.plane_388);
}

void plane_spawn_camera_update_0060cc90(PlaneSpawnCameraState& camera, float dt,
    PlaneSpawnCameraHost& host) {
    if (camera.plane_388 == 0) {
        return;
    }
    auto& m = camera.matrix_74;
    m[12] = camera.seed_38c[12];
    m[13] = camera.seed_38c[13];
    m[14] = camera.seed_38c[14];
    const Vec3f target = plane_spawn_camera_target_0060cb60(camera, host);
    m[8] = target.x - m[12];   // forward (+94h) = target - eye, each an FSUB/FSTP
    m[9] = target.y - m[13];
    m[10] = target.z - m[14];
    m[4] = 0.0f;               // up (+84h) = (0, 1, 0)
    m[5] = 1.0f;
    m[6] = 0.0f;
    host.orthonormalize_0085dc80(m);
    host.publish_pose_004329d0(dt);
}

Vec3f plane_spawn_camera_eye_offset(const Vec3f& row0, const Vec3f& row2, float tuning) noexcept {
    const auto f = [](double v) { return static_cast<float>(v); };
    const double t = tuning;
    const float a = f(-row0.x * t);          // [esp+20h]
    const float b = f(kPlaneSpawnEyeUp * t);  // [esp+24h] and [esp+0Ch]
    const float c = f(-row0.z * t);          // [esp+28h]
    const float px = f(row2.x * t);
    const float pz = f(row2.z * t);
    const float x = f(f(px * kPlaneSpawnEyeBack) + static_cast<double>(a));
    const float mid = f(f(b * kPlaneSpawnEyeBack) + static_cast<double>(b));
    const float z = f(f(kPlaneSpawnEyeBack * pz) + static_cast<double>(c));
    const float y = f(t * kPlaneSpawnEyeHeight + mid);
    return {x, y, z};
}

void plane_spawn_camera_set_targets_0060cd50(PlaneSpawnCameraState& camera,
    std::uintptr_t home, std::uintptr_t plane, PlaneSpawnCameraHost& host) {
    if (camera.home_384 != 0) {
        host.unregister_pair_006952a0(camera.home_384);
        camera.home_384 = 0;
    }
    if (camera.plane_388 != 0) {
        host.unregister_pair_006952a0(camera.plane_388);
        camera.plane_388 = 0;
    }
    camera.home_384 = home;
    camera.plane_388 = plane;
    if (home != 0) {
        host.register_pair_00694a60(home);
    }
    if (camera.plane_388 != 0) {
        host.register_pair_00694a60(camera.plane_388);
    }
    if (const std::uintptr_t block = host.air_ops_block_006bcd20(camera.home_384)) {
        camera.seed_38c = host.block_matrix(block);
    } else if (camera.plane_388 != 0) {
        camera.seed_38c = host.plane_matrix(camera.plane_388);
        const PlaneSpawnPlaneView view = host.plane(camera.plane_388);
        const Vec3f offset = plane_spawn_camera_eye_offset(view.row0_cc, view.row2_ec, host.tuning_7c());
        camera.seed_38c[12] = camera.seed_38c[12] + offset.x;
        camera.seed_38c[13] = camera.seed_38c[13] + offset.y;
        camera.seed_38c[14] = camera.seed_38c[14] + offset.z;
    }
    host.camera_virtual_ac(camera.home_384);
    camera.matrix_74 = camera.seed_38c;  // 004134F0
}
void plane_spawn_attach_plane_0060cf90(PlaneSpawnScreenState& screen, std::uintptr_t plane,
    PlaneSpawnAttachHost& host) {
    screen.plane_20 = plane;
    if (plane != 0) {
        host.register_pair_00694a60(plane);
        if (const std::uintptr_t home = host.screen_home_entity()) {
            host.register_pair_00694a60(home);
            screen.holder_24 = host.air_ops_block_006bcd20(home);
        }
    }
    if (screen.observed_1c == 0) {
        screen.observed_1c = host.create_camera();
        host.init_camera_00923870(screen.observed_1c);
        host.register_pair_00694a60(screen.observed_1c);
    }
    const std::uintptr_t home_entity =
        screen.holder_24 != 0 ? host.holder_entity_7c(screen.holder_24) : 0;
    host.set_targets_0060cd50(screen.observed_1c, home_entity, screen.plane_20);
    host.set_camera_mover_004bc410(screen.observed_1c);
}
void chat_add_message_005d0050(ChatScreenState& screen, const std::string& sender, int type,
    const std::string& message, const std::string& second, ChatScreenHost& host) {
    if (host.game_online_1fe4() && host.lua_event_dispatcher_present()) {
        host.fire_lua_chat_event_00985f30(sender, type, message);
    }
    std::u16string text;
    switch (static_cast<ChatMessageType>(type)) {
    case ChatMessageType::Swap:
        host.set_lua_global_string("SwapPlayer1", message);
        host.set_lua_global_string("SwapPlayer2", second);
        text = host.resolve_key_00a9fad0("globals.swapmessage");
        break;
    case ChatMessageType::Join:
        host.set_lua_global_string("SwapPlayer1", message);
        text = host.resolve_key_00a9fad0("globals.joinmessage");
        break;
    case ChatMessageType::Left:
        host.set_lua_global_string("SwapPlayer1", message);
        text = host.resolve_key_00a9fad0("globals.leftmessage");
        break;
    case ChatMessageType::Surrender:
        host.set_lua_global_string("SwapPlayer1", message);
        text = host.resolve_key_00a9fad0("ingame.multi_surrender");
        break;
    case ChatMessageType::NotSurrender:
        host.set_lua_global_string("SwapPlayer1", message);
        text = host.resolve_key_00a9fad0("ingame.multi_notsurrender");
        break;
    case ChatMessageType::LostUnit:
        // The second string is itself a key; its text, narrowed to its low
        // bytes, becomes the Lua global the message key interpolates.
        host.set_lua_global_string("lostunitplayername",
            host.narrow_low_bytes(host.resolve_key_00a9fad0(second)));
        text = host.resolve_key_00a9fad0(message);
        break;
    default:
        // An empty sender (00CE3A0C, case-insensitive compare) marks a system
        // message whose text is a key; otherwise the text is shown verbatim.
        text = sender.empty() ? host.resolve_key_00a9fad0(message) : host.widen_004c5e60(message);
        break;
    }
    ChatMessage entry{sender, type, text, kChatOverlaySeconds};
    if (screen.overlay_68.size() > kChatOverlayCap - 1) {
        screen.overlay_68.pop_back();
    }
    screen.overlay_68.insert(screen.overlay_68.begin(), entry);
    entry.timer = 0.0f;  // the history and lobby nodes have no timer field
    if (screen.history_74.size() > kChatHistoryCap - 1) {
        screen.history_74.pop_back();
    }
    screen.history_74.insert(screen.history_74.begin(), entry);
    if (screen.lobby_80.size() > kChatLobbyCap - 1) {
        screen.lobby_80.erase(screen.lobby_80.begin());
    }
    screen.lobby_80.push_back(entry);
    screen.dirty_8d = true;
}
bool chat_message_passes_mute_005d0770(std::uint64_t sender_xuid, ChatMuteHost& host) {
    const int slot = host.local_player_slot();
    const std::uint64_t local = (slot < 0 || slot > 7) ? ~std::uint64_t{0} : host.player_xuid(slot);
    return !host.muted_007786e0(local, sender_xuid);
}

int chat_key_mode_005d1460(std::uint8_t key, bool down, bool in_mission) noexcept {
    if (down) {
        // 26h with no mission interface manager (00E198C4 == 0): mode 1.
        return (key == 0x26 && !in_mission) ? 1 : 0;
    }
    if (key == 9) {
        return in_mission ? 3 : 1;  // LEA EAX,[EAX+EAX+1]
    }
    return 0;
}
}  // namespace bsp
