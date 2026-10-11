// Front-end and in-game screens of partition segments 12, 15 and 19 (packet cc13-menus4).
// Evidence: docs/CC13_MENU_SCREENS_2.md.

#include "bsp/menu_screens_2.hpp"

namespace bsp {

namespace {

using BW = SessionBrowserWidget;

constexpr std::uint16_t kShow = 0x34;
constexpr std::uint16_t kFocus = 0x60;
constexpr std::uint16_t kToggle = 0x84;

std::array<MainMenuCommandArgument, 5> empty_command_bar() noexcept {
    // 0057A272..0057A30B: five (0, "", 1) triples, pushed last-to-first.
    return {{{0, "", 1}, {0, "", 1}, {0, "", 1}, {0, "", 1}, {0, "", 1}}};
}

std::array<MainMenuCommandArgument, 5> back_only_command_bar() noexcept {
    // 0057A57D..0057A616: the fourth triple is (0A3h, "globals.back" 00CEB3A8, 2).
    return {{{0, "", 1}, {0, "", 1}, {0, "", 1}, {0xA3, "globals.back", 2}, {0, "", 1}}};
}

}  // namespace

// 0057A680, __thiscall(screen), RET; SEH frame 00C6F6A8. ESI = screen throughout.
void session_browser_enter_0057a680(SessionBrowserState& screen, SessionBrowserHost& host) {
    host.prepare_00570300();                                         // 0057A69D
    host.set_frame_title_00518d60(kSessionBrowserTitle);             // 0057A6A2..0057A70D
    // 0057A712..0057A75F: the signed-in answer is kept in a stack byte that is
    // later pushed as a whole dword (EDI at 0057A744); the callees read its low byte.
    const bool signed_in = host.session_query(0x198);
    host.list_row_set_byte_77(BW::FilterList34, 8, !signed_in);      // 0057A730 00A9BE00(8)
    host.list_row_virtual_34(BW::FilterList34, 8, signed_in ? 1 : 0);
    host.widget_set_byte_77(BW::Widget74, !signed_in);
    host.widget_virtual(BW::Widget74, kShow, signed_in ? 1 : 0);
    // 0057A761..0057A7CD: the game-type filter 00E086C0 picks one filter byte pair.
    const std::int32_t filter = host.game_type_filter_e086c0();
    if (filter != -1) {
        for (std::size_t i = 0; i < 5; ++i) {                        // [EAX-9] and [EAX], EAX = +19h..+1Dh
            screen.filter_bytes[i] = false;
            screen.filter_bytes[kSessionBrowserEditFilterBase + i] = false;
        }
        std::size_t pick = 5;
        if (filter == 7) pick = 3;
        else if (filter == 4) pick = 1;
        else if (filter == 5) pick = 2;
        else if (filter == 6) pick = 4;
        else if (filter <= 3) pick = 0;
        if (pick < 5) {
            screen.filter_bytes[pick] = true;
            screen.filter_bytes[kSessionBrowserEditFilterBase + pick] = true;
        }
    }
    host.widget_virtual(BW::Toggle3C, kToggle, 0);                   // 0057A7D0
    screen.refresh_pending = false;                                  // 0057A7EB
    screen.refresh_timer = kSessionBrowserRefreshSeconds;
    screen.refresh_row = 0;
    session_browser_set_page_0057a0f0(screen, 0, host);              // 0057A804
    if (host.online_1fe4()) {                                        // 0057A809..0057A831
        const std::int32_t mode = host.mode_9c();
        if (mode == 6 || mode == 7) session_browser_set_page_0057a0f0(screen, 2, host);
    }
    host.widget_virtual(BW::FilterPanel2C, kShow, 0);                // 0057A836..0057A84C
    host.widget_virtual(BW::FilterPanel2C, kFocus, 0);
    screen.toggle_51 = false;
    screen.toggle_50 = false;
    host.widget_virtual(BW::Toggle3C, kToggle, 0);
    host.widget_virtual(BW::Toggle40, kToggle, 0);
    // 0057A874..0057A8C3: the list pointer is read before the input tick and
    // compared after it; only an unchanged list is cleared or focused.
    const std::uintptr_t list_before = host.widget_identity(BW::SessionList30);
    host.input_owner_update_00a92c40(0.0F);
    if (list_before == host.widget_identity(BW::SessionList30)) {
        if (host.session_count_00566310() == 0) host.list_clear_00a9cdc0(BW::SessionList30);
        else host.widget_virtual(BW::SessionList30, kFocus, 1);
    }
    // 0057A8C5..0057A8E3: the +198h answer is pushed again for widget +D4h.
    host.widget_virtual(BW::WidgetD4, kShow, host.session_query(0x198) ? 1 : 0);
    screen.field_240 = 0.0F;                                         // 0057A8ED
}

// 0056DCB0, __thiscall(screen), RET; SEH frame 00C6EAD8.
void session_browser_exit_0056dcb0(SessionBrowserState& screen, SessionBrowserHost& host) {
    host.set_game_type_filter_e086c0(-1);                            // 0056DCCB
    if (screen.filters_shown_f0) {                                   // 0056DCD5..0056DD3B
        screen.filters_reset_f1 = true;
        host.list_clear_00a9bf90(BW::FilterList34);
        host.reset_cucc_group_00aa83f0();
    }
    host.please_wait_clear_byte_4();                                 // 0056DD40
    host.widget_virtual(BW::FilterPanel2C, kShow, 0);                // 0056DD4A..0056DD60
    host.widget_virtual(BW::FilterPanel2C, kFocus, 0);
}

// 0057A0F0, __thiscall(screen, int page), RET 4; EBP frame, SEH 00C6F690.
void session_browser_set_page_0057a0f0(SessionBrowserState& screen, std::int32_t page, SessionBrowserHost& host) {
    host.input_owner_update_00a92c40(0.0F);                          // 0057A10B..0057A120
    screen.page = page;
    if (!screen.base_byte_05 || !screen.base_byte_04) return;        // 0057A12E..0057A13D
    host.list_set_listener_00a9ac40(BW::SessionList30, false);       // 0057A143
    host.widget_virtual(BW::SessionList30, kFocus, 0);
    host.widget_virtual(BW::SessionList30, kShow, 0);
    host.widget_virtual(BW::Widget88, kShow, 0);
    host.please_wait_clear_byte_4();                                 // 0057A174

    if (screen.page == 0) {
        host.session_call(0x8C);                                     // 0057A18C
        screen.refresh_timer = kSessionBrowserRefreshSeconds;
        bool started = true;                                         // [ESP+0Fh]
        SessionSearchRequest& request = host.search_request_e08700();
        if (!screen.refresh_pending) {
            // 0057A1BE..0057A20F: the type mask from the filter bytes.
            request.refresh_target_e08702 = false;
            request.type_mask_e08730 = 0;
            if (screen.filter_bytes[4]) request.type_mask_e08730 = 0x40;
            if (screen.filter_bytes[0]) request.type_mask_e08730 |= 0x01;
            if (screen.filter_bytes[1]) request.type_mask_e08730 |= 0x10;
            if (screen.filter_bytes[3]) request.type_mask_e08730 |= 0x80;
            if (screen.filter_bytes[2]) request.type_mask_e08730 |= 0x20;
            host.session_start_search_88(request);                   // 0057A216
        } else {
            // 0057A354..0057A3E0: refresh around the selected record.
            const std::int32_t index = host.list_absolute_selection_005661c0();
            const bool online = host.online_1fe4();
            if (online && host.session_count_00566310() > 0 && index >= 0 &&
                static_cast<std::size_t>(index) < host.session_count_00566310()) {
                const SessionRecordView record = host.session_record_005663b0(static_cast<std::size_t>(index));
                request.refresh_target_e08702 = true;
                if (record.kind_04 == 0xFF && record.has_target) {
                    request.target_e08710 = record.target_04;
                    request.target_e08714 = record.target_08;
                }
                host.session_start_search_88(request);               // joins 0057A216
            } else {
                started = false;                                     // 0057A3DB -> 0057A22B
            }
        }
        screen.poll_timer = kSessionBrowserPollSeconds;              // 0057A22B..0057A24A
        screen.field_274 = -1;
        host.reset_search_0056ac40();
        if (!started || screen.refresh_pending) return;              // 0057A24F..0057A261
        host.please_wait_show_004b6e30();                            // 0057A267
        host.rebuild_command_bar_0054b530(empty_command_bar());      // 0057A30B
        return;
    }

    if (screen.page == 1) {
        host.session_call(0x94);                                     // 0057A3EE
        bool rows_built = false;
        if (screen.refresh_pending) {                                // 0057A3FE..0057A424
            screen.refresh_pending = false;
            if (!host.session_query(0x19C)) {
                host.build_rows_default_00576d20();
                rows_built = true;
            }
        }
        if (!rows_built) host.build_rows_005789c0(6);                // 0057A42D
        host.widget_virtual(BW::SessionList30, kShow, 1);            // 0057A434
        host.list_set_listener_00a9ac40(BW::SessionList30, true);    // screen+8
        host.refresh_rows_00577650();
        host.widget_virtual(BW::SessionList30, kFocus, 1);
        host.widget_virtual_40(BW::SessionList30, 0.0F);             // 0057A45F FLDZ
        host.widget_virtual(BW::SessionList30, kFocus, 0);
        if (screen.toggle_50) {                                      // 0057A47B
            host.widget_virtual(BW::Toggle3C, kToggle, 1);
            return;
        }
        if (screen.toggle_51) {                                      // 0057A4A6
            host.widget_virtual(BW::Toggle40, kToggle, 1);
            return;
        }
        if (host.list_has_rows_00a9ac50()) {                         // 0057A4D1..0057A502
            host.select_first_0056aa70();
            host.widget_virtual(BW::EmptyText4C, kShow, 0);
            host.widget_virtual(BW::SessionList30, kShow, 1);
            return;
        }
        host.widget_virtual(BW::Toggle40, kToggle, 1);               // 0057A518..0057A541
        screen.toggle_51 = true;
        host.widget_virtual(BW::EmptyText4C, kShow, 1);
        host.widget_virtual(BW::SessionList30, kShow, 0);
        return;
    }

    if (screen.page == 2) {
        screen.join_elapsed = 0.0F;                                  // 0057A560
        host.refresh_rows_00577650();
        host.please_wait_show_004b6e30();
        host.rebuild_command_bar_0054b530(back_only_command_bar());  // 0057A616
    }
}

// 0057AE50, __thiscall(screen), RET. EBX = screen, EDI = screen+2A0h (the record vector).
void session_browser_join_selected_0057ae50(SessionBrowserState&, SessionBrowserHost& host) {
    if (!host.online_1fe4()) return;                                 // 0057AE51..0057AE60
    const std::size_t count = host.session_count_00566310();         // 0057AE62..0057AE89, inlined
    if (count == 0) return;
    if (host.list_selected_index_00a9c920() < 0) return;             // 0057AE8E
    // 0057AEB9..0057AEC4: the unsigned compare JAE.
    if (static_cast<std::uint32_t>(host.list_selected_index_00a9c920()) >= count) return;
    const std::int32_t index = host.list_absolute_selection_005661c0();  // 0057AECB
    host.join_session_0057a9c0(static_cast<std::size_t>(index), 0);  // 0057AEDB, RET 8
}

// 0057AEF0, listbox-listener slot +04h, ECX = screen+8, RET 8.
void session_browser_on_listbox_activate_0057aef0(SessionBrowserState& screen, bool event_list_is_session_list,
    SessionBrowserHost& host) {
    if (screen.page != 1) return;                                    // 0057AEF0..0057AEFD (0, then 1)
    if (host.widget_byte_85(BW::FilterPanel2C)) return;              // 0057AEFF..0057AF09
    if (!event_list_is_session_list) return;                         // 0057AF0B..0057AF12
    session_browser_join_selected_0057ae50(screen, host);            // 0057AF14 ECX -= 8
}

// 0057B600, __thiscall(screen, float seconds), RET 4. EBX = 0 throughout.
void session_browser_update_0057b600(SessionBrowserState& screen, float seconds, SessionBrowserHost& host) {
    host.layout_00568930();                                          // 0057B607
    const bool listing_ready = host.session_query(0x19C);            // [ESP+0Bh]
    const bool has_sessions = host.session_count_00566310() != 0;    // inlined (end - begin) / 0A0h

    // 0057B620..0057B77D: the empty-list text and the refresh countdown.
    bool reset_refresh = false;
    std::int32_t list_show = 0;
    if (!has_sessions && !host.widget_open_38(BW::FilterPanel2C) && screen.page != 0) {
        host.widget_virtual(BW::EmptyText4C, kShow, 1);              // 0057B65D
        list_show = 0;
        reset_refresh = true;
    } else if (has_sessions && !host.widget_open_38(BW::FilterPanel2C) && screen.page == 1) {
        host.widget_virtual(BW::EmptyText4C, kShow, 0);              // 0057B6BB..0057B6DF
        host.widget_virtual(BW::SessionList30, kShow, 1);
        host.widget_virtual(BW::Widget88, kShow, 1);
        if (!screen.scroller_1e4_22c && screen.scroller_1e4_204 == 0) screen.refresh_timer -= seconds;
        screen.refresh_pending = false;
        if (screen.refresh_timer < 0.0F) {                           // 0057B701..0057B711 COMISS, JBE
            screen.refresh_row = host.list_selected_row_00425e50();
            screen.refresh_timer = kSessionBrowserRefreshSeconds;
            screen.refresh_pending = true;
            session_browser_set_page_0057a0f0(screen, 0, host);
            return;
        }
    } else if (screen.page != 0) {
        host.widget_virtual(BW::EmptyText4C, kShow, 0);              // 0057B750
        list_show = 1;
        reset_refresh = true;
    }
    if (reset_refresh) {                                             // 0057B75D..0057B77D
        host.widget_virtual(BW::SessionList30, kShow, list_show);
        screen.refresh_pending = false;
        screen.refresh_timer = kSessionBrowserRefreshSeconds;
        screen.refresh_row = 0;
    }

    host.filters_tick_0056dd80();                                    // 0057B787
    host.command_bar_tick_0056e3e0();                                // 0057B78E

    // 0057B793..0057B821: one scroller runs.
    if (!host.widget_open_38(BW::FilterPanel2C)) {
        if (screen.toggle_51) {
            host.scroller_update_00683820(0x188);
        } else if (screen.toggle_50) {
            host.scroller_update_00683820(0x12C);
        } else if (host.widget_byte_85(BW::SessionList30)) {
            host.scroller_update_00683820(0x1E4);
            if (screen.scroller_1e4_22c || screen.scroller_1e4_204 != 0) {
                const std::int32_t before = screen.field_2cc;
                host.scroll_follow_0056b610();
                if (screen.field_2cc != before) screen.refresh_timer = kSessionBrowserRefreshSeconds;
            }
        }
    }

    // 0057B829..0057B8B3: action 4Ah toggles the filter panel while +50h is set.
    if (host.action_004d92b0(0x4A) && screen.toggle_50) {
        if (host.widget_open_38(BW::FilterPanel2C)) {
            host.apply_filter_edit_0057a910();
            host.widget_virtual(BW::FilterPanel2C, kShow, 0);
            host.widget_virtual(BW::FilterPanel2C, kFocus, 0);
            host.widget_virtual(BW::FilterList34, kFocus, 0);
        } else {
            host.begin_filter_edit_0056e360();
            host.widget_virtual(BW::FilterPanel2C, kShow, 1);
            host.widget_virtual(BW::FilterPanel2C, kFocus, 1);
            host.widget_virtual(BW::FilterList34, kFocus, 1);
            host.widget_virtual(BW::EmptyText4C, kShow, 0);
            host.widget_virtual(BW::SessionList30, kShow, 1);
        }
    }

    // 0057B8B5..0057B959: back (4Bh).
    if (host.action_004d92b0(0x4B)) {
        if (host.widget_open_38(BW::FilterPanel2C)) {
            host.begin_filter_edit_0056e360();
            host.widget_virtual(BW::FilterPanel2C, kShow, 0);
            host.widget_virtual(BW::FilterPanel2C, kFocus, 0);
            host.widget_virtual(BW::FilterList34, kFocus, 0);
            return;
        }
        if (screen.page == 1) {
            host.back_from_results_00568cb0();
            return;
        }
        if (screen.page == 2) {
            screen.join_elapsed = kSessionBrowserJoinTimeout;
            if (host.search_request_e08700().flag_e08700) screen.join_cancel_requested = true;
        }
        return;
    }

    if (screen.page == 0) {                                          // 0057B95C..0057B9CA
        if (listing_ready) {
            const float remaining = screen.poll_timer - host.frame_seconds_21ec();
            screen.poll_timer = remaining;
            if (!(0.0F < remaining)) host.session_call(0x8C);        // FCOMIP, JC skips
        }
        if (!host.session_query(0x90)) host.poll_session_list_0057b530();
        return;
    }

    if (screen.page == 1) {                                          // 0057B9CD..0057BB96
        if (host.action_004d92b0(0x50) && !host.widget_byte_85(BW::FilterPanel2C)) {
            session_browser_set_page_0057a0f0(screen, 0, host);
            return;
        }
        if (host.action_004d92b0(0x4F) && host.widget_byte_85(BW::SessionList30) && host.session_query(0x198)) {
            // 0057BA34..0057BA4B: XUID first, then the user index; 00A4D440 is stdcall
            // (user, XUID) and pops the two dwords pushed before 00A3EAC0.
            const std::uint64_t xuid = host.selected_player_xuid_00568d40();
            const std::uint32_t user = host.signed_in_user_00a3eac0();
            host.show_gamer_card_00a4d440(user, xuid);
            return;
        }
        if (!host.widget_byte_85(BW::SessionList30) && host.action_004d92b0(0x46)) {
            host.list_action_00568df0(0);
            return;
        }
        if (!host.widget_byte_85(BW::SessionList30) && host.pressed_004c43c0(0x47)) {
            if (!screen.toggle_50 && !host.widget_byte_85(BW::FilterPanel2C)) {
                screen.toggle_51 = false;
                host.widget_virtual(BW::Toggle40, kToggle, 0);
                host.widget_virtual(BW::Toggle3C, kToggle, 1);
                screen.toggle_50 = true;
                host.widget_virtual(BW::SessionList30, kFocus, 0);
            }
            return;
        }
        if (host.action_004d92b0(0x4C)) {
            if (host.widget_byte_85(BW::FilterPanel2C)) host.filter_left_0056ee00();
            else host.list_left_0056aad0();
            return;
        }
        if (host.action_004d92b0(0x4D)) {
            if (host.widget_byte_85(BW::FilterPanel2C)) {
                host.filter_right_0056efb0();
            } else if (host.widget_byte_85(BW::SessionList30)) {
                host.widget_virtual(BW::Toggle40, kToggle, 1);
                screen.toggle_51 = true;
                host.widget_virtual(BW::SessionList30, kFocus, 0);
            }
        }
        return;
    }

    if (screen.page != 2) return;
    // 0057BBA2..0057BBD8: elapsed += min(seconds, 0.1); 00415510 returns on the x87 stack.
    const float step = seconds < kSessionBrowserJoinStepCap ? seconds : kSessionBrowserJoinStepCap;
    screen.join_elapsed = step + screen.join_elapsed;
    const bool flag = host.search_request_e08700().flag_e08700;
    if (!(screen.join_elapsed < kSessionBrowserJoinTimeout)) {       // COMISS, JC
        if (flag) {
            host.join_finished_0057ac10();                           // 0057BC4F
            return;
        }
        host.cancel_session_00688c70();                              // 0057BBE8
        if (!host.held_004c5090(0x4B)) {
            host.push_request_interface_logged_00688ad0(0x1B);
            host.error_screen_show_005d2e70(4);
        }
        screen.page = 0;
        return;
    }
    const std::int32_t mode = host.mode_9c();                        // 0057BC30, EDI
    if (mode == 5) {
        if (flag) {
            host.join_finished_0057ac10();
            return;
        }
        host.cancel_session_00688c70();                              // 0057BC60..0057BC90
        host.push_request_interface_logged_00688ad0(0x1B);
        host.error_screen_show_005d2e70(4);                          // tail JMP with [ESP+4] = 4
        return;
    }
    if (mode == 7 && host.occupied_player_slots() > 1 && host.field_18c8() >= 0 && host.check_004b6bc0()) {
        host.pop_request_interface_006881f0();                       // 0057BD0E..0057BD43
        host.push_request_interface_logged_00688ad0(0x14);
        screen.page = 0;
        screen.poll_timer = kSessionBrowserPollSeconds;
        return;
    }
    if (mode == 6 && host.join_status_00a39750() == kSessionBrowserJoinStatusReady) {
        host.session_set_00076a7a0(screen.field_24);                 // 0057BD5D..0057BD76
        session_browser_set_page_0057a0f0(screen, 2, host);
    }
}

// 0056ED20, SEH 00C6ECD0. The selected row of +34h, its "setting_value_Text" child, 00ABAED0(text, 1).
void session_browser_set_filter_value_text_0056ed20(std::string_view text, SessionBrowserFilterHost& host) {
    const std::uintptr_t row = host.filter_list_selected_row_00425e50();            // 0056ED43
    const std::uintptr_t label = host.find_child_00aa7e00(row, kSessionBrowserFilterValueText);  // 0056ED92
    host.text_submit_00abaed0(label, text);                                          // 0056ED99
}

namespace {

// Shared shape of 0056EE00 and 0056EFB0: rows 0..4 (and any negative index, the
// signed JG 4 at 0056EE0F) and row 8 toggle a yes/no byte; rows 5..7 step a count
// whose non-zero state is mirrored into the byte; a zero count reads "globals.no".
void filter_value_step(SessionBrowserState& screen, SessionBrowserFilterHost& host, bool up) {
    const std::int32_t row = host.filter_list_selected_index_00a9c920();  // 0056EE04..0056EE07
    auto edit_byte = [&](std::int32_t index) -> bool& {
        return screen.filter_bytes[static_cast<std::size_t>(static_cast<std::int32_t>(kSessionBrowserEditFilterBase) + index)];
    };
    if (row <= 4) {                                                  // 0056EE0C..0056EE35
        bool& value = edit_byte(row);
        value = !value;
        host.set_game_type_filter_e086c0(-1);
        session_browser_set_filter_value_text_0056ed20(value ? kSessionBrowserYes : kSessionBrowserNo, host);
        return;
    }
    if (row == 8) {                                                  // 0056EF6B..0056EFA7
        bool& value = edit_byte(8);
        value = !value;
        session_browser_set_filter_value_text_0056ed20(value ? kSessionBrowserYes : kSessionBrowserNo, host);
        return;
    }
    if (row > 7) return;                                             // 0056EF6E JNZ: rows above 8 do nothing
    std::int32_t& count = screen.edit_counts[static_cast<std::size_t>(row - 5)];
    if (row == 7) {                                                  // 0..500 in steps of 100, wrapping
        if (up) count = count < 500 ? count + 100 : 0;
        else count = count == 0 ? 500 : count - 100;
    } else {                                                         // rows 5 and 6: 0..7, wrapping
        if (up) count = count == 7 ? 0 : count + 1;
        else count = count == 0 ? 7 : count - 1;
    }
    bool& value = edit_byte(row);                                    // +1Eh, +1Fh, +20h
    value = count != 0;
    if (value) session_browser_set_filter_value_text_0056ed20(host.format_int_004260b0(count), host);
    else session_browser_set_filter_value_text_0056ed20(kSessionBrowserNo, host);
}

}  // namespace

void session_browser_filter_value_step_0056ee00(SessionBrowserState& screen, SessionBrowserFilterHost& host) {
    filter_value_step(screen, host, false);
}

void session_browser_filter_value_step_0056efb0(SessionBrowserState& screen, SessionBrowserFilterHost& host) {
    filter_value_step(screen, host, true);
}

namespace {

// The "select toggle 3Ch" block shared by 0057B0E6.. (+3Ch), +BCh, +A8h and +ACh.
void browser_select_toggle_3c(SessionBrowserState& screen, SessionBrowserHost& host) {
    screen.toggle_51 = false;
    host.widget_virtual(BW::Toggle40, kToggle, 0);
    host.widget_virtual(BW::Toggle3C, kToggle, 1);
    screen.toggle_50 = true;
    host.widget_virtual(BW::SessionList30, kFocus, 0);
}

}  // namespace

// 0057AF20, ESI = screen+0Ch, EDI = event. Paths that end in a tail JMP or RET 4 skip
// the closing kind test at 0057B24B; the others fall into it.
void session_browser_handle_gui_event_0057af20(SessionBrowserState& screen, SessionBrowserHost& host,
    SessionBrowserEventHost& event) {
    if (event.event_kind_5c() == kGuiEventCommandButton) {           // 0057AF26..0057AF34
        const std::uint8_t command = event.event_command();          // re-read per test in the original
        if (command == 0xA2) {                                       // 0057AF49
            if (host.widget_open_38(BW::FilterPanel2C)) {            // 0057AF52..0057AF92
                host.apply_filter_edit_0057a910();
                host.widget_virtual(BW::FilterPanel2C, kShow, 0);
                host.widget_virtual(BW::FilterPanel2C, kFocus, 0);
                host.widget_virtual(BW::FilterList34, kFocus, 0);
                return;
            }
            if (screen.toggle_50 && !host.widget_open_38(BW::FilterPanel2C)) {  // 0057AF94..0057AFF2
                host.begin_filter_edit_0056e360();
                host.widget_virtual(BW::FilterPanel2C, kShow, 1);
                host.widget_virtual(BW::FilterPanel2C, kFocus, 1);
                host.widget_virtual(BW::FilterList34, kFocus, 1);
                host.widget_virtual(BW::EmptyText4C, kShow, 0);
                host.widget_virtual(BW::SessionList30, kShow, 1);
                return;
            }
            session_browser_join_selected_0057ae50(screen, host);    // 0057AFF4
            return;
        }
        if (command == 0xA3) {                                       // back
            if (host.widget_open_38(BW::FilterPanel2C)) {
                host.begin_filter_edit_0056e360();
                host.widget_virtual(BW::FilterPanel2C, kShow, 0);
                host.widget_virtual(BW::FilterPanel2C, kFocus, 0);
                host.widget_virtual(BW::FilterList34, kFocus, 0);
                return;
            }
            host.back_from_results_00568cb0();
            return;
        }
        if (command == 0xA7) {                                       // refresh
            session_browser_set_page_0057a0f0(screen, 0, host);
            return;
        }
        if (command != 0xA5) return;                                 // 0057B084..0057B096
        if (!host.widget_byte_85(BW::SessionList30)) return;         // 0057B09C..0057B0A6
        if (!event.session_signed_in_198()) return;                  // 0057B0AC..0057B0BE
        const std::uint64_t xuid = host.selected_player_xuid_00568d40();  // 0057B0CD
        const std::uint32_t user = host.signed_in_user_00a3eac0();   // 0057B0D6
        host.show_gamer_card_00a4d440(user, xuid);                   // 0057B0DC
        return;
    }

    if (event.event_is(0x40)) {                                      // 0057B0E6..0057B13D
        if (screen.toggle_51 || host.widget_byte_85(BW::FilterPanel2C)) return;
        host.widget_virtual(BW::Toggle40, kToggle, 1);
        screen.toggle_50 = false;
        screen.toggle_51 = true;
        host.widget_virtual(BW::Toggle3C, kToggle, 0);
        host.widget_virtual(BW::SessionList30, kFocus, 0);           // tail JMP
        return;
    }
    if (event.event_is(0x3C)) {                                      // 0057B13F..0057B196
        if (screen.toggle_50 || host.widget_byte_85(BW::FilterPanel2C)) return;
        browser_select_toggle_3c(screen, host);                      // ends in a tail JMP
        return;
    }
    // 0057B198..0057B1F2: four column headers rebuild the rows with a sort key.
    if (event.event_is(0x2B4)) { host.build_rows_005789c0(0); return; }
    if (event.event_is(0x2B8)) { host.build_rows_005789c0(1); return; }
    if (event.event_is(0x2BC)) { host.build_rows_005789c0(5); return; }
    if (event.event_is(0x2C0)) { host.build_rows_005789c0(4); return; }

    if (event.event_is(0xBC)) {                                      // 0057B1F4..0057B246
        if (!screen.toggle_50 && !host.widget_byte_85(BW::FilterPanel2C)) browser_select_toggle_3c(screen, host);
        event.scroller_begin_thumb_drag_00683a40(0x12C);
    } else if (event.event_is(0xA8)) {                               // 0057B278..0057B2D8
        if (!screen.toggle_50 && !host.widget_byte_85(BW::FilterPanel2C)) browser_select_toggle_3c(screen, host);
        event.scroller_set_auto_scroll_00683300(0x12C, 2, kSessionBrowserAutoScrollStep, 1);
    } else if (event.event_is(0xAC)) {                               // 0057B2E2..0057B342
        if (!screen.toggle_50 && !host.widget_byte_85(BW::FilterPanel2C)) browser_select_toggle_3c(screen, host);
        event.scroller_set_auto_scroll_00683300(0x12C, 1, kSessionBrowserAutoScrollStep, 1);
    } else if (event.event_is(0xB8)) {                               // 0057B34C..0057B354 -> 0057B246
        event.scroller_begin_thumb_drag_00683a40(0x188);
    } else if (event.event_is(0xB0)) {                               // 0057B35F..0057B37B
        event.scroller_set_auto_scroll_00683300(0x188, 2, kSessionBrowserAutoScrollStep, 1);
    } else if (event.event_is(0xB4)) {                               // 0057B385..0057B3A1
        event.scroller_set_auto_scroll_00683300(0x188, 1, kSessionBrowserAutoScrollStep, 1);
    } else if (event.event_is(0x78) || event.event_is(0x80) || event.event_is(0x7C)) {
        // 0057B3AB..0057B486: the result-list scroller, which also restarts the refresh countdown.
        const bool thumb = event.event_is(0x78);
        const std::int32_t direction = event.event_is(0x80) ? 2 : 1;
        if (host.widget_byte_85(BW::FilterPanel2C)) return;
        host.list_left_0056aad0();
        if (!host.widget_byte_85(BW::SessionList30)) return;
        if (thumb) event.scroller_begin_thumb_drag_00683a40(0x1E4);
        else event.scroller_set_auto_scroll_00683300(0x1E4, direction, kSessionBrowserAutoScrollStep, 1);
        screen.refresh_timer = kSessionBrowserRefreshSeconds;        // 0057B3E3..0057B3EB
    } else {
        // 0057B48B..0057B504: the filter panel's row arrows select the row, then step it.
        if (!host.widget_open_38(BW::FilterPanel2C)) return;
        if (event.event_name_equals(kSessionBrowserArrowRight)) {
            event.filter_list_select_00a9c8f0(event.event_row_d8());
            host.filter_right_0056efb0();
            return;
        }
        if (!event.event_name_equals(kSessionBrowserArrowLeft)) return;
        event.filter_list_select_00a9c8f0(event.event_row_d8());
        host.filter_left_0056ee00();
        return;
    }
    // 0057B24B..0057B51F: highlight the pressed widget by its kind.
    if (event.event_kind_5c() == 6) {
        event.event_virtual_88(2, 0, 1.0F);
    } else if (event.event_kind_5c() == 0x12) {
        event.event_virtual_84(2);
    }
}

// 0056E3E0. Each branch builds five (command, label, placement) triples for 0054B530 on
// *(00E1930C); the triples below are in argument order (pushed last-to-first).
bool session_browser_command_bar_0056e3e0(const SessionBrowserCommandBarView& view,
    std::array<MainMenuCommandArgument, 5>& arguments) {
    if (view.filter_list_focused) {                                  // 0056E40E falls through
        arguments = {{{0xA2, "globals.accept", 0}, {0xB3, "globals.navigate", 1}, {0xAF, "globals.change", 1},
            {0xA3, "globals.cancel", 2}, {0x00, "", 1}}};            // 0056E414..0056E4AD
        return true;
    }
    if (view.toggle_50) {                                            // 0056E55E..0056E601
        arguments = {{{0xB3, "globals.navigate", 1}, {0xB2, "globals.scroll_menu", 1},
            {0xA2, "FE.change_filter", 1}, {0xA7, "globals.refresh", 1}, {0xA3, "globals.back", 2}}};
        return true;
    }
    if (!view.session_list_focused) {                                // 0056E6B5..0056E6BC
        if (!view.toggle_51) return false;                           // 0056EBAC..0056EBB0
        arguments = {{{0xB3, "globals.navigate", 1}, {0xB2, "globals.scroll_menu", 1}, {0x00, "", 1},
            {0xA7, "globals.refresh", 1}, {0xA3, "globals.back", 2}}};  // 0056EBB6..0056EC4F
        return true;
    }
    // 0056E6C8..0056E70C: online, a non-empty record vector and an in-range selection
    // (unsigned compare against the count).
    const bool selected = view.online && view.session_count != 0 && view.selection >= 0 &&
        static_cast<std::size_t>(static_cast<std::uint32_t>(view.selection)) < view.session_count;
    if (!selected) {
        arguments = {{{0xB3, "globals.navigate", 1}, {0x00, "", 1}, {0xA7, "globals.refresh", 1},
            {0xA3, "globals.back", 2}, {0x00, "", 1}}};              // 0056EA99..0056EB32
        return true;
    }
    if (!view.signed_in) {                                           // 0056E71A..0056E728 -> 0056E8D0
        if (view.byte_264) {
            arguments = {{{0xB3, "globals.navigate", 1}, {0xA2, "globals.join", 1}, {0xA7, "globals.refresh", 1},
                {0xA3, "globals.back", 2}, {0x00, "", 1}}};          // 0056E8E7..0056E972
        } else {
            arguments = {{{0xB3, "globals.navigate", 1}, {0x00, "", 1}, {0xA7, "globals.refresh", 1},
                {0xA3, "globals.back", 2}, {0x00, "", 1}}};          // 0056E9AF..0056EA3A
        }
        return true;
    }
    if (view.byte_264) {
        arguments = {{{0xB3, "globals.navigate", 1}, {0xA2, "globals.join", 1}, {0xA5, "globals.showgamercard", 1},
            {0xA7, "globals.refresh", 1}, {0xA3, "globals.back", 2}}};  // 0056E735..0056E7D0
    } else {
        arguments = {{{0xB3, "globals.navigate", 1}, {0x00, "", 1}, {0xA5, "globals.showgamercard", 1},
            {0xA7, "globals.refresh", 1}, {0xA3, "globals.back", 2}}};  // 0056E808..0056E898
    }
    return true;
}

// 0052E150, EDI = screen, EBP = index.
void credits_exit_0052e150(std::vector<std::uintptr_t>& items, CreditsExitHost& host) {
    host.stream54_request_stop_00a86f40();                           // 0052E172..0052E175
    const std::string name = host.menu_music_name_e18e64();          // 0052E17A..0052E1C4 strlen + copy
    host.stream50_start_00a867b0(name);                              // 0052E1C7..0052E1DC
    host.stream50_request_fade_in_00a85c20();                        // 0052E208..0052E211
    host.gui_pause_movie_pages_00aa33a0(0);                          // 0052E216..0052E21F
    // 0052E226..0052E26D: the bound is re-read every pass; 00BF6713 guards the index.
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (items[i] != 0) {
            host.item_scalar_delete(items[i]);
            items[i] = 0;
        }
    }
}

