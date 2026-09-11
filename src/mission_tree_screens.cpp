#include "bsp/mission_tree_screens.hpp"

#include <cctype>

namespace bsp {
namespace {

// The native comparison is __stricmp on the two data pointers, guarded by a
// size equality test first (005C34AF: `if (*record == *name)`), so an empty
// record name only ever matches an empty argument.
bool case_insensitive_equal(std::string_view a, std::string_view b) noexcept
{
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        const auto lhs = static_cast<unsigned char>(a[i]);
        const auto rhs = static_cast<unsigned char>(b[i]);
        if (std::tolower(lhs) != std::tolower(rhs)) {
            return false;
        }
    }
    return true;
}

} // namespace

std::size_t mission_side_index(const MissionRecord& record) noexcept
{
    // XOR EDX,EDX / CMP byte ptr [ESI+0B8h],BL / SETZ DL. Only the low byte of
    // the block-0 enabled dword takes part.
    const auto low_byte = static_cast<std::uint8_t>(record.sides[0].enabled & 0xFFu);
    return low_byte == 0 ? 1u : 0u;
}

const MissionSideBlock& mission_side_block(const MissionRecord& record) noexcept
{
    return record.sides[mission_side_index(record)];
}

bool mission_opens_briefing(const MissionRecord& record) noexcept
{
    // 005C573A compares the block's briefing-key size against zero.
    return !mission_side_block(record).briefing_key.empty();
}

std::uint32_t effective_mission_difficulty(
    const MissionRecord& record, std::uint32_t player_choice) noexcept
{
    return record.difficulty == kMissionDifficultyFromPlayer ? player_choice : record.difficulty;
}

std::string_view mission_scene_name(const MissionRecord& record) noexcept
{
    // A null data pointer becomes DAT_00E1952F, the empty string.
    return record.scene;
}

MissionTreeIndex find_mission_by_name_005c3470(
    const MissionTreeScreenState& state, std::string_view name) noexcept
{
    MissionTreeIndex result{};
    for (std::size_t group = 0; group < state.groups.size(); ++group) {
        const auto& missions = state.groups[group].missions;
        for (std::size_t mission = 0; mission < missions.size(); ++mission) {
            if (case_insensitive_equal(missions[mission].name, name)) {
                // The native loop does not break, so the last match wins.
                result.mission = static_cast<std::uint32_t>(mission);
                result.group = static_cast<std::uint32_t>(group);
            }
        }
    }
    return result;
}

void apply_restored_selection_005caaf0(
    MissionTreeScreenState& state, MissionTreeIndex index) noexcept
{
    state.selected_mission = index.mission;
    state.selected_group = index.group;
    if (static_cast<std::int32_t>(state.selected_mission) < 0) {
        state.selected_mission = 0;
        state.selected_group = 0;
    }
}

const MissionRecord* selected_mission_005c3870(const MissionTreeScreenState& state) noexcept
{
    if (state.selected_group >= state.groups.size()) {
        return nullptr;
    }
    const auto& missions = state.groups[state.selected_group].missions;
    if (state.selected_mission >= missions.size()) {
        return nullptr;
    }
    return &missions[state.selected_mission];
}

const MissionRecord* mission_by_current_key_005c3870(
    const MissionTreeScreenState& state, std::string_view current_key) noexcept
{
    for (const auto& record : state.records) {
        if (case_insensitive_equal(record.name, current_key)) {
            return &record;
        }
    }
    return nullptr;
}

void set_briefing_mission_0051dce0(BriefingScreenState& briefing,
    const MissionRecord& record, std::size_t side)
{
    const MissionSideBlock& block = record.sides[side % kMissionSideBlockCount];
    briefing.title = record.title;
    briefing.word0 = record.briefing_word0;
    briefing.word1 = record.briefing_word1;
    briefing.word2 = record.briefing_word2;
    briefing.key = block.briefing_key;
    briefing.objectives_a = block.objectives_a;
    briefing.objectives_b = block.objectives_b;
}

