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

} // namespace bsp

#endif
