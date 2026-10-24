// In-game GUI pieces of segments 26 and 27. Evidence: docs/CC13_INGAME_GUI.md.

#include "bsp/ingame_gui.hpp"

#include "bsp/gui_layer.hpp"
#include "bsp/main_menu_mission_detail.hpp"
#include "bsp/profile_checkpoint.hpp"

#include <algorithm>
#include <cstdio>
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

std::string format_marker_distance_0063e5c0(void* target, MarkerDistanceHost& host) {
    float distance = 0.0f;                         // 0063E5D8 XORPS / 0063E5E9
    void* const camera = host.controlled_unit_00e188d8(); // 0063E60D
    if (camera != nullptr && target != nullptr) {  // 0063E613, 0063E61B
        if (!host.pose_current_c8(camera)) host.refresh_pose_00414db0(camera); // 0063E61F
        if (!host.pose_current_c8(target)) host.refresh_pose_00414db0(target); // 0063E62E
        float from[3];
        float to[3];
        host.position_fc(camera, from);
        host.position_fc(target, to);
        const float vec[3] = {to[0] - from[0], to[1] - from[1], to[2] - from[2]};
        distance = host.vector_length_0042b2f0(vec); // 0063E689, FSTP float
    }
    // The flag is read only on the text path (after the 0063E6AF zero test).
    if (distance == 0.0f) return {};
    return format_marker_distance_0063e5c0(distance, host.nautical_miles_00f88988());
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

std::vector<RewardsScoreRow> rewards_build_score_rows_0062a0b0(RewardsEnterHost& host) {
    // 0062A0D4..0062A121: the old rows go (00625980); the game mode is read once.
    std::vector<RewardsScoreRow> rows;
    const std::int32_t mode = host.effective_game_mode_004bca50();
    for (std::int32_t slot = 0; slot < 8; ++slot) {
        // 0062A130..0062A15C: a null slot, a negative index, or an AI slot
        // without its +0Ah byte is skipped.
        RewardsPlayerSlot player;
        if (!host.player_slot_18cc(slot, player)) continue;
        if (player.index20 < 0) continue;
        if (player.ai09 && !player.byte0a) continue;
        // 0062A162..0062A1D4: a zeroed row is pushed (00627820), then filled in place.
        RewardsScoreRow& row = rows.emplace_back();

        // 0062A1D9..0062A2A9: badge count over the score record's key list.
        std::int32_t badges = 0;
        for (const std::string& key : host.player_score_keys_6c(slot)) {
            if (host.badge_award_earned_0090c000(key)) ++badges;
        }

        // 0062A2AE..0062A4E2: an AI with a name of at most one character is
        // named by its difficulty.
        if (player.ai09 && player.name58.size() <= 1) {
            row.name = std::string(kRewardsAiNameKeys[static_cast<std::size_t>(player.difficulty2c)]);
        } else {
            row.name = player.name58;
        }
        // 0062A4E5..0062A695
        row.clan = player.ai09 ? std::string(kRewardsAiClan) : player.clan78;
        // 0062A698..0062A935
        row.id0 = player.id80;
        row.id4 = player.id84;
        row.ai5c = player.ai09;
        row.side = player.side28;
        row.score = host.player_score_1e0(slot);
        row.game_mode34 = mode;
        row.badges48 = badges;
        row.rank = player.index20 == host.game_dword(0x18EC) ? host.profile_rank_byte_00574050() : player.rank90;
    }
    return rows;
}

void rewards_sort_score_rows_00625d10(std::vector<RewardsScoreRow>& rows) {
    // 00623457/006234B1: CMP new+50h against the row before it, JG/JLE; a
    // row moves up only past strictly lower scores.
    std::stable_sort(rows.begin(), rows.end(),
                     [](const RewardsScoreRow& a, const RewardsScoreRow& b) { return a.score > b.score; });
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
        host.icon_state_88(medal, rewards_icon_state(record.field04));
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
    std::vector<RewardsScoreRow>& rows = host.score_rows_220();
    rows = rewards_build_score_rows_0062a0b0(host);
    rewards_sort_score_rows_00625d10(rows);

    std::int32_t local_row = 0;  // [esp+9Ch]
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

// The objectives page cursor: x from the item template (+DCh), the running
// y, and z = -1.0f ([esp+78h..80h]).
struct RewardsObjectiveLayout {
    RewardsVec3 cursor;
    float gap{};      // [esp+9Ch]: +E0h.y - +ECh.y
    float spacing{};  // [esp+44h]
};

// One objectives section: header text (`header`), box (`box`, resized to the
// lines), then one clone of +DCh per list node. Native copies: primary
// 0062F06A..0062F742, secondary 0062EBC5..0062F966, hidden 0062F97B..00630173.
void rewards_objective_section(RewardsEnterHost& host, RewardsObjectiveLayout& layout, std::int32_t side,
                               RewardsObjectiveKind kind, std::uint32_t box_offset, std::uint32_t header_offset,
                               std::uint32_t header_x_offset) {
    const std::vector<RewardsObjective>& list = host.commit_objectives(side, kind);
    const RewardsWidget box = host.screen_widget(box_offset);
    const RewardsWidget header = host.screen_widget(header_offset);
    RewardsVec3& at = layout.cursor;
    if (list.empty()) {
        // 0062EBA0..0062EBBF, 0062F968..0062F979, 00630175..
        host.widget_show_34(box, false);
        host.widget_show_34(header, false);
        return;
    }
    host.widget_show_34(box, true);
    host.widget_show_34(header, true);
    // The primary header takes its x from the secondary header (+F0h), 0062F09B.
    host.widget_set_position_00aa8240(header, {host.widget_position_00aa6750(host.screen_widget(header_x_offset)).x, at.y, at.z});
    at.y = at.y + layout.gap;
    host.widget_set_position_00aa8240(
        box, {host.widget_position_00aa6750(box).x, at.y, static_cast<float>(static_cast<double>(at.z) + 1.0)});
    // FILD count, FMUL line, FADDP line: the box is one line taller than its list.
    const auto count = static_cast<std::int32_t>(list.size());
    host.widget_set_size_58(box, {host.widget_size_00aa6740(box).x,
                                  static_cast<float>(count * kRewardsObjectiveLine + kRewardsObjectiveLine)});
    at.y = static_cast<float>(at.y + kRewardsObjectiveHeaderGap);

    std::int32_t number = 1;
    for (const RewardsObjective& node : list) {
        const RewardsWidget item = host.clone_00aab4c0(host.screen_widget(0xDC));
        const std::string text = node.key.size() > 1 ? node.key.substr(1) : std::string();
        bool completed = node.completed != 0;
        host.text_source_00abaed0(host.child_00aa7e00(item, "number_Text"),
                                  std::to_string(number) + std::string(kRewardsObjectiveNumberSuffix), true);
        if (kind == RewardsObjectiveKind::Primary) {
            host.text_ellipsized_00abb000(host.child_00aa7e00(item, "primary_Text"), text, kRewardsObjectiveDepth, true);
        } else if (kind == RewardsObjectiveKind::Secondary) {
            host.text_source_00abaed0(host.child_00aa7e00(item, "primary_Text"), text, true);
        } else {
            // 0062FB16..0062FD98: a hidden objective is named only once the
            // 00594A70 record marks it; its own +1Ch still picks the verdict.
            const bool revealed = host.hidden_objective_revealed(side, node.key);
            host.text_source_00abaed0(host.child_00aa7e00(item, "primary_Text"),
                                      revealed ? text : std::string(kRewardsHiddenObjective), true);
        }
        host.text_cstring_00abbe50(host.child_00aa7e00(item, "primary_value_Text"),
                                   completed ? kRewardsObjectiveCompleted : kRewardsObjectiveFailed, true);
        std::int32_t points = 0;
        const bool scored = host.commit_objective_points(node.points_key, points);
        host.text_source_00abaed0(host.child_00aa7e00(item, "primary_point_value_Text"),
                                  scored ? std::to_string(points) : std::string(kRewardsZeroPoints), false);
        if (!completed) host.widget_set_color_50(host.child_00aa7e00(item, "primary_value_Text"), kRewardsFailedColor);
        host.widget_show_34(item, true);
        host.keep_created_widget_f8_0052ed20(item);
        host.widget_set_position_00aa8240(item, at);
        at.y = static_cast<float>(at.y + kRewardsObjectiveLine);
        ++number;
    }
    // FLD spacing, FADD 1/72, FADD y.
    at.y = static_cast<float>(layout.spacing + kRewardsObjectiveHeaderGap + at.y);
}

// 0062EA1B..00630188: the objectives page.
void rewards_fill_objectives(RewardsEnterHost& host) {
    const RewardsCommitRecord& record = host.commit_record();
    const std::int32_t side = host.game_byte(0x1EE3) == 0 ? host.mission_side_005c27e0() : (record.field1e4 == 0 ? 1 : 0);

    // 0062EA85..0062EB79: the free height is shared out between the gaps
    // above, between and below the non-empty sections.
    const auto pri = static_cast<std::int32_t>(host.commit_objectives(side, RewardsObjectiveKind::Primary).size());
    const auto sec = static_cast<std::int32_t>(host.commit_objectives(side, RewardsObjectiveKind::Secondary).size());
    const auto hid = static_cast<std::int32_t>(host.commit_objectives(side, RewardsObjectiveKind::Hidden).size());
    RewardsObjectiveLayout layout;
    const double top_y = host.widget_position_00aa6750(host.screen_widget(0xE0)).y;
    layout.gap = static_cast<float>(top_y - host.widget_position_00aa6750(host.screen_widget(0xEC)).y);
    const std::int32_t sections = (hid > 0) + (sec > 0) + (pri > 0);
    const std::int32_t lines = hid + sec + pri;
    layout.spacing = static_cast<float>(
        ((kRewardsObjectiveArea - lines * kRewardsObjectiveLine) - (kRewardsObjectiveLine + layout.gap) * sections) /
        (sections + 1));
    layout.cursor = {host.widget_position_00aa6750(host.screen_widget(0xDC)).x,
                     static_cast<float>(layout.spacing + kRewardsObjectiveTop), kRewardsObjectiveDepth};

    host.set_objectives_flag_e08c8a(pri != 0 || sec != 0 || hid != 0);
    rewards_objective_section(host, layout, side, RewardsObjectiveKind::Primary, 0xE0, 0xEC, 0xF0);
    rewards_objective_section(host, layout, side, RewardsObjectiveKind::Secondary, 0xE4, 0xF0, 0xF0);
    rewards_objective_section(host, layout, side, RewardsObjectiveKind::Hidden, 0xE8, 0xF4, 0xF4);
}

// What 0063018D..0063089E measures once for the scoring page: +10Ch is the
// table template, +110h the group header, +114h the row, +118h their parent.
struct RewardsScoringLayout {
    RewardsVec3 table_pos;        // [esp+134h], world position of +10Ch
    RewardsVec3 group_pos;        // [esp+B4h], +110h
    RewardsVec3 row_pos;          // [esp+CCh], +114h
    RewardsVec3 cursor;           // [esp+48h], starts at table_pos
    // Local positions of the +10Ch children, re-applied to every clone.
    RewardsVec3 local_separator, local_row_line, local_vline[4];
    RewardsVec3 row_line_pos;     // [esp+C0h], world position of +10Ch/row_line_FrameBox
    GuiSize2 row_line_size;       // [esp+ECh]
    float vline_width{};          // [esp+58h]
    GuiSize2 group_row_line_size; // [esp+D8h]
    GuiSize2 done_to_size;        // [esp+F4h]
    std::int32_t mode{};          // [esp+40h]
};

constexpr std::array<std::string_view, 4> kRewardsVlines{"vline1_FrameBox", "vline2_FrameBox", "vline3_FrameBox",
                                                         "vline4_FrameBox"};

RewardsScoringLayout rewards_measure_scoring(RewardsEnterHost& host) {
    RewardsScoringLayout s;
    const RewardsWidget table = host.screen_widget(0x10C);
    const RewardsWidget group = host.screen_widget(0x110);
    const RewardsWidget row = host.screen_widget(0x114);
    s.mode = host.game_dword(0x1FE4) != 0 ? 2 : host.game_dword(0x6AC);
    s.table_pos = host.widget_position_00aa6750(table);
    s.group_pos = host.widget_position_00aa6750(group);
    s.row_pos = host.widget_position_00aa6750(row);
    s.cursor = s.table_pos;
    s.local_separator = host.widget_local_position_00aa67f0(host.child_00aa7e00(table, "separator_line_FrameBox"));
    s.local_row_line = host.widget_local_position_00aa67f0(host.child_00aa7e00(table, "row_line_FrameBox"));
    for (std::size_t i = 0; i < kRewardsVlines.size(); ++i) {
        s.local_vline[i] = host.widget_local_position_00aa67f0(host.child_00aa7e00(table, kRewardsVlines[i]));
    }
    s.row_line_pos = host.widget_position_00aa6750(host.child_00aa7e00(table, "row_line_FrameBox"));
    s.row_line_size = host.widget_size_00aa6740(host.child_00aa7e00(table, "row_line_FrameBox"));
    s.vline_width = host.widget_size_00aa6740(host.child_00aa7e00(table, "vline1_FrameBox")).x;
    s.group_row_line_size = host.widget_size_00aa6740(host.child_00aa7e00(group, "row_line_FrameBox"));
    s.done_to_size = host.widget_size_00aa6740(host.child_00aa7e00(row, "done_to_value_Text"));
    // 006307BF..0063084F: the done-to column runs up to the total column less 1/48.
    const double total_x = host.widget_local_position_00aa67f0(host.child_00aa7e00(row, "total_value_Text")).x;
    s.done_to_size.x = static_cast<float>(
        (total_x - host.widget_local_position_00aa67f0(host.child_00aa7e00(row, "done_to_value_Text")).x) -
        kRewardsDoneToMargin);
    return s;
}

// A table: a clone of +10Ch under +118h at the cursor, its title, the saved
// child positions, then the cursor moves to the first group line.
RewardsWidget rewards_table_begin(RewardsEnterHost& host, RewardsScoringLayout& s, std::string_view title,
                                  bool command_columns) {
    const RewardsWidget table = host.clone_under_00aab4c0(host.screen_widget(0x10C), host.screen_widget(0x118));
    host.widget_set_position_00aa8240(table, s.cursor);
    host.text_ellipsized_00abb000(host.child_00aa7e00(table, "header_Text"), title, kRewardsObjectiveDepth, true);
    if (command_columns) {
        // 00633080..0063328E
        host.text_ellipsized_00abb000(host.child_00aa7e00(table, "done_by_Text"), kRewardsScoringCommandType,
                                      kRewardsObjectiveDepth, true);
        host.text_ellipsized_00abb000(host.child_00aa7e00(table, "done_to_Text"), kRewardsScoringCommandAmount,
                                      kRewardsObjectiveDepth, true);
        host.widget_show_34(host.child_00aa7e00(table, "damage_Text"), false);
        host.widget_show_34(host.child_00aa7e00(table, "kills_Text"), false);
    }
    for (std::size_t i = 0; i < kRewardsVlines.size(); ++i) {
        host.widget_set_local_position_00aa7dc0(host.child_00aa7e00(table, kRewardsVlines[i]), s.local_vline[i]);
    }
    host.widget_set_local_position_00aa7dc0(host.child_00aa7e00(table, "row_line_FrameBox"), s.local_row_line);
    host.widget_set_local_position_00aa7dc0(host.child_00aa7e00(table, "separator_line_FrameBox"), s.local_separator);
    host.widget_show_34(table, true);
    s.cursor.y = static_cast<float>((static_cast<double>(s.group_pos.y) - s.table_pos.y) + s.cursor.y);
    return table;
}

// The vertical lines run from vline1's top to the cursor; the command table
// hides vline2 and vline3 instead (006357A4..00635811).
void rewards_table_end(RewardsEnterHost& host, const RewardsScoringLayout& s, RewardsWidget table, bool command_columns) {
    const float top = host.widget_position_00aa6750(host.child_00aa7e00(table, "vline1_FrameBox")).y;
    const GuiSize2 vline{s.vline_width, s.cursor.y - top};
    for (std::size_t i = 0; i < kRewardsVlines.size(); ++i) {
        const RewardsWidget line = host.child_00aa7e00(table, kRewardsVlines[i]);
        if (command_columns && (i == 1 || i == 2)) {
            host.widget_show_34(line, false);
        } else {
            host.widget_set_size_58(line, vline);
        }
    }
    host.widget_set_size_58(host.child_00aa7e00(table, "row_line_FrameBox"), s.row_line_size);
    host.widget_set_size_58(host.child_00aa7e00(table, "separator_line_FrameBox"), s.row_line_size);
}

RewardsWidget rewards_group_begin(RewardsEnterHost& host, const RewardsScoringLayout& s, std::string_view title) {
    const RewardsWidget group = host.clone_under_00aab4c0(host.screen_widget(0x110), host.screen_widget(0x118));
    host.text_ellipsized_00abb000(host.child_00aa7e00(group, "done_by_value_Text"), title, kRewardsObjectiveDepth, true);
    host.widget_set_position_00aa8240(group, {s.group_pos.x, s.cursor.y, s.group_pos.z});
    host.widget_show_34(group, true);
    return group;
}

void rewards_group_end(RewardsEnterHost& host, RewardsScoringLayout& s, RewardsWidget group) {
    s.cursor.y = static_cast<float>(s.cursor.y + kRewardsScoreGroupGap);
    host.widget_set_size_58(host.child_00aa7e00(group, "row_line_FrameBox"), s.group_row_line_size);
    host.widget_set_position_00aa8240(host.child_00aa7e00(group, "row_line_FrameBox"),
                                      {s.row_line_pos.x, s.cursor.y, s.row_line_pos.z});
}

RewardsWidget rewards_row_clone(RewardsEnterHost& host) {
    return host.clone_under_00aab4c0(host.screen_widget(0x114), host.screen_widget(0x118));
}

void rewards_row_place(RewardsEnterHost& host, RewardsScoringLayout& s, RewardsWidget row) {
    host.widget_set_position_00aa8240(row, {s.row_pos.x, s.cursor.y, s.row_pos.z});
    host.widget_show_34(row, true);
}

void rewards_row_advance(RewardsScoringLayout& s) {
    s.cursor.y = static_cast<float>(s.cursor.y + kRewardsScoreRow);
}

void rewards_row_hide_damage_kills(RewardsEnterHost& host, RewardsWidget row) {
    host.widget_show_34(host.child_00aa7e00(row, "damage_value_Text"), false);
    host.widget_show_34(host.child_00aa7e00(row, "kills_value_Text"), false);
}

// "<category>|. (x<count>)", the remaining/suicide row label (00633957..00633A3B).
std::string rewards_count_label(std::int32_t category, std::int32_t count) {
    std::string text(kRewardsUnitCategoryKeys[static_cast<std::size_t>(category)]);
    text += kRewardsCountOpen;
    text += std::to_string(count);
    text += kRewardsCountClose;
    return text;
}

// 00630C13..0063181C (player, maps +9Ch/+B4h) and 00631F1B..00632BD2 (party,
// +A8h/+C0h). A friendly hit (L2 key 0) prefixes the target and paints three
// (player) or four (party) columns.
void rewards_damage_table(RewardsEnterHost& host, RewardsScoringLayout& s, std::string_view title,
                          std::uint32_t damage_offset, std::uint32_t kills_offset, bool party) {
    const RewardsCommitRecord& record = host.commit_record();
    host.set_scoring_flag_e08c8b(false);
    const RewardsWidget table = rewards_table_begin(host, s, title, false);
    for (const RewardsDamageGroup& by : host.damage_groups(damage_offset)) {
        const RewardsWidget group =
            rewards_group_begin(host, s, kRewardsUnitCategoryKeys[static_cast<std::size_t>(by.by_category)]);
        for (const RewardsDamageSide& side : by.sides) {
            for (const RewardsDamageTarget& target : side.targets) {
                std::int32_t points = host.damage_points_00911ce0(by.by_category, target.category, side.friendly_key,
                                                                  target.damage, s.mode, record.flag24d);
                const RewardsWidget row = rewards_row_clone(host);
                host.text_source_00abaed0(host.child_00aa7e00(row, "damage_value_Text"), std::to_string(target.damage),
                                          false);
                const std::int32_t kills =
                    host.kills_from_copy(kills_offset, side.friendly_key, by.by_category, target.category);
                points += host.kill_points_00911c00(side.friendly_key, by.by_category, target.category,
                                                    static_cast<float>(kills), s.mode, record.flag24d);
                host.text_source_00abaed0(host.child_00aa7e00(row, "kills_value_Text"), std::to_string(kills), false);
                host.text_source_00abaed0(host.child_00aa7e00(row, "total_value_Text"), std::to_string(points), false);
                rewards_row_place(host, s, row);
                const std::string_view category = kRewardsUnitCategoryKeys[static_cast<std::size_t>(target.category)];
                if (side.friendly_key == 0) {
                    host.text_ellipsized_00abb000(host.child_00aa7e00(row, "done_to_value_Text"),
                                                  std::string(kRewardsFriendlyHitPrefix) + std::string(category),
                                                  kRewardsObjectiveDepth, true);
                    host.widget_set_color_50(host.child_00aa7e00(row, "done_to_value_Text"), kRewardsFailedColor);
                    if (party) {
                        host.widget_set_color_50(host.child_00aa7e00(row, "damage_value_Text"), kRewardsFailedColor);
                        host.widget_set_color_50(host.child_00aa7e00(row, "kills_value_Text"), kRewardsFailedColor);
                        host.widget_set_color_50(host.child_00aa7e00(row, "total_value_Text"), kRewardsFailedColor);
                    } else {
                        host.widget_set_color_50(host.child_00aa7e00(row, "total_value_Text"), kRewardsFailedColor);
                        host.widget_set_color_50(host.child_00aa7e00(row, "kills_value_Text"), kRewardsFailedColor);
                    }
                } else {
                    host.text_ellipsized_00abb000(host.child_00aa7e00(row, "done_to_value_Text"), category,
                                                  kRewardsObjectiveDepth, true);
                }
                rewards_row_advance(s);
            }
        }
        rewards_group_end(host, s, group);
    }
    rewards_table_end(host, s, table, false);
}

// 006348A6..00634900: "%d:%02d:%02d" from an hour, "%d:%02d" from a minute,
// else "%d" of at least 1; the seconds are truncated (CVTTSS2SI).
std::string rewards_map_usage_time(float seconds) {
    const auto t = static_cast<std::int32_t>(seconds);
    char buffer[64];
    if (t >= 3600) {
        std::snprintf(buffer, sizeof buffer, "%d:%02d:%02d", t / 3600, (t % 3600) / 60, (t % 3600) % 60);
    } else if (t >= 60) {
        std::snprintf(buffer, sizeof buffer, "%d:%02d", t / 60, t % 60);
    } else {
        std::snprintf(buffer, sizeof buffer, "%d", t > 0 ? t : 1);
    }
    return buffer;
}

// 00632F5C..006359A3: the command table and its five groups.
void rewards_command_table(RewardsEnterHost& host, RewardsScoringLayout& s) {
    const RewardsCommitRecord& record = host.commit_record();
    const std::vector<RewardsRemainingSide> remaining = host.remaining_units_cc();
    const std::vector<RewardsCategoryCount> suicides = host.suicides_d8();
    const std::vector<RewardsMissionSpecial> specials = host.mission_specials_48();
    // 00632F5C..00632F8E: COMISS +14h > 0.0f, +1DCh signed.
    if (remaining.empty() && suicides.empty() && !(record.map_seconds > 0.0f) && record.badge <= 0 && specials.empty()) {
        return;
    }
    host.set_scoring_flag_e08c8b(false);
    const RewardsWidget table = rewards_table_begin(host, s, kRewardsScoringCommand, true);

    // 0063354D..00633CF6
    if (!remaining.empty()) {
        const RewardsWidget group = rewards_group_begin(host, s, kRewardsScoringRemaining);
        for (const RewardsRemainingSide& side : remaining) {
            if (side.side_key != 0) continue;
            for (const RewardsCategoryCount& entry : side.counts) {
                const std::int32_t points =
                    host.remaining_points_00910500(entry.category, record.field08) * entry.count;
                const RewardsWidget row = rewards_row_clone(host);
                rewards_row_hide_damage_kills(host, row);
                host.text_source_00abaed0(host.child_00aa7e00(row, "total_value_Text"), std::to_string(points), false);
                host.widget_set_size_58(host.child_00aa7e00(row, "done_to_value_Text"), s.done_to_size);
                host.text_ellipsized_00abb000(host.child_00aa7e00(row, "done_to_value_Text"),
                                              rewards_count_label(entry.category, entry.count), kRewardsObjectiveDepth,
                                              true);
                rewards_row_place(host, s, row);
                rewards_row_advance(s);
            }
        }
        rewards_group_end(host, s, group);
    }

    // 00633CFB..00634576
    if (!suicides.empty()) {
        const RewardsWidget group = rewards_group_begin(host, s, kRewardsScoringSuicide);
        for (const RewardsCategoryCount& entry : suicides) {
            const std::int32_t points =
                host.suicide_points_00910570(entry.category, s.mode, record.field04) * entry.count;
            const RewardsWidget row = rewards_row_clone(host);
            rewards_row_hide_damage_kills(host, row);
            host.text_source_00abaed0(host.child_00aa7e00(row, "total_value_Text"), std::to_string(points), false);
            host.widget_set_color_50(host.child_00aa7e00(row, "total_value_Text"), kRewardsFailedColor);
            host.widget_set_size_58(host.child_00aa7e00(row, "done_to_value_Text"), s.done_to_size);
            host.text_ellipsized_00abb000(host.child_00aa7e00(row, "done_to_value_Text"),
                                          rewards_count_label(entry.category, entry.count), kRewardsObjectiveDepth, true);
            host.widget_set_color_50(host.child_00aa7e00(row, "done_to_value_Text"), kRewardsFailedColor);
            rewards_row_place(host, s, row);
            rewards_row_advance(s);
        }
        rewards_group_end(host, s, group);
    }

    // 0063457B..00634B93
    if (record.map_seconds > 0.0f) {
        const RewardsWidget group = rewards_group_begin(host, s, kRewardsScoringMapUsage);
        const std::int32_t points = host.map_usage_points_009064d0(record.map_seconds);
        const RewardsWidget row = rewards_row_clone(host);
        rewards_row_hide_damage_kills(host, row);
        host.text_source_00abaed0(host.child_00aa7e00(row, "total_value_Text"), std::to_string(points), false);
        host.widget_set_size_58(host.child_00aa7e00(row, "done_to_value_Text"), s.done_to_size);
        host.text_ellipsized_00abb000(host.child_00aa7e00(row, "done_to_value_Text"),
                                      rewards_map_usage_time(record.map_seconds), kRewardsObjectiveDepth, false);
        rewards_row_place(host, s, row);
        rewards_row_advance(s);
        rewards_group_end(host, s, group);
    }

    // 00634B98..006350EF: the done-to column shows the badge points / 100.
    if (record.badge > 0) {
        const RewardsWidget group = rewards_group_begin(host, s, kRewardsScoringBadge);
        const RewardsWidget row = rewards_row_clone(host);
        rewards_row_hide_damage_kills(host, row);
        host.text_source_00abaed0(host.child_00aa7e00(row, "total_value_Text"), std::to_string(record.badge), false);
        host.widget_set_size_58(host.child_00aa7e00(row, "done_to_value_Text"), s.done_to_size);
        host.text_ellipsized_00abb000(host.child_00aa7e00(row, "done_to_value_Text"), std::to_string(record.badge / 100),
                                      kRewardsObjectiveDepth, false);
        rewards_row_place(host, s, row);
        rewards_row_advance(s);
        rewards_group_end(host, s, group);
    }

    // 006350F4..00635668
    if (!specials.empty()) {
        const RewardsWidget group = rewards_group_begin(host, s, kRewardsScoringMissionSpec);
        for (const RewardsMissionSpecial& entry : specials) {
            const RewardsWidget row = rewards_row_clone(host);
            rewards_row_hide_damage_kills(host, row);
            host.text_source_00abaed0(host.child_00aa7e00(row, "total_value_Text"), std::to_string(entry.points), false);
            host.widget_set_size_58(host.child_00aa7e00(row, "done_to_value_Text"), s.done_to_size);
            host.text_ellipsized_00abb000(host.child_00aa7e00(row, "done_to_value_Text"), entry.name,
                                          kRewardsObjectiveDepth, true);
            rewards_row_place(host, s, row);
            rewards_row_advance(s);
        }
        rewards_group_end(host, s, group);
    }

    rewards_table_end(host, s, table, true);
}

// 00630188..006359A3: the scoring page. Returns the final cursor y, which
// the rank page code at 006359A3 reads ([esp+4Ch]).
float rewards_fill_scoring(RewardsEnterHost& host) {
    host.set_scoring_flag_e08c8b(false);
    RewardsScoringLayout s = rewards_measure_scoring(host);
    if (!host.damage_groups(0x9C).empty()) rewards_damage_table(host, s, kRewardsScoringPlayerAct, 0x9C, 0xB4, false);
    if (!host.damage_groups(0xA8).empty()) rewards_damage_table(host, s, kRewardsScoringPartyAct, 0xA8, 0xC0, true);
    rewards_command_table(host, s);
    return s.cursor.y;
}

// 006359A3..006359F1: the scoring scroller (+128h) range is the cursor less
// the +108h view's height and y.
void rewards_scoring_scroll(RewardsEnterHost& host, float cursor_y) {
    const RewardsWidget view = host.screen_widget(0x108);
    const double below = static_cast<double>(cursor_y) - host.widget_size_00aa6740(view).y;
    host.scroller_set_range_006834a0(0x128, static_cast<float>(below - host.widget_position_00aa6750(view).y));
}

// 006359F6..00635F9E: the rank page, only in a session whose +4Dh byte is set.
void rewards_fill_rank(RewardsEnterHost& host) {
    host.set_rank_flag_e08c8c(false);
    if (host.game_byte(0x1EE3) == 0 || host.session_byte_4d() == 0) return;
    host.set_rank_flag_e08c8c(true);

    // 00635A32..00635A79: rank name and icon from the clamped rank index.
    std::int32_t rank = host.profile_rank_byte_00574050() - 1;
    if (rank < 0) rank = 0;
    else if (rank > 19) rank = 19;
    host.text_cstring_00abbe50(host.screen_widget(0x204), kRewardsPlayerRankKeys[static_cast<std::size_t>(rank)], true);
    host.icon_state_88(host.screen_widget(0x208), rank);

    // 00635A7B..00635BCE: the "RANK" thresholds either side.
    const std::int32_t next = host.profile_rank_byte_00574050() != kRewardsTopRank ? host.rank_threshold_0050fc30(rank + 1) : 0;
    const std::int32_t previous = host.profile_rank_byte_00574050() > 1 ? host.rank_threshold_0050fc30(rank) : 0;

    // 00635BCE..00635C87: the gain since the last screen (+30h, -1 before any).
    const std::int32_t score = host.profile_score_0090bf50();
    const std::int32_t last = host.profile_last_score_30();
    host.text_source_00abaed0(host.screen_widget(0x200), std::to_string(last == -1 ? score : score - last), false);

    // 00635C87..00635CE9: FILD/FIDIV, so both shares are float quotients.
    float before = 1.0f;
    float now = 1.0f;
    if (next != 0) {
        before = last >= previous ? static_cast<float>(static_cast<double>(last - previous) / (next - previous)) : 0.0f;
        now = static_cast<float>(static_cast<double>(score - previous) / (next - previous));
    }
    host.set_profile_last_score_30(score);

    // 00635CED..00635EB9
    if (next != 0) {
        host.text_source_00abaed0(host.screen_widget(0x1FC),
                                  std::to_string(score) + std::string(kRewardsScoreSeparator) + std::to_string(next), false);
    } else {
        host.text_source_00abaed0(host.screen_widget(0x1FC), std::to_string(score), false);
    }

    // 00635EB9..00635F9C: both bars (+20Ch, +210h) start at the old share;
    // +21Ch is the width the update (0062B640) grows them to.
    const float height = host.widget_size_00aa6740(host.screen_widget(0x20C)).y;
    const auto width = static_cast<float>(static_cast<double>(host.widget_size_00aa6740(host.screen_widget(0x214)).x) -
                                          kRewardsRankBarInset);
    host.set_screen_byte(0x218, 0);
    host.set_screen_float(0x21C, clamp_pointer_004155b0(now, {0.0f, 1.0f}) * width);
    const GuiSize2 bar{clamp_pointer_004155b0(before, {0.0f, 1.0f}) * width, height};
    host.widget_set_size_58(host.screen_widget(0x20C), bar);
    host.widget_set_size_58(host.screen_widget(0x210), bar);
}

// One award entry: a clone of +18Ch under +188h with award_Icon,
// award_title_Text and award_detail_Text already filled by the caller, then
// laid out at the running local position (`at`, [esp+E0h]). Native copies:
// 00636258..006363A7, 006365DA..0063671F, 00636A4D..00636BA7.
void rewards_award_layout(RewardsEnterHost& host, RewardsWidget item, RewardsWidget icon, RewardsWidget title,
                          RewardsWidget detail, RewardsVec3& at) {
    // The detail sits 1/144 below the title's text.
    const double title_bottom =
        static_cast<double>(host.widget_local_position_00aa67f0(title).y) + host.text_height_00ab6bd0(title);
    host.widget_set_local_y_00aa78f0(detail, static_cast<float>(title_bottom + kRewardsAwardGap));
    host.widget_show_34(item, true);
    host.widget_set_local_position_00aa7dc0(item, at);
    // FCOMI: the icon height wins only when it exceeds the float-rounded text
    // height; otherwise the text height is recomputed unrounded.
    const double icon_height = host.widget_size_00aa6740(icon).y;
    const double text_height =
        (static_cast<double>(host.widget_local_position_00aa67f0(detail).y) + host.text_height_00ab6bd0(detail)) -
        host.widget_local_position_00aa67f0(title).y;
    const double height = icon_height > static_cast<float>(text_height) ? icon_height : text_height;
    host.widget_set_size_58(item, {host.widget_size_00aa6740(item).x, static_cast<float>(height + kRewardsAwardPadding)});
    at.y = static_cast<float>(host.widget_size_00aa6740(item).y + kRewardsAwardGap + at.y);
}

struct RewardsAwardItem {
    RewardsWidget item;
    RewardsWidget icon;
};

RewardsAwardItem rewards_award_clone(RewardsEnterHost& host) {
    const RewardsWidget item = host.clone_under_00aab4c0(host.screen_widget(0x18C), host.screen_widget(0x188));
    return {item, host.child_00aa7e00(item, "award_Icon")};
}

// 00635FA3..00636C33: the award pages (+18Ch entries in the +184h view).
void rewards_fill_awards(RewardsEnterState& screen, RewardsEnterHost& host) {
    const RewardsCommitRecord& record = host.commit_record();
    host.set_awards_flag_e08c8d(false);
    if (host.game_byte(0x1EE3) == 0 && !screen.commit_present19) return;
    const std::vector<RewardsUnlockAward> unlocks = host.unlock_awards_248();
    host.set_awards_flag_e08c8d(host.profile_has_pending_awards_0061eb80() || !unlocks.empty() || record.badge > 0);
    RewardsVec3 at = host.widget_local_position_00aa67f0(host.screen_widget(0x18C));

    // 0063603D..006363A7: one entry per unlocked vehicle class.
    for (const RewardsUnlockAward& unlock : unlocks) {
        const RewardsAwardItem entry = rewards_award_clone(host);
        host.award_icon_by_id_004fb150(unlock.icon_id, entry.icon);
        const RewardsWidget title = host.child_00aa7e00(entry.item, "award_title_Text");
        host.text_source_00abaed0(title, std::string(kRewardsUnlockedUnitPrefix) + unlock.unit, true);
        const RewardsWidget detail = host.child_00aa7e00(entry.item, "award_detail_Text");
        host.text_cstring_00abbe50(detail, kRewardsUnlockText, true);
        rewards_award_layout(host, entry.item, entry.icon, title, detail, at);
    }

    // 006363AC..00636735: the profile's pending awards, cleared afterwards.
    if (host.profile_has_pending_awards_0061eb80()) {
        for (const RewardsPendingAward& award : host.profile_pending_awards_70c()) {
            const RewardsAwardItem entry = rewards_award_clone(host);
            if (award.icon_id == 0) {
                host.award_icon_by_name_004fc070(award.icon_name, entry.icon);
            } else {
                host.award_icon_by_id_004fb150(award.icon_id, entry.icon);
            }
            const RewardsWidget title = host.child_00aa7e00(entry.item, "award_title_Text");
            host.text_source_00abaed0(title, award.title, true);
            const RewardsWidget detail = host.child_00aa7e00(entry.item, "award_detail_Text");
            host.text_source_00abaed0(detail, award.detail, true);
            rewards_award_layout(host, entry.item, entry.icon, title, detail, at);
        }
        host.profile_clear_pending_awards_006277c0();
    }

    // 00636739..00636BF9: badges, from the commit record's key list.
    if (record.badge > 0) {
        for (const std::string& key : host.commit_record_keys_6c()) {
            if (!host.badge_award_earned_0090c000(key)) continue;
            const RewardsAwardItem entry = rewards_award_clone(host);
            const RewardsAwardDefinition award = host.award_definition(key);
            // 00449AF0 against "" (00CE3A0C), case-insensitive; the path is "" + texture.
            if (!award.texture.empty()) {
                host.set_state_texture_00ab44b0(entry.icon, 0, award.texture, kRewardsAwardTextureColor);
                host.widget_show_34(entry.icon, true);
            } else {
                host.widget_show_34(entry.icon, false);
            }
            const RewardsWidget title = host.child_00aa7e00(entry.item, "award_title_Text");
            host.text_source_00abaed0(title, award.title, true);
            const RewardsWidget detail = host.child_00aa7e00(entry.item, "award_detail_Text");
            host.text_source_00abaed0(detail, award.detail, true);
            rewards_award_layout(host, entry.item, entry.icon, title, detail, at);
        }
    }

    // 00636BFE..00636C33: the award scroller (+19Ch).
    const double below = static_cast<double>(at.y) - host.widget_size_00aa6740(host.screen_widget(0x184)).y;
    host.scroller_set_range_006834a0(0x19C, static_cast<float>(below + kRewardsAwardGap));
}

// 00636C38..00636D60
void rewards_enter_tail(RewardsEnterState& screen, RewardsEnterHost& host) {
    if (screen.input_locked258 && !screen.pages238.empty()) {
        host.widget_set_active_60(host.screen_widget(0xD4), false);
        host.apply_award_pages_0062a970();
    }
    // +1Dh: more than one of the six page flags 00E08C88..00E08C8D is set.
    std::int32_t pages = 0;
    for (const std::uint8_t flag : host.page_flags_e08c88()) {
        if (flag != 0) ++pages;
    }
    screen.field1d = pages > 1;
    host.set_frame_title_00518d60("");
    host.change_page_00622e30(0);
    host.set_help_line_0054a0c0("");
    screen.field38 = screen.score_zero18 ? -1 : 3;
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
    rewards_fill_objectives(host);
    rewards_scoring_scroll(host, rewards_fill_scoring(host));
    rewards_fill_rank(host);
    rewards_fill_awards(screen, host);
    rewards_enter_tail(screen, host);
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