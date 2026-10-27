// In-game GUI pieces of segments 26 and 27 (packet cc13_ingame_gui).
//
// Addresses: 0065F170, 0065F2C0, 0065F410, 0065F560, 0065F6B0, 0065F800,
// 0065F950, 0065FAA0 (the support-manager screen's eight GUI sounds).
// docs/CC13_INGAME_GUI.md carries the evidence.
//
// New C++ interfaces, not drop-in binary replacements; offsets are comments.

#ifndef BSP_INGAME_GUI_HPP
#define BSP_INGAME_GUI_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// Support-manager GUI sounds
// ---------------------------------------------------------------------------

// The support-manager screen (HUD slot 4Eh, register 0066EA60) acquires eight
// samples into +10h..+2Ch (0066EC7B..0066F27F). Each player below is
// __fastcall(ECX = that screen), RET: on first use it caches the sound class
// "InGameGUI" (00A7ACF0) behind its own guard bit, then creates a tracked
// sound of type "Normal" (00A7B0A0, 00A7E490 on *00F8BBD8, flag 1) from its
// sample and releases the returned handle at once (InterlockedDecrement on
// +4h, slot 0 at zero). The eight bodies are identical apart from the three
// addresses in each row.
struct SupportManagerSound {
    std::uint32_t player;          // the routine
    std::uint16_t sample_field;    // screen+N
    std::string_view sample;       // the .fsb 0066EA60 loads into that field
    std::uint32_t guard_global;    // bit 0 set once the class is cached
    std::uint32_t class_global;    // the cached class index
};

inline constexpr std::string_view kSupportManagerSoundClass = "InGameGUI";
inline constexpr std::string_view kSupportManagerSoundType = "Normal";

inline constexpr std::array<SupportManagerSound, 8> kSupportManagerSounds{{
    {0x0065F170, 0x10, "environment/environment_unavailable.fsb", 0x00E1983C, 0x00E19838},
    {0x0065F2C0, 0x14, "environment/sm_sp_to_unit.fsb", 0x00E19844, 0x00E19840},
    {0x0065F410, 0x18, "environment/sm_change_weap.fsb", 0x00E1984C, 0x00E19848},
    {0x0065F6B0, 0x1C, "environment/sm_unit_to_unit.fsb", 0x00E1985C, 0x00E19858},
    {0x0065F560, 0x20, "environment/sm_launch_unit.fsb", 0x00E19854, 0x00E19850},
    {0x0065F800, 0x24, "environment/sm_left_right.fsb", 0x00E19864, 0x00E19860},
    {0x0065F950, 0x28, "environment/sm_open.fsb", 0x00E1986C, 0x00E19868},
    {0x0065FAA0, 0x2C, "environment/sm_close.fsb", 0x00E19874, 0x00E19870},
}};

const SupportManagerSound* support_manager_sound_for_player(std::uint32_t player) noexcept;

struct SupportManagerSoundHost {
    virtual ~SupportManagerSoundHost() = default;
    virtual bool class_cached(std::uint32_t guard_global) = 0;
    virtual void cache_class(std::uint32_t guard_global, std::uint32_t class_global,
        std::int32_t sound_class) = 0;
    virtual std::int32_t cached_class(std::uint32_t class_global) = 0;
    virtual std::int32_t find_sound_class_00a7acf0(std::string_view name) = 0;
    virtual std::int64_t find_sound_type_00a7b0a0(std::string_view name) = 0;  // EDX:EAX
    // 00A7E490 on *00F8BBD8: (out handle, sample, class, type, flag 1).
    virtual void* create_tracked_sound_00a7e490(std::uint16_t sample_field,
        std::int32_t sound_class, std::int64_t sound_type, bool flag) = 0;
    virtual void release_handle(void* handle) = 0;
};

void play_support_manager_sound(const SupportManagerSound& sound, SupportManagerSoundHost& host);

// ---------------------------------------------------------------------------
// HUD marker distance text (0063E5C0)
// ---------------------------------------------------------------------------

// 0063E5C0: stdcall(NativeString* out, entity* target), RET 8. Callers 00642040
// BSP_InGameHudMarkers_BuildUnitMarker and 00641910. `out` is cleared first;
// with a controlled unit (00E188D8) and a target, the distance is
// BSP_Vector3_LengthFloatThreshold (0042B2F0) of target+FCh - unit+FCh after
// both pose refreshes. A distance equal to 0.0f (or no unit/target) leaves
// `out` empty. Otherwise it is divided by 100.0 (kilometres) or by 185.2
// (nautical miles, when 00F88988 is set), stored to float, split into
// tens.digit of that tenth-unit count with a round-up when the remainder is
// above 0.5, and formatted "<tens>.<digit> |<unit key>".
inline constexpr double kDistanceTenthKilometre = 100.0;    // 00D7A220
inline constexpr double kDistanceTenthNauticalMile = 185.2; // 00CF0B08
inline constexpr std::string_view kDistanceKilometreKey = "globals.kilometer";  // 00CF5900
inline constexpr std::string_view kDistanceMileKey = "globals.mile";            // 00CF5914

std::string format_marker_distance_0063e5c0(float distance, bool nautical_miles);

// The front half, 0063E5D8..0063E68E: the distance itself. The camera unit is
// read once (0063E60D, kept in EDI); each entity whose byte +C8h is clear gets
// 00414DB0 (ECX = entity), camera first. The difference is formed per axis in
// float (FSUB/FSTP, 0063E63D..0063E66D) and its length taken by 0042B2F0
// (ECX = &vec, result in ST0, stored to float at 0063E68E).
struct MarkerDistanceHost {
    virtual ~MarkerDistanceHost() = default;
    virtual void* controlled_unit_00e188d8() = 0;        // [00E188D8]
    virtual bool pose_current_c8(void* entity) = 0;      // byte [entity + C8h] != 0
    virtual void refresh_pose_00414db0(void* entity) = 0;
    virtual void position_fc(void* entity, float out[3]) = 0; // entity + FCh/100h/104h
    virtual float vector_length_0042b2f0(const float vec[3]) = 0;
    virtual bool nautical_miles_00f88988() = 0;          // byte [00F88988]
};