// 0052E4B0, __thiscall(screen, float), RET 4. ESI = screen.
void credits_update_0052e4b0(CreditsState& screen, float /*seconds*/, CreditsHost& host) {
    // 0052E4B4..0052E4C4: the argument slot [ESP+10h] receives game+21ECh.
    const float seconds = host.frame_seconds_21ec();
    if (host.action_004d92b0(0x4B)) host.push_interface_request_004cc460(1, 0);  // 0052E4CA..0052E4DD
    // 0052E4E2..0052E554: 46h slower, 47h faster (x87, double step), 4Ch pause, 4Dh normal.
    if (host.action_004d92b0(0x46)) {
        screen.speed_offset = static_cast<float>(screen.speed_offset - kCreditsSpeedStep);
    } else if (host.action_004d92b0(0x47)) {
        screen.speed_offset = static_cast<float>(screen.speed_offset + kCreditsSpeedStep);
    } else if (host.action_004d92b0(0x4C)) {
        screen.speed_offset = -0.0F - screen.base_speed;             // 00D7A208 is -0.0
    } else if (host.action_004d92b0(0x4D)) {
        screen.speed_offset = 0.0F;
    }
    // 0052E559..0052E578: COMISS clamp to [-1, 1].
    if (screen.speed_offset > kCreditsSpeedMax) screen.speed_offset = kCreditsSpeedMax;
    else if (kCreditsSpeedMin > screen.speed_offset) screen.speed_offset = kCreditsSpeedMin;
    // 0052E57D..0052E58F: the speed is summed before +34h is wrapped.
    const float speed = screen.base_speed + screen.speed_offset;
    if (screen.scroll_y > 0.0F) screen.scroll_y = -0.0F - screen.content_height;  // 0052E580..0052E5A2
    if (!screen.has_scroll_widget) return;                           // 0052E5A7..0052E5AC
    host.widget_set_local_y_00aa78f0(screen.scroll_y);               // 0052E5B5
    // 0052E5BA..0052E5E0: x87 product rounded to float in the argument slot.
    const float moved = static_cast<float>(screen.scroll_y - static_cast<double>(speed) * seconds);
    screen.scroll_y = moved;
    if (-screen.content_height > moved) screen.scroll_y = 0.0F;
}

