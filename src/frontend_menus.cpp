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

HudUnitRef pick_nearest_map_unit_005a1310(const HudMapPickView& view,
    const std::vector<HudMapPickIcon>& icons, bool skip_own_group, HudMapPickHost& host) {
    host.set_hovered_520({});
    // 005A1318..005A1389: the cursor in screen units.
    const float cursor_x = static_cast<float>(
        (static_cast<double>(view.cursor_x_554) - view.origin_x_54c) / view.extent_x_30 +
        kHudMapPickCentre);
    const float cursor_y = static_cast<float>(
        (static_cast<double>(view.origin_z_550) - view.cursor_z_558) / view.extent_z_34 +
        kHudMapPickCentre);
    // 005A1343..005A1366: NEG/SBB/AND, the controlled unit or zero.
    const HudUnitRef own = skip_own_group ? host.controlled_unit_00e188d8() : HudUnitRef{};
    float best = kHudMapPickRadiusSquared;
    const HudMapPickIcon* picked = nullptr;
    for (const HudMapPickIcon& icon : icons) {
        if (!icon.unit || !icon.active) continue;
        if (!host.unit_is_kind_5c(icon.unit, 0x02)) continue;
        if (own && host.unit_owner_140(icon.unit).native == own.native) continue;
        if (host.unit_is_kind_5c(icon.unit, 0x2B)) continue;  // MTorpedo
        if (host.unit_is_kind_5c(icon.unit, 0x34)) continue;  // MWaterMine
        // 005A1432..005A14C9: two position calls, x from the first, z from the second.
        const float world_x = host.unit_position_c4(icon.unit)[0];
        const float map_x = static_cast<float>(
            (static_cast<double>(world_x) - view.origin_x_54c) / view.extent_x_30 + kHudMapPickCentre);
        const float world_z = host.unit_position_c4(icon.unit)[2];
        const float dx = static_cast<float>(static_cast<double>(map_x) - cursor_x);
        const float map_y = static_cast<float>(
            (static_cast<double>(view.origin_z_550) - world_z) / view.extent_z_34 + kHudMapPickCentre);
        const float dy = static_cast<float>(static_cast<double>(map_y) - cursor_y);
        const float dy2 = static_cast<float>(static_cast<double>(dy) * dy);
        // 005A14B7..005A14C9: dx*dx is stored to float before the add.
        const float dx2 = static_cast<float>(static_cast<double>(dx) * dx);
        const float distance = static_cast<float>(static_cast<double>(dy2) + dx2);
        if (best > distance) {
            best = distance;
            picked = &icon;
        }
    }
    if (picked) {
        // 005A1509..005A1565: hover the entity, or its owner when that is a
        // different entity and the pick is not an MLandVehicle (19h).
        HudUnitRef hovered = host.unit_is_kind_5c(picked->unit, 0x02) ? picked->unit : HudUnitRef{};
        host.set_hovered_520(hovered);
        if (hovered) {
            const HudUnitRef owner = host.unit_owner_140(hovered);
            if (owner && host.unit_is_kind_5c(owner, 0x02) && owner.native != hovered.native &&
                !host.unit_is_kind_5c(hovered, 0x19)) {
                host.set_hovered_520(owner);
            }
        }
        if (host.unit_is_kind_5c(picked->unit, 0x05)) return picked->unit;
        if (host.unit_is_kind_5c(picked->unit, 0x18)) return host.squadron_leader_3d0(picked->unit);
    }
    return {};
}

MinimapIconEntry construct_minimap_icon_entry_005bd590(HudUnitRef unit, MinimapIconHost& host) {
    MinimapIconEntry entry{unit, {}};
    MinimapField template_field = MinimapField::BuildingTemplate;
    if (host.unit_is_kind_5c(unit, 0x08)) template_field = MinimapField::SubTemplate;
    else if (host.unit_is_kind_5c(unit, 0x06)) template_field = MinimapField::ShipTemplate;
    else if (host.unit_is_kind_5c(unit, 0x0F)) template_field = MinimapField::PlaneTemplate;
    entry.widget = host.clone_template_00aab4c0(template_field);
    switch (host.unit_side_54(unit)) {
    case 0: host.widget_set_colour_50(entry.widget, MinimapField::BlueColour); break;
    case 1: host.widget_set_colour_50(entry.widget, MinimapField::RedColour); break;
    case 2: host.widget_set_colour_50(entry.widget, MinimapField::GreyColour); break;
    default: break;
    }
    host.attach_to_unit_marker_group_00aaa5a0(entry.widget);
    host.widget_set_visible_34(entry.widget, true);
    return entry;
}

