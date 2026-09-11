#include "bsp/mission_map_flag_policy.hpp"

#include "bsp/mission_briefing_start.hpp"

namespace bsp {

bool mission_map_flag_visible_005c2f70(
    const MissionRecordData& record, const MissionProgress& progress)
{
    // Compose 005C2F70's record-to-id adapter with the established 0090C560
    // contract; do not use score/count/ranking or the top-level counter maps.
    const auto found = progress.mission_scores_00.find(record.screen.name);
    return found != progress.mission_scores_00.end()
        && found->second.mission_completed_00 != 0;
}

void run_return_to_mission_list_00599340(MainMenuPage current_page,
    const MainMenuScreenState& screen, MissionListRefreshHost& host)
{
    if (current_page != MainMenuPage::MissionDetail) { // 00599340..0059934A
        return;
    }

    // The native tests this byte before either selected-mission call.
    const bool dlc_campaign = screen.dlc_campaign; // 0059934C
    const auto& record = host.selected_mission_005806a0();
    const auto side = mission_side_index_005c27e0(record); // 0059935C / 80
    const MainMenuPage page = dlc_campaign
        ? (side == 0 ? MainMenuPage::CampaignUsDlc : MainMenuPage::CampaignJapanDlc)
        : (side == 0 ? MainMenuPage::CampaignUs : MainMenuPage::CampaignJapan);
    host.build_mission_list_page_00597870(page);
}

} // namespace bsp
