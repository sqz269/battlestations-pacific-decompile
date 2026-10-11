// Main-menu list pages, the new-profile text owner and the mission-tree DLC
// gates. Evidence: docs/CC13_FRONTEND_MENUS.md.

#include "bsp/frontend_menus.hpp"

#include <algorithm>
#include <cstring>

namespace bsp {

bool mission_group_has_missions_005805a0_00580530(const MissionTreeTables& tree,
    std::size_t group, FrontEndMenuFailureHost& failure) {
    // 005805A6..005805C5: null begin or (end - begin) / 34h < group + 1.
    if (tree.groups.size() < group + 1) failure.invalid_parameter_00bf6713();
    // 005805CB..005805E9: a null begin answers false, otherwise the signed
    // (end - begin) / 434h is compared with zero.
    return !tree.groups[group].missions.empty();
}

std::array<MainMenuCommandArgument, 5> main_menu_list_page_commands() noexcept {
    // 005852B4..005852ED (00584F50), 005856CE..00585707 (005853C0) and
    // 00588972..005889A7 (005886F0): pushed last-to-first.
    return {{
        {0xA2, "globals.select", 0},
        {0xB3, "globals.navigate", 1},
        {0x00, "", 1},
        {0xA3, "globals.back", 2},
        {0x00, "", 1},
    }};
}

namespace {

template <std::size_t N>
std::string_view label_at(const std::array<std::string_view, N>& labels, std::size_t index) {
    return labels[index];
}

std::string_view page_label(const MainMenuListPageSpec& spec, std::size_t index) {
    switch (spec.label_table) {
    case 0x00E087F0: return label_at(kSinglePlayerLabels, index);
    case 0x00E08820: return label_at(kMultiplayerLabels, index);
    default: return label_at(kTacticalLibraryLabels, index);
    }
}

} // namespace

void build_main_menu_list_page(const MainMenuListPageSpec& spec, MainMenuListPageHost& host) {
    host.set_frame_title_00518d60(0x0C, 0, 0, spec.title);
    if (spec.sets_frame_flag) host.set_frame_flag_00518b90(2);
    host.set_page_00e08874(spec.page);

    if (spec.hides_map_panel_2f0) host.set_widget_visible(MainMenuPageWidget::MapPanel2F0, false);
    host.set_widget_visible(MainMenuPageWidget::Panel3DC, false);
    host.set_widget_visible(MainMenuPageWidget::ListBox1B8, false);
    host.set_widget_visible(MainMenuPageWidget::Panel41C, false);
    if (spec.detaches_listener) host.listbox_set_listener_00a9ac40(false);
    host.listbox_clear_rows_00a9bec0();
    if (spec.starts_title_music) host.start_title_music_005884a0();
    if (spec.marks_listbox_149) host.listbox_set_byte(0x149, 1);

    const bool single_player = spec.label_table == 0x00E087F0;
    for (std::size_t index = 0; index < spec.label_count; ++index) {
        // 00585050..00585079: rows 2 and 3 exist only when their mission group
        // has missions. The tree is read through *(00E198AC)+5Ch each time.
        if (single_player && (index == 2 || index == 3)) {
            const std::size_t group = index == 3 ? kUsnDlcMissionGroup : kIjnDlcMissionGroup;
            if (!mission_group_has_missions_005805a0_00580530(
                    host.mission_tree_00e198ac_5c(), group, host)) {
                continue;
            }
        }
        const MainMenuWidgetRef row = host.clone_row_00aab4c0();
        host.listbox_insert_row_00a9d750(row);
        host.row_set_label_00abbe50(row, page_label(spec, index));
        host.row_set_index_d8(row, static_cast<int>(index));
        if (single_player) {
            // 005850A8..005850B8: +77h = (00CEF784[index] == 0).
            host.row_set_byte_77(row, kSinglePlayerRowFlags[index] == 0);
        }
        if (spec.selects_inside_loop &&
            static_cast<std::int32_t>(index) == host.remembered_selection(spec.selection_global)) {
            // 005850C3..005850D3: the row just inserted, by the list's count.
            host.listbox_select_index_00a9c7c0(host.listbox_row_count_104() - 1);
        }
    }
    if (!spec.selects_inside_loop) {
        host.listbox_select_index_00a9c7c0(host.remembered_selection(spec.selection_global));
    }

    if (spec.detaches_listener) host.listbox_set_listener_00a9ac40(true);
    if (spec.virtual_80_first) {
        host.listbox_virtual(0x80, true);
        host.listbox_virtual(0x34, true);
        host.listbox_virtual(0x60, true);
    } else {
        host.listbox_virtual(0x34, true);
        host.listbox_virtual(0x60, true);
        host.listbox_virtual(0x80, true);
    }
    if (spec.marks_listbox_14a) host.listbox_set_byte(0x14A, 1);
    host.listbox_call_00a9cf30(true, true);
    if (spec.places_from_558) host.listbox_place_from_558_00aa8240();
    if (spec.sets_widget_listener_40) host.listbox_set_widget_listener_00aa6bc0();

    host.rebuild_command_bar_0054b530(main_menu_list_page_commands());
    if (spec.resets_selected_00e08878) host.set_selected_00e08878(-1);
    if (spec.clears_multi_menu_flags) host.clear_multi_menu_flags_00e198b4();
}

std::string mission_map_icon_name(std::string_view prefix, std::int32_t ordinal) {
    std::string name(prefix);
    name += std::to_string(ordinal);
    name += kMapIconSuffix;
    return name;
}

void build_mission_map_icons_0058d9e0(bool destroy, MissionMapIconHost& host) {
    if (destroy) {
        for (const std::uint16_t field : kMissionMapIconDestroyOrder) {
            host.delete_cloned_children_00aa83f0(field);
        }
        host.clear_selected_point_330();
        return;
    }
    for (const MissionMapIconGroup& group : kMissionMapIconGroups) {
        // e.g. 0058DA65..0058DA8F: the tree is re-read through 00E198AC for
        // every group, and the 34h vector must hold tree_group + 1 entries.
        const MissionTreeTables& tree = host.mission_tree_00e198ac_5c();
        if (tree.groups.size() < group.tree_group + 1) host.invalid_parameter_00bf6713();
        const auto& missions = tree.groups[group.tree_group].missions;
        std::int32_t ordinal = 1;
        for (const MissionRecordData& mission : missions) {
            const MainMenuWidgetRef point =
                host.clone_template_00aab4c0(kMapPointTemplateField, group.parent_field);
            host.widget_set_name_2c(point, mission_map_icon_name(kMapPointPrefix, ordinal));
            host.widget_set_local_position_00aa7dc0(point, mission.extra.position);
            host.append_point(group.point_vector, point);
            const MainMenuWidgetRef flag =
                host.clone_template_00aab4c0(kMapFlagTemplateField, group.parent_field);
            host.widget_set_name_2c(flag, mission_map_icon_name(kMapFlagPrefix, ordinal));
            host.widget_set_local_position_00aa7dc0(flag, mission.extra.position);
            if (group.us_flag_frame) host.widget_virtual_88(flag, 1, 0, 1.0f);
            ++ordinal;
        }
    }
}

void new_profile_text_changed_00588370(NewProfileEditState& state, NewProfileTextHost& host) {
    // 00588397..005883FD: length above 15 takes substr(0, 15) and the cursor
    // becomes min(cursor, 15) by a signed compare with 10h.
    if (state.text.size() > kProfileNameMaxLength) {
        state.text.resize(kProfileNameMaxLength);
        if (state.cursor >= 0x10) state.cursor = static_cast<std::int32_t>(kProfileNameMaxLength);
    }
    host.name_set_source_00abaed0(state.text);
    host.name_set_cursor_mode_00ab95f0(host.caret_flag_00e12f34() ? 0 : 1);
    host.name_set_cursor_00ab9240(state.cursor + 1);
}

void new_profile_cancel_005807d0(NewProfileTextHost& host) {
    host.name_set_cstring_00abbe50(kNewProfilePrompt);
    host.name_set_cursor_mode_00ab95f0(2);
    host.deactivate_editor_00a966e0();
    host.reenable_list_box();
}

void trim_left_set_00585eb0(std::string& text, std::string_view set) {
    if (set.empty() || text.empty()) return;
    const std::size_t span = text.find_first_not_of(set);
    if (span == 0) return;
    if (span == std::string::npos) {
        text.clear();
        return;
    }
    text.erase(0, span);
}

std::string normalised_profile_name(std::string_view text) {
    std::string name(text);
    trim_left_set_00585eb0(name, kProfileNameTrimSet);
    // 00584110: trailing bytes in the set are dropped, the first byte is kept.
    if (!name.empty()) {
        std::size_t end = name.size();
        while (end > 1 && kProfileNameTrimSet.find(name[end - 1]) != std::string_view::npos) --end;
        name.resize(end);
    }
    if (name.empty()) name = kDefaultProfileName;
    return name;
}

namespace {

bool equal_ignoring_ascii_case(std::string_view a, std::string_view b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
        const auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c + 32) : c; };
        return lower(x) == lower(y);
    });
}

} // namespace

