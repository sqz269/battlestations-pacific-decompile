#include "bsp/main_menu_screen.hpp"

#include <cmath>

namespace bsp {
namespace {

// The five mission-list pages, in the order 0059A28D..0059A2B4 tests them, with
// the group each selects at 0059A4B4..0059A511.
struct MissionListPage {
    int page;
    MissionGroup group;
};

constexpr MissionListPage kMissionListPages[] = {
    {0x04, MissionGroup::Ijn},       // falls through every compare, EBP stays 0
    {0x05, MissionGroup::Usn},       // LEA EBP,[EAX-4] at 0059A4D2
    {0x08, MissionGroup::Training},  // LEA EBP,[EAX-6] at 0059A4E2
    {0x06, MissionGroup::IjnDlc},    // LEA EBP,[EAX-3] at 0059A4F2
    {0x07, MissionGroup::UsnDlc},    // LEA EBP,[EAX-3] at 0059A50E
};

// 004D92B0's sound groups. Action 4Bh alone reaches slot +1Bh.
constexpr int kSelectSoundActions[] = {0x4A, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55};
constexpr int kNavigateSoundActions[] = {0x46, 0x47, 0x4C, 0x4D, 0x57, 0x58};

bool contains(const int* first, const int* last, int value) noexcept {
    for (; first != last; ++first) {
        if (*first == value) {
            return true;
        }
    }
    return false;
}

}  // namespace

const MainMenuScreenVirtual* main_menu_screen_virtual(FrontEndScreenSlot slot) noexcept {
    for (const MainMenuScreenVirtual& entry : kMainMenuScreenVirtuals) {
        if (entry.slot == slot) {
            return &entry;
        }
    }
    return nullptr;
}

const MissionGroupBinding& mission_group_binding(MissionGroup group) noexcept {
    for (const MissionGroupBinding& binding : kMissionGroupBindings) {
        if (binding.group == group) {
            return binding;
        }
    }
    return kMissionGroupBindings[0];
}

bool is_mission_list_page_00599db0(MainMenuPage page) noexcept {
    const int value = static_cast<int>(page);
    for (const MissionListPage& entry : kMissionListPages) {
        if (entry.page == value) {
            return true;
        }
    }
    return false;
}

MissionGroup mission_group_for_page(MainMenuPage page) noexcept {
    const int value = static_cast<int>(page);
    for (const MissionListPage& entry : kMissionListPages) {
        if (entry.page == value) {
            return entry.group;
        }
    }
    // 0059A4C5 clears EBP before the compare chain, so an unmatched page keeps
    // index 0 and the JP group at +314h that 0059A4B4 already stored.
    return MissionGroup::Ijn;
}

MissionGroup mission_group_for_detail(bool us_campaign, bool dlc_campaign) noexcept {
    // 0059A31C tests the DLC byte first, 0059A32A and 0059A356 the US byte.
    // Training is unreachable through this path.
    if (dlc_campaign) {
        return us_campaign ? MissionGroup::UsnDlc : MissionGroup::IjnDlc;
    }
    return us_campaign ? MissionGroup::Usn : MissionGroup::Ijn;
}

FrontEndActionSound front_end_action_sound_004d92b0(int action) noexcept {
    if (action == kFrontEndActionBack) {
        return FrontEndActionSound::Back;
    }
    if (contains(kSelectSoundActions,
                 kSelectSoundActions + (sizeof(kSelectSoundActions) / sizeof(int)), action)) {
        return FrontEndActionSound::Select;
    }
    if (contains(kNavigateSoundActions,
                 kNavigateSoundActions + (sizeof(kNavigateSoundActions) / sizeof(int)), action)) {
        return FrontEndActionSound::Navigate;
    }
    return FrontEndActionSound::None;
}

bool front_end_action_fired_004d92b0(const InputActionRecord& record,
                                     float front_end_clock,
                                     float& deadline) noexcept {
    // 004D92C8: down now with a positive hold, and not already down last frame
    // with a positive hold.
    const bool fresh = record.current_down && record.current_hold > 0.0f &&
                       !(record.previous_down && record.previous_hold > 0.0f);
    if (fresh) {
        // 004D9331: the map slot becomes clock + 0.4.
        deadline = front_end_clock + kFrontEndActionRepeatDelay;
        return true;
    }
    // 004D9302: the repeat arm needs the record still down and the hold above
    // the floor at 00D7A218, which is 0.0f in this image.
    if (!record.current_down) {
        return false;
    }
    if (!(record.current_hold > kFrontEndActionHoldFloor)) {
        return false;
    }
    // 004D931C: strictly greater, so the deadline frame itself does not fire.
    if (!(front_end_clock > deadline)) {
        return false;
    }
    deadline = front_end_clock + kFrontEndActionRepeatInterval;
    return true;
}

const MainMenuItemAction& main_menu_item_action(int index) noexcept {
    for (const MainMenuItemAction& action : kMainMenuItemActions) {
        if (action.index == index) {
            return action;
        }
    }
    return kMainMenuItemActions[0];
}

int top_level_selection_00584ae0(bool select_first, int cached_selection) noexcept {
    // 00584C2F CMP byte [ESP+54h],0 / JZ 00584C40: a zero argument restores the
    // cached index, a non-zero one pushes 0.
    return select_first ? 0 : cached_selection;
}

float map_zoom_delta_0059a517(const MapZoomInput& input, bool pad_axis_override_off) noexcept {
    float zoom_out = input.zoom_out_hold;  // records+3654h
    float zoom_in = input.zoom_in_hold;    // records+3684h
    if (!pad_axis_override_off && input.pad_axis_valid) {
        if (input.pad_axis < 0.0f) {
            // 0059A59E FABS: a negative axis replaces the zoom-out hold.
            zoom_out = std::fabs(input.pad_axis);
        } else if (input.pad_axis > kFrontEndActionHoldFloor) {
            // 0059A5B6 COMISS against 00D7A218, then JBE: strictly greater.
            zoom_in = input.pad_axis;
        }
    }
    // 0059A5C5 FLD [ESP+18h] / FSUB [ESP+20h].
    return zoom_in - zoom_out;
}

float apply_map_zoom_00588c70(float zoom, float delta) noexcept {
    // 00588CDA gates the accumulation on `this+19Ch != 0 || delta != 0`. +19Ch
    // has no writer in this packet, so only the delta term is modelled.
    if (delta == 0.0f) {
        return zoom;
    }
    float next = delta * kMapZoomGain + zoom;
    if (next < kMapZoomMin) {
        next = kMapZoomMin;
    } else if (next > kMapZoomMax) {
        next = kMapZoomMax;
    }
    return next;
}

float decay_map_offset_0059a5f2(float offset) noexcept {
    // The listing is FLD / FLD ST0 / FLDZ / FSUBRP / FMUL qword [00D7A3A0] /
    // FADDP, that is `offset + (0 - offset) * 0.1`. Written out rather than
    // folded to `offset * 0.9` so the float rounding matches the x87 form.
    return (0.0f - offset) * kMapOffsetDecay + offset;
}

int page_scene_id_0059a6bc(MainMenuPage page) noexcept {
    switch (static_cast<int>(page)) {
        case 0x01:
        case 0x02:
        case 0x03:
            return 1;  // PUSH 1 at 0059A2FF
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
            return 3;  // PUSH 3 at 0059A6BA
        case 0x09:
            return 4;  // PUSH 4 at 0059A3F5 and 0059A421
        case 0x0C:
            return 5;  // PUSH 5 at 0059A449, 0059A468 and 0059A4AD
        default:
            return 0;  // page 0Bh and every unmatched page skip the epilogue
    }
}

namespace {

// 0059A288..0059A2DC. Returns true when the arm reached the shared epilogue.
bool run_top_level_arm(MainMenuScreenUpdateHost& host, MainMenuPage page) {
    if (static_cast<int>(page) == 0x02 || static_cast<int>(page) == 0x03) {
        if (host.action_fired(kFrontEndActionBack)) {
            host.build_top_level_page(false);
        }
    }
    return true;
}

// 0059A30B..0059A423, the mission-detail page.
void run_mission_detail_arm(MainMenuScreenState& state,
                            MainMenuScreenUpdateHost& host,
                            float seconds) {
    const MissionGroup group = mission_group_for_detail(state.us_campaign, state.dlc_campaign);
    host.set_active_mission_group(group);
    state.map_offset_y =
        host.blend_map_offset(state.map_offset_y, kMissionDetailOffsetLow, kMissionDetailOffsetHigh);
    host.drive_map(seconds, group, kMissionDetailZoomDelta, false);
    state.map_zoom = apply_map_zoom_00588c70(state.map_zoom, kMissionDetailZoomDelta);
    if (host.action_fired(kFrontEndActionBack)) {
        host.mission_detail_back();
    } else if (host.action_fired(kFrontEndActionAccept)) {
        host.mission_detail_accept();
    }
    host.animate_detail_group();
}

// 0059A428..0059A4AD, the objectives page.
void run_objectives_arm(MainMenuScreenUpdateHost& host) {
    if (host.action_fired(kFrontEndActionBack)) {
        host.build_mission_detail_page();
        return;
    }
    if (host.action_fired(kFrontEndActionAccept)) {
        host.open_tactical_library_with_selection();
        return;
    }
    if (host.action_fired(kFrontEndActionContext)) {
        host.open_tactical_library_mode2();
    }
}

// 0059A4B4..0059A6BA, the five mission-list pages.
void run_mission_list_arm(MainMenuScreenState& state,
                          MainMenuScreenUpdateHost& host,
                          MainMenuPage page,
                          float seconds) {
    const MissionGroup group = mission_group_for_page(page);
    host.set_active_mission_group(group);

    const MapZoomInput input = host.map_zoom_input();
    const float delta = map_zoom_delta_0059a517(input, state.pad_axis_override_off);
    if (delta != 0.0f) {
        state.zoom_axis_latched = true;  // 0059A5E5, never cleared here
    }
    state.map_offset_y = decay_map_offset_0059a5f2(state.map_offset_y);

    float zoom_argument = delta;
    bool moved = true;
    if (!state.zoom_axis_latched) {
        // 0059A62D: once the axis has never moved, the map drifts back only
        // while the zoom is above 1.0.
        zoom_argument = state.map_zoom > kMapZoomIdleThreshold ? kMapZoomIdleDrift : 0.0f;
        moved = false;
    }
    host.drive_map(seconds, group, zoom_argument, moved);
    state.map_zoom = apply_map_zoom_00588c70(state.map_zoom, zoom_argument);

    if (host.action_fired(kFrontEndActionBack)) {
        host.position_backdrop();
        host.build_single_player_page();
    }
    host.animate_page_group(state.pad_axis_override_off);
}

// 0059A085..0059A280, the async-text page. Reached only when the manager global
// tests as null at 0059A028, which the arm then dereferences at 0059A0E9; the
// reconstruction keeps that shape instead of adding a guard the binary lacks.
void run_async_text_arm(MainMenuScreenUpdateHost& host) {
    if (host.async_text_busy()) {
        host.async_text_read_title(kMainMenuScreenStringB);
    }
    if (host.action_fired(kFrontEndActionBack)) {
        host.async_text_submit(kMainMenuScreenStringA, kMainMenuScreenStringB);
        host.rebuild_top_level_after_text(true);
    }
    if (host.action_fired(kFrontEndActionAccept)) {
        host.request_game_state(6);
        host.request_game_state(7);
        host.set_page(MainMenuPage::TopLevel);
    }
}

}  // namespace

MainMenuUpdateResult run_main_menu_screen_update_00599db0(MainMenuScreenState& state,
                                                          MainMenuScreenUpdateHost& host,
                                                          float seconds) {
    MainMenuUpdateResult result;

    // 00599DD2: the text sub-object at +0Ch drains its queue with the frame delta.
    if (host.text_owner_has_queue()) {
        host.dispatch_text_events(seconds);
    }

    // 00599DEE: the damaged-content sweep. The first flagged entry raises the
    // modal and breaks; the dirty flag is cleared either way.
    if (host.content_set_dirty() && host.content_manager_present()) {
        const int count = host.content_entry_count();
        for (int index = 0; index < count; ++index) {
            if (host.content_entry_damaged(index)) {
                host.show_message(kContentDamagedMessage);
                break;
            }
        }
        host.clear_content_set_dirty();
    }

    // 00599EF8: the downloaded-content notice. 00599F44 repeats the sign-in test
    // the previous compare already forced, so the second term can never fail.
    if (host.content_download_ready()) {
        if (host.content_manager_present() && host.menu_command_screen_idle() &&
            host.has_selected_user() && host.selected_sign_in_state() == kSignInStateOnline &&
            (host.selected_sign_in_state() == kSignInStateOnline ||
             host.selected_sign_in_state() == kSignInStateLocal) &&
            host.content_manager_ready()) {
            host.content_manager_commit();
            host.show_message(kContentDownloadedMessage);
        }
        host.clear_content_download_ready();
    }

    // 0059A020: the async-text manager either owns page 0Ah or polls a pending
    // request; the two are exclusive in the listing.
    if (!host.async_text_manager_present()) {
        if (static_cast<int>(host.page()) == kMainMenuPageAsyncText) {
            run_async_text_arm(host);
        }
    } else if (host.async_text_pending()) {
        if (host.async_text_busy()) {
            host.set_async_text_pending(false);
        } else if (host.async_text_status() != 4 && host.async_text_status() == 2) {
            host.set_async_text_pending(false);
        }
    }

    // 0059A283 reloads the page, so the async-text arm's write is visible here.
    const MainMenuPage page = host.page();
    const int page_value = static_cast<int>(page);

    if (page_value == 0x01) {
        result.arm = MainMenuUpdateArm::TopLevel;
        run_top_level_arm(host, page);
    } else if (is_mission_list_page_00599db0(page)) {
        result.arm = MainMenuUpdateArm::MissionList;
        run_mission_list_arm(state, host, page, seconds);
    } else if (page_value == 0x0B) {
        // 0059A2BF: the only arm that skips the epilogue after acting.
        result.arm = MainMenuUpdateArm::TacticalLibrary;
        if (host.action_fired(kFrontEndActionBack)) {
            host.build_top_level_page(false);
        }
        return result;
    } else if (page_value == 0x02 || page_value == 0x03) {
        result.arm = MainMenuUpdateArm::TopLevel;
        run_top_level_arm(host, page);
    } else if (page_value == 0x09) {
        result.arm = MainMenuUpdateArm::MissionDetail;
        run_mission_detail_arm(state, host, seconds);
    } else if (page_value == 0x0C) {
        result.arm = MainMenuUpdateArm::Objectives;
        run_objectives_arm(host);
    } else {
        return result;  // 0059A43E's default: no epilogue
    }

    // 0059A6BC: 004C1E90(scene) then 00427190 on what it returned.
    result.scene_id = page_scene_id_0059a6bc(page);
    result.reached_epilogue = true;
    host.update_scene(result.scene_id);
    return result;
}

}  // namespace bsp