// 00517820, __thiscall(screen), RET.
void in_game_screen23_enter_00517820(ObservedUnitScreenState& screen, ObservedUnitScreenHost& host) {
    host.game_set_view_fov_004dc940(1.0F, 1);                        // 00517820..00517831 FLD1
    if (screen.unit_1c == 0) return;                                 // 00517836..0051783B
    if (host.observer_registered_00694af0(screen.unit_1c)) return;   // 00517843..0051784A
    host.observer_register_00694a60(screen.unit_1c);                 // 00517853 tail JMP
}

// 00517860, __thiscall(screen), RET.
void in_game_screen23_exit_00517860(ObservedUnitScreenState& screen, ObservedUnitScreenHost& host) {
    if (screen.unit_1c != 0) host.unit_send_0077c470(screen.unit_1c, kObservedUnitReleaseCode, 0);
}

// 00519120, __thiscall(screen), RET.
void hud_bomber_enter_00519120(ObservedUnitScreenState& screen, ObservedUnitScreenHost& host) {
    host.weapon_group_bind_unit_00549260(screen.unit_1c, screen.field_28);  // 00519120..00519134
    if (screen.unit_1c == 0) return;
    if (host.observer_registered_00694af0(screen.unit_1c)) return;   // 00519146
    host.observer_register_00694a60(screen.unit_1c);                 // 00519156 tail JMP
}