int on_briefing_widget_activated_0051aa70(BriefingScreenState& briefing,
    int history_widget, int item_index, std::size_t item_count) noexcept
{
    if (history_widget == 0) {
        // 0051AA92: ADD dword ptr [ESI+114h], -1.
        briefing.history_page -= 1u;
        return -1;
    }
    if (history_widget == 1) {
        // 0051AAA3: ADD dword ptr [ESI+114h], 1.
        briefing.history_page += 1u;
        return -1;
    }
    if (item_index < 0 || static_cast<std::size_t>(item_index) >= item_count) {
        return -1;
    }
    return item_index;
}

std::vector<BriefingHelpEntry> build_briefing_help_line_0051b450(
    const BriefingScreenState& briefing, bool suppress_all, bool suppress_history,
    bool non_campaign_session)
{
    std::vector<BriefingHelpEntry> entries;
    if (!suppress_all && !non_campaign_session) {
        entries.push_back({kBriefingHelpPlayCode, kBriefingHelpPlayKey});
    }
    if (!suppress_all && !suppress_history && briefing.history_available) {
        entries.push_back({kBriefingHelpHistoryCode, kBriefingHelpHistoryKey});
    }
    entries.push_back({kBriefingHelpNavigateCode, kBriefingHelpNavigateKey});
    entries.push_back({kBriefingHelpBackCode, kBriefingHelpBackKey});
    return entries;
}

void request_mission_start_00439020(MissionLaunchHost& host)
{
    for (const std::uint32_t request : kMissionStartRequests) {
        host.request_state(request);
    }
}

bool start_selected_mission_005c5600(const MissionRecord& record,
    std::uint32_t player_difficulty, MissionLaunchHost& host)
{
    const std::size_t side = mission_side_index(record);
    const MissionSideBlock& block = record.sides[side];

    host.set_current_mission_key(record.name);
    // 005C5682 pushes null for the override name, so the reader picks the
    // mission's own weather descriptor.
    host.set_pending_scene(mission_scene_name(record), std::string_view{});

    const auto second_side_enabled =
        static_cast<std::uint8_t>(record.sides[1].enabled & 0xFFu) != 0;
    host.publish_loading_config(block.loading_text, record.title, second_side_enabled);

    host.set_mission_key_mirror(record.name);
    host.publish_mission_to_scoring(record, side);

    bool launched = false;
    if (block.briefing_key.empty()) {
        host.set_effective_difficulty(effective_mission_difficulty(record, player_difficulty));
        request_mission_start_00439020(host);
        launched = true;
    } else {
        host.fill_briefing_screen(record, side);
        host.push_interface_request(kInterfaceMainMenu, nullptr);
    }

    host.reset_front_end_timer();
    host.finish_launch();
    return launched;
}

bool activate_selected_mission_005c57d0(const MissionRecord& record, bool unlocked,
    std::uint32_t player_difficulty, MissionLaunchHost& host)
{
    if (!unlocked) {
        return false;
    }
    start_selected_mission_005c5600(record, player_difficulty, host);
    return true;
}

void mission_tree_update_005c4040(FrontEndScreenUpdateHost& host)
{
    if (host.action_edge_or_repeat(kFrontEndBackAction)) {
        host.push_interface_request(kInterfaceMainMenu, nullptr);
        return;
    }
    host.select_backdrop_scene(kMissionTreeBackdropScene);
}

void briefing_back_0051b890(FrontEndScreenUpdateHost& host)
{
    if (host.non_campaign_session()) {
        host.leave_to_multi_menu();
        return;
    }
    host.push_interface_request(kInterfaceMissionTree, nullptr);
}

void briefing_update_0051c7d0(FrontEndScreenUpdateHost& host)
{
    if (!host.action_edge_or_repeat(kFrontEndBackAction)) {
        return;
    }
    briefing_back_0051b890(host);
}
}