// 0063E5C0 whole: the distance (0.0f without a camera unit or a target), then
// the formatter above. `out` cleared at 0063E5E5..0063E60A is the empty return.
std::string format_marker_distance_0063e5c0(void* target, MarkerDistanceHost& host);

// ---------------------------------------------------------------------------
// Device-class icon index (0065EA30)
// ---------------------------------------------------------------------------

// 0065EA30: stdcall(const char* name), RET 4, EAX. No reference to it in the
// image. Case-insensitive compares in this order (the first through _stricmp
// on the copied string, the rest through 00425850); an unknown name logs "%s"
// through 004254B0 and answers 7.
struct DeviceClassIcon {
    std::string_view name;
    std::int32_t index;
};
inline constexpr std::array<DeviceClassIcon, 20> kDeviceClassIcons{{
    {"globals.deviceclass_450lbsbomb", 2}, {"globals.deviceclass_200lbsbomb", 1},
    {"globals.deviceclass_100lbsbomb", 0}, {"globals.deviceclass_1000lbsbomb", 0x0D},
    {"globals.deviceclass_usdc", 3}, {"globals.deviceclass_japdc", 3},
    {"globals.deviceclass_5inch_HVAR", 4}, {"globals.deviceclass_R4M_AA", 0x10},
    {"globals.deviceclass_tinytimrocket", 0x0E}, {"globals.deviceclass_fidotorp", 0x0F},
    {"globals.deviceclass_usaetorp", 5}, {"globals.deviceclass_ukaetorp", 5},
    {"globals.deviceclass_japaetorp", 5}, {"us_Paratrooper_lockitid", 6},
    {"jp_Paratrooper_lockitid", 6}, {"globals.deviceclass_500kgbomb", 0x0C},
    {"globals.deviceclass_800kgbomb", 0x0C}, {"globals.deviceclass_100kgbomb", 0x0A},
    {"globals.deviceclass_250kgbomb", 0x0B}, {"globals.unitclass_kamikaze", 9},
}};
inline constexpr std::int32_t kDeviceClassIconUnknown = 7;

std::int32_t device_class_icon_index_0065ea30(std::string_view name) noexcept;

// ---------------------------------------------------------------------------
// Rewards (FE_scoring) update, 0062B640
// ---------------------------------------------------------------------------

// 0062B640: slot +20h of vtable 00CF50CC, __thiscall(screen, float seconds),
// RET 4. The argument is overwritten before use: the step is the game's frame
// time *(00E188A8)+21ECh when it is below 0.03 (double 00CEB690), else 0.03
// (float 00CE3DB8). Page numbers follow the order 00629D10 names the pages in
// (1 multiplayer page, 4 rank page); that mapping is provisional.
inline constexpr float kRewardsMaxStep = 0.03F;            // 00CE3DB8 / 00CEB690
inline constexpr double kRewardsRankGrowRate = 0.25;      // 00D7A348
inline constexpr float kRewardsRankAlphaMax = 1.0F;       // 00D7A24C
inline constexpr float kRewardsArrowFlashSeconds = 0.1F;  // 00D7A2F0
inline constexpr std::int32_t kRewardsActionLeave = 0x4A;
inline constexpr std::int32_t kRewardsActionGamerCard = 0x4F;
inline constexpr std::int32_t kRewardsActionPlayerReview = 0x50;
inline constexpr std::int32_t kRewardsActionPagePrevious = 0x4C;
inline constexpr std::int32_t kRewardsActionPageNext = 0x4D;

struct RewardsScreenUpdateState {
    std::uint8_t storage_slot{};            // +19h, passed to 007FA1B0
    std::int32_t storage_flush_countdown{}; // +38h, -1 when idle
    bool leaving{};                         // +3Ch
    std::int32_t page{};                    // +58h
    float arrow_flash{};                    // +64h, seconds left
    bool rank_alpha_rising{};               // +218h
    float rank_grow_target{};               // +21Ch, 0 when idle
    bool input_locked{};                    // +258h
};

struct GuiSize2 {
    float x;
    float y;
};

enum class RewardsArrow : std::uint8_t { Previous, Next };  // widgets +5Ch, +60h

struct RewardsScreenUpdateHost {
    virtual ~RewardsScreenUpdateHost() = default;
    virtual float game_frame_seconds_21ec() = 0;                  // *(00E188A8)+21ECh
    // 007FA1B0 on *(00E188A8)+650h with the continuation 00BD53C0.
    virtual void flush_storage_007fa1b0(std::uint8_t slot) = 0;
    virtual bool action_004d92b0(std::int32_t action) = 0;        // ECX = *(00E188A8)
    virtual void leave_0062b110() = 0;                            // ECX = screen
    virtual bool gamer_card_available_0061d8b0() = 0;
    virtual std::uint64_t shown_player_xuid_0061d710() = 0;       // EDX:EAX
    virtual std::uint64_t local_player_xuid_00a3eb00() = 0;       // *(00F8ABE8)
    virtual std::int32_t active_user_slot_00a3eac0() = 0;         // *(00F8ABE8), RET
    virtual void show_gamer_card_00a4d440(std::int32_t user, std::uint64_t xuid) = 0;
    virtual void show_player_review_00a4d470(std::int32_t user, std::uint64_t xuid) = 0;
    virtual bool session_virtual_198() = 0;                       // *(00F8A2FC) slot +198h
    // The rank widget *(screen+210h): 00AA6740 size, slots +58h set size,
    // +54h colour (alpha at +0Ch), +4Ch set alpha.
    virtual GuiSize2 rank_size_00aa6740() = 0;
    virtual void rank_set_size_58(GuiSize2 size) = 0;
    virtual float rank_alpha_54() = 0;
    virtual void rank_set_alpha_4c(float alpha) = 0;
    // 00683820(1) on the scrollers at +68h, +128h and +19Ch, in that order.
    virtual void scroller_update_00683820(std::uint32_t offset) = 0;
    virtual void change_page_00622e30(std::int32_t direction) = 0;
    // Arrow widget slot +88h(state, 0, 1.0f).
    virtual void arrow_set_state_88(RewardsArrow arrow, std::int32_t state) = 0;
};

void rewards_screen_update_0062b640(RewardsScreenUpdateState& screen, RewardsScreenUpdateHost& host);

