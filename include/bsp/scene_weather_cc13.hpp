#pragma once

// Scene-header readers for weather, fog, render clips, cloud lights, sun/ambient
// lights, sky names and music. Caller BSP_SceneRecord_ApplyHeaderProperties
// 004F1D70 with sub-bags of the header's properties: g_Weather -> 004ECA30
// (004F2237), Sound -> 004ED690 (004F22C0, only when its stack flag byte is set),
// g_Environment -> 004EE450 (004F230B), which passes its Lights to 004EA870.
// Packet cc13_scene_weather, docs/CC13_SCENE_WEATHER.md.
//
// These are semantic C++ interfaces for MSVC Win32 over ScenePropertyBagModel,
// not binary replacements. Member suffixes are the native scene-record offsets
// (record = [game+5FCh]); "block" is the record+990h environment block that
// include/bsp/water_render.hpp names (block offset = record offset - 990h).
// Names are hypotheses; the scene keys are the image's own strings.
//
// Native lookups that are not null-checked (the image dereferences the 008F2260
// result) are host errors here: the reader returns false and keeps the fields
// already written, as the native would have before faulting.
//
// RNG: none of these readers draws. The rain these fields enable does: the rain
// emitter 00801400 draws from 00BD2F10 (shared stream 0) every frame while
// block+60h (IsRain) is set (docs/CC13_RNG_STREAM_CENSUS.md). Nothing here wires
// rain into the host.

#include <array>
#include <cstdint>
#include <string>

#include "bsp/scene_file.hpp"
#include "bsp/scene_property_bag.hpp"

