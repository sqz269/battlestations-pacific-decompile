#include "bsp/scene_weather_cc13.hpp"

#include <cmath>
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
constexpr double kPi = 3.14159265358979311600;  // 00CE3D28 (double)
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
        rgb[i] = static_cast<float>(std::exp2(static_cast<double>(kLinearizeExponent) *
                                              std::log2(static_cast<double>(c))));
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

ScenePropertyBagModel scene_bag_from_block(const ScenePropertyBlock& block) {
    ScenePropertyBagModel model;
    append_block(model, 0, block);
    return model;
}

}  // namespace bsp::scene_weather_cc13