// ---------------------------------------------------------------------------
// Rewards enter, opening segment (0062C2C0..0062CA7B)
// ---------------------------------------------------------------------------

inline constexpr std::string_view kRewardsRankCounter = "RANK";        // 00CEF15C
inline constexpr std::string_view kRewardsServiceRankPrefix = "RANK_SR";  // 00CF5678

// Kind byte of a 10h-byte award page pushed onto +238h by 006278D0 (its
// string is empty here): 0 = rank change, 1 = one unlocked vehicle class.
enum class RewardsAwardPage : std::uint8_t { RankChange = 0, Unlock = 1 };

// Page modes stored at screen+58h by 0062CB8D..0062CBD4.
enum class RewardsPageMode : std::int32_t {
    CampaignWon = 0,   // single player, +19h set: the debriefing page (+40h)
    Session = 1,       // game+1EE3h or game+1FE4h set: the scoreboard page (+44h)
    CampaignLost = 2,  // single player, +19h clear: neither page is filled here
};

struct RewardsEnterState {
    float field64{};                       // +64h, cleared
    bool input_locked258{};                // +258h, cleared; set at 0062CA92 when pages are queued
    bool score_zero18{};                   // +18h: commit record +0Ch (play seconds) == 0.0f (UCOMISS, ordered)
    bool commit_present19{};               // +19h: *(commit record) != 0, the mission-won word (0062D461)
    RewardsPageMode page_mode58{};         // +58h
    bool group_completed1c{};              // +1Ch
    bool field1d{};                        // +1Dh, cleared
    bool unlocks_pending24{};              // +24h: 007F93F0 on the profile
    bool advanced_unfinished25{};          // +25h
    bool advanced27{};                     // +27h: 005C3DE0 advanced the campaign
    bool skip3c{};                         // +3Ch: title cleared and return
    std::string mission28;                 // +28h, copy of game+2198h
    std::string mission30;                 // +30h, copy of game+2198h, then the profile's +68h id
    std::vector<RewardsAwardPage> pages238;  // +238h (cleared by 00625A50)
    std::int32_t field38{};                // +38h: 3 when +18h is clear, else -1 (00636D51)
};

// A native GUI widget (vtable 00D5BED0 family), opaque here.
using RewardsWidget = void*;

struct RewardsVec3 {
    float x{};
    float y{};
    float z{};
};

// The commit record 004B4750 returns (ECX = *(00E188A8)), held at [esp+24h]
// from 0062C3B2 on; only the fields the page builders read.
struct RewardsCommitRecord {
    std::int32_t won{};        // +00h: nonzero selects FE.rewards_missioncompleted
    std::int32_t field04{};    // +04h: medal_Icon state + 1; third argument of 00910570
    std::int32_t field08{};    // +08h: second argument of 00910500
    float map_seconds{};       // +14h: FE.scoring_mapusage input
    std::uint8_t flag24d{};    // +24Dh: last argument of 00911CE0/00911C00
    std::int32_t badge{};      // +1DCh: FE.scoring_badge points
    float play_seconds{};      // +0Ch: BSP_Text_FormatPlayTime input
    std::int32_t score{};      // +1E0h: score_value_Text
    std::int32_t field1e4{};   // +1E4h: side test when game+1EE3h is set (0062EA31)
    std::string end_text;      // +254h: END_Text source (localised)
    std::string usn_text;      // +25Ch: USA_Text source (localised)
    std::string ijn_text;      // +264h: JAP_Text source (localised)
};

// One 60h-byte row of the scoreboard vector at screen+220h (begin +224h, end
// +228h), filled by 0062A0B0 and sorted by 00625D10.
struct RewardsScoreRow {
    std::uint32_t id0{};       // +00h, compared with the local player's +80h
    std::uint32_t id4{};       // +04h, compared with the local player's +84h
    std::string name;          // +08h
    std::string clan;          // +10h
    std::int32_t side{};       // +18h: 0 = USN rows, 1 = IJN rows
    std::int32_t score{};      // +50h
    std::int32_t rank{};       // +58h: rank_Icon state + 1
    std::int32_t game_mode34{};  // +34h: 004BCA50 when the row was built
    std::int32_t badges48{};   // +48h: keys of the player's score record that 0090C000 accepts
    bool ai5c{};               // +5Ch: the slot's +9h byte (an AI slot; its name may be a difficulty key)
};

// The player record *(game+18CCh + 4*slot) fields 0062A0B0 reads.
struct RewardsPlayerSlot {
    bool ai09{};               // +09h
    bool byte0a{};             // +0Ah: an AI slot without it is skipped
    std::int32_t index20{};    // +20h: negative = empty; compared with game+18ECh
    std::int32_t side28{};     // +28h
    std::int32_t difficulty2c{};  // +2Ch: index into kRewardsAiNameKeys
    std::string name58;        // +58h, C string
    std::string clan78;        // +78h, C string
    std::uint32_t id80{};      // +80h
    std::uint32_t id84{};      // +84h
    std::uint8_t rank90{};     // +90h
};

// 00E08940: the AI name keys by difficulty (6..8 are not strings in the image).
inline constexpr std::array<std::string_view, 6> kRewardsAiNameKeys{"FE.easy", "FE.easy", "FE.easy", "FE.easy", "FE.normal", "FE.hard"};
inline constexpr std::string_view kRewardsAiClan = " ";  // 00CE3A90

struct RewardsEnterHost;

// 0062A0B0: __thiscall(screen), RET. Rebuilds the +220h rows from the eight player slots.
std::vector<RewardsScoreRow> rewards_build_score_rows_0062a0b0(RewardsEnterHost& host);
// 00625D10: the rows by descending score; equal scores keep their order (the
// insertion-sort path, 00623410, which is the only one up to 20h rows).
void rewards_sort_score_rows_00625d10(std::vector<RewardsScoreRow>& rows);

// The local player's record *(game+18CCh + 4*game+18ECh), read only when
// game+18ECh is in 0..7.
struct RewardsLocalPlayer {
    std::int32_t side{};       // +28h
    std::string name;          // +58h
    std::uint32_t id80{};      // +80h
    std::uint32_t id84{};      // +84h
};

