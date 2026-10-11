// Ocean surface rendering helpers (packet cc13_water1). Addresses: 0078C9B0
// (optics setter calls), 00B99E60..00B9A2C0, 00B9A3F0, 00B9A450, 00B9A4C0,
// 00BA0090 fragments, 00B999F0, 00B9A8A0, 00B99BF0, 00B99C80, 00B9CA00,
// 00BA5610/00BA6BD0 grid sizes, 00B9E3F0. Semantic C++, not binary
// replacements; evidence and ABI notes in include/bsp/water_render.hpp and
// docs/CC13_WATER_RENDER.md.

#include "bsp/water_render.hpp"

#include <cmath>
#include <cstring>

#include "bsp/native_camera_plane_transform.hpp"

namespace bsp::water_render {
namespace {

float read_f32(const std::uint8_t* block, std::uint32_t offset) {
    float value;
    std::memcpy(&value, block + offset, sizeof value);
    return value;
}

Float4 read_quad(const std::uint8_t* block, std::uint32_t offset) {
    return {read_f32(block, offset), read_f32(block, offset + 4), read_f32(block, offset + 8),
            read_f32(block, offset + 12)};
}

Float4 transform4(const Float4& v, const float matrix[16]) {
    float source[4] = {v.x, v.y, v.z, v.w};
    float result[4];
    transform_native_vector4_00b62d10(source, result, matrix);
    return {result[0], result[1], result[2], result[3]};
}

}  // namespace

void apply_authored_optics_0078c9b0(const std::uint8_t* block, OceanOpticsFields& fields) {
    fields.sky_reflection_350 = read_quad(block, block::kSkyReflection);              // 00B99E90
    fields.local_reflection_tint_36c = read_quad(block, block::kLocalReflectionTint); // 00B99EC0
    fields.spray_tint_3dc = read_quad(block, block::kSprayTint);                      // 00B99F00
    fields.reflection_tint_37c = read_quad(block, block::kReflectionTint);            // 00B99E60
    fields.refraction_tint_38c = read_quad(block, block::kRefractionTint);            // 00B99F60
    fields.underwater_refraction_39c = read_quad(block, block::kUWRefractionTint);    // 00B99F90
    fields.underwater_glow_360 = {read_f32(block, block::kUWGlow),                    // 00B99FC0
                                  read_f32(block, block::kUWGlow + 4),
                                  read_f32(block, block::kUWGlow + 8)};
    // 0078C9B0 calls 00B9A030/00B9A130/00B9A080 (system-parameter slots) and
    // 00B9DBB0 here; they are reconstructed elsewhere and are not optics fields.
    fields.refraction_specular_3fc = read_quad(block, block::kRefrSpecularColor);     // 00B9A180
    fields.underwater_refraction_index_414 = read_f32(block, block::kUnderWaterRefrIndex);  // 00B9A2C0
}

NormalMapLayers set_normal_map_layers_00b9a3f0(const float scales[4], const float speeds[4]) {
    NormalMapLayers layers;
    layers.inverse_scale_44 = {1.0f / scales[0], 1.0f / scales[1], 1.0f / scales[2], scales[3]};
    layers.scroll_speed_54 = {speeds[0], speeds[1], speeds[2], speeds[3]};
    return layers;
}

NormalMapOffsets normal_map_scroll_00ba0792(const NormalMapLayers& layers, float wind_cos_64,
                                            float wind_sin_68, float ocean_clock) {
    // Each product is formed on the x87 stack and rounded once at its FSTP.
    const double t = ocean_clock;
    const double c = wind_cos_64;
    const double s = wind_sin_68;
    const double k0 = static_cast<double>(layers.inverse_scale_44.x) * layers.scroll_speed_54.x;
    const double k1 = static_cast<double>(layers.inverse_scale_44.y) * layers.scroll_speed_54.y;
    const double k2 = static_cast<double>(layers.inverse_scale_44.z) * layers.scroll_speed_54.z;
    NormalMapOffsets offsets;
    offsets.layer0_6c = static_cast<float>(k0 * t * c);
    offsets.layer0_70 = static_cast<float>(k0 * t * s);
    offsets.layer1_74 = static_cast<float>(k1 * t * c);
    offsets.layer1_78 = static_cast<float>(k1 * t * s);
    offsets.layer2_7c = static_cast<float>(k2 * t * c);
    offsets.layer2_80 = static_cast<float>(k2 * t * s);
    return offsets;
}

float ocean_clock_00ba00ad(float update_argument) {
    // 00BA00A8 FLD arg; 00BA00AD FMUL double 10.0 (00CE3DC0); 00BA00BA FSTP [01090494].
    return static_cast<float>(10.0 * update_argument);
}

OceanLayerParams set_layer_params_00b9a4c0(float falloff, float layer_scale, float time_scale) {
    OceanLayerParams params;
    params.falloff_18 = falloff;
    params.layer_scale_1c = layer_scale;
    params.time_scale_20 = time_scale;
    params.tile_b0 = layer_scale;
    params.inverse_tile_b4 = 1.0f / layer_scale;  // 00B9A4D2 FDIVR from 1.0
    return params;
}

WindTrig wind_trig_00b9a450(float direction) {
    WindTrig trig;
    trig.direction_59c = direction;
    trig.cos_64 = static_cast<float>(std::cos(static_cast<double>(direction)));
    trig.sin_68 = static_cast<float>(std::sin(static_cast<double>(direction)));
    return trig;
}

OceanWaveScale ocean_wave_scale_00ba0b32(float height_scale_b8, float spectrum_choppy_88) {
    OceanWaveScale scale;
    scale.height_scale_2c = height_scale_b8;  // 00BA0B55 MOVSS [ESI+2Ch]
    const double product = static_cast<double>(height_scale_b8) * spectrum_choppy_88;
    scale.wave_scale_28 = static_cast<float>(product);  // 00BA0B5F FSTP [ESI+28h]
    scale.wave_scale_30 = static_cast<float>(product);  // 00BA0B6D FSTP [ESI+30h]
    scale.amplitude_24 = scale.height_scale_2c;         // 00BA0B90 FLD [ESI+2Ch], 00BA0B93 FSTP [ESI+24h]
    return scale;
}

Float3 plane_intersect_segment_00b999f0(const Float4& plane, const Float3& a, const Float3& b) {
    // The native spills n.a, (n.a - d), n.b and the ratio to float slots.
    const float da = a.x * plane.x + a.y * plane.y + a.z * plane.z;
    const float sa = da - plane.w;
    const float db = b.x * plane.x + b.y * plane.y + b.z * plane.z;
    const float sb = db - plane.w;
    const float t = sa / (sa - sb);
    return {a.x + t * (b.x - a.x), a.y + t * (b.y - a.y), a.z + t * (b.z - a.z)};
}

bool plane_crossing_00b9a8a0(const Float4& plane, const Float3& a, const Float3& b, Float3& out) {
    const float level = plane.w;
    const bool down = level < a.y && b.y < level;
    const bool up = a.y < level && level < b.y;
    if (!down && !up) {
        return false;
    }
    float dy = a.y - b.y;
    if (dy <= 0.0f) {
        dy = -0.0f - dy;  // SUBSS from the -0.0f at 00D7A208
    }
    if (!(1.1f < dy)) {  // [00CE6448]
        return false;
    }
    out = plane_intersect_segment_00b999f0(plane, a, b);
    return true;
}

Float3 transform_homogeneous_divide_00b99bf0(const Float3& point, const float matrix[16]) {
    const Float4 h = transform4({point.x, point.y, point.z, 1.0f}, matrix);
    const double inverse_w = 1.0 / h.w;
    return {static_cast<float>(h.x * inverse_w), static_cast<float>(h.y * inverse_w),
            static_cast<float>(h.z * inverse_w)};
}

Float4 clip_ray_water_plane_00b99c80(float u, float v, const float matrix[16]) {
    const Float4 a = transform4({u, v, -1.0f, 1.0f}, matrix);
    const Float4 b = transform4({u, v, 1.0f, 1.0f}, matrix);
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    const float dz = b.z - a.z;
    const float dw = b.w - a.w;
    const float t = -a.y / dy;
    return {t * dx + a.x, t * dy + a.y, a.z + dz * t, a.w + t * dw};
}

GridCorners compute_grid_corners_00b9ca00(const float projector_matrix_174[16]) {
    GridCorners corners;
    corners.corner[0] = clip_ray_water_plane_00b99c80(0.0f, 0.0f, projector_matrix_174);
    corners.corner[1] = clip_ray_water_plane_00b99c80(1.0f, 0.0f, projector_matrix_174);
    corners.corner[2] = clip_ray_water_plane_00b99c80(0.0f, 1.0f, projector_matrix_174);
    corners.corner[3] = clip_ray_water_plane_00b99c80(1.0f, 1.0f, projector_matrix_174);
    for (Float4& corner : corners.corner) {
        corner.y = 0.0f;
    }
    return corners;
}

GridResolution initial_grid_resolution_00ba5610(bool simple_ocean_f9) {
    return simple_ocean_f9 ? GridResolution{0x20, 0x20} : GridResolution{0x20, 0x40};
}

GridResolution quality_grid_resolution_00ba6bd0(bool simple_ocean_f9) {
    return simple_ocean_f9 ? GridResolution{0x20, 0x20} : GridResolution{0x80, 0x100};
}

WaterRingRecord make_water_ring_00b9e3f0(const Float3& position, float a, std::uint32_t b,
                                         float duration, float ocean_clock) {
    WaterRingRecord ring;
    ring.position_00 = position;
    ring.a_0c = a;
    ring.b_10 = b;
    ring.duration_14 = duration;
    ring.start_clock_18 = ocean_clock;
    const float rate = static_cast<float>((static_cast<double>(a) - 6.2831854820251465) / duration);
    ring.rate_1c = rate;
    ring.rate_20 = rate;
    return ring;
}

}  // namespace bsp::water_render
