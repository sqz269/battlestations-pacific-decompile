#include "bsp/hud_weapon_group_screen.hpp"

// Screen 2Eh's selection helpers and its per-call body 005484F0. Packet
// cc9_screen_2eh_group, docs/SHIP_SCREEN_UPDATE.md sections 27 and 28. Every
// address below is the image's; the control flow is taken from the listing
// (the pseudocode loses the stack arguments of 00548410, 00545370 and
// 00545410). Names are hypotheses, not recovered symbols.

namespace bsp {

std::uint32_t weapon_group_role_mask_00545410(int group) noexcept {
    switch (group) {
    case 1: return 0x04u;   // role 2
    case 2: return 0x0Cu;   // roles 2 and 3
    case 3: return 0x10u;   // role 4
    case 4: return 0x20u;   // role 5
    case 5: return 0x80u;   // role 7
    default: return 0u;
    }
}

namespace {

// 009542B0's group 0 and group 1 need no unit: the switch answers 1 for
// case 0 and 0 for the default.
bool group_available(HudWeaponGroupScreenHost& host, std::size_t unit, int group) {
    if (group == 0) return true;
    if (group < 2 || group > 5) return false;
    return host.group_available_009542b0(unit, group);
}

// 00545410, __thiscall(screen, unit, take), RET 8. 009542B0 on the selected
// group first, then the role message when the group is available or when
// the call releases.
void role_request_00545410(const HudWeaponGroupScreenState& screen,
                           HudWeaponGroupScreenHost& host, std::size_t unit, bool take) {
    const bool available = group_available(host, unit, screen.group_44);
    if (!available && take) return;
    const std::uint32_t mask = weapon_group_role_mask_00545410(screen.group_44);
    if (mask != 0) host.role_request_0077c470(unit, mask, take);
}

// 00545370 and 00545E50 share the group-to-role switch; `test` is the role
// query (one unit, or 00545B80 over every entry).
template <class Test>
bool group_roles(int group, Test test) {
    switch (group) {
    case 0: return true;
    case 1: return test(2);
    case 2: return test(2) || test(3);
    case 3: return test(4);
    case 4: return test(5);
    case 5: return test(7);
    default: return false;
    }
}

}  // namespace

// 00545B30, __thiscall(screen, group), RET 4.
bool weapon_group_any_available_00545b30(const HudWeaponGroupScreenState& screen,
                                         HudWeaponGroupScreenHost& host, int group) {
    for (const HudWeaponGroupEntry& entry : screen.entries) {
        if (entry.unit != 0 && group_available(host, entry.unit, group)) return true;
    }
    return false;
}

// 00545B80, __thiscall(screen, role), RET 4.
bool weapon_group_any_role_00545b80(const HudWeaponGroupScreenState& screen,
                                    HudWeaponGroupScreenHost& host, int role) {
    for (const HudWeaponGroupEntry& entry : screen.entries) {
        if (entry.unit != 0 && host.local_player_role(entry.unit, role)) return true;
    }
    return false;
}

// 00545370, __thiscall(screen, unit), RET 4.
bool weapon_group_unit_roles_00545370(const HudWeaponGroupScreenState& screen,
                                      HudWeaponGroupScreenHost& host, std::size_t unit) {
    return group_roles(screen.group_44,
                       [&](int role) { return host.local_player_role(unit, role); });
}

// 00545E50, __fastcall(screen).
bool weapon_group_roles_held_00545e50(const HudWeaponGroupScreenState& screen,
                                      HudWeaponGroupScreenHost& host) {
    return group_roles(screen.group_44, [&](int role) {
        return weapon_group_any_role_00545b80(screen, host, role);
    });
}

// 00545BD0, __thiscall(screen, take), RET 4.
void weapon_group_set_roles_00545bd0(const HudWeaponGroupScreenState& screen,
                                     HudWeaponGroupScreenHost& host, bool take) {
    for (const HudWeaponGroupEntry& entry : screen.entries) {
        if (entry.unit != 0) role_request_00545410(screen, host, entry.unit, take);
    }
}

// 00548360, __fastcall(screen): the screen-side tests; the host holds the
// applied byte +5h and the rest.
void weapon_group_hint_00548360(const HudWeaponGroupScreenState& screen,
                                HudWeaponGroupScreenHost& host) {
    if (screen.entries.empty() || screen.entries.front().unit == 0) return;
    if (screen.group_44 != 2) return;
    host.lvlaa_hint_00548360();
}

// 00548410, __thiscall(screen, forward, unused), RET 8. The second argument
// is pushed by every caller and never read.
void weapon_group_cycle_00548410(HudWeaponGroupScreenState& screen,
                                 HudWeaponGroupScreenHost& host, bool forward) {
    if (screen.entries.empty() || screen.entries.front().unit == 0) return;
    bool available[6]{};
    bool any = false;
    for (int group = 1; group < 6; ++group) {
        available[group] = weapon_group_any_available_00545b30(screen, host, group);
        any = any || available[group];
    }
    int next = 0;
    if (any) {
        next = screen.group_44;
        do {
            next = (next + (forward ? 1 : 5)) % 6;
        } while (!available[next]);
    }
    if (next == screen.group_44) return;
    weapon_group_set_roles_00545bd0(screen, host, false);
    screen.group_44 = next;
    weapon_group_set_roles_00545bd0(screen, host, true);
    weapon_group_hint_00548360(screen, host);
}

// 005484B0, __thiscall(screen, group), RET 4.
void weapon_group_select_005484b0(HudWeaponGroupScreenState& screen,
                                  HudWeaponGroupScreenHost& host, int group) {
    if (screen.group_44 == group) return;
    if (!weapon_group_any_available_00545b30(screen, host, group)) return;
    weapon_group_set_roles_00545bd0(screen, host, false);
    screen.group_44 = group;
    weapon_group_set_roles_00545bd0(screen, host, true);
    weapon_group_hint_00548360(screen, host);
}

namespace {

// 00548E4A..0054900D and 005488A3..005488FC: the target marks. `gunner` is
// 00545B80(2) || 00545B80(3), [ESP+16h]. Returns [ESP+17h], set when column
// 4 was shown.
bool target_marks(HudWeaponGroupScreenState& screen, HudWeaponGroupScreenHost& host,
                  bool gunner, int row) {
    const std::size_t first = screen.entries.front().unit;
    const std::size_t target = screen.target_54;
    if (!gunner || target == 0) {
        // 005488A3.
        if (target != 0 && host.relation_00803ce0(first, target) == 0 &&
            !host.unit_is_kind_of(target, 0x36) && host.row_widget_present(row, 1) &&
            target != first) {
            host.show_row_widget(row, 1, screen.flag_d4);
        }
        return false;
    }
    // 00548E59: the nearest entry unit holding role 2 or 3 with the target
    // inside its unit+49Ch.
    std::size_t nearest = 0;
    float nearest_distance = 0.0f;
    for (const HudWeaponGroupEntry& entry : screen.entries) {
        const std::size_t unit = entry.unit;
        if (unit == 0) continue;
        if (!host.local_player_role(unit, 2) && !host.local_player_role(unit, 3)) continue;
        const float distance = host.gunner_distance_00548ea9(unit, target);
        if (!(host.gunner_range_49c(unit) > distance)) continue;
        if (nearest == 0 || nearest_distance > distance) {
            nearest = unit;
            nearest_distance = distance;
        }
    }
    // 00548F32..00548F69: the kind-0Fh role-2 query's answer is discarded.
    if (host.target_virtual_10c(target) && host.unit_is_kind_of(first, 0x0F)) {
        static_cast<void>(host.local_player_role(first, 2));
    }
    if (nearest == 0) {
        screen.target_54 = 0;                                          // 0054900D
        return false;
    }
    if (host.relation_00803ce0(nearest, target) == 1) {                // 00548F7C
        if (!host.row_widget_present(row, 4)) return false;
        host.show_row_widget(row, 4, screen.flag_d4);
        return true;
    }
    if (host.unit_is_kind_of(target, 0x36)) return false;
    if (!host.row_widget_present(row, 1)) return false;
    if (host.relation_00803ce0(nearest, target) == 2) return false;    // 00548FDF
    if (target == first) return false;
    host.show_row_widget(row, 1, screen.flag_d4);
    return false;
}

// 00548A2F..0054911A: group 5's held-fire latch.
void hold_latch(HudWeaponGroupScreenState& screen, HudWeaponGroupScreenHost& host) {
    if (screen.group_44 != 5) {
        screen.hold_100 = false;                                        // 0054911A
        return;
    }
    screen.hold_100 = screen.hold_100 || host.input_held(0x99);
    const bool blocked = host.input_held(0x4B) || host.input_held(0xEC);
    screen.hold_100 = screen.hold_100 && !blocked;
    const bool keep = host.input_held(0x99) || !host.camera_busy_00518f30();
    screen.hold_100 = screen.hold_100 && keep;
    if (!screen.hold_100) return;
    const std::size_t first = screen.entries.front().unit;
    if (host.depth_charge_child_00549061(first)) screen.hold_100 = false;
    if (!screen.hold_100) return;
    host.hold_camera_005490a6(first);
}

}  // namespace

bool weapon_group_screen_update_005484f0(HudWeaponGroupScreenState& screen,
                                         HudWeaponGroupScreenHost& host) {
    // 00548512..0054854E.
    if (!screen.mover_40 || screen.entries.empty() || screen.entries.front().unit == 0) {
        return false;
    }
    if (!host.player_unit_present()) return false;
    const std::size_t first = screen.entries.front().unit;
    if (!host.local_player_role(first, 0)) return false;

    // 00548554..0054867A: each one-shot byte clears before its block runs.
    if (screen.one_shot_108) {
        screen.one_shot_108 = false;
        host.one_shot_108_0054856b();
    }
    if (screen.one_shot_109) {
        screen.one_shot_109 = false;
        host.one_shot_109_0054861b(screen.pitch_10c, screen.yaw_110);
    }

    // 0054867A..00548692.
    if (!weapon_group_any_available_00545b30(screen, host, screen.group_44) ||
        screen.group_44 == 0) {
        weapon_group_cycle_00548410(screen, host, true);
    }
    host.hide_widgets_005452f0();
    if (screen.mover_40) host.group_sight_00548300(screen.group_44);

    // 005486A5..005487EF: the widget row.
    int row = 0;
    bool row_shown = false;                                             // [ESP+17h]
    switch (screen.group_44) {
    case 3:
        if (screen.lst_rocket_d6) {
            row = 4;
        } else {
            row = 1;
            host.rocket_widgets_00548716();
        }
        break;
    case 4:
        row = 2;
        break;
    case 5:
        row = 3;
        host.widget_98_005486c7(screen.flag_d5);
        break;
    default:
        break;
    }

    // 005487F4..00548CA7: the key tests.
    if (host.input_pressed(0x99)) screen.fire_press_104 = host.clock_f876a4();
    int key_group = -1;
    if (host.input_pressed(0x9C)) {
        key_group = 2;
    } else if (host.input_pressed(0x9D)) {
        key_group = 3;
    } else if (host.input_pressed(0x9E)) {
        key_group = 4;
    } else if (host.input_pressed(0x9F)) {
        key_group = 5;
    }
    if (key_group >= 0) {
        weapon_group_select_005484b0(screen, host, key_group);
    } else if (host.input_pressed(0x9B) && host.input_held(0x99)) {
        if (host.lock_branch_00548b06(screen)) return true;
    } else if (host.input_axis_active_004d9480(0x9B)) {
        weapon_group_cycle_00548410(screen, host, host.input_axis_value_004c5070(0x9B) > 0.0f);
    } else if (host.input_held(0x99) && screen.group_44 == 4) {
        if (host.torpedo_branch_00548cb6(screen)) return true;
    }

    // 00548839: the mover's aim (0042D7E0, 00521370) feeds only the fire
    // message, which the host builds; then the target.
    screen.target_54 = host.hud_target_00548856();
    if (screen.group_44 != 0) {
        if (weapon_group_roles_held_00545e50(screen, host)) {
            // 0054887D..00548A2D.
            const bool gunner = weapon_group_any_role_00545b80(screen, host, 2) ||
                                weapon_group_any_role_00545b80(screen, host, 3);
            row_shown = target_marks(screen, host, gunner, row);
            // 005488FE..00548975.
            bool has_target = false;
            std::uint16_t target_id = 0;
            const std::size_t target = screen.target_54;
            if (gunner && target != 0 && !host.same_side_54(target, first) &&
                host.target_virtual_10c(target)) {
                target_id = host.target_id_174(target);
                has_target = true;
                host.target_debug_00548942(target);
            }
            const bool pressed = host.input_pressed(0x99);
            const bool held = host.input_held(0x99);
            host.build_fire_message_00954a10(screen.group_44, pressed, held, has_target,
                                             target_id);
            for (const HudWeaponGroupEntry& entry : screen.entries) {
                if (entry.unit != 0 && weapon_group_unit_roles_00545370(screen, host, entry.unit)) {
                    host.route_fire_message_0077c2a0(entry.unit);
                }
            }
            hold_latch(screen, host);
        }
        // 00549134.
        if (screen.group_44 != 0 && !host.input_in_set_00547250(0x99)) {
            if (!row_shown && host.row_widget_present(row, 0)) host.show_row_widget(row, 0, screen.flag_d4);
            const int mode = host.input_mode_f8a0c4();
            const int column = mode == 1 ? 2 : mode == 2 ? 3 : -1;
            if (column >= 0 && host.row_widget_present(row, column)) {
                host.show_row_widget(row, column, screen.flag_d4);
            }
            screen.row_d0 = row;
            return true;
        }
    }
    host.show_widget_c0_005491c2();                                     // 005491C2
    screen.row_d0 = row;
    return true;
}

// 00546A20 (section 37). The names come from the literals the routine
// copies; the column rules are its jump table at 0054708C.
void weapon_group_layout_00546a20(HudWeaponGroupScreenState& screen,
                                  HudWeaponGroupLayoutHost& host) {
    static const char* const kGroups[5] = {
        "ship_AA_Group", "ship_art_Group", "ship_torpedo_Group", "ship_DC_Group",
        "ship_rocket_Group"};
    static const char* const kCells[5] = {
        "cross__Icon", "cross_F_Icon", "cross_H_Icon", "cross_HF_Icon", "cross_L_Icon"};
    screen.row_d0 = 0;                                                  // 00546A30
    const std::uintptr_t ship = host.page_58();
    if (ship != 0) {
        for (int row = 0; row < 5; ++row) {
            const std::uintptr_t group = host.find_child(ship, kGroups[row]);   // 00546AAF
            if (row == 1) {                                             // 00546AC3..00546C1B
                screen.aim_c4 = host.find_child(group, "cross_botton_Icon");
                screen.aim_c8 = host.find_child(group, "cross_left_Icon");
                screen.aim_cc = host.find_child(group, "cross_right_Icon");
            }
            for (int column = 0; column < 5; ++column) {                // 00546C28
                screen.cells[row][column] = 0;
                const bool skip = (column == 1 && row == 3) ||
                                  (column == 3 && (row == 2 || row == 3)) ||
                                  (column == 4 && (row == 1 || row == 2 || row == 3));
                if (!skip) screen.cells[row][column] = host.find_child(group, kCells[column]);
            }
            const std::uintptr_t f_cell = screen.cells[row][1];
            if (f_cell != 0) {                                          // 00546D61
                host.place_f2(f_cell, host.find_child(f_cell, "cross_F2_Icon"), row);
            }
        }
    }
    const std::uintptr_t gunstate = host.page_dc();
    screen.disable_c0 = host.find_child(gunstate, "CrosshairDisable_Icon");  // 00546F27
    screen.gunstate_fc = host.find_child(gunstate, "GunState_Icon");        // 00546FA6
    host.set_visible(screen.gunstate_fc, false);                            // 00546FD7
    screen.circle_d8 = host.find_child(gunstate, "circle_Section");        // 00547030
    screen.layout_done = true;
}

}  // namespace bsp
