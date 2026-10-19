// cc15-prof: the signed-in profile read, run read-only. Evidence, the no-write
// proof and the pairs: docs/CC15_PROFILE_LOAD.md. Every routine is an existing
// typed body (request_profile_read_007ff100, complete_profile_read_007fefe0,
// read_profile_archive_007fdf00, read_mission_progress_00920000, and
// PcStorageBackend's read half); this file only composes them.
//
// NO-WRITE RULE. Of PcStorageBackend this file calls only storage_query_1c,
// immediate_read_00bd4380, has_storage_buffer_30, free_and_clear_storage_buffer_30,
// close_storage_archive_00b65e80 and storage_lua_38. The members that write
// (request_read_00bd3d70's completion stamps <name>\valid in update_00beb9b0;
// request_write_00bd3dc0, request_single_write_00bd3e10, request_delete_00bd3e70)
// are never called, and update_00beb9b0, the only caller of the writers, is never
// run. The archive executes in a Lua state opened with mask 1 (base only: no io
// or os library). 007FA710 and 007F9540 (the profile write) are not referenced.

#include "bsp/game_profile_load.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_native_lua_globals.hpp"
#include "bsp/game_native_settings_application.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_profile_owner.hpp"
#include "bsp/game_settings.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/input_settings.hpp"
#include "bsp/keyboard_restore.hpp"
#include "bsp/locale_tables.hpp"
#include "bsp/lua_runtime_globals.hpp"
#include "bsp/native_game_settings_owner.hpp"
#include "bsp/native_string.hpp"
#include "bsp/profile_archive.hpp"
#include "bsp/profile_commit.hpp"
#include "bsp/settings_archive_reader.hpp"
#include "bsp/storage_backend.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>

#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bsp::game {
namespace {

// 0109CECC kind 1 = "player"; 004374F0 asks for kind 2 = "game".
constexpr std::uint32_t kPlayerKind = 1;
constexpr std::uint32_t kCheckpointKind = 2;

// 007FDF00's host calls, on the lane-L2 owner's +64h.
class OwnerArchiveHost final : public ProfileArchiveHost {
public:
    OwnerArchiveHost(GameProfileOwner& owner, GameHostLog& log, PcStorageBackend& backend)
        : owner_(owner), log_(log), backend_(backend) {}

    int hints_owner_field_08_004c1e90() override {
        // [00E17664] is not built; its constructor 00426250 stores +08h = 0 at
        // 00426285. Either value leaves the same list here: 007FDB20 has just
        // cleared profile+08h, so the AllSeenHints rows are the whole list.
        log_.unimplemented("ProfileLoad::hints_owner_field_08", "004c1e90");
        return 0;
    }
    void destroy_mission_progress_007fd780() override {
        owner_.destroy_mission_progress_007fd780();
    }
    bool construct_mission_progress_00920e10() override {
        return owner_.construct_mission_progress_00920e10();
    }
    void read_mission_progress_00920000(
        ProfileResetState& profile, ProfileArchiveReader& reader) override {
        // Called even after a failed new(24h), as natively; the owner has none then.
        MissionProgress* progress = owner_.mission_progress();
        if (progress == nullptr) {
            log_.note("profile load: 00920000 skipped, +64h was not allocated");
            return;
        }
        bsp::read_mission_progress_00920000(*progress, reader, profile.version_e8);
        sync_mission_completion(*progress, profile.unlock_state);
        log_.implemented("ProfileLoad::read_mission_progress", "00920000");
    }
    void write_mission_progress_0090cb40(const ProfileResetState&, ProfileArchiveWriter&) override {
        // Reached only from 007F9540, the profile write, which this file never runs.
        throw std::logic_error("profile load is read-only: 0090CB40 must not run");
    }
    int sum_mission_progress_0090bf50() override {
        const MissionProgress* progress = owner_.mission_progress();
        return progress != nullptr ? bsp::sum_mission_progress_0090bf50(*progress) : 0;
    }
    void refresh_profile_manager_004374f0(ProfileResetState& profile) override {
        // 004374F0 builds the game Lua table BSP_Chk_Save, then reads the
        // checkpoint save (kind 2, "game") only when +1Ch answers true. The
        // queued read it would issue (00BD3D70 + 006ADB50) stamps <name>\valid,
        // so it is not run; the query alone is read-only.
        const bool present = !profile.save_name_34.empty()
            && backend_.storage_query_1c(profile.save_name_34, kCheckpointKind);
        log_.unimplemented("ProfileLoad::refresh_profile_manager", "004374f0");
        log_.notef("profile load: 004374F0 checkpoint save %s\\game %s", profile.save_name_34.c_str(),
            present ? "present (its read is withheld)" : "absent (the native reads nothing either)");
    }

private:
    GameProfileOwner& owner_;
    GameHostLog& log_;
    PcStorageBackend& backend_;
};

// 0109CECC kind 0 = "quick", the settings archive 008D7A50 asks for.
constexpr std::uint32_t kQuickKind = 0;

// cc15-prof2: the live settings 00F88980. The raw BCh owner is authoritative;
// the typed bodies (007FDB20's half, 008D6DC0, 008D5B50) run on a view copied
// from it, and write_back stores every scalar the view changed at its native
// offset, as those bodies store into 00F88980 directly.
class LiveSettings {
public:
    LiveSettings(NativeGameSettingsStorage& raw, const std::vector<LanguageEntry>& languages,
        GameHostLog& log)
        : raw_(raw), log_(log) {
        copy_native_game_settings_read_view(raw_, languages, view);
        copied_ = view;
    }

