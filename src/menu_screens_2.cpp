// Front-end and in-game screens of partition segments 12, 15 and 19 (packet cc13-menus4).
// Evidence: docs/CC13_MENU_SCREENS_2.md.

#include "bsp/menu_screens_2.hpp"

#include <algorithm>

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
            session_browser_focus_list_0056aa70(host);
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
    host.join_session_0057a9c0(false, static_cast<std::size_t>(index), 0);  // 0057AEDB, RET 8
}

// 0057B530, EDI = screen, EBX = 0.
void session_browser_poll_session_list_0057b530(SessionBrowserState& screen, SessionBrowserHost& host) {
    host.session_call(0x94);                                         // 0057B535..0057B543, keeps the vector
    SessionSearchRequest& request = host.search_request_e08700();
    if (request.dirty_e08701) {                                      // 0057B547..0057B574
        request.dirty_e08701 = false;
        if (host.client_result_count_94() != 0) {
            host.join_session_0057a9c0(true, 0, 0);                  // 0057B576..0057B580: 00566370(0)
            return;
        }
    }
    if (request.flag_e08700) {                                       // 0057B589..0057B5D5
        screen.field_274 = -1;
        host.clear_search_list_0056a090();
        screen.join_cancel_requested = false;                        // +288h
        host.join_finished_0057ac10();                               // tail JMP
        return;
    }
    request.refresh_target_e08702 = false;                           // 0057B5DA..0057B5E2
    session_browser_set_page_0057a0f0(screen, 1, host);              // 0057B5E8..0057B5EC
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
    session_browser_layout_filter_rows_00568930(screen, host);                                        // 0057B607
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
            session_browser_back_from_results_00568cb0(host);
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
        if (!host.session_query(0x90)) session_browser_poll_session_list_0057b530(screen, host);
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
            const std::uint64_t xuid = session_browser_selected_xuid_00568d40(host);
            const std::uint32_t user = host.signed_in_user_00a3eac0();
            host.show_gamer_card_00a4d440(user, xuid);
            return;
        }
        if (!host.widget_byte_85(BW::SessionList30) && host.action_004d92b0(0x46)) {
            session_browser_select_toggle_00568df0(screen, 0, host);
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
            else session_browser_return_to_list_0056aad0(screen, host);
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

// 00568930, ESI = screen, EBX = row, EBP = last visible row.
void session_browser_layout_filter_rows_00568930(SessionBrowserState& screen, SessionBrowserHost& host) {
    float y = kSessionBrowserFilterRowTop;                           // 00568933 (00CEED68)
    std::int32_t last_visible = 0;
    for (std::int32_t row = 0; row < 9; ++row) {                     // 00568950..005689AB
        const float x = host.widget_f4_x_00aa67f0();
        host.filter_row_set_position_00aa7dc0(row, x, y, 0.0F);
        y = y + screen.row_spacing_25c;                              // FLD +25Ch, FADD, FSTP float
        if (host.filter_row_visible_38(row)) last_visible = row;     // CMOVNE EBP, EBX
    }
    // 005689AD..005689C4: FILD (last + 1), FMUL float, FADD double 00CEED60, rounded to float.
    const float height = static_cast<float>(
        static_cast<double>(last_visible + 1) * screen.row_spacing_25c + kSessionBrowserFilterRowPad);
    if (height == screen.field_240) return;                          // FUCOMIP; LAHF; TEST AH,44h; JNP
    screen.field_240 = height;                                       // 005689DC..00568A18
    host.scroller_attach_content_00683790(0x12C, 0);
    const float range = static_cast<float>(screen.field_240 - host.widget_height_00aa6740(BW::Viewport44));
    host.scroller_set_scroll_range_006834a0(0x12C, range);
}

namespace {

// 00568AFD..00568B52 and 00568C4F..00568CA2: the pressed/hovered widget is redrawn by its kind.
void browser_highlight(SessionBrowserEventHost& event, bool state) {
    if (event.event_kind_5c() == 6) {
        event.event_virtual_88(state ? 1 : 0, 0, 1.0F);
    } else if (event.event_kind_5c() == 0x12) {
        event.event_virtual_84(state ? 1 : 0);
    }
}

}  // namespace

// 00568A30, ESI = screen+0Ch, EDI = event. Offsets below are screen offsets (listener + 0Ch).
void session_browser_on_widget_released_00568a30(SessionBrowserState& screen, SessionBrowserEventHost& event) {
    if (event.event_is(0x40) || event.event_is(0x3C)) return;        // 00568A38..00568A44
    if (event.event_is(0xBC)) {
        event.scroller_end_thumb_drag_00683320(0x12C);
    } else if (event.event_is(0xB8)) {
        event.scroller_end_thumb_drag_00683320(0x188);
    } else if (event.event_is(0xA8) || event.event_is(0xAC)) {
        event.scroller_set_auto_scroll_00683300(0x12C, 0, 0.0F, 0);
    } else if (event.event_is(0xB0) || event.event_is(0xB4)) {
        event.scroller_set_auto_scroll_00683300(0x188, 0, 0.0F, 0);
    } else if (event.event_is(0x7C) || event.event_is(0x80)) {
        event.scroller_set_auto_scroll_00683300(0x1E4, 0, 0.0F, 0);
        screen.refresh_timer = kSessionBrowserRefreshSeconds;        // 00568AB4..00568ABC
    } else if (event.event_is(0x78)) {
        event.scroller_end_thumb_drag_00683320(0x1E4);
        screen.refresh_timer = kSessionBrowserRefreshSeconds;
    }
    browser_highlight(event, event.event_byte_d4());                 // SETNE on byte +D4h
}

// 00568B60, ESI = screen+0Ch, EDI = event.
void session_browser_on_widget_hover_00568b60(SessionBrowserState& screen, bool hovered, SessionBrowserHost& host,
    SessionBrowserEventHost& event) {
    if (event.event_kind_5c() == kGuiEventCommandButton) {           // 00568B71..00568BA4
        event.event_virtual_4c(hovered ? 0.5F : 1.0F);               // 00CE3800 / 00D7A24C
        return;
    }
    if (event.event_is(0x114)) {                                     // 00568BAB..00568BE0
        if (!host.widget_byte_85(BW::SessionList30)) return;
        host.widget_virtual(BW::Toggle3C, kToggle, 1);
        screen.toggle_50 = true;
        host.list_clear_00a9cdc0(BW::SessionList30);
        return;
    }
    if (event.event_is(0x40) || event.event_is(0x3C)) return;        // 00568BE3..00568BEF
    // 00568BF5..00568C4D: the compiler's chain. Only an event equal to +BCh, +B8h and +78h at
    // once with all three scrollers dragging skips the highlight.
    const bool listed = event.event_is(0xAC) || event.event_is(0xA8) || event.event_is(0xB0) ||
        event.event_is(0xB4) || event.event_is(0x7C) || event.event_is(0x80);
    if (!listed && event.event_is(0xBC) && screen.scroller_12c_174 && event.event_is(0xB8) &&
        screen.scroller_188_1d0 && event.event_is(0x78) && screen.scroller_1e4_22c) {
        return;
    }
    browser_highlight(event, hovered);
}

// 00568CB0.
void session_browser_back_from_results_00568cb0(SessionBrowserHost& host) {
    host.session_call(0x8C);                                         // 00568CB0..00568CBE
    host.session_set_mode_0076fad0(0);                               // 00568CC0..00568CCE
    host.pop_request_interface_006881f0();                           // 00568CD9 tail JMP
}

// 00568D40, EBX = screen, EDI = screen+2A0h.
std::uint64_t session_browser_selected_xuid_00568d40(SessionBrowserHost& host) {
    if (!host.online_1fe4()) return 0;                               // 00568D41..00568D51
    const std::size_t count = host.session_count_00566310();         // inlined
    if (count == 0) return 0;
    if (host.list_selected_index_00a9c920() < 0) return 0;           // 00568D7F..00568D89
    if (static_cast<std::uint32_t>(host.list_selected_index_00a9c920()) >= count) return 0;  // JAE
    const std::int32_t index = host.list_absolute_selection_005661c0();  // 00568DBA
    return host.session_record_005663b0(static_cast<std::size_t>(index)).xuid_98;  // +98h/+9Ch
}

// 00568DF0, ESI = screen; both arms end in a tail JMP to list virtual +60h(0).
void session_browser_select_toggle_00568df0(SessionBrowserState& screen, std::int32_t which, SessionBrowserHost& host) {
    if (which == 0) {
        if (screen.toggle_51 || host.widget_byte_85(BW::FilterPanel2C)) return;
        host.widget_virtual(BW::Toggle40, kToggle, 1);
        screen.toggle_50 = false;
        screen.toggle_51 = true;
        host.widget_virtual(BW::Toggle3C, kToggle, 0);
        host.widget_virtual(BW::SessionList30, kFocus, 0);
        return;
    }
    if (which != 1) return;
    if (screen.toggle_50 || host.widget_byte_85(BW::FilterPanel2C)) return;
    screen.toggle_51 = false;
    host.widget_virtual(BW::Toggle40, kToggle, 0);
    host.widget_virtual(BW::Toggle3C, kToggle, 1);
    screen.toggle_50 = true;
    host.widget_virtual(BW::SessionList30, kFocus, 0);
}

// 0056AA70. The list argument is compared with +30h after the input tick; all callers pass +30h.
void session_browser_focus_list_0056aa70(SessionBrowserHost& host) {
    host.input_owner_update_00a92c40(0.0F);                          // 0056AA70..0056AA80
    if (host.session_count_00566310() == 0) host.list_clear_00a9cdc0(BW::SessionList30);
    else host.widget_virtual(BW::SessionList30, kFocus, 1);          // tail JMP virtual +60h(1)
}

// 0056AAD0, ESI = screen.
void session_browser_return_to_list_0056aad0(SessionBrowserState& screen, SessionBrowserHost& host) {
    if (screen.toggle_50) {                                          // 0056AAD3..0056AAFE
        if (!host.list_has_rows_00a9ac50()) return;
        screen.toggle_50 = false;
        host.widget_virtual(BW::Toggle3C, kToggle, 0);
    } else if (screen.toggle_51) {                                   // 0056AB00..0056AB2A
        if (!host.list_has_rows_00a9ac50()) return;
        host.widget_virtual(BW::Toggle40, kToggle, 0);
        screen.toggle_51 = false;
    } else {
        return;
    }
    session_browser_focus_list_0056aa70(host);                       // 0056AB2D
    host.widget_virtual(BW::EmptyText4C, kShow, 0);
    host.widget_virtual(BW::SessionList30, kShow, 1);
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
            session_browser_back_from_results_00568cb0(host);
            return;
        }
        if (command == 0xA7) {                                       // refresh
            session_browser_set_page_0057a0f0(screen, 0, host);
            return;
        }
        if (command != 0xA5) return;                                 // 0057B084..0057B096
        if (!host.widget_byte_85(BW::SessionList30)) return;         // 0057B09C..0057B0A6
        if (!event.session_signed_in_198()) return;                  // 0057B0AC..0057B0BE
        const std::uint64_t xuid = session_browser_selected_xuid_00568d40(host);  // 0057B0CD
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
        session_browser_return_to_list_0056aad0(screen, host);
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

namespace {

// 00436630 BSP_NativeString_ConstructFromWideLowBytes: keeps the low byte of each UTF-16 unit.
std::string narrow_low_bytes(const std::wstring& text) {
    std::string out;
    out.reserve(text.size());
    for (const wchar_t c : text) out.push_back(static_cast<char>(c & 0xFF));
    return out;
}

// 00425850 BSP_NativeString_EqualsCStringInsensitive delegates to the CRT _stricmp (C locale).
bool equals_ascii_insensitive(const std::string& a, const std::string& b) noexcept {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const auto lower = [](char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c; };
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return true;
}

}  // namespace

// 00568490, __thiscall(record), RET. Every bit the record requires must be set in the game's
// two mask words.
bool session_record_content_allowed_00568490(const SessionJoinRecord& record, SessionJoinHost& host) {
    return (host.content_mask_728() & record.required_mask_88) == record.required_mask_88
        && (host.content_mask_72c() & record.required_mask_8c) == record.required_mask_8c;
}

// 00574100, __thiscall(record), RET; SEH frame 00C6F1B9 (the temporary narrow name). ESI = record.
bool session_record_joinable_00574100(const SessionJoinRecord& record, SessionJoinHost& host) {
    // 00574117..0057417C: a session named after the selected user's own name is refused.
    if (host.has_selected_user()
        && equals_ascii_insensitive(narrow_low_bytes(record.name_08), host.selected_username())) {
        return false;
    }
    // 005741C1..00574213: a free slot, the content masks, and the profile rank inside [+48h, +49h].
    if (!(record.players_28 < record.max_players_2c)) return false;  // JNC on the unsigned compare
    if (record.open_slots_24 <= 0) return false;                     // JLE
    if (!session_record_content_allowed_00568490(record, host)) return false;
    if (host.profile_rank_00574050() < record.rank_min_48) return false;
    return host.profile_rank_00574050() <= record.rank_max_49;      // the rank is read twice
}

SessionInfoBlock session_info_from_search_result(const SessionSearchResultView& result) {
    // 0057AAC9..0057AB23 after 00568CE0: object +04h/+08h -> +96Ch/+970h, +0Ch..+18h -> +998h..+9A4h,
    // +1Ch..+3Fh (REP MOVSD, 9 dwords) -> +974h..+997h.
    SessionInfoBlock block;
    block.session_id = result.session_id_04;
    block.host_address = result.host_address_1c;
    block.key_exchange_key = result.key_0c;
    return block;
}

// 0057A9C0, __thiscall(screen, record, arg), RET 8; SEH frame 00C6F6E1. EBP = record, EDI = 2.
void session_browser_join_session_0057a9c0(SessionBrowserState& screen, const SessionJoinRecord& record,
    SessionJoinHost& host) {
    host.copy_stored_lobby_options_005d49e0(record.lobby_options_4c);       // 0057A9E9, ECX = record+4Ch
    host.lua_set_global_string("sessionname", narrow_low_bytes(record.name_08));  // 0057A9F1..0057AA77
    host.session_cancel_8c();                                               // 0057AA7C..0057AA8A
    if (host.network_mode_1fe4() != kSessionJoinNetworkMode) {              // 0057AA92..0057AAA6
        host.session_set_mode_0076fad0(kSessionJoinNetworkMode);
    }
    if (record.kind_04 == kSessionRecordKindOnline) {                       // 0057AAB1..0057AAB6
        host.client_session_info_96c() = session_info_from_search_result(record.result);  // 0057AABC..0057AB23
        host.network_client_connect_00a39b80();                             // 0057AB29, XSessionCreate
        if (host.client_mode_9c() != kSessionClientModeJoining) {           // 0057AB2E..0057AB47
            host.set_client_mode_9c(kSessionClientModeJoining);
        }
        host.set_page_0057a0f0(2);                                          // 0057AB51..0057AB55
        screen.join_elapsed = 0.0F;                                         // +284h
        screen.field_24 = record.key_00;                                    // +24h = *record
        return;
    }
    host.select_listed_server_007701f0(record.kind_04);                     // 0057AB70..0057AB7A, MOVSX
    const std::int32_t mode = host.client_mode_9c();
    if (mode == kSessionClientModeJoined || mode == kSessionClientModeJoining || host.search_flag_e08700()) {
        host.set_page_0057a0f0(2);                                          // 0057ABE8..0057ABED (EDI = 2)
        return;
    }
    host.session_set_mode_0076fad0(0);                                      // 0057ABA4..0057ABDC
    host.pop_request_interface_006881f0();
    host.push_request_interface_logged_00688ad0(kRequestJoinFailed);
    host.error_screen_show_005d2e70(kErrorScreenJoinFailed);
    screen.page = 0;                                                        // +28Ch
}

// 0057AC10, __thiscall(screen), RET; EBX = screen, EDI = the result index, [ESP+10h] its byte
// offset. The tried set at +278h is std::set<std::uint32_t> (find 0056B850, insert 00571FD0,
// subtree erase 0056A090); the clear at 0057ADF3..0057AE43 is the inlined _Tidy loop.
void session_browser_finish_join_0057ac10(SessionBrowserState& screen, SessionJoinState& join,
    SessionJoinHost& host) {
    if (screen.join_cancel_requested) {                                     // 0057AC1F..0057AC4E
        screen.join_cancel_requested = false;
        host.session_cancel_8c();
        host.session_set_mode_0076fad0(0);
        host.pop_request_interface_006881f0();
    }
    // 0057AC61..0057AD37: the client's +94h vector is fetched afresh for every index.
    for (std::size_t i = 0; i < host.client_results_94().size(); ++i) {
        const SessionJoinRecord& record = host.client_results_94()[i];
        auto& tried = join.tried_keys_278;
        const auto at = std::lower_bound(tried.begin(), tried.end(), record.key_00);
        if (at != tried.end() && *at == record.key_00) continue;            // 0057AD12..0057AD19, find != end
        if (!session_record_joinable_00574100(record, host)) continue;      // 0057AD1B..0057AD24
        host.set_join_retry_timer_e08760(kSessionJoinRetryTimerReset);      // 0057AD3E..0057AD56
        tried.insert(at, record.key_00);                                    // 0057AD62, 00571FD0
        session_browser_join_session_0057a9c0(screen, record, host);        // 0057AD67..0057AD6C, arg 0
        return;
    }
    host.set_join_retry_timer_e08760(kSessionJoinRetryTimerReset);          // 0057AD79..0057AD87
    if (host.request_count_006878d0() > 0) host.pop_request_interface_006881f0();  // 0057AD8F..0057AD9E
    host.set_global_e19610(1);                                              // 0057ADAB
    host.push_request_interface_logged_00688ad0(kRequestNoSessionFound);    // 0057ADA9..0057ADB5
    host.set_game_byte_2059(host.client_byte_4d());                         // 0057ADBA..0057ADD1
    screen.page = 0;                                                        // +28Ch
    screen.poll_timer = kSessionBrowserPollSeconds;                         // +270h = 00CEECEC (2.5)
    screen.field_274 = -1;
    join.tried_keys_278.clear();                                            // 0057ADF3..0057AE43
}

// 00576D20, __thiscall(screen), SEH frame 00C6F43B; ESI = screen, EBP = screen+2A0h until
// 00576F5C, then the mode_Text widget. The Connection_Group lookup's result is discarded on
// both paths.
void session_browser_update_selected_row_00576d20(SessionRowState& screen, SessionRowHost& host) {
    const bool listing = host.network_mode_1fe4() != 0 && !screen.records_2a0.empty();  // 00576D3E..00576D84
    if (listing) {
        if (screen.selected_row_26c == 0) return;                           // 00576D8A -> 0057763D
        if (host.client_results_94().size() == 1) {                         // 00576D97..00576DCB
            const SessionJoinRecord result = host.client_results_94()[0];   // 00576DD1..00576DE5, 00566370(0)
            // 00576DF5..00576E65: the first record whose search result has the same session id
            // (result +04h/+08h) is replaced by a fresh 0A0h copy (00571E50 + 00571EA0, insert
            // 00574BA0 before it, erase 00573220 after it). No match leaves the vector alone.
            for (auto& record : screen.records_2a0) {
                if (record.result.session_id_04 == result.result.session_id_04) {
                    record = result;
                    break;
                }
            }
            const std::uintptr_t row = screen.selected_row_26c;
            const std::uintptr_t server = host.find_child_00aa7e00(row, "server_Text");    // 00576EE1
            const std::uintptr_t mode = host.find_child_00aa7e00(row, "mode_Text");        // 00576F33
            const std::uintptr_t players = host.find_child_00aa7e00(row, "players_Text");  // 00576F81
            host.find_child_00aa7e00(row, "Connection_Group");                             // 00576FD0
            host.text_set_00abaed0(server, narrow_low_bytes(result.name_08));              // 0057701B..0057704A
            host.text_set_ellipsized_00abb000(mode,                                        // 00577072..005770C2
                host.lobby_option_text(kLobbyOptionGameMode, result.lobby_options_4c[2]), -1.0F);
            host.text_set_00abaed0(players,                                                // 005770EA..00577145
                std::to_string(result.players_28) + "/" + std::to_string(result.max_players_2c));
            // 005771D8..00577204: the inlined 00568490 test on the result.
            const bool allowed = (host.content_mask_728() & result.required_mask_88) == result.required_mask_88
                && (host.content_mask_72c() & result.required_mask_8c) == result.required_mask_8c;
            const GuiColour& colour = allowed ? kSessionRowJoinable : kSessionRowLocked;  // 0057720A / 005772F4
            host.widget_set_colour_50(server, colour);
            host.widget_set_colour_50(mode, colour);
            host.widget_set_colour_50(players, colour);
            return;
        }
    }
    // 005773D6..0057763D: no row, or exactly one client result, returns; otherwise the row greys.
    if (screen.selected_row_26c == 0) return;
    if (host.client_results_94().size() == 1) return;
    const std::uintptr_t row = screen.selected_row_26c;
    const std::uintptr_t server = host.find_child_00aa7e00(row, "server_Text");
    const std::uintptr_t mode = host.find_child_00aa7e00(row, "mode_Text");
    const std::uintptr_t players = host.find_child_00aa7e00(row, "players_Text");
    host.find_child_00aa7e00(row, "Connection_Group");
    host.widget_set_colour_50(server, kSessionRowIdle);
    host.widget_set_colour_50(mode, kSessionRowIdle);
    host.widget_set_colour_50(players, kSessionRowIdle);
}

// 0056BDA0, __thiscall(screen), RET; ESI = screen, EDI = screen+0Ch for the listener calls
// (17 calls of 00AA6BC0, each (screen+0Ch, 0)). Every lookup passes the recursive flag 1.
void session_browser_bind_layout_0056bda0(SessionBrowserLayout& layout, SessionBrowserLayoutHost& host) {
    auto& w = layout.widgets;
    w.clear();
    const auto bind = [&](std::uint16_t parent, std::string_view name, std::uint16_t offset) {
        const std::uintptr_t widget = host.find_child_00aa7e00(layout.at(parent), name);
        w.emplace_back(offset, widget);
        return widget;
    };
    const auto bind_listened = [&](std::uint16_t parent, std::string_view name, std::uint16_t offset) {
        host.set_listener_00aa6bc0(bind(parent, name, offset));
    };
    w.emplace_back(0x2C, host.load_page_00aa5840("FE_serverbrowser_filters"));  // 0056BDC2..0056BE11
    bind(0x2C, "cucc_Group", kBindLocalCucc);                                  // 0056BE3A..0056BE84
    host.widget_show_34(layout.at(0x2C), false);                              // 0056BEA9..0056BEB2
    bind(kBindLocalCucc, "menu_line_Group", 0xEC);                             // 0056BED9
    bind(kBindLocalCucc, "Settings_Listbox", 0x34);                            // 0056BF4B
    w.emplace_back(0x28, host.load_page_00aa5840("FE_serverbrowser"));          // 0056BFBA..0056BFDF
    host.widget_show_34(layout.at(0x28), false);                              // 0056C013
    bind(0x28, "servers_fix_Group", 0x128);                                    // 0056C03A
    bind(0x128, "servers_FrameBox", 0x38);
    bind(0x128, "noserver_Text", 0x4C);
    bind(0x128, "realation_server_Icon", 0x2B4);                               // sort headers, keys 0, 1, 5, 4
    bind(0x128, "realation_mode_Icon", 0x2B8);
    bind(0x128, "realation_player_Icon", 0x2BC);
    bind(0x128, "realation_connect_Icon", 0x2C0);
    for (const std::uint16_t icon : std::array<std::uint16_t, 4>{0x2B4, 0x2B8, 0x2BC, 0x2C0}) {             // 0056C346..0056C378
        host.set_listener_00aa6bc0(layout.at(icon));
    }
    bind(0x28, "servers_Clipbox", 0x84);                                       // 0056C3A2
    const std::uintptr_t list = bind(0x84, "servers_Listbox", 0x30);           // 0056C416
    host.list_configure_00a9ceb0(list, kSessionListRowSpan, 0);                // 0056C463..0056C471
    host.list_set_pads_128(list, kSessionListPad128, 0.0F, kSessionListPad130, 0.0F);  // 0056C476..0056C4A7
    host.list_set_byte_149(list, false);                                       // 0056C4BA
    host.widget_show_34(bind(0x30, "listbox1_Group", 0xE8), false);            // 0056C4DD..0056C537, row template
    bind(0x28, "slider_pos_Group", 0x88);                                      // 0056C55E
    bind(0x88, "slider_Group", 0x118);
    bind_listened(0x118, "slider_FrameBox", 0x78);                             // 0056C647..0056C69A
    layout.slider_back_height_90 =                                             // 0056C6C4..0056C6FE
        host.group_height_00aa6740(host.find_child_00aa7e00(layout.at(0x118), "back_Icon"));
    // 0056C748..0056C8EF: ScrollUp_Icon / ScrollDown_Icon are searched inside a fresh
    // slider_pos_Group lookup each time.
    bind(0x28, "slider_pos_Group", kBindLocalSliderPos);
    bind_listened(kBindLocalSliderPos, "ScrollUp_Icon", 0x7C);
    bind(0x28, "slider_pos_Group", kBindLocalSliderPos);
    w.emplace_back(0x80, host.find_child_00aa7e00(w.back().second, "ScrollDown_Icon"));
    host.set_listener_00aa6bc0(layout.at(0x80));
    host.scroller_attach_content_00683790(0x1E4, list);                        // 0056C8F4..0056C900
    layout.clip_height_8c = host.group_height_00aa6740(layout.at(0x84));      // 0056C905..0056C919
    // 0056C91F..0056C94F: extent = +90h - height(+78h) (x87, stored to float before the call).
    host.scroller_attach_scroll_bar_00683380(0x1E4, layout.at(0x78),
        static_cast<float>(static_cast<double>(layout.slider_back_height_90) - host.group_height_00aa6740(layout.at(0x78))),
        layout.at(0x7C), layout.at(0x80));
    bind(0x28, "settings_Group", 0x120);                                       // 0056C979
    bind_listened(0x120, "settings_FrameBox", 0x40);
    bind(0x120, "setting_Clipbox", 0x48);
    bind(0x48, "setting_template_Group", 0x124);
    bind(0x124, "rank_Icon", 0xA4);
    bind(0x124, "setting_Text", 0x94);
    bind(0x124, "setting_var_Text", 0x98);
    bind(0x120, "game_mode_var_Text", 0xD0);
    bind(0x120, "map_Icon", 0xA0);
    bind(0x120, "tilt_Icon", 0x9C);
    for (const std::uint16_t hidden : std::array<std::uint16_t, 3>{0xA4, 0x94, 0x98}) host.widget_show_34(layout.at(hidden), false);  // 0056CDFD..0056CE19
    bind_listened(0x120, "slider_FrameBox", 0xB8);                             // 0056CE63
    bind_listened(0x120, "scroll_down2_Icon", 0xB0);
    bind_listened(0x120, "scroll_up2_Icon", 0xB4);
    host.scroller_attach_content_00683790(0x188, layout.at(0x124));            // 0056CFA7..0056CFB6
    {  // 0056CFBD..0056D053: back height first, then the frame's.
        const float back = host.group_height_00aa6740(host.find_child_00aa7e00(layout.at(0x120), "scroll_back2_Icon"));
        const float frame = host.group_height_00aa6740(layout.at(0xB8));
        host.scroller_attach_scroll_bar_00683380(0x188, layout.at(0xB8), back - frame, layout.at(0xB4), layout.at(0xB0));
    }
    host.set_listener_00aa6bc0(layout.at(0x120));                              // 0056D07B..0056D086
    bind(0x28, "filters_Group", 0x114);                                        // 0056D0D0
    bind(0x114, "setting_Clipbox", 0x44);
    bind(0x44, "textbox_Group", 0xFC);
    bind(0x44, "moving_Group", 0x100);
    host.widget_show_34(bind(0xFC, "setting_template_Group", 0xF4), false);
    host.widget_show_34(bind(0xFC, "setting_template_next_line_Group", 0xF8), false);
    bind_listened(0x114, "scroll_slider2_FrameBox", 0xBC);                     // 0056D3A7
    bind_listened(0x114, "scroll_down2_Icon", 0xA8);
    bind_listened(0x114, "scroll_up2_Icon", 0xAC);
    host.scroller_attach_content_00683790(0x12C, layout.at(0x100));            // 0056D4EB..0056D4FA
    {  // 0056D547..0056D597
        const float back = host.group_height_00aa6740(host.find_child_00aa7e00(layout.at(0x114), "scroll_back2_Icon"));
        const float frame = host.group_height_00aa6740(layout.at(0xBC));
        host.scroller_attach_scroll_bar_00683380(0x12C, layout.at(0xBC), back - frame, layout.at(0xAC), layout.at(0xA8));
    }
    host.set_listener_00aa6bc0(layout.at(0x114));                              // 0056D5BF..0056D5CA
    bind_listened(0x114, "New_FrameBox", 0x3C);                                // 0056D617..0056D647
}

namespace {

// _wcsicmp in the C locale: ASCII letters fold, every other unit compares by value.
int wcsicmp_c_locale(const std::wstring& a, const std::wstring& b) noexcept {
    const auto lower = [](wchar_t c) { return (c >= L'A' && c <= L'Z') ? static_cast<wchar_t>(c - L'A' + L'a') : c; };
    const std::size_t n = std::min(a.size(), b.size());
    for (std::size_t i = 0; i < n; ++i) {
        const wchar_t x = lower(a[i]);
        const wchar_t y = lower(b[i]);
        if (x != y) return x < y ? -1 : 1;
    }
    return a.size() == b.size() ? 0 : (a.size() < b.size() ? -1 : 1);
}

}  // namespace

// 0056A870, __thiscall(ECX = the key functor, a, b), RET 8; a jump table on *ECX.
bool session_record_less_0056a870(SessionSortKey key, const SessionJoinRecord& a, const SessionJoinRecord& b) {
    switch (key) {
    case SessionSortKey::Name: return wcsicmp_c_locale(a.name_08, b.name_08) < 0;
    case SessionSortKey::GameMode: return a.lobby_options_4c[2] < b.lobby_options_4c[2];  // +54h, unsigned
    case SessionSortKey::MaxPlayers: return b.max_players_2c < a.max_players_2c;
    case SessionSortKey::FreeSlots: return b.max_players_2c - b.players_28 < a.max_players_2c - a.players_28;
    case SessionSortKey::Field34: return b.field_34 < a.field_34;
    case SessionSortKey::Fullness:
        return b.max_players_2c * 16U + b.players_28 < a.max_players_2c * 16U + a.players_28;
    default: return false;
    }
}

// 00576BE0, __thiscall(screen, record), RET 4. ESI = record.
bool session_record_passes_filters_00576be0(const SessionBrowserState& filters, const SessionRowState& screen,
    const SessionJoinRecord& record, SessionRowHost& host) {
    const auto& applied = filters.filter_bytes;  // +10h..+18h
    const std::int32_t mode = static_cast<std::int32_t>(record.lobby_options_4c[2]);  // +54h
    const std::int32_t wanted = host.game_type_filter_e086c0();
    bool type_matches = false;
    switch (mode) {
    case 5: if (!applied[2]) return false; type_matches = wanted == 5; break;
    case 6: if (!applied[4]) return false; type_matches = wanted == 6; break;
    case 7: if (!applied[3]) return false; type_matches = wanted == 7; break;
    case 4: if (!applied[1]) return false; type_matches = wanted == 4; break;
    default:
        if (mode > 3) return false;                       // signed: negative modes join 0..3
        if (!applied[0]) return false;
        type_matches = wanted < 4;                        // signed, so -1 passes here too
        break;
    }
    if (!type_matches && wanted != -1) return false;
    // 00576C7A..: the three count filters (unsigned compares).
    if (applied[5] && record.max_players_2c < screen.applied_counts_244[0]) return false;
    if (applied[6] && record.max_players_2c - record.players_28 < screen.applied_counts_244[1]) return false;
    if (applied[7] && screen.applied_counts_244[2] < record.field_30) return false;
    // 00576CB0..00576D17: x87 ratio +94h / +90h (each unsigned, the 2^32 fix-up at 00CE3978),
    // compared with registry+BCh; JA rejects. Double stands in for the 80-bit intermediate.
    if (applied[8] && host.ratio_minimum_b8() < record.ratio_den_90) {
        const double ratio = static_cast<double>(record.ratio_num_94) / static_cast<double>(record.ratio_den_90);
        if (ratio > static_cast<double>(host.ratio_maximum_bc())) return false;
    }
    return true;
}

// 00578320, __fastcall(ECX = group, EDX = connection value), RET; SEH frame 00C6F4A8. EDI = group,
// EBP walks the name table 00E08738..00E0874C (ping_1_Icon..ping_5_Icon, 00CEF1EC..00CEF21C).
void session_row_set_connection_icons_00578320(std::uintptr_t group, std::int32_t connection,
    SessionRowHost& host) {
    static constexpr std::array<std::string_view, 5> kIcons{
        "ping_1_Icon", "ping_2_Icon", "ping_3_Icon", "ping_4_Icon", "ping_5_Icon"};
    std::int32_t shown = 0;                                          // [ESP+14h]
    for (std::size_t i = 0; i < kIcons.size(); ++i) {               // 00578364..00578498
        const bool show = !(host.ping_threshold_90(i) > connection);  // CMP [thr], value; JG hides
        host.widget_show_34(host.find_child_00aa7e00(group, kIcons[i]), show);
        if (show) ++shown;
    }
    // 0057849E..005784B5: fewer than two bars -> state 2, fewer than four -> 1, else 0.
    const std::int32_t state = shown < 2 ? 2 : (shown < 4 ? 1 : 0);
    for (const std::string_view icon : kIcons) {                     // 005784BA..0057855D
        host.widget_set_state_88(host.find_child_00aa7e00(group, icon), state, 0, 1.0F);
    }
}

// 005789C0, __thiscall(screen, sort key), RET 4; EDI/[ESP+10h] = screen. The name the row +26Ch
// showed before the rebuild picks the new +26Ch; the list's selected-row name is also read and
// compared (00579817..0057982D) but the result is discarded.
void session_browser_build_rows_005789c0(const SessionBrowserState& filters, SessionRowState& screen,
    SessionSortReverseFlags& reverse, std::int32_t sort_key, SessionRowHost& host) {
    if (host.widget_byte_85(screen.filter_panel_2c)) return;            // 005789DE..005789ED
    if (sort_key != static_cast<std::int32_t>(SessionSortKey::KeepCurrent)) {  // 005789FB..00578A1E
        if (screen.sort_key_2b0 == sort_key) {
            reverse[static_cast<std::size_t>(sort_key)] = !reverse[static_cast<std::size_t>(sort_key)];
        } else {
            screen.sort_key_2b0 = sort_key;
        }
    }
    for (const std::uintptr_t icon : screen.sort_icons_2b4) host.widget_set_state_88(icon, 0, 0, 1.0F);  // 00578A24..00578A8A
    // 00578A8C..00578B33: the active key's header shows 3 when reversed, else 2; keys 2 and 3
    // have no header.
    static constexpr std::array<std::pair<std::int32_t, std::size_t>, 4> kHeaders{{{0, 0}, {1, 1}, {5, 2}, {4, 3}}};
    for (const auto& [key, icon] : kHeaders) {
        if (screen.sort_key_2b0 != key) continue;
        host.widget_set_state_88(screen.sort_icons_2b4[icon], reverse[static_cast<std::size_t>(key)] ? 3 : 2, 0, 1.0F);
        break;
    }
    std::string previous_row_name;                                       // [ESP+24h]
    if (screen.selected_row_26c != 0) {                                 // 00578B64..00578BFD
        previous_row_name = host.widget_text_f4(host.find_child_00aa7e00(screen.selected_row_26c, "server_Text"));
    }
    std::string list_selection_name;                                     // [ESP+2Ch]
    if (const std::uintptr_t row = host.list_selected_row_00425e50(screen.session_list_30); row != 0) {
        list_selection_name = host.widget_text_f4(host.find_child_00aa7e00(row, "server_Text"));
    }
    host.list_set_listener_00a9ac40(screen.session_list_30, false);     // 00578C93..00578CA0
    host.list_clear_rows_00a9bec0(screen.session_list_30);
    if (sort_key == static_cast<std::int32_t>(SessionSortKey::KeepCurrent)) {  // 00578CA5..00578DBC
        screen.results_290 = host.client_results_94();                   // clear 00573C90, push 00575150, copy 00571EA0
    }
    screen.records_2a0.clear();                                          // 00578DC1..00578DFA, 00573490
    for (const SessionJoinRecord& record : screen.results_290) {        // 00578E20..00578E81
        if (session_record_passes_filters_00576be0(filters, screen, record, host)) screen.records_2a0.push_back(record);
    }
    // 00578E83..00578ECB: std::sort (introsort 00574A50, insertion 005738C0, threshold 32) with
    // the key functor; ties keep no particular order, so std::sort stands in for it.
    const auto key = static_cast<SessionSortKey>(screen.sort_key_2b0);
    std::sort(screen.records_2a0.begin(), screen.records_2a0.end(),
        [key](const SessionJoinRecord& a, const SessionJoinRecord& b) { return session_record_less_0056a870(key, a, b); });
    if (reverse[static_cast<std::size_t>(screen.sort_key_2b0)]) {       // 00578ED0..00578FA5, swaps 005725A0
        std::reverse(screen.records_2a0.begin(), screen.records_2a0.end());
    }
    screen.selected_row_26c = 0;                                         // 00578FB0
    for (const SessionJoinRecord& record : screen.records_2a0) {        // 00578FBC..00579865
        const std::uintptr_t row = host.clone_subtree_00aab4c0(screen.row_template_e8, 0);  // 0057900F..00579024
        host.widget_set_floats_30(row, -0.008333334F, 0.0F);            // +30h = 00CEF228, +34h = 0
        const std::uintptr_t server = host.find_child_00aa7e00(row, "server_Text");
        const std::uintptr_t mode = host.find_child_00aa7e00(row, "mode_Text");
        const std::uintptr_t players = host.find_child_00aa7e00(row, "players_Text");
        const std::uintptr_t connection = host.find_child_00aa7e00(row, "Connection_Group");
        const std::string name = narrow_low_bytes(record.name_08);
        host.text_set_00abaed0(server, name);                            // 0057921A..00579278
        host.text_set_ellipsized_00abb000(mode,                          // 005792A4..0057933E
            host.lobby_option_text(kLobbyOptionGameMode, record.lobby_options_4c[2]), -1.0F);
        host.text_set_00abaed0(players,                                  // 00579370..00579400
            std::to_string(record.players_28) + "/" + std::to_string(record.max_players_2c));
        session_row_set_connection_icons_00578320(connection, record.connection_38, host);  // 005794A8..005794B2
        const bool allowed = (host.content_mask_728() & record.required_mask_88) == record.required_mask_88
            && (host.content_mask_72c() & record.required_mask_8c) == record.required_mask_8c;
        const GuiColour& colour = allowed ? kSessionRowJoinable : kSessionRowLocked;  // 005794B7..005796D2
        host.widget_set_colour_50(server, colour);
        host.widget_set_colour_50(mode, colour);
        host.widget_set_colour_50(players, colour);
        host.list_append_row_00a9d750(screen.session_list_30, row);      // 005796D4..005796E0
        host.widget_set_byte_77(row, false);                             // 005796E5
        // 005796E9..005797B1: equal lengths and _stricmp == 0 (two empty names match).
        if (previous_row_name.size() == name.size() && equals_ascii_insensitive(previous_row_name, name)) {
            screen.selected_row_26c = row;
        }
        (void)list_selection_name;                                       // 005797B7..00579835, result unused
    }
    host.widget_virtual_80(screen.session_list_30, 1);                   // 0057986A..00579877
    host.list_set_listener_00a9ac40(screen.session_list_30, true);       // 00579879..00579880
    host.widget_focus_60(screen.session_list_30, host.widget_byte_85(screen.session_list_30));  // 00579885..0057989C
    host.refresh_row_details_00577650();                                 // 0057989E..005798A0
    const std::int32_t rows = host.list_row_count_104(screen.session_list_30);
    const float row_height = rows != 0                                   // 005798A5..005798C4
        ? host.group_height_00aa6740(host.list_row_at_00a9be00(screen.session_list_30, 0)) : 0.0F;
    host.scroller_attach_content_00683790(0);                            // 005798C7..005798D6
    // 005798DB..00579904: FILD count, FMUL height, FSUB +8Ch, rounded to float through a store.
    host.scroller_set_scroll_range_006834a0(static_cast<float>(static_cast<double>(rows) * row_height
        - screen.viewport_height_8c));
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
    const float count = static_cast<float>(screen.current_index);
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
void tactical_library_leave_3d_view(const TacticalLibraryNavState& screen, TacticalLibraryNavHost& host) {
    host.toggle_view_00511b40();
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

void tactical_library_toggle_view_00511b40(TacticalLibraryNavState& screen, TacticalLibraryViewHost& host) {
    constexpr std::array<std::uint16_t, 4> kPanels{0x508, 0x50C, 0x510, 0x518};
    if (screen.view_90 == 0) {                                       // 00511B43..00511B91
        screen.view_90 = 1;
        for (const std::uint16_t offset : kPanels) host.panel_widget_virtual_34(offset, 0);
        return;
    }
    if (screen.view_90 != 1) return;                                 // 00511B95..00511B98
    screen.view_90 = 0;                                              // 00511B9A..00511BDE
    for (const std::uint16_t offset : kPanels) host.panel_widget_virtual_34(offset, 1);
    host.refresh_entry_00510c20();                                   // 00511BE3 tail JMP
}

void tactical_library_switch_view_00514c20(TacticalLibraryNavState& screen, TacticalLibraryViewHost& host) {
    tactical_library_toggle_view_00511b40(screen, host);             // 00514C41
    const bool model_view = screen.view_90 == 1;                     // 00514C4C / 00514E3B
    if (!model_view && screen.view_90 != 0) return;
    if (screen.entry_kind_94 == 5 || screen.entry_kind_94 == 6) {   // 00514C5B..00514CB2, 00514E49..00514E92
        host.set_field_4b0(host.selector_434_field_48_004fa890());
        if (screen.entry_kind_94 == 5) host.show_order_of_battle_page_004fff20();
        else host.show_unlocks_page_004ffd00();
        if (model_view) host.load_vehicle_model_0050aa10();
        return;
    }
    if (screen.entry_kind_94 == 2) {                                 // 00514CC7..00514D68, 00514EA7..00514F48
        host.rebuild_command_bar_0054b530({{
            {0x00, "", 0},
            {0xA7, model_view ? "globals.2dview" : "globals.3dview", 1},
            {0xB3, "globals.navigate", 1},
            {0xB2, "globals.scroll_menu", 1},
            {0xA3, "globals.back", 2},
        }});
    }
    if (model_view) host.load_vehicle_model_0050aa10();              // 00514E24..00514E26
}

void tactical_library_leave_00515bc0(const TacticalLibraryNavState& screen, TacticalLibraryNavHost& host) {
    if (!tactical_library_may_navigate(host)) return;
    if (screen.view_90 == 1) tactical_library_leave_3d_view(screen, host);
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
        tactical_library_leave_3d_view(screen, host);
        return;
    }
    tactical_library_leave_00515bc0(screen, host);                   // 00515FEF
}

}  // namespace bsp