// 00519160, __thiscall(screen), RET.
void hud_bomber_exit_00519160(ObservedUnitScreenState& screen, ObservedUnitScreenHost& host) {
    host.weapon_group_release_005470c0();                            // 0051916B
    if (screen.unit_1c != 0) {                                       // 00519170..00519194
        host.unit_send_0077c470(screen.unit_1c, kObservedUnitReleaseCode, 0);
        const std::uintptr_t child = host.unit_child_9d4(screen.unit_1c);
        if (child != 0) host.unit_send_0077c470(child, 4, 0);
    }
    host.screen26_call_0051e8e0();                                   // 005191A3 tail JMP
}

// 00528C40 / 00528C50, __thiscall(screen), RET.
void hud_counters_enter_00528c40(HudCountersHost& host) { host.widget8_virtual_34(1); }
void hud_counters_exit_00528c50(HudCountersHost& host) { host.widget8_virtual_34(0); }

// 0052BAA0, __thiscall(screen, float seconds), RET 4. ESI = screen, EDI = index.
void hud_counters_update_0052baa0(const HudCountersState& screen, float seconds, HudCountersHost& host) {
    host.counter_tick_0052b080(screen.counter_0c, seconds);          // 0052BAA9..0052BAAF
    for (const std::uintptr_t counter : screen.counters_14) {        // 0052BAB4..0052BAE1
        if (counter != 0) host.counter_tick_0052b080(counter, seconds);
    }
    host.counter_tick_0052b080(screen.counter_10, seconds);          // 0052BAE3..0052BAEE
    host.finish_00529810();                                          // 0052BAF5
}