HudUnitRef select_nearby_capture_point_005bd420(const std::array<float, 3>& controlled_position,
    const std::vector<CapturePointCandidate>& points) {
    for (const CapturePointCandidate& point : points) {
        const float dx = controlled_position[0] - point.position[0];
        const float dy = controlled_position[1] - point.position[1];
        const float dz = controlled_position[2] - point.position[2];
        // 005BD4D0..005BD516.
        const float radius = static_cast<float>(point.radius_7c4);
        const float sum = static_cast<float>(
            (static_cast<double>(dx) * dx + static_cast<double>(dy) * dy) +
            static_cast<double>(dz) * dz);
        const float radius_squared = static_cast<float>(static_cast<double>(radius) * radius);
        if (!(sum > radius_squared) && point.side_54 == 2) return point.point;
    }
    return {};
}

bool advance_campaign_selection_005c3de0(MissionTreeScreenState& state,
    std::string_view current_mission_key, MissionAdvanceHost& host) {
    const MissionTreeIndex found = find_mission_by_name_005c3470(state, current_mission_key);
    state.selected_mission = found.mission;
    state.selected_group = found.group;
    if (found.group >= state.groups.size()) host.invalid_parameter_00bf6713();
    const auto& missions = state.groups[found.group].missions;
    const auto count = static_cast<std::uint32_t>(missions.size());
    // 005C3E3E..005C3E6E: an unsigned compare with count - 1.
    if (state.selected_mission < count - 1u) {
        if (state.selected_mission >= count) host.invalid_parameter_00bf6713();
        if (host.mission_completed_0090c560(missions[state.selected_mission])) ++state.selected_mission;
    }
    // Signed clamp to [0, found + 1].
    auto selected = static_cast<std::int32_t>(state.selected_mission);
    const auto found_mission = static_cast<std::int32_t>(found.mission);
    if (selected < 0) selected = 0;
    else if (static_cast<std::int32_t>(static_cast<std::uint32_t>(found_mission) + 1u) < selected) {
        selected = found_mission + 1;
    }
    state.selected_mission = static_cast<std::uint32_t>(selected);
    if (state.selected_mission >= count) host.invalid_parameter_00bf6713();
    host.set_profile_selected_mission_6b8(missions[state.selected_mission].name);
    return found_mission < selected;
}

float adjusted_control_value_0055ccc0(std::int32_t kind, float old_value, std::int32_t delta) noexcept {
    if (kind == 3 || kind == 4) return -0.0f - old_value;  // 0055CD58..0055CD60, SSE
    // 0055CE0E..0055CE1C: FILD, FMUL double 0.25, FADD float, FSTP float.
    float value = static_cast<float>(static_cast<double>(delta) * 0.25 + old_value);
    if (kControlSensitivityMin > value) value = kControlSensitivityMin;   // FCOMIP / JBE
    else if (value > kControlSensitivityMax) value = kControlSensitivityMax;  // COMISS / JBE
    return value;
}

std::int32_t control_slider_position_0055ccc0(float value) noexcept {
    // 0055CE62..0055CE72: _ftol truncates; EDX = -1 - result.
    return -1 - static_cast<std::int32_t>(static_cast<double>(value) * kControlSliderScale);
}

void adjust_control_setting_0055ccc0(std::int32_t delta, ControlSettingHost& host) {
    const std::int32_t kind = host.setting_kind_0055a9a0();
    const float old_value = host.setting_value_00444be0();
    const float value = adjusted_control_value_0055ccc0(kind, old_value, delta);
    host.set_setting_value_006a9e70(value);
    if (kind == 3 || kind == 4) {
        host.set_row_label_00abbe50(value > 0.0f ? kControlNormalLabel : kControlInvertedLabel);
    } else {
        host.set_row_slider_005f3750(control_slider_position_0055ccc0(host.setting_value_00444be0()));
    }
    host.mark_changed_19c();
}

