#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/app_bootstrap.hpp"
#include "bsp/locale_tables.hpp"

// Packet options_settings_commit. The settings object is the static
// cGameSettings at 00f88980 that docs/APP_INIT_BOOTSTRAP.md loads from
// options.txt; this header adds the rest of its layout, the persistence key
// table at 008d64a0, the four reset writers, and the commit paths 008d5b50
// and 008d5430. See docs/OPTIONS_SETTINGS_COMMIT.md.
//
// Nothing here is ABI compatible: bsp::GameSettings is already a projection,
// and the sub-structs below group native offsets by subsystem rather than
// mirroring the object.
namespace bsp {

struct GuiLuaVariant;

// Variant tag written into the first dword of the 8-byte value pairs the
// serializer at 008d64a0 pushes (008d6a8e writes 0 for a string, 008d6aa8 1
// for an int, 008d662d 2 for a float, 008d64eb 3 for a bool).
enum class SettingsValueType : std::uint32_t {
    String = 0,
    Int = 1,
    Float = 2,
    Bool = 3,
};

// One 8-byte native variant. Only the member selected by `type` is meaningful.
struct SettingsValue {
    SettingsValueType type{SettingsValueType::Bool};
    bool boolean{false};
    int integer{0};
    float real{0.0f};
    const char* text{""};

