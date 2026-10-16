#pragma once

#include "bsp/game_settings.hpp"
#include "bsp/native_game_settings_owner.hpp"
#include "bsp/profile_commit.hpp"
#include "bsp/weather_config.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {
namespace cc13_seg49_64_78 {
struct WarningObject;
struct WarningHost;
}  // namespace cc13_seg49_64_78


// Host projections, not original layouts or binary replacement interfaces.
// Address, ABI, assembly evidence and uncertainty: docs/CC13_OPTIONS_AND_HARDWARE.md.

// ---------------------------------------------------------------------------
// Settings save to the profile store: 008D6C00 and its continuation 008D6B70.
// The write counterpart of restore_profile_settings_008d7a50. Both share the
// pending-callback slot 00F88958, so the save takes ProfileSettingsRestoreState.
// ---------------------------------------------------------------------------
struct ProfileSettingsSaveHost {
    virtual ~ProfileSettingsSaveHost() = default;
    // Byte +21h of the storage manager at [0109CECC].
    virtual bool storage_available_21() = 0;
    // 00A3E510 with ECX = [00F8ABE8]; the native does not null-check the global.
    virtual bool xenon_has_selected_user_00a3e510() = 0;
    // 007F8CA0 with ECX = game+650h (the profile object).
    virtual bool profile_has_save_name_007f8ca0() = 0;
    // 00425F40 copy of game+684h (profile+34h).
    virtual std::string copy_profile_save_name_684() = 0;
    // 004C5E00: ECX = storage manager, calls manager vtable +10h with an
    // out-string. On the PC vtable 00D68C10 that slot is 00BEB630, the newest
    // valid slot (newest_valid_storage_slot_00beb630 below).
    virtual std::string storage_fallback_name_004c5e00() = 0;
    // 00BD3E10 (ECX = manager, NativeString*, DWORD kind; RET 8).
    virtual void request_single_write_00bd3e10(std::string_view name, std::uint32_t kind) = 0;
    // 006ADB50 stores ECX in 00E198F8 and drives the storage manager. It may
    // invoke the continuation synchronously or retain it across a prompt.
    virtual void run_storage_operation_006adb50(ProfileCompletion continuation) = 0;
    // Storage manager +08h. The writer is filled only in state 2.
    virtual int storage_state_08() = 0;
    // The temporary 8h store, vtable 00CE4104 and word +4 = 0. The native
    // normal path stores no retiring vtable here; end_ releases the host writer.
    virtual SettingsWriter& begin_settings_writer_00ce4104() = 0;
    virtual void end_settings_writer(SettingsWriter&) noexcept = 0;
};

// 008D6C00: __thiscall with ECX = settings (00F88980 at all seven call sites)
// but ECX is never read; one stack callback, RET 4. Only an accepted write
// stores the callback in 00F88958; every rejection invokes it directly.
void save_profile_settings_008d6c00(const GameSettingsBlock& settings,
    ProfileSettingsRestoreState& state, ProfileSettingsSaveHost& host,
    ProfileCompletion completion);

// 008D6B70: no arguments, RET. No Ghidra function existed at this address
// before this packet; it is reached only as the 006ADB50 continuation literal
// pushed at 008D6D11. State 2 writes through 008D64A0 and hands the pending
// callback to the storage driver; any other state delivers it directly. The
// slot is cleared before either use.
void complete_profile_settings_save_008d6b70(const GameSettingsBlock& settings,
    ProfileSettingsRestoreState& state, ProfileSettingsSaveHost& host);

inline constexpr std::uint32_t kSettingsSaveStorageKind = 0;   // 008D6CFD push 0
inline constexpr int kSettingsSaveWriterState = 2;             // 008D6B8D cmp [eax+8],2

// One FindFirstFileA/FindNextFileA result of 00BEB630's walk over the pattern
// at manager +53Ch (root + "\*", 00D68C58), with the two per-entry queries.
struct StorageSlotCandidate {
    std::string name;
    bool has_valid_marker{false};   // vtable +20h = 00BEB1C0, root\name\valid exists
    std::uint64_t valid_write_time{0}; // 00BEB0C0: last-write FILETIME of root\name\valid, 0 on failure
};

