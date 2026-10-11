// In-game GUI pieces of segments 26 and 27. Evidence: docs/CC13_INGAME_GUI.md.

#include "bsp/ingame_gui.hpp"

#include "bsp/gui_layer.hpp"
#include "bsp/main_menu_mission_detail.hpp"
#include "bsp/profile_checkpoint.hpp"

#include <string>

namespace bsp {

const SupportManagerSound* support_manager_sound_for_player(std::uint32_t player) noexcept {
    for (const SupportManagerSound& sound : kSupportManagerSounds) {
        if (sound.player == player) return &sound;
    }
    return nullptr;
}

void play_support_manager_sound(const SupportManagerSound& sound, SupportManagerSoundHost& host) {
    // e.g. 0065F18D..0065F1DD: the function-static class index.
    if (!host.class_cached(sound.guard_global)) {
        host.cache_class(sound.guard_global, sound.class_global,
            host.find_sound_class_00a7acf0(kSupportManagerSoundClass));
    }
    const std::int64_t type = host.find_sound_type_00a7b0a0(kSupportManagerSoundType);
    void* handle = host.create_tracked_sound_00a7e490(sound.sample_field,
        host.cached_class(sound.class_global), type, true);
    if (handle != nullptr) host.release_handle(handle);
}

std::string format_marker_distance_0063e5c0(float distance, bool nautical_miles) {
    // 0063E69A..0063E6AF: UCOMISS with 0.0f; equality skips the text.
    if (distance == 0.0f) return {};
    const float tenths = static_cast<float>(static_cast<double>(distance) /
        (nautical_miles ? kDistanceTenthNauticalMile : kDistanceTenthKilometre));
    // 0063E6E7..0063E72A: CVTTSS2SI, signed /10 and %10, x87 remainder test.
    const std::int32_t whole = static_cast<std::int32_t>(tenths);
    std::int32_t tens = whole / 10;
    std::int32_t digit = whole % 10;
    if (static_cast<double>(tenths) - whole > 0.5) ++digit;
    if (digit == 10) {
        ++tens;
        digit = 0;
    }
    std::string text = std::to_string(tens);
    text += '.';
    text += std::to_string(digit);
    text += " |";
    text += nautical_miles ? kDistanceMileKey : kDistanceKilometreKey;
    return text;
}

std::int32_t device_class_icon_index_0065ea30(std::string_view name) noexcept {
    const auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c + 32) : c; };
    for (const DeviceClassIcon& entry : kDeviceClassIcons) {
        if (entry.name.size() != name.size()) continue;
        bool equal = true;
        for (std::size_t i = 0; i < name.size() && equal; ++i) equal = lower(entry.name[i]) == lower(name[i]);
        if (equal) return entry.index;
    }
    return kDeviceClassIconUnknown;  // 0065EE53..0065EE76, after the "%s" diagnostic
}