    static SettingsValue from_bool(bool value) noexcept;
    static SettingsValue from_int(int value) noexcept;
    static SettingsValue from_float(float value) noexcept;
    static SettingsValue from_text(const char* value) noexcept;
    bool operator==(const SettingsValue& other) const noexcept;
};

// One row of the key table 008d64a0 walks in body order. `offset` is the
// native field offset inside the settings object, kept because the recovered
// name is a hypothesis and the offset is not. `omit_when_default` marks the
// rows guarded by the equality test at 00bd5680: those are skipped when the
// field still equals `default_value`. Float rows carry no guard and are always
// written.
struct SettingsKey {
    const char* name;
    std::uint16_t offset;
    SettingsValueType type;
    bool omit_when_default;
    SettingsValue default_value;
};

// Section names pushed through the writer's virtual +4h.
inline constexpr char kSettingsSectionName[] = "Options";              // 00d15d50
inline constexpr char kKeyboardSetupSectionName[] = "keyboardSetup";   // 00d15c94
inline constexpr char kDownloadedContentSectionName[] = "DownloadedContent"; // 00d15bfc

// The 24 scalar rows of 008d64a0, in the order the body emits them. The
// keyboardSetup and DownloadedContent sections are not scalar rows and are
// handled by write_settings_008d64a0 directly.
const std::vector<SettingsKey>& settings_persistence_keys_008d64a0();

// Game page fields. Offsets are inside the settings object.
struct GameplaySettings {
    int language_index_04{0};   // +04h, index into the 0x20-stride table at 00f88974
    bool imperial_08{true};     // +08h, "imperial", default 1
    bool subtitle_09{false};    // +09h, "subtitle", default 0
    int hints_0c{2};            // +0Ch, "hints", default 2, range 0..2
    bool camera_shake_10{true}; // +10h, "cameraShake", default 1
    bool target_indicator_4a{true}; // +4Ah, "TargetIndicator", default 1
    bool water_drops_7c{true};  // +7Ch, "waterDrops"
    float marker_alpha_80{0.0f};// +80h, "markerAlpha"
    bool cockpit_mode_b2{true}; // +B2h, "CockpitMode"
    bool show_safe_area_b1{true};   // +B1h, "ShowSafeArea"
    int clan_text_b8{0};        // +B8h, "ClanText"
};

// Audio page fields. All four volumes are written by the audio reset 008d41f0.
struct AudioSettings {
    float master_20{0.5f};  // +20h, "masterVolume"
    bool enabled_24{true};  // +24h, sound-system enable; the SoundEnabled token has no handler
    float music_28{0.5f};   // +28h, "musicVol"
    float effects_2c{0.5f}; // +2Ch, "sfxVol"
    float speech_30{0.5f};  // +30h, "speechVol"
};

// Control page fields, +40h..+44h plus the compatibility byte.
struct ControlSettings {
    bool rumble_off_40{false};        // +40h, "rumbleOff"
    bool swap_sticks_41{true};        // +41h, "swapSticks"
    bool invert_camera_y_42{false};   // +42h, "invertCameraY"
    bool invert_plane_y_43{false};    // +43h, "invertPlaneY"
    bool swap_map_sticks_44{true};    // +44h, "swapMapSticks"
    bool xbox_compatibility_b0{false};// +B0h, "XboxCompatibilityMode"
};

// Video fields that bsp::GameSettings does not already carry.
struct PresentationSettings {
    float gamma_64{0.0f};       // +64h, "gamma", pushed to renderer virtual +F0h
    float unknown_34{0.0f};     // +34h, set to the constant at 00ce7d20 by 008d4520
    int unknown_70{0};          // +70h, latched in 00f88a40; not persisted
    int shader_flag_8c{1};      // +8Ch, paired with +8Dh in 00b107f0; not persisted
    bool motion_blur_8d{true};  // +8Dh, "MotionBlur", mirrored into 00f8d39c+219h
    int old_film_effect_90{1};  // +90h, "oldFilmEffect", pushed to 00f8d39c+220h
    bool hardware_reported_95{false}; // +95h, "HardwareReported", default 0
};

// The whole settings object. `options_file` is the projection
// docs/APP_INIT_BOOTSTRAP.md already reconstructed (+14h, +18h, +1Ch, +1Dh,
// +1Eh, +54h, +58h, +5Ch, +60h, +68h, +6Ch, +74h, +78h, +84h, +85h, +86h,
// +88h, +94h and the language name); the four members beside it hold the
// offsets that only the options screens and the serializer touch.
struct GameSettingsBlock {
    GameSettings options_file{};
    GameplaySettings gameplay{};
    AudioSettings audio{};
    ControlSettings control{};
    PresentationSettings presentation{};
    // +98h data / +9Ch count, the stride-8 vector emitted under the
    // DownloadedContent section. Each entry's +4h is a char*.
    std::vector<std::string> downloaded_content{};
};

// 008d41c0 game reset, __thiscall(void), RET. Writes +08h, +09h, +0Ch, +10h,
// +4Ah, +B2h, +7Ch and +80h and leaves every other field alone.
void reset_game_defaults_008d41c0(GameSettingsBlock& settings) noexcept;

// 008d41f0 audio reset, __thiscall(void), RET. Stores the constant at 00ce3800
// (0.5f) into +20h, +28h, +2Ch and +30h.
inline constexpr float kDefaultVolume = 0.5f; // 00ce3800
void reset_audio_defaults_008d41f0(GameSettingsBlock& settings) noexcept;

// 008d4520 video reset, __thiscall(bool reset_resolution, bool keep_fullscreen),
// RET 8. The second flag is inverted in the body: fullscreen is forced on only
// when the argument is zero. The call sites pass (0, 1).
inline constexpr float kVideoResetUnknown34 = 0.698161f; // 00ce7d20, 0x3f32b8c3
void reset_video_defaults_008d4520(
    GameSettingsBlock& settings, bool reset_resolution, bool keep_fullscreen) noexcept;

// 008d4820 control reset, __thiscall(void), RET. Writes +40h..+44h, then tail
// calls 008d45d0 when the Xenon system manager at 00f8abe8 reports state 2 and
// a selected user. That tail is not recovered; the flag reports whether the
// native body would have taken it.
bool reset_control_defaults_008d4820(
    GameSettingsBlock& settings, bool xenon_state_is_two, bool xenon_user_selected) noexcept;

// 008d4210, __thiscall(const cGameSettings* other), RET 4. True when the shader
// model at +88h or the flag at +8Ch differ. The options commit 005f65c0 calls
// it against a snapshot before deciding what a save has to warn about.
bool video_mode_differs_008d4210(
    const GameSettingsBlock& lhs, const GameSettingsBlock& rhs) noexcept;

// 008d4260, __thiscall(void), RET, returns a localisation key. Non-zero +08h
// selects the metric key.
inline constexpr char kUnitKeyMetric[] = "FE.opt_game_metric";     // 00d15aac
inline constexpr char kUnitKeyImperial[] = "FE.opt_game_imperial"; // 00d15ac0
const char* unit_label_key_008d4260(const GameSettingsBlock& settings) noexcept;

// 008d48c0, __thiscall(void), RET. Indexes the language table with stride 0x20
// and returns entry +0Ch, the lockit id; a null pointer falls back to the empty
// buffer at 00f88a3c, so an out-of-range index yields "" here.
std::string language_lockit_id_008d48c0(
    const GameSettingsBlock& settings, const std::vector<LanguageEntry>& table);

// 008d56c0, __thiscall(native_string name), RET 4. Scans the language table for
// an entry whose lanfile matches and stores its index into +04h. No match
// leaves the index untouched; the return reports whether one was found. The
// installed options.txt value is a single token such as "englishauthentic".
bool select_language_by_name_008d56c0(
    GameSettingsBlock& settings, const std::vector<LanguageEntry>& table, const std::string& name);

// The visitor the serializer drives. Native vtable slots: +4h begin_section,
// +8h end_section, +0Ch write_field, each __thiscall with the 8-byte variants
// passed by value.
class SettingsWriter {
public:
    virtual ~SettingsWriter() = default;
    // Required first call008d64a9: persist options.txt before archive fields.
    virtual void write_options_text_008d6170() = 0;
    virtual void begin_section(const char* name) = 0;   // virtual +4h
    // The keyboard binding slots at 006a533d use integer variant keys 0 and 1.
    // A string-only implementation cannot preserve their archive meaning.
    virtual void begin_section(const GuiLuaVariant& key) = 0;
    virtual void end_section() = 0;                     // virtual +8h
    virtual void write_field(const char* key, const SettingsValue& value) = 0; // virtual +0Ch
    // 006a51c0 on the input-settings singleton, the keyboardSetup body.
    // write_keyboard_setup_006a51c0 implements this traversal over InputSettings.
    virtual void write_keyboard_setup() = 0;
};

// 008d64a0, __thiscall(SettingsWriter* writer), RET 4. Writes the options text
// file through008d6170 first, opens the Options section, emits the scalar rows
// that differ from their defaults, delegates keyboardSetup, then emits the
// downloaded-content list as index rows and closes both sections.
void write_settings_008d64a0(const GameSettingsBlock& settings, SettingsWriter& writer);

// Globals 00f88a3d, 00f88a40, 00f88a44 and the seeded-bit set 00f88a48. The
// commit seeds each one from the settings object the first time and then uses
// it as the previous value, so a change is detected only from the second
// commit onward.
struct SettingsCommitLatches {
    bool motion_blur_seeded{false};       // 00f88a48 bit 0
    bool motion_blur_8d{false};           // 00f88a44
    bool unknown_70_seeded{false};        // 00f88a48 bit 1
    int unknown_70{0};                    // 00f88a40
    bool shadow_seeded{false};            // 00f88a48 bit 2
    bool shadow_84{false};                // 00f88a3d
};

// Integration boundary for the audio commit. One method per native call site,
// in body order.
class SettingsAudioHost {
public:
    virtual ~SettingsAudioHost() = default;
    // 00f8bbd8 +70h = settings +24h, a plain store ahead of the volume calls.
    virtual void sound_set_enabled(bool enabled) = 0;
    // 00a7a440 on 00f8bbd8: stores +4Ch then applies group mask 0xFFFF.
    virtual void sound_set_master_volume(float volume) = 0;
    // 00f8bbcc +218h and +21Ch, two direct stores.
    virtual void sound_store_music_volume(float volume) = 0;
    virtual void sound_store_speech_volume(float volume) = 0;
    // 00a864f0 on (00e198ac)+50h, guarded by both pointers being non-null.
    virtual void music_player_set_volume(float volume) = 0;
    // 00a7abf0 on 00f8bbd8, called once with mask 0xFFFF.
    virtual void sound_set_group_volume(std::uint32_t group_mask, float volume) = 0;
    // 00a7acf0 then 00a7f8e0 on 00f8bbd8, once per named bus.
    virtual void sound_set_named_bus_volume(const char* bus, float volume) = 0;
    // 004c1710 singleton +10h, the master volume clamped to [0, 1].
    virtual void ui_sound_set_master_volume(float volume) = 0;
};

inline constexpr std::uint32_t kAllSoundGroups = 0xFFFFu; // 008d54d4
inline constexpr char kBusWarnings[] = "Warnings";        // 00ce7848, speech volume
inline constexpr char kBusGuiTestSpeech[] = "GUITestSpeech"; // 00cef890, speech volume
inline constexpr char kBusGuiMusic[] = "GUIMusic";        // 00d15b08, music volume

// 008d5430, __thiscall(void), RET. Runs on every volume keystroke on the audio
// page and as the first statement of the full commit.
void apply_audio_settings_008d5430(const GameSettingsBlock& settings, SettingsAudioHost& host);

// Integration boundary for the full commit. One method per native call site, in
// body order. Nothing has a default implementation.
class SettingsApplyHost : public SettingsAudioHost {
public:
    // 00b1ffb0 on 00f8d394: renderer +1D84h = 2 - texture detail.
    virtual void renderer_set_texture_detail_bias(int bias) = 0;
    // Renderer virtual +F0h with settings +64h.
    virtual void renderer_set_gamma(float gamma) = 0;
    // 00699b80 on (00e188a8)+3Ch: stores the four bytes at +4C9h..+4CCh and,
    // when any changed and +520h is set, reloads the bindings.
    virtual void input_set_stick_modifiers(
        bool swap_sticks, bool invert_camera_y, bool invert_plane_y, bool swap_map_sticks) = 0;
    // 004dcdf0 on 00e188a8, no arguments.
    virtual void game_refresh_after_settings() = 0;
    // (00e188a8)+19FCh +4Ch = the detail scale selected by object detail.
    virtual void foliage_system_set_detail_scale(float scale) = 0;
    // ((00e188a8)+19F0h)+A8h +F8h = reflection.
    virtual void ocean_set_reflection_enabled(bool enabled) = 0;
    // 004c1710 singleton +14h = subtitles.
    virtual void ui_set_subtitles_enabled(bool enabled) = 0;
    // 007371d0 then 00ac3610: the font registry entry for "Fonts\" plus the
    // language font path.
    virtual void font_registry_set_language_path(const char* prefix, const char* font_path) = 0;
    // 00aa09d0 on 00f8bc4c with the language lanfile.
    virtual void localization_load_table(const char* lanfile) = 0;
    // 00a94c50 with rumble enabled = !(game+634h) && !rumble_off.
    virtual void set_rumble_enabled(bool enabled) = 0;
    // 00f8a304 virtuals +24h, +14h(settings+98h) and +20h, taken only when
    // 00f8a304 is non-null and its byte +1Ch is set.
    virtual void downloadable_content_rebind() = 0;
    // 00aa0d30 then 00aa06d0 on 00f8bc4c, the "globals" table reload.
    virtual void localization_reload_globals() = 0;
    // 00b72270 lookup of "FoliageGroup" then 00b6da70(node, factor, 1).
    virtual void foliage_group_set_visibility(float factor) = 0;
    // (00e188a8)+19E8h child +3Ch then 00bbcfe0 with the cloud flag.
    virtual void clouds_set_enabled(bool enabled) = 0;
    // ((00e188a8)+19F0h)+A8h virtual +10h with settings +70h.
    virtual void ocean_set_unknown_70(int value) = 0;
    // 00f8bbf0 virtual +8h with settings +84h.
    virtual void shadow_system_set_enabled(bool enabled) = 0;
    // 00b0d580 on 00f8d39c: stores +219h then rebuilds the render targets.
    virtual void renderer_set_motion_blur(bool enabled, int sample_count) = 0;
    // Renderer virtual +80h, the current back-buffer size.
    virtual Resolution renderer_current_resolution() = 0;
    // 00b0fc00 on 00f8d39c, taken only when the current size differs.
    virtual void renderer_release_size_dependent_targets() = 0;
    // 00b29e60. The last argument is the accumulated change flag below.
    virtual void renderer_change_presentation_mode(int width, int height, bool fullscreen,
        int sample_count, bool vsync, bool subsystem_changed) = 0;
    // 0109cf04 virtual +0Ch with fullscreen, width, height.
    virtual void platform_window_set_mode(bool fullscreen, int width, int height) = 0;
    // XLiveOnResetDevice(00f8d394 + 1A28h), taken only when 00e198c4 is null.
    virtual void xlive_on_reset_device() = 0;
    // 00b107f0 on 00f8d39c with +8Ch, +8Dh and the antialias sample count.
    virtual void renderer_rebuild_post_effects(int shader_flag, bool motion_blur, int sample_count) = 0;
    // 004c12b0 then 00aa4ef0, taken only when 00e198c4 is null.
    virtual void gui_manager_reload() = 0;
    // 00b0d080 on 00f8d39c: +220h = old film effect.
    virtual void renderer_set_old_film_effect(int mode) = 0;
    // 00439100 copied into the global native string at 0108ff24/0108ff28.
    virtual void publish_module_directory(const std::string& path) = 0;
};

// State the commit reads from outside the settings object. Grouping them keeps
// the reconstruction from inventing globals.
struct SettingsApplyEnvironment {
    bool downloadable_content_present{false}; // 00f8a304 != 0
    bool downloadable_content_ready{false};   // (00f8a304)+1Ch
    bool xenon_manager_present{false};        // 00f8abe8 != 0
    bool xenon_user_selected{false};          // 00a3e510
    bool game_present{false};                 // 00e188a8 != 0
    bool foliage_system_present{false};       // (00e188a8)+19FCh
    bool ocean_present{false};                // (00e188a8)+19F0h
    bool scene_present{false};                // (00e188a8)+19ECh
    bool cloud_system_present{false};         // (00e188a8)+19E8h
    bool rumble_blocked_by_game{false};       // (00e188a8)+634h
    bool xlive_suppressed{false};             // 00e198c4 != 0
    std::string module_directory{};           // 00439100
};

// The three detail scales indexed by object detail at 008d5c24, from 00ce3800,
// 00ce3e18 and 00d7a24c.
const float* foliage_detail_scales_008d5b50() noexcept;

// 008d5b50, __thiscall(void), RET. The single commit point: the options screens
// call it on reset, on apply-and-save and on the clean back path, and the
// bootstrap calls it from GGame::OnInit. Returns the change flag the native
// body accumulates in a stack byte and passes as the last argument of
// renderer_change_presentation_mode.
bool apply_all_settings_008d5b50(const GameSettingsBlock& settings,
    const std::vector<LanguageEntry>& languages, const SettingsApplyEnvironment& env,
    SettingsCommitLatches& latches, SettingsApplyHost& host);

// 008d44c0, __thiscall(bool enabled), RET 4. Stores +B0h, mirrors it into
// 0108ff20 and, when the console command sink at ((00e188a8)+1A08h)+4h exists,
// pushes one of the two literals through 006b8ad0(text, 0, 0, 2).
inline constexpr char kX360CompatibilityOn[] = "X360COMP=true";   // 00cf7f38
inline constexpr char kX360CompatibilityOff[] = "X360COMP=false"; // 00cf7f28
const char* set_xbox_compatibility_008d44c0(
    GameSettingsBlock& settings, bool enabled, bool command_sink_present) noexcept;
}