// 00BEB630 selection rule: ECX = manager, out NativeString*, RET 4. Starts from
// the empty string and a zero time; an entry with the valid marker replaces the
// choice only when its time is strictly greater (high dword first, then low),
// so ties keep the earlier entry and a zero time never wins. FindClose is
// called on the handle even when FindFirstFileA failed. The Win32 walk itself
// is not projected here; PcStorageBackend does not implement this slot yet.
std::string newest_valid_storage_slot_00beb630(const std::vector<StorageSlotCandidate>& entries);

// ---------------------------------------------------------------------------
// Settings equality: 008D4AF0 and its content-vector helper 008D48E0.
// Neither has a caller in the image (rel32 and absolute-dword census, see the
// doc); they are recovered for the field set they define, not for a call path.
// ---------------------------------------------------------------------------

// 008D48E0: __thiscall(ECX = vector, [ESP+4] = other vector), RET 4, AL.
// Stride-8 NativeString entries (+0 length, +4 text). Counts must match; a
// zero-length entry equals only a zero-length entry; otherwise __stricmp
// (00BF7FBF) decides, with no length comparison first.
bool native_string_vector_equal_008d48e0(
    const std::vector<std::string>& self, const std::vector<std::string>& other);

// The pointer-bearing members are passed as projections; every other compared
// field is read from the native 0BCh byte image.
struct GameSettingsComparand {
    const NativeGameSettingsStorage& bytes;
    const std::vector<std::string>& downloaded_content_98; // +98h data / +9Ch count
    const std::string& clan_text_b4;                       // NativeString +B4h
};

enum class SettingsFieldWidth : std::uint8_t { Byte, Dword, Float };

struct SettingsComparedField {
    std::uint8_t offset;
    SettingsFieldWidth width;
};

// The scalar fields 008D4AF0 compares before the +98h vector call, in native
// order (008D4AF3..008D4D0D). Floats use FUCOMIP/LAHF/TEST AH,44h/JP, which is
// IEEE equality: NaN differs from itself and +0 equals -0.
inline constexpr SettingsComparedField kSettingsEqualityHead[] = {
    {0x14, SettingsFieldWidth::Dword}, {0x18, SettingsFieldWidth::Dword},
    {0x1E, SettingsFieldWidth::Byte},  {0x20, SettingsFieldWidth::Float},
    {0x24, SettingsFieldWidth::Byte},  {0x28, SettingsFieldWidth::Float},
    {0x2C, SettingsFieldWidth::Float}, {0x30, SettingsFieldWidth::Float},
    {0x34, SettingsFieldWidth::Float}, {0x38, SettingsFieldWidth::Byte},
    {0x3C, SettingsFieldWidth::Dword}, {0x4B, SettingsFieldWidth::Byte},
    {0x49, SettingsFieldWidth::Byte},  {0x4A, SettingsFieldWidth::Byte},
    {0x40, SettingsFieldWidth::Byte},  {0x4C, SettingsFieldWidth::Byte},
    {0x50, SettingsFieldWidth::Dword}, {0x41, SettingsFieldWidth::Byte},
    {0x42, SettingsFieldWidth::Byte},  {0x43, SettingsFieldWidth::Byte},
    {0x44, SettingsFieldWidth::Byte},  {0x54, SettingsFieldWidth::Dword},
    {0x90, SettingsFieldWidth::Dword}, {0x84, SettingsFieldWidth::Byte},
    {0x85, SettingsFieldWidth::Byte},  {0x5C, SettingsFieldWidth::Dword},
    {0x58, SettingsFieldWidth::Dword}, {0x60, SettingsFieldWidth::Byte},
    {0x64, SettingsFieldWidth::Float}, {0x68, SettingsFieldWidth::Dword},
    {0x6C, SettingsFieldWidth::Byte},  {0x70, SettingsFieldWidth::Dword},
    {0x74, SettingsFieldWidth::Byte},  {0x78, SettingsFieldWidth::Dword},
    {0x7C, SettingsFieldWidth::Byte},  {0x04, SettingsFieldWidth::Dword},
    {0x08, SettingsFieldWidth::Byte},  {0x09, SettingsFieldWidth::Byte},
    {0x10, SettingsFieldWidth::Byte},
};