void rewards_screen_update_0062b640(RewardsScreenUpdateState& screen, RewardsScreenUpdateHost& host) {
    // 0062B646: the leaving byte goes straight to 0062B110.
    if (screen.leaving) {
        host.leave_0062b110();
        return;
    }
    // 0062B64C..0062B670: a pending storage flush counts down through zero.
    if (screen.storage_flush_countdown >= 0) {
        if (screen.storage_flush_countdown == 0) host.flush_storage_007fa1b0(screen.storage_slot);
        --screen.storage_flush_countdown;
    }
    // 0062B674..0062B6A6: FCOMI of the double 0.03 with the frame time; the
    // result replaces the seconds argument in its stack slot.
    const float frame = host.game_frame_seconds_21ec();
    const float step = frame < kRewardsMaxStep ? frame : kRewardsMaxStep;

    if (host.action_004d92b0(kRewardsActionLeave)) {
        host.leave_0062b110();
        return;
    }
    if (screen.input_locked) return;  // 0062B6C3

    // 0062B6D8..0062B70B: the action is read before the page test.
    if (host.action_004d92b0(kRewardsActionGamerCard) && screen.page == 1 &&
        host.gamer_card_available_0061d8b0()) {
        const std::uint64_t xuid = host.shown_player_xuid_0061d710();
        host.show_gamer_card_00a4d440(host.active_user_slot_00a3eac0(), xuid);
    }

    if (screen.page == 1) {
        // 0062B718..0062B783: review anyone but the local player.
        if (host.session_virtual_198() && host.action_004d92b0(kRewardsActionPlayerReview) &&
            host.local_player_xuid_00a3eb00() != host.shown_player_xuid_0061d710()) {
            const std::uint64_t xuid = host.shown_player_xuid_0061d710();
            host.show_player_review_00a4d470(host.active_user_slot_00a3eac0(), xuid);
        }
    } else if (screen.page == 4) {
        // 0062B791..0062B815: UCOMISS/LAHF/JNP, so NaN also grows. The width
        // moves toward the target by target * step * 0.25 (x87, stored as
        // float) and snaps to it, clearing the target, once it gets there.
        if (screen.rank_grow_target != 0.0f) {
            GuiSize2 size = host.rank_size_00aa6740();
            const float target = screen.rank_grow_target;
            size.x = static_cast<float>(static_cast<double>(target) * step * kRewardsRankGrowRate + size.x);
            if (size.x >= target) {
                size.x = target;
                screen.rank_grow_target = 0.0f;
            }
            host.rank_set_size_58(size);
        }
        // 0062B817..0062B8AC: the alpha bounces between 0 and 1. The direction
        // flips when the current alpha is at or beyond either end, then moves
        // by one step and is clamped through 004155B0.
        const float alpha = host.rank_alpha_54();
        if (alpha >= kRewardsRankAlphaMax || alpha <= 0.0f) screen.rank_alpha_rising = !screen.rank_alpha_rising;
        const float moved = screen.rank_alpha_rising ? alpha + step : alpha - step;
        host.rank_set_alpha_4c(clamp_pointer_004155b0(moved, {0.0f, kRewardsRankAlphaMax}));
    }

    host.scroller_update_00683820(0x68);
    host.scroller_update_00683820(0x128);
    host.scroller_update_00683820(0x19C);

    // 0062B8D2..0062B92C: page arrows flash for 0.1 s.
    bool turned = false;
    if (host.action_004d92b0(kRewardsActionPagePrevious)) {
        host.change_page_00622e30(-1);
        host.arrow_set_state_88(RewardsArrow::Previous, 2);
        turned = true;
    } else if (host.action_004d92b0(kRewardsActionPageNext)) {
        host.change_page_00622e30(1);
        host.arrow_set_state_88(RewardsArrow::Next, 2);
        turned = true;
    }
    if (turned) screen.arrow_flash = kRewardsArrowFlashSeconds;

    // 0062B931..0062B994: the countdown is stored before the <= 0 test.
    if (screen.arrow_flash != 0.0f) {
        screen.arrow_flash -= step;
        if (screen.arrow_flash <= 0.0f) {
            screen.arrow_flash = 0.0f;
            host.arrow_set_state_88(RewardsArrow::Previous, 0);
            host.arrow_set_state_88(RewardsArrow::Next, 0);
        }
    }
}

namespace {

// 0062C790..0062C812 / 0062C9A0..0062CA38: one unlock page per counted row,
// then the recording pass (skipped when the count is 0).
void rewards_queue_unlocks(RewardsEnterState& screen, RewardsEnterHost& host, std::string_view key) {
    const std::int32_t count = host.vehicle_unlocks_00622360(key, true);
    if (count == 0) return;
    for (std::int32_t i = 0; i < count; ++i) screen.pages238.push_back(RewardsAwardPage::Unlock);
    host.vehicle_unlocks_00622360(key, false);
}

} // namespace

