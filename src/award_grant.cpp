#include "bsp/award_grant.hpp"

namespace bsp {
namespace {

// 00443d00 compares two NativeStrings case-insensitively, which is what orders
// the registry map. Only ASCII letters are folded, matching the CRT compare the
// native uses for these table keys.
char fold(char c) noexcept
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

bool equal_ignoring_case(const char* a, const char* b) noexcept
{
    if (a == nullptr || b == nullptr) {
        return a == b;
    }
    while (*a != '\0' && fold(*a) == fold(*b)) {
        ++a;
        ++b;
    }
    return fold(*a) == fold(*b);
}

} // namespace

// Row keys and resolved "XLastAchievementID" values of
// Scripts\datatables\Achievements.lua, ascending by id. The ids are dense from
// 27 to 78 with no repeats.
const AwardNameId kAwardNameIds[52] = {
    {"RUA_DU", 27},
    {"RUA_CU", 28},
    {"RUA_BU", 29},
    {"RUA_MU", 30},
    {"RUA_SU", 31},
    {"RUA_FU", 32},
    {"RUA_TBU", 33},
    {"RUA_DBU", 34},
    {"RUA_LBU", 35},
    {"GA_SL", 36},
    {"GA_AE", 37},
    {"RUA_KU", 38},
    {"MA_SA", 39},
    {"MA_TP", 40},
    {"MA_PS", 41},
    {"MA_FD", 42},
    {"MA_QS", 43},
    {"MA_MT", 44},
    {"MA_YC", 45},
    {"SA_PM", 46},
    {"SA_IM", 47},
    {"SA_UH", 48},
    {"SA_TK", 49},
    {"SA_OC", 50},
    {"AO_MOH", 51},
    {"CA_USV", 52},
    {"CA_USE", 53},
    {"CA_JPV", 54},
    {"CA_JPE", 55},
    {"CA_HOS", 56},
    {"CA_WOG", 57},
    {"CA_AA", 58},
    {"CA_FA", 59},
    {"CA_CJM", 60},
    {"CA_CUM", 61},
    {"GA_SH", 62},
    {"CA_OF", 63},
    {"CA_SA", 64},
    {"CA_IC", 65},
    {"CA_VR", 66},
    {"RANK_FR", 67},
    {"RANK_SR", 68},
    {"GA_DN", 69},
    {"GA_AI", 70},
    {"GA_DB", 71},
    {"GA_PL", 72},
    {"GA_AO", 73},
    {"GA_WY", 74},
    {"GA_MP", 75},
    {"GA_HM", 76},
    {"MA_LM", 77},
    {"MA_CS", 78},
};

int award_id_for_name_006b8da0(const char* name) noexcept
{
    if (name == nullptr) {
        return 0;
    }
    for (std::size_t i = 0; i < kAwardNameIdCount; ++i) {
        if (equal_ignoring_case(kAwardNameIds[i].name, name)) {
            return kAwardNameIds[i].achievement_id;
        }
    }
    // The native returns 0 for a name the map does not hold. A row that carries
    // no "XLastAchievementID" is present in the map but answers -1; either way
    // the range check below rejects it, so the table omits those rows.
    return 0;
}

bool is_grantable_award_id(int achievement_id) noexcept
{
    return achievement_id >= kAwardIdMin && achievement_id <= kAwardIdMax;
}

int selected_slot_state_00a3ead0(const OnlineSignInState& state) noexcept
{
    if (state.user_index < 0
        || static_cast<std::size_t>(state.user_index) >= kAwardSignInSlotCount) {
        return kSignedOut;
    }
    return state.slot_state[state.user_index];
}

bool live_enabled_account_00a3e520(const OnlineSignInState& state) noexcept
{
    return state.live_enabled;
}

bool signed_into_live_004b44f0(const OnlineSignInState& state) noexcept
{
    return state.user_selected && selected_slot_state_00a3ead0(state) == kSignedInToLive;
}

bool may_queue_award_00a410a0(const OnlineSignInState& state) noexcept
{
    if (!state.user_selected) {
        return false;
    }
    const int slot = selected_slot_state_00a3ead0(state);
    if (slot != kSignedInToLive && slot != kSignedInLocally) {
        return false;
    }
    return state.live_enabled;
}

bool midway_save_folder_preexisted_0090c5d0(bool create_directory_succeeded,
    std::uint32_t last_error) noexcept
{
    // SHGetSpecialFolderPathA failing leaves the routine on its return 0 path,
    // which a caller models by reporting a successful create it never made.
    return !create_directory_succeeded && last_error == kErrorAlreadyExists;
}

bool should_grant_award_004e4310(const AwardGrantInputs& in) noexcept
{
    if (!in.midway_save_folder_preexisted) {
        return false;
    }
    if (!is_grantable_award_id(in.achievement_id)) {
        return false;
    }
    if (!live_enabled_account_00a3e520(in.online)) {
        return false;
    }
    return signed_into_live_004b44f0(in.online);
}

bool grant_award_if_earned(AwardGrantHost& host, const char* name, int count) noexcept
{
    if (!host.midway_save_folder_preexisted()) {
        return false;
    }
    const int achievement_id = host.award_id_for_name(name);
    if (!is_grantable_award_id(achievement_id)) {
        return false;
    }
    if (!host.live_enabled_account()) {
        return false;
    }
    if (!host.signed_into_live()) {
        return false;
    }
    host.queue_online_award(achievement_id);
    host.record_local_award(name, count);
    return true;
}

} // namespace bsp