// Between the vector call (008D4D1B) and the NativeString call (008D4DB1).
// +90h is compared a second time here; the native repeats it.
inline constexpr SettingsComparedField kSettingsEqualityMiddle[] = {
    {0x86, SettingsFieldWidth::Byte},  {0x94, SettingsFieldWidth::Byte},
    {0x88, SettingsFieldWidth::Dword}, {0x8C, SettingsFieldWidth::Byte},
    {0x8D, SettingsFieldWidth::Byte},  {0x90, SettingsFieldWidth::Dword},
    {0xB0, SettingsFieldWidth::Byte},  {0xB2, SettingsFieldWidth::Byte},
};

// After the NativeString call, ending at 008D4DD6.
inline constexpr SettingsComparedField kSettingsEqualityTail[] = {
    {0xB1, SettingsFieldWidth::Byte}, {0x1D, SettingsFieldWidth::Byte},
    {0x1C, SettingsFieldWidth::Byte},
};

// 008D4AF0: __thiscall(ECX = this, [ESP+4] = other), RET 4, EAX 1 or 0.
// The +B4h text uses 00435C40 (length equality, then __stricmp). Fields the
// body never reads, among them +0Ch hints, +80h marker alpha and +95h
// HardwareReported, do not affect the result.
bool game_settings_equal_008d4af0(
    const GameSettingsComparand& self, const GameSettingsComparand& other);

// ---------------------------------------------------------------------------
// Weather state on the WeatherConfig at game+21C4h (segment 3).
// ---------------------------------------------------------------------------

// The camera mover as 00445790 uses it: vtable +12Ch returns the view-mode
// name as a C string; 0042C4C0 (non-virtual, ECX = mover, six floats by value,
// RET 18h) stores amplitudes at +1D0h..+1D8h and frequencies at +1DCh..+1E4h,
// the block 004329D0 advances (docs/MISSION_CAMERA.md step 13).
struct WeatherShakeTarget {
    virtual ~WeatherShakeTarget() = default;
    virtual const char* view_mode_name_12c() = 0;
    virtual void set_shake_0042c4c0(const std::array<float, 3>& amplitudes,
        const std::array<float, 3>& frequencies) = 0;
};

// 00445790: ECX = config, [ESP+4] = mover (may be null), RET 4. A null mover
// does nothing. Otherwise amplitude = base_amplitudes_44 * m, with
// m = view_mode_amp_multipliers_5c[view] * weather_amp_multipliers_68[current],
// the weather map looked up first; both lookups insert a missing key as 0.0f.
// The +74h/+78h "current multiplier" words are not written here. Callers:
// 00445A30, BSP_Game_SetCameraMover 004BC410, BSP_MovCam_EnterMode 007A5110.
void apply_weather_camera_shake_00445790(WeatherConfig& config,
    WeatherShakeTarget* mover, NativeStringStorage& strings);

// 00445A30: ECX = config, NativeString by value (callee-destroyed), RET 8.
// current_weather_2c = name (self-copy guarded; resize with preserve, then a
// length-byte copy), current_weather_modifier_34 = weather_recon_modifiers_38
// [name] (inserting 0.0f when absent), then 00445790 on the game's current
// camera mover [game+1ED4h]. Callers: luaMW_SetWeather 008B21A0 (cc9's mission
// host) and the unreferenced reader 00445BF0. +34h feeds recon detection
// (docs/GUNNERY_RECON_DETECTION.md); its consumers are cc9's.
void set_current_weather_00445a30(WeatherConfig& config, NativeString name,
    WeatherShakeTarget* current_mover, NativeStringStorage& strings);

// What 00445BF0 asks of the GuiLuaReader-style archive (vtable +10h,
// 00BD6830 on the Lua reader): key {0, "dayTime"} into a Float field, key
// {0, "actWeather"} into a String field. Absent keys are the reader's business.
struct WeatherStateReader {
    virtual ~WeatherStateReader() = default;
    virtual void read_day_time(float& destination) = 0;              // 00CE47B4
    virtual void read_active_weather(NativeString& destination) = 0; // 00CE47A8
};

inline constexpr float kWeatherDayHoursF = 24.0f;   // 00CE4788
inline constexpr double kWeatherDayHoursD = 24.0;   // 00CE4790

