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
// emitter 00801400 draws from 00BD2F10 with ECX = 1 (stream 1, not the shared
// gameplay stream 0; 00801439, census site 0080144F; cc13-sys6b) every frame while
// block+60h (IsRain) is set (docs/CC13_RNG_STREAM_CENSUS.md). Nothing here wires
// rain into the host.

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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

// 004E5D20 BSP_Color_ScaleRgbaByScalar, __thiscall(ECX = float rgba[4]; float s), RET 4:
// rgba[i] *= s for i = 0..3, each product stored to float. Caller 004EC99E (weather fog colour L).
void scale_rgba_004e5d20(float* rgba, float s) noexcept;

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

// ---------------------------------------------------------------------------
// 004EB9B0 g_Ocean (caller 004F2205). Block offsets match include/bsp/water_render.hpp
// water_render::block (record offset - 990h). The two particle descriptors are
// separate objects the record points at (+C18h, +C1Ch), read by 004EB180 and by
// the tail routine 004EB5A0 BSP_SceneRecord_ReadWaterBeamParticles (its own function
// 004EB5A0-004EB9A7 since 2026-10-12; reached only by the tail jump at 004ECA1B).
struct SceneParticleSetFields {           // [record+C18h] / [record+C1Ch]
    std::int32_t count_00{0};             // NumParticles / NumBeams, raw dword
    float speed_04{0.0f};                 // Speed
    std::string texture_file_08;          // TextureFile
    Rgba color_10{};                      // Color.R/G/B/A, rgb linearized when set
    std::array<float, 16> tex_20{};       // Tex00..Tex15 (WaterParticle Tex04 is halved)
};

struct SceneOceanFields {
    // Foam
    float foam_scroll_speed_990{0.0f};
    float foam_noise_scroll_speed_994{0.0f};
    float foam_fade_range_998{0.0f};
    std::uint8_t use_foam_99c{0};
    float foam_height_9a0{0.0f};
    float foam_scale_9a4{0.0f};
    float foam_noise_scale_9a8{0.0f};
    std::string foam_texture_9ac;
    std::string foam_noise_texture_9b4;
    // Waves
    float waves_falloff_9bc{0.0f};
    float waves_layer_scale_9c0{0.0f};
    float waves_time_scale_9c4{0.0f};
    float wave_dampening_depth_9c8{0.0f};
    float wave_dampening_power_9cc{0.0f};
    float wave_dampening_scale_9d0{0.0f};
    std::string wave_damp_model_9d4;
    float waves_wind_direction_9dc{0.0f};
    float waves_wind_speed_9e0{0.0f};
    float waves_wave_height_9e4{0.0f};
    float waves_choppy_factor_9e8{0.0f};
    float waves_height_scale_9ec{0.0f};
    // Fog (water); Minvalue and Maxvalue are stored times 0.01.
    float uw_exp_dist_maxdist_a94{0.0f};
    float uw_exp_dist_minvalue_a98{0.0f};
    float uw_exp_height_cutdepth_a9c{0.0f};
    float uw_exp_height_maxdepth_aa0{0.0f};
    float uw_exp_height_maxvalue_aa4{0.0f};
    Rgba uw_fog_color_aa8{};                  // rgba*L (004E5D20), a = MaxDarken
    // Caustics
    float caustics_scale_ab8{0.0f};
    Rgba caustics_color_abc{};                // rgb only
    std::string caustics_type_acc;
    // Optics
    std::array<float, 4> sky_reflection_ad4{};  // 7SkyRefl BaseFact, PowExp, PowDiv, DesatFact
    Rgba local_reflection_tint_ae4{};
    Rgba reflection_tint_af4{};
    Rgba refraction_tint_b04{};
    Rgba uw_refraction_tint_b14{};
    std::array<float, 3> uw_glow_b24{};       // 8UWBaseGlow, 8UWSkyGlow, 8UWSunSpot (optional)
    Rgba spray_tint_b30{};                    // linearized unconditionally (image quirk)
    float shadow_strength_b40{0.0f};
    float refraction_tint_exponent_b44{0.0f};
    float refraction_tint_power_b48{0.0f};
    float deep_water_refraction_exponent_b4c{0.0f};
    float deep_water_refraction_tint_power_b50{0.0f};
    float refraction_max_depth_b54{0.0f};
    float refraction_depth_fade_range_b58{0.0f};
    std::array<float, 3> fresnel_b5c{};       // Exponent, Scale, Offset
    std::array<float, 3> shore_fade_b68{};    // Depth, Range, Power
    Rgba refr_specular_color_b74{};
    float under_water_refr_index_b84{0.0f};
    float reflection_multiplier_bbc{0.0f};
    float final_multiplier_bc0{0.0f};
    // NormalMaps
    std::array<float, 3> normal_map_layer_scale_bf0{};
    std::array<float, 3> normal_map_scroll_speed_c00{};
    std::string normal_map_texture_c10;
    std::uint8_t simple_ocean_c20{0};         // 0_SimpleOcean, Bool only
    SceneParticleSetFields water_particle_c18;
    SceneParticleSetFields beams_c1c;
};

