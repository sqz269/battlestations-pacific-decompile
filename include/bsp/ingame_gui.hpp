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

struct RewardsEnterState {
    float field64{};                       // +64h, cleared
    bool input_locked258{};                // +258h, cleared
    bool score_zero18{};                   // +18h: commit record +0Ch == 0.0f (UCOMISS, ordered)
    bool commit_present19{};               // +19h: *(commit record) != 0
    bool group_completed1c{};              // +1Ch
    bool field1d{};                        // +1Dh, cleared
    bool unlocks_pending24{};              // +24h: 007F93F0 on the profile
    bool advanced_unfinished25{};          // +25h
    bool advanced27{};                     // +27h: 005C3DE0 advanced the campaign
    bool skip3c{};                         // +3Ch: title cleared and return
    std::string mission28;                 // +28h, copy of game+2198h
    std::string mission30;                 // +30h, copy of game+2198h, then the profile's +68h id
    std::vector<RewardsAwardPage> pages238;  // +238h (cleared by 00625A50)
};

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
};

// 0062C2C0: slot +18h of vtable 00CF50CC, __thiscall(screen), RET. Partial:
// the state reset, the early return on +3Ch and the award-page queue up to
// 0062CA7B; the page builders that follow (to 00636D79) are not reconstructed.
// Returns false where the native routine returns (+3Ch).
bool rewards_screen_enter_award_queue_0062c2c0(RewardsEnterState& screen, RewardsEnterHost& host);

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