namespace bsp::scene_weather_cc13 {

struct Rgba {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
    float a{0.0f};
};

// ---------------------------------------------------------------------------
// 004ECA30 targets. Defaults: the block constructor 004CB420 (record+990h) and
// 004B6980 (record+1014h), both run by BSP_SceneRecord_Construct 004DA2A0.
struct SceneWeatherFields {
    // "Rain" (block +60h..+7Fh); consumer 00865900 (rain descriptor + fog owner).
    std::uint8_t is_rain_9f0{0};                       // IsRain, byte
    std::int32_t rain_drop_num_9f4{1400};              // RainDropNum, raw dword
    std::array<float, 3> rain_speed_9f8{6.0f, -12.0f, 3.0f};  // RainSpeed.X/Y/Z
    // RainColor bytes in memory order B, G, R, A (A04h..A07h): a D3DCOLOR.
    std::array<std::uint8_t, 4> rain_color_bgra_a04{0xFF, 0xD0, 0xD0, 0x80};
    float rain_drop_length_min_a08{0.15f};             // RainDropLengthMin, optional
    float rain_drop_length_max_a0c{1.05f};             // RainDropLengthMax, optional
    // "Fog" (block +80h..+FFh); consumers 0078C9B0 and 00865900 (fog owner).
    Rgba fog_color_a10{0.644f, 0.64f, 0.667f, 0.0f};   // FogColor, rgb*L, a = L
    // DirFogColor Right, Left, Front, Back; rgb*L, a = 1.0.
    std::array<Rgba, 4> dir_fog_color_a20{{{0.4f, 0.4f, 0.4f, 0.0f},
                                           {0.4f, 0.4f, 0.4f, 0.0f},
                                           {0.4f, 0.4f, 0.4f, 0.0f},
                                           {0.4f, 0.4f, 0.4f, 0.0f}}};
    float lin_fog_dist_density_a60{0.7f};
    float lin_fog_dist_start_a64{100.0f};
    float lin_fog_dist_end_a68{7500.0f};
    float lin_fog_height_density_a6c{0.7f};
    float lin_fog_height_end_a70{200.0f};
    float lin_fog_uw_height_end_a74{-120.0f};
    // Render clips, each optional. Consumers in BSP_Game_ConstructWorld 004DE610
    // and 0078C9B0 (see the doc's consumer table).
    float object_render_clip_a78{30000.0f};
    float reflect_render_clip_a7c{30000.0f};
    float refract_render_clip_a80{30000.0f};
    float shadow_render_clip_a84{30000.0f};
    float foliage_render_clip_a88{30000.0f};           // xFoliageRenderClip
    float foliage_low_res_dist_a8c{30000.0f};          // xFoliageLowResDist
    std::uint8_t clouds_affected_by_wind_bec{1};       // block +25Ch
    // "CloudLights" (outside the block); alpha is never read from the scene.
    Rgba cloud_ambient_1014{0.2f, 0.2f, 0.2f, 0.0f};
    Rgba cloud_sun_1024{0.8f, 0.8f, 0.8f, 0.0f};
    Rgba cloud_bottom_1034{0.1f, 0.1f, 0.3f, 0.0f};
};

// 004E5C00: __fastcall(float* rgb), RET; 00CE89D8 = 2.2f, 00D7A208 = -0.0f.
// Each of the three channels: 0 stays 0 (UCOMISS/LAHF/TEST AH,44h: NaN takes the
// compute path); a negative value becomes -0.0f - c; then FYL2X/FRNDINT/F2XM1/
// FSCALE gives 2^(2.2*log2(c)) = c^2.2, stored as float. Run only when the
// settings byte 00F88A0C (GameSettings +8Ch, shader_flag_8c) is set. The host
// computes in double; the image keeps 80-bit intermediates, so the last bit can
// differ.
void linearize_rgb_004e5c00(float* rgb) noexcept;

// 004ECA30: __fastcall(ECX = g_Weather bag, EDX = scene record), RET; caller
// 004F2237 only when the header's g_Weather entry exists and is a sub-bag.
// `bag_index` is the g_Weather bag in `model`. `linearize` is the byte 00F88A0C.
// Order and gates are the image's; see the doc for the per-key table.
bool read_weather_fog_and_cloud_lights_004eca30(const ScenePropertyBagModel& model, int bag_index,
                                                bool linearize, SceneWeatherFields& out,
                                                std::string& error);

// ---------------------------------------------------------------------------
// 004EA870 targets. F48h..100Bh is the lighting configuration that
// include/bsp/lighting_configuration_apply.hpp consumes (LightingConfigurationFields).
struct SceneLightsFields {
    Rgba water_tracer_color_c44{};                     // WaterTracerColor, optional
    float particle_multiplier_1054{0.0f};              // ParticleMultiplier
    std::uint8_t bush_shadow_allowed_a90{1};           // BushShadowAllowed, block +100h
    Rgba ambient_f48{0.5f, 0.5f, 0.5f, 1.0f};          // Ambient, a = L*scale + offset
    Rgba ambient_mode3_f58{0.5f, 0.5f, 0.5f, 1.0f};    // UWAmbient, rgb*L, a = 1.0
    // AmbientCube.HDR Left, Right, Top, Bottom, Front, Back; a = L*scale + offset.
    // 004B6800 leaves these unset (indeterminate natively); zero here.
    std::array<Rgba, 6> ambient_cube_f68{};
    Rgba base_diffuse_fc8{1.0f, 1.0f, 1.0f, 1.0f};     // Diffuse, a = 1.0
    Rgba diffuse_mode3_fd8{1.0f, 1.0f, 1.0f, 1.0f};    // UWDiffuse, rgb*L, a = 1.0
    Rgba base_specular_fe8{1.0f, 1.0f, 1.0f, 1.0f};    // Specular or 0.6 grey, a = 1.0
    float diffuse_scale_ffc{1.0f};                     // Diffuse.Luminance when linearizing
    float specular_scale_1000{0.0f};                   // Specular.Luminance when linearizing
    float horizontal_angle_1004{0.61086524f};          // radians (35 degrees default)
    float vertical_angle_1008{0.61086524f};
    float horizontal_angle_copy_100c{0.61086524f};     // record ctor 004DA409 default
    float vertical_angle_copy_1010{0.61086524f};
};

// The foliage system [00F8C210] setters 004EA870 calls.
struct SceneLightsHost {
    virtual ~SceneLightsHost() = default;
    virtual void set_bush_diffuse_factor_00ad7140(float factor) = 0;
    virtual void set_bush_texture_name_00ad7fe0(const std::string& name) = 0;
    // The image pushes a whole stack dword whose low byte is the property byte
    // (004EAA8A); the upper bytes are not initialised. The host passes the byte.
    virtual void set_bush_normal_allowed_00ad5730(std::uint8_t allowed) = 0;
};

// 004EA870: __fastcall(ECX = Lights bag, EDX = scene record), RET; caller 004EE4A3.
bool read_sun_light_properties_004ea870(const ScenePropertyBagModel& model, int bag_index,
                                        bool linearize, SceneLightsFields& out,
                                        SceneLightsHost& host, std::string& error);

// 004B6800 (via 004EE4AA..004EE4C2): the lighting block F48h..100Bh that the sky
// reader copies over the record when the header has no Lights sub-bag.
void reset_lighting_configuration_004b6800(SceneLightsFields& out) noexcept;

// ---------------------------------------------------------------------------
// 004EE450 targets (strings are NativeStrings in the record).
struct SceneSkyFields {
    std::string refl_cubemap_b98;          // ReflCubemap, "HDR_" prefixed when linearizing
    std::string ocean_refl_cubemap_ba0;    // stem + "_ocean" + ".dds", likewise prefixed
    std::string skybox_c54{"sky_001"};     // Skybox; record ctor default 00CE7AB0
    float skybox_rotate_c5c{0.0f};         // SkyboxRotate, degrees (00BBF1A0)
    float skybox_luminance_c60{0.0f};      // SkyboxLuminance
    std::string sun_effect_1044;           // Sun (string), effect name, game+19D4h
    std::string thunderstorm_effect_104c;  // ThunderStorm, effect name, game+19DCh
};

struct SceneSkyHost : SceneLightsHost {
    // 004E7220 with the header bag and the record (cloud areas, gui_screens_3.hpp).
    virtual void read_cloud_areas_004e7220(const ScenePropertyBagModel& model, int bag_index) = 0;
};

// 004EE450: __fastcall(ECX = g_Environment bag, EDX = scene record), RET; caller 004F230B.
bool read_sky_and_lights_004ee450(const ScenePropertyBagModel& model, int bag_index,
                                  bool linearize, SceneSkyFields& sky, SceneLightsFields& lights,
                                  SceneSkyHost& host, std::string& error);

// ---------------------------------------------------------------------------
// 004ED690 targets: three environment sounds and 2 x 9 music states.
inline constexpr int kMusicPartyCount = 2;
inline constexpr int kMusicStateCount = 9;   // Default..Custom3
inline constexpr int kEnvironmentSoundCount = 3;  // Ocean, Air, Underwater

struct SceneEnvironmentSound {   // record+D60h + i*14h
    std::string path_d60;        // "sound/environment/" + FileName
    float distance_d68{0.0f};    // Distance, Float only
    float distance2_d6c{0.0f};   // Distance2, Float only
};

struct SceneMusicState {         // record+D98h + (party*9 + state)*18h
    std::string path_d98;        // "sound/music/" + FileName, cleared first
    std::string channel_da0;     // Channel, cleared first
};

struct SceneMusicFields {
    std::array<SceneEnvironmentSound, kEnvironmentSoundCount> environment{};
    std::array<SceneMusicState, kMusicPartyCount * kMusicStateCount> music{};
};

struct SceneMusicHost {
    virtual ~SceneMusicHost() = default;
    // 00A83FD0 on [00F8BBE8] then 004E7BB0 into record+DA8h + slot*18h.
    virtual void assign_sound_effect(int slot, const std::string& name) = 0;
    // 00A79480 on [00F8BBCC](&path, party, state): the manager's slot +1C0h + (party*9+state)*4.
    virtual void set_music_track_00a79480(const std::string& path, int party, int state) = 0;
};

// 004ED690: __fastcall(ECX = Sound bag, EDX = scene record), RET; caller 004F22C0. Uses
// 004ED530 (__fastcall(ECX = Ambient entry, EDX = key), stack (index, record),
// RET 8) and 004ED2D0 (__fastcall(ECX = state entry or null, EDX = party),
// stack (state, record), RET 8).
bool read_music_and_ambience_004ed690(const ScenePropertyBagModel& model, int bag_index,
                                      SceneMusicFields& out, SceneMusicHost& host,
                                      std::string& error);

// Host adapter, not a native routine: the host's parsed header block
// (include/bsp/scene_file.hpp, SceneDocument::header.properties) as a bag model
// for the readers above. Int, Float, Bool, String and V3 assignments carry over;
// a value the host parser left without a decoded payload (empty or partial
// assignment) is omitted, so a reader reports it absent where the image would
// keep the record's previous value. Root is bag 0.
ScenePropertyBagModel scene_bag_from_block(const ScenePropertyBlock& block);

}  // namespace bsp::scene_weather_cc13
