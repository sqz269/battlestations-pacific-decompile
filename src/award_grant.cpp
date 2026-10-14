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

namespace {
void release_midway_string(MidwaySaveFolderHost& host, const MidwayNativeString& s)
{
    if (s.data != nullptr) {
        void* pool = host.pool_singleton_00419cc0();
        host.pool_return_block_00bd1510(pool, s.data, s.length + 1, 1);
    }
}
}  // namespace

// 0090C5D0..0090C738. The buffer is the 104h-byte stack array at [ESP+14h].
bool midway_save_folder_preexisted_0090c5d0(MidwaySaveFolderHost& host)
{
    char folder[0x104];
    if (host.sh_get_special_folder_path_a(folder, 5, 0) == 0) {
        return false;                                                    // 0090C5FE -> 0090C722
    }
    MidwayNativeString suffix;                                           // [ESP+8]
    host.native_string_from_cstring_0041e870(suffix, "\\Battlestations-Midway\\save");  // 00D18734
    MidwayNativeString base;                                             // [ESP+10h]
    host.native_string_from_cstring_0041e870(base, folder);
    MidwayNativeString path;                                             // [ESP+0]
    host.native_string_concat_004261a0(base, path, suffix);
    release_midway_string(host, base);                                   // 0090C644..0090C666
    release_midway_string(host, suffix);                                 // 0090C66B..0090C68D
    const char* text = path.data != nullptr ? path.data : host.empty_text_00f899a0();
    if (host.create_directory_a(text, nullptr) == 0 && host.get_last_error() == kErrorAlreadyExists) {
        release_midway_string(host, path);
        return true;                                                     // 0090C6E2 MOV AL,1
    }
    release_midway_string(host, path);
    return false;
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