// 00517280, ECX = screen+64h, RET 8. ESI = listener.
void tactical_library_on_listbox_activate_00517280(TacticalLibraryScrollState& screen, bool list_is_main_list,
    TacticalLibraryActivateHost& host) {
    if (!list_is_main_list) return;                                  // 00517280..0051728A
    // 0051728C..005172A5: the row's +28h text against "99999"; the push at
    // 00517295 is 00425850's argument (RET 4), the row virtual takes none.
    if (host.row_text_equals_insensitive(kTacticalLibrarySpecialRow)) {
        host.open_special_00515e00();                                // 005172A7 ECX = listener-64h
        return;
    }
    // 005172B3..005172E9: the count is FILD-ed and rounded to float in the
    // argument slot; FCOMI decides each store.
    const float count = static_cast<float>(screen.row_count);
    if (count > screen.scroll_value) screen.scroll_value = static_cast<float>(count - kTacticalLibraryScrollStep);
    if (screen.scroll_value > count) screen.scroll_value = static_cast<float>(count + kTacticalLibraryScrollStep);
}

// 00518D60. The three slot indices go to 00E18D84/00E18D88/00E18D8C before anything else.
void front_end_frame_select_00518d60(std::int32_t backdrop, std::int32_t panel, std::int32_t title,
    std::string_view text, FrontEndFrameHost& host) {
    FrontEndFrameSelection& selection = host.selection();
    selection.backdrop_e18d84 = backdrop;                            // 00518D64
    selection.panel_e18d88 = panel;                                  // 00518D6A
    selection.title_e18d8c = title;                                  // 00518D70
    front_end_frame_apply_selection_00518c80(host);                  // 00518D75..00518D7C
    if (!text.empty()) front_end_frame_set_title_005189b0(text, host);  // 00518D81..00518D8A: length word
}

