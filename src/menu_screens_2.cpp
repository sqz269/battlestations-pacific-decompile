// Front-end and in-game screens of partition segments 12, 15 and 19 (packet cc13-menus4).
// Evidence: docs/CC13_MENU_SCREENS_2.md.

#include "bsp/menu_screens_2.hpp"
#include "bsp/unit_motion.hpp"
#include "bsp/unit_rudder.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

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
// Documented deviation (cc13-D3): 00574A50 / 005738C0 are the VC8 std::sort
// instantiation (library code, bound not built: library_instantiation_out_of_scope);
// the toolchain's std::sort may order equal-key rows differently. Only the display
// order of sessions with equal sort keys can differ.
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

namespace {

// 004C5E60 widens byte by byte (provisional: the byte-to-unit rule was not traced), then
// 0052D1E0 = vswprintf(buffer, text, 0x2122). Only the directives this installation's
// Credits.lua uses are modelled: %c takes the argument, %% emits '%'; any other directive is
// copied unchanged.
std::wstring credits_format_percent(const std::string& text) {
    std::wstring out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const wchar_t c = static_cast<wchar_t>(static_cast<unsigned char>(text[i]));
        if (c == L'%' && i + 1 < text.size() && (text[i + 1] == 'c' || text[i + 1] == 'C')) {
            out.push_back(kCreditsPercentArgument);
            ++i;
        } else if (c == L'%' && i + 1 < text.size() && text[i + 1] == '%') {
            out.push_back(L'%');
            ++i;
        } else {
            out.push_back(c);
        }
    }
    return out;
}

}  // namespace

