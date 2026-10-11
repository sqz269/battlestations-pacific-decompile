// 004BFFA0 mission campaign kind. Evidence: docs/CC13_WEATHER_RENDER.md.

#include "bsp/game_mission_kind.hpp"

#include <cstring>

namespace bsp {

MissionCampaignKind mission_campaign_kind_004bffa0(const char* mission_path) noexcept {
    if (mission_path == nullptr) {
        return MissionCampaignKind::Other;
    }
    // The native also checks found - base != -1 after each strstr, which is
    // always true for a hit.
    struct Probe {
        const char* fragment;
        MissionCampaignKind kind;
    };
    static constexpr Probe kProbes[] = {
        {"/IJN/", MissionCampaignKind::Ijn},
        {"/ijn/", MissionCampaignKind::Ijn},
        {"/Multi/", MissionCampaignKind::Multiplayer},
        {"/multi/", MissionCampaignKind::Multiplayer},
        {"/USN/", MissionCampaignKind::Usn},
        {"/usn/", MissionCampaignKind::Usn},
        {"/TrainingGrounds/", MissionCampaignKind::TrainingGrounds},
        {"/traininggrounds/", MissionCampaignKind::TrainingGrounds},
    };
    for (const Probe& p : kProbes) {
        if (std::strstr(mission_path, p.fragment) != nullptr) {
            return p.kind;
        }
    }
    return MissionCampaignKind::Other;
}

}  // namespace bsp