// 00518C80, ESI = frame, EDI = slot. Each existing handle of the current set is shown
// exactly when its slot is the selected one (SETE AL into virtual +34h).
void front_end_frame_apply_selection_00518c80(FrontEndFrameHost& host) {
    FrontEndFrameState& frame = host.frame_004c1ac0();
    const FrontEndFrameSelection& selection = host.selection();
    const auto set = static_cast<std::size_t>(frame.set);
    for (std::int32_t slot = 0; slot < 2; ++slot) {                 // 00518C90..00518CC0
        const std::uintptr_t handle = frame.panels[set * kFrontEndFramePanelSlots + slot];
        if (handle != 0) host.widget_virtual_34(handle, slot == selection.panel_e18d88);
    }
    for (std::int32_t slot = 0; slot < 2; ++slot) {                 // 00518CD0..00518D00
        const std::uintptr_t handle = frame.titles[set * kFrontEndFrameTitleSlots + slot];
        if (handle != 0) host.widget_virtual_34(handle, slot == selection.title_e18d8c);
    }
    if (host.game_mode_5d4() == kFrontEndFrameSkipBackdropMode) return;  // 00518D02..00518D0E
    for (std::int32_t slot = 0; slot < 13; ++slot) {                // 00518D12..00518D4A
        const std::uintptr_t handle = frame.backdrops[set * kFrontEndFrameBackdropSlots + slot];
        if (handle != 0) host.widget_virtual_34(handle, slot == selection.backdrop_e18d84);
    }
    front_end_frame_set_flag_00518b90(selection.flag_e08654, host);  // 00518D4C..00518D54 tail JMP
}