// 00445BF0: ECX = config, [ESP+4] = reader, RET 4. No rel32 or absolute-dword
// reference exists (callsite_census), so it is not in the 00CE4934 vtable and
// never runs. Reads +04h and the active weather, applies 00445A30, then wraps
// +04h: add 24.0 while 0 > t, subtract while t > 24.0f (each step rounded to
// float; a NaN leaves both loops at once).
void read_weather_state_00445bf0(WeatherConfig& config, WeatherStateReader& reader,
    WeatherShakeTarget* current_mover, NativeStringStorage& strings);

// ---------------------------------------------------------------------------
// WarningManager voice/text reports (segment 64). The manager is the singleton
// [00F8A0C4] = game+21E0h; report_warning_009763e0 (bsp/mission_events.hpp)
// is the queue step these feed. Reconstruct-only: no host calls these yet.
// ---------------------------------------------------------------------------

using WarningEntity = const void*; // opaque native entity pointer

// Entity bytes the report guard reads (00977050 template).
struct WarningEntityFlags {
    bool present_5c{false};
    bool flag_5d{false};
    bool flag_60{false};
    bool flag_5e{false};
};

struct WarningReportHost {
    virtual ~WarningReportHost() = default;
    // EnterCriticalSection on manager+24h (when nonnull) and its +18h count.
    virtual void lock_24() = 0;
    virtual void unlock_24() noexcept = 0;
    virtual bool suppressed_d0() = 0;                 // manager+D0h
    virtual std::int32_t local_slot_18ec() = 0;       // game+18ECh
    virtual float mission_clock_f876a4() = 0;
    virtual WarningEntityFlags entity_flags(WarningEntity) = 0;
    // 0077EDF0 on manager+D4h: map operator[] keyed by the entity, so a miss
    // inserts a zero byte; the report proceeds only when the byte is zero.
    virtual bool entity_suppressed_0077edf0(WarningEntity) = 0;
    // 00965810: the controlled unit [00E188D8], or a kind-0Fh plane whose
    // +9D4h is the controlled unit (docs/WARNING_MANAGER.md section 2).
    virtual bool is_player_controlled_00965810(WarningEntity) = 0;
    virtual bool is_kind_5c(WarningEntity, std::int32_t kind) = 0; // vtable +5Ch
    // operator_new(84h), 00974150(entity, id) and 009763E0. A failed
    // allocation reports null in the native.
    virtual void report_entity_warning_00974150(WarningEntity, std::string_view id) = 0;
};

inline constexpr float kWarningReportGrace = 4.0f; // 00CE3D34, clock must exceed it

// The shared guard and report. `excluded_kind` (torpedo only) is tested last,
// after 00965810, as at 0097776C. Returns whether a warning was reported.
bool raise_entity_warning(WarningReportHost& host, WarningEntity entity,
    std::string_view id, std::int32_t excluded_kind = -1);

inline constexpr std::string_view kWarningIdSubmarineAirLow = "submarineairlow";             // 00977050
inline constexpr std::string_view kWarningIdSubmarineAirCritical = "submarineaircritical";   // 009771E0
inline constexpr std::string_view kWarningIdSubmarineDepthDamage = "submarinedepthdamage";   // 00977370
inline constexpr std::string_view kWarningIdSubmarinePeriscopeBroken = "submarineperiscopebroken"; // 00977500
inline constexpr std::string_view kWarningIdTorpedo = "torpedo";                             // 00977690
inline constexpr std::int32_t kTorpedoWarningExcludedKind = 8;                               // 00977771

// __thiscall(ECX = manager, [ESP+4] = entity), RET 4 each.
bool report_submarine_air_low_00977050(WarningReportHost&, WarningEntity);
bool report_submarine_air_critical_009771e0(WarningReportHost&, WarningEntity);
bool report_submarine_depth_damage_00977370(WarningReportHost&, WarningEntity);
bool report_submarine_periscope_broken_00977500(WarningReportHost&, WarningEntity);
bool report_torpedo_00977690(WarningReportHost&, WarningEntity);

// Message ids that Warning subclass constructors write into Warning+4h.
// `network_session` is game+1FE4h != 0.
std::string warning_id_newunit_009756a0();                                    // vtable 00D1B460
std::string warning_id_powerup_009757b0(bool network_session);                // vtable 00D1B4B0
std::string warning_id_named_powerup_009758e0(std::string_view name, bool network_session);
std::string warning_id_score_00975a10();                                      // vtable 00D1B4D4