// 004EB9B0: __fastcall(ECX = g_Ocean bag, EDX = record), RET; caller 004F2205.
bool read_water_properties_004eb9b0(const ScenePropertyBagModel& model, int bag_index,
                                    bool linearize, SceneOceanFields& out, std::string& error);

// ---------------------------------------------------------------------------
// 004EE9E0 g_Terrain (caller 004F232A). Scalars go to the terrain shader globals
// at 00E18808..00E18988 (named by address); textures to the record; the LUT name
// to the global NativeString 00E18990.
struct SceneTerrainSubtypes {             // w0, d01, w1, d12, w2, each times 0.01
    std::array<float, 5> v{};
};

struct SceneTerrainFields {
    // Optional scalars keyed by their global address.
    struct Scalar { const char* key; std::uint32_t global; float value; bool set; };
    std::array<Scalar, 26> scalars{};
    std::array<Rgba, 3> uw_color_e18960{};          // h_UW_Color0..2, rgb (stride 10h)
    std::array<bool, 3> uw_color_set{};
    SceneTerrainSubtypes coral_e18894, uw_rock_e1886c, shore_rock_e18844;
    SceneTerrainSubtypes grass_e18830, dirt_e1881c, ground_rock_e18808;
    bool uw_subtypes_set{false}, shore_subtypes_set{false}, ground_subtypes_set{false};
    std::string diff_atlas_bc4, norm_atlas_bcc, mask_texture_bd4, noise_texture_bdc,
        static_shadow_texture_be4;
    std::array<float, 3> static_shadow_shot_e1891c{};  // Size, OffsetX, OffsetZ (optional)
    std::array<bool, 3> static_shadow_shot_set{};
    std::string terrain_lut_e18990;                    // "LUT_" + stem + ".tga"
    bool terrain_lut_set{false};
};

// 004EE9E0: __fastcall(ECX = g_Terrain bag, EDX = record), RET. `game_record_90c`
// is [[00E188A8]+5FCh]+90Ch, the string the LUT name is derived from (the image
// reads the game's current record, not EDX). +90Ch is the scene path (length +90Ch,
// buffer +910h), written only by 004DA2A0 (zeroed) and 004E1D70 (004E1FD2/004E2017, copied
// from that function's scenePath argument).
bool read_static_shadow_and_shader_textures_004ee9e0(const ScenePropertyBagModel& model,
                                                     int bag_index,
                                                     const std::string& game_record_90c,
                                                     SceneTerrainFields& out, std::string& error);

// The 004EE9E0 rule for 00E18990: empty source -> unchanged (returns false);
// otherwise strip through the last '/' (else the last '\\') and the 4-character
// extension, cap at 14h characters, and wrap as "LUT_" + name + ".tga".
bool terrain_lut_name_004ee9e0(const std::string& source, std::string& lut);

// ---------------------------------------------------------------------------
// 004ED910 BSP_SceneFile_ApplyTerrainTextureHeader: __fastcall(ECX = record), RET;
// caller BSP_SceneFile_ReadHeaderBlock 00469BF0. Publishes record strings to the
// world parameter table, loads the environment sounds, applies FileSystemLua.
struct TerrainTextureHeaderInputs {
    std::string refl_cubemap_b98, cloud_shadow_map_ba8, diff_atlas_bc4, norm_atlas_bcc,
        mask_texture_bd4, noise_texture_bdc, static_shadow_texture_be4;
    std::string terrain_lut_e18990;
    std::string file_system_lua_918;   // FileSystemLua (004F1D91)
    // Environment sound slots record+D5Ch + i*14h: sample handle present, path +4.
    std::array<bool, kEnvironmentSoundCount> env_sample_present{};
    std::array<std::string, kEnvironmentSoundCount> env_path{};
};