// 00518B90. The flag is stored even when no panel is selected.
void front_end_frame_set_flag_00518b90(std::uint32_t flag, FrontEndFrameHost& host) {
    FrontEndFrameSelection& selection = host.selection();
    selection.flag_e08654 = flag;                                    // 00518BAF
    if (selection.panel_e18d88 < 0) return;                          // 00518BA8..00518BB5
    FrontEndFrameState& frame = host.frame_004c1ac0();
    const std::uintptr_t panel =
        frame.panels[static_cast<std::size_t>(frame.set) * kFrontEndFramePanelSlots + selection.panel_e18d88];
    if (panel == 0) return;                                          // 00518BD8..00518BDC
    const std::uintptr_t icon = host.find_child_00aa7e00(panel, kFrontEndFrameFlagIcon);  // 00518C1D
    // 00518C22..00518C3C: virtual +88h(word 00E08654, 0, 1.0F).
    host.widget_virtual_88(icon, static_cast<std::uint16_t>(selection.flag_e08654), 0, 1.0F);
}

// 005189B0, EBP = the string, EBX = title_Text, ESI = szarnyas_FrameBox.
void front_end_frame_set_title_005189b0(std::string_view text, FrontEndFrameHost& host) {
    FrontEndFrameState& frame = host.frame_004c1ac0();
    const FrontEndFrameSelection& selection = host.selection();
    const std::uintptr_t layout =
        frame.titles[static_cast<std::size_t>(frame.set) * kFrontEndFrameTitleSlots + selection.title_e18d8c];
    if (layout == 0) return;                                         // 005189CC..005189EC
    const std::uintptr_t label = host.find_child_00aa7e00(layout, kFrontEndFrameTitleText);  // 00518A2E
    const std::uintptr_t box = host.find_child_00aa7e00(layout, kFrontEndFrameTitleBox);     // 00518A98
    host.text_submit_00abaed0(label, text);                          // 00518AC6..00518ACB
    float width = kFrontEndTitleFixedWidth;                          // 00518B4D..00518B61 (61Fh or 634h set)
    if (!host.game_byte_61f() && !host.game_byte_634()) {            // 00518AD0..00518AE5
        // 00518AE7..00518B21: the extent over 960 is rounded to float; FCOMI against 0.2667.
        const float extent = static_cast<float>(host.text_extent_114(label) / kFrontEndTitlePixelsPerUnit);
        width = extent > kFrontEndTitlePad ? static_cast<float>(extent + kFrontEndTitlePad) : kFrontEndTitleMinWidth;
    }
    host.widget_set_size_58(box, width, host.widget_height_00aa6740(box));  // 00518B71..00518B79
}