inline constexpr std::string_view kRewardsMissionCompleted = "FE.rewards_missioncompleted";  // 00CF560C
inline constexpr std::string_view kRewardsMissionFailed = "FE.rewards_missionfailed";        // 00CF55F0
inline constexpr std::string_view kRewardsUsnWins = "FE.scoring_USN_wins";                   // 00CF5594
inline constexpr std::string_view kRewardsIjnWins = "FE.scoring_IJN_wins";                   // 00CF5580
inline constexpr std::string_view kRewardsScoringMissionFailed = "FE.scoring_missionfailed"; // 00CF5558
inline constexpr std::string_view kRewardsNewPlayer = "globals.newplayer";                   // 00CEF794
inline constexpr std::int32_t kRewardsGameModeNoSides = 7;  // game+614h value with one shared table
inline constexpr double kRewardsRowGap = 0.008333333333333333;  // 00CF0DE8, added to the row height
inline constexpr double kRewardsIjnTableRows = 6.0;             // 00CE6628
inline constexpr std::int32_t kRewardsIjnFirstSlot = 6;         // 0062E2D4
inline constexpr float kRewardsListbox124 = 0.575f;             // 00CF557C
inline constexpr float kRewardsListbox12c = -0.0541666672f;     // 00CF5578
inline constexpr float kRewardsListbox130 = 0.00763888890f;     // 00CF5574
inline constexpr float kRewardsListbox128 = 0.00555555569f;     // 00CF2548
// The debriefing-voice completion callback pushed at 0062D1D4: it hands the name
// to *(*(00E198AC)+60h) through 0061F9D0, then removes the file from the store.
inline constexpr std::uint32_t kRewardsDebriefVoiceCallback = 0x00622E00;

// One node of a commit-record objective list (node fields after next/prev).
struct RewardsObjective {
    std::string key;           // node+0Ch: shown without its first character (00469840(1, 7FFFFFFFh))
    std::string points_key;    // node+14h: key of the record+18h points map
    std::int32_t completed{};  // node+1Ch
};

enum class RewardsObjectiveKind : std::uint8_t { Primary = 0, Secondary = 1, Hidden = 2 };

inline constexpr std::string_view kRewardsObjectiveCompleted = "^FE.rewards_completed";  // 00CF5514
inline constexpr std::string_view kRewardsObjectiveFailed = "^FE.rewards_failed";        // 00CF5500
inline constexpr std::string_view kRewardsHiddenObjective = "globals.obj_hid";                 // 00CEFCE8
inline constexpr std::string_view kRewardsObjectiveNumberSuffix = ".";                   // 00CE3A70
inline constexpr std::string_view kRewardsZeroPoints = "0";                              // 00CE43BC
inline constexpr double kRewardsObjectiveLine = 0.02777777798473835;   // 00CF5550 (1/36 as a float)
inline constexpr double kRewardsObjectiveArea = 0.6527777910232544;    // 00CF5548
inline constexpr double kRewardsObjectiveTop = 0.1736111111111111;     // 00CF5540
inline constexpr double kRewardsObjectiveHeaderGap = 0.013888888992369175;  // 00CF0B20
inline constexpr float kRewardsObjectiveDepth = -1.0f;                 // 00D7A260
inline constexpr std::array<float, 4> kRewardsFailedColor{0.7f, 0.1f, 0.1f, 1.0f};  // 00CE3E18, 00D7A2F0 x2, 00D7A24C

// The scoring page (0063018D..006359A3) reads these maps of the commit record.
// Damage: record+9Ch (player) / +A8h (party), map<by category, map<friendly
// key, map<target category, damage>>>; an L1 node with an empty inner map
// still opens a group.
struct RewardsDamageTarget {
    std::int32_t category{};   // L3 key, index into kRewardsUnitCategoryKeys
    std::int32_t damage{};     // L3 value
};
struct RewardsDamageSide {
    std::int32_t friendly_key{};  // L2 key: 0 is a friendly hit
    std::vector<RewardsDamageTarget> targets;
};
struct RewardsDamageGroup {
    std::int32_t by_category{};   // L1 key
    std::vector<RewardsDamageSide> sides;
};
// record+CCh (count +D4h): map<side key, map<category, count>>; record+D8h
// (count +E0h): map<category, count>.
struct RewardsCategoryCount {
    std::int32_t category{};
    std::int32_t count{};
};
struct RewardsRemainingSide {
    std::int32_t side_key{};      // only key 0 is listed
    std::vector<RewardsCategoryCount> counts;
};
// record+48h (count +50h): map<name, int>, node +0Ch and +14h.
struct RewardsMissionSpecial {
    std::string name;
    std::int32_t points{};
};

// 00E0B5E0: the unit-category keys the scoring tables index (20 pointers;
// 18 and 19 are empty strings).
inline constexpr std::array<std::string_view, 20> kRewardsUnitCategoryKeys{
    "globals.unitcat_carrier", "globals.unitcat_destroyer", "globals.unitcat_torpedoboat",
    "globals.unitcat_battleship", "globals.unitcat_cruiser", "globals.unitcat_cargo",
    "globals.unitcat_landingship", "globals.unitcat_levelbomber", "globals.unitcat_divebomber",
    "globals.unitcat_torpedobomber", "globals.unitcat_fighter", "globals.unitcat_reconplane",
    "globals.unitcat_kamikaze", "globals.unitcat_submarine", "globals.unitcat_landvehicle",
    "globals.unitcat_landfort", "globals.unitcat_airfield", "globals.unitcat_shipyard", "", ""};

