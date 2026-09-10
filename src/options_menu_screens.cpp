#include "bsp/options_menu_screens.hpp"

namespace bsp {
namespace {

// 005EF4D0's step count for every enumerated row that does not read one out of
// a runtime table. Both quality rows, the hints row and the old-film row pass a
// literal 3; the old-film row therefore never reaches its fourth string.
constexpr int kEnumeratedStepCount = 3;

// The byte table at 005F7C20, indexed by page over 0..7, selecting between the
// three arms 005F7A0D, 005F7A28 and 005F7A2F.
constexpr std::array<unsigned char, 8> kBackArm{0, 1, 1, 1, 1, 2, 2, 1};

bool is_page_with_defaults(OptionsPage page) noexcept {
    // The four pages 005F8960 hands to a defaults writer. Page 7 empties its
    // content vector instead and page 0 has no reset row at all.
    return page == OptionsPage::Game || page == OptionsPage::Audio ||
           page == OptionsPage::Video || page == OptionsPage::Control;
}

} // namespace

const OptionsScreenClass* options_screen_by_field_offset(int offset) noexcept {
    for (const OptionsScreenClass& screen : kOptionsScreens) {
        if (static_cast<int>(screen.manager_field_offset) == offset) {
            return &screen;
        }
    }
    return nullptr;
}

const OptionsScreenClass* options_screen_by_id(int screen_id) noexcept {
    for (const OptionsScreenClass& screen : kOptionsScreens) {
        if (screen.screen_id == screen_id) {
            return &screen;
        }
    }
    return nullptr;
}

const OptionsScreenClass* options_screen_by_interface(int interface_id) noexcept {
    for (const OptionsScreenClass& screen : kOptionsScreens) {
        if (screen.interface_id == interface_id) {
            return &screen;
        }
    }
    return nullptr;
}

const OptionsPageInfo* options_page_info(OptionsPage page) noexcept {
    for (const OptionsPageInfo& info : kOptionsPages) {
        if (info.page == page) {
            return &info;
        }
    }
    return nullptr;
}

const OptionsSetting* options_setting(OptionsPage page, int row) noexcept {
    for (const OptionsSetting& setting : kOptionsSettings) {
        if (setting.page == page && setting.row == row) {
            return &setting;
        }
    }
    return nullptr;
}

OptionsBackTarget options_back_target(OptionsPage page) noexcept {
    const int index = static_cast<int>(page);
    if (index < 0 || index >= static_cast<int>(kBackArm.size())) {
        return OptionsBackTarget::Ignored;
    }
    switch (kBackArm[static_cast<std::size_t>(index)]) {
    case 0:
        return OptionsBackTarget::LeaveScreen;
    case 1:
        return OptionsBackTarget::ReturnToList;
    default:
        return OptionsBackTarget::Ignored;
    }
}

bool options_adjust_005f42d0(OptionsScreenHost& host, OptionsScreenState& state,
                             int step) noexcept {
    // 005F42D5 tests the focused row for -1 before anything else, so a page
    // with no focus is left alone and stays clean.
    const int row = host.focused_row();
    if (row < 0) {
        return false;
    }
    // 005F42E4 sets the dirty flag before the switch, so a row the switch does
    // not cover still marks the page changed.
    state.value_changed = true;

    const OptionsSetting* setting = options_setting(state.page, row);
    if (setting == nullptr) {
        return false;
    }

    bool toggled = false;
    switch (setting->kind) {
    case OptionsValueKind::Toggle:
        toggled = host.toggle(setting->field_offset);
        break;
    case OptionsValueKind::Enumerated:
        // Only the language row reads its count out of the game; the rest pass
        // the literal 3.
        host.step_enumerated(setting->field_offset,
                             setting->page == OptionsPage::Game && setting->row == 0
                                 ? host.language_count()
                                 : kEnumeratedStepCount);
        break;
    case OptionsValueKind::Ranged:
        host.step_ranged(setting->field_offset, step);
        break;
    case OptionsValueKind::Indexed:
        // Page 3 row 0 is the resolution and row 2 the antialias level; both
        // step an index and then copy a record out of a runtime table.
        if (setting->row == 0) {
            host.step_resolution(step);
        } else {
            host.step_antialias(step);
        }
        break;
    }

    if (setting->timing != OptionsApplyTiming::Immediately) {
        return true;
    }

    if (setting->page == OptionsPage::Audio) {
        // Every volume row calls 008D5430; the speech and effect rows also
        // replay a sample through virtual +34h of +A4h and +A0h.
        host.apply_audio();
        if (setting->row == 2) {
            host.preview_speech();
        } else if (setting->row == 3) {
            host.preview_effects();
        }
        return true;
    }

    // The four video rows that reach the running renderer as soon as they move.
    switch (setting->row) {
    case 9:
        // 005F4844 skips the cloud calls entirely when 00E188A8+19E8h is null.
        if (host.cloud_system_present()) {
            host.apply_clouds(toggled);
        }
        break;
    case 10:
        host.apply_foliage(toggled);
        break;
    case 11:
        host.apply_motion_blur(toggled);
        break;
    case 12:
        host.apply_film_effect();
        break;
    default:
        break;
    }
    return true;
}

const OptionsMainListRow* options_activate_main_row_005f7c40(
    OptionsScreenHost& host, OptionsScreenState& state) noexcept {
    // 005F7C58 refuses to act while the screen is still sliding.
    if (state.transition_seconds != 0.0f) {
        return nullptr;
    }
    const int row = host.selected_row();
    state.selected_main_row = row;
    state.gamepad_ui = host.gamepad_ui_flag();

    if (row < 0 || row >= static_cast<int>(kOptionsMainListRowCount)) {
        return nullptr;
    }
    const OptionsMainListRow& entry = kOptionsMainList[static_cast<std::size_t>(row)];
    switch (entry.action) {
    case OptionsMainListAction::OpenPage:
        host.build_page(entry.page);
        state.page = entry.page;
        break;
    case OptionsMainListAction::PushInterface:
        // Row 4 fills the gamepad layout screen from +118h..+11Bh first.
        if (entry.sends_layout_payload) {
            host.send_layout_payload();
        }
        host.push_interface(entry.interface_id);
        break;
    case OptionsMainListAction::OpenClanText:
        // 005F7D33 and 005F7D42 both have to agree before the row does
        // anything, and the second gate also clears +25Ch.
        if (!host.clan_text_available()) {
            return nullptr;
        }
        state.gamepad_ui = false;
        host.open_clan_text();
        break;
    }
    return &entry;
}

void options_run_command_005f8960(OptionsScreenHost& host, OptionsScreenState& state,
                                  OptionsCommand command) noexcept {
    switch (command) {
    case OptionsCommand::Reset:
        // 005F8A0C onwards. Page 0 matches none of the five arms, so the reset
        // row does nothing on the main list.
        if (state.page == OptionsPage::MainList || state.page == OptionsPage::Unused5 ||
            state.page == OptionsPage::Unused6) {
            return;
        }
        state.value_changed = true;
        state.rebuild_page = true;
        if (state.page == OptionsPage::Game) {
            host.set_pending_language();
        }
        if (state.page == OptionsPage::DownloadedContent) {
            host.clear_content_selection();
        } else if (is_page_with_defaults(state.page)) {
            host.write_defaults(state.page);
        }
        host.apply_all();
        if (state.page == OptionsPage::Video && !host.multiplayer_menu_present()) {
            host.reset_device();
        }
        return;

    case OptionsCommand::ApplyAndSave:
        // 005F8C90 onwards. Only the video arm commits before prompting; the
        // page 4 and page 7 arms of this command were not read.
        if (state.page == OptionsPage::Video) {
            host.apply_all();
            if (!host.multiplayer_menu_present()) {
                host.reset_device();
            }
        }
        if (host.gamepad_present() || state.save_target_available) {
            host.raise_save_prompt();
        }
        return;

    case OptionsCommand::Cancel:
        if (state.page == OptionsPage::MainList) {
            options_leave_005f0a80(host, state);
        } else {
            options_return_to_list_005f5e60(host, state);
        }
        return;

    case OptionsCommand::Layout:
        // 005F9340 checks the page before it touches the payload.
        if (state.page == OptionsPage::Control) {
            host.send_layout_payload();
            host.push_interface(kInterfaceControlLayout);
        }
        return;
    }
}

void options_leave_005f0a80(OptionsScreenHost& host, OptionsScreenState& state) noexcept {
    // 005F0A9C clears the gamepad prompt before anything else; the empty-string
    // help-line reset at 005F0AD5 is not modelled.
    host.set_gamepad_prompt(false);
    if (!host.multiplayer_menu_present()) {
        for (const int request : kOptionsLeaveStateRequests) {
            host.request_game_state(request);
        }
    } else {
        host.notify_multiplayer_menu();
    }
    // 005F0AF6 and 005F0AFD. The page global is not touched here: 00588A80 and
    // 005D43C0 reset it to 0 before the screen is raised again.
    state.selected_main_row = 0;
    state.gamepad_ui = false;
}

bool options_return_to_list_005f5e60(OptionsScreenHost& host,
                                     OptionsScreenState& state) noexcept {
    // 005F5E7B: a changed value raises the discard prompt instead of leaving.
    if (state.value_changed) {
        host.raise_discard_prompt();
        return false;
    }
    host.apply_all();
    host.build_main_list();
    state.page = OptionsPage::MainList;
    return true;
}

void options_update_005f7310(OptionsScreenHost& host, OptionsScreenState& state,
                             float seconds) noexcept {
    // 005F7339. The two overlapped polls that follow it need the storage
    // handles and are not modelled here; see the storage follow-up packet.
    state.storage_seconds += seconds;

    // 005F7859. Cancelling an outstanding save or load wins over everything
    // else and returns immediately.
    if (host.action_fired(kOptionsActionBack) && host.gamepad_prompt_shown()) {
        host.set_gamepad_prompt(false);
        host.cancel_storage();
        state.write_pending = false;
        state.read_pending = false;
        return;
    }

    // 005F791C. While the screen is sliding it only counts the timer down.
    if (state.transition_seconds != 0.0f) {
        state.transition_seconds -= seconds;
        if (state.transition_seconds < 0.0f) {
            state.transition_seconds = 0.0f;
        }
        return;
    }

    // 005F79A5, the jump table at 005F7BF8, repaints the value column of the
    // current page through the per_frame routine of kOptionsPages. It is a pure
    // redraw and needs the GUI list, so it is not modelled here.

    // 005F79E4, the byte table at 005F7C20.
    if (host.action_fired(kOptionsActionBack)) {
        switch (options_back_target(state.page)) {
        case OptionsBackTarget::LeaveScreen:
            options_leave_005f0a80(host, state);
            return;
        case OptionsBackTarget::ReturnToList:
            options_return_to_list_005f5e60(host, state);
            break;
        case OptionsBackTarget::Ignored:
            break;
        }
    }

    // 005F7A2F. The reset button synthesises the reset command row and pushes
    // it through the same listener the row itself uses, so both paths share one
    // body. Only pages 1..4 carry the row.
    if (host.action_fired(kOptionsActionReset) && is_page_with_defaults(state.page)) {
        host.synthesise_reset_command();
    }

    // 005F7AA7. The gamepad layout shortcut, page 4 only and only with a pad.
    if (host.gamepad_present() && host.action_fired(kOptionsActionViewLayout) &&
        state.page == OptionsPage::Control) {
        host.send_layout_payload();
        host.push_interface(kInterfaceControlLayout);
    }

    // 005F7B07. The value arrows, gated on the focused row being editable.
    if (host.focused_row_editable()) {
        const bool decrease = host.action_fired(kOptionsActionDecrease);
        const bool increase = host.action_fired(kOptionsActionIncrease);
        if (decrease || increase) {
            // 005F7B3A computes the step as `(!decrease) * 2 - 1`, so a
            // decrease wins when both fire in the same frame.
            options_adjust_005f42d0(host, state, decrease ? -1 : 1);
        }
    }

    // 005F7B4D, the jump table at 005F7C28. Only pages 1..4 can rebuild; any
    // other page clears the flag instead.
    if (state.rebuild_page) {
        const bool rebuildable = state.page == OptionsPage::Game ||
                                 state.page == OptionsPage::Audio ||
                                 state.page == OptionsPage::Video ||
                                 state.page == OptionsPage::Control;
        state.rebuild_page = false;
        if (rebuildable) {
            host.build_page(state.page);
        }
    }
}

} // namespace bsp
