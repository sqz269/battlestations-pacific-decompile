#include "bsp/game_settings.hpp"
#include "bsp/gui_lua_reader.hpp"

#include <algorithm>
#include <cstddef>

namespace bsp {
namespace {
// 008d64eb and the rows around it; the guard call is 00bd5680, an equality test
// between the field variant and a default variant built on the stack.
SettingsKey make_bool(const char* name, std::uint16_t offset, bool default_value) noexcept
{
    return SettingsKey{name, offset, SettingsValueType::Bool, true,
        SettingsValue::from_bool(default_value)};
}

SettingsKey make_bool_always(const char* name, std::uint16_t offset) noexcept
{
    return SettingsKey{name, offset, SettingsValueType::Bool, false, SettingsValue{}};
}

SettingsKey make_int(const char* name, std::uint16_t offset, int default_value) noexcept
{
    return SettingsKey{name, offset, SettingsValueType::Int, true,
        SettingsValue::from_int(default_value)};
}

SettingsKey make_string(const char* name, std::uint16_t offset) noexcept
{
    return SettingsKey{name, offset, SettingsValueType::String, false, SettingsValue{}};
}

SettingsKey make_float(const char* name, std::uint16_t offset) noexcept
{
    return SettingsKey{name, offset, SettingsValueType::Float, false, SettingsValue{}};
}

// Reads one persisted field out of the grouped reconstruction by its native
// offset, so the key table stays the single description of the layout.
SettingsValue read_field(const GameSettingsBlock& s, std::uint16_t offset) noexcept
{
    switch (offset) {
    case 0x08: return SettingsValue::from_bool(s.gameplay.imperial_08);
    case 0x09: return SettingsValue::from_bool(s.gameplay.subtitle_09);
    case 0x0C: return SettingsValue::from_int(s.gameplay.hints_0c);
    case 0x10: return SettingsValue::from_bool(s.gameplay.camera_shake_10);
    case 0x20: return SettingsValue::from_float(s.audio.master_20);
    case 0x28: return SettingsValue::from_float(s.audio.music_28);
    case 0x2C: return SettingsValue::from_float(s.audio.effects_2c);
    case 0x30: return SettingsValue::from_float(s.audio.speech_30);
    case 0x40: return SettingsValue::from_bool(s.control.rumble_off_40);
    case 0x41: return SettingsValue::from_bool(s.control.swap_sticks_41);
    case 0x42: return SettingsValue::from_bool(s.control.invert_camera_y_42);
    case 0x43: return SettingsValue::from_bool(s.control.invert_plane_y_43);
    case 0x44: return SettingsValue::from_bool(s.control.swap_map_sticks_44);
    case 0x4A: return SettingsValue::from_bool(s.gameplay.target_indicator_4a);
    case 0x64: return SettingsValue::from_float(s.presentation.gamma_64);
    case 0x7C: return SettingsValue::from_bool(s.gameplay.water_drops_7c);
    case 0x80: return SettingsValue::from_float(s.gameplay.marker_alpha_80);
    case 0x8D: return SettingsValue::from_bool(s.presentation.motion_blur_8d);
    case 0x90: return SettingsValue::from_int(s.presentation.old_film_effect_90);
    case 0x95: return SettingsValue::from_bool(s.presentation.hardware_reported_95);
    case 0xB0: return SettingsValue::from_bool(s.control.xbox_compatibility_b0);
    case 0xB1: return SettingsValue::from_bool(s.gameplay.show_safe_area_b1);
    case 0xB2: return SettingsValue::from_bool(s.gameplay.cockpit_mode_b2);
    case 0xB4: return SettingsValue::from_text(s.gameplay.clan_text_b4.c_str());
    default: return SettingsValue{};
    }
}
}

SettingsValue SettingsValue::from_bool(bool value) noexcept
{
    SettingsValue v;
    v.type = SettingsValueType::Bool;
    v.boolean = value;
    return v;
}

SettingsValue SettingsValue::from_int(int value) noexcept
{
    SettingsValue v;
    v.type = SettingsValueType::Int;
    v.integer = value;
    return v;
}

SettingsValue SettingsValue::from_float(float value) noexcept
{
    SettingsValue v;
    v.type = SettingsValueType::Float;
    v.real = value;
    return v;
}

SettingsValue SettingsValue::from_text(const char* value) noexcept
{
    SettingsValue v;
    v.type = SettingsValueType::String;
    v.text = value != nullptr ? value : "";
    return v;
}

bool SettingsValue::operator==(const SettingsValue& other) const noexcept
{
    if (type != other.type) {
        return false;
    }
    switch (type) {
    case SettingsValueType::Bool: return boolean == other.boolean;
    case SettingsValueType::Int: return integer == other.integer;
    case SettingsValueType::Float: return real == other.real;
    case SettingsValueType::String: break;
    }
    const char* a = text != nullptr ? text : "";
    const char* b = other.text != nullptr ? other.text : "";
    return std::string(a) == b;
}

const std::vector<SettingsKey>& settings_persistence_keys_008d64a0()
{
    // Body order, 008d64cd through 008d6a48. Offsets and defaults come from the
    // listing, not from the pseudocode, whose stack slots shift across the
    // pushes that build the two variants.
    static const std::vector<SettingsKey> table = {
        make_bool("imperial", 0x08, true),          // 008d64cd, 00d15d44
        make_bool("subtitle", 0x09, false),         // 008d6514, 00d15d38
        make_int("hints", 0x0C, 2),                 // 008d6555, 00cf1568
        make_bool("cameraShake", 0x10, true),       // 008d659f, 00d15d2c
        make_bool("TargetIndicator", 0x4A, true),   // 008d65e1, 00d15d1c
        make_float("masterVolume", 0x20),           // 008d6623, 00d15d0c
        make_float("musicVol", 0x28),               // 008d6651, 00d15d00
        make_float("sfxVol", 0x2C),                 // 008d667d, 00d15cf8
        make_float("speechVol", 0x30),              // 008d66a9, 00d15cec
        make_bool_always("rumbleOff", 0x40),        // 008d66d5, 00d15ce0
        make_bool("swapSticks", 0x41, false),       // 008d66f9, 00d15cd4
        make_bool("invertCameraY", 0x42, true),     // 008d6738, 00d15cc4
        make_bool("invertPlaneY", 0x43, true),      // 008d677a, 00d15cb4
        make_bool("swapMapSticks", 0x44, false),    // 008d67bc, 00d15ca4
        make_bool("HardwareReported", 0x95, false), // 008d6827, 00d15c80
        make_bool("waterDrops", 0x7C, true),        // 008d686c, 00d15c74
        make_int("oldFilmEffect", 0x90, 1),         // 008d68ae, 00d15c64
        make_bool("MotionBlur", 0x8D, true),        // 008d68f7, 00ce8fec
        make_float("gamma", 0x64),                  // 008d693c, 00d15c5c
        make_float("markerAlpha", 0x80),            // 008d696c, 00d15c50
        make_bool_always("XboxCompatibilityMode", 0xB0), // 008d699f, 00d15c38
        make_bool_always("ShowSafeArea", 0xB1),     // 008d69ca, 00d15c28
        make_bool_always("CockpitMode", 0xB2),      // 008d69f5, 00d15c1c
        make_string("ClanText", 0xB4),             // 008d6a25 tag0, +B8h text
    };
    return table;
}

void reset_game_defaults_008d41c0(GameSettingsBlock& settings) noexcept
{
    settings.gameplay.imperial_08 = true;         // 008d41c5
    settings.gameplay.subtitle_09 = false;        // 008d41c8
    settings.gameplay.hints_0c = 2;               // 008d41cc
    settings.gameplay.camera_shake_10 = true;     // 008d41d3
    settings.gameplay.target_indicator_4a = true; // 008d41d6
    settings.gameplay.cockpit_mode_b2 = true;     // 008d41d9
    settings.gameplay.water_drops_7c = true;      // 008d41df
    settings.gameplay.marker_alpha_80 = 0.0f;     // 008d41e2, XORPS
}

void reset_audio_defaults_008d41f0(GameSettingsBlock& settings) noexcept
{
    settings.audio.master_20 = kDefaultVolume;  // 008d41f8
    settings.audio.music_28 = kDefaultVolume;   // 008d41fd
    settings.audio.effects_2c = kDefaultVolume; // 008d4202
    settings.audio.speech_30 = kDefaultVolume;  // 008d4207
}

void reset_video_defaults_008d4520(
    GameSettingsBlock& settings, bool reset_resolution, bool keep_fullscreen) noexcept
{
    if (reset_resolution) { // 008d4522
        settings.options_file.resolution_index_78 = 0;
        settings.options_file.width_14 = kFallbackResolution.width;
        settings.options_file.height_18 = kFallbackResolution.height;
    }
    settings.options_file.antialias_index_5c = 0;  // 008d4545
    settings.options_file.antialias_58 = 0;        // 008d4548
    settings.options_file.vsync_60 = true;         // 008d454b
    settings.presentation.gamma_64 = 0.0f;         // 008d454e, XORPS
    settings.options_file.texture_detail_68 = 2;   // 008d4553
    settings.options_file.hires_shadow_1d = false; // 008d455a
    settings.options_file.no_lod_1c = false;       // 008d455d
    settings.options_file.object_detail_54 = 2;    // 008d4560
    settings.options_file.shadow_84 = true;        // 008d4567
    settings.options_file.shadow_85 = false;       // 008d456d
    settings.options_file.reflection_6c = true;    // 008d4573
    settings.presentation.unknown_70 = 0;          // 008d4576
    settings.options_file.clouds_74 = true;        // 008d4579
    settings.gameplay.water_drops_7c = true;       // 008d457c
    settings.presentation.shader_flag_8c = 1;      // 008d457f
    settings.presentation.motion_blur_8d = true;   // 008d4585
    settings.presentation.old_film_effect_90 = 1;  // 008d458b
    if (!keep_fullscreen) {                        // 008d4591, JNZ skips the store
        settings.options_file.fullscreen_1e = true;
    }
    settings.presentation.unknown_34 = kVideoResetUnknown34; // 008d4596
    settings.options_file.foliage_86 = true;                 // 008d45a3
    settings.options_file.shader_model_88 = 1;               // 008d45a9
}

bool reset_control_defaults_008d4820(
    GameSettingsBlock& settings, bool xenon_state_is_two, bool xenon_user_selected) noexcept
{
    settings.control.invert_camera_y_42 = false; // 008d4827
    settings.control.invert_plane_y_43 = false;  // 008d482a
    settings.control.swap_sticks_41 = true;      // 008d482d
    settings.control.rumble_off_40 = false;      // 008d4830
    settings.control.swap_map_sticks_44 = true;  // 008d4833
    // 008d4836..008d485c: the tail call to 008d45d0 is not recovered.
    return xenon_state_is_two && xenon_user_selected;
}

bool video_mode_differs_008d4210(
    const GameSettingsBlock& lhs, const GameSettingsBlock& rhs) noexcept
{
    if (lhs.options_file.shader_model_88 != rhs.options_file.shader_model_88) { // 008d421a
        return true;
    }
    return lhs.presentation.shader_flag_8c != rhs.presentation.shader_flag_8c; // 008d422d
}

const char* unit_label_key_008d4260(const GameSettingsBlock& settings) noexcept
{
    return settings.gameplay.imperial_08 ? kUnitKeyMetric : kUnitKeyImperial; // 008d4260
}

std::string language_lockit_id_008d48c0(
    const GameSettingsBlock& settings, const std::vector<LanguageEntry>& table)
{
    const int index = settings.gameplay.language_index_04;
    if (index < 0 || static_cast<std::size_t>(index) >= table.size()) {
        return std::string();
    }
    return table[static_cast<std::size_t>(index)].lockit_id; // entry +0Ch
}

bool select_language_by_name_008d56c0(
    GameSettingsBlock& settings, const std::vector<LanguageEntry>& table, const std::string& name)
{
    for (std::size_t i = 0; i < table.size(); ++i) { // 008d56f8 loop over 00f88978 entries
        if (table[i].lanfile == name) {
            settings.gameplay.language_index_04 = static_cast<int>(i);
            return true;
        }
    }
    return false;
}

void write_settings_008d64a0(const GameSettingsBlock& settings, SettingsWriter& writer)
{
    writer.write_options_text_008d6170(); //008d64a9; before the first archive call
    writer.begin_section(kSettingsSectionName); // 008d64c4
    for (const SettingsKey& key : settings_persistence_keys_008d64a0()) {
        if (key.offset == 0x95) { // 008d67f5..008d681c, before HardwareReported
            writer.begin_section(kKeyboardSetupSectionName);
            writer.write_keyboard_setup();
            writer.end_section();
        }
        const SettingsValue value = read_field(settings, key.offset);
        if (key.omit_when_default && value == key.default_value) {
            continue; // 00bd5680 returned true
        }
        writer.write_field(key.name, value); // virtual +0Ch
    }
    writer.begin_section(kDownloadedContentSectionName); // 008d6a6c
    for (std::size_t i = 0; i < settings.downloaded_content.size(); ++i) {
        writer.write_field(gui_lua_key_by_index(static_cast<std::int32_t>(i)),
            SettingsValue::from_text(settings.downloaded_content[i].c_str()));
    }
    writer.end_section(); // 008d6ac7
    writer.end_section(); // 008d6ad0
}

void apply_audio_settings_008d5430(const GameSettingsBlock& settings, SettingsAudioHost& host)
{
    host.sound_set_enabled(settings.audio.enabled_24);          // 008d5455
    host.sound_set_master_volume(settings.audio.master_20);     // 008d546e
    host.sound_store_music_volume(settings.audio.music_28);     // 008d5483
    host.sound_store_speech_volume(settings.audio.speech_30);   // 008d5490
    host.music_player_set_volume(settings.audio.music_28);      // 008d54ba
    host.sound_set_group_volume(kAllSoundGroups, settings.audio.effects_2c); // 008d54d9
    host.sound_set_named_bus_volume(kBusWarnings, settings.audio.speech_30); // 008d5531
    host.sound_set_named_bus_volume(kBusGuiTestSpeech, settings.audio.speech_30); // 008d55bc
    host.sound_set_named_bus_volume(kBusGuiMusic, settings.audio.music_28);  // 008d5647
    float ui_volume = settings.audio.master_20; // 008d568b clamp to [0, 1]
    // COMISS/JA then COMISS/JBE preserve unordered values and negative zero.
    if (ui_volume < 0.0f) {
        ui_volume = 0.0f;
    } else if (ui_volume > 1.0f) {
        ui_volume = 1.0f;
    }
    host.ui_sound_set_master_volume(ui_volume); // 008d56b0
}

const float* foliage_detail_scales_008d5b50() noexcept
{
    static const float scales[3] = {0.5f, 0.7f, 1.0f}; // 00ce3800, 00ce3e18, 00d7a24c
    return scales;
}

const char* set_xbox_compatibility_008d44c0(
    GameSettingsBlock& settings, bool enabled, bool command_sink_present) noexcept
{
    settings.control.xbox_compatibility_b0 = enabled; // 008d44c6, mirrored into 0108ff20
    if (!command_sink_present) {
        return nullptr; // 008d44d8/008d44e1/008d44ee all fall to the bare RET 4
    }
    return enabled ? kX360CompatibilityOn : kX360CompatibilityOff; // 008d44ff/008d450c
}

bool apply_all_settings_008d5b50(const GameSettingsBlock& settings,
    const std::vector<LanguageEntry>& languages, const SettingsApplyEnvironment& env,
    SettingsCommitLatches& latches, SettingsApplyHost& host)
{
    apply_audio_settings_008d5430(settings, host); // 008d5b6c
    host.renderer_set_texture_detail_bias(2 - settings.options_file.texture_detail_68); // 008d5b80
    host.renderer_set_gamma(settings.presentation.gamma_64); // 008d5b9a, renderer virtual +F0h
    host.input_set_stick_modifiers(settings.control.swap_sticks_41,
        settings.control.invert_camera_y_42, settings.control.invert_plane_y_43,
        settings.control.swap_map_sticks_44); // 008d5bb6
    host.game_refresh_after_settings();       // 008d5bc1

    // 008d5bc6: the 0x844-byte allocation guarded by a missing content object,
    // a live Xenon manager and a selected user is a construction whose body
    // (009955f0) is outside this packet, so it is not modelled.
    (void)env.downloadable_content_present;
    (void)env.xenon_manager_present;
    (void)env.xenon_user_selected;

    if (env.game_present) {
        if (env.foliage_system_present) { // 008d5c1b
            int detail = settings.options_file.object_detail_54;
            detail = std::max(0, std::min(detail, 2));
            host.foliage_system_set_detail_scale(foliage_detail_scales_008d5b50()[detail]);
        }
        if (env.ocean_present) { // 008d5c67
            host.ocean_set_reflection_enabled(settings.options_file.reflection_6c);
        }
    }
    host.ui_set_subtitles_enabled(settings.gameplay.subtitle_09); // 008d5c85

    const int language_index = settings.gameplay.language_index_04;
    const LanguageEntry* entry = nullptr;
    if (language_index >= 0 && static_cast<std::size_t>(language_index) < languages.size()) {
        entry = &languages[static_cast<std::size_t>(language_index)];
    }
    // 008d5c90: an empty font path selects the empty string at 00ce3a0c.
    const std::string font_path = (entry != nullptr) ? entry->font_path : std::string();
    host.font_registry_set_language_path("Fonts\\", font_path.c_str()); // 008d5d1c
    const std::string lanfile = (entry != nullptr) ? entry->lanfile : std::string();
    host.localization_load_table(lanfile.c_str()); // 008d5da1

    // 008d5dc8: rumble is on only when neither the game nor the setting blocks it.
    host.set_rumble_enabled(!env.rumble_blocked_by_game && !settings.control.rumble_off_40);

    if (env.downloadable_content_present) { // 008d5de7
        if (env.downloadable_content_ready) {
            host.downloadable_content_rebind();
        }
        host.localization_reload_globals(); // 008d5e53 then 008d5e81
    }

    if (!latches.motion_blur_seeded) { // 008d5e8b, bit 0
        latches.motion_blur_8d = settings.presentation.motion_blur_8d;
        latches.motion_blur_seeded = true;
    }
    bool subsystem_changed = false; // the stack byte cleared at 008d5ea4
    if (!latches.unknown_70_seeded) { // 008d5ea2, bit 1
        latches.unknown_70 = settings.presentation.unknown_70;
        latches.unknown_70_seeded = true;
    }
    if (!latches.shadow_seeded) { // 008d5ebc, bit 2
        latches.shadow_84 = settings.options_file.shadow_84;
        latches.shadow_seeded = true;
    }

    if (env.scene_present) { // 008d5ed9
        host.foliage_group_set_visibility(settings.options_file.foliage_86 ? 1.0f : 0.0f); // 008d5f67
        if (env.cloud_system_present) { // 008d5f71
            host.clouds_set_enabled(settings.options_file.clouds_74);
        }
        if (env.ocean_present) { // 008d5f96
            host.ocean_set_reflection_enabled(settings.options_file.reflection_6c);
            if (latches.unknown_70 != settings.presentation.unknown_70) {
                host.ocean_set_unknown_70(settings.presentation.unknown_70);
                latches.unknown_70 = settings.presentation.unknown_70;
                subsystem_changed = true; // 008d5fdc
            }
        }
        if (latches.shadow_84 != settings.options_file.shadow_84) { // 008d5fed
            host.shadow_system_set_enabled(settings.options_file.shadow_84);
            latches.shadow_84 = settings.options_file.shadow_84;
            subsystem_changed = true; // 008d6009
        }
        if (latches.motion_blur_8d != settings.presentation.motion_blur_8d) { // 008d6019
            subsystem_changed = true; // 008d6021
            latches.motion_blur_8d = settings.presentation.motion_blur_8d;
        }
    }

    host.renderer_set_motion_blur(
        settings.presentation.motion_blur_8d, settings.options_file.antialias_58); // 008d603c
    const Resolution current = host.renderer_current_resolution(); // renderer virtual +80h
    if (current.width != settings.options_file.width_14
        || current.height != settings.options_file.height_18) { // 008d6058
        host.renderer_release_size_dependent_targets(); // 008d606c
    }
    host.renderer_change_presentation_mode(settings.options_file.width_14,
        settings.options_file.height_18, settings.options_file.fullscreen_1e,
        settings.options_file.antialias_58, settings.options_file.vsync_60,
        subsystem_changed); // 008d6092
    host.platform_window_set_mode(settings.options_file.fullscreen_1e,
        settings.options_file.width_14, settings.options_file.height_18); // 008d60af
    if (!env.xlive_suppressed) { // 008d60b1
        host.xlive_on_reset_device();
    }
    host.renderer_rebuild_post_effects(settings.presentation.shader_flag_8c,
        settings.presentation.motion_blur_8d, settings.options_file.antialias_58); // 008d60e6
    if (!env.xlive_suppressed) { // 008d60eb
        host.gui_manager_reload();
    }
    host.renderer_set_old_film_effect(settings.presentation.old_film_effect_90); // 008d610d
    host.publish_module_directory(env.module_directory); // 008d6112 then 008d6137
    return subsystem_changed;
}
}