inline constexpr std::string_view kRewardsScoringPlayerAct = "FE.scoring_playeract";      // 00CF5424
inline constexpr std::string_view kRewardsScoringPartyAct = "FE.scoring_partyact";        // 00CF539C
inline constexpr std::string_view kRewardsScoringCommand = "FE.scoring_command";          // 00CF5388
inline constexpr std::string_view kRewardsScoringCommandType = "FE.scoring_command_type";  // 00CF5370
inline constexpr std::string_view kRewardsScoringCommandAmount = "FE.scoring_command_amount";  // 00CF5344
inline constexpr std::string_view kRewardsScoringRemaining = "FE.scoring_remaining";      // 00CF5304
inline constexpr std::string_view kRewardsScoringSuicide = "FE.scoring_suicide";          // 00CF52E8
inline constexpr std::string_view kRewardsScoringMapUsage = "FE.scoring_mapusage";        // 00CF52D4
inline constexpr std::string_view kRewardsScoringBadge = "FE.scoring_badge";              // 00CF52A8
inline constexpr std::string_view kRewardsScoringMissionSpec = "FE.scoring_missionspec";  // 00CF5290
inline constexpr std::string_view kRewardsFriendlyHitPrefix = "FE.scoring_friendlyhit|.: |";  // 00CF53C0
inline constexpr std::string_view kRewardsCountOpen = "|. (x";                            // 00CF52FC
inline constexpr std::string_view kRewardsCountClose = ")";                               // 00CE584C
inline constexpr double kRewardsScoreRow = 0.030555555829778314;      // 00CF53B8
inline constexpr double kRewardsScoreGroupGap = 0.0027777778450399637;  // 00CF53B0
inline constexpr double kRewardsDoneToMargin = 0.020833333333333332;  // 00CF0000
inline constexpr double kRewardsRankBarInset = 0.00625;               // 00CF2920
inline constexpr std::string_view kRewardsScoreSeparator = " / ";  // 00CF2928
inline constexpr std::int32_t kRewardsTopRank = 20;                  // 00574050 value with no next threshold
// 00E08E00: FE.playerrank_01 .. FE.playerrank_20.
inline constexpr std::array<std::string_view, 20> kRewardsPlayerRankKeys{
    "FE.playerrank_01", "FE.playerrank_02", "FE.playerrank_03", "FE.playerrank_04", "FE.playerrank_05",
    "FE.playerrank_06", "FE.playerrank_07", "FE.playerrank_08", "FE.playerrank_09", "FE.playerrank_10",
    "FE.playerrank_11", "FE.playerrank_12", "FE.playerrank_13", "FE.playerrank_14", "FE.playerrank_15",
    "FE.playerrank_16", "FE.playerrank_17", "FE.playerrank_18", "FE.playerrank_19", "FE.playerrank_20"};

// One 10h-byte element of the screen+248h vector (the unlock awards).
struct RewardsUnlockAward {
    std::int32_t icon_id{};   // +04h
    std::string unit;         // +08h
};
// One 1Ch-byte element of the profile's pending awards (game+70Ch).
struct RewardsPendingAward {
    std::string title;        // +00h
    std::string detail;       // +08h
    std::string icon_name;    // +10h, used when the id is 0
    std::int32_t icon_id{};   // +18h
};
// The 54h-byte award record fields the badge pages read.
struct RewardsAwardDefinition {
    std::string title;        // +00h
    std::string detail;       // +0Ch
    std::string texture;      // +14h
};

inline constexpr std::string_view kRewardsUnlockedUnitPrefix = "FE.scoring_unlockedunit|.: |";  // 00CF5250
inline constexpr std::string_view kRewardsUnlockText = "FE.scoring_unlock_text";                // 00CF5208
inline constexpr double kRewardsAwardGap = 0.006944444444444444;    // 00CF4570 (1/144)
inline constexpr double kRewardsAwardPadding = 0.013888888888888888;  // 00CEBDD0 (1/72)
inline constexpr std::array<float, 4> kRewardsAwardTextureColor{0.0f, 0.0f, 1.0f, 1.0f};

struct RewardsEnterHost {
    virtual ~RewardsEnterHost() = default;
    virtual bool session_present_f8a2fc() = 0;
    virtual bool session_virtual_198() = 0;
    virtual void session_virtual_58() = 0;
    virtual void widget_25c_show(bool show) = 0;                  // slot +34h
    virtual void clear_pages_248_00625a50() = 0;                  // +238h is cleared through the state
    virtual float commit_record_float_0c_004b4750() = 0;          // ECX = *(00E188A8)
    virtual bool commit_record_head_nonzero_004b4750() = 0;
    virtual void mission_record_00594a70() = 0;                   // game+6B4h, &game+2198h
    virtual void selected_mission_005c3870() = 0;                 // *(00E198AC)+5Ch
    virtual bool mission_completed_009052d0() = 0;                // ECX = the 00594A70 record
    virtual std::string game_mission_2198() = 0;
    virtual void set_frame_title_00518d60(std::string_view title) = 0;  // ECX = EDX = 0, stack 0
    virtual std::int32_t profile_counter_005097d0(std::string_view name) = 0;  // ECX = game+650h
    virtual void profile_update_007fcd90(std::int32_t arg) = 0;
    virtual std::int32_t game_dword(std::uint32_t offset) = 0;
    virtual bool profile_007f93f0() = 0;
    // Keys (node +0Ch) of the profile set at game+6CCh not present in the set
    // at game+6C0h (004C7F10 find against the end +6C4h), in iteration order.
    virtual std::vector<std::string> pending_unlock_keys_6cc() = 0;
    // 00622360(key, count): VehicleClass rows with Unlock == true whose UnlockID
    // contains key; count = 1 only counts, count = 0 also records them.
    virtual std::int32_t vehicle_unlocks_00622360(std::string_view key, bool count_only) = 0;
    virtual void profile_flush_pending_unlocks_007fb990() = 0;
    virtual bool advance_campaign_selection_005c3de0() = 0;       // *(00E198AC)+5Ch
    virtual std::string profile_mission_id_00586150() = 0;
    virtual bool group_completed_005c3be0(const std::string& mission) = 0;
    virtual std::vector<std::string> commit_record_keys_6c() = 0;  // list +6Ch, node +0Ch
    virtual bool has_prefix_00553c80(std::string_view key, std::string_view prefix) = 0;  // (prefix, 0)

