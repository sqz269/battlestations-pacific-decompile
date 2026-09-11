#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "bsp/main_menu_screens.hpp"

// The mission-tree screen (00E198AC+5Ch, constructor 005CA880, INTF_MISSIONTREE)
// and the briefing screen (00E198AC+64h, constructor 0051E4D0, INTF_BRIEFING),
// and the path they form from the campaign list into a running mission.
// docs/MISSION_TREE_BRIEFING_SCREENS.md carries the evidence.
//
// The class table itself lives in bsp/main_menu_screens.hpp; nothing here
// repeats it. What this header adds is the data model behind the mission tree,
// the selection rules, the briefing screen's field map, and the request
// sequence a mission start emits.
//
// None of these structs is binary compatible. Offsets are recorded as comments
// and as the k*Offset constants; the C++ members are ordinary values.

namespace bsp {

// ---------------------------------------------------------------------------
// Field offsets
// ---------------------------------------------------------------------------

// Mission-tree screen, 34h bytes. Both vectors have the MSVC iterator-debugging
// shape: a _Myproxy dword, then first/last/end. The constructor zeroes exactly
// the two first/last/end triples.
inline constexpr std::size_t kMissionTreeSelectedMissionOffset = 0x0C;
inline constexpr std::size_t kMissionTreeSelectedGroupOffset = 0x10;
inline constexpr std::size_t kMissionTreeGroupVectorOffset = 0x14;  // stride 34h
inline constexpr std::size_t kMissionTreeRecordVectorOffset = 0x24; // stride 434h

// Group entry, 34h bytes. Only the nested mission vector is recovered.
inline constexpr std::size_t kMissionGroupSizeBytes = 0x34;
inline constexpr std::size_t kMissionGroupMissionVectorOffset = 0x24;

// Mission record, 434h bytes.
inline constexpr std::size_t kMissionRecordSizeBytes = 0x434;
inline constexpr std::size_t kMissionRecordNameOffset = 0x00;
inline constexpr std::size_t kMissionRecordTitleOffset = 0x08;
inline constexpr std::size_t kMissionRecordSceneOffset = 0x20;
inline constexpr std::size_t kMissionRecordBriefingWord0Offset = 0x60;
inline constexpr std::size_t kMissionRecordBriefingWord1Offset = 0x64;
inline constexpr std::size_t kMissionRecordBriefingWord2Offset = 0x68;
inline constexpr std::size_t kMissionRecordUnlockListOffset = 0x78;
inline constexpr std::size_t kMissionRecordDifficultyOffset = 0xB4;
inline constexpr std::size_t kMissionRecordSideBlockOffset = 0xB8;

// Side block, 154h bytes, two per record at +0B8h and +20Ch.
inline constexpr std::size_t kMissionSideBlockSizeBytes = 0x154;
inline constexpr std::size_t kMissionSideBlockCount = 2;
inline constexpr std::size_t kMissionSideEnabledOffset = 0x00;
inline constexpr std::size_t kMissionSideBriefingKeyOffset = 0x04;
inline constexpr std::size_t kMissionSideObjectivesAOffset = 0x14;
inline constexpr std::size_t kMissionSideObjectivesBOffset = 0x24;
inline constexpr std::size_t kMissionSideLoadingTextOffset = 0x64;

// Briefing screen, 138h bytes.
inline constexpr std::size_t kBriefingListboxPageOffset = 0x10;   // FE_briefing_listbox
inline constexpr std::size_t kBriefingGridPageOffset = 0x14;      // FE_briefing_grid
inline constexpr std::size_t kBriefingMainListboxOffset = 0x18;   // Main_Listbox
inline constexpr std::size_t kBriefingListboxTextOffset = 0x1C;   // MainListbox_Text
inline constexpr std::size_t kBriefingActiveKeyOffset = 0x28;
inline constexpr std::size_t kBriefingDifficultyWidget0Offset = 0x9C;
inline constexpr std::size_t kBriefingDifficultyWidget1Offset = 0xA0;
inline constexpr std::size_t kBriefingDifficultyWidget2Offset = 0xA4;
inline constexpr std::size_t kBriefingItemWidgetVectorOffset = 0xAC;
inline constexpr std::size_t kBriefingTitleOffset = 0xC4;
inline constexpr std::size_t kBriefingWord0Offset = 0xCC;
inline constexpr std::size_t kBriefingWord1Offset = 0xD0;
inline constexpr std::size_t kBriefingWord2Offset = 0xD4;
inline constexpr std::size_t kBriefingKeyOffset = 0xD8;
inline constexpr std::size_t kBriefingHistoryPrevWidgetOffset = 0xFC;
inline constexpr std::size_t kBriefingHistoryNextWidgetOffset = 0x100;
inline constexpr std::size_t kBriefingHistoryAvailableOffset = 0x104;
inline constexpr std::size_t kBriefingHistoryPageOffset = 0x114;
inline constexpr std::size_t kBriefingListHeadOffset = 0x130;

// Game fields the launch path writes, all relative to 00E188A8.
inline constexpr std::size_t kGameCurrentMissionKeyOffset = 0x2198; // 005C56C9
inline constexpr std::size_t kGameEffectiveDifficultyOffset = 0x6AC; // 005C577A
inline constexpr std::size_t kGameChosenDifficultyOffset = 0x6B0;    // 0051B7B0
inline constexpr std::size_t kGameMissionKeyMirrorOffset = 0x6B8;    // 005C5705
inline constexpr std::size_t kGameProfileBlockOffset = 0x650;        // 005C57E1
inline constexpr std::size_t kGameNonCampaignFlagOffset = 0x1FE4;    // 005C3870

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

// The backdrop scene index the mission-tree update keeps alive through
// 00427190, the SCRIPTS/datatables/MPakScenes.lua selector (005C4065).
inline constexpr int kMissionTreeBackdropScene = 3;

// record+0B4h. 3 means "the player picked it on the briefing"; any other value
// is the mission's own fixed difficulty (005C576F).
inline constexpr std::uint32_t kMissionDifficultyFromPlayer = 3;

// The two state requests 00439020 enqueues, in order. Request 6 runs the
// manager virtual +0Ch and request 0Ah runs 004DFB70, which begins the loading
// screen. The drain is FIFO within one pass.
inline constexpr std::uint32_t kRequestApplyPendingInterface = 0x06;
inline constexpr std::uint32_t kRequestLoadScene = 0x0A;
inline constexpr std::array<std::uint32_t, 2> kMissionStartRequests{
    kRequestApplyPendingInterface, kRequestLoadScene};

// Help-line control codes 0051B450 publishes through 0054B530, in push order.
inline constexpr int kBriefingHelpPlayCode = 0xA2;
inline constexpr int kBriefingHelpHistoryCode = 0xA7;
inline constexpr int kBriefingHelpNavigateCode = 0xAF;
inline constexpr int kBriefingHelpBackCode = 0xA3;
inline constexpr std::string_view kBriefingHelpPlayKey = "FE.briefing_play";
inline constexpr std::string_view kBriefingHelpHistoryKey = "FE.briefing_history";
inline constexpr std::string_view kBriefingHelpNavigateKey = "FE_xbox.bhelp_navigate";
inline constexpr std::string_view kBriefingHelpBackKey = "globals.back";

// The GUI pages 0051E280 binds, in bind order.
inline constexpr std::string_view kBriefingListboxPage = "FE_briefing_listbox";
inline constexpr std::string_view kBriefingGridPage = "FE_briefing_grid";
inline constexpr std::string_view kBriefingMainListboxWidget = "Main_Listbox";
inline constexpr std::string_view kBriefingListboxTextWidget = "MainListbox_Text";

// The multiplayer mode groups 0051CDE0 hides while DAT_00E18D91 is clear, in
// the order the enter virtual names them.
inline constexpr std::array<std::string_view, 6> kBriefingModeGroups{
    "Duel_Group", "Escort_Group", "Siege_Group", "Competitive_Group",
    "Island_Capture_Group", "BG_01_Group"};

// ---------------------------------------------------------------------------
// The recovered data model
// ---------------------------------------------------------------------------

// One 154h side block. Only the fields a call site touches are modelled.
struct MissionSideBlock {
    std::uint32_t enabled{};        // +00h, read as a byte at 005C572A
    std::string briefing_key;       // +04h; empty means "no briefing screen"
    std::vector<std::string> objectives_a; // +14h
    std::vector<std::string> objectives_b; // +24h
    std::vector<std::string> loading_text; // +64h, published by 0057D060
};

// One 434h mission record.
struct MissionRecord {
    std::string name;   // +00h, the key 005C3870's non-campaign arm matches on
    std::string title;  // +08h
    std::string scene;  // +20h, handed to 004E2770; empty is legal
    std::uint32_t briefing_word0{}; // +60h
    std::uint32_t briefing_word1{}; // +64h
    std::uint32_t briefing_word2{}; // +68h
    std::vector<std::string> unlock_requirements; // +78h
    std::uint32_t difficulty{};                   // +0B4h
    std::array<MissionSideBlock, kMissionSideBlockCount> sides{}; // +0B8h, +20Ch
};

// One 34h group entry.
struct MissionGroupRecord {
    std::vector<MissionRecord> missions; // group+28h..+2Ch, stride 434h
};

// The mission-tree screen's own state.
struct MissionTreeScreenState {
    std::vector<MissionGroupRecord> groups;   // +18h..+1Ch, stride 34h
    std::vector<MissionRecord> records; // +28h..+2Ch, stride 434h
    std::uint32_t selected_mission{};   // +0Ch
    std::uint32_t selected_group{};     // +10h
};

// The briefing screen's mission-facing state, the part 0051DCE0 writes.
struct BriefingScreenState {
    std::string title;   // +0C4h
    std::string key;     // +0D8h
    std::uint32_t word0{}; // +0CCh
    std::uint32_t word1{}; // +0D0h
    std::uint32_t word2{}; // +0D4h
    std::vector<std::string> objectives_a; // from side block +14h
    std::vector<std::string> objectives_b; // from side block +24h
    std::uint32_t history_page{};   // +114h
    bool history_available{false};  // +104h
};

// The result of 005C3470: the pair it writes into its out parameter.
struct MissionTreeIndex {
    std::uint32_t mission{0xFFFFFFFFu};
    std::uint32_t group{0};
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 005C572A / 005C5749: side = (record+0B8h == 0) ? 1 : 0. Block 0's enabled
// dword picks the block, which is why the record is asked twice.
std::size_t mission_side_index(const MissionRecord& record) noexcept;
const MissionSideBlock& mission_side_block(const MissionRecord& record) noexcept;

// 005C573A: a mission opens the briefing exactly when the selected side block's
// briefing key is non-empty; otherwise it launches straight away.
bool mission_opens_briefing(const MissionRecord& record) noexcept;

// 005C5769..005C5793: the value written into game+6ACh.
std::uint32_t effective_mission_difficulty(
    const MissionRecord& record, std::uint32_t player_choice) noexcept;

// 005C5600 step 4: the scene name handed to 004E2770. A null data pointer
// becomes DAT_00E1952F, the empty string, so an absent scene is not an error.
std::string_view mission_scene_name(const MissionRecord& record) noexcept;

// 005C3470, __thiscall(this, pair* out, const string* name) returning out. Walks
// every group and every mission, compares the record name case-insensitively,
// and keeps the last match. Returns {FFFFFFFFh, 0} when nothing matches.
MissionTreeIndex find_mission_by_name_005c3470(
    const MissionTreeScreenState& state, std::string_view name) noexcept;

// The tail of 005CAAF0: a negative mission index resets both indices to zero,
// so an unknown saved mission falls back to the first mission of the first
// group. `index.mission` is compared as a signed int, exactly as the native
// `if (*(int *)(this+0Ch) < 0)` does.
void apply_restored_selection_005caaf0(
    MissionTreeScreenState& state, MissionTreeIndex index) noexcept;

// 005C3870, campaign arm (game+1FE4h == 0): groups[selected_group]
// .missions[selected_mission], bound-checked. Returns nullptr where the native
// code would trip the iterator-debug check at 00BF6713.
const MissionRecord* selected_mission_005c3870(
    const MissionTreeScreenState& state) noexcept;

// 005C3870, non-campaign arm: a linear scan of the flat record vector for the
// record whose name equals the key at game+2198h. The native comparison is
// __stricmp, and it returns the first match.
const MissionRecord* mission_by_current_key_005c3870(
    const MissionTreeScreenState& state, std::string_view current_key) noexcept;

// 0051DCE0, __thiscall(this, record, side), RET 8.
void set_briefing_mission_0051dce0(BriefingScreenState& briefing,
    const MissionRecord& record, std::size_t side);

// 0051AA70, the widget-activated handler of the 00CEC914 base. Returns the
// list-box index the native code selects through 00A9C7C0, or -1 when the
// widget was one of the two history arrows (which step +114h instead) or was
// not in the handle vector. `history_widget` is -1 for neither arrow, 0 for
// +0FCh and 1 for +100h.
int on_briefing_widget_activated_0051aa70(BriefingScreenState& briefing,
    int history_widget, int item_index, std::size_t item_count) noexcept;

// One help-line entry as 0051B450 pushes it into 0054B530.
struct BriefingHelpEntry {
    int control_code{};
    std::string_view locale_key;
};

// 0051B450, __thiscall(this, bool suppress_all, bool suppress_history), RET 8.
// `non_campaign_session` is game+1FE4h. Play appears only when nothing is
// suppressed and the session flag is clear; history needs +104h as well.
std::vector<BriefingHelpEntry> build_briefing_help_line_0051b450(
    const BriefingScreenState& briefing, bool suppress_all, bool suppress_history,
    bool non_campaign_session);

// ---------------------------------------------------------------------------
// The launch boundary
// ---------------------------------------------------------------------------

// Integration boundary for 005C5600, one method per native call site in the
// order the listing runs them. There are no default implementations; nothing
// here stands in for unrecovered behaviour.
struct MissionLaunchHost {
    virtual ~MissionLaunchHost() = default;
    // 005C56C9 and 005C5705: the mission key copied into game+2198h and
    // game+6B8h. Two separate strings in the native object.
    virtual void set_current_mission_key(std::string_view key) = 0;
    virtual void set_mission_key_mirror(std::string_view key) = 0;
    // 004E2770, ECX = game: clears game+5FCh and the string at +600h, then
    // 004E1D70(scene, 0, -1, weather_override) and 004C6890(0). The second
    // stack argument is the weather-descriptor override name forwarded to
    // 0046DF00 argument 5, not a flag word; 005C5682 pushes null for it, so a
    // campaign mission lets the .scn reader pick its own descriptor. Corrected
    // from `int flags` per docs/MISSION_LOAD_PATH.md.
    virtual void set_pending_scene(
        std::string_view scene, std::string_view weather_override) = 0;
    // 0057D060 -> BSP_LoadingScreen_PublishConfig (0057CFF0).
    virtual void publish_loading_config(const std::vector<std::string>& text,
        std::string_view title, bool second_side_enabled) = 0;
    // 00626930, __fastcall(ECX = record, EDX = side).
    virtual void publish_mission_to_scoring(const MissionRecord& record, std::size_t side) = 0;
    // 005C577A / 005C578D: game+6ACh.
    virtual void set_effective_difficulty(std::uint32_t difficulty) = 0;
    // 00439020: BSP_Game_RequestState(6) then BSP_Game_RequestState(0Ah).
    virtual void request_state(std::uint32_t request) = 0;
    // 0051DCE0 on [00E198AC]+64h.
    virtual void fill_briefing_screen(const MissionRecord& record, std::size_t side) = 0;
    // 004CC460, ECX = [00E198AC]. The literal at 005C5760 is 1, kInterfaceMainMenu.
    virtual void push_interface_request(int interface_id, void* payload) = 0;
    // 005C5798..005C57B6: 004BEC00 then 00A92C40 with 0.0f, then 004D2A80.
    virtual void reset_front_end_timer() = 0;
    virtual void finish_launch() = 0;
};

// 00439020, __cdecl(void), RET. The two-request mission-start pair.
void request_mission_start_00439020(MissionLaunchHost& host);

// 005C5600, __thiscall(this = the mission-tree screen), RET. Returns true when
// it took the launch arm and false when it took the briefing arm.
bool start_selected_mission_005c5600(const MissionRecord& record,
    std::uint32_t player_difficulty, MissionLaunchHost& host);

// 005C57D0, __fastcall(this), RET. `unlocked` is the result of
// 007FC820(ECX = game+650h, record+78h): a locked mission does nothing.
bool activate_selected_mission_005c57d0(const MissionRecord& record, bool unlocked,
    std::uint32_t player_difficulty, MissionLaunchHost& host);

// ---------------------------------------------------------------------------
// The two update virtuals
// ---------------------------------------------------------------------------

// What one update pass asks of the front end. Both updates run the same
// edge-or-repeat test on action 4Bh through 004D92B0.
inline constexpr int kFrontEndBackAction = 0x4B;

struct FrontEndScreenUpdateHost {
    virtual ~FrontEndScreenUpdateHost() = default;
    // 004D92B0, ECX = 00E188A8.
    virtual bool action_edge_or_repeat(int action) = 0;
    // 004CC460, ECX = [00E198AC].
    virtual void push_interface_request(int interface_id, void* payload) = 0;
    // game+1FE4h.
    virtual bool non_campaign_session() = 0;
    // 006881F0, ECX = [00E198B4], the multi-menu manager.
    virtual void leave_to_multi_menu() = 0;
    // 00427190, ECX = 004C1E90's singleton: the MPakScenes.lua backdrop.
    virtual void select_backdrop_scene(int scene_index) = 0;
};

// 005C4040, __thiscall(this, float), RET 4. Back requests INTF_MAINMENU;
// otherwise the backdrop scene is re-selected every frame.
void mission_tree_update_005c4040(FrontEndScreenUpdateHost& host);

// 0051C7D0, __thiscall(this, float), RET 4, and 0051B890, its back action.
// Back leaves to the multi-menu manager in a non-campaign session and requests
// INTF_MISSIONTREE otherwise.
void briefing_back_0051b890(FrontEndScreenUpdateHost& host);
void briefing_update_0051c7d0(FrontEndScreenUpdateHost& host);
}