// 0052EE30, __thiscall(screen), RET; EDI = screen, EBP = the item index, ESI = screen+4Ch,
// [ESP+1Ch] the item byte offset. Locals: x [ESP+28h], y [ESP+30h], width [ESP+38h], next
// indent [ESP+40h], row y [ESP+44h], height [ESP+14h], text-clone flag [ESP+13h], image [ESP+20h].
void credits_enter_0052ee30(CreditsLayoutState& screen, CreditsEnterHost& host) {
    if (!host.stream54_exists()) host.stream54_construct_00a877d0();    // 0052EE51..0052EECE
    host.stream50_request_stop_00a86f40();                               // 0052EED6..0052EEDF
    host.stream54_start_00a867b0(host.stream_name_e18e64());             // 0052EEE4..0052EF41
    host.stream54_request_fade_in_00a85c20();                            // 0052EF6C..0052EF75
    host.gui_pause_movie_pages_00aa33a0(1);                              // 0052EF7A..0052EF83
    host.set_frame_title_005189b0("FE.main_credits");                    // 0052EF88..0052EFCA
    host.rebuild_command_bar_0054b530(back_only_command_bar());          // 0052EFF5..0052F14B
    host.widget_set_local_y_00aa78f0(screen.moving_group_1c, 0.0F);      // 0052F211..0052F21A
    screen.scroll_y_34 = 0.0F;                                           // 0052F229
    screen.indent_48 = 0.0F;                                             // 0052F22E
    float y = screen.start_y_40;
    float width = 0.0F;
    float next_indent = 0.0F;
    float row_y = 0.0F;
    float x = 0.0F;
    for (const CreditsItem& item : screen.items_4c) {                    // 0052F252..0052F8D6
        bool text_clone = false;
        std::uintptr_t image = 0;
        std::uintptr_t clone = 0;
        float height = 0.0F;
        bool placed = false;
        switch (item.type) {                                             // jump table 0052F910
        case CreditsItemType::Category:                                  // 0052F2CB
            clone = host.clone_subtree_00aab4c0(screen.category_template_10, screen.moving_group_1c);
            host.text_set_00abaed0(clone, item.text);
            text_clone = true;
            placed = clone != 0;
            break;
        case CreditsItemType::Person:                                    // 0052F326..0052F47B
            clone = host.clone_subtree_00aab4c0(screen.person_template_14, screen.moving_group_1c);
            if (!item.text.empty() && item.text.find('%') != std::string::npos) {
                host.text_set_utf16_00ab6ab0(clone, credits_format_percent(item.text));
            } else {
                host.text_set_00abaed0(clone, item.text);
            }
            text_clone = true;
            placed = clone != 0;
            break;
        case CreditsItemType::Image:                                     // 0052F480..0052F54D
            // 0052F4D8..0052F4EE: the clone's parent argument (+1Ch) is pushed before the lookup.
            image = host.clone_subtree_00aab4c0(
                host.find_child_00aa7e00(screen.moving_group_1c, item.text + "_Icon"), screen.moving_group_1c);
            placed = image != 0;
            break;
        case CreditsItemType::BlankLine: height = kCreditsBlankLineHeight; break;  // 0052F5D2
        case CreditsItemType::VerticalGap:                                // 0052F5E5..0052F630
            height = -0.0F - screen.row_space_44;
            y = y + item.offset;
            break;
        case CreditsItemType::SetIndent: screen.indent_48 = item.offset; break;    // 0052F635..0052F668
        case CreditsItemType::SameLine:                                   // 0052F66D..0052F684
            screen.indent_48 = next_indent;
            y = row_y;
            break;
        default: break;                                                   // JA 0052F89B
        }
        if (placed) {
            x = kCreditsStaleX;                                          // 0052F558..0052F560
            const std::uintptr_t widget = text_clone ? clone : image;
            host.widget_show_34(widget, true);                           // 0052F566..0052F575 / 0052F689..0052F692
            host.widget_set_colour_50(widget, item.colour);
            if (text_clone) {                                            // 0052F5B2..0052F5C9
                width = static_cast<float>(host.text_pixel_width_114(widget) / kCreditsTextPixels);
                height = host.text_normalized_height_00ab6bd0(widget);
            } else {                                                     // 0052F6D5..0052F6F6
                const auto size = host.group_size_00aa6740(widget);
                width = size[0];
                height = size[1];
            }
            // 0052F725..0052F7B6: x87, each result rounded to float at 0052F7B6.
            switch (item.align) {
            case 0: x = static_cast<float>(static_cast<double>(host.group_origin_x_00aa6750(screen.frame_18)) + screen.indent_48); break;
            case 1: {
                const double left = host.group_origin_x_00aa6750(screen.frame_18);
                x = static_cast<float>(host.group_size_00aa6740(screen.frame_18)[0] + left - width - screen.indent_48);
                break;
            }
            case 2: {
                const double left = host.group_origin_x_00aa6750(screen.frame_18);
                x = static_cast<float>(host.group_size_00aa6740(screen.frame_18)[0] * 0.5 + left - width * 0.5);
                break;
            }
            default: break;
            }
            // 0052F7C4..0052F885: position (x + Offset, y, -100).
            host.set_resolved_position_00aa8240(widget, {static_cast<float>(item.offset + static_cast<double>(x)), y,
                kCreditsItemDepth});
            screen.widgets_24.push_back(widget);                         // 0052F88F..0052F896, 0052ED20
        }
        // 0052F89B..0052F8D2: next indent, row y and the row step (x87, stored to float).
        next_indent = static_cast<float>(static_cast<double>(width) + screen.indent_48);
        row_y = static_cast<float>(static_cast<double>(y) - screen.row_space_44);
        y = static_cast<float>(static_cast<double>(y) + screen.row_space_44 + height);
    }
    screen.content_height_38 = y;                                         // 0052F8DB..0052F8E4
    host.widget_virtual_70(screen.frame_18);                              // 0052F8E1..0052F8EE
    screen.speed_offset_20 = 0.0F;                                        // 0052F8F0..0052F8FA
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

// 0053DD90, __thiscall(screen), RET; ESI = screen, EBX = 0.
void free_camera_enter_0053dd90(FreeCameraState& screen, FreeCameraHost& host) {
    if (screen.mover_24 == 0) {                                          // 0053DDAE..0053DE80
        screen.mover_24 = host.create_mover_00432750();
        FreeCameraMatrix identity{};
        identity[0] = identity[5] = identity[10] = identity[15] = 1.0F;  // 0053DE03..0053DE5D
        host.mover_place_00923870(screen.mover_24, 0, identity);
        host.observer_register_pair_00694a60(screen.mover_24);
    }
    // 0053DE85..0053DEC0: a pose saved by an earlier screen wins once, else the player camera.
    const FreeCameraMatrix source = host.take_saved_pose_e192a9() ? host.saved_pose_e192b0()
                                                                  : host.player_camera_world_19fc();
    host.mover_set_matrix_74_004134f0(screen.mover_24, source);
    screen.angles_54 = host.mover_angles_0042d2e0(screen.mover_24);      // 0053DEC5..0053DED6
    host.mover_invalidate_pose_tree(screen.mover_24);                    // 0053DEDB..0053DEFD
    host.game_set_camera_mover_004bc410(screen.mover_24);                // 0053DEFF..0053DF09
    screen.speed_forward_38 = 0.0F;                                      // 0053DF0E..0053DF2A
    screen.speed_strafe_3c = 0.0F;
    screen.speed_vertical_40 = 0.0F;
    screen.rate_yaw_44 = 0.0F;
    screen.rate_pitch_48 = 0.0F;
}

namespace {

// One smoothed channel (00540472..005404FE and four copies): opposite signs stop at once, a smaller
// magnitude is taken at once, a larger one is approached by dt * scale * +34h per update. The
// product and both magnitudes are compared in x87 after float stores; the step is rounded to float
// before the call.
void free_camera_smooth(float& value, float target, double scale, float seconds, float acceleration) {
    if (value * target < 0.0F) {                                         // FCOMIP; JB
        value = 0.0F;
        return;
    }
    if (std::fabs(target) <= std::fabs(value)) {                         // AND 7FFFFFFFh; FCOMIP; JBE
        value = target;
        return;
    }
    const float step = static_cast<float>(static_cast<double>(seconds) * scale * acceleration);
    value = unit_step_towards_0042ac60(value, target, step);
}

}  // namespace

// 00540390, __thiscall(screen, float seconds), RET 4; ESI = screen. Locals: [ESP+18h] forward
// input, later angle Y; [ESP+14h] strafe input, later angle Z; [ESP+10h] vertical input, later
// angle X; [ESP+20h]/[ESP+24h] yaw/pitch inputs; [ESP+2Ch] roll input.
void free_camera_update_00540390(FreeCameraState& screen, float seconds, FreeCameraHost& host) {
    if (!screen.active_04 || screen.mover_24 == 0) return;              // 00540398..005403A7
    if (!host.game_has_player_units_1970()) {                            // 005403AD..005403D7
        host.show_message_004c7070(kFreeCameraNoUnitsMessageA, kFreeCameraNoUnitsMessageB, kFreeCameraNoUnitsText);
    }
    // 005403DF..0054046E: each axis times its double scale, stored to float.
    const float forward = static_cast<float>(host.input_axis(FreeCameraAxis::Forward) * kFreeCameraForwardScale);
    const float strafe = static_cast<float>(host.input_axis(FreeCameraAxis::Strafe) * kFreeCameraStrafeScale);
    const float vertical = static_cast<float>(host.input_axis(FreeCameraAxis::Vertical) * kFreeCameraStrafeScale);
    const float yaw = static_cast<float>(host.input_axis(FreeCameraAxis::Yaw) * kFreeCameraTurnScale);
    const float pitch = static_cast<float>(host.input_axis(FreeCameraAxis::Pitch) * kFreeCameraTurnScale);
    const float roll = static_cast<float>(host.input_axis(FreeCameraAxis::Roll) * kFreeCameraRollScale);
    free_camera_smooth(screen.speed_forward_38, forward, kFreeCameraForwardScale, seconds, screen.acceleration_34);
    free_camera_smooth(screen.speed_strafe_3c, strafe, kFreeCameraStrafeScale, seconds, screen.acceleration_34);
    free_camera_smooth(screen.speed_vertical_40, vertical, kFreeCameraStrafeScale, seconds, screen.acceleration_34);
    free_camera_smooth(screen.rate_yaw_44, yaw, kFreeCameraTurnScale, seconds, screen.acceleration_34);
    free_camera_smooth(screen.rate_pitch_48, pitch, kFreeCameraTurnScale, seconds, screen.acceleration_34);
    // 00540748..005408B4: velocity = forward * row2 + strafe * row0 + vertical * row1 of the
    // mover's world matrix (+ECh, +CCh, +DCh), every product and sum stored to float; then
    // position += dt * velocity.
    const auto rows = host.mover_world_rows_cc(screen.mover_24);
    std::array<float, 3> velocity{};
    for (std::size_t i = 0; i < 3; ++i) {
        const float up = screen.speed_vertical_40 * rows[3 + i];
        const float side = screen.speed_strafe_3c * rows[i];
        const float ahead = screen.speed_forward_38 * rows[6 + i];
        const float sum = ahead + side;
        velocity[i] = sum + up;
    }
    auto& position = host.mover_position_a4(screen.mover_24);
    for (std::size_t i = 0; i < 3; ++i) position[i] = position[i] + seconds * velocity[i];
    std::array<float, 3> angles = host.mover_angles_0042d2e0(screen.mover_24);  // 005408B7..005408C7
    float& angle_x = angles[0];
    float& angle_y = angles[1];
    float& angle_z = angles[2];
    // 005408CC..00540991: the two reset controls step the angles toward 0 at pi/2 per second.
    bool resetting = false;
    if (host.input_control_active(kFreeCameraResetPitchControl)) {
        angle_x = unit_step_towards_0042ac60(angle_x, 0.0F, static_cast<float>(seconds * kFreeCameraResetRate));
        resetting = true;
    }
    if (host.input_control_active(kFreeCameraResetAllControl)) {
        const float step = static_cast<float>(seconds * kFreeCameraResetRate);
        angle_x = unit_step_towards_0042ac60(angle_x, 0.0F, step);
        angle_y = unit_step_towards_0042ac60(angle_y, 0.0F, step);
        angle_z = unit_step_towards_0042ac60(angle_z, 0.0F, step);
    } else if (!resetting) {                                             // 00540996..00540A3C
        angle_x = wrapped_angle_add_00438aa0(angle_x, screen.rate_pitch_48 * seconds);
        if (angle_x > kFreeCameraPitchLimit) {                           // FCOMIP; JBE skips
            angle_x = kFreeCameraPitchLimit;
        } else if (-kFreeCameraPitchLimit > angle_x) {                   // COMISS; JBE skips
            angle_x = -kFreeCameraPitchLimit;
        }
        angle_y = wrapped_angle_add_00438aa0(angle_y, screen.rate_yaw_44 * seconds);
        angle_z = wrapped_angle_add_00438aa0(angle_z, roll * seconds);   // the raw roll input, unsmoothed
    }
    host.mover_set_rotation_00467050(screen.mover_24, angle_x, angle_y, angle_z);  // 00540A40..00540A60
    if (host.action_pressed_004c43c0(kFreeCameraMarkAction)) {           // 00540A65..00540A92
        host.mark_position_0053d230(host.mover_world_translation_fc(screen.mover_24));
    }
    host.mover_invalidate_subtree_0042ed50(screen.mover_24);             // 00540A97..00540A9A
    host.mover_apply_shake_and_publish_004329d0(screen.mover_24, seconds);  // 00540A9F..00540AAA
    screen.burst_64 = screen.field_20;                                   // 00540AAF..00540AB8
    host.burst_update_0053f2d0(seconds);                                 // 00540ABA..00540ADB
}

namespace {

using TlKind = TacticalLibraryKind;
using CommandBar = std::array<MainMenuCommandArgument, 5>;

// The four-plus-back bars of the kind pages (decompiled argument lists, string pushes confirmed in
// the listing).
constexpr CommandBar kTlBarScrollNavigate{{{0, "", 1}, {0xB2, "globals.scroll_menu", 0},
    {0xB3, "globals.navigate", 1}, {0xA3, "globals.back", 2}, {0, "", 1}}};

}  // namespace

// 00511F40, __thiscall(screen), RET; ESI = screen, EBX = 5 until 00512547, [ESP+24h] the panel
// template the kind picks. Sections: 00511F65..005125D4 common setup, jump table 00514BD8 on +94h
// (0..8, above 8 -> 00513808), 0051380C..005145E2 the three-panel carousel, 005145E2..00514BD6 the
// list selection and frame layout.
void tactical_library_enter_00511f40(TacticalLibraryEnterState& screen, TacticalLibraryEnterHost& host) {
    host.singleton_f8d394_00b33ac0(1);                                   // 00511F6D..00511F75
    host.reset_004f8fa0();                                               // 00511F7A..00511F7C
    screen.show_side_arrows_560 = true;                                  // 00511F87
    screen.selected_value_3c0 = 0;                                       // 00511F8E
    for (const std::uintptr_t widget : screen.hidden_128) host.widget_show_34(widget, false);  // +12Ch, +128h, +130h
    if (screen.kind_94 == TlKind::NavalAcademy) {                        // 00511FB8..00511FED
        host.diagnostic_004254b0(host.lua_run_string_006b8ad0(kTacticalLibraryControlNamesLua)
            ? "luaLoadControlFunctionNames() lua parancs vegrehajtva a a GIAchievements-ben."
            : "Nem sikerult vegrehajtani a luaLoadControlFunctionNames() lua parancsot a GIAchievements-ben.");
    }
    // 00511FF5..00512041: an entry to open switches to kind 6 (+A4h set) or 5.
    if (screen.kind_94 != TlKind::None && screen.entry_arg_a0 != 2 && screen.entry_9c != 0) {
        if (screen.entry_unlocks_a4) {
            screen.kind_94 = TlKind::Unlocks;
            host.open_entry_unlocks_0050e3c0(screen.entry_9c, screen.entry_arg_a0);
        } else {
            screen.kind_94 = TlKind::OrderOfBattle;
            host.open_entry_order_of_battle_0050ef80(screen.entry_9c, screen.entry_arg_a0);
        }
    }
    host.widget_show_34(screen.list_bc, false);                          // 00512046..00512060
    host.widget_show_34(screen.list_c0, false);
    screen.list_b8 = screen.kind_94 == TlKind::OrderOfBattle ? screen.list_bc : screen.list_c0;  // 00512062..0051207E
    screen.item_group_ac = host.find_child_00aa7e00(screen.list_b8, "Item_Group", true);        // 00512084..005120D0
    host.widget_show_34(screen.item_group_ac, false);
    screen.main_text_b0 = host.find_child_00aa7e00(screen.list_frame_c4, "MainListbox_Text", true);  // 0051210F..0051215D
    host.widget_show_34(screen.main_text_b0, false);
    if (screen.kind_94 != TlKind::None) {                                // 00512197..005121D9
        host.list_set_pads_12c(screen.list_b8, 0.0F,
            screen.kind_94 == TlKind::OrderOfBattle ? kTacticalLibraryListPad130Oob : kTacticalLibraryListPad130,
            kTacticalLibraryListPad134);
    }
    host.list_clear_rows_00a9bec0(screen.list_b8);                       // 005121E1..005121F9
    host.widget_virtual_60(screen.list_b8, 1);
    screen.item_group_ac = host.find_child_00aa7e00(screen.list_b8, "Item_Group", true);  // 005121FD..0051224B
    host.list_set_listener_00a9ac40(screen.list_b8);                     // 00512277..00512281
    host.widget_virtual_60(screen.list_b8, 1);
    host.list_select_index_00a9c7c0(screen.list_b8, 0);                  // 00512295..0051229C
    host.widget_virtual_80(screen.list_b8, 1);
    host.widget_show_34(screen.list_frame_c4, false);                    // 005122B3..005122D1
    host.widget_set_byte_77(screen.list_frame_c4, true);
    host.widget_set_byte_77(screen.list_b8, false);
    host.widget_show_34(screen.list_b8, true);                           // 005122D5..005122E2
    if (screen.kind_94 == TlKind::OrderOfBattle) {                       // 005122E4..005122F5
        host.show_order_of_battle_page_004fff20();
    } else if (screen.kind_94 != TlKind::None) {                         // 005122FA..005123A2
        host.rebuild_command_bar_0054b530({{{0xA2, "globals.select", 0}, {0, "", 1}, {0, "", 1}, {0, "", 1},
            {0xA3, "globals.back", 2}}});
    }
    for (const std::uintptr_t widget : screen.templates_4ec) host.widget_show_34(widget, false);  // 00512468..005124C0
    for (std::size_t i = 0; i < kTacticalLibraryRowPairs; ++i) {         // 005124C2..00512543
        screen.rows_1d8[i] = host.clone_subtree_00aab4c0(screen.row_template_56c, 0);
        screen.rows_278[i] = host.clone_subtree_00aab4c0(screen.row_template_570, 0);
        host.widget_show_34(screen.rows_1d8[i], false);
        host.widget_show_34(screen.rows_278[i], false);
        host.widget_set_dword_e0(screen.rows_1d8[i], 1);
        host.widget_set_dword_e0(screen.rows_278[i], 1);
        host.set_listener_00aa6bc0(screen.rows_1d8[i], true);
        host.set_listener_00aa6bc0(screen.rows_278[i], true);
    }
    host.widget_show_34(host.find_child_00aa7e00(screen.info_parent_4dc, "INFO_box_Group", true), true);  // 00512545..005125A0

    std::uintptr_t panel_template = 0;                                   // [ESP+24h]
    const auto& t = screen.templates_4ec;                                // +4ECh, +4F0h, +4F4h, +4F8h, +500h, +504h
    switch (screen.kind_94) {
    case TlKind::PersonalAchievements:                                   // 00512853
        host.prepare_achievements_00509de0();
        panel_template = t[2];
        host.frame_select_00518d60("FE.taclib_personalachievements_title");
        host.set_help_line_0054a0c0("FE.taclib_personalachievements_help");
        host.build_achievements_list_005012f0();
        host.rebuild_command_bar_0054b530(kTlBarScrollNavigate);
        break;
    case TlKind::PowerUps:                                               // 00512F1F
        panel_template = t[3];
        host.frame_select_00518d60("FE.taclib_powerups_title");
        host.set_help_line_0054a0c0("FE.taclib_powerups_help");
        host.build_power_up_list_00500e90();
        host.rebuild_command_bar_0054b530(kTlBarScrollNavigate);
        break;
    case TlKind::UnitsWeapons:                                           // 0051316A
        panel_template = t[0];
        host.frame_select_00518d60("FE.taclib_unitsweapons_title");
        host.set_help_line_0054a0c0("FE.taclib_unitsweapons_help");
        host.build_units_weapons_list_00500860();
        host.preview_setup_00502480();
        host.rebuild_command_bar_0054b530({{{0, "", 0}, {0xA7, "globals.3dview", 1}, {0xB3, "globals.navigate", 1},
            {0xB2, "globals.scroll_menu", 1}, {0xA3, "globals.back", 2}}});
        break;
    case TlKind::Movies:                                                 // 005125E1
        host.prepare_movies_004fac50();
        panel_template = t[1];
        host.frame_select_00518d60("FE.taclib_movies_title");
        host.set_help_line_0054a0c0("FE.taclib_movies_help");
        host.build_movie_list_00500450();
        host.rebuild_command_bar_0054b530({{{0xA2, "globals.playmovie", 0}, {0, "", 1}, {0xB3, "globals.navigate", 1},
            {0xB2, "globals.scroll_menu", 1}, {0xA3, "globals.back", 2}}});
        break;
    case TlKind::NavalAcademy: {                                         // 00512ABA
        panel_template = t[4];
        host.frame_select_00518d60("FE.taclib_navalacademy_title");
        host.set_help_line_0054a0c0("FE.taclib_navalacademy_help");
        const std::size_t count = host.academy_record_count();
        if (screen.entry_9c == 0) {                                      // 00512B79 -> 00512EAD
            for (std::size_t i = 0; i < count; ++i) host.academy_set_marked_44(i, true);
        } else {                                                         // 00512B85..00512C85
            for (std::size_t i = 0; i < count; ++i) host.academy_set_marked_44(i, false);
            std::uint32_t last = 0;
            for (const std::uint32_t id : host.entry_ids_8c(screen.entry_9c)) {
                const std::size_t index = host.academy_find_004faac0(id);
                host.academy_set_marked_44(index, true);
                last = host.academy_value_40(index);
            }
            screen.selected_value_3c0 = last;
        }
        host.build_academy_list_005010c0();                              // 00512C8D..00512C8F
        host.rebuild_command_bar_0054b530(kTlBarScrollNavigate);
        break;
    }
    case TlKind::OrderOfBattle:                                          // 005133CE
        panel_template = t[0];
        screen.show_side_arrows_560 = false;
        host.frame_select_00518d60("FE.taclib_orderofbattle_title");
        host.set_help_line_0054a0c0("FE.taclib_orderofbattle_help");
        host.build_order_of_battle_list_00506660();
        host.vehicle_class_registry_004356f0();
        host.preview_setup_00502480();
        host.show_order_of_battle_page_004fff20();
        break;
    case TlKind::Unlocks:                                                // 005134BA, no frame title
        host.prepare_unlocks_005021e0();
        panel_template = t[0];
        screen.show_side_arrows_560 = false;
        host.set_help_line_0054a0c0("FE.taclib_unlocks_help");
        host.build_unlocks_list_005001a0();
        host.preview_setup_00502480();
        host.show_unlocks_page_004ffd00();
        break;
    case TlKind::NoseArt:                                                // 00513547
        panel_template = t[5];
        host.frame_select_00518d60("FE.taclib_noseart_title");
        host.set_help_line_0054a0c0("FE.taclib_noseart_help");
        host.build_nose_art_list_00500640();
        host.widget_show_34(host.find_child_00aa7e00(screen.info_parent_4dc, "INFO_box_Group", true), false);
        host.rebuild_command_bar_0054b530({{{0xA2, "globals.select", 0}, {0, "", 1}, {0xB3, "globals.navigate", 1},
            {0, "", 1}, {0xA3, "globals.cancel", 2}}});
        break;
    case TlKind::None:                                                   // 00513801
        screen.show_side_arrows_560 = false;
        break;
    default: break;                                                      // 00513808
    }
    if (!screen.show_side_arrows_560) {                                  // 0051380C..00513847
        for (const std::uintptr_t arrow : screen.arrows_574) {
            host.widget_show_34(arrow, false);
            host.set_listener_00aa6bc0(arrow, false);
        }
    }
    if (panel_template != 0) {                                           // 0051384C..005145DF
        for (std::uintptr_t& panel : screen.panels_508) panel = host.clone_subtree_00aab4c0(panel_template, 0);
        for (const std::uintptr_t panel : screen.panels_508) host.widget_show_34(panel, true);
        host.widget_set_scale_48(screen.panels_508[0], kTacticalLibrarySidePanelScale, kTacticalLibrarySidePanelScale);
        host.widget_set_scale_48(screen.panels_508[1], 1.0F, 1.0F);
        host.widget_set_scale_48(screen.panels_508[2], kTacticalLibrarySidePanelScale, kTacticalLibrarySidePanelScale);
        screen.panel_spacing_43c = kTacticalLibraryPanelSpacing;         // 00513921..00513957
        screen.centre_x_440 = host.widget_local_x_00aa67f0(screen.panels_508[1]);
        host.widget_set_local_x_00aa78d0(screen.panels_508[0],           // x87, rounded through a float store
            static_cast<float>(static_cast<double>(screen.centre_x_440) - screen.panel_spacing_43c));
        host.widget_set_local_x_00aa78d0(screen.panels_508[1], screen.centre_x_440);
        host.widget_set_local_x_00aa78d0(screen.panels_508[2],
            static_cast<float>(static_cast<double>(screen.centre_x_440) + screen.panel_spacing_43c));
        for (std::size_t i = 0; i < 3; ++i) {                            // 005139A7..00513DD2
            screen.pic_bg_580[i] = host.find_child_00aa7e00(screen.panels_508[i], "pic_BG_Icon", true);
        }
        for (std::size_t i = 0; i < 3; ++i) {
            screen.big_bg_57c[i] = host.find_child_00aa7e00(screen.panels_508[i], "icon_big_BG_Icon", true);
        }
        for (std::size_t i = 0; i < 3; ++i) {
            screen.big_bg2_584[i] = host.find_child_00aa7e00(screen.panels_508[i], "icon_big_BG2_Icon", false);
        }
        if (host.find_child_00aa7e00(screen.panels_508[0], "keret_Framebox", false) != 0) {  // 00513E28..00514024
            for (std::size_t i = 0; i < 3; ++i) {
                screen.frames_5a0[i] = host.find_child_00aa7e00(screen.panels_508[i], "keret_Framebox", true);
                host.widget_show_34(screen.frames_5a0[i], true);
            }
        }
        screen.lock_540 = {};                                             // 0051402E..0051404C
        screen.hide_54c = {};
        if (screen.kind_94 == TlKind::PersonalAchievements) {            // 0051407D..0051419B
            for (std::size_t i = 0; i < 3; ++i) {
                screen.lock_540[i] = host.find_child_00aa7e00(screen.panels_508[i], "lock_Icon", true);
            }
        } else if (screen.kind_94 == TlKind::NavalAcademy) {             // 005141F0..005144C6
            for (std::size_t i = 0; i < 3; ++i) {
                screen.video_51c[i] = host.find_child_00aa7e00(screen.panels_508[i], "video_Movie", true);
                host.widget_set_byte_f4(screen.video_51c[i], false);
                screen.video_pic_528[i] = host.clone_subtree_00aab4c0(
                    host.find_child_00aa7e00(screen.panels_508[i], "pic_BG_Icon", true), 0);
            }
        } else if (screen.kind_94 == TlKind::Movies) {                   // 005144DA..005145B1
            for (std::size_t i = 0; i < 3; ++i) {
                screen.hide_54c[i] = host.find_child_00aa7e00(screen.panels_508[i], "pic_Hide_Icon", true);
            }
        }
    }
    // 005145E2..00514668: the kind's data list (+3C4h..+414h, 10h apart) into the selection +444h.
    switch (screen.kind_94) {
    case TlKind::PersonalAchievements: host.select_list_entry_0050d900(0x3D4); break;
    case TlKind::PowerUps: host.select_list_entry_0050d900(0x3E4); break;
    case TlKind::UnitsWeapons:
    case TlKind::OrderOfBattle:
    case TlKind::Unlocks: host.select_list_entry_0050d900(0x3F4); break;
    case TlKind::Movies: host.select_list_entry_0050d900(0x3C4); break;
    case TlKind::NavalAcademy: host.select_list_entry_0050d900(0x404); break;
    case TlKind::NoseArt: host.select_list_entry_0050d900(0x414); break;
    default: break;
    }
    host.show_selection_00511bf0(screen.selected_value_3c0);             // 0051466D..0051467D
    if (screen.kind_94 == TlKind::NoseArt) {                             // 00514682..0051469F
        host.nose_art_refresh_004fc8c0();
    } else if (screen.kind_94 == TlKind::Unlocks) {
        host.unlocks_refresh_004fbe90(1);
    }
    // 005146A4..00514BAF: each keret frame takes its own size and sits on its panel's
    // icon_big_BG_Icon origin, one unit nearer (z + 1.0 from 00D7A210).
    if (screen.panels_508[0] != 0 && host.find_child_00aa7e00(screen.panels_508[0], "keret_Framebox", false) != 0) {
        for (std::size_t i = 0; i < 3; ++i) {
            screen.frames_5a0[i] = host.find_child_00aa7e00(screen.panels_508[i], "keret_Framebox", true);
            host.widget_set_size_58(screen.frames_5a0[i], host.group_size_00aa6740(screen.frames_5a0[i]));
            const auto origin = host.group_origin_00aa6750(
                host.find_child_00aa7e00(screen.panels_508[i], "icon_big_BG_Icon", false));
            host.set_resolved_position_00aa8240(screen.frames_5a0[i], {static_cast<float>(origin[0] + 0.0),
                static_cast<float>(0.0 + origin[1]), static_cast<float>(origin[2] + 1.0)});
            host.widget_show_34(screen.frames_5a0[i], true);
        }
    }
    screen.field_5b4 = -1.0F;                                            // 00514BB1..00514BC0
}

// ---------------------------------------------------------------------------
// Tactical library Update 005162B0, carousel step 00516010, nose-art apply 005161C0 (cc13-menus6)
// ---------------------------------------------------------------------------

namespace {

// 00516639..005166A9 and its copies: virtual +94h is read twice; a frame outside 1..2 is reset to 0.
void tactical_library_settle_arrow(std::uintptr_t widget, TacticalLibraryUpdateHost& host) {
    if (host.widget_frame_94(widget) < 1 || host.widget_frame_94(widget) > 2) {
        host.widget_virtual_88(widget, kTacticalLibraryArrowNormal, 0, 1.0F);
    }
}

void tactical_library_set_arrow(std::uintptr_t widget, std::uint16_t frame, TacticalLibraryUpdateHost& host) {
    host.widget_virtual_88(widget, frame, 0, 1.0F);   // FLD1 / FSTP [esp], push 0, push frame
}

// The left arrow sits left of the row origin by its own width / 1.2 (FDIV qword 00CEC160, FSUBR the
// origin x spilled as a double), the right arrow at origin x + the highlight widget's width (FADD of
// two floats); both drop by a double constant. Kind 5 forces z = -10, kind 6 keeps the origin z.
std::array<float, 3> tactical_library_left_arrow_position(const std::array<float, 3>& origin, float arrow_width,
    double drop, float z) noexcept {
    return {static_cast<float>(static_cast<double>(origin[0]) -
                static_cast<double>(arrow_width) / kTacticalLibraryArrowWidthDivisor),
        static_cast<float>(static_cast<double>(origin[1]) - drop), z};
}

std::array<float, 3> tactical_library_right_arrow_position(const std::array<float, 3>& origin, float highlight_width,
    double drop, float z) noexcept {
    return {highlight_width + origin[0], static_cast<float>(static_cast<double>(origin[1]) - drop), z};
}

std::array<MainMenuCommandArgument, 5> tactical_library_continue_bar(bool change) {
    if (change) {
        // 00516F64..00516FFD: continue / change / navigate / 3dview / back.
        return {{{0xA2, "globals.continue", 0}, {0xAF, "globals.change", 1}, {0xB3, "globals.navigate", 1},
            {0xA7, "globals.3dview", 1}, {0xA3, "globals.back", 2}}};
    }
    // 00516DFE..00516E97: continue / (empty) / navigate / back / (empty).
    return {{{0xA2, "globals.continue", 0}, {0x00, "", 1}, {0xB3, "globals.navigate", 1},
        {0xA3, "globals.back", 2}, {0x00, "", 1}}};
}

// 00516481..00516849: order-of-battle rows. EDI is the record index parsed from the row's data
// text, [esp+10h] the row counter; arrows are indexed by the record, not by the row.
bool tactical_library_place_order_of_battle_rows(TacticalLibraryUpdateState& screen,
    TacticalLibraryUpdateHost& host) {
    auto& records = screen.order_of_battle_460;
    for (std::int32_t row_index = 0; static_cast<std::size_t>(row_index) < records.size(); ++row_index) {
        const std::uintptr_t row = host.list_row_at_00a9be00(screen.list_b8, row_index);
        const auto index = static_cast<std::uint32_t>(std::atol(host.row_data_text_28(row).c_str()));
        if (index >= records.size()) {
            host.invalid_parameter_00bf6713();   // 0051650A, the CRT's range check
            return false;
        }
        host.order_of_battle_row_004fb090(records[index].key_00);
        const std::uintptr_t left = screen.rows_1d8[index];
        const std::uintptr_t right = screen.rows_278[index];
        if (records[index].flag_0c == 0) {                                   // 0051681E
            host.widget_show_34(left, false);
            host.widget_show_34(right, false);
            continue;
        }
        const float highlight_width = host.group_size_00aa6740(host.gui_highlight_widget_00aa0f50(1))[0];
        const auto origin = host.group_origin_00aa6750(row);
        host.set_resolved_position_00aa8240(left, tactical_library_left_arrow_position(origin,
            host.group_size_00aa6740(left)[0], kTacticalLibraryArrowDropOob, kTacticalLibraryArrowZOob));
        host.widget_show_34(left, true);
        host.widget_set_dword_d8(left, row_index);
        const std::size_t children = records[index].child_count_18;    // 004F9360 re-reads the record
        if (children == 0 && records[index].flag_0c == 1) {
            tactical_library_set_arrow(left, kTacticalLibraryArrowLocked, host);
        } else if (screen.sub_index_138[index] == 0) {
            tactical_library_set_arrow(left, kTacticalLibraryArrowDisabled, host);
        } else {
            tactical_library_settle_arrow(left, host);
        }
        const auto origin_right = host.group_origin_00aa6750(row);      // 005166AB, read again
        host.set_resolved_position_00aa8240(right, tactical_library_right_arrow_position(origin_right,
            highlight_width, kTacticalLibraryArrowDropOob, kTacticalLibraryArrowZOob));
        host.widget_show_34(right, true);
        host.widget_set_dword_d8(right, row_index);
        if (children == 0 && records[index].flag_0c == 1) {
            tactical_library_set_arrow(right, kTacticalLibraryArrowLocked, host);
        } else if (screen.sub_index_138[index] == static_cast<std::int32_t>(children)) {
            tactical_library_set_arrow(right, kTacticalLibraryArrowDisabled, host);
        } else {
            tactical_library_settle_arrow(right, host);
        }
    }
    return true;
}

// 0051685B..00516B10: unlock rows. EBP is the row and record index, EDI walks +1D8h (EDI-0A0h =
// +138h, EDI+0A0h = +278h), [esp+10h] the record byte offset; the image never bounds the arrays.
void tactical_library_place_unlock_rows(TacticalLibraryUpdateState& screen, TacticalLibraryUpdateHost& host) {
    const auto& records = screen.unlocks_470;
    for (std::size_t i = 0; i < records.size() && i < kTacticalLibraryRowPairs; ++i) {
        const std::uintptr_t left = screen.rows_1d8[i];
        const std::uintptr_t right = screen.rows_278[i];
        const std::size_t children = records[i].child_count_18;
        if (children == 0) {                                              // 00516AEB
            host.widget_show_34(left, false);
            host.widget_show_34(right, false);
            continue;
        }
        const std::uintptr_t row = host.list_row_at_00a9be00(screen.list_b8, static_cast<std::int32_t>(i));
        const float highlight_width = host.group_size_00aa6740(host.gui_highlight_widget_00aa0f50(1))[0];
        const auto origin = host.group_origin_00aa6750(row);
        host.set_resolved_position_00aa8240(left, tactical_library_left_arrow_position(origin,
            host.group_size_00aa6740(left)[0], kTacticalLibraryArrowDropUnlocks, origin[2]));
        host.widget_show_34(left, true);
        if (screen.sub_index_138[i] == 0) {
            tactical_library_set_arrow(left, kTacticalLibraryArrowDisabled, host);
        } else {
            tactical_library_settle_arrow(left, host);
        }
        const auto origin_right = host.group_origin_00aa6750(row);
        host.set_resolved_position_00aa8240(right, tactical_library_right_arrow_position(origin_right,
            highlight_width, kTacticalLibraryArrowDropUnlocks, origin_right[2]));
        host.widget_show_34(right, true);
        if (screen.sub_index_138[i] == static_cast<std::int32_t>(children) - 1) {   // ADD EAX,-1
            tactical_library_set_arrow(right, kTacticalLibraryArrowDisabled, host);
        } else {
            tactical_library_settle_arrow(right, host);
        }
    }
}

}  // namespace

void tactical_library_update_005162b0(TacticalLibraryUpdateState& screen, float dt,
    TacticalLibraryUpdateHost& host) {
    host.scroller_update_00683820(1);                                    // 005162CC..005162D4
    if (screen.carousel_timer_8c > 0.0F) {                               // COMISS 00D7A218 (0.0)
        screen.carousel_timer_8c -= dt;
    }
    // 00516300..00516387: a pending launch counts down and then plays the selected mission.
    if (screen.countdown_5b4 != kTacticalLibraryNoCountdown) {
        screen.countdown_5b4 -= dt;
        if (screen.countdown_5b4 <= 0.0F) {
            host.loading_queue_stop_and_wait_005092e0();
            host.gui_reset_screens_00aa0f70();
            host.set_game_selection_6ac(host.list_selected_index_00a9c920(screen.list_c4));
            host.play_selected_mission_005922f0();
            screen.countdown_5b4 = kTacticalLibraryNoCountdown;
        }
    }
    // 0051638B..00516434: kind 3 skips a playing movie and restarts the menu stream (empty name 00E18D78).
    if (screen.kind_94 == TlKind::Movies && host.movie_playing_004f89a0() && host.action_004d92b0(0x4B)) {
        host.movie_stop_004f8ac0();
        host.menu_stream_start_00a867b0("");
        host.menu_stream_fade_in_00a85c20();
        return;
    }
    if (screen.kind_94 == TlKind::OrderOfBattle) {                      // 00516437..0051647E
        host.profile_hints_00427190(6);
        if (host.action_004d92b0(0x4A)) {
            host.order_of_battle_cancel_004ff6f0();
            return;
        }
        if (!tactical_library_place_order_of_battle_rows(screen, host)) {
            return;   // the CRT range-check abort (00BF6713) stands here
        }
    }
    if (screen.kind_94 == TlKind::Unlocks) {
        tactical_library_place_unlock_rows(screen, host);
    }
    if (screen.kind_94 == TlKind::None) {                                // 00516B15..00516B8D
        if (host.action_004d92b0(0x4B)) {
            host.kind7_activate_00514fb0();
            return;
        }
        if (host.action_004d92b0(0x4A) && screen.countdown_5b4 == kTacticalLibraryNoCountdown) {
            screen.countdown_5b4 = kTacticalLibraryCountdownStart;
        }
        host.profile_hints_00427190(7);
    }
    // 00516B92..00516D6E: in the 3D view, 4Bh leaves it.
    if (screen.view_90 == 1 && host.action_004d92b0(0x4B)) {
        host.toggle_view_00511b40();
        if (screen.kind_94 == TlKind::OrderOfBattle) {
            host.show_order_of_battle_page_004fff20();
        } else if (screen.kind_94 == TlKind::Unlocks) {
            host.show_unlocks_page_004ffd00();
        } else if (screen.kind_94 == TlKind::UnitsWeapons) {
            host.rebuild_command_bar_0054b530({{{0x00, "", 0}, {0xA7, "globals.3dview", 1},
                {0xB3, "globals.navigate", 1}, {0xB2, "globals.scroll_menu", 1}, {0xA3, "globals.back", 2}}});
        }
        return;
    }
    // 00516D71..0051714B: the rest of the input chain; every branch joins at 0051714B.
    const bool unit_kind = screen.kind_94 == TlKind::UnitsWeapons || screen.kind_94 == TlKind::OrderOfBattle ||
        screen.kind_94 == TlKind::Unlocks;
    if (host.console_flag_f88a30() && host.action_004d92b0(0x50) && unit_kind) {
        host.switch_view_00514c20();
    } else if (screen.view_90 == 0 && host.action_004d92b0(0x4B)) {   // 00516DAC
        if (host.list_byte_85(screen.list_b8)) {
            host.back_00515e00();                                        // 00517104
        } else {
            host.widget_virtual_60(screen.list_b8, 1);
            if (screen.kind_94 == TlKind::OrderOfBattle) {
                host.show_order_of_battle_page_004fff20();
            } else {
                host.rebuild_command_bar_0054b530(tactical_library_continue_bar(false));
            }
        }
    } else if (host.action_004d92b0(0x50) && screen.kind_94 == TlKind::OrderOfBattle) {  // 00516F11
        const std::uintptr_t row = host.list_selected_row_00a9ca00(screen.list_b8);
        host.show_selection_00511bf0(static_cast<std::int32_t>(std::atol(host.row_data_text_28(row).c_str())));
        host.rebuild_command_bar_0054b530(tactical_library_continue_bar(true));
    } else if (host.action_004d92b0(0x4A)) {                             // 005170C1
        if (screen.kind_94 == TlKind::Movies) {
            host.movies_cancel_004ff580();
        } else if (screen.kind_94 == TlKind::NoseArt) {
            host.nose_art_apply_005161c0();
        } else if (screen.kind_94 == TlKind::Unlocks && host.unlocks_cancel_004fc9d0()) {
            host.back_00515e00();
        }
    } else if (host.action_004d92b0(0x4C) && screen.kind_94 != TlKind::None) {  // 0051710D
        host.carousel_step_00516010(true);
    } else if (host.action_004d92b0(0x4D) && screen.kind_94 != TlKind::None) {
        host.carousel_step_00516010(false);
    }
    if (screen.kind_94 != TlKind::None) {                                // 0051714B..0051715E
        host.carousel_animate_00515050(dt);
    }
    if (screen.kind_94 != TlKind::NoseArt) {
        return;
    }
    // 00517170..00517262: kind 8 lights the keret frame of the panel showing the profile's nose art.
    const std::uint32_t current = host.profile_field_28() ? host.game_nose_art_730() : host.game_nose_art_734();
    for (const std::uintptr_t frame : screen.frames_5a0) {
        host.widget_virtual_84(frame, false);
    }
    std::uintptr_t lit = 0;
    if (host.widget_virtual_38(screen.panels_508[0]) && screen.scroll_value_438 > 1.0F &&   // 00D7A24C
        host.selector_record_48_004fa8f0() == current) {
        lit = screen.frames_5a0[0];
    } else if (host.widget_virtual_38(screen.panels_508[1]) && host.selector_record_48_004fa960() == current) {
        lit = screen.frames_5a0[1];
    } else if (host.widget_virtual_38(screen.panels_508[2]) && host.selector_record_48_004faa00() == current) {
        lit = screen.frames_5a0[2];
    }
    if (lit != 0) {
        host.widget_virtual_84(lit, true);
    }
}

void tactical_library_carousel_step_00516010(TacticalLibraryCarouselState& screen, bool forward,
    TacticalLibraryCarouselHost& host) {
    if (screen.kind_94 == TlKind::None) {
        return;
    }
    if (screen.kind_94 == TlKind::NavalAcademy && screen.carousel_timer_8c > 0.0F) {   // 0051602F..0051603E
        return;
    }
    screen.carousel_timer_8c = kTacticalLibraryCarouselTime;
    if (forward) {
        host.selector_step_004fc720();
    } else {
        host.selector_step_004fc7a0();
    }
    host.refresh_entry_00510c20();
    if (screen.kind_94 == TlKind::OrderOfBattle) {
        host.order_of_battle_refresh_00506550();
    } else if (screen.kind_94 == TlKind::Unlocks) {
        host.unlocks_refresh_00504c60();
    }
    host.carousel_animate_00515050(kTacticalLibraryCarouselSettleDt);
    const double pad = screen.kind_94 == TlKind::NavalAcademy ? kTacticalLibraryAcademyScrollPad : 0.0;
    host.scroller_attach_00683790(0);
    // 005160CD..00516102: the pad is narrowed to float in the argument slot, added to the x87 text
    // height, spilled as a double, then the view height is subtracted and the result narrowed.
    const double total = host.text_normalized_height_00ab6bd0() + static_cast<double>(static_cast<float>(pad));
    const float view_height = host.group_size_00aa6740(screen.scroll_view_5ac)[1];
    host.scroller_set_range_006834a0(static_cast<float>(total - static_cast<double>(view_height)));
}

void tactical_library_apply_nose_art_005161c0(std::uint32_t selector_index, TacticalLibraryNoseArtHost& host) {
    if (selector_index >= host.nose_art_count_448()) {                   // 005161C7..005161F4
        host.invalid_parameter_00bf6713();
        // Not a deviation (cc13-D3): the listing falls through after the call, but
        // 00BF6713 -> 00BF66EF _invalid_parameter never returns in this image. Its
        // handler [0109DD64] has two writers: 00BF65B1 from __init_pointers (00BFBE10),
        // which stores the encoded null (00C04FD5), and 00BF66D0 inside the setter
        // _set_invalid_parameter_handler (00BF66B7), which is never referenced. So
        // 00BF66EF decodes 0 and takes __invoke_watson; the fall-through code is
        // unreachable.
        return;
    }
    const std::uint32_t value = host.nose_art_value_48(selector_index);
    // 00516215..00516250: row "0" of the library list sets game+734h, any other row game+730h.
    if (std::atol(host.library_selected_row_text().c_str()) == 0) {
        host.set_game_nose_art_734(value);
    } else {
        host.set_game_nose_art_730(value);
    }
    host.profile_store_nose_art_007f8690({host.game_nose_art_734(), host.game_nose_art_730(), 0});
    host.back_00515e00();
}

// ---------------------------------------------------------------------------
// Credits ReadItemStyle 0052D9C0 and Register 0052FFC0 (cc13-menus6)
// ---------------------------------------------------------------------------

void credits_read_item_style_0052d9c0(CreditsItem& item, CreditsLuaReader& reader) {
    // 0052D9CE..0052DA49: defaults (the white static 00E18E50 is built once, guard 00E18E60 bit 0).
    item.type = CreditsItemType::Person;
    item.colour = {1.0F, 1.0F, 1.0F, 1.0F};
    item.align = static_cast<std::int32_t>(CreditsAlign::Centre);
    item.offset = 0.0F;
    if (reader.has_key_00bd5eb0("Type")) {
        auto type = static_cast<std::int32_t>(item.type);
        reader.read_int_00bd6830("Type", type);
        item.type = static_cast<CreditsItemType>(type);
    }
    if (reader.has_key_00bd5eb0("Text")) reader.read_string_00bd6830("Text", item.text);
    if (reader.has_key_00bd5eb0("Offset")) reader.read_float_00bd6830("Offset", item.offset);
    if (reader.has_key_00bd5eb0("Color")) reader.read_vec4_00bd6830("Color", item.colour);
    if (reader.has_key_00bd5eb0("Align")) reader.read_int_00bd6830("Align", item.align);  // 0052DB84: [esp+10h] = item+20h
}

void credits_register_0052ffc0(CreditsLayoutState& screen, std::uintptr_t& page_0c, float& base_speed_3c,
    CreditsRegisterHost& host) {
    host.register_front_end_screen_004f71d0();                          // 0052FFE1
    page_0c = host.load_page_00aa5840(kCreditsPage);                     // 0052FFEA..00530036
    screen.category_template_10 = host.find_child_00aa7e00(page_0c, "category_template_Text");
    host.widget_show_34(screen.category_template_10, false);
    screen.person_template_14 = host.find_child_00aa7e00(page_0c, "person_template_Text");
    host.widget_show_34(screen.person_template_14, false);
    screen.frame_18 = host.find_child_00aa7e00(page_0c, "clipbox_Clipbox");            // 005301B1
    screen.moving_group_1c = host.find_child_00aa7e00(screen.frame_18, "moving_Group"); // 0053020D..00530227
    // 00530250..0053036B: Credits.lua's global table Credits.
    CreditsLuaReader& reader = host.open_credits_table(kCreditsScript, "Credits");
    if (reader.has_key_00bd5eb0("Speed")) reader.read_float_00bd6830("Speed", base_speed_3c);       // +3Ch
    if (reader.has_key_00bd5eb0("StartY")) reader.read_float_00bd6830("StartY", screen.start_y_40);
    if (reader.has_key_00bd5eb0("RowSpace")) reader.read_float_00bd6830("RowSpace", screen.row_space_44);
    // 00530464..00530563: Credits_Items[1..n] while the index exists; each row is pushed (0052FF10)
    // with an empty text, then filled in place through back().
    reader.enter_key_00bd8e20("Credits_Items");
    for (std::int32_t index = 1; reader.has_index_00bd5eb0(index); ++index) {
        screen.items_4c.push_back(CreditsItem{});
        reader.enter_index_4(index);
        credits_read_item_style_0052d9c0(screen.items_4c.back(), reader);
        reader.leave_8();
    }
    reader.leave_key_00bd7a20();
    host.close_credits_table();                                          // 00530568..005305DC
}

// ---------------------------------------------------------------------------
// Session browser 00579980 (cc13-menus6): uncalled row appender (E8/E9 rel32 and absolute-dword
// scans of the image find no reference; the same scan finds BuildRows' 8 call sites).
// ---------------------------------------------------------------------------

void session_browser_append_record_rows_00579980(const SessionRowState& screen, SessionRowHost& host) {
    for (const SessionJoinRecord& record : screen.records_2a0) {        // 005799C0..0057A0AF
        const std::uintptr_t row = host.clone_subtree_00aab4c0(screen.row_template_e8, 0);  // 00579A21..00579A2C
        const std::uintptr_t server = host.find_child_00aa7e00(row, "server_Text");
        const std::uintptr_t mode = host.find_child_00aa7e00(row, "mode_Text");
        const std::uintptr_t players = host.find_child_00aa7e00(row, "players_Text");
        const std::uintptr_t connection = host.find_child_00aa7e00(row, "Connection_Group");
        host.text_set_00abaed0(server, narrow_low_bytes(record.name_08));  // 00579C16..00579C43
        host.text_set_ellipsized_00abb000(mode,                          // 00579C6E..00579CF7
            host.lobby_option_text(kLobbyOptionGameMode, record.lobby_options_4c[2]), -1.0F);
        host.text_set_00abaed0(players,                                  // 00579D36..00579DB3
            std::to_string(record.players_28) + "/" + std::to_string(record.max_players_2c));
        session_row_set_connection_icons_00578320(connection, record.connection_38, host);  // 00579E55..00579E5C
        const bool allowed = (host.content_mask_728() & record.required_mask_88) == record.required_mask_88
            && (host.content_mask_72c() & record.required_mask_8c) == record.required_mask_8c;
        const GuiColour& colour = allowed ? kSessionRowJoinable : kSessionRowLocked;  // 00579E61..0057A083
        host.widget_set_colour_50(server, colour);
        host.widget_set_colour_50(mode, colour);
        host.widget_set_colour_50(players, colour);
        host.list_append_row_00a9d750(screen.session_list_30, row);      // 0057A085..0057A093
        host.widget_set_byte_77(row, false);                             // 0057A0A8
    }
}

// ---------------------------------------------------------------------------
// Session browser RefreshRowDetails 00577650 (cc13-menus6)
// ---------------------------------------------------------------------------

namespace {

// 005779CE..00577BC9: game mode record+54h (jump table 005782E4); modes 0..3 pick by record+58h
// (table 00578304). Each case reads one 18h-byte extent of the map definition (+364h + 18h*j)
// and adds a frame offset to the icon base.
struct SessionMapExtentPick {
    std::uint16_t offset;
    std::int32_t frame_add;
};

bool session_map_extent_pick(std::uint32_t mode, std::uint32_t sub_mode, SessionMapExtentPick& pick) noexcept {
    static constexpr std::array<SessionMapExtentPick, 4> kBySubMode{{{0x364, 6}, {0x37C, 5}, {0x394, 4}, {0x3AC, 3}}};
    switch (mode) {
    case 0: case 1: case 2: case 3:
        if (sub_mode > 3) return false;
        pick = kBySubMode[sub_mode];
        return true;
    case 4: pick = {0x3F4, 1}; return true;
    case 5: pick = {0x40C, 2}; return true;
    case 6: pick = {0x3C4, 7}; return true;
    case 7: pick = {0x3DC, 0}; return true;
    default: return false;
    }
}

// 00577DC2..00577E9E: a rank byte in 1..20 shows icon frame rank-1, otherwise the text shows "-".
void session_details_rank(std::uint8_t rank, std::uintptr_t icon, std::uintptr_t text, SessionDetailsHost& host) {
    if (rank > 0 && rank <= 0x14) {
        host.widget_set_state_88(icon, static_cast<std::uint16_t>(rank - 1), 0, 1.0F);
        host.widget_show_34(icon, 1);
        host.widget_show_34(text, 0);
    } else {
        host.text_set_localised_00abbe50(text, kSessionDetailsBlank, 1);
        host.widget_show_34(icon, 0);
        host.widget_show_34(text, 1);
    }
}

}  // namespace

void session_browser_refresh_row_details_00577650(const SessionRowState& screen,
    const SessionDetailsWidgets& widgets, bool& content_allowed_264, SessionDetailsHost& host) {
    const auto& records = screen.records_2a0;
    const std::int32_t index = host.list_selected_index_00a9c920(screen.session_list_30) +
        host.list_scroll_offset_164(screen.session_list_30);
    if (records.empty() || index < 0 || static_cast<std::size_t>(index) >= records.size()) {
        // 00578199..005782CF: no record; the UV store is 00568290 inlined.
        host.text_set_localised_00abbe50(widgets.map_name_text_d0, kSessionDetailsBlank, 1);
        host.widget_set_state_88(widgets.map_icon_a0, 0, 0, 1.0F);
        host.icon_set_uv_rect_00568290(widgets.map_icon_a0, {0.0F, 0.0F, 1.0F, 1.0F});
        host.widget_show_34(widgets.tilt_icon_9c, 1);
        host.text_set_localised_00abbe50(widgets.rank_min_text_c4, kSessionDetailsBlank, 1);
        host.widget_show_34(widgets.rank_min_icon_dc, 0);
        host.widget_show_34(widgets.rank_min_text_c4, 1);
        host.text_set_localised_00abbe50(widgets.rank_max_text_cc, kSessionDetailsBlank, 1);
        host.widget_show_34(widgets.rank_max_icon_e0, 0);
        host.widget_show_34(widgets.rank_max_text_cc, 1);
        host.text_set_localised_00abbe50(widgets.ratio_text_d8, kSessionDetailsBlank, 1);
        host.widget_show_34(widgets.ratio_icon_e4, 0);
        host.widget_show_34(widgets.ratio_text_d8, host.session_client_virtual_198());
        return;
    }
    const SessionJoinRecord& record = records[static_cast<std::size_t>(index)];
    const auto map_value = record.open_slots_24;   // used here as the map option value (see the doc)
    // 005776F6..005777F3: the map's localisation key from registry option 1 (searched on a copy of
    // the inner map, read through operator[]), else FE.unknown_mapname.
    const std::string map_name = host.lobby_option_text_known_34(kLobbyOptionMap, map_value)
        ? host.lobby_option_text_34(kLobbyOptionMap, map_value) : std::string(kSessionUnknownMapName);
    // 005777F8..0057783B: the record is fetched again with a fresh selection read.
    const std::int32_t again = host.list_selected_index_00a9c920(screen.session_list_30) +
        host.list_scroll_offset_164(screen.session_list_30);
    if (again < 0 || static_cast<std::size_t>(again) >= records.size()) {
        host.invalid_parameter_00bf6713();   // 005663B0's range check
        return;
    }
    content_allowed_264 = host.content_allowed_00568490(records[static_cast<std::size_t>(again)]);
    host.text_submit_00abaed0(widgets.map_name_text_d0, map_name, 1);
    if (!host.map_icon_known_1c(kLobbyOptionMap, map_value)) {
        // 00577D66..00577DC0: no icon entry: frame 0, the whole texture, tilt icon shown.
        host.widget_set_state_88(widgets.map_icon_a0, 0, 0, 1.0F);
        host.icon_set_uv_rect_00568290(widgets.map_icon_a0, {0.0F, 0.0F, 1.0F, 1.0F});
        host.widget_show_34(widgets.tilt_icon_9c, 1);
    } else {
        std::int32_t frame = host.map_icon_index_1c(kLobbyOptionMap, map_value) * 8 - 7;   // LEA ebp*8-7
        // 005778F2..005779C9: find the map definition whose name parses to the map value.
        std::size_t found = host.map_definition_count();
        for (std::size_t i = 0; i < host.map_definition_count(); ++i) {
            if (std::atol(host.map_definition_name(i).c_str()) == map_value) {
                found = i;
                break;
            }
        }
        SessionMapExtentPick pick{};
        if (found < host.map_definition_count() &&
            session_map_extent_pick(static_cast<std::uint32_t>(record.lobby_options_4c[2]),
                static_cast<std::uint32_t>(record.lobby_options_4c[3]), pick)) {
            frame += pick.frame_add;
            // [0] = x0, [1] = z0, [2] = x1, [3] = z1.
            const auto e = host.map_definition_extent(found, pick.offset);
            auto uv_of = [](float world) {
                return static_cast<float>((static_cast<double>(world) - kSessionMapWorldOrigin) / kSessionMapWorldSize);
            };
            // 00577BE3..00577C72: u from x, v flipped from z; 00577C76..00577CB4: width / height.
            const float v0 = uv_of(e[1]);
            const float v1 = uv_of(e[3]);
            host.icon_set_uv_rect_00568290(widgets.map_icon_a0, {uv_of(e[0]),
                static_cast<float>(1.0 - static_cast<double>(v0)), uv_of(e[2]),
                static_cast<float>(1.0 - static_cast<double>(v1))});
            const float width = std::fabs(static_cast<float>(static_cast<double>(e[0]) - e[2]));
            const float height = std::fabs(static_cast<float>(static_cast<double>(e[1]) - e[3]));
            const float aspect = static_cast<float>(static_cast<double>(width) / height);
            // 00577CBD..00577D3A: fit the icon into a 150-pixel box of the 960x720 screen.
            std::array<float, 2> size{kSessionMapIconWidth, kSessionMapIconHeight};
            if (1.0F > aspect) {
                size[0] = static_cast<float>(static_cast<double>(aspect) * kSessionMapIconPixels / kSessionScreenWidth);
            } else if (aspect > 1.0F) {
                size[1] = static_cast<float>(kSessionMapIconPixels / static_cast<double>(aspect) / kSessionScreenHeight);
            }
            host.widget_set_size_58(widgets.map_icon_a0, size);
        }
        // An unknown mode leaves the extent registers uninitialised in the image (see the doc);
        // the reconstruction skips the UV and size update there.
        host.widget_show_34(widgets.tilt_icon_9c, 0);                    // 00577D3C
        host.widget_set_state_88(widgets.map_icon_a0, frame, 0, 1.0F);
    }
    session_details_rank(record.rank_min_48, widgets.rank_min_icon_dc, widgets.rank_min_text_c4, host);
    session_details_rank(record.rank_max_49, widgets.rank_max_icon_e0, widgets.rank_max_text_cc, host);
    // 00577EA0..0057811E: the +94h/+90h ratio (unsigned FILD fix-ups, x87 at 53-bit precision).
    if (record.ratio_den_90 > host.ratio_minimum_b8()) {
        host.widget_show_34(widgets.ratio_icon_e4, 0);
        const double ratio = static_cast<double>(record.ratio_num_94) / static_cast<double>(record.ratio_den_90);
        const auto percent = static_cast<std::int32_t>(ratio * 100.0);   // _ftol2, truncation
        host.text_submit_00abaed0(widgets.ratio_text_d8, std::to_string(percent) + std::string(kSessionRatioSuffix), 0);
        host.widget_show_34(widgets.ratio_text_d8, host.session_client_virtual_198());
        // FCOMI ratio, registry+BCh; JBE keeps the joinable colour.
        host.widget_set_colour_50(widgets.ratio_text_d8,
            ratio > static_cast<double>(host.ratio_maximum_bc()) ? kSessionRowLocked : kSessionRowJoinable);
    } else {
        host.text_set_localised_00abbe50(widgets.ratio_text_d8, kSessionDetailsBlank, 1);
        host.widget_show_34(widgets.ratio_icon_e4, 0);
        host.widget_show_34(widgets.ratio_text_d8, host.session_client_virtual_198());
        host.widget_set_colour_50(widgets.ratio_text_d8, kSessionRowJoinable);
    }
}

// ---------------------------------------------------------------------------
// Menu prompt Exit 00530970, HUD formation Enter 0053A4D0 / Exit 00536800 (cc13-menus6)
// ---------------------------------------------------------------------------

void menu_prompt_exit_00530970(MenuPromptExitState& screen, MenuPromptExitHost& host) {
    host.widget_show_34(screen.panel_18, false);                         // 00530974..0053098A
    host.widget_show_34(screen.panel_24, false);
    // 0053098C..005309CD: any of the four managers with byte +3Ch set raises the command bar's +4.
    bool any = false;
    for (const auto which : {InterfaceManagerGlobal::FrontEnd, InterfaceManagerGlobal::Interface,
             InterfaceManagerGlobal::Manager198b8, InterfaceManagerGlobal::Title}) {
        if (host.manager_present(which) && host.manager_byte_3c(which)) {
            any = true;
            break;
        }
    }
    if (any && host.command_bar_present_e1930c()) host.set_command_bar_byte_4();
    screen.flag_288 = false;                                             // 005309D1..005309D7
    screen.selection_284 = -1;
}

void hud_formation_enter_0053a4d0(HudFormationState& screen, HudFormationHost& host) {
    if (host.group_owner_present_e188d8()) {
        screen.group_138 = host.group_owner_284();                       // 0053A501..0053A509
        if (screen.group_138 == 0) {
            host.collapse_overlays_0068ab80(1, 1);                       // 0053A511..0053A532
            return;
        }
        // 0053A533..0053A5E7: the leader's world x/z place the two markers (y from +18Ch/+188h).
        const std::uintptr_t leader = host.group_leader_14(screen.group_138);
        if (!host.unit_pose_fresh_c8(leader)) host.unit_refresh_pose_00414db0(leader);
        screen.leader_x_17c = host.unit_matrix_x_fc(leader);
        if (!host.unit_pose_fresh_c8(leader)) host.unit_refresh_pose_00414db0(leader);
        screen.leader_z_180 = host.unit_matrix_z_104(leader);
        host.widget_virtual_30(screen.marker_178, {screen.leader_x_17c, screen.marker_y_18c, screen.leader_z_180});
        host.widget_virtual_30(screen.marker_174, {screen.leader_x_17c, screen.marker_y_188, screen.leader_z_180});
        if (screen.group_138 != 0) {                                     // 0053A5E9, re-tested
            host.award_tracker_0068e420();
            host.hud_refresh_0054e610();
            host.trigger_hint_00690fd0(kFormationHint, 0);               // 0053A612..0053A636
            const std::int32_t count = host.group_member_count_4f8(screen.group_138);
            for (std::int32_t i = 0; i < count; ++i) {                   // 0053A680..0053A89B
                const std::uintptr_t member = host.group_member_at_0070d060(screen.group_138, i);
                if (member == 0) continue;
                host.register_observer_pair_00694a60(member);
                host.add_member_marker_00538d60(0x110, member, 1.0F);
                host.add_member_marker_00538d60(0x11C, member, 1.0F);
                const std::uintptr_t row = host.clone_subtree_00aab4c0(screen.row_template_b0, 0);
                const std::uintptr_t icon = host.find_child_00aa7e00(row, "icon_Icon");
                host.widget_virtual_88(icon, host.icon_frame_00653200(host.unit_icon_key_538(member)), 0, 1.0F);
                host.widget_show_34(icon, true);
                host.list_append_row_00a9d750(screen.member_list_ac, row);
                if (member == leader) host.list_select_row_00a9c740(screen.member_list_ac, icon);  // the icon, as pushed
                // 0053A781..0053A885: one curve widget per member under +98h, kept in +128h.
                const std::uintptr_t link = host.create_widget_00aa6640(kFormationLinkWidgetType, "");
                host.widget_set_local_z_00aa7d40(link, kFormationLinkDepth);
                host.widget_show_34(link, true);
                host.curve_set_texture_00abdd80(link, host.link_texture_e08694());
                host.widget_set_colour_50(link, kFormationLinkColour);
                host.append_child_00aaa5a0(screen.link_parent_98, link);
                screen.links_12c.push_back(link);                        // 005389D0
            }
        }
    }
    // 0053A8A1..0053A8C7: reached with whatever +138h holds when *(00E188D8) is null.
    screen.leader_value_13c = host.unit_738_virtual_50(host.group_leader_14(screen.group_138));
    screen.flag_7c = false;
    screen.flag_7d = false;
    host.screen_refresh_00533a30();
}

void hud_formation_exit_00536800(HudFormationState& screen, HudFormationHost& host) {
    host.hud_refresh_0054e610();                                         // 00536809..0053681A
    host.screen_refresh_00533a30();
    // 00536826..00536914: +110h markers: unregister the screen's observer pair from a marker's
    // registrar (+4), delete the marker (virtual +0), then the tree is cleared.
    for (const std::uintptr_t marker : screen.markers_110) {
        if (host.marker_registrar_4(marker) != 0) host.unregister_pair_006952a0(host.marker_registrar_4(marker));  // +4 read unguarded
        if (marker != 0) host.delete_virtual_0(marker);
    }
    screen.markers_110.clear();
    for (const std::uintptr_t marker : screen.markers_11c) {             // 00536917..005369C3
        if (marker != 0) host.delete_virtual_0(marker);
    }
    screen.markers_11c.clear();
    host.list_clear_rows_00a9bec0(screen.member_list_ac);                // 005369C6..005369CC
    for (std::uintptr_t& link : screen.links_12c) {                      // 005369D1..00536A26
        if (link != 0) {
            host.delete_virtual_4(link);
            link = 0;
        }
    }
    screen.links_12c.clear();                                            // 00536A28..00536A73 (erase all)
}

// ---------------------------------------------------------------------------
// HUD formation Update 0053A8E0 (cc13-hud2). Evidence: the listing 0053A8E0..0053B9CA, read whole;
// every x87 chain below follows the listing (double intermediates, float at each FSTP).
// ---------------------------------------------------------------------------

namespace {

// The sign-bit abs the update open-codes: FCOMIP against 0, then MOVSS or -0.0f (00D7A208) SUBSS.
float formation_abs(float v) noexcept { return v > 0.0F ? v : -0.0F - v; }

FormationMatrix formation_marker_scale() noexcept {
    // 0053B06F..0053B11D and 0053B20D..0053B2D7: diag(0.5, 0.5, 0.5, 1).
    FormationMatrix m{};
    m[0] = m[5] = m[10] = kFormationMarkerScale;
    m[15] = 1.0F;
    return m;
}

// Map space from a record's negated slot offset (0053AB09..0053AB58, 0053ACE1..0053AD2A).
FormationVec2 formation_slot_map(float r10, float r20, float extent_x, float extent_z) noexcept {
    const float nx = -r10;                                               // FCHS, FSTP
    const float nz = -r20;
    const float u = static_cast<float>(static_cast<double>(nx) / extent_x + 0.5);
    const float v = static_cast<float>(static_cast<double>(-nz) / extent_z + 0.5);
    return {u, v};
}

}  // namespace

void hud_formation_update_0053a8e0(HudFormationState& screen, HudFormationUpdateHost& host, float dt) {
    screen.moved_this_frame_95 = false;                                  // 0053A905
    if (!screen.active_4) return;                                        // 0053A8FF..0053A90C
    if (host.action_pressed_004c43c0(kFormationActionMenu) && !host.game_byte_61f()) {
        host.open_menu_00534c00();                                       // 0053A912..0053A933
    }
    if (host.active_hint_empty_0068e5b0()) host.trigger_hint_00690fd0(kFormationMapHint, 0);  // 0053A938..0053A999
    if (!host.group_owner_present_e188d8()) return;                      // 0053A9C8..0053A9E1
    const std::uintptr_t group = host.group_owner_284();                 // local [S+34h]
    if (group == 0) return;

    // 0053A9E7..0053AA55: the stick is "moving" when |b + a| exceeds 1e-4.
    const float axis_a = host.input_axis_2994();
    const float axis_b = host.input_axis_29c4();
    const float sum = static_cast<float>(static_cast<double>(axis_b) + axis_a);
    bool moving = static_cast<double>(formation_abs(sum)) > kFormationAxisDeadZone;
    // 0053AA55..0053AAA1: reread and scale by 0.02 (a*c, then c*b).
    float step_x = static_cast<float>(static_cast<double>(host.input_axis_2994()) * kFormationCursorStep);
    float step_y = static_cast<float>(kFormationCursorStep * host.input_axis_29c4());
    host.unit_order_icon_00534870();                                     // 0053AAA5

    if (moving) {                                                        // 0053AAAA..0053AC1C
        screen.flag_7c = true;
        screen.flag_7d = true;
        const FormationRecordView record = host.group_record_0070d070(screen.group_138, screen.selected_8c);
        const std::uintptr_t leader = host.group_leader_14(screen.group_138);
        if (!host.unit_pose_fresh_c8(leader)) host.unit_refresh_pose_00414db0(leader);
        if (record.unit_0 == leader) {                                   // 0053AAE5..0053AB06
            step_x = 0.0F;
            step_y = 0.0F;
            moving = false;
            screen.flag_7c = false;
            screen.flag_7d = false;
        }
        const FormationVec2 slot = formation_slot_map(record.r10, record.r20, screen.map_extent_x_30,
            screen.map_extent_z_34);
        if (host.vec2_equal_00533d30(screen.last_slot_84, slot)) {       // 0053AB5C..0053AB88
            screen.cursor_speed_80 = static_cast<float>(static_cast<double>(dt) * kFormationCursorAccel +
                screen.cursor_speed_80);
        } else {
            screen.cursor_speed_80 = 1.0F;
        }
        if (screen.cursor_speed_80 > kFormationCursorSpeedCap) screen.cursor_speed_80 = kFormationCursorSpeedCap;
        screen.last_slot_84 = slot;                                      // 0053ABAD..0053ABBD
        const float k = screen.cursor_speed_80;
        const float kx = static_cast<float>(static_cast<double>(k) * step_x);
        const float ky = static_cast<float>(static_cast<double>(k) * step_y);
        host.gui_set_cursor_00aa1fa0(static_cast<float>(static_cast<double>(kx) + slot[0]),
            static_cast<float>(static_cast<double>(ky) + slot[1]));      // 0053ABEB..0053AC17
    } else {                                                             // 0053AC1E..0053AC34
        if (screen.flag_7d) {
            screen.flag_7c = false;
            screen.flag_7d = false;
        }
        screen.cursor_speed_80 = 1.0F;
    }

    // 0053AC3C..0053AD34: the promote action orders the leader to Leave and then follow the selected
    // member (the two 0077D600 calls of this function), then puts the cursor on the old leader's slot.
    if (screen.selected_8c >= 0) {
        const std::uintptr_t member = host.group_member_at_0070d060(group, screen.selected_8c);
        if (host.action_pressed_004c43c0(kFormationActionPromote)) {
            const std::uintptr_t leader = host.group_leader_14(screen.group_138);
            if (member != leader) {
                host.entity_issue_command_0077d600(leader, kCommandLeave,
                    host.command_target_from_entity_00465080(member, 0.0F), 1);   // 0053AC97
                host.entity_issue_command_0077d600(leader, kCommandFollow,
                    host.command_target_from_entity_00465080(member, 0.0F), 1);   // 0053ACB9
                static_cast<void>(host.list_selected_index_00a9c920(screen.member_list_ac));  // result unused
                const FormationRecordView record = host.group_record_0070d070(screen.group_138,
                    host.group_index_of_member_0070d030(screen.group_138, leader));
                const FormationVec2 slot = formation_slot_map(record.r10, record.r20,
                    screen.map_extent_x_30, screen.map_extent_z_34);
                host.gui_set_cursor_00aa1fa0(slot[0], slot[1]);
            }
        }
    }

    // 0053AD3D..0053AF53: dragging the selected slot to the cursor.
    if (host.action_pressed_004c43c0(kFormationActionDrag)) {
        host.drag_pressed_00535f50();
    } else if (screen.flag_7c || moving) {
        if (host.action_released_00535ee0(kFormationActionDrag)) {
            screen.flag_7c = false;
        } else {
            const FormationVec2 cursor = host.gui_cursor_5c_60();
            const float x = static_cast<float>((static_cast<double>(cursor[0]) - 0.5) * screen.map_extent_x_30);
            const float y = static_cast<float>((static_cast<double>(cursor[1]) - 0.5) *
                -static_cast<double>(screen.map_extent_z_34));
            bool blocked = false;
            const std::int32_t count = host.group_member_count_4f8(screen.group_138);
            for (std::int32_t i = 0; i < count && !blocked; ++i) {      // 0053ADF0..0053AF1A
                if (i == screen.selected_8c) continue;
                const FormationRecordView other = host.group_record_0070d070(screen.group_138, i);
                const float dx = static_cast<float>((0.0 - x) - other.r10);
                const bool near_x = kFormationSlotGapX > formation_abs(dx);
                const float dz = static_cast<float>((0.0 - y) - other.r20);
                const bool near_z = kFormationSlotGapZ > formation_abs(dz);
                if (near_x && near_z) {                                  // 0053AEBE..0053AEC4 -> 0053AF51
                    blocked = true;
                    break;
                }
                // 0053AECA..0053AF09: the radius test sits inside the loop and does not depend on i.
                const float yy = static_cast<float>(static_cast<double>(y) * y);
                const float xx = static_cast<float>(static_cast<double>(x) * x);
                const float sq = static_cast<float>(static_cast<double>(yy) + xx);
                const float dist = static_cast<float>(std::sqrt(static_cast<double>(sq)));  // 00BF7030
                if (dist > screen.drag_radius_90) blocked = true;
            }
            if (!blocked) {                                              // 0053AF20..0053AF48
                host.group_record_write_slot(screen.group_138, screen.selected_8c, -0.0F - x, -0.0F - y);
                screen.moved_this_frame_95 = true;
                screen.dirty_94 = true;
            }
        }
    }

    // 0053AF55..0053AFAF: once a drag frame passes without a move, replicate the formation.
    if (screen.dirty_94 && !screen.moved_this_frame_95) {
        if (screen.group_138 != 0) {
            host.replicate_formation_0077a080_0077c880(screen.group_138, host.group_leader_14(screen.group_138));
        }
        screen.dirty_94 = false;
    }

    // 0053AFB6..0053B870: per member, place the unit marker (+11Ch map) and the slot marker (+110h map),
    // colour both, and draw the member->slot curve.
    const std::uintptr_t leader = host.group_leader_14(group);
    if (!host.unit_pose_fresh_c8(leader)) host.unit_refresh_pose_00414db0(leader);
    const FormationMatrix leader_matrix = host.unit_matrix_cc(leader);   // [S+14h] = leader+CCh
    const std::int32_t count = host.group_member_count_4f8(group);
    const std::int32_t leader_index = host.group_index_of_member_0070d030(screen.group_138,
        host.group_leader_14(screen.group_138));                         // 0053AFC9..0053AFEE
    std::int32_t row = -1;
    for (std::int32_t i = 0; i < count; ++i) {
        const std::uintptr_t member = host.group_member_at_0070d060(group, i);
        if (member == 0) continue;
        ++row;
        const std::uintptr_t unit_marker = host.map_subscript_00538910(0x11C, member);
        const std::uintptr_t slot_marker = host.map_subscript_00538910(0x110, member);
        if (!host.unit_pose_fresh_c8(member)) host.unit_refresh_pose_00414db0(member);
        FormationMatrix world = host.matrix_multiply_00413920(host.unit_matrix_cc(member), formation_marker_scale());
        if (!host.unit_pose_fresh_c8(member)) host.unit_refresh_pose_00414db0(member);
        const FormationMatrix member_matrix = host.unit_matrix_cc(member);
        world[12] = member_matrix[12];                                   // member+FCh
        world[13] = static_cast<float>(static_cast<double>(screen.marker_height_190) - kFormationUnitMarkerDrop);
        world[14] = member_matrix[14];                                   // member+104h
        host.marker_node_set_transform_34(unit_marker, world);           // 0053B1AE..0053B1C1
        host.marker_node_visibility_00b6da70(slot_marker, member == leader ? 0.0F : 1.0F, false);

        const FormationRecordView record = host.group_member_record_0070d080(group, member);
        const FormationVec3 offset{-0.0F - record.r10, 0.0F, -0.0F - record.r20};   // [S+B0h..B8h]
        FormationMatrix slot = host.matrix_multiply_00413920(leader_matrix, formation_marker_scale());
        const FormationVec3 slot_point = host.transform_project_point_00439820(offset, leader_matrix);
        slot[12] = slot_point[0];
        slot[13] = screen.marker_height_190;                             // 0053B355..0053B35D overwrite y
        slot[14] = slot_point[2];
        host.marker_node_set_transform_34(slot_marker, slot);            // 0053B366..0053B379

        // 0053B37B..0053B48F: colours from the config table at 00432650()+108h.
        const bool team0 = host.local_team_28() == 0;
        GuiColour slot_colour{};
        GuiColour unit_colour{};
        if (screen.selected_8c == i && leader_index != i) {
            slot_colour = host.formation_colour(0x50);
            unit_colour = host.formation_colour(team0 ? 0x00 : 0x20);
        } else {
            const std::uint16_t offset_colour = team0 ? (leader_index == i ? 0x10 : 0x00)
                                                      : (leader_index == i ? 0x30 : 0x20);
            slot_colour = host.formation_colour(offset_colour);
            unit_colour = slot_colour;
        }
        host.marker_write_diffuse_00b179f0(unit_marker, unit_colour);   // 0053B496..0053B4B6
        host.marker_write_diffuse_00b179f0(slot_marker, slot_colour);   // 0053B4B9..0053B4E6

        const std::uintptr_t link = row < static_cast<std::int32_t>(screen.links_12c.size())
            ? screen.links_12c[static_cast<std::size_t>(row)] : 0;       // bounds-checked in the image (00BF6713)
        if (member == leader) {                                          // 0053B82B..0053B85F
            host.widget_show_34(link, false);
            continue;
        }
        host.curve_clear_00abc920(link);                                 // 0053B521
        if (!host.unit_pose_fresh_c8(member)) host.unit_refresh_pose_00414db0(member);
        const FormationVec3 local = host.transform_affine_point_004142e0(host.unit_position_fc(member),
            host.pose_derived_inverse_00414e10(leader));                 // 0053B53F..0053B556
        // 0053B55B..0053B5BC.
        const float u1 = static_cast<float>(static_cast<double>(local[0]) / screen.map_extent_x_30 + 0.5);
        const float v1 = static_cast<float>(-static_cast<double>(local[2]) / screen.map_extent_z_34 + 0.5);
        const float u2 = static_cast<float>(static_cast<double>(offset[0]) / screen.map_extent_x_30 + 0.5);
        const float v2 = static_cast<float>(-static_cast<double>(offset[2]) / screen.map_extent_z_34 + 0.5);
        GuiColour colour = host.widget_get_colour_54(link);              // [S+DCh..E8h]
        host.widget_show_34(link, true);
        host.curve_add_point_00abc970(link, {u1, v1}, kFormationLinkWidth);
        host.curve_add_point_00abc970(link, {u2, v2}, kFormationLinkWidth);
        // 0053B673..0053B731: fade the curve out when both ends nearly coincide.
        const float adv = formation_abs(static_cast<float>(static_cast<double>(v1) - v2));
        const float adu = formation_abs(static_cast<float>(static_cast<double>(u2) - u1));
        const float span = adu > adv ? adu : adv;
        const float target = kFormationLinkFadeLength > static_cast<double>(span) ? 0.0F : kFormationLinkAlphaOn;
        colour[3] = host.step_towards_0042ac60(colour[3], target, dt);
        const auto& rgb = (screen.flag_7c && screen.selected_8c == i) ? kFormationLinkSelectedRgb : kFormationLinkRgb;
        host.widget_set_colour_50(link, {rgb[0], rgb[1], rgb[2], colour[3]});   // 0053B736..0053B804
        host.curve_rebuild_00abca10(link);                               // 0053B824
    }

    // 0053B876..0053B8E6: a top-down camera over the leader, up = the leader's third matrix row.
    const FormationVec3 eye{leader_matrix[12], kFormationCameraHeight, leader_matrix[14]};
    const FormationVec3 target{leader_matrix[12], 0.0F, leader_matrix[14]};
    const FormationVec3 up{leader_matrix[8], leader_matrix[9], leader_matrix[10]};
    host.camera_set_view_00b71490(screen.camera_10, host.build_look_at_00b63f10(eye, target, up));

    // 0053B8EB..0053B999.
    if (host.action_pressed_004c43c0(kFormationActionPrev) && screen.mode_action_170 == kFormationModeIdle) {
        screen.pending_16c = 0;
        host.select_previous_00533920();
        screen.mode_action_170 = kFormationActionPrev;
        screen.hold_154 = 0.0F;
        screen.hold_flag_158 = true;
        screen.hold_15c = 0.0F;
        screen.hold_flag_160 = true;
    }
    if (host.action_pressed_004c43c0(kFormationActionNext) && screen.mode_action_170 == kFormationModeIdle) {
        screen.pending_16c = 0;
        host.select_next_00533830();
        screen.mode_action_170 = kFormationActionNext;
        screen.hold_154 = 0.0F;
        screen.hold_flag_158 = true;
        screen.hold_15c = 0.0F;
        screen.hold_flag_160 = true;
    }
    host.update_order_labels_00536a90(dt);                               // 0053B9A0..0053B9AD
}

void frame_stats_register_0053c740(FrameStatsState& screen, FrameStatsRegisterHost& host) {
    host.register_front_end_screen_004f71d0();                          // 0053C763
    screen.page_0c = host.load_page_00aa5840("_Fps");                    // 0053C768..0053C7B5
    static constexpr std::array<std::array<std::string_view, 3>, 6> kGroups{{
        {"PG_1p_Text", "PG_1_Text", "PG_1s_Text"}, {"PG_2p_Text", "PG_2_Text", "PG_2s_Text"},
        {"PG_3p_Text", "PG_3_Text", "PG_3s_Text"}, {"PG_4p_Text", "PG_4_Text", "PG_4s_Text"},
        {"PG_5p_Text", "PG_5_Text", "PG_5s_Text"}, {"PG_6p_Text", "PG_6_Text", "PG_6s_Text"}}};
    for (std::size_t g = 0; g < kGroups.size(); ++g) {                   // 0053C7E0..0053CF98, +20h..+64h
        for (std::size_t k = 0; k < 3; ++k) screen.groups_20[g][k] = host.find_child_00aa7e00(screen.page_0c, kGroups[g][k]);
    }
    screen.fps_10 = host.find_child_00aa7e00(screen.page_0c, "Fps_Text");           // 0053D008
    screen.min_fps_18 = host.find_child_00aa7e00(screen.page_0c, "MinFps_Text");    // 0053D078
    screen.max_fps_1c = host.find_child_00aa7e00(screen.page_0c, "MaxFps_Text");    // 0053D0E8
    screen.warning_14 = host.find_child_00aa7e00(screen.page_0c, "Warning_Text");   // 0053D158
    // 0053D180..0053D1D9: Fingerprint_Text is looked up and given the payload, not stored.
    const std::string payload = host.fingerprint_payload_text();         // read before the lookup
    host.text_set_localised_00abbe50(host.find_child_00aa7e00(screen.page_0c, "Fingerprint_Text"), payload, 1);
}
}  // namespace bsp