    // --- page builders (0062CA7B on) ---
    virtual const RewardsCommitRecord& commit_record() = 0;       // [esp+24h]
    // The 005C3870 mission record kept at [esp+84h]: +38h debriefingText,
    // +50h debriefingVoice (docs/MISSION_TREE_LUA_READER.md).
    virtual std::string selected_debriefing_text_38() = 0;
    virtual std::string selected_debriefing_voice_50() = 0;
    virtual std::int32_t mission_side_005c27e0() = 0;            // ECX = the 005C3870 record
    // 00574050 on game+650h: the low byte of the profile's "RANK" value (+A0h map, 005070C0).
    virtual std::int32_t profile_rank_byte_00574050() = 0;
    virtual RewardsWidget screen_widget(std::uint32_t offset) = 0;  // *(screen + offset)
    virtual std::uint32_t screen_dword(std::uint32_t offset) = 0;
    virtual float screen_float(std::uint32_t offset) = 0;
    virtual std::uint8_t game_byte(std::uint32_t offset) = 0;
    // 00AA7E00(name, 1) on ECX = parent; the name is a temporary NativeString.
    virtual RewardsWidget child_00aa7e00(RewardsWidget parent, std::string_view name) = 0;
    virtual void widget_show_34(RewardsWidget widget, bool show) = 0;
    virtual void widget_set_active_60(RewardsWidget widget, bool active) = 0;
    virtual void icon_state_88(RewardsWidget widget, std::int32_t state) = 0;  // (state, 0, 1.0f)
    // 00ABAED0(source, localise): RET 8.
    virtual void text_source_00abaed0(RewardsWidget text, std::string_view source, bool localise) = 0;
    virtual float text_height_00ab6bd0(RewardsWidget text) = 0;
    virtual GuiSize2 widget_size_00aa6740(RewardsWidget widget) = 0;
    virtual RewardsVec3 widget_position_00aa6750(RewardsWidget widget) = 0;
    virtual void widget_set_position_00aa8240(RewardsWidget widget, const RewardsVec3& position) = 0;
    virtual RewardsWidget clone_00aab4c0(RewardsWidget widget) = 0;   // parent 0
    virtual void scroller_set_range_006834a0(std::uint32_t scroller_offset, float range) = 0;
    // 00E08C88 / 00E08C89, written beside +58h.
    virtual void set_debrief_flags_e08c88(bool won, bool session) = 0;
    // The six page titles in the NativeString vector at 00E196C0 (read by
    // 00621C60, the page switch); element 1 is the +44h page, 2 the +48h page.
    virtual void set_page_title_e196c0(std::size_t index, std::string_view key) = 0;
    virtual float round_value_004396f0(float value) = 0;
    // The debriefing voice: 004FC150 -> 00BE80B0 -> BSP_FileStore_RequestFile
    // (00BE7CD0) with the completion callback 00622E00.
    virtual void request_streamed_dialog_00be7cd0(std::string_view path, std::uint32_t callback) = 0;
    virtual std::string_view audio_language_folder_008d57a0() = 0;  // ECX = 00F88980
    // The scoreboard rows (+220h), rebuilt by rewards_build_score_rows_0062a0b0
    // and sorted by rewards_sort_score_rows_00625d10.
    virtual std::vector<RewardsScoreRow>& score_rows_220() = 0;
    // --- 0062A0B0 inputs ---
    virtual bool player_slot_18cc(std::int32_t slot, RewardsPlayerSlot& out) = 0;  // false for a null slot
    virtual std::int32_t effective_game_mode_004bca50() = 0;
    // The per-player score record *(game+21A0h)+4+slot*284h: +1E0h and the key list +6Ch.
    virtual std::int32_t player_score_1e0(std::int32_t slot) = 0;
    virtual std::vector<std::string> player_score_keys_6c(std::int32_t slot) = 0;
    virtual bool local_player(RewardsLocalPlayer& out) = 0;       // false when game+18ECh is outside 0..7
    // The scoreboard listbox at screen+D4h and its row template at +D8h.
    virtual void listbox_clear_rows_00a9bec0() = 0;
    virtual void listbox_set_listener_00a9ac40(bool screen_listener) = 0;  // 0 or screen+8
    virtual void listbox_insert_row_00a9da30(RewardsWidget row, std::int32_t data) = 0;  // (row, data, 0, 0)
    virtual std::int32_t listbox_row_count_104() = 0;
    virtual void listbox_select_00a9c7c0(std::int32_t index) = 0;
    virtual void listbox_set_float(std::uint32_t offset, float value) = 0;
    virtual void listbox_set_124_00a9ceb0(float value, bool relayout) = 0;
    // byte screen+22Fh+count after an insert (+230h..+237h are cleared first).
    virtual void set_row_human_flag(std::int32_t count_after_insert, bool human) = 0;
    virtual void clear_row_human_flags_230() = 0;

    // --- objectives page (0062EA1B..00630188) ---
    // The commit record's three objective lists for a side (0: +204h, +210h,
    // +21Ch; else +228h, +234h, +240h), each a std::list (head +4, size +8).
    virtual const std::vector<RewardsObjective>& commit_objectives(std::int32_t side, RewardsObjectiveKind kind) = 0;
    // 0061EC70 on record+18h: the points map, keyed by node+14h.
    virtual bool commit_objective_points(const std::string& points_key, std::int32_t& points) = 0;
    // 0061F340 on the 00594A70 record +21Ch (side 0) or +240h: true when the key
    // is present and that node's +1Ch is nonzero.
    virtual bool hidden_objective_revealed(std::int32_t side, const std::string& key) = 0;
    virtual void text_ellipsized_00abb000(RewardsWidget text, std::string_view source, float width, bool localise) = 0;
    virtual void text_cstring_00abbe50(RewardsWidget text, std::string_view source, bool localise) = 0;
    virtual void widget_set_size_58(RewardsWidget widget, GuiSize2 size) = 0;
    virtual void widget_set_color_50(RewardsWidget widget, const std::array<float, 4>& rgba) = 0;
    // 0052ED20: push_back onto the widget vector at screen+F8h.
    virtual void keep_created_widget_f8_0052ed20(RewardsWidget widget) = 0;
    virtual void set_objectives_flag_e08c8a(bool any) = 0;