    GameSettingsBlock view{};

    void write_back(const char* stage) {
        std::size_t changed = 0;
        const auto flag = [&](std::uint16_t offset, const char* name, bool value) {
            const auto before = std::to_integer<unsigned>(raw_.data()[offset]);
            const unsigned after = value ? 1u : 0u;
            if (before == after) return;
            raw_.data()[offset] = static_cast<std::byte>(after);
            log_.notef("quick settings: %s +%02Xh %s %u -> %u", stage, offset, name, before, after);
            ++changed;
        };
        const auto integer = [&](std::uint16_t offset, const char* name, int value) {
            int before{};
            std::memcpy(&before, raw_.data() + offset, sizeof before);
            if (before == value) return;
            std::memcpy(raw_.data() + offset, &value, sizeof value);
            log_.notef("quick settings: %s +%02Xh %s %d -> %d", stage, offset, name, before, value);
            ++changed;
        };
        const auto real = [&](std::uint16_t offset, const char* name, float value) {
            float before{};
            std::memcpy(&before, raw_.data() + offset, sizeof before);
            if (std::memcmp(&before, &value, sizeof value) == 0) return;
            std::memcpy(raw_.data() + offset, &value, sizeof value);
            log_.notef("quick settings: %s +%02Xh %s %.6g -> %.6g", stage, offset, name,
                static_cast<double>(before), static_cast<double>(value));
            ++changed;
        };
        const auto& g = view.gameplay;
        const auto& a = view.audio;
        const auto& c = view.control;
        const auto& p = view.presentation;
        const auto& o = view.options_file;
        // Every field 007FDB20's half (008D4820, 008D41C0), 008D6DC0 and its
        // shader-model tail (008D768E..769C) store.
        flag(0x08, "imperial", g.imperial_08);
        flag(0x09, "subtitle", g.subtitle_09);
        integer(0x0C, "hints", g.hints_0c);
        flag(0x10, "cameraShake", g.camera_shake_10);
        real(0x20, "masterVolume", a.master_20);
        real(0x28, "musicVol", a.music_28);
        real(0x2C, "sfxVol", a.effects_2c);
        real(0x30, "speechVol", a.speech_30);
        flag(0x40, "rumbleOff", c.rumble_off_40);
        flag(0x41, "swapSticks", c.swap_sticks_41);
        flag(0x42, "invertCameraY", c.invert_camera_y_42);
        flag(0x43, "invertPlaneY", c.invert_plane_y_43);
        flag(0x44, "swapMapSticks", c.swap_map_sticks_44);
        flag(0x4A, "TargetIndicator", g.target_indicator_4a);
        real(0x64, "gamma", p.gamma_64);
        flag(0x7C, "waterDrops", g.water_drops_7c);
        real(0x80, "markerAlpha", g.marker_alpha_80);
        flag(0x84, "Shadow", o.shadow_84);
        flag(0x85, "Shadow+85h", o.shadow_85);
        flag(0x8D, "MotionBlur", p.motion_blur_8d);
        integer(0x90, "oldFilmEffect", p.old_film_effect_90);
        flag(0x95, "HardwareReported", p.hardware_reported_95);
        flag(0xB0, "XboxCompatibilityMode", c.xbox_compatibility_b0);
        flag(0xB1, "ShowSafeArea", g.show_safe_area_b1);
        flag(0xB2, "CockpitMode", g.cockpit_mode_b2);
        // +B4h (NativeString) and +98h (vector) own native allocations; a
        // change to either is reported, not stored.
        if (g.clan_text_b4 != copied_.gameplay.clan_text_b4) {
            log_.notef("quick settings: %s +B4h ClanText changed; not stored", stage);
        }
        if (view.disabled_downloaded_content != copied_.disabled_downloaded_content) {
            log_.notef("quick settings: %s +98h DownloadedContent changed; not stored", stage);
        }
        log_.notef("quick settings: %s stored %zu fields into 00F88980", stage, changed);
    }

private:
    NativeGameSettingsStorage& raw_;
    GameHostLog& log_;
    GameSettingsBlock copied_{};
};

const std::vector<LanguageEntry>& no_languages() {
    static const std::vector<LanguageEntry> empty;
    return empty;
}

// 00F88A3D/00F88A40/00F88A44 and the seeded bits 00F88A48: process globals.
SettingsCommitLatches& settings_commit_latches_00f88a48() {
    static SettingsCommitLatches latches;
    return latches;
}

// 008D7A50 / 008D79A0 / 008D6DC0's host calls: the read half of PcStorageBackend
// only, as for the player read. The keyboardSetup table goes to the input
// settings 00698A10 builds; the host builds none, so the table is entered and
// left with nothing to read (006ABA50 reads only names the settings hold).
class QuickSettingsReadHost final : public ProfileSettingsRestoreHost,
    public SettingsArchiveReadHost, public KeyboardRuntimeHost {
public:
    QuickSettingsReadHost(const ProfileResetState& profile, LiveSettings& live,
        GameHostLog& log, PcStorageBackend& backend, const GameProfileSettingsSinks& sinks,
        const bool& crt_sse2)
        : profile_(profile), live_(live), log_(log), backend_(backend), sinks_(sinks),
          crt_sse2_(crt_sse2) {}

    ProfileSettingsRestoreState state{};   // 00F88958

    void reset_storage_operation_00bd3450() override { backend_.reset_storage_operation_00bd3450(); }
    // [0109CECC]+21h is 1 on PC, as for 0067CC60.
    bool storage_available_21() override { return true; }
    // 00E188A8: the game that owns the +650h profile exists.
    bool game_present() override { return true; }
    bool profile_has_save_name_007f8ca0() override { return !profile_.save_name_34.empty(); }
    std::string copy_profile_save_name_00425f40() override { return profile_.save_name_34; }
    bool storage_query_1c(std::string_view name, bool kind) override {
        // GENERIC_READ opens of <name>\valid and <name>\quick (00BEB1E0).
        const bool present = backend_.storage_query_1c(name, kind ? kPlayerKind : kQuickKind);
        log_.implemented("QuickSettings::storage_query_1c", "00beb1e0");
        log_.notef("quick settings: %.*s\\quick %s", static_cast<int>(name.size()), name.data(),
            present ? "present" : "absent");
        return present;
    }
    void request_read_00bd3d70(std::string_view name, bool kind) override {
        // As for the player read: the queued read's completion would re-stamp
        // <name>\valid, so the name is kept and the immediate read runs instead.
        read_name_.assign(name);
        read_kind_ = kind ? kPlayerKind : kQuickKind;
        log_.implemented("QuickSettings::request_read (immediate, no valid stamp)", "00bd3d70");
    }
    void run_storage_operation_006adb50(ProfileCompletion continuation) override {
        backend_.immediate_read_00bd4380(read_name_, read_kind_);
        log_.implemented("QuickSettings::read_archive", "00bd3ec0");
        if (continuation) continuation();   // 008D79A0
    }
    void immediate_read_00bd4380(std::string_view name, bool kind) override {
        backend_.immediate_read_00bd4380(name, kind ? kPlayerKind : kQuickKind);
        log_.implemented("QuickSettings::immediate_read", "00bd4380");
    }
    int storage_state_08() override { return backend_.operation().state_08; }
    void begin_settings_reader_004425c0() override {
        lua_State* lua_state = backend_.storage_lua_38();
        if (lua_state == nullptr) throw std::logic_error("settings reader requires the open storage Lua");
        readers_.push_back(std::make_unique<ReaderFrame>(*lua_state, crt_sse2_));
    }
    void deserialize_settings_008d6dc0(GameSettingsBlock& settings) override {
        read_settings_archive_008d6dc0(settings, readers_.back()->reader, input_, *this, *this);
        log_.implemented("QuickSettings::read_settings_archive", "008d6dc0");
        log_.unimplemented("QuickSettings::keyboard_setup (no input configuration 00698a10)",
            "006aba50");
        log_.notef("quick settings: read masterVolume=%.6g musicVol=%.6g sfxVol=%.6g speechVol=%.6g "
            "rumbleOff=%d swapSticks=%d invertCameraY=%d invertPlaneY=%d swapMapSticks=%d gamma=%.6g "
            "markerAlpha=%.6g CockpitMode=%d HardwareReported=%d XboxCompatibilityMode=%d dlc=%zu",
            static_cast<double>(settings.audio.master_20), static_cast<double>(settings.audio.music_28),
            static_cast<double>(settings.audio.effects_2c), static_cast<double>(settings.audio.speech_30),
            settings.control.rumble_off_40 ? 1 : 0, settings.control.swap_sticks_41 ? 1 : 0,
            settings.control.invert_camera_y_42 ? 1 : 0, settings.control.invert_plane_y_43 ? 1 : 0,
            settings.control.swap_map_sticks_44 ? 1 : 0,
            static_cast<double>(settings.presentation.gamma_64),
            static_cast<double>(settings.gameplay.marker_alpha_80),
            settings.gameplay.cockpit_mode_b2 ? 1 : 0,
            settings.presentation.hardware_reported_95 ? 1 : 0,
            settings.control.xbox_compatibility_b0 ? 1 : 0,
            settings.disabled_downloaded_content.size());
    }
    bool has_storage_buffer_30() override { return backend_.has_storage_buffer_30(); }
    void free_and_clear_storage_buffer_30() override { backend_.free_and_clear_storage_buffer_30(); }
    void close_storage_archive_00b65e80() override { backend_.close_storage_archive_00b65e80(); }
    void destroy_settings_reader_00441a20() override { readers_.pop_back(); }

    // 008D6DC0's two host calls.
    void publish_xbox_compatibility_008d44c0(bool enabled) override {
        // 008D44C0 stores +B0h and mirrors it into 0108FF20. Its console
        // command ((00E188A8)+1A08h)+4h, 006B8AD0) has no host sink.
        static_cast<void>(publish_lua_xbox_compatibility_008d44c0(live_.view,
            game_native_lua_globals_process().globals_0108ff20(), enabled, false));
        log_.implemented("QuickSettings::publish_xbox_compatibility", "008d44c0");
        log_.unimplemented("QuickSettings::x360comp_console_command", "006b8ad0");
    }
    std::uint32_t renderer_shader_model_104_28() override { return sinks_.pixel_shader_version_28; }

    // 006AA640's host calls. With no input configuration, 006ABA50 binds nothing.
    bool action_registered_00a92260(std::uint32_t) override {
        log_.unimplemented("QuickSettings::action_registered (no input configuration)", "00a92260");
        return false;
    }
    bool use_alternate_axis_slots_006aa090(InputSettings&, const std::string&,
        const std::string&) override {
        throw std::logic_error("quick settings: no input device is configured (006AA090)");
    }
    void bind_action_00a93750(std::int32_t, std::int32_t, const KeyboardInputBinding&, float) override {
        throw std::logic_error("quick settings: no input device is configured (00A93750)");
    }

private:
    struct ReaderFrame {
        GuiLua51Host lua;
        GuiLuaReader reader;
        ReaderFrame(lua_State& state, const bool& crt_sse2)
            : lua(state), reader(lua, lua.globals(), crt_sse2) {}
    };

    const ProfileResetState& profile_;
    LiveSettings& live_;
    GameHostLog& log_;
    PcStorageBackend& backend_;
    const GameProfileSettingsSinks& sinks_;
    const bool& crt_sse2_;
    InputSettings input_{};
    std::string read_name_;
    std::uint32_t read_kind_{kQuickKind};
    std::vector<std::unique_ptr<ReaderFrame>> readers_;
};

// 008D5B50's sinks. Each one reaches the host state that exists; the rest log
// as unimplemented, with the native call they stand for.
class QuickSettingsApplyHost final : public SettingsApplyHost {
public:
    QuickSettingsApplyHost(GameHostLog& log, const GameProfileSettingsSinks& sinks,
        const GameSettingsBlock& settings)
        : log_(log), sinks_(sinks), settings_(settings) {}