bool rewards_screen_enter_award_queue_0062c2c0(RewardsEnterState& screen, RewardsEnterHost& host) {
    // 0062C2EF..0062C3AD
    if (host.session_present_f8a2fc() && host.session_virtual_198()) host.session_virtual_58();
    screen.field64 = 0.0f;
    screen.input_locked258 = false;
    host.widget_25c_show(false);
    screen.pages238.clear();
    host.clear_pages_248_00625a50();

    // 0062C3B2..0062C4B2: LAHF/TEST AH,44h/JP, so only an ordered 0.0 counts.
    screen.score_zero18 = host.commit_record_float_0c_004b4750() == 0.0f;
    host.mission_record_00594a70();
    host.selected_mission_005c3870();
    const bool commit_present = host.commit_record_head_nonzero_004b4750();
    const bool completed = host.mission_completed_009052d0();
    screen.mission28 = host.game_mission_2198();
    screen.mission30 = host.game_mission_2198();
    screen.commit_present19 = commit_present;
    screen.unlocks_pending24 = false;
    screen.advanced_unfinished25 = false;
    screen.advanced27 = false;
    screen.group_completed1c = false;
    screen.field1d = false;

    if (screen.skip3c) {
        host.set_frame_title_00518d60("");
        return false;
    }
    if (screen.score_zero18) {
        rewards_screen_enter_pages_0062ca7b(screen, host);
        return true;
    }

    // 0062C53F..0062C684: a rank change (007FCD90(0) between two reads).
    const std::int32_t rank_before = host.profile_counter_005097d0(kRewardsRankCounter);
    host.profile_update_007fcd90(0);
    if (rank_before != host.profile_counter_005097d0(kRewardsRankCounter)) {
        screen.pages238.push_back(RewardsAwardPage::RankChange);
    }

    if (host.game_dword(0x1FE4) == 0) {
        // 0062C69A..0062C847
        screen.unlocks_pending24 = host.profile_007f93f0();
        if (screen.unlocks_pending24) {
            for (const std::string& key : host.pending_unlock_keys_6cc()) rewards_queue_unlocks(screen, host, key);
        }
        // 0062C84C..0062C905
        host.profile_flush_pending_unlocks_007fb990();
        screen.advanced27 = host.advance_campaign_selection_005c3de0();
        if (screen.advanced27) screen.mission30 = host.profile_mission_id_00586150();
        if (screen.commit_present19) {
            screen.advanced_unfinished25 = screen.advanced27 && !completed;
            if (!completed && host.group_completed_005c3be0(screen.mission28)) screen.group_completed1c = true;
        }
    }

    // 0062C90B..0062CA76: service-rank awards in the commit record.
    for (const std::string& key : host.commit_record_keys_6c()) {
        if (host.has_prefix_00553c80(key, kRewardsServiceRankPrefix)) rewards_queue_unlocks(screen, host, key);
    }
    rewards_screen_enter_pages_0062ca7b(screen, host);
    return true;
}

std::int32_t rewards_icon_state(std::int32_t value) noexcept {
    const std::int32_t state = value - 1 > 0 ? value - 1 : 0;
    return state & 0xFFFF;
}