struct WarningSubjectHost {
    virtual ~WarningSubjectHost() = default;
    virtual bool is_kind_5c(WarningEntity, std::int32_t kind) = 0;
    virtual WarningEntity plane_owner_9d4(WarningEntity) = 0;   // entity+9D4h
    virtual WarningEntity controlled_unit_e188d8() = 0;
    virtual std::int32_t field_3cc(WarningEntity) = 0;          // entity+3CCh
    // 00425850 BSP_NativeString_EqualsCStringInsensitive (ECX = string)(text), RET 4.
    virtual bool native_string_equals_ci_00425850(const std::string& string, const char* text) = 0;
};

// 00975340: __thiscall(ECX = warning, entity, bool exiting), RET 8; vtable
// 00D1B344, secondary 00D1B32C. "exitzone"; unless exiting, + "enter", and
// when the entity (a kind-0Fh plane's +9D4h owner) is the controlled unit,
// + "player" and, for a kind-6 entity, + "ship".
std::string warning_id_exit_zone_00975340(WarningSubjectHost&, WarningEntity entity,
    bool exiting);

// 00975540 BSP_ConfirmWarning_Construct: __thiscall(ECX = warning, entity,
// NativeString* order), RET 8, returns the warning. Listing 00975540-0097569B:
// 00973BA0(ECX warning)(entity, 0); vtable 00D1B380 at +0h, then 00D1B368 at
// +6Ch (no type store). The id at +4h: kind 6 (entity vtable+5Ch) ->
// "confirmship" (0041DD40 resize to 0Bh, memcpy 00D1B44C); else kind 18h ->
// "confirmplane" (0041E350, 00D1B43C), the order appended when non-empty
// (0041DD40 + memcpy), then "player" (00D199D0: a temporary 0041E870,
// 00425E10 append, released through 00419CC0/00BD1510) when the order equals
// "land" (00CFB614) or "retreat" (00CFB5D4) under 00425850 and entity+3CCh
// == 1. Neither kind leaves the base's id. The NativeString steps are folded
// into std::string operations on WarningObject::id, as the other warning
// constructors in engine_seg49_64_78_cc13.cpp do. Reached only through the
// new_confirm_warning_00975540 hook (operator new + this constructor).
cc13_seg49_64_78::WarningObject& construct_confirm_warning_00975540(cc13_seg49_64_78::WarningObject& w,
    cc13_seg49_64_78::WarningHost& host, WarningSubjectHost& subject, WarningEntity entity,
    const std::string& order);

// Hit feedback on manager+174h..+180h, from the hit channel 00988510.
struct WarningHitFeedbackState {
    float deadline_174{0.0f};
    std::int32_t kind_178{0};      // 1 damaging, 2 ineffective; 00968550 clears it
    float text_deadline_17c{0.0f};
    std::int32_t text_state_180{0}; // 1 ineffective text shown, 2 friendly text
};

struct WarningTextHost {
    virtual ~WarningTextHost() = default;
    // 00682990(NativeString* key, float rgba[4], int mode).
    virtual void post_text_00682990(std::string_view key, const std::array<float, 4>& rgba,
        std::int32_t mode) = 0;
};

inline constexpr double kHitFeedbackWindow = 0.25; // 00D7A348
inline constexpr std::array<float, 4> kFriendlyFireRgba{0.1f, 0.9f, 0.1f, 1.0f};    // 00D7A2F0, 00CE3860
inline constexpr std::array<float, 4> kIneffectiveFireRgba{0.9f, 0.9f, 0.9f, 1.0f}; // 00CE3860, 00D7A24C

// 0096E8F0: __fastcall(ECX = manager). Text state 2, deadline now + 0.25,
// posts "ingame.warning_friendly" in mode 2.
void post_friendly_fire_warning_0096e8f0(WarningHitFeedbackState&, float clock,
    WarningTextHost&);
// 0096EF60: __thiscall(ECX = manager, bool damaging, bool friendly), RET 8.
// damaging is the hit's +48h > 0.0 at 00988808. Friendly goes to 0096E8F0;
// otherwise an ineffective hit with no text showing posts
// "ingame.warning_ineffective" in mode 1.
void report_hit_feedback_0096ef60(WarningHitFeedbackState&, float clock, bool damaging,
    bool friendly, WarningTextHost&);

} // namespace bsp