struct TerrainTextureHeaderHost {
    virtual ~TerrainTextureHeaderHost() = default;
    virtual void set_world_parameter_string_00b1b830(const std::string& name, const std::string& value) = 0;
    // [00F8D394] vtable+120h at 004EDD95: the only renderer class (00B32410 stores vtable
    // 00D5F0A8) has 00B24DD0 BSP_Renderer_RefreshMaterialTextures in that slot (00D5F1C8),
    // which re-runs 00B19000 on every registered material so the parameters published above
    // take effect.
    virtual void refresh_material_textures_00b24dd0() = 0;
    virtual void acquire_environment_sample(int index, const std::string& path) = 0;  // 00A83FD0 -> D5Ch slot
    virtual void set_retained_channel_00a7f2f0(int index) = 0;         // [00F8BBD8], slot, index
    virtual void vfs_clear_aliases_00bdf940() = 0;
    // The Lua "Files" table of the FileSystemLua script: (name, newname) rows 1..n.
    virtual std::vector<std::pair<std::string, std::string>> read_file_aliases(const std::string& script) = 0;
    virtual void vfs_add_alias_00bdf590(const std::string& name, const std::string& newname) = 0;
    virtual void refresh_bush_textures_00ad60d0() = 0;
};

// The world-parameter names in publish order with the value each receives.
// DiffAtlas_B receives record+BC4h (DiffAtlas_A's string) in the image.
void apply_terrain_texture_header_004ed910(const TerrainTextureHeaderInputs& in,
                                           TerrainTextureHeaderHost& host);

// ---------------------------------------------------------------------------
// 004E8510 g_PostProcess (caller 004F2349): three parameter blocks for the
// renderer service [00F8D39C] and four Colorize texture names in the record.
// Member initializers are the image's stack defaults; every key is required,
// so none survives a successful read.
struct ScenePostProcessHdr {              // 00B0E9B0 block, service +84h..+ACh
    float bright_pass_threshold{0.2f};    // BrightPassTreshold (sic)
    float bright_pass_offset{0.8f};
    float middle_gray{0.18f};
    float adaptation{0.98f};              // 1 - AdaptationPercent/100
    float bloom_scale{1.0f};
    float min_luminance{0.0f};
    float max_luminance{1000.0f};
    float new_hdr_param0{3.0f};
    float new_hdr_param1{10.0f};
    std::uint8_t anti_banding_on{0};
    float particle_luminance_mul{1.0f};
};
struct ScenePostProcessMotionBlur {       // 00B0EA20 block
    float max_velocity{1.0f};
    float velocity_scale{0.5f};
    float min_speed{100.0f};
    float divider{600.0f};
    float max_blur{100.0f};
    std::array<float, 3> unused{};        // +14h..+1Ch stay 0
};
struct ScenePostProcessUnderwaterWave {   // 00B0EA70 block; amplitudes / 1000
    float horiz_speed{4.0f}, horiz_scale{25.0f}, horiz_phase{0.0f}, horiz_amplitude{0.01f};
    float vert_speed{2.7f}, vert_scale{22.0f}, vert_phase{0.6f}, vert_amplitude{0.01f};
};
struct ScenePostProcessHost {
    virtual ~ScenePostProcessHost() = default;
    virtual void set_hdr_params_00b0e9b0(const ScenePostProcessHdr&) = 0;
    virtual void set_motion_blur_params_00b0ea20(const ScenePostProcessMotionBlur&) = 0;
    virtual void set_underwater_wave_params_00b0ea70(const ScenePostProcessUnderwaterWave&) = 0;
};
struct ScenePostProcessFields {           // Colorize strings in the record
    std::string texture_name_c24, texture_name_uw_c2c, texture_name_old_film_c34,
        texture_name_old_film_uw_c3c;
};

// 004E8510: __fastcall(ECX = g_PostProcess bag, EDX = record), RET.
bool read_post_process_004e8510(const ScenePropertyBagModel& model, int bag_index,
                                ScenePostProcessFields& out, ScenePostProcessHost& host,
                                std::string& error);

// Host adapter, not a native routine: the host's parsed header block
// (include/bsp/scene_file.hpp, SceneDocument::header.properties) as a bag model
// for the readers above. Int, Float, Bool, String and V3 assignments carry over;
// a value the host parser left without a decoded payload (empty or partial
// assignment) is omitted, so a reader reports it absent where the image would
// keep the record's previous value. Root is bag 0.
ScenePropertyBagModel scene_bag_from_block(const ScenePropertyBlock& block);

}  // namespace bsp::scene_weather_cc13
