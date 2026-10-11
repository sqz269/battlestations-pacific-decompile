#pragma once

// Mission campaign kind read from the loaded mission path (004BFFA0), used by the
// Xbox LIVE presence refresh BSP_Game_UpdatePresenceContext (004C0170, call at
// 004C0343). Evidence: docs/CC13_WEATHER_RENDER.md ("Segment 9 triage"). The
// enumerator names are hypotheses drawn from the path fragments.

#include <cstdint>

namespace bsp {

enum class MissionCampaignKind : std::uint32_t {
    Other = 0,
    Ijn = 1,             // "/IJN/" or "/ijn/"
    Usn = 2,             // "/USN/" or "/usn/"
    TrainingGrounds = 3, // "/TrainingGrounds/" or "/traininggrounds/"
    Multiplayer = 4,     // "/Multi/" or "/multi/"
};

// 004BFFA0: __fastcall(ECX game), no stack arguments. The path is the char* at
// [[game+5FCh]+910h]; the native returns Other when game+5FCh is null and skips a
// test whose path pointer is null. Tests run in the native order (IJN, Multi,
// USN, TrainingGrounds, each upper-case form first) with case-sensitive strstr,
// so a path containing several fragments takes the first kind in that order.
MissionCampaignKind mission_campaign_kind_004bffa0(const char* mission_path) noexcept;

}  // namespace bsp