namespace {

// The y offset of scoreboard slot `slot`: FLD height, FADD QWORD 1/120,
// FIMUL slot, FSTP float (0062DAC1..0062DAE7 and its two copies).
float rewards_row_offset(const GuiSize2& template_size, std::int32_t slot) {
    return static_cast<float>((static_cast<double>(template_size.y) + kRewardsRowGap) * slot);
}

// x and z go through FLDZ/FADD, so they are 0.0f + the template's value.
RewardsVec3 rewards_offset_y(const RewardsVec3& origin, float dy) {
    return {0.0f + origin.x, origin.y + dy, origin.z + 0.0f};
}

// 0062D6A3..0062DB83, 0062DDD0..0062E2A9 and 0062E44B..0062E91D: one cloned
// row of the scoreboard listbox. `rank_number` is the rank_Text value,
// `slot` the layout slot, `flag_state` the flag_Icon state; returns true when
// the row is the local player's.
bool rewards_add_score_row(RewardsEnterHost& host, const RewardsScoreRow& row, std::int32_t row_index,
                           std::int32_t rank_number, std::int32_t slot, std::int32_t flag_state) {
    const RewardsWidget templ = host.screen_widget(0xD8);
    const RewardsWidget item = host.clone_00aab4c0(templ);
    host.widget_show_34(item, true);
    host.listbox_insert_row_00a9da30(item, row_index);
    host.set_row_human_flag(host.listbox_row_count_104(), !row.ai5c);

    host.text_source_00abaed0(host.child_00aa7e00(item, "rank_Text"), std::to_string(rank_number), false);
    const bool localise_name = row.ai5c || row.name == kRewardsNewPlayer;
    host.text_source_00abaed0(host.child_00aa7e00(item, "name_Text"), row.name, localise_name);
    host.text_source_00abaed0(host.child_00aa7e00(item, "clan_Text"), row.clan, true);
    host.text_source_00abaed0(host.child_00aa7e00(item, "score_Text"), std::to_string(row.score), false);
    host.icon_state_88(host.child_00aa7e00(item, "flag_Icon"), flag_state);
    host.icon_state_88(host.child_00aa7e00(item, "rank_Icon"), rewards_icon_state(row.rank));
    host.widget_show_34(host.child_00aa7e00(item, "rank_Icon"), !row.ai5c);

    const float dy = rewards_row_offset(host.widget_size_00aa6740(templ), slot);
    host.widget_set_position_00aa8240(item, rewards_offset_y(host.widget_position_00aa6750(templ), dy));

    RewardsLocalPlayer player;
    return host.local_player(player) && row.id0 == player.id80 && row.id4 == player.id84 && row.name == player.name;
}

// 0062CBE6..0062D2F6: the campaign debriefing page (+40h).
void rewards_fill_debriefing(RewardsEnterState& screen, RewardsEnterHost& host) {
    const RewardsCommitRecord& record = host.commit_record();
    const RewardsWidget page = host.screen_widget(0x40);
    const RewardsWidget clip = host.child_00aa7e00(page, "debriefing_Clipbox");
    // 0062CC42..0062CCA9: move_Group/congrat1_Text shows the debriefing text.
    host.text_source_00abaed0(host.child_00aa7e00(host.child_00aa7e00(clip, "move_Group"), "congrat1_Text"),
                              host.selected_debriefing_text_38(), true);
    // 0062CD06..0062CDDA: the scroller range is the text height less +D0h.
    const float height = host.text_height_00ab6bd0(
        host.child_00aa7e00(host.child_00aa7e00(clip, "move_Group"), "congrat1_Text"));
    host.scroller_set_range_006834a0(0x68, height - host.screen_float(0xD0));

    // 0062CDDF..0062CE8C: rank_Icon from the profile's "RANK" byte (00574050).
    host.icon_state_88(host.child_00aa7e00(page, "rank_Icon"), rewards_icon_state(host.profile_rank_byte_00574050()));

    const RewardsWidget info = host.child_00aa7e00(page, "info_Group");
    host.text_source_00abaed0(host.child_00aa7e00(info, "time_value_Text"),
                              format_play_time_0043cc20(host.round_value_004396f0(record.play_seconds)), true);
    host.text_source_00abaed0(host.child_00aa7e00(info, "score_value_Text"), std::to_string(record.score), true);

    // 0062D05C..0062D10F
    const RewardsWidget medal = host.child_00aa7e00(page, "medal_Icon");
    if (screen.commit_present19) {
        host.widget_show_34(medal, true);
        host.icon_state_88(medal, rewards_icon_state(record.medal));
    } else {
        host.widget_show_34(medal, false);
    }

    // 0062D111..0062D2F6: NEG/SBB/TEST EAX,00E18B5Ch is a non-null test of
    // +10h; with it clear the debriefing voice is requested.
    if (host.screen_dword(0x10) != 0) return;
    const std::string path = streamed_dialog_path(host.audio_language_folder_008d57a0(),
                                                  host.selected_debriefing_voice_50());
    host.request_streamed_dialog_00be7cd0(path, kRewardsDebriefVoiceCallback);
}

// 0062D304..0062E9A4: the session scoreboard page (+44h).
void rewards_fill_scoreboard(RewardsEnterState&, RewardsEnterHost& host) {
    const RewardsCommitRecord& record = host.commit_record();
    const RewardsWidget page = host.screen_widget(0x44);
    host.text_source_00abaed0(host.child_00aa7e00(page, "time_value_Text"),
                              format_play_time_0043cc20(host.round_value_004396f0(record.play_seconds)), true);

    // 0062D3BD..0062D43B
    host.clear_row_human_flags_230();
    host.listbox_clear_rows_00a9bec0();
    host.listbox_set_listener_00a9ac40(false);
    host.build_score_rows_0062a0b0();
    host.sort_score_rows_00625d10();

    std::int32_t local_row = 0;  // [esp+9Ch]
    const std::vector<RewardsScoreRow>& rows = host.score_rows_220();
    if (host.game_dword(0x614) == kRewardsGameModeNoSides) {
        // 0062D461..0062DB95: one table, every row in order.
        host.set_page_title_e196c0(1, record.won != 0 ? kRewardsMissionCompleted : kRewardsMissionFailed);
        host.text_source_00abaed0(host.child_00aa7e00(page, "END_Text"), record.end_text, true);
        host.widget_show_34(host.child_00aa7e00(page, "END_Text"), true);
        host.widget_show_34(host.child_00aa7e00(page, "USA_Text"), false);
        host.widget_show_34(host.child_00aa7e00(page, "JAP_Text"), false);
        for (std::size_t i = 0; i < rows.size(); ++i) {
            const auto index = static_cast<std::int32_t>(i);
            if (rewards_add_score_row(host, rows[i], index, index + 1, index, rows[i].side != 0 ? 1 : 0)) {
                local_row = index;
            }
        }
    } else {
        // 0062DB9A..0062DBF7: USN wins when the local side won as USN or lost as IJN.
        RewardsLocalPlayer player;
        host.local_player(player);  // indexed without the 0..7 check here
        const bool usn = record.won != 0 ? player.side == 0 : player.side == 1;
        host.set_page_title_e196c0(1, usn ? kRewardsUsnWins : kRewardsIjnWins);
        host.text_source_00abaed0(host.child_00aa7e00(page, "END_Text"), record.end_text, true);
        host.widget_show_34(host.child_00aa7e00(page, "END_Text"), true);

        // 0062DCCE..0062E2C3: USA_Text, then the side-0 rows in slots 0, 1, ...
        const RewardsWidget usa = host.child_00aa7e00(page, "USA_Text");
        host.text_source_00abaed0(usa, record.usn_text, true);
        host.widget_show_34(usa, true);
        std::int32_t slot = 0;
        for (std::size_t i = 0; i < rows.size(); ++i) {
            if (rows[i].side != 0) continue;
            const auto index = static_cast<std::int32_t>(i);
            if (rewards_add_score_row(host, rows[i], index, slot + 1, slot, 0)) local_row = index;
            ++slot;
        }

        // 0062E2C8..0062E3C7: JAP_Text six row heights below USA_Text.
        const RewardsWidget jap = host.child_00aa7e00(page, "JAP_Text");
        host.widget_show_34(jap, true);
        host.text_source_00abaed0(jap, record.ijn_text, true);
        const GuiSize2 size = host.widget_size_00aa6740(host.screen_widget(0xD8));
        const auto dy = static_cast<float>((static_cast<double>(size.y) + kRewardsRowGap) * kRewardsIjnTableRows);
        host.widget_set_position_00aa8240(jap, rewards_offset_y(host.widget_position_00aa6750(usa), dy));

        // 0062E3CC..0062E931: the side-1 rows from slot 6, numbered from 1.
        slot = kRewardsIjnFirstSlot;
        for (std::size_t i = 0; i < rows.size(); ++i) {
            if (rows[i].side != 1) continue;
            const auto index = static_cast<std::int32_t>(i);
            if (rewards_add_score_row(host, rows[i], index, slot - 5, slot, 1)) local_row = index;
            ++slot;
        }
    }

    // 0062E936..0062E9B7
    host.listbox_set_listener_00a9ac40(true);
    host.listbox_select_00a9c7c0(local_row);
    host.widget_set_active_60(host.screen_widget(0xD4), true);
    host.listbox_set_float(0x12C, kRewardsListbox12c);
    host.listbox_set_float(0x130, kRewardsListbox130);
    host.listbox_set_float(0x134, 0.0f);
    host.listbox_set_124_00a9ceb0(kRewardsListbox124, false);
    host.listbox_set_float(0x128, kRewardsListbox128);
}

} // namespace