    void sound_set_enabled(bool) override { missing("sound_set_enabled", "00f8bbd8+70h"); }
    void sound_set_master_volume(float) override { missing("sound_set_master_volume", "00a7a440"); }
    void sound_store_music_volume(float volume) override { store_alternate(0x218, volume, "music"); }
    void sound_store_speech_volume(float volume) override { store_alternate(0x21C, volume, "speech"); }
    void music_player_set_volume(float) override { missing("music_player_set_volume", "00a864f0"); }
    void sound_set_group_volume(std::uint32_t, float) override { missing("sound_set_group_volume", "00a7abf0"); }
    void sound_set_named_bus_volume(const char*, float) override {
        missing("sound_set_named_bus_volume", "00a7acf0");
    }
    void ui_sound_set_master_volume(float) override { missing("ui_sound_set_master_volume", "004c1710+10h"); }
    void renderer_set_texture_detail_bias(int) override {
        missing("renderer_set_texture_detail_bias", "00b1ffb0");
    }
    void renderer_set_gamma(float) override { missing("renderer_set_gamma", "00f8d394+f0h"); }
    void input_set_stick_modifiers(bool, bool, bool, bool) override {
        missing("input_set_stick_modifiers", "00699b80");
    }
    void game_refresh_after_settings() override { missing("game_refresh_after_settings", "004dcdf0"); }
    void foliage_system_set_detail_scale(float) override {
        missing("foliage_system_set_detail_scale", "00e188a8+19fch");
    }
    void ocean_set_reflection_enabled(bool) override { missing("ocean_set_reflection_enabled", "00e188a8+19f0h"); }
    void ui_set_subtitles_enabled(bool) override { missing("ui_set_subtitles_enabled", "004c1710+14h"); }
    void font_registry_set_language_path(const char*, const char*) override {
        missing("font_registry_set_language_path", "007371d0");
    }
    void localization_load_table(const char*) override { missing("localization_load_table", "00aa09d0"); }
    void set_rumble_enabled(bool enabled) override {
        // 00A94C50: 00E12F2C, then each device's two motors.
        if (sinks_.set_rumble_enabled_00a94c50) {
            sinks_.set_rumble_enabled_00a94c50(enabled);
            log_.implemented("QuickSettingsApply::set_rumble_enabled", "00a94c50");
            log_.notef("quick settings: 00A94C50 rumble enabled=%d", enabled ? 1 : 0);
        } else {
            missing("set_rumble_enabled", "00a94c50");
        }
    }
    void downloadable_content_rebind() override { missing("downloadable_content_rebind", "00f8a304"); }
    void localization_reload_globals() override { missing("localization_reload_globals", "00aa0d30"); }
    void foliage_group_set_visibility(float) override { missing("foliage_group_set_visibility", "00b72270"); }
    void clouds_set_enabled(bool) override { missing("clouds_set_enabled", "00bbcfe0"); }
    void ocean_set_unknown_70(int) override { missing("ocean_set_unknown_70", "00e188a8+19f0h"); }
    void shadow_system_set_enabled(bool) override { missing("shadow_system_set_enabled", "00f8bbf0+8h"); }
    void renderer_set_motion_blur(bool, int) override { missing("renderer_set_motion_blur", "00b0d580"); }
    Resolution renderer_current_resolution() override {
        // Renderer virtual +80h. Answered with the settings' own size, so the
        // 00B0FC00 release is not asked for.
        missing("renderer_current_resolution", "00f8d394+80h");
        return Resolution{settings_.options_file.width_14, settings_.options_file.height_18};
    }
    void renderer_release_size_dependent_targets() override {
        missing("renderer_release_size_dependent_targets", "00b0fc00");
    }
    void renderer_change_presentation_mode(int, int, bool, int, bool, bool) override {
        missing("renderer_change_presentation_mode", "00b29e60");
    }
    void platform_window_set_mode(bool, int, int) override {
        missing("platform_window_set_mode", "0109cf04+0ch");
    }
    void xlive_on_reset_device() override { missing("xlive_on_reset_device", "XLiveOnResetDevice"); }
    void renderer_rebuild_post_effects(int, bool, int) override {
        missing("renderer_rebuild_post_effects", "00b107f0");
    }
    void gui_manager_reload() override { missing("gui_manager_reload", "004c12b0"); }
    void renderer_set_old_film_effect(int) override { missing("renderer_set_old_film_effect", "00b0d080"); }
    void publish_lua_region(const std::string& region) override {
        // 008D6132: 0108FF24's NativeString, read by every later Lua owner open.
        publish_lua_region_008d6132(game_native_lua_globals_process().globals_0108ff20(), region,
            game_native_string_process().raw_context());
        log_.implemented("QuickSettingsApply::publish_lua_region", "008d6132");
        log_.notef("quick settings: 008D6132 region=%s", region.c_str());
    }

private:
    void missing(const char* method, const char* address) {
        std::string name = "QuickSettingsApply::";
        name += method;
        log_.unimplemented(name.c_str(), address);
    }
    void store_alternate(std::size_t offset, float volume, const char* which) {
        // 008D5483 / 008D5490 reload 00F8BBCC for each direct store.
        void* alternate = sinks_.sound_alternate_00f8bbcc ? *sinks_.sound_alternate_00f8bbcc : nullptr;
        if (alternate == nullptr) {
            missing(offset == 0x218 ? "sound_store_music_volume" : "sound_store_speech_volume",
                "00f8bbcc");
            return;
        }
        std::memcpy(static_cast<std::byte*>(alternate) + offset, &volume, sizeof volume);
        log_.implemented(offset == 0x218 ? "QuickSettingsApply::sound_store_music_volume"
            : "QuickSettingsApply::sound_store_speech_volume", offset == 0x218 ? "008d5483" : "008d5490");
        log_.notef("quick settings: 00F8BBCC+%zXh %s volume=%.6g", offset, which,
            static_cast<double>(volume));
    }