    // --- scoring page (0063018D..006359A3) ---
    virtual void set_scoring_flag_e08c8b(bool value) = 0;
    virtual RewardsWidget clone_under_00aab4c0(RewardsWidget widget, RewardsWidget parent) = 0;
    virtual RewardsVec3 widget_local_position_00aa67f0(RewardsWidget widget) = 0;  // &widget+0Ch
    virtual void widget_set_local_position_00aa7dc0(RewardsWidget widget, const RewardsVec3& position) = 0;
    virtual std::vector<RewardsDamageGroup> damage_groups(std::uint32_t map_offset) = 0;  // +9Ch / +A8h
    // 00593480 copies the map at record+`kills_offset` (+B4h / +C0h); the
    // 0062C170 -> 0062BB60 -> 00625900 chain indexes the copy (a missing key
    // reads 0 there); the copy is freed after the row (00593180).
    virtual std::int32_t kills_from_copy(std::uint32_t kills_offset, std::int32_t friendly_key,
                                         std::int32_t by_category, std::int32_t target_category) = 0;
    virtual std::vector<RewardsRemainingSide> remaining_units_cc() = 0;
    virtual std::vector<RewardsCategoryCount> suicides_d8() = 0;
    virtual std::vector<RewardsMissionSpecial> mission_specials_48() = 0;
    // The point formulas on *(game+21A0h).
    virtual std::int32_t damage_points_00911ce0(std::int32_t by, std::int32_t target, std::int32_t friendly_key,
                                                std::int32_t damage, std::int32_t mode, std::uint8_t flag24d) = 0;
    virtual std::int32_t kill_points_00911c00(std::int32_t friendly_key, std::int32_t by, std::int32_t target,
                                              float kills, std::int32_t mode, std::uint8_t flag24d) = 0;
    virtual std::int32_t remaining_points_00910500(std::int32_t category, std::int32_t field08) = 0;
    virtual std::int32_t suicide_points_00910570(std::int32_t category, std::int32_t mode, std::int32_t field04) = 0;
    virtual std::int32_t map_usage_points_009064d0(float seconds) = 0;

    // --- rank page (006359A3..00635F9E) ---
    virtual void set_rank_flag_e08c8c(bool value) = 0;
    virtual std::uint8_t session_byte_4d() = 0;                 // *(00F8A2FC)+4Dh, read without a null test
    // 0050FC30(*(00E19900)+10h, "RANK"): the record's int vector at +48h.
    virtual std::int32_t rank_threshold_0050fc30(std::int32_t index) = 0;
    virtual std::int32_t profile_score_0090bf50() = 0;          // ECX = *(game+650h+64h)
    virtual std::int32_t profile_last_score_30() = 0;           // game+650h+30h, -1 = none yet
    virtual void set_profile_last_score_30(std::int32_t score) = 0;
    virtual void set_screen_byte(std::uint32_t offset, std::uint8_t value) = 0;
    virtual void set_screen_float(std::uint32_t offset, float value) = 0;

    // --- award pages (00635FA3..00636C33) and tail ---
    virtual void set_awards_flag_e08c8d(bool value) = 0;
    virtual bool profile_has_pending_awards_0061eb80() = 0;     // profile+BCh vector (1Ch elements) non-empty
    virtual std::vector<RewardsUnlockAward> unlock_awards_248() = 0;
    virtual std::vector<RewardsPendingAward> profile_pending_awards_70c() = 0;
    virtual void profile_clear_pending_awards_006277c0() = 0;
    // On *(*(00E198AC)+70h): an icon by id (004FB150) or by name (004FC070).
    virtual void award_icon_by_id_004fb150(std::int32_t id, RewardsWidget icon) = 0;
    virtual void award_icon_by_name_004fc070(const std::string& name, RewardsWidget icon) = 0;
    virtual bool badge_award_earned_0090c000(const std::string& key) = 0;  // ECX = *(game+21A0h), key by value
    // 0050FC30(*(00E19900)+10h, key) copied by 0050A210, freed by 00501FA0.
    virtual RewardsAwardDefinition award_definition(const std::string& key) = 0;
    virtual void set_state_texture_00ab44b0(RewardsWidget icon, std::int32_t state, std::string_view path,
                                            const std::array<float, 4>& rgba) = 0;
    virtual void widget_set_local_y_00aa78f0(RewardsWidget widget, float y) = 0;
    virtual void apply_award_pages_0062a970() = 0;              // ECX = screen
    virtual std::array<std::uint8_t, 6> page_flags_e08c88() = 0;  // 00E08C88..00E08C8D
    virtual void change_page_00622e30(std::int32_t direction) = 0;
    virtual void set_help_line_0054a0c0(std::string_view text) = 0;  // ECX = *(00E1930C)
};

// Folds a native rank or medal number into an icon state: max(value - 1, 0),
// passed on as its low word (MOVZX ... WORD PTR).
std::int32_t rewards_icon_state(std::int32_t value) noexcept;

// 0062C2C0: slot +18h of vtable 00CF50CC, __thiscall(screen), RET. The state
// reset, the early return on +3Ch and the award-page queue up to 0062CA7B,
// then rewards_screen_enter_pages_0062ca7b. Returns false where the native
// routine returns early (+3Ch).
bool rewards_screen_enter_award_queue_0062c2c0(RewardsEnterState& screen, RewardsEnterHost& host);

// 0062CA7B..00636D78: the page builders of the same body (one native function;
// split here at the point both opening paths join). Range table and evidence in
// docs/CC13_INGAME_GUI.md.
void rewards_screen_enter_pages_0062ca7b(RewardsEnterState& screen, RewardsEnterHost& host);

// ---------------------------------------------------------------------------
// Support-manager info panel refresh (0066B6C0)
// ---------------------------------------------------------------------------

inline constexpr std::string_view kSupportInfoSupportKey = "ingame.sm_support";       // 00CF69F8
inline constexpr std::string_view kSupportInfoCpPrefix = "ingame.sm_cp|.: ";          // 00CF69CC
inline constexpr std::string_view kSupportInfoCostSeparator = " / |ingame.sm_cost|.: ";  // 00CF69E0
inline constexpr std::string_view kSupportInfoStockPrefix = "ingame.sm_stock|.: ";    // 00CF69B8

// Opaque pointers to a unit (vtable query slot +5Ch, name slot +10h) or a
// class record (flag query slot +18h, name +54h, icon key +70h, cost +124h).
using SupportObject = std::uintptr_t;

// Widgets of the support-manager screen this routine touches.
enum class SupportInfoWidget : std::uint8_t {
    NameText,     // +170h, text set with 00ABBE50(name, 1)
    TypeIcon,     // +1CCh, slot +88h(state, 0, 1.0f)
    DetailF0,     // +F0h, slot +34h(show)
    DetailF4,     // +F4h, slot +34h(show)
    LoadoutIcon,  // +158h, 00AB2690 state texture and slot +88h
    CostText,     // +1A8h, text set with 00ABAED0(source, 1)
    BarCp,        // +180h, slot +58h(size) and +34h
    BarAfford,    // +184h
    BarRemain,    // +188h, also the height source (00AA6740 size +4)
};