void control_slider_changed_0055d330(ControlSliderListenerHost& host) {
    if (host.page_168() == 3) return;
    const std::uint32_t id = host.widget_id_d8();
    if (id == 0 || (id & 0xFFFF0000u) != 0) return;
    if (host.list_selected_index_00a9c920() != static_cast<std::int32_t>(id & 0xFFu)) return;
    const std::int32_t delta = host.slider_delta_005f2290();
    if (delta != 0) host.adjust_0055ccc0(delta);
}

void control_binding_screen_update_00560430(ControlBindingUpdateState& screen,
    ControlBindingUpdateHost& host) {
    const float step = host.game_frame_seconds_21ec();  // 0056044B..00560460

    // 00560469..0056049C: the hint timer runs down to exactly zero.
    if (screen.hint_timer != 0.0f) {
        screen.hint_timer -= step;
        if (screen.hint_timer < 0.0f) screen.hint_timer = 0.0f;
    }

    // 005604A3..00560688: the Xbox keyboard that names a preset has finished.
    if (screen.keyboard_pending && !host.overlapped_pending_is_io_pending()) {
        screen.keyboard_pending = false;
        if (host.xget_overlapped_result_00a4d42e() == 0) {
            for (char16_t& c : screen.preset_name) {
                if (c == 0) break;
                host.filter_name_char_005ef500(c);
            }
            if (!host.directory_exists_0109ceec_8(kControlPresetDirectory)) {
                host.make_directory_00bf94c6(kControlPresetDirectory);
            }
            const std::string path = host.format_preset_path_sprintf_s(screen.preset_name);
            host.preset_writer_open_00bd5d60(path);
            host.preset_writer_root_00bd6db0(kControlPresetRoot);
            host.write_control_preset_006a5640();
            host.preset_writer_close_00bd5860();
        }
    }

    // 0056068D..00560706: while input keeps arriving the capture and setting
    // pages wait for the hold time to run out; +195h skips one update.
    const bool was_seen = screen.input_seen;
    const bool seen = host.input_pending_00a90f90();
    screen.input_hold -= step;
    screen.input_seen = seen;
    if (was_seen && screen.input_hold > 0.0f) {
        if (screen.page == 3 && !screen.editing_setting) return;
        if (screen.page == 2) return;
    }
    if (screen.skip_next_update) {
        screen.skip_next_update = false;
        return;
    }

    switch (screen.page) {
    case 0:
        // 00560715..00560780
        if (host.action_004d92b0(kControlActionBack)) {
            host.set_global_e19650(0);
            host.push_interface_request_004cc460(0x0C, 0);
            if (!host.x360comp_00f88a30()) host.input_action_configuration_00698a10();
            screen.device_selection.fill(0);
        } else {
            host.page_widget_virtual_34(0);
        }
        return;
    case 1:
        if (host.action_004d92b0(kControlActionBack)) host.refresh_page_0055e3a0(0);
        return;
    case 2: {
        // 005607B2..00560845: a moved slider is remembered.
        ControlInputEvent event;
        float value = 0.0f;
        host.read_input_event_00a90530(event, value);
        if (event.kind == 2 && host.input_event_name_0069e500(event) == kControlSliderEventName) {
            screen.last_slider_value = value;
        }
        if (event.kind == 0 && !screen.editing_setting) {
            if (event.code == kControlKeyDelete) {
                // 0056086F..0056091C: Delete stores the unbound event.
                const ControlBindingKey key = host.binding_key_00427ae0(screen.device_selection[0]);
                ControlInputEvent unbound;
                unbound.code = -1;
                host.set_binding_006a9a40(key, screen.binding_slot, unbound, value);
                host.binding_text_submit_00abaed0(host.binding_slot_text_006a9850(key, screen.binding_slot));
                host.update_binding_row_text_0055c5a0();
                screen.unsaved_changes = true;
                return;
            }
            if (event.code == kControlKeyReturn) {
                // 0056092E..0056094D: UCOMISS/LAHF/JP, so only an exact 0.0f
                // timer starts the capture; otherwise the actions are skipped.
                if (screen.hint_timer == 0.0f) {
                    host.begin_edit_0055c3b0();
                    host.refresh_page_0055e3a0(3);
                    return;
                }
                break;
            }
        }
        // 00560952..005609C9
        if (host.action_004d92b0(kControlActionBack)) {
            host.confirm_or_next_device_00560210();
            return;
        }
        if (host.action_004d92b0(kControlActionLeft)) {
            if (screen.editing_setting) {
                host.adjust_setting_0055ccc0(-1);
            } else {
                screen.binding_slot = 0;
                host.begin_edit_0055c3b0();
            }
        } else if (host.action_004d92b0(kControlActionRight)) {
            if (screen.editing_setting) {
                host.adjust_setting_0055ccc0(1);
            } else {
                screen.binding_slot = 1;
                host.begin_edit_0055c3b0();
            }
        }
        break;
    }
    case 3: {
        // 00560A17..00560B79: capture the next input unless editing a value.
        if (screen.editing_setting || !seen) return;
        ControlInputEvent event;
        float value = 0.0f;
        host.read_input_event_00a90530(event, value);
        if (event.kind == 2 && host.input_event_name_0069e500(event) == kControlSliderEventName) {
            event.flag10 = true;
            // 00560AA1..00560AB8: an unmoved slider is not a binding.
            if (value == screen.last_slider_value) return;
        }
        // 00560ABE..00560AC9: key 1 (DirectInput Escape) leaves the binding as is.
        if (!(event.kind == 0 && event.code == 1)) {
            const ControlBindingKey key = host.binding_key_00427ae0(screen.device_selection[0]);
            host.device_tree_index_0055c110(key);
            host.set_binding_006a9a40(key, screen.binding_slot, event, value);
            host.binding_text_submit_00abaed0(host.binding_slot_text_006a9850(key, screen.binding_slot));
            screen.unsaved_changes = true;
        }
        host.refresh_page_0055e3a0(2);
        host.update_binding_row_text_0055c5a0();
        return;
    }
    case 4:
        // 00560B80..00560BC5: the second 004D92B0 call is a fresh read.
        if (!host.action_004d92b0(kControlActionBack) && !host.input_device_present_004ba6d0(2, 0)) return;
        host.set_global_e19428(host.action_004d92b0(kControlActionBack));
        host.dismiss_prompt_slot_00532a20(3);
        host.refresh_page_0055e3a0(2);
        return;
    default:
        return;
    }

    // 005609CE..00560A04, reached from page 2 only.
    host.scroller_update_00683820();
    if (screen.scroll_dragging || screen.scroll_target != 0) host.scroll_to_selection_0055cec0(0);
    host.focus_00553b00(!screen.editing_setting);
}

