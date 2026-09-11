#include "bsp/main_menu_mission_detail.hpp"

// Reconstruction of 0058C010, the main menu's mission-detail page builder.
// Evidence addresses are in include/bsp/main_menu_mission_detail.hpp and in
// docs/MAIN_MENU_MISSION_DETAIL.md. Names are hypotheses, not recovered
// symbols.

namespace bsp {
namespace {

std::string map_icon_name(std::string_view prefix, int one_based_ordinal) {
    // 0058C336 and 0058C518 build the prefix, 0058C536 appends the ordinal
    // through 004263B0 and 0058C54F concatenates "_Icon" through 004261A0.
    std::string name(prefix);
    name += std::to_string(one_based_ordinal);
    name += kMissionMapIconSuffix;
    return name;
}

}  // namespace

bool mission_detail_group_is_supported(int published_group) noexcept {
    // 0058C097: ADD EAX,-1 then CMP EAX,3 / JA. The comparison is unsigned, so
    // group 0 wraps to 0FFFFFFFFh and is rejected with everything above 4.
    return published_group >= 1 && published_group <= 4;
}

MissionGroup mission_detail_group(int published_group) noexcept {
    switch (published_group) {
    case 1:
        return MissionGroup::Ijn;     // 0058C0BE: +314h, EDI = 0
    case 2:
        return MissionGroup::Usn;     // 0058C0AA: +310h, EDI = 1
    case 3:
        return MissionGroup::IjnDlc;  // 0058C0DC: +31Ch, EDI = 3
    case 4:
        return MissionGroup::UsnDlc;  // 0058C0FD: +318h, EDI = 4
    default:
        break;
    }
    // Unreachable through 0058C010; the jump table at 0058CE20 has four entries
    // and every other value has already left through 0058CE0A.
    return MissionGroup::Ijn;
}

MissionDetailCampaignFlags mission_detail_campaign_flags(int published_group) noexcept {
    MissionDetailCampaignFlags flags;
    switch (published_group) {
    case 1:  // 0058C0CA, 0058C0D1
        flags.dlc_campaign = false;
        flags.us_campaign = false;
        break;
    case 2:  // 0058C0B0 then the shared tail at 0058C10F
        flags.dlc_campaign = false;
        flags.us_campaign = true;
        break;
    case 3:  // 0058C0E8, 0058C0EF
        flags.dlc_campaign = true;
        flags.us_campaign = false;
        break;
    case 4:  // 0058C103 then 0058C10F
        flags.dlc_campaign = true;
        flags.us_campaign = true;
        break;
    default:
        break;
    }
    return flags;
}

std::array<bool, 5> mission_detail_group_visibility(
    bool us_campaign, bool dlc_campaign) noexcept {
    // 0058C70D..0058C7B2, one arm per group, in the order JP, US, JP DLC,
    // US DLC, training. Training is a literal 0 at 0058C7B0.
    std::array<bool, 5> visible{};
    visible[static_cast<std::size_t>(MissionGroup::Ijn)] = !us_campaign && !dlc_campaign;
    visible[static_cast<std::size_t>(MissionGroup::Usn)] = us_campaign && !dlc_campaign;
    visible[static_cast<std::size_t>(MissionGroup::IjnDlc)] = !us_campaign && dlc_campaign;
    visible[static_cast<std::size_t>(MissionGroup::UsnDlc)] = us_campaign && dlc_campaign;
    visible[static_cast<std::size_t>(MissionGroup::Training)] = false;
    return visible;
}

std::string mission_map_flag_widget_name(int one_based_ordinal) {
    return map_icon_name(kMissionMapFlagPrefix, one_based_ordinal);
}

std::string mission_map_point_widget_name(int one_based_ordinal) {
    return map_icon_name(kMissionMapPointPrefix, one_based_ordinal);
}

float mission_map_flag_level(bool selected) noexcept {
    // 0058C474..0058C494.
    return selected ? kMissionMapFlagSelectedLevel : kMissionMapFlagUnselectedLevel;
}

int mission_map_point_state(bool selected, bool side_mission) noexcept {
    // 0058C636..0058C645 for the selected arm and 0058C648..0058C65D for the
    // other. Both reduce to the same two-bit code.
    return (selected ? 1 : 0) | (side_mission ? 2 : 0);
}

double mission_detail_scroll_range(double text_height, float clip_height) noexcept {
    // 0058C86D: FLD [EAX+4] is the second component of the 00AA6740 size pair.
    // 0058C870 subtracts it from the x87 double already on the stack and
    // 0058C877 adds the constant, all in extended precision before the single
    // store at 0058C87D.
    return text_height - static_cast<double>(clip_height) + kMissionDetailScrollPadding;
}

std::string streamed_dialog_path(std::string_view language, std::string_view voice_key) {
    // 0058C971: "sound/messages/" + language.
    // 0058C989: that + "/streamed_dialogs/".
    // 0058C9AA: that + the record's backgroundVoice.
    std::string path(kStreamedDialogRoot);
    path += language;
    path += kStreamedDialogFolder;
    path += voice_key;
    return path;
}

MissionDetailOutcome build_mission_detail_page_0058c010(MissionDetailHost& host) {
    MissionDetailOutcome outcome;

    // 0058C037..0058C047. The audio request happens before the group is known
    // and therefore also runs for the training group.
    host.request_page_audio(host.selected_mission_name());

    // 0058C04C..0058C064.
    host.set_background_icon_state(0, 0, 1.0f);

    // 0058C066..0058C08D.
    outcome.published_group = host.read_tree_selected_group();
    outcome.published_mission = host.read_tree_selected_mission_index();
    host.publish_selection(outcome.published_group, outcome.published_mission);

    // 0058C092..0058C09D. The training group and anything unknown stop here,
    // with 00E08874 left on whatever page called in.
    if (!mission_detail_group_is_supported(outcome.published_group)) {
        outcome.built = false;
        return outcome;
    }
    outcome.built = true;
    outcome.group = mission_detail_group(outcome.published_group);
    outcome.flags = mission_detail_campaign_flags(outcome.published_group);
    host.set_active_group_widget(outcome.group);
    host.set_campaign_flags(outcome.flags);

    // 0058C11C..0058C190. The widget for the mission that is selected right
    // now, looked up non-recursively and cached in +330h.
    host.bind_selected_map_point(
        mission_map_point_widget_name(outcome.published_mission + 1));

    // 0058C235..0058C26F. The emptiness of the point list is latched before the
    // loop runs, so a list that the loop fills is still treated as empty for
    // every iteration.
    const bool fill_point_list = host.map_point_list_size(outcome.group) == 0;
    outcome.filled_point_list = fill_point_list;

    // 0058C274..0058C6DB.
    const std::size_t missions = host.group_mission_count();
    const std::array<float, 3> backdrop = host.backdrop_position();
    for (std::size_t mission = 0; mission < missions; ++mission) {
        const int ordinal = static_cast<int>(mission) + 1;
        const bool selected = ordinal == outcome.published_mission + 1;

        // 0058C301..0058C472.
        const std::string flag_name = mission_map_flag_widget_name(ordinal);
        host.set_map_flag(flag_name, host.map_flag_visible(mission),
            mission_map_flag_level(selected));

        // 0058C4AB..0058C660.
        const std::string point_name = mission_map_point_widget_name(ordinal);
        host.set_map_point(point_name,
            mission_map_point_state(selected, host.mission_is_side_mission(mission)),
            kMissionMapPointStateArg, kMissionMapPointBlend);

        // 0058C662..0058C6B3.
        if (fill_point_list) {
            const std::array<float, 3> position = host.map_point_position(point_name);
            host.append_map_point(outcome.group,
                {position[0] - backdrop[0], position[1] - backdrop[1],
                    position[2] - backdrop[2]});
        }
        ++outcome.missions_visited;
    }

    // 0058C6E0..0058C7F2, in source order.
    host.set_widget_visible(0x2F0, true);
    host.set_widget_visible(0x328, true);
    host.set_widget_visible(0x32C, true);
    const std::array<bool, 5> group_visible =
        mission_detail_group_visibility(outcome.flags.us_campaign, outcome.flags.dlc_campaign);
    host.set_widget_visible(0x314, group_visible[static_cast<std::size_t>(MissionGroup::Ijn)]);
    host.set_widget_visible(0x310, group_visible[static_cast<std::size_t>(MissionGroup::Usn)]);
    host.set_widget_visible(0x31C, group_visible[static_cast<std::size_t>(MissionGroup::IjnDlc)]);
    host.set_widget_visible(0x318, group_visible[static_cast<std::size_t>(MissionGroup::UsnDlc)]);
    host.set_widget_visible(0x320, group_visible[static_cast<std::size_t>(MissionGroup::Training)]);
    host.set_widget_visible(0x324, false);
    host.set_widget_visible(0x2EC, false);  // 0058C7C3 skips the call when null
    host.set_widget_visible(0x244, false);
    host.set_widget_visible(0x248, false);

    // 0058C7F4 and 0058C7FB.
    host.commit_page_state();
    host.set_widget_visible(0x344, true);

    // 0058C80A..0058C83F.
    host.set_briefing_text(host.selected_background_key());

    // 0058C844..0058C888.
    host.reset_scroller();
    host.set_scroll_range(
        mission_detail_scroll_range(host.briefing_text_height(), host.text_clip_height()));

    // 0058C88D..0058C8AB.
    host.start_preview_movie(host.selected_background_movie_key());

    // 0058C8B0..0058CAB5.
    if (!host.page_audio_suppressed()) {
        host.request_streamed_dialog(
            streamed_dialog_path(host.audio_language_folder(),
                host.selected_background_voice_key()),
            kStreamedDialogCallback);
        outcome.requested_dialog = true;
    }

    // 0058CAC3. The page word is set before the footer is built, so a failure
    // in 0054B530 would still leave the page on MissionDetail.
    outcome.page = MainMenuPage::MissionDetail;
    host.set_page(outcome.page);

    // 0058CABA..0058CC1F and 0058CD5D..0058CD73.
    host.set_footer_commands(kMissionDetailCommands);

    // 0058CCE8..0058CD26.
    host.set_widget_visible(kMissionDetailUnnamedHideA, false);
    host.set_widget_visible(0x1B8, false);
    host.set_widget_visible(kMissionDetailUnnamedHideB, false);
    host.set_widget_visible(0x1B8, false);  // 0058CD12 repeats the call verbatim
    host.clear_list_box();
    host.clear_help_line();

    // 0058CD9E..0058CE05.
    host.set_list_box_flags(true, true);
    host.move_list_box(host.list_box_position());
    host.finish_list_box();

    return outcome;
}

}  // namespace bsp