struct SupportManagerInfoState {
    std::int32_t page{};             // +5Ch: 0, 1 or 2; any other value does nothing
    bool detail70{};                 // +70h: cleared on page 0, set when a detail is shown
    bool flag66{};                   // +66h: forces the sm_support text on page 2
    std::uint32_t list_a_count{};    // +1F4h, list +1ECh, iterator +300h/+304h
    std::uint32_t list_b_count{};    // +34Ch, list +344h, iterator +350h/+354h
    std::uint32_t selection_2c4{};   // +2C4h, clamped to list_c_count - 1
    std::int32_t slot_324{};         // +324h, slot index on the selected unit
    std::int32_t field164{};         // +164h
    bool field2b0{};                 // +2B0h
    bool field2c1{};                 // +2C1h: keeps the CP bar hidden when unaffordable
    float bar_width_1ac{};           // +1ACh
    SupportObject catalog_3a4{};     // +3A4h, 0065AB40's answer
    SupportObject catalog_2a0{};     // +2A0h, 0065AE90's answer
};

// One node of list +3E0h: byte +14h picks the live unit (+18h) over the class
// record (+08h); +0Ch is the loadout id passed to 009552E0.
struct SupportListCEntry {
    bool live_unit{};
    SupportObject unit{};
    SupportObject record{};
    std::int32_t loadout{};
};

// A unit slot: the class record and loadout id. 00846FD0's out object (+08h,
// +0Ch; vtable 00CF6544, destroyed by 0065BB00) or a 58h-byte hangar record
// (+04h, +10h; -1 = no loadout).
struct SupportSlotEntry {
    SupportObject record{};
    std::int32_t loadout{};
};

// Page 1: the selected node of list +344h. +08h, and the unit at +3Ch of the
// object at the front of the node's list +0Ch (0 when absent).
struct SupportListBEntry {
    SupportObject node_value8{};
    SupportObject front_unit3c{};
};

// The loadout's device icon: 009552E0(id) on the class, then +4 -> +8 -> +4
// resolved by 00443490 (DL = 0); +74h -> +34h gives the texture (+7Ch) and
// the UV rectangle (+90h..+9Ch).
struct SupportLoadoutIcon {
    void* texture{};
    std::array<float, 4> uv{};
};

struct SupportManagerInfoHost {
    virtual ~SupportManagerInfoHost() = default;
    virtual void rebuild_list_c_0066a8a0() = 0;                   // ECX = screen
    virtual std::uint32_t list_c_count_3e8() = 0;                 // list +3E0h (head +3E4h)
    virtual void sync_cursor_0065a2b0() = 0;                      // wraps +308h, places +300h/+350h
    virtual SupportListCEntry list_c_entry(std::uint32_t index) = 0;
    virtual SupportObject selected_unit_a() = 0;                  // *(+304h)+8 (00658A60 on page 2)
    virtual SupportListBEntry selected_list_b() = 0;
    virtual SupportObject catalog_0065ab40() = 0;                 // list +37Ch at index +310h
    virtual SupportObject catalog_0065ae90() = 0;                 // list +2E4h at index +310h
    virtual std::int32_t stock_count_0065ac30() = 0;              // -1 when unknown
    virtual void rebuild_00665f30() = 0;
    virtual SupportSlotEntry unit_slot_00846fd0(SupportObject unit, std::int32_t index) = 0;
    // (unit ? unit + (kind45 ? 72Ch : 1188h) : 0)+4Ch, record index*58h.
    virtual SupportSlotEntry hangar_slot(SupportObject unit, bool kind45, std::int32_t index) = 0;
    virtual bool unit_is_kind(SupportObject unit, std::int32_t kind) = 0;      // slot +5Ch
    virtual bool record_has_flag(SupportObject record, std::int32_t flag) = 0; // slot +18h
    virtual const char* unit_name(SupportObject unit) = 0;        // slot +10h(1)
    virtual SupportObject unit_owner_3d0(SupportObject unit) = 0;
    virtual SupportObject unit_class_538(SupportObject unit) = 0;
    virtual std::int32_t unit_loadout_c54(SupportObject unit) = 0;
    virtual const char* record_name_54(SupportObject record) = 0;
    virtual std::int32_t record_cost_124(SupportObject record) = 0;
    // The type-icon map at +42Ch keyed by the record's +70h string: 00614060
    // find against the end at +430h (with 00611C80), 006171B0 subscript.
    virtual bool icon_map_contains(SupportObject record) = 0;
    virtual std::uint16_t icon_map_subscript(SupportObject record) = 0;  // inserts 0 if absent
    virtual SupportLoadoutIcon loadout_icon(SupportObject class_record, std::int32_t loadout) = 0;
    virtual void set_text_localised(SupportInfoWidget widget, const char* text) = 0;  // 00ABBE50
    virtual void set_text_source(SupportInfoWidget widget, std::string_view text) = 0; // 00ABAED0
    virtual void layout_name_label_0065bbc0() = 0;
    virtual void widget_show(SupportInfoWidget widget, bool show) = 0;            // +34h
    virtual void widget_set_state(SupportInfoWidget widget, std::int32_t state) = 0;  // +88h
    virtual void loadout_icon_set_texture(const SupportLoadoutIcon& icon) = 0;     // 00AB2690(3, ...)
    virtual float widget_height(SupportInfoWidget widget) = 0;                     // 00AA6740 +4
    virtual void widget_set_size(SupportInfoWidget widget, GuiSize2 size) = 0;     // +58h
    virtual float party_cp_00946f00() = 0;                        // ECX = *(00E188A8)+18ECh
    virtual float party_cp_cap_00942130() = 0;
    virtual std::int32_t effective_game_mode_004bca50() = 0;
};

// 0066B6C0: __thiscall(screen), RET; callers 0066D020 and 00675C40.
void support_manager_refresh_info_0066b6c0(SupportManagerInfoState& screen, SupportManagerInfoHost& host);

} // namespace bsp

#endif