void multi_mode_selector_enter_005ec5b0(MultiModeSelectorState& screen, MultiModeSelectorHost& host) {
    using W = MultiModeWidget;
    // 005EC5CE..005EC656: the frame title unless the interface query answers.
    if (!host.interface_query_00687860()) {
        host.set_frame_title_00518d60(host.skirmish_218c() ? kMultiSkirmishTitle : kMultiModeSelectorTitle);
        host.set_frame_flag_00518b90(2);
    }
    const bool any_mode_allowed = host.global_e19610() == 2;  // 005EC65E..005EC66A

    host.widget_virtual(W::ModeList20, 0x60, 0);
    host.listbox_set_listener_00a9ac40(W::ModeList20, false);
    host.listbox_clear_rows_00a9bec0();
    // 005EC6F3..005EC763: six rows; +77h is set when the row is not usable.
    for (std::size_t index = 0; index < kMultiModeLabels.size(); ++index) {
        const MainMenuWidgetRef row = host.add_mode_row();
        host.row_set_label_00abbe50(row, kMultiModeLabels[index]);
        const bool enabled = !(index == 5 && !any_mode_allowed);
        host.row_virtual_34(row, enabled);
        host.row_set_byte_77(row, !(enabled && kMultiModeRowFlags[index] != 0));
    }
    // 005EC765..005EC789: the remembered mode is clamped to 0..4, or 0..5.
    const std::int32_t last = any_mode_allowed ? 5 : 4;
    const std::int32_t remembered = host.global_e19614();
    host.set_global_e19614(remembered < 0 ? 0 : (remembered > last ? last : remembered));

    host.listbox_call_00a9cf30(true, true);
    if (screen.pending_selection != -1) {
        host.listbox_select_index_00a9c7c0(W::ModeList20, screen.pending_selection);
    } else {
        host.listbox_select_first_selectable_00a9c310();
    }
    screen.pending_selection = -1;
    host.listbox_set_listener_00a9ac40(W::ModeList20, true);
    host.widget_virtual(W::ModeList20, 0x80, 1);
    host.widget_virtual(W::ModeList20, 0x60, 1);
    host.widget_virtual(W::Panel1C, 0x34, 0);
    screen.option_list_active = false;
    host.widget_virtual(W::OptionListA8, 0x60, 0);
    host.widget_virtual(W::ModeList20, 0x60, 1);
    host.listbox_set_widget_listener_00aa6bc0(W::ModeList20, true);
    host.listbox_set_listener_00a9ac40(W::ModeList20, true);
    host.listbox_set_widget_listener_00aa6bc0(W::OptionListA8, false);
    host.listbox_set_listener_00a9ac40(W::OptionListA8, false);
    host.input_owner_update_00a92c40(0.0f);
    screen.field_e4 = 8;
    screen.field_e8 = 0;
    screen.field_ec = 0x13;
    screen.field_f0 = 0;
    screen.session_stage = 3;

    // 005EC87B..005EC921: reopen on the option list (focus moves to +A8h).
    if (host.restore_options_e08700()) {
        host.open_options_005ec250();
        host.widget_virtual(W::Panel1C, 0x34, 1);
        screen.option_list_active = true;
        host.widget_virtual(W::OptionListA8, 0x60, 1);
        host.widget_virtual(W::ModeList20, 0x60, 0);
        host.listbox_set_widget_listener_00aa6bc0(W::ModeList20, false);
        host.listbox_set_listener_00a9ac40(W::ModeList20, false);
        host.listbox_set_widget_listener_00aa6bc0(W::OptionListA8, true);
        host.listbox_set_listener_00a9ac40(W::OptionListA8, true);
        host.widget_virtual(W::OptionListA8, 0x34, 1);
        host.set_byte_77(W::OptionListA8, false);
        host.listbox_select_index_00a9c7c0(W::OptionListA8, 0);
        host.layout_options_005ebdd0();
        host.input_owner_update_00a92c40(0.0f);
    }

    // 005EC926..005EC970: refresh the QoS service lookup on a live session.
    if (host.session_present_f8a2fc() && host.session_virtual(0x198, 0)) {
        if (screen.qos_handle != 0) {
            host.xnet_qos_release_00a4d45e(screen.qos_handle);
            screen.qos_handle = 0;
        }
        if (host.xnet_qos_service_lookup_00a4d458(screen.qos_handle) == 0) host.xnet_get_opt_00a4d452();
    }
}

