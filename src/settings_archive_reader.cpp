#include "bsp/settings_archive_reader.hpp"
#include "bsp/keyboard_restore.hpp"
#include <utility>

namespace bsp {
namespace {
using Type = GuiLuaFieldType;
void boolean(GuiLuaReader& reader, const char* name, bool& dest, bool fallback) {
    GuiLuaVariant value; value.tag = 3; value.value.integer = fallback ? 1 : 0;
    reader.read_or_default_00bd68d0(gui_lua_key_by_name(name),
        gui_lua_field(Type::Bool, &dest), value);
}
void integer(GuiLuaReader& reader, const char* name, int& dest, int fallback) {
    GuiLuaVariant value; value.tag = 1; value.value.integer = fallback;
    reader.read_or_default_00bd68d0(gui_lua_key_by_name(name),
        gui_lua_field(Type::Int, &dest), value);
}
void real(GuiLuaReader& reader, const char* name, float& dest, float fallback) {
    GuiLuaVariant value; value.tag = 2; value.value.number = fallback;
    reader.read_or_default_00bd68d0(gui_lua_key_by_name(name),
        gui_lua_field(Type::Float, &dest), value);
}
}
void read_settings_archive_008d6dc0(GameSettingsBlock& s, GuiLuaReader& reader,
    InputSettings& input, KeyboardRuntimeHost& keyboard, SettingsArchiveReadHost& host) {
    reader.enter_00bd8e20(gui_lua_key_by_name("Options"));
    boolean(reader, "imperial", s.gameplay.imperial_08, true);
    boolean(reader, "subtitle", s.gameplay.subtitle_09, false);
    integer(reader, "hints", s.gameplay.hints_0c, 2);
    boolean(reader, "cameraShake", s.gameplay.camera_shake_10, true);
    boolean(reader, "TargetIndicator", s.gameplay.target_indicator_4a, true);
    real(reader, "masterVolume", s.audio.master_20, 1.0f);
    real(reader, "musicVol", s.audio.music_28, 1.0f);
    real(reader, "sfxVol", s.audio.effects_2c, 1.0f);
    real(reader, "speechVol", s.audio.speech_30, 1.0f);
    boolean(reader, "rumbleOff", s.control.rumble_off_40, false);
    boolean(reader, "swapSticks", s.control.swap_sticks_41, false);
    boolean(reader, "invertCameraY", s.control.invert_camera_y_42, true);
    boolean(reader, "invertPlaneY", s.control.invert_plane_y_43, true);
    boolean(reader, "swapMapSticks", s.control.swap_map_sticks_44, false);
    boolean(reader, "XboxCompatibilityMode", s.control.xbox_compatibility_b0, false);
    host.publish_xbox_compatibility_008d44c0(s.control.xbox_compatibility_b0);
    reader.enter_00bd8e20(gui_lua_key_by_name("keyboardSetup"));
    read_keyboard_setup_006aba50(input, reader, keyboard);
    reader.leave_00bd7a20();
    boolean(reader, "HardwareReported", s.presentation.hardware_reported_95, false);
    boolean(reader, "waterDrops", s.gameplay.water_drops_7c, true);
    integer(reader, "oldFilmEffect", s.presentation.old_film_effect_90, 1);
    boolean(reader, "MotionBlur", s.presentation.motion_blur_8d, true);
    real(reader, "gamma", s.presentation.gamma_64, 0.0f);
    real(reader, "markerAlpha", s.gameplay.marker_alpha_80, 0.0f);
    boolean(reader, "ShowSafeArea", s.gameplay.show_safe_area_b1, false);
    boolean(reader, "CockpitMode", s.gameplay.cockpit_mode_b2, true);
    boolean(reader, "MotionBlur", s.presentation.motion_blur_8d, true); // native repeats
    if (reader.has_key_00bd5eb0(gui_lua_key_by_name("ClanText"))) {
        reader.read_or_default_00bd68d0(gui_lua_key_by_name("ClanText"),
            gui_lua_field(Type::String, &s.gameplay.clan_text_b4), gui_lua_key_by_name(""));
    }
    s.downloaded_content.clear();
    if (reader.has_key_00bd5eb0(gui_lua_key_by_name("DownloadedContent"))) {
        reader.enter_00bd8e20(gui_lua_key_by_name("DownloadedContent"));
        for (std::int32_t index = 0; reader.has_key_00bd5eb0(gui_lua_key_by_index(index)); ++index) {
            std::string value;
            reader.read_00bd6830(gui_lua_key_by_index(index), gui_lua_field(Type::String, &value));
            s.downloaded_content.push_back(std::move(value));
        }
        reader.leave_00bd7a20();
    }
    reader.leave_00bd7a20();
    if (host.renderer_shader_model_104_28() < 0x200u) {
        s.options_file.shadow_84 = false;
        s.options_file.shadow_85 = false;
        s.presentation.old_film_effect_90 = 0;
    }
}
}