namespace {

// 00515BE4..00515C07 (00515BC0) and the same block in 00515E00: online with both
// managers present, the screen acts only when 00687860(0Bh, 0) answers true.
bool tactical_library_may_navigate(TacticalLibraryNavHost& host) {
    if (host.online_1fe4() && host.front_end_manager_present_e198ac() && host.interface_manager_present_e198b4()) {
        return host.interface_query_00687860(0x0B, 0);
    }
    return true;
}

// 00515C1A..00515CF0 (00515BC0) and the same block in 00515E00.
void tactical_library_close_view(const TacticalLibraryNavState& screen, TacticalLibraryNavHost& host) {
    host.close_view_00511b40();
    if (screen.entry_kind_94 == 5) host.show_order_of_battle_page_004fff20();
    if (screen.entry_kind_94 == 6) host.show_unlocks_page_004ffd00();
    if (screen.entry_kind_94 == 2) {
        // Strings 00CE3A0C, 00CEB430, 00CEB394, 00CEB3FC, 00CEB3A8; pushed last-to-first.
        host.rebuild_command_bar_0054b530({{
            {0x00, "", 0},
            {0xA7, "globals.3dview", 1},
            {0xB3, "globals.navigate", 1},
            {0xB2, "globals.scroll_menu", 1},
            {0xA3, "globals.back", 2},
        }});
    }
}

}  // namespace

void tactical_library_leave_00515bc0(const TacticalLibraryNavState& screen, TacticalLibraryNavHost& host) {
    if (!tactical_library_may_navigate(host)) return;
    if (screen.view_90 == 1) tactical_library_close_view(screen, host);
    // 00515D9F..00515DE6.
    if (host.online_1fe4()) {
        host.pop_request_interface_006881f0();
        return;
    }
    host.set_return_screen_e08874((screen.field_9c != 0 ? 1 : 0) + 0x0B);
    host.push_interface_request_004cc460(1, 0);
}

void tactical_library_back_00515e00(const TacticalLibraryNavState& screen, TacticalLibraryNavHost& host) {
    if (!tactical_library_may_navigate(host)) return;
    if (screen.view_90 == 1) {
        tactical_library_close_view(screen, host);
        return;
    }
    tactical_library_leave_00515bc0(screen, host);                   // 00515FEF
}

}  // namespace bsp