void multi_mode_selector_update_005ecd80(MultiModeSelectorState& screen, float seconds,
    MultiModeSelectorHost& host) {
    using W = MultiModeWidget;
    if (host.main_menu_child_blocked_60_4()) return;  // 005ECD9B..005ECDAE
    // 005ECDB4..005ECDEC: back, unless the accept action fired in the same frame.
    if (host.action_004d92b0(0x4B) && !host.action_004d92b0(0x4A)) {
        host.back_005eca90();
        return;
    }
    if (!screen.option_list_active) {
        // 005ECDFF..005ECF7A
        host.scroller_update_00683820();
        const bool scrolls = host.scroller_has_range_96();
        host.rebuild_command_bar_0054b530({{
            {0xA2, "globals.select", 0},
            {0xB3, "globals.navigate", 1},
            {static_cast<std::uint8_t>(scrolls ? 0xB2 : 0x00), scrolls ? "globals.scroll_menu" : "", 1},
            {0xA3, "globals.back", 2},
            {0x00, "", 1},
        }});
        host.profile_hints_00427190(8);
        return;
    }

    // 005ECF7F..005ECFCB: an empty selection in the idle stage reselects row 0.
    if (host.option_list_selected_00a9c920() == -1 && screen.session_stage == 3) {
        host.listbox_select_index_00a9c7c0(W::OptionListA8, 0);
        host.widget_virtual(W::OptionListA8, 0x60, 1);
        host.set_byte_77(W::OptionListA8, false);
        host.widget_virtual(W::OptionListA8, 0x34, 1);
    }
    if (host.action_004d92b0(0x4C)) {
        host.change_option_005ecba0(-1);
    } else if (host.action_004d92b0(0x4D)) {
        host.change_option_005ecba0(1);
    } else if (host.action_004d92b0(0x4A)) {
        host.open_options_005ec250();
    }
    if (screen.session_stage == 3) {
        host.rebuild_command_bar_0054b530({{
            {0xA2, "FE.create", 0},
            {0xB3, "globals.navigate", 1},
            {0xAF, "globals.change", 1},
            {0xA3, "globals.back", 2},
            {0x00, "", 1},
        }});
    }
    // 005ED17E..005ED19B: stages 0..2 run the timer on the real argument.
    const std::int32_t stage = screen.session_stage;
    if (stage == 0 || stage == 1 || stage == 2) screen.session_timer -= seconds;
    // 005ED1A1..005ED1B4: COMISS 0 with the timer, so only a negative timer.
    if (stage == 1 && 0.0f > screen.session_timer) {
        host.rebuild_command_bar_0054b530({{
            {0x00, "", 0},
            {0x00, "", 1},
            {0x00, "", 1},
            {0xA3, "globals.back", 2},
            {0x00, "", 1},
        }});
    }
    if (screen.session_stage == 0) host.poll_session_005eac60();
    if (screen.session_stage == 1) {
        if (host.session_virtual(0x180, 0)) {
            screen.session_stage = 2;
        } else {
            // 005ED2D9..005ED32E: back cancels only once the timer has expired.
            if (!host.action_004d92b0(0x4B)) return;
            if (!(0.0f > screen.session_timer)) return;
            host.cancel_session_00688c70();
            host.session_virtual(0x170, 0);
            host.session_set_mode_0076fad0(0);
            screen.session_stage = 3;
        }
    }
    if (screen.session_stage == 2) {
        host.push_request_interface_logged_00688ad0(0x14);
        screen.session_stage = 3;
        return;
    }
    // 005ED36A..005ED415: +B0h and +B4h take the option list's height as Y.
    const float y = host.option_list_height_f4();
    for (const W follower : {W::FollowerB0, W::FollowerB4}) {
        std::array<float, 3> position = host.widget_position_00aa6750(follower);
        position[1] = y;
        host.widget_set_position_00aa8240(follower, position);
    }
    host.layout_options_005ebdd0();
    host.profile_hints_00427190(8);
}