    GameHostLog& log_;
    const GameProfileSettingsSinks& sinks_;
    const GameSettingsBlock& settings_;
};

// The quick half of one load: the live view, 008D7A50's host and 008D5B50.
struct QuickSettingsLoad {
    LiveSettings live;
    QuickSettingsReadHost read;
    QuickSettingsLoad(NativeGameSettingsStorage& raw, const std::vector<LanguageEntry>& languages,
        const ProfileResetState& profile, GameHostLog& log, PcStorageBackend& backend,
        const GameProfileSettingsSinks& sinks, const bool& crt_sse2)
        : live(raw, languages, log), read(profile, live, log, backend, sinks, crt_sse2) {}
};

// 007FF100 / 007FEFE0's host calls.
class ReadOnlyProfileIoHost final : public ProfileIoHost {
public:
    ReadOnlyProfileIoHost(GameProfileOwner& owner, GameHostLog& log,
        PcStorageBackend& backend, OwnerArchiveHost& archive, const bool& crt_sse2)
        : owner_(owner), log_(log), backend_(backend), archive_(archive), crt_sse2_(crt_sse2) {}

    GameProfileLoadReport report{};
    ProfileIoState io{};
    GameSettingsBlock discarded_settings{};
    // cc15-prof2, kQuickSettingsApplyCc15Bound only.
    QuickSettingsLoad* quick{nullptr};
    const GameProfileSettingsSinks* sinks{nullptr};
    GameSettingsBlock& settings() { return quick != nullptr ? quick->live.view : discarded_settings; }