void new_profile_commit_00590e60(const NewProfileEditState& state, NewProfileCommitHost& host) {
    // 00590E86..00590ECC: the same reset as the cancel slot.
    new_profile_cancel_005807d0(host);
    if (state.text.empty()) return;  // 00590ED0

    const std::string name = normalised_profile_name(state.text);
    // 00590F44..00590F64: a ready storage that refuses the name raises a prompt.
    if (host.storage_ready_21() && !host.storage_can_create_14(name)) {
        host.raise_prompt_00531b00(host.storage_virtual_18() ? kProfileNameConflict
                                                             : kProfileCannotCreate);
        return;
    }
    host.profile_reset_to_defaults_007fdb20();
    host.profile_set_name_007f9290(name);
    if (!host.x360comp_00f88a30()) host.input_settings_reset_006ac030();
    // 00591072..005910BD: 00449AF0 is a case-insensitive inequality test.
    const bool write = !equal_ignoring_ascii_case(name, kDefaultProfileName) &&
                       host.storage_ready_21() && host.storage_can_create_14(name);
    if (write) host.profile_request_write_007fa710(name);
    host.build_top_level_page_00584ae0(false);
}

std::int32_t hud_unit_icon_index_0059cb90(HudUnitRef unit, HudClassRef cls, HudUnitIconHost& host) {
    const bool class_supplied = static_cast<bool>(cls);
    if (!unit) {
        if (!cls) return 0;
    } else if (!cls && (host.unit_is_kind_5c(unit, 0x05) || host.unit_is_kind_5c(unit, 0x18))) {
        // 0059CBC4..0059CBE8: a squadron answers its leader's descriptor.
        cls = host.unit_is_kind_5c(unit, 0x18)
            ? host.unit_class_538(host.squadron_leader_3d0(unit))
            : host.unit_class_538(unit);
    }
    if (cls) {
        if (host.class_is_kind_18(cls, 0x06)) {            // ship
            if (host.class_is_kind_18(cls, 0x0D))          // MBattleship
                return host.class_byte(cls, 0x808) ? 0x17 : 0x02;
            if (host.class_is_kind_18(cls, 0x0A))          // MCruiser, +808h HeavyCruiser
                return host.class_byte(cls, 0x808) ? 0x03 : 0x04;
            if (host.class_is_kind_18(cls, 0x09))          // MMothership
                return host.class_byte(cls, 0x82C) ? 0x06 : 0x05;
            if (host.class_is_kind_18(cls, 0x07)) return 0x08;  // MDestroyer
            if (host.class_is_kind_18(cls, 0x0C)) {        // MLandingShip
                if (host.class_byte(cls, 0x818)) return 0x1A;
                return host.class_byte(cls, 0x81A) ? 0x1B : 0x0A;
            }
            if (host.class_is_kind_18(cls, 0x0B))          // MCargo
                return host.class_byte(cls, 0x809) ? 0x16 : 0x15;
            if (host.class_is_kind_18(cls, 0x0E)) return 0x0D;  // MTorpedoBoat
            return host.class_is_kind_18(cls, 0x08) ? 0x10 : 0x00;  // MSubmarine
        }
        if (host.class_is_kind_18(cls, 0x0F)) {            // plane
            if (!class_supplied) {
                HudUnitRef plane = unit;
                if (host.unit_is_kind_5c(plane, 0x18)) plane = host.squadron_leader_3d0(plane);
                if (host.unit_is_kind_5c(plane, 0x17) && host.unit_byte_c24(plane) == 0) return 0x0B;
                switch (host.plane_projectile_kind_007bca50(plane)) {
                case 0x09: case 0x0D: return 0x01;
                case 0x0A: return 0x11;
                case 0x0B: return 0x07;
                case 0x0F: return 0x0C;
                case 0x12: return 0x0E;
                default: return 0x09;
                }
            }
            if (host.class_is_kind_18(cls, 0x10) || host.class_is_kind_18(cls, 0x12)) return 0x01;
            if (host.class_is_kind_18(cls, 0x11)) return 0x11;
            return host.class_is_kind_18(cls, 0x17) ? 0x0B : 0x09;
        }
        if (host.class_is_kind_18(cls, 0x35)) return 0x0C;  // the dummy-target vehicle
    }
    if (class_supplied || !unit) return 0;
    if (host.unit_is_kind_5c(unit, 0x46)) return 0x0F;     // MShipyard
    if (host.unit_is_kind_5c(unit, 0x45)) return 0x14;     // MAirfield
    if (host.unit_is_kind_5c(unit, 0x1A)) return 0x1E;     // LandConvoy
    if (!host.unit_is_kind_5c(unit, 0x1C)) {               // not MCommandBuilding
        if (host.unit_is_kind_5c(unit, 0x1B)) {            // MLandFort
            for (HudUnitRef m = host.first_member_48(unit); m; m = host.next_member_44(m)) {
                if (host.unit_is_kind_5c(m, 0x20)) return 0x18;
            }
        }
        return 0x19;
    }
    const HudClassRef building = host.unit_class_538(unit);
    if (host.class_byte(building, 0x1A5)) return 0x1C;
    if (host.class_byte(building, 0x1A4)) return 0x1D;
    if (host.class_byte(building, 0x1A8)) return 0x21;
    return host.class_byte(building, 0x1A7) ? 0x20 : 0x1F;
}

} // namespace bsp