void rewards_screen_enter_pages_0062ca7b(RewardsEnterState& screen, RewardsEnterHost& host) {
    // 0062CA7B..0062CA92
    if (!screen.pages238.empty()) screen.input_locked258 = true;

    // 0062CA99..0062CB88: both background icons take state 1 for a lost mission.
    const std::int32_t lost_state = screen.commit_present19 ? 0 : 1;
    host.icon_state_88(host.child_00aa7e00(host.screen_widget(0x20), "backgrnd_Icon"), lost_state);
    host.icon_state_88(host.child_00aa7e00(host.screen_widget(0x25C), "back_Icon"), lost_state);

    // 0062CB8D..0062CBD4
    if (host.game_byte(0x1EE3) == 0 && host.game_dword(0x1FE4) == 0) {
        screen.page_mode58 = screen.commit_present19 ? RewardsPageMode::CampaignWon : RewardsPageMode::CampaignLost;
        host.set_debrief_flags_e08c88(screen.commit_present19, false);
    } else {
        screen.page_mode58 = RewardsPageMode::Session;
        host.set_debrief_flags_e08c88(false, true);
    }

    // 0062CBDB..0062CBE0 and 0062D2FB..0062D2FE
    if (screen.page_mode58 == RewardsPageMode::CampaignWon) {
        rewards_fill_debriefing(screen, host);
    } else if (screen.page_mode58 == RewardsPageMode::Session) {
        rewards_fill_scoreboard(screen, host);
    }

    // 0062E9BF..0062EA44: a lost single-player mission titles page 2.
    if (host.game_byte(0x1EE3) == 0 && host.game_dword(0x1FE4) == 0 && !screen.commit_present19) {
        host.set_page_title_e196c0(2, kRewardsScoringMissionFailed);
    }
    // NOT RECONSTRUCTED from 0062EA1B on: the objectives page and everything after
    // it (docs/CC13_INGAME_GUI.md, rewards enter range table).
}