    // ProfileResetHost: the owner's, so 007FDB20 behaves as lane L2 has it.
    void destroy_mission_progress_007fd780() override { owner_.destroy_mission_progress_007fd780(); }
    bool construct_mission_progress_00920e10() override { return owner_.construct_mission_progress_00920e10(); }
    bool xenon_state_is_two() override { return owner_.xenon_state_is_two(); }
    bool xenon_user_selected() override { return owner_.xenon_user_selected(); }
    void restore_selected_user_controls_008d45d0() override { owner_.restore_selected_user_controls_008d45d0(); }

    bool storage_query_1c(std::string_view name, bool kind) override {
        // GENERIC_READ opens of <name>\valid and <name>\player (00BEB1E0).
        report.queried = backend_.storage_query_1c(name, kind ? kPlayerKind : 0u);
        log_.implemented("ProfileLoad::storage_query_1c", "00beb1e0");
        return report.queried;
    }
    void request_read_00bd3d70(std::string_view name, bool kind) override {
        // Natively this queues operation 2; 00BEB9B0 then reads and, on success,
        // re-stamps <name>\valid (CREATE_ALWAYS + SetFileTime). The host keeps
        // the name and kind and reads in run_storage_operation_006adb50 instead.
        read_name_.assign(name);
        read_kind_ = kind ? kPlayerKind : 0u;
        log_.implemented("ProfileLoad::request_read (immediate, no valid stamp)", "00bd3d70");
    }
    void request_write_00bd3dc0(std::string_view) override {
        throw std::logic_error("profile load is read-only: 00BD3DC0 must not run");
    }
    void run_storage_operation_006adb50(ProfileIoTask task) override {
        if (task != ProfileIoTask::ReadCompleted) {
            throw std::logic_error("profile load is read-only: only 007FEFE0 may be driven");
        }
        // 00BD4380: CreateFileA(GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING),
        // ReadFile, block decode, then 00B6A020(1) + DoBuffer. No prompt, no
        // marker write, no operation-code change.
        backend_.immediate_read_00bd4380(read_name_, read_kind_);
        report.read = backend_.operation().state_08 == 0;
        log_.implemented("ProfileLoad::read_archive", "00bd3ec0");
        if (!report.read) {
            // 00BEB9B0 would raise "globals.loadfailed" here; nothing is shown.
            log_.notef("profile load: reading %s\\player failed; 007FEFE0 resets", read_name_.c_str());
        }
        complete_profile_read_007fefe0(owner_.profile(), settings(), io, *this);
        log_.implemented("ProfileLoad::complete_read", "007fefe0");
    }
    int storage_state_08() override { return backend_.operation().state_08; }

