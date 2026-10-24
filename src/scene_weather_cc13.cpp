#include "bsp/scene_weather_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cmath>
#include <limits>
#include <cstring>
#include <stdexcept>

// Scene-header weather, lights, sky and music readers. Evidence and per-key
// tables: docs/CC13_SCENE_WEATHER.md. Every native lookup that the image does
// not null-check throws ReaderFault here; the public entry points turn that into
// `false` plus a message, leaving earlier writes in place.

namespace bsp::scene_weather_cc13 {
namespace {

struct ReaderFault : std::runtime_error {
    using std::runtime_error::runtime_error;
};

constexpr float kLinearizeExponent = 2.2f;   // 00CE89D8
constexpr float kOne = 1.0f;                 // 00D7A24C
constexpr float kSpecularGrey = 0.6f;        // 00CE3D30
// 00CE3D28 (double) = 00 00 00 60 FB 21 09 40: float pi widened to double, not
// exact pi. FMUL double ptr [00CE3D28] at 004EB0E0 and 004EB11B.
constexpr double kPi = 3.1415927410125732;
constexpr double kDegrees = 180.0;           // 00CE3D20 (double)

const ScenePropertyEntry* find(const ScenePropertyBagModel& model, int bag, const char* key) {
    return scene_property_bag_find(model, bag, key);
}

const ScenePropertyEntry& require(const ScenePropertyBagModel& model, int bag, const char* key) {
    const ScenePropertyEntry* entry = find(model, bag, key);
    if (entry == nullptr) {
        throw ReaderFault(std::string("required scene key absent (native dereferences null): ") + key);
    }
    return *entry;
}

// The record's +0Ch dword for the types whose value lives there.
std::uint32_t raw_dword(const ScenePropertyEntry& entry) {
    switch (entry.value.type) {
    case ScenePropertyType::Int:
    case ScenePropertyType::Bool:
    case ScenePropertyType::Enum:
        return static_cast<std::uint32_t>(entry.value.int_value);
    case ScenePropertyType::Float: {
        std::uint32_t bits = 0;
        std::memcpy(&bits, &entry.value.float_value, sizeof bits);
        return bits;
    }
    default:
        throw ReaderFault("scene key '" + entry.key + "' has a type whose +0Ch the model does not keep");
    }
}

// CMP [EAX+4],0 / JNZ: CVTSI2SS for type 0, otherwise the +0Ch bits as a float.
float read_number(const ScenePropertyEntry& entry) {
    if (entry.value.type == ScenePropertyType::Int) {
        return static_cast<float>(entry.value.int_value);
    }
    const std::uint32_t bits = raw_dword(entry);
    float value = 0.0f;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

std::uint8_t read_byte(const ScenePropertyEntry& entry) {
    return static_cast<std::uint8_t>(raw_dword(entry) & 0xFFu);
}

// The +0Ch dword used as a child bag pointer. A non-bag entry whose dword is 0
// is the image's null bag; any other non-bag value would be dereferenced.
// Returns -1 for null.
int child_bag(const ScenePropertyEntry& entry) {
    if (entry.value.type == ScenePropertyType::SubBag) {
        return entry.sub_bag;
    }
    if (raw_dword(entry) == 0) {
        return -1;
    }
    throw ReaderFault("scene key '" + entry.key + "' is used as a sub-bag but is not one");
}

int require_child(const ScenePropertyEntry& entry) {
    const int bag = child_bag(entry);
    if (bag < 0) {
        throw ReaderFault("scene sub-bag '" + entry.key + "' is null (native dereferences it)");
    }
    return bag;
}

const std::string& read_string(const ScenePropertyEntry& entry) {
    if (entry.value.type != ScenePropertyType::String) {
        throw ReaderFault("scene key '" + entry.key + "' read as a string (008F0DF0) is not one");
    }
    return entry.value.text;
}

float req_number(const ScenePropertyBagModel& model, int bag, const char* key) {
    return read_number(require(model, bag, key));
}

void read_rgb(const ScenePropertyBagModel& model, int bag, Rgba& out) {
    out.r = req_number(model, bag, "R");
    out.g = req_number(model, bag, "G");
    out.b = req_number(model, bag, "B");
}

void linearize_if(bool linearize, Rgba& colour) {
    if (linearize) {
        float rgb[3] = {colour.r, colour.g, colour.b};
        linearize_rgb_004e5c00(rgb);
        colour.r = rgb[0];
        colour.g = rgb[1];
        colour.b = rgb[2];
    }
}

// FLD L / FLD ST0 / FMUL component / FSTP: each product of two floats is exact
// in the 80-bit register, so one float rounding matches the image.
void scale_rgba(Rgba& colour, float l) {
    colour.r = colour.r * l;
    colour.g = colour.g * l;
    colour.b = colour.b * l;
    colour.a = colour.a * l;
}

// FLD L / FMUL scale / FADD offset / FSTP: 80-bit natively, double here.
float lum_scale_offset(float l, float scale, float offset) {
    return static_cast<float>(static_cast<double>(l) * scale + offset);
}

// FLD deg / FMUL qword pi / FDIV qword 180 / FSTP float.
float degrees_to_radians(float degrees) {
    return static_cast<float>(static_cast<double>(degrees) * kPi / kDegrees);
}

// ---------------------------------------------------------------------------
// 004ECA30 body.
void read_weather(const ScenePropertyBagModel& model, int bag, bool linearize, SceneWeatherFields& out) {
    // 004ECA39: no null check, stored only for type 3.
    const ScenePropertyEntry& wind = require(model, bag, "CloudsAffectedByWind");
    if (wind.value.type == ScenePropertyType::Bool) {
        out.clouds_affected_by_wind_bec = read_byte(wind);
    }

    // 004ECA64 "Rain": optional; a null child skips to "Fog".
    if (const ScenePropertyEntry* rain_entry = find(model, bag, "Rain")) {
        const int rain = child_bag(*rain_entry);
        if (rain >= 0) {
            out.is_rain_9f0 = read_byte(require(model, rain, "IsRain"));
            out.rain_drop_num_9f4 = static_cast<std::int32_t>(raw_dword(require(model, rain, "RainDropNum")));
            const int speed = child_bag(require(model, rain, "RainSpeed"));
            if (speed >= 0) {
                out.rain_speed_9f8[0] = req_number(model, speed, "X");
                out.rain_speed_9f8[1] = req_number(model, speed, "Y");
                out.rain_speed_9f8[2] = req_number(model, speed, "Z");
            }
            // 004ECB3E: 004772C0 presence test, then 004773A0 and an unchecked child.
            if (find(model, rain, "RainColor") != nullptr) {
                const int colour = require_child(require(model, rain, "RainColor"));
                out.rain_color_bgra_a04[2] = read_byte(require(model, colour, "R"));  // A06h
                out.rain_color_bgra_a04[1] = read_byte(require(model, colour, "G"));  // A05h
                out.rain_color_bgra_a04[0] = read_byte(require(model, colour, "B"));  // A04h
                out.rain_color_bgra_a04[3] = read_byte(require(model, colour, "A"));  // A07h
            }
            if (const ScenePropertyEntry* e = find(model, rain, "RainDropLengthMin")) {
                out.rain_drop_length_min_a08 = read_number(*e);
            }
            if (const ScenePropertyEntry* e = find(model, rain, "RainDropLengthMax")) {
                out.rain_drop_length_max_a0c = read_number(*e);
            }
        }
    }

    // 004ECC1F "Fog": optional; a null child skips to "CloudLights".
    const ScenePropertyEntry* fog_entry = find(model, bag, "Fog");
    const int fog = fog_entry != nullptr ? child_bag(*fog_entry) : -1;
    if (fog >= 0) {
        out.lin_fog_dist_density_a60 = req_number(model, fog, "LinFogDistDensity");
        out.lin_fog_height_density_a6c = req_number(model, fog, "LinFogHeightDensity");
        out.lin_fog_dist_start_a64 = req_number(model, fog, "LinFogDistStart");
        out.lin_fog_dist_end_a68 = req_number(model, fog, "LinFogDistEnd");
        out.lin_fog_height_end_a70 = req_number(model, fog, "LinFogHeightEnd");
        out.lin_fog_uw_height_end_a74 = req_number(model, fog, "LinFogUWHeightEnd");
        struct Clip { const char* key; float SceneWeatherFields::*field; };
        static constexpr Clip kClips[] = {
            {"ObjectRenderClip", &SceneWeatherFields::object_render_clip_a78},
            {"ReflectRenderClip", &SceneWeatherFields::reflect_render_clip_a7c},
            {"ShadowRenderClip", &SceneWeatherFields::shadow_render_clip_a84},
            {"RefractRenderClip", &SceneWeatherFields::refract_render_clip_a80},
            {"xFoliageRenderClip", &SceneWeatherFields::foliage_render_clip_a88},
            {"xFoliageLowResDist", &SceneWeatherFields::foliage_low_res_dist_a8c},
        };
        for (const Clip& clip : kClips) {
            if (const ScenePropertyEntry* e = find(model, fog, clip.key)) {
                out.*clip.field = read_number(*e);
            }
        }

        // 004ECE72 "FogColor": unchecked lookup, null child skips to DirFogColor.
        const int colour = child_bag(require(model, fog, "FogColor"));
        if (colour >= 0) {
            read_rgb(model, colour, out.fog_color_a10);
            // 004ECF1B..004ECF49: r, g, b and the previous +A1Ch times L; the
            // +A1Ch product is dead, overwritten with L at 004ECF63.
            scale_rgba(out.fog_color_a10, req_number(model, colour, "L"));
            out.fog_color_a10.a = req_number(model, colour, "L");
            linearize_if(linearize, out.fog_color_a10);
        }

        // 004ECF7B "DirFogColor": a null child ends the Fog section.
        const int dir = child_bag(require(model, fog, "DirFogColor"));
        if (dir >= 0) {
            static constexpr const char* kFaces[4] = {"Right", "Left", "Front", "Back"};
            int faces[4];
            for (int i = 0; i < 4; ++i) {
                faces[i] = require_child(require(model, dir, kFaces[i]));
            }
            for (int i = 0; i < 4; ++i) {  // 004ECFF0 loop, stride 10h from A20h
                Rgba& c = out.dir_fog_color_a20[i];
                read_rgb(model, faces[i], c);
                scale_rgba(c, req_number(model, faces[i], "L"));
                c.a = kOne;  // 004ED0B3
            }
        }
    }

    // 004ED0BE "CloudLights": optional; no gates beyond a null child per colour.
    const ScenePropertyEntry* lights_entry = find(model, bag, "CloudLights");
    const int lights = lights_entry != nullptr ? child_bag(*lights_entry) : -1;
    if (lights < 0) {
        return;
    }
    struct Light { const char* key; Rgba SceneWeatherFields::*field; };
    static constexpr Light kLights[] = {
        {"Ambient", &SceneWeatherFields::cloud_ambient_1014},
        {"Sun", &SceneWeatherFields::cloud_sun_1024},
        {"Bottom", &SceneWeatherFields::cloud_bottom_1034},
    };
    for (const Light& light : kLights) {
        const int colour = child_bag(require(model, lights, light.key));
        if (colour < 0) {
            if (light.field == &SceneWeatherFields::cloud_bottom_1034) {
                return;
            }
            continue;
        }
        read_rgb(model, colour, out.*light.field);
        linearize_if(linearize, out.*light.field);  // 004ED183 / 004ED21E / 004ED2C0 (tail)
    }
}

// ---------------------------------------------------------------------------
// 004EA870 body.
void read_lights(const ScenePropertyBagModel& model, int bag, bool linearize, SceneLightsFields& out,
                 SceneLightsHost& host) {
    if (const ScenePropertyEntry* tracer = find(model, bag, "WaterTracerColor")) {
        const int colour = require_child(*tracer);
        read_rgb(model, colour, out.water_tracer_color_c44);
        out.water_tracer_color_c44.a = req_number(model, colour, "A");
        linearize_if(linearize, out.water_tracer_color_c44);
    }
    out.particle_multiplier_1054 = req_number(model, bag, "ParticleMultiplier");
    out.bush_shadow_allowed_a90 = read_byte(require(model, bag, "BushShadowAllowed"));
    const float lum_scale = req_number(model, bag, "AmbientLumScale");
    const float lum_offset = req_number(model, bag, "AmbientLumOffset");
    const float bush_diffuse = req_number(model, bag, "BushDiffuseFactor");
    const std::string bush_texture = read_string(require(model, bag, "BushTextureName"));
    const std::uint8_t bush_normal = read_byte(require(model, bag, "BushNormalAllowed"));
    host.set_bush_diffuse_factor_00ad7140(bush_diffuse);
    host.set_bush_texture_name_00ad7fe0(bush_texture);
    host.set_bush_normal_allowed_00ad5730(bush_normal);

    // Ambient: rgb, a = L*scale + offset.
    {
        const int colour = require_child(require(model, bag, "Ambient"));
        read_rgb(model, colour, out.ambient_f48);
        out.ambient_f48.a = lum_scale_offset(req_number(model, colour, "L"), lum_scale, lum_offset);
        linearize_if(linearize, out.ambient_f48);
    }
    // UWAmbient: rgb and the previous alpha times L, then a = 1.0.
    {
        const int colour = require_child(require(model, bag, "UWAmbient"));
        read_rgb(model, colour, out.ambient_mode3_f58);
        scale_rgba(out.ambient_mode3_f58, req_number(model, colour, "L"));
        out.ambient_mode3_f58.a = kOne;
        linearize_if(linearize, out.ambient_mode3_f58);
    }
    // AmbientCube.HDR faces, looked up before any is read.
    {
        const int cube = require_child(require(model, bag, "AmbientCube"));
        const int hdr = require_child(require(model, cube, "HDR"));
        static constexpr const char* kFaces[6] = {"Left", "Right", "Top", "Bottom", "Front", "Back"};
        int faces[6];
        for (int i = 0; i < 6; ++i) {
            faces[i] = require_child(require(model, hdr, kFaces[i]));
        }
        for (int i = 0; i < 6; ++i) {
            Rgba& c = out.ambient_cube_f68[i];
            read_rgb(model, faces[i], c);
            c.a = lum_scale_offset(req_number(model, faces[i], "L"), lum_scale, lum_offset);
            linearize_if(linearize, c);
        }
    }
    // Diffuse: rgb, a = 1.0; scale 1.0 unless linearizing and Luminance exists.
    {
        const int colour = require_child(require(model, bag, "Diffuse"));
        read_rgb(model, colour, out.base_diffuse_fc8);
        out.base_diffuse_fc8.a = kOne;
        linearize_if(linearize, out.base_diffuse_fc8);
        out.diffuse_scale_ffc = kOne;
        if (find(model, colour, "Luminance") != nullptr && linearize) {
            out.diffuse_scale_ffc = req_number(model, colour, "Luminance");
        }
    }
    // UWDiffuse: as UWAmbient.
    {
        const int colour = require_child(require(model, bag, "UWDiffuse"));
        read_rgb(model, colour, out.diffuse_mode3_fd8);
        scale_rgba(out.diffuse_mode3_fd8, req_number(model, colour, "L"));
        out.diffuse_mode3_fd8.a = kOne;
        linearize_if(linearize, out.diffuse_mode3_fd8);
    }
    // Specular: optional; absent gives 0.6 grey without linearizing.
    if (find(model, bag, "Specular") != nullptr) {
        const int colour = require_child(require(model, bag, "Specular"));
        read_rgb(model, colour, out.base_specular_fe8);
        linearize_if(linearize, out.base_specular_fe8);
        out.base_specular_fe8.a = kOne;
        out.specular_scale_1000 = kOne;
        if (find(model, colour, "Luminance") != nullptr && linearize) {
            out.specular_scale_1000 = req_number(model, colour, "Luminance");
        }
    } else {
        out.base_specular_fe8.r = kSpecularGrey;
        out.base_specular_fe8.g = kSpecularGrey;
        out.base_specular_fe8.b = kSpecularGrey;
        out.base_specular_fe8.a = kOne;
        out.specular_scale_1000 = kOne;
        // 004EB031: the image then looks Luminance up in ESI = the stack dword
        // holding the BushNormalAllowed byte, not a bag. Every Lights block in
        // this installation has Specular, so the path is unreached; the host
        // refuses it rather than inventing a bag.
        throw ReaderFault("Lights has no Specular: native looks up Luminance in an uninitialised bag");
    }
    // Angles, degrees to radians; +100Ch/+1010h repeat the two results.
    const int angles = require_child(require(model, bag, "Angles"));
    out.horizontal_angle_1004 = degrees_to_radians(req_number(model, angles, "HorizontalAngle"));
    out.vertical_angle_1008 = degrees_to_radians(req_number(model, angles, "VerticalAngle"));
    out.horizontal_angle_copy_100c = out.horizontal_angle_1004;
    out.vertical_angle_copy_1010 = out.vertical_angle_1008;
}

// 00469840 substring(0, count) with count = size - 4 computed unsigned.
std::string strip_extension(const std::string& name) {
    const std::uint32_t size = static_cast<std::uint32_t>(name.size());
    const std::uint32_t count = size - 4u;
    return name.substr(0, count > size ? size : count);
}

void read_sky(const ScenePropertyBagModel& model, int bag, bool linearize, SceneSkyFields& sky,
              SceneLightsFields& lights, SceneSkyHost& host) {
    host.read_cloud_areas_004e7220(model, bag);
    const ScenePropertyEntry* lights_entry = find(model, bag, "Lights");
    if (lights_entry != nullptr && lights_entry->value.type == ScenePropertyType::SubBag) {
        read_lights(model, lights_entry->sub_bag, linearize, lights, host);
    } else {
        reset_lighting_configuration_004b6800(lights);
    }
    if (const ScenePropertyEntry* e = find(model, bag, "Skybox")) {
        if (e->value.type == ScenePropertyType::String) {
            sky.skybox_c54 = e->value.text;
        }
    }
    if (linearize) {
        sky.skybox_c54 = "HDR_" + sky.skybox_c54;
    }
    if (const ScenePropertyEntry* e = find(model, bag, "SkyboxRotate")) {
        if (e->value.type == ScenePropertyType::Float) {
            sky.skybox_rotate_c5c = e->value.float_value;
        }
    }
    if (const ScenePropertyEntry* e = find(model, bag, "SkyboxLuminance")) {
        if (e->value.type == ScenePropertyType::Float) {
            sky.skybox_luminance_c60 = e->value.float_value;
        }
    }
    if (find(model, bag, "ReflCubemap") != nullptr) {
        const std::string name = read_string(require(model, bag, "ReflCubemap"));
        sky.refl_cubemap_b98 = name;
        sky.ocean_refl_cubemap_ba0 = strip_extension(name) + "_ocean" + ".dds";
    }
    if (linearize) {
        sky.refl_cubemap_b98 = "HDR_" + sky.refl_cubemap_b98;
        sky.ocean_refl_cubemap_ba0 = "HDR_" + sky.ocean_refl_cubemap_ba0;
    }
    if (const ScenePropertyEntry* e = find(model, bag, "Sun")) {
        if (e->value.type == ScenePropertyType::String) {
            sky.sun_effect_1044 = e->value.text;
        }
    }
    if (const ScenePropertyEntry* e = find(model, bag, "ThunderStorm")) {
        if (e->value.type == ScenePropertyType::String) {
            sky.thunderstorm_effect_104c = e->value.text;
        }
    }
}

bool non_empty_string(const ScenePropertyEntry* e) {
    return e != nullptr && e->value.type == ScenePropertyType::String && !e->value.text.empty();
}

// 004ED530.
void read_environment_sound(const ScenePropertyBagModel& model, int ambient_bag, const char* key,
                            SceneEnvironmentSound& out) {
    const ScenePropertyEntry* entry = find(model, ambient_bag, key);
    if (entry == nullptr || entry->value.type != ScenePropertyType::SubBag) {
        return;
    }
    const int bag = entry->sub_bag;
    const ScenePropertyEntry* file = find(model, bag, "FileName");
    if (!non_empty_string(file)) {
        return;  // 004ED578 / 004ED59D jump past Distance and Distance2 as well
    }
    out.path_d60 = "sound/environment/" + file->value.text;
    if (const ScenePropertyEntry* e = find(model, bag, "Distance")) {
        if (e->value.type == ScenePropertyType::Float) {
            out.distance_d68 = e->value.float_value;
        }
    }
    if (const ScenePropertyEntry* e = find(model, bag, "Distance2")) {
        if (e->value.type == ScenePropertyType::Float) {
            out.distance2_d6c = e->value.float_value;
        }
    }
}

// 004ED2D0.
void read_music_state(const ScenePropertyBagModel& model, const ScenePropertyEntry* state, int party,
                      int index, SceneMusicFields& out, SceneMusicHost& host) {
    const int slot = party * kMusicStateCount + index;
    SceneMusicState& music = out.music[slot];
    music.path_d98.clear();
    music.channel_da0.clear();
    if (state != nullptr && state->value.type == ScenePropertyType::SubBag) {
        const int bag = state->sub_bag;
        const ScenePropertyEntry* file = find(model, bag, "FileName");
        if (non_empty_string(file)) {
            music.path_d98 = "sound/music/" + file->value.text;
        }
        const ScenePropertyEntry* channel = find(model, bag, "Channel");
        if (non_empty_string(channel)) {
            music.channel_da0 = channel->value.text;
        }
        const ScenePropertyEntry* effect = find(model, bag, "SoundEffect");
        if (non_empty_string(effect)) {
            host.assign_sound_effect(slot, effect->value.text);
        }
    }
    host.set_music_track_00a79480(music.path_d98, party, index);
}

void read_music(const ScenePropertyBagModel& model, int bag, SceneMusicFields& out, SceneMusicHost& host) {
    const ScenePropertyEntry* ambient = find(model, bag, "Ambient");
    if (ambient != nullptr && ambient->value.type == ScenePropertyType::SubBag) {
        static constexpr const char* kSounds[kEnvironmentSoundCount] = {"Ocean", "Air", "Underwater"};
        for (int i = 0; i < kEnvironmentSoundCount; ++i) {
            read_environment_sound(model, ambient->sub_bag, kSounds[i], out.environment[i]);
        }
    }
    const ScenePropertyEntry* music = find(model, bag, "Music");
    if (music == nullptr || music->value.type != ScenePropertyType::SubBag) {
        return;
    }
    static constexpr const char* kParties[kMusicPartyCount] = {"Party0", "Party1"};
    static constexpr const char* kStates[kMusicStateCount] = {
        "Default", "Tension", "Action", "Victory", "Defeat", "Calm", "Custom1", "Custom2", "Custom3"};
    for (int party = 0; party < kMusicPartyCount; ++party) {
        const ScenePropertyEntry* party_entry = find(model, music->sub_bag, kParties[party]);
        if (party_entry == nullptr || party_entry->value.type != ScenePropertyType::SubBag) {
            if (party == 1) {
                return;
            }
            continue;
        }
        for (int state = 0; state < kMusicStateCount; ++state) {
            read_music_state(model, find(model, party_entry->sub_bag, kStates[state]), party, state, out,
                             host);
        }
    }
}

// ---------------------------------------------------------------------------
// 004EB9B0 g_Ocean and its two particle-set helpers.

// 008F0DF0 then strlen with a null test: a null pointer assigns "".
void read_string_into(const ScenePropertyEntry& entry, std::string& out) {
    out = read_string(entry);
}

void read_tint(const ScenePropertyBagModel& model, int bag, const char* key, bool linearize, Rgba& out) {
    const int colour = require_child(require(model, bag, key));
    read_rgb(model, colour, out);
    linearize_if(linearize, out);
}

// 004EB180 (WaterParticle) and 004EB5A0 (Beam): identical layouts; only the count
// key and the WaterParticle Tex04 halving (004EB3C0, FMUL qword 0.5) differ.
void read_particle_set(const ScenePropertyBagModel& model, int bag, const char* count_key,
                       bool halve_tex04, bool linearize, SceneParticleSetFields& out) {
    out.count_00 = static_cast<std::int32_t>(raw_dword(require(model, bag, count_key)));
    out.speed_04 = req_number(model, bag, "Speed");
    read_string_into(require(model, bag, "TextureFile"), out.texture_file_08);
    const int colour = require_child(require(model, bag, "Color"));
    read_rgb(model, colour, out.color_10);
    out.color_10.a = req_number(model, colour, "A");
    linearize_if(linearize, out.color_10);
    char key[6] = {'T', 'e', 'x', '0', '0', '\0'};
    for (int i = 0; i < 16; ++i) {
        key[3] = static_cast<char>('0' + i / 10);
        key[4] = static_cast<char>('0' + i % 10);
        float value = req_number(model, bag, key);
        if (halve_tex04 && i == 4) {
            value = static_cast<float>(static_cast<double>(value) * 0.5);
        }
        out.tex_20[i] = value;
    }
}

int gated_sub_bag(const ScenePropertyBagModel& model, int bag, const char* key) {
    // Required key (no null test), section skipped unless the type is 6.
    const ScenePropertyEntry& entry = require(model, bag, key);
    return entry.value.type == ScenePropertyType::SubBag ? entry.sub_bag : -1;
}

void read_ocean(const ScenePropertyBagModel& model, int bag, bool linearize, SceneOceanFields& out) {
    const ScenePropertyEntry& simple = require(model, bag, "0_SimpleOcean");
    if (simple.value.type == ScenePropertyType::Bool) {
        out.simple_ocean_c20 = read_byte(simple);
    }

    const int optics = gated_sub_bag(model, bag, "Optics");
    if (optics >= 0) {
        read_tint(model, optics, "1LocalReflectionTint", linearize, out.local_reflection_tint_ae4);
        // 004EBB21: no 00F88A0C test before this 004E5C00 call.
        read_tint(model, optics, "1SprayTintColor", true, out.spray_tint_b30);
        static constexpr const char* kSky[4] = {"7SkyReflBaseFact", "7SkyReflPowExp", "7SkyReflPowDiv",
                                                "7SkyReflDesatFact"};
        for (int i = 0; i < 4; ++i) {
            out.sky_reflection_ad4[i] = req_number(model, optics, kSky[i]);
        }
        static constexpr const char* kGlow[3] = {"8UWBaseGlowStrength", "8UWSkyGlowStrength",
                                                 "8UWSunSpotStrength"};
        for (int i = 0; i < 3; ++i) {
            if (const ScenePropertyEntry* e = find(model, optics, kGlow[i])) {
                out.uw_glow_b24[i] = read_number(*e);
            }
        }
        read_tint(model, optics, "1ReflectionTint", linearize, out.reflection_tint_af4);
        read_tint(model, optics, "1RefractionTint", linearize, out.refraction_tint_b04);
        read_tint(model, optics, "1UWRefractionTint", linearize, out.uw_refraction_tint_b14);
        out.refraction_tint_exponent_b44 = req_number(model, optics, "2RefractionTintExponent");
        out.refraction_tint_power_b48 = req_number(model, optics, "2RefractionTintPower");
        out.deep_water_refraction_exponent_b4c = req_number(model, optics, "2DeepWaterRefractionExponent");
        out.deep_water_refraction_tint_power_b50 = req_number(model, optics, "2DeepWaterRefractionTintPower");
        out.shadow_strength_b40 = req_number(model, optics, "9ShadowStrength");
        out.refraction_max_depth_b54 = req_number(model, optics, "3RefractionMaxDepth");
        out.refraction_depth_fade_range_b58 = req_number(model, optics, "3RefractionDepthFadeRange");
        read_tint(model, optics, "RefrSpecularColor", linearize, out.refr_specular_color_b74);
        out.fresnel_b5c[0] = req_number(model, optics, "4FresnelExponent");
        out.fresnel_b5c[1] = req_number(model, optics, "4FresnelScale");
        out.fresnel_b5c[2] = req_number(model, optics, "4FresnelOffset");
        out.shore_fade_b68[0] = req_number(model, optics, "5ShoreFadeDepth");
        out.shore_fade_b68[1] = req_number(model, optics, "5ShoreFadeRange");
        out.shore_fade_b68[2] = req_number(model, optics, "5ShoreFadePower");
        out.under_water_refr_index_b84 = req_number(model, optics, "UnderWaterRefrIndex");
        out.reflection_multiplier_bbc = req_number(model, optics, "6ReflectionMultiplier");
        out.final_multiplier_bc0 = req_number(model, optics, "6FinalMultiplier");
    }

    const int waves = gated_sub_bag(model, bag, "Waves");
    if (waves >= 0) {
        out.waves_wind_direction_9dc = req_number(model, waves, "WindDirection");
        out.waves_wind_speed_9e0 = req_number(model, waves, "WindSpeed");
        out.waves_wave_height_9e4 = req_number(model, waves, "WaveHeight");
        out.waves_choppy_factor_9e8 = req_number(model, waves, "ChoppyWavesFactor");
        out.waves_height_scale_9ec = req_number(model, waves, "HeightScale");
        out.waves_falloff_9bc = req_number(model, waves, "Falloff");
        out.waves_layer_scale_9c0 = req_number(model, waves, "LayerScale");
        out.waves_time_scale_9c4 = req_number(model, waves, "TimeScale");
        out.wave_dampening_depth_9c8 = req_number(model, waves, "1WaveDampeningDepth");
        out.wave_dampening_power_9cc = req_number(model, waves, "1WaveDampeningPower");
        out.wave_dampening_scale_9d0 = req_number(model, waves, "1WaveDampeningScale");
        read_string_into(require(model, waves, "1WaveDampModel"), out.wave_damp_model_9d4);
    }

    const int normals = gated_sub_bag(model, bag, "NormalMaps");
    if (normals >= 0) {
        static constexpr const char* kScale[3] = {"layer0_Scale", "layer1_Scale", "layer2_Scale"};
        static constexpr const char* kSpeed[3] = {"layer0_ScrollSpeed", "layer1_ScrollSpeed",
                                                  "layer2_ScrollSpeed"};
        for (int i = 0; i < 3; ++i) {  // read order scale0, speed0, scale1, ...
            out.normal_map_layer_scale_bf0[i] = req_number(model, normals, kScale[i]);
            out.normal_map_scroll_speed_c00[i] = req_number(model, normals, kSpeed[i]);
        }
        read_string_into(require(model, normals, "Texture"), out.normal_map_texture_c10);
    }

    const int foam = gated_sub_bag(model, bag, "Foam");
    if (foam >= 0) {
        out.use_foam_99c = read_byte(require(model, foam, "UseFoam"));
        out.foam_scroll_speed_990 = req_number(model, foam, "ScrollSpeed");
        out.foam_noise_scroll_speed_994 = req_number(model, foam, "NoiseScrollSpeed");
        out.foam_height_9a0 = req_number(model, foam, "Height");
        out.foam_fade_range_998 = req_number(model, foam, "FadeRange");
        out.foam_scale_9a4 = req_number(model, foam, "Scale");
        out.foam_noise_scale_9a8 = req_number(model, foam, "NoiseScale");
        read_string_into(require(model, foam, "Texture"), out.foam_texture_9ac);
        read_string_into(require(model, foam, "NoiseTexture"), out.foam_noise_texture_9b4);
    }

    // 004EC648 "Caustics": optional, child not null-tested.
    if (const ScenePropertyEntry* caustics_entry = find(model, bag, "Caustics")) {
        const int caustics = require_child(*caustics_entry);
        if (find(model, caustics, "CausticsColor") != nullptr) {
            read_tint(model, caustics, "CausticsColor", linearize, out.caustics_color_abc);
        }
        if (const ScenePropertyEntry* e = find(model, caustics, "CausticsScale")) {
            out.caustics_scale_ab8 = read_number(*e);
        }
        if (const ScenePropertyEntry* e = find(model, caustics, "CausticsType")) {
            read_string_into(*e, out.caustics_type_acc);
        }
    }

    // 004EC77C "Fog": optional; a null child skips to WaterParticle.
    const ScenePropertyEntry* fog_entry = find(model, bag, "Fog");
    const int fog = fog_entry != nullptr ? child_bag(*fog_entry) : -1;
    if (fog >= 0) {
        if (const ScenePropertyEntry* e = find(model, fog, "UWExpDistMaxdist")) {
            out.uw_exp_dist_maxdist_a94 = read_number(*e);
        }
        // FLD / FMUL qword 0.01 (00CE6638) / FSTP float: 80-bit natively.
        if (const ScenePropertyEntry* e = find(model, fog, "UWExpDistMinvalue")) {
            out.uw_exp_dist_minvalue_a98 = static_cast<float>(static_cast<double>(read_number(*e)) * 0.01);
        }
        if (const ScenePropertyEntry* e = find(model, fog, "UWExpHeightCutdepth")) {
            out.uw_exp_height_cutdepth_a9c = read_number(*e);
        }
        if (const ScenePropertyEntry* e = find(model, fog, "UWExpHeightMaxdepth")) {
            out.uw_exp_height_maxdepth_aa0 = read_number(*e);
        }
        if (const ScenePropertyEntry* e = find(model, fog, "UWExpHeightMaxvalue")) {
            out.uw_exp_height_maxvalue_aa4 = static_cast<float>(static_cast<double>(read_number(*e)) * 0.01);
        }
        if (find(model, fog, "UWFogColor") != nullptr) {
            const int colour = child_bag(require(model, fog, "UWFogColor"));
            if (colour >= 0) {
                read_rgb(model, colour, out.uw_fog_color_aa8);
                // 004E5D20: rgba *= L, including the previous +AB4h, then MaxDarken.
                scale_rgba(out.uw_fog_color_aa8, req_number(model, colour, "L"));
                out.uw_fog_color_aa8.a = req_number(model, colour, "MaxDarken");
                linearize_if(linearize, out.uw_fog_color_aa8);
            }
        }
    }

    const ScenePropertyEntry* particle = find(model, bag, "WaterParticle");
    if (particle != nullptr && particle->value.type == ScenePropertyType::SubBag) {
        read_particle_set(model, particle->sub_bag, "NumParticles", true, linearize, out.water_particle_c18);
    }
    const ScenePropertyEntry* beam = find(model, bag, "Beam");
    if (beam != nullptr && beam->value.type == ScenePropertyType::SubBag) {
        read_particle_set(model, beam->sub_bag, "NumBeams", false, linearize, out.beams_c1c);
    }
}

// ---------------------------------------------------------------------------
// 004EE9E0 g_Terrain.
struct TerrainScalarRow { const char* key; std::uint32_t global; };
constexpr TerrainScalarRow kTerrainScalars[26] = {
    {"a_UW_Coral_Sand_Slope", 0x00E188F8}, {"ab_UW_Sand_Rock_Slope", 0x00E188FC},
    {"a_UW_Coral_Sand_Delta", 0x00E18900}, {"ab_UW_Sand_Rock_Delta", 0x00E18904},
    {"c_GROUND_Grass_Dirt_Slope", 0x00E188E4}, {"cb_GROUND_Dirt_Rock_Slope", 0x00E188E8},
    {"b_SHORE_Sand_Rock_Delta", 0x00E188EC}, {"c_GROUND_Grass_Dirt_Delta", 0x00E188F0},
    {"cb_GROUND_Dirt_Rock_Delta", 0x00E188F4}, {"d_Shore_Height", 0x00E18928},
    {"d_Dirt_Height", 0x00E1892C}, {"e_TerrSlope_Scale", 0x00E1894C},
    {"e_UV_Scale", 0x00E18950}, {"e_MaskUV_Scale", 0x00E18954},
    {"eb_UW_TerrSlope_Scale", 0x00E18958}, {"eb_UW_UV_Scale", 0x00E1895C},
    {"x_Slope_Noise_Divisor", 0x00E18938}, {"x_Slope_Noise_Scaler", 0x00E1893C},
    {"x_Noise_UVmul", 0x00E18940}, {"x_Height_Noise_Divisor", 0x00E18944},
    {"x_Height_Noise_Scaler", 0x00E18948}, {"f_NmPower_Coral", 0x00E18908},
    {"f_NmPower_Sand", 0x00E1890C}, {"f_NmPower_Grass", 0x00E18910},
    {"f_NmPower_Dirt", 0x00E18914}, {"f_NmPower_Rock", 0x00E18918},
};

// Five required numbers, then each times 0.01 (FLD qword 0.01 once, then per
// value FLD / FMUL ST,ST(1) (D8 C9) / FSTP float).
void read_subtypes(const ScenePropertyBagModel& model, int group_bag, const char* key, char prefix0,
                   SceneTerrainSubtypes& out) {
    const int bag = require_child(require(model, group_bag, key));
    const char p = prefix0;
    const std::string names[5] = {std::string(1, p) + "0_width",
                                  std::string(1, p) + "0" + p + "1_delta",
                                  std::string(1, p) + "1_width",
                                  std::string(1, p) + "1" + p + "2_delta",
                                  std::string(1, p) + "2_width"};
    for (int i = 0; i < 5; ++i) {
        out.v[i] = req_number(model, bag, names[i].c_str());
    }
    for (float& v : out.v) {
        v = static_cast<float>(static_cast<double>(v) * 0.01);
    }
}

void read_terrain(const ScenePropertyBagModel& model, int bag, const std::string& game_record_90c,
                  SceneTerrainFields& out) {
    for (std::size_t i = 0; i < out.scalars.size(); ++i) {
        SceneTerrainFields::Scalar& s = out.scalars[i];
        s.key = kTerrainScalars[i].key;
        s.global = kTerrainScalars[i].global;
        if (const ScenePropertyEntry* e = find(model, bag, s.key)) {
            s.value = read_number(*e);
            s.set = true;
        }
    }
    static constexpr const char* kUwColours[3] = {"h_UW_Color0", "h_UW_Color1", "h_UW_Color2"};
    for (int i = 0; i < 3; ++i) {
        const int colour = child_bag(require(model, bag, kUwColours[i]));
        if (colour >= 0) {
            read_rgb(model, colour, out.uw_color_e18960[i]);
            out.uw_color_set[i] = true;
        }
    }
    const int uw = child_bag(require(model, bag, "i_UW_subtypes"));
    if (uw >= 0) {
        read_subtypes(model, uw, "coral_subtypes", 'c', out.coral_e18894);
        read_subtypes(model, uw, "rock_subtypes", 'r', out.uw_rock_e1886c);
        out.uw_subtypes_set = true;
    }
    // 004EF2D3: the SHORE child is only tested; its values come from the
    // i_UW_subtypes bag still in EDI (rock_subtypes). With no UW bag the image
    // would look the key up in a null bag.
    if (child_bag(require(model, bag, "i_SHORE_subtypes")) >= 0) {
        if (uw < 0) {
            throw ReaderFault("i_SHORE_subtypes without i_UW_subtypes: native reads a null bag");
        }
        read_subtypes(model, uw, "rock_subtypes", 'r', out.shore_rock_e18844);
        out.shore_subtypes_set = true;
    }
    const int ground = child_bag(require(model, bag, "i_GROUND_subtypes"));
    if (ground >= 0) {
        read_subtypes(model, ground, "grass_subtypes", 'g', out.grass_e18830);
        read_subtypes(model, ground, "dirt_subtypes", 'd', out.dirt_e1881c);
        read_subtypes(model, ground, "rock_subtypes", 'r', out.ground_rock_e18808);
        out.ground_subtypes_set = true;
    }
    struct Tex { const char* key; std::string SceneTerrainFields::*field; };
    static constexpr Tex kTextures[] = {
        {"g_DiffAtlas", &SceneTerrainFields::diff_atlas_bc4},
        {"g_NormAtlas", &SceneTerrainFields::norm_atlas_bcc},
        {"g_MaskTexture", &SceneTerrainFields::mask_texture_bd4},
        {"g_NoiseTexture", &SceneTerrainFields::noise_texture_bdc},
        {"g_StaticShadowTexture", &SceneTerrainFields::static_shadow_texture_be4},
    };
    for (const Tex& t : kTextures) {
        if (find(model, bag, t.key) != nullptr) {
            out.*t.field = read_string(require(model, bag, t.key));
        }
    }
    static constexpr const char* kShot[3] = {"ga_StaticShadowShotSize", "ga_StaticShadowShotOffsetX",
                                             "ga_StaticShadowShotOffsetZ"};
    for (int i = 0; i < 3; ++i) {
        if (const ScenePropertyEntry* e = find(model, bag, kShot[i])) {
            out.static_shadow_shot_e1891c[i] = read_number(*e);
            out.static_shadow_shot_set[i] = true;
        }
    }
    out.terrain_lut_set = terrain_lut_name_004ee9e0(game_record_90c, out.terrain_lut_e18990);
}

// ---------------------------------------------------------------------------
// 004E8510 g_PostProcess.
void read_post_process(const ScenePropertyBagModel& model, int bag, ScenePostProcessFields& out,
                       ScenePostProcessHost& host) {
    const int hdr_bag = gated_sub_bag(model, bag, "HDR");
    if (hdr_bag >= 0) {
        ScenePostProcessHdr hdr;
        hdr.bright_pass_threshold = req_number(model, hdr_bag, "BrightPassTreshold");
        hdr.bright_pass_offset = req_number(model, hdr_bag, "BrightPassOffset");
        hdr.middle_gray = req_number(model, hdr_bag, "MiddleGray");
        // FLD / FDIV qword 100 / FLD1 / FSUBRP / FSTP float: 80-bit natively.
        hdr.adaptation = static_cast<float>(1.0 - static_cast<double>(req_number(model, hdr_bag,
                                                                                 "AdaptationPercent")) / 100.0);
        hdr.bloom_scale = req_number(model, hdr_bag, "BloomScale");
        hdr.min_luminance = req_number(model, hdr_bag, "MinLuminance");
        hdr.max_luminance = req_number(model, hdr_bag, "MaxLuminance");
        hdr.new_hdr_param1 = req_number(model, hdr_bag, "NewHDRParam1");
        hdr.new_hdr_param0 = req_number(model, hdr_bag, "NewHDRParam0");
        hdr.anti_banding_on = read_byte(require(model, hdr_bag, "AntiBandingON"));
        hdr.particle_luminance_mul = req_number(model, hdr_bag, "ParticleLuminanceMul");
        host.set_hdr_params_00b0e9b0(hdr);
    }
    const int blur_bag = gated_sub_bag(model, bag, "MotionBlur");
    if (blur_bag >= 0) {
        ScenePostProcessMotionBlur blur;
        blur.max_velocity = req_number(model, blur_bag, "MaxVelocity");
        blur.velocity_scale = req_number(model, blur_bag, "VelocityScale");
        blur.min_speed = req_number(model, blur_bag, "MinSpeed");
        blur.max_blur = req_number(model, blur_bag, "MaxBlur");
        blur.divider = req_number(model, blur_bag, "Divider");
        host.set_motion_blur_params_00b0ea20(blur);
    }
    const int wave_bag = gated_sub_bag(model, bag, "UnderWaterWave");
    if (wave_bag >= 0) {
        ScenePostProcessUnderwaterWave wave;
        wave.horiz_speed = req_number(model, wave_bag, "HorizSpeed");
        wave.horiz_scale = req_number(model, wave_bag, "HorizScale");
        wave.horiz_phase = req_number(model, wave_bag, "HorizPhase");
        // FDIV qword 1000 (00CE47A0) then FSTP float.
        wave.horiz_amplitude = static_cast<float>(static_cast<double>(req_number(model, wave_bag,
                                                                                 "HorizAmplitude")) / 1000.0);
        wave.vert_speed = req_number(model, wave_bag, "VertSpeed");
        wave.vert_scale = req_number(model, wave_bag, "VertScale");
        wave.vert_phase = req_number(model, wave_bag, "VertPhase");
        wave.vert_amplitude = static_cast<float>(static_cast<double>(req_number(model, wave_bag,
                                                                                "VertAmplitude")) / 1000.0);
        host.set_underwater_wave_params_00b0ea70(wave);
    }
    const int colorize = gated_sub_bag(model, bag, "Colorize");
    if (colorize >= 0) {
        read_string_into(require(model, colorize, "TextureName"), out.texture_name_c24);
        read_string_into(require(model, colorize, "TextureNameUW"), out.texture_name_uw_c2c);
        read_string_into(require(model, colorize, "TextureNameOldFilm"), out.texture_name_old_film_c34);
        read_string_into(require(model, colorize, "TextureNameOldFilmUW"), out.texture_name_old_film_uw_c3c);
    }
}

void append_block(ScenePropertyBagModel& model, int bag, const ScenePropertyBlock& block) {
    for (const SceneProperty& prop : block.values) {
        ScenePropertyValue value;
        if (prop.has_integer) {
            value.type = ScenePropertyType::Int;
            value.int_value = prop.integer_value;
        } else if (prop.has_float) {
            value.type = ScenePropertyType::Float;
            value.float_value = prop.float_value;
        } else if (prop.has_boolean) {
            value.type = ScenePropertyType::Bool;
            value.int_value = prop.boolean_value;
        } else if (prop.has_vector3) {
            value.type = ScenePropertyType::Vector3;
            for (int i = 0; i < 3; ++i) {
                value.vector[i] = prop.vector3[i];
            }
        } else if (prop.type_letter == "S" || prop.type_letter == "s") {
            if (prop.values.empty()) {
                continue;
            }
            value.type = ScenePropertyType::String;
            value.text = prop.values.front();
        } else {
            continue;
        }
        ScenePropertyEntry entry;
        entry.key = prop.key;
        entry.value = std::move(value);
        model.bags[bag].entries.push_back(std::move(entry));
    }
    for (const auto& child : block.blocks) {
        const int index = static_cast<int>(model.bags.size());
        model.bags.emplace_back();
        ScenePropertyEntry entry;
        entry.key = child.first;
        entry.value.type = ScenePropertyType::SubBag;
        entry.sub_bag = index;
        model.bags[bag].entries.push_back(std::move(entry));
        append_block(model, index, child.second);
    }
}

template <class Body>
bool guarded(std::string& error, Body&& body) {
    try {
        body();
        return true;
    } catch (const ReaderFault& fault) {
        error = fault.what();
        return false;
    }
}

}  // namespace

void linearize_rgb_004e5c00(float* rgb) noexcept {
    for (int i = 0; i < 3; ++i) {
        float c = rgb[i];
        if (c == 0.0f) {
            rgb[i] = 0.0f;
            continue;
        }
        if (c < 0.0f) {
            c = -0.0f - c;
        }
        // cc13-fixture2 (fixture-found): an infinite exponent product makes FRNDINT/FSUB form
        // inf - inf, so F2XM1/FSCALE return the default NaN instead of inf or 0. Follow-up: the
        // callers include the frontend tactical library (00502480, 00504130) on the main
        // thread, so the tail's FSUB and FADDP round to 24 bits (cc13x87::x87_pow2_tail24).
        const double y = cc13x87::x87_fyl2x(kLinearizeExponent, c);
        rgb[i] = static_cast<float>(cc13x87::x87_pow2_tail24(y));
    }
}

bool read_weather_fog_and_cloud_lights_004eca30(const ScenePropertyBagModel& model, int bag_index,
                                                bool linearize, SceneWeatherFields& out,
                                                std::string& error) {
    return guarded(error, [&] { read_weather(model, bag_index, linearize, out); });
}

bool read_sun_light_properties_004ea870(const ScenePropertyBagModel& model, int bag_index,
                                        bool linearize, SceneLightsFields& out,
                                        SceneLightsHost& host, std::string& error) {
    return guarded(error, [&] { read_lights(model, bag_index, linearize, out, host); });
}

void reset_lighting_configuration_004b6800(SceneLightsFields& out) noexcept {
    out.ambient_f48 = {0.5f, 0.5f, 0.5f, 1.0f};
    out.ambient_mode3_f58 = {0.5f, 0.5f, 0.5f, 1.0f};
    out.ambient_cube_f68 = {};
    out.base_diffuse_fc8 = {1.0f, 1.0f, 1.0f, 1.0f};
    out.diffuse_mode3_fd8 = {1.0f, 1.0f, 1.0f, 1.0f};
    out.base_specular_fe8 = {1.0f, 1.0f, 1.0f, 1.0f};
    out.diffuse_scale_ffc = 1.0f;
    out.specular_scale_1000 = 0.0f;
    out.horizontal_angle_1004 = 0.61086524f;  // 00CE74F4
    out.vertical_angle_1008 = 0.61086524f;
}

bool read_sky_and_lights_004ee450(const ScenePropertyBagModel& model, int bag_index,
                                  bool linearize, SceneSkyFields& sky, SceneLightsFields& lights,
                                  SceneSkyHost& host, std::string& error) {
    return guarded(error, [&] { read_sky(model, bag_index, linearize, sky, lights, host); });
}

bool read_music_and_ambience_004ed690(const ScenePropertyBagModel& model, int bag_index,
                                      SceneMusicFields& out, SceneMusicHost& host,
                                      std::string& error) {
    return guarded(error, [&] { read_music(model, bag_index, out, host); });
}

bool read_water_properties_004eb9b0(const ScenePropertyBagModel& model, int bag_index,
                                    bool linearize, SceneOceanFields& out, std::string& error) {
    return guarded(error, [&] { read_ocean(model, bag_index, linearize, out); });
}

bool read_static_shadow_and_shader_textures_004ee9e0(const ScenePropertyBagModel& model,
                                                     int bag_index,
                                                     const std::string& game_record_90c,
                                                     SceneTerrainFields& out, std::string& error) {
    return guarded(error, [&] { read_terrain(model, bag_index, game_record_90c, out); });
}

bool terrain_lut_name_004ee9e0(const std::string& source, std::string& lut) {
    if (source.empty()) {
        return false;  // 004EFB94: _stricmp(name, "") == 0 skips the whole tail
    }
    std::string name = source;
    // Last '/' (00CE7898), else last '\\' (00CE7894); then substr(i+1, size-i-5),
    // the count computed unsigned and clamped by 00469840.
    std::size_t sep = name.find_last_of('/');
    if (sep == std::string::npos) {
        sep = name.find_last_of('\\');
    }
    if (sep != std::string::npos) {
        const std::uint32_t size = static_cast<std::uint32_t>(name.size());
        const std::uint32_t count = size - static_cast<std::uint32_t>(sep) - 5u;
        name = name.substr(sep + 1, count);
    }
    if (name.size() > 0x14) {  // 004EFCB6: CMP size,14h / JBE
        name = name.substr(0, 0x14);
    }
    lut = "LUT_" + name + ".tga";
    return true;
}

void apply_terrain_texture_header_004ed910(const TerrainTextureHeaderInputs& in,
                                           TerrainTextureHeaderHost& host) {
    host.set_world_parameter_string_00b1b830("ReflectionCubeMap", in.refl_cubemap_b98);
    host.set_world_parameter_string_00b1b830("CloudShadowMap", in.cloud_shadow_map_ba8);
    host.set_world_parameter_string_00b1b830("DiffAtlas_A", in.diff_atlas_bc4);
    host.set_world_parameter_string_00b1b830("DiffAtlas_B", in.diff_atlas_bc4);  // 004EDAE6 pushes +BC4h
    host.set_world_parameter_string_00b1b830("NormAtlas", in.norm_atlas_bcc);
    host.set_world_parameter_string_00b1b830("MaskTexture", in.mask_texture_bd4);
    host.set_world_parameter_string_00b1b830("NoiseTexture", in.noise_texture_bdc);
    host.set_world_parameter_string_00b1b830("TerrainLUT", in.terrain_lut_e18990);
    host.set_world_parameter_string_00b1b830("StaticShadowTexture", in.static_shadow_texture_be4);
    host.refresh_material_textures_00b24dd0();
    for (int i = 0; i < kEnvironmentSoundCount; ++i) {
        if (!in.env_sample_present[i] && !in.env_path[i].empty()) {
            host.acquire_environment_sample(i, in.env_path[i]);
            host.set_retained_channel_00a7f2f0(i);
        }
    }
    host.vfs_clear_aliases_00bdf940();
    if (!in.file_system_lua_918.empty()) {
        for (const auto& alias : host.read_file_aliases(in.file_system_lua_918)) {
            host.vfs_add_alias_00bdf590(alias.first, alias.second);
        }
    }
    host.refresh_bush_textures_00ad60d0();
}

bool read_post_process_004e8510(const ScenePropertyBagModel& model, int bag_index,
                                ScenePostProcessFields& out, ScenePostProcessHost& host,
                                std::string& error) {
    return guarded(error, [&] { read_post_process(model, bag_index, out, host); });
}

ScenePropertyBagModel scene_bag_from_block(const ScenePropertyBlock& block) {
    ScenePropertyBagModel model;
    append_block(model, 0, block);
    return model;
}

}  // namespace bsp::scene_weather_cc13