namespace {

using SW = SupportInfoWidget;

// CVTTSS2SI: out-of-range and NaN give the integer-indefinite INT_MIN.
std::int32_t truncate_cvttss2si(float value) noexcept {
    if (!(value > -2147483904.0f && value < 2147483648.0f)) return INT32_MIN;
    return static_cast<std::int32_t>(value);
}

// The checked form: 00614060 find, 00611C80 against the end, then 006171B0.
void set_type_icon_checked(SupportManagerInfoHost& host, SupportObject record) {
    host.widget_set_state(SW::TypeIcon,
        host.icon_map_contains(record) ? host.icon_map_subscript(record) : 0);
}

void show_detail(SupportManagerInfoHost& host, bool detail) {
    host.widget_show(SW::DetailF0, !detail);
    host.widget_show(SW::DetailF4, detail);
}

// e.g. 0066B950..0066BA49: the loadout's device icon in state 3, or state
// 0/1 by flag 17h when there is no loadout.
void show_loadout_icon(SupportManagerInfoHost& host, SupportObject class_record, std::int32_t loadout,
    bool flag17) {
    if (loadout == 0) {
        host.widget_set_state(SW::LoadoutIcon, flag17 ? 1 : 0);
        return;
    }
    host.loadout_icon_set_texture(host.loadout_icon(class_record, loadout));
    host.widget_set_state(SW::LoadoutIcon, 0);
    host.widget_set_state(SW::LoadoutIcon, 3);
}

void support_info_page0(SupportManagerInfoState& screen, SupportManagerInfoHost& host) {
    screen.detail70 = false;
    // 0066B6FD..0066B775: list +3E0h at the clamped selection (unsigned JB).
    if (screen.list_b_count != 0) {
        host.rebuild_list_c_0066a8a0();
        const std::uint32_t count = host.list_c_count_3e8();
        if (count != 0) {
            if (screen.selection_2c4 >= count) screen.selection_2c4 = count - 1;
            const SupportListCEntry entry = host.list_c_entry(screen.selection_2c4);
            if (entry.live_unit) {
                const SupportObject unit = entry.unit;
                if (host.unit_is_kind(unit, 6)) {
                    // 0066B7A5..0066B86C
                    host.set_text_localised(SW::NameText, host.unit_name(unit));
                    host.layout_name_label_0065bbc0();
                    set_type_icon_checked(host, host.unit_class_538(unit));
                    show_detail(host, false);
                } else {
                    // 0066B871..0066BA49: kind 18h shows its owner's class and loadout.
                    const SupportObject owner = host.unit_is_kind(unit, 0x18) ? host.unit_owner_3d0(unit) : unit;
                    host.set_text_localised(SW::NameText, host.unit_name(unit));
                    screen.detail70 = true;
                    host.layout_name_label_0065bbc0();
                    set_type_icon_checked(host, host.unit_class_538(owner));
                    show_detail(host, true);
                    const std::int32_t loadout = host.unit_loadout_c54(owner);
                    show_loadout_icon(host, host.unit_class_538(owner), loadout,
                        loadout == 0 && host.unit_is_kind(owner, 0x17));
                }
            } else {
                // 0066BA4E..0066BCB3: a class record; +0Ch is the loadout.
                const SupportObject record = entry.record;
                const bool flag6 = host.record_has_flag(record, 6);
                host.set_text_localised(SW::NameText, host.record_name_54(record));
                if (!flag6) screen.detail70 = true;
                host.layout_name_label_0065bbc0();
                set_type_icon_checked(host, record);
                show_detail(host, !flag6);
                if (!flag6) {
                    show_loadout_icon(host, record, entry.loadout,
                        entry.loadout == 0 && host.record_has_flag(record, 0x17));
                }
            }
        }
    }

    // 0066BCB5..0066BCFA: the unit selected in list +1ECh.
    if (screen.list_a_count == 0) return;
    const SupportObject unit = host.selected_unit_a();
    const bool hangar = host.unit_is_kind(unit, 0x45) || host.unit_is_kind(unit, 9);
    if (!hangar) {
        // 0066BD62..0066BF53: 00846FD0's slot object, destroyed by 0065BB00.
        const SupportSlotEntry slot = host.unit_slot_00846fd0(unit, screen.slot_324);
        if (slot.record == 0) return;
        host.set_text_localised(SW::NameText, host.record_name_54(slot.record));
        host.layout_name_label_0065bbc0();
        host.widget_set_state(SW::TypeIcon, host.icon_map_subscript(slot.record));  // no find first
        const bool flag_f = host.record_has_flag(slot.record, 0xF);
        if (flag_f) screen.detail70 = true;
        show_detail(host, flag_f);
        if (flag_f) {
            show_loadout_icon(host, slot.record, slot.loadout,
                slot.loadout == 0 && host.record_has_flag(slot.record, 0x17));
        }
        return;
    }
    // 0066BF6E..0066C205: the 58h-byte hangar record under +72Ch (kind 45h,
    // queried again) or +1188h.
    const SupportSlotEntry slot = host.hangar_slot(unit, host.unit_is_kind(unit, 0x45), screen.slot_324);
    screen.detail70 = true;
    host.set_text_localised(SW::NameText, host.record_name_54(slot.record));
    host.layout_name_label_0065bbc0();
    host.widget_set_state(SW::TypeIcon, host.icon_map_subscript(slot.record));
    if (slot.loadout == -1) {
        show_detail(host, false);
        return;
    }
    show_detail(host, true);
    show_loadout_icon(host, slot.record, slot.loadout,
        slot.loadout == 0 && host.record_has_flag(slot.record, 0x17));
}

// 0066C206..0066C400: the type icon is 1 unless the front object's unit is
// kind 9 and +08h is empty.
void support_info_page1(const SupportManagerInfoState& screen, SupportManagerInfoHost& host) {
    if (screen.list_b_count == 0) return;
    const SupportListBEntry entry = host.selected_list_b();
    if (entry.node_value8 != 0) {
        host.widget_set_state(SW::TypeIcon, 1);
        return;
    }
    const bool kind9 = entry.front_unit3c != 0 && host.unit_is_kind(entry.front_unit3c, 9);
    host.widget_set_state(SW::TypeIcon, kind9 ? 0 : 1);
}

void show_catalog_name_and_icon(SupportManagerInfoHost& host, SupportObject record) {
    host.set_text_localised(SW::NameText, host.record_name_54(record));
    host.layout_name_label_0065bbc0();
    set_type_icon_checked(host, record);
}

void support_info_page2(SupportManagerInfoState& screen, SupportManagerInfoHost& host) {
    if (screen.list_b_count == 0) {
        // 0066C99C..0066CC1C: stock of the catalog entry for the selected unit.
        if (screen.list_a_count == 0) return;
        host.sync_cursor_0065a2b0();
        const SupportObject unit = host.selected_unit_a();
        SupportObject record;
        if (host.unit_is_kind(unit, 0x45) || host.unit_is_kind(unit, 9)) {
            record = screen.catalog_2a0 = host.catalog_0065ae90();
        } else if (host.unit_is_kind(unit, 0x46)) {
            record = screen.catalog_3a4 = host.catalog_0065ab40();
        } else {
            return;
        }
        show_catalog_name_and_icon(host, record);
        host.set_text_source(SW::CostText,
            std::string(kSupportInfoStockPrefix) + std::to_string(host.stock_count_0065ac30()));
        return;
    }

    // 0066C417..0066C522
    host.sync_cursor_0065a2b0();
    const SupportObject record = screen.catalog_3a4 = host.catalog_0065ab40();
    if (screen.field164 == 0) {
        screen.field2b0 = true;
        host.rebuild_00665f30();
    }
    show_catalog_name_and_icon(host, record);
    show_detail(host, !host.record_has_flag(record, 6));

    // 0066C520..0066C5C0: modes 0..3 and 8 show the price unless +66h is set.
    const float cp = host.party_cp_00946f00();
    const float cap = host.party_cp_cap_00942130();
    const std::int32_t cost = host.record_cost_124(record);
    const std::int32_t mode = host.effective_game_mode_004bca50();
    const bool priced_mode = mode == 0 || host.effective_game_mode_004bca50() == 1 ||
        host.effective_game_mode_004bca50() == 2 || host.effective_game_mode_004bca50() == 3 ||
        host.effective_game_mode_004bca50() == 8;
    if (!priced_mode || screen.flag66) {
        // 0066C5C6..0066C73B (two copies differing only in the EH state).
        host.set_text_source(SW::CostText, kSupportInfoSupportKey);
        host.widget_set_size(SW::BarAfford, {screen.bar_width_1ac, host.widget_height(SW::BarRemain)});
        host.widget_show(SW::BarCp, false);
        host.widget_show(SW::BarRemain, false);
        host.widget_show(SW::BarAfford, true);
        return;
    }

    // 0066C73C..0066C838: "sm_cp|.: <cp> / |sm_cost|.: <cost>".
    host.set_text_source(SW::CostText, std::string(kSupportInfoCpPrefix) +
        std::to_string(truncate_cvttss2si(cp)) + std::string(kSupportInfoCostSeparator) +
        std::to_string(cost));

    // 0066C83D..0066C851: FILD/FSTP rounds the cost to float; FCOMI then JB,
    // so an unordered compare also takes the unaffordable side.
    const float cost_f = static_cast<float>(cost);
    const double width = screen.bar_width_1ac;
    if (!(cp >= cost_f)) {
        // 0066C914..0066C99B
        if (!screen.field2c1) {
            host.widget_set_size(SW::BarCp,
                {static_cast<float>(width * cp / cap), host.widget_height(SW::BarRemain)});
            host.widget_show(SW::BarCp, true);
        }
        host.widget_show(SW::BarAfford, false);
        host.widget_show(SW::BarRemain, false);
        return;
    }
    // 0066C857..0066C913: what is left after the purchase fills +188h.
    host.widget_set_size(SW::BarAfford,
        {static_cast<float>(width * cp / cap), host.widget_height(SW::BarRemain)});
    host.widget_set_size(SW::BarRemain,
        {static_cast<float>((static_cast<double>(cp) - cost_f) * width / cap), host.widget_height(SW::BarRemain)});
    host.widget_show(SW::BarCp, false);
    host.widget_show(SW::BarAfford, true);
    host.widget_show(SW::BarRemain, true);
}

} // namespace

void support_manager_refresh_info_0066b6c0(SupportManagerInfoState& screen, SupportManagerInfoHost& host) {
    switch (screen.page) {
    case 0: support_info_page0(screen, host); break;
    case 1: support_info_page1(screen, host); break;
    case 2: support_info_page2(screen, host); break;
    default: break;
    }
}

} // namespace bsp