    void begin_profile_reader_004425c0() override {
        lua_State* state = backend_.storage_lua_38();
        if (state == nullptr) throw std::logic_error("profile reader requires the open storage Lua");
        readers_.push_back(std::make_unique<ReaderFrame>(*state, crt_sse2_));
    }
    void deserialize_profile_007fdf00(ProfileResetState& profile) override {
        read_profile_archive_007fdf00(profile, readers_.back()->reader, archive_);
        log_.implemented("ProfileLoad::read_profile_archive", "007fdf00");
    }
    bool has_storage_buffer_30() override { return backend_.has_storage_buffer_30(); }
    void free_and_clear_storage_buffer_30() override { backend_.free_and_clear_storage_buffer_30(); }
    void close_storage_archive_00b65e80() override { backend_.close_storage_archive_00b65e80(); }
    bool has_manager_00f8a2fc() override {
        log_.unimplemented("ProfileLoad::manager_00f8a2fc", "00f8a2fc");
        return false;
    }
    void notify_manager_virtual_a0() override {}
    void restore_settings_008d7a50(ProfileCompletion completion) override {
        if constexpr (kQuickSettingsApplyCc15Bound) {
            if (quick != nullptr) {
                // 008D7A50 reads <name>\quick into 00F88980 through 008D6DC0.
                // The completion is the sign-in's 0067CA40 slot, here the empty
                // callback the call site passes (its title skip is the host's).
                restore_profile_settings_008d7a50(quick->live.view, quick->read.state, quick->read,
                    std::move(completion));
                log_.implemented("ProfileLoad::restore_settings", "008d7a50");
                quick->live.write_back("008D7A50");
                return;
            }
        }
        // Withheld, as lane L2 withholds 007FDB20's settings half: 008D7A50 would
        // read <name>\quick into the live settings. The completion slot is the
        // sign-in's 0067CA40, whose title skip the host's own path performs.
        log_.unimplemented("ProfileLoad::restore_settings (withheld)", "008d7a50");
        static_cast<void>(completion);
    }
    void apply_settings_008d5b50() override {
        if constexpr (kQuickSettingsApplyCc15Bound) {
            if (quick != nullptr) {
                // 008D5B50 at its title-time state: the game exists; the scene,
                // ocean, foliage, clouds (game+19E8h..19FCh), the download
                // manager 00F8A304 and the in-mission interface 00E198C4 do not.
                SettingsApplyEnvironment env{};
                env.game_present = true;
                env.user_region = user_geography_region_00439100();
                QuickSettingsApplyHost apply(log_, *sinks, quick->live.view);
                const bool changed = apply_all_settings_008d5b50(quick->live.view,
                    sinks->languages != nullptr ? *sinks->languages : no_languages(), env,
                    settings_commit_latches_00f88a48(), apply);
                log_.implemented("ProfileLoad::apply_settings", "008d5b50");
                log_.notef("quick settings: 008D5B50 subsystem_changed=%d", changed ? 1 : 0);
                return;
            }
        }
        log_.unimplemented("ProfileLoad::apply_settings (withheld)", "008d5b50");
    }
    void commit_profile_007fae70(ProfileResetState&) override {
        log_.unimplemented("ProfileLoad::commit_content_state", "007fae70");
    }
    void destroy_profile_reader_00441a20() override { readers_.pop_back(); }

private:
    struct ReaderFrame {
        GuiLua51Host lua;
        GuiLuaReader reader;
        ReaderFrame(lua_State& state, const bool& crt_sse2)
            : lua(state), reader(lua, lua.globals(), crt_sse2) {}
    };