void multi_game_lobby_enter_005e51d0(MultiGameLobbyEnterState& screen, MultiGameLobbyEnterHost& host) {
    using W = LobbyWidget;
    if (host.session_present_f8a2fc()) host.session_clear_byte_75();  // 005E51EB..005E51FF

    // 005E5202..005E5349: "<skirmish prefix><mode label>" unless the query answers.
    if (!host.interface_query_00687860()) {
        std::string title = host.skirmish_218c() ? std::string(kLobbySkirmishTitlePrefix) : std::string();
        title += host.option_label_00575e80(kLobbyModeOptionCategory, host.global_e19564());
        host.set_frame_title_00518d60(title);
    }
    // 005E534E..005E537D: one of +130h / +B0h is shown, by skirmish.
    const bool skirmish = host.skirmish_218c();
    host.widget_virtual(W::Panel130, 0x34, skirmish ? 0 : 1);
    host.widget_virtual(W::PanelB0, 0x34, skirmish ? 1 : 0);
    host.option_panel_005ce750();
    if (host.skirmish_218c()) host.session_00771dd0();

    // 005E53B0..005E548C: the player list.
    host.bind_player_list();
    host.player_list_set_byte(0x149, 0);
    host.player_list_set_float(0x12C, kLobbyListScrollA);
    host.player_list_set_float(0x130, kLobbyListScrollB);
    host.player_list_set_float(0x134, 0.0f);
    host.player_list_00a9ceb0(kLobbyListValue, 0);
    host.player_list_set_float(0x128, kLobbyListScrollB);

    // 005E54BF..005E56F4: eight rows stacked under the template by its height.
    host.bind_row_template();
    host.widget_virtual(W::RowTemplate, 0x34, 1);
    for (std::size_t index = 0; index < kLobbyPlayerRows; ++index) {
        host.clone_row(index);
        const float height = host.row_template_height_00aa6740();
        const float row = static_cast<float>(static_cast<std::int32_t>(index));
        const float step_x = static_cast<float>(kLobbyRowStepX * row);
        const float step_y = static_cast<float>(static_cast<double>(row) * height);
        const GuiPoint3 base = host.row_template_position_00aa67f0();
        host.row_set_position_00aa7dc0(index, {
            static_cast<float>(static_cast<double>(step_x) + base.x),
            static_cast<float>(static_cast<double>(base.y) + step_y),
            static_cast<float>(static_cast<double>(base.z) + step_x)});
        host.row_set_byte_78(index, true);
        host.row_set_floats_30_34(index, kLobbyRowField30, 0.0f);
        host.row_child_set_text(index, "player_name_Text", "");
        host.row_child_set_text(index, "player_tag_Text", "");
        host.player_list_append_row_00a9d750(index);
        host.row_set_widget_listener_00aa6bc0(index);
        host.row_set_index_d8(index, static_cast<std::int32_t>(index));
    }
    host.widget_virtual(W::RowTemplate, 0x34, 0);
    host.player_list_select_index_00a9c7c0(0);
    host.player_list_clear_listener_00a9ac40();
    host.player_list_set_byte(0x14A, 0);

    // 005E571D..005E57AE: arrow states by the option panel's +8Ch byte.
    const bool panel_byte = host.option_panel_byte_8c();
    host.widget_virtual_88(W::Arrow144, panel_byte ? 0 : 2);
    host.widget_virtual_88(W::Arrow148, panel_byte ? 2 : 0);
    host.widget_virtual_88(W::Arrow14C, panel_byte ? 2 : 0);

    // 005E57B0..005E5804: JA, so modes 0..3 are also remembered in 00E19568.
    const std::int32_t mode = host.effective_game_mode_004bca50();
    host.set_global(0x00E19564, mode);
    if (static_cast<std::uint32_t>(mode) <= 3) host.set_global(0x00E19568, mode);
    host.set_global(0x00E19594, host.game_dword(0x60C));
    screen.field278 = 1;
    screen.field280 = false;
    screen.field281 = false;
    host.set_global(0x00E08958, -1);
    screen.field2d0 = kLobbyField2D0;
    if (host.game_dword(0x1FE4) == 1) host.lobby_005e20a0();
    screen.field10 = 0;
    host.lobby_005e3580();
    host.lobby_005d5870(0);
    host.lobby_005d67e0(screen.field1c != 0 ? screen.field1c : screen.field_a4);

    // 005E5848..005E58DA: the stored masks are the new values (Ghidra's
    // pseudocode ORs the old +2C0h; the listing stores first).
    if (host.game_dword(0x1FE4) == 1) {
        screen.mode_mask_2c8 = host.mode_mask_005df510(host.effective_game_mode_004bca50());
        screen.mode_mask_2c0 = host.mode_mask_005d9c50(host.effective_game_mode_004bca50());
        const std::uint64_t mask = screen.mode_mask_2c8 | screen.mode_mask_2c0;
        host.game_set_dword(0x2060, static_cast<std::uint32_t>(mask));
        host.game_set_dword(0x2064, static_cast<std::uint32_t>(mask >> 32));
        if (host.game_dword(0x1FE4) == 1) host.session_virtual_174(1);
    }
    host.media_00a4c2d0();

    screen.spectating282 = host.game_byte(0x2164);
    if (screen.spectating282) {
        host.set_e19698_byte4(true);
        host.widget_virtual(W::PlayerList, 0x60, 0);
        host.widget_virtual(W::Panel94, 0x60, 0);
    }
    const std::int32_t slot = host.game_dword(0x18EC);
    if (slot >= 0 && slot <= 7) host.lobby_005d6c10();
    screen.field2d4 = 0;
    screen.field10 = 1;
    host.set_help_variant_00549620(0);
    // 005E5955..005E5A8C: noseart_Group/New_5_Text under +14h.
    host.nose_art_set_text_00abbe50(" ");  // 00CE3A90
    host.nose_art_set_ellipsized_00abb000("FE.multi_lobby_noseart", kLobbyNoseArtWidth);
    host.lobby_005d5340();
    host.game_004bb3e0();
}

} // namespace bsp