    GameProfileOwner& owner_;
    GameHostLog& log_;
    PcStorageBackend& backend_;
    OwnerArchiveHost& archive_;
    const bool& crt_sse2_;
    std::string read_name_;
    std::uint32_t read_kind_{kPlayerKind};
    std::vector<std::unique_ptr<ReaderFrame>> readers_;
};

} // namespace

std::string read_only_save_root() {
    if (const char* root = std::getenv("BSP_PROFILE_LOAD_ROOT"); root != nullptr && *root != '\0') {
        return root;
    }
    char personal[MAX_PATH]{};
    // fCreate FALSE: unlike 00BEB2C0, nothing is created.
    if (!SHGetSpecialFolderPathA(nullptr, personal, CSIDL_PERSONAL, FALSE)) return {};
    std::string path = personal;
    path += "\\Battlestations-Pacific\\save";
    return path;
}

GameProfileLoadReport load_signed_in_profile_read_only(GameProfileOwner& owner,
    GameHostLog& log, const std::string& save_root, std::string_view save_name,
    LuaStateOwnerEnvironment lua, const GameProfileSettingsSinks* sinks)
{
    // 0109EEA4, bound as GameMenuHost binds it.
    static const bool crt_sse2 = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;

    PcStorageLuaOwner storage_lua(std::move(lua));
    ArchiveCompressionState unused_compression; // the write half's buffer, never touched
    PcStorageBackend backend(save_root, crt_string_storage(), storage_lua, unused_compression);
    backend.reset_storage_operation_00bd3450(); // 0067CC60's 00BD3450; memory only
    OwnerArchiveHost archive(owner, log, backend);
    ReadOnlyProfileIoHost host(owner, log, backend, archive, crt_sse2);
    std::unique_ptr<QuickSettingsLoad> quick;
    if constexpr (kQuickSettingsApplyCc15Bound) {
        if (sinks != nullptr && sinks->settings_00f88980 != nullptr) {
            quick = std::make_unique<QuickSettingsLoad>(*sinks->settings_00f88980,
                sinks->languages != nullptr ? *sinks->languages : no_languages(), owner.profile(), log,
                backend, *sinks, crt_sse2);
            host.quick = quick.get();
            host.sinks = sinks;
        } else {
            log.note("quick settings: no live settings bound; 008D7A50/008D5B50 stay withheld");
        }
    }

    // 0067CC60: 007FDB20 first, then (PC: [0109CECC]+21h is 1) 007FF100.
    owner.reset_to_defaults_007fdb20();
    log.implemented("ProfileLoad::reset_to_defaults", "007fdb20");
    if (quick) {
        // 007FDB20's settings half on 00F88980: 008D4820, the Xenon query for
        // 008D45D0, then 008D41C0 (reset_profile_007fdb20's tail, same order).
        GameSettingsBlock& settings = quick->live.view;
        reset_control_defaults_008d4820(settings, false, false);
        if (owner.xenon_state_is_two() && owner.xenon_user_selected()) {
            owner.restore_selected_user_controls_008d45d0();
        }
        reset_game_defaults_008d41c0(settings);
        log.implemented("ProfileLoad::reset_settings_half", "007fdb20");
        quick->live.write_back("007FDB20");
    }
    log.notef("profile load: root %s save %.*s", save_root.c_str(),
        static_cast<int>(save_name.size()), save_name.data());
    request_profile_read_007ff100(owner.profile(), host.settings(), host.io, host,
        save_name, [] {});
    log.implemented("ProfileLoad::request_read", "007ff100");
    if (quick) {
        // 007FF100's no-save arm and 007FEFE0's state-1 arm reset through
        // 007FDB20 on these settings and skip 008D7A50.
        quick->live.write_back("load end");
    }

    GameProfileLoadReport report = host.report;
    const ProfileResetState& profile = owner.profile();
    report.seen_hints = profile.string_list_08.size();
    report.score_total = profile.score_total_30;
    if (const MissionProgress* progress = owner.mission_progress()) {
        report.mission_records = progress->mission_scores_00.size();
        for (const auto& [key, record] : progress->mission_scores_00) {
            if (record.mission_completed_00 != 0) {
                ++report.completed_records;
                log.notef("profile load: completed record \"%s\"", key.c_str());
            }
        }
    }
    log.notef("profile load: queried=%d read=%d version=%u selected_mission=%s seen_hints=%zu "
        "unlocks=%zu achievements=%zu mission_records=%zu completed=%zu score_total=%d",
        report.queried ? 1 : 0, report.read ? 1 : 0, profile.version_e8,
        profile.selected_mission_id_68.c_str(), report.seen_hints,
        profile.unlock_state.unlocks.size(), profile.unlock_state.named_counters.size(),
        report.mission_records, report.completed_records, report.score_total);
    return report;
}

} // namespace bsp::game
