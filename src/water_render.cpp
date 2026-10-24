// Ocean surface rendering helpers (packet cc13_water1). Addresses: 0078C9B0
// (optics setter calls), 00B99E60..00B9A2C0, 00B9A3F0, 00B9A450, 00B9A4C0,
// 00BA0090 fragments, 00B999F0, 00B9A8A0, 00B99BF0, 00B99C80, 00B9CA00,
// 00BA5610/00BA6BD0 grid sizes, 00B9E3F0. Semantic C++, not binary
// replacements; evidence and ABI notes in include/bsp/water_render.hpp and
// docs/CC13_WATER_RENDER.md.

#include "bsp/water_render.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <array>
#include <cmath>
#include <cstring>

#include "bsp/camera_affine.hpp"
#include "bsp/camera_frame_state.hpp"
#include "bsp/camera_inverse.hpp"
#include "bsp/camera_look_at.hpp"
#include "bsp/camera_multiply.hpp"
#include "bsp/main_menu_map_point_geometry.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/native_vector2_math.hpp"

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
    // cc13-fixture4: main-thread precision (24-bit control, extended exponent), each x87
    // operation rounded once and each FSTP float a cast. The native spills n.a (00B99A6A),
    // n.a - d (00B99A7C), the differences b - a, n.b (00B99AD4), n.b - d, the ratio
    // (00B99AEC) and the three t (b - a) products to float slots; sa - sb stays on the stack.
    using cc13x87::x87_add24; using cc13x87::x87_sub24; using cc13x87::x87_mul24; using cc13x87::x87_div24;
    const auto fstp = [](double v) { return static_cast<float>(v); };
    const float da = fstp(x87_add24(x87_add24(x87_mul24(a.x, plane.x), x87_mul24(a.y, plane.y)),
                                 x87_mul24(a.z, plane.z)));
    const float sa = fstp(x87_sub24(da, plane.w));
    const float dx = fstp(x87_sub24(b.x, a.x)), dy = fstp(x87_sub24(b.y, a.y)), dz = fstp(x87_sub24(b.z, a.z));
    const float db = fstp(x87_add24(x87_add24(x87_mul24(b.y, plane.y), x87_mul24(b.x, plane.x)),
                                 x87_mul24(b.z, plane.z)));
    const float sb = fstp(x87_sub24(db, plane.w));
    const float t = fstp(x87_div24(sa, x87_sub24(sa, sb)));
    const float mx = fstp(x87_mul24(dx, t)), my = fstp(x87_mul24(dy, t)), mz = fstp(x87_mul24(t, dz));
    return {fstp(x87_add24(a.x, mx)), fstp(x87_add24(a.y, my)), fstp(x87_add24(a.z, mz))};
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

namespace {

// The plane dot product the native keeps for up = (0, 1, 0): x*0 + y, then
// + z*0, in extended precision (B9B4B3, B9B77C, B9B93E, B9BA2A, B9BE49).
double up_dot(const Float3& v) {
    return (static_cast<double>(v.x) * 0.0 + v.y) + static_cast<double>(v.z) * 0.0;
}

// Projects p onto y = 0 the way B9BE49..B9C046 does: d and d*0 are spilled.
Float3 flatten_to_water_plane(const Float3& p) {
    const float d = static_cast<float>(up_dot(p));
    const float d0 = d * 0.0f;
    return {p.x - d0, p.y - d, p.z - d0};
}

}  // namespace

bool compute_projector_matrices_00b9ae00(ProjectorCamera& camera, float displacement_height_b8,
                                         const CameraAxesCrtAccess& crt, ProjectorMatrices& out) {
    // B9AE0E..B9AEA0: save, widen the fov, move the camera to (0, y, 0).
    camera.refresh_world_if_dirty();
    const float saved_fov = camera.field_of_view_1c4();
    const Float3 saved_position = camera.position_120();
    camera.set_field_of_view_00b6fbb0(
        static_cast<float>(saved_fov * static_cast<double>(1.2f)));  // [00CEC160]
    camera.set_position_vtable_30({0.0f, saved_position.y, 0.0f});

    // B9AEA2..B9AF7C: the two displacement planes and the water plane.
    const float h = displacement_height_b8;
    const Float4 upper_plane{0.0f, 1.0f, 0.0f, h};
    const Float4 lower_plane{0.0f, 1.0f, 0.0f, -0.0f - h};  // SUBSS from [00D7A208]
    const Float4 water_plane{0.0f, 1.0f, 0.0f, 0.0f};

    // B9B00F..B9B148: near clamp, perspective, general inverse.
    float near_plane = camera.near_plane_1d4();
    if (1.5 > static_cast<double>(near_plane)) {  // [00CE3D78], JBE keeps NaN
        near_plane = 1.5f;                         // [00CE380C]
    }
    CameraMatrix projection{};
    build_projection_00b642f0(projection, camera.field_of_view_1c4(), camera.aspect_1c8(), near_plane,
                              camera.far_plane_1d8());
    CameraMatrix inverse_projection{};
    invert_camera_general_00b632d0(inverse_projection, projection);

    // B9B157..B9B213: near clip corners back to view space.
    static constexpr float kNearCorners[4][2] = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, 1.0f}};
    Float3 frustum[8];
    for (int i = 0; i < 4; ++i) {
        frustum[i] = transform_homogeneous_divide_00b99bf0({kNearCorners[i][0], kNearCorners[i][1], 0.0f},
                                                           inverse_projection.data());
    }
    // B9B219..B9B37E: far corners = near corners * (far / near), ratio spilled.
    const float ratio = static_cast<float>(static_cast<double>(camera.far_plane_1d8()) / near_plane);
    for (int i = 0; i < 4; ++i) {
        frustum[4 + i] = {ratio * frustum[i].x, ratio * frustum[i].y, ratio * frustum[i].z};
    }
    // B9B382..B9B3BA: world space.
    for (Float3& corner : frustum) {
        camera.refresh_world_if_dirty();
        std::array<float, 3> world{};
        transform_point_004142e0({corner.x, corner.y, corner.z}, camera.world_f0(), world);
        corner = {world[0], world[1], world[2]};
    }

    // B9B3D0..B9B48C: edge table at [ESP+210h]; upper plane first, then lower.
    static constexpr int kEdges[12][2] = {{0, 1}, {0, 2}, {2, 3}, {1, 3}, {0, 4}, {2, 6},
                                          {3, 7}, {1, 5}, {4, 6}, {4, 5}, {5, 7}, {6, 7}};
    constexpr int kMaxPoints = 32;
    Float3 points[kMaxPoints];
    int count = 0;
    for (const auto& edge : kEdges) {
        Float3 hit;
        if (plane_crossing_00b9a8a0(upper_plane, frustum[edge[0]], frustum[edge[1]], hit) &&
            count < kMaxPoints) {
            points[count++] = hit;
        }
        if (plane_crossing_00b9a8a0(lower_plane, frustum[edge[0]], frustum[edge[1]], hit) &&
            count < kMaxPoints) {
            points[count++] = hit;
        }
    }
    // B9B492..B9B59F: corners inside the slab. FDIVRP gives (h + y) / (y - h).
    for (const Float3& corner : frustum) {
        const double y = up_dot(corner);
        const double quotient = (static_cast<double>(h) + y) / (y + static_cast<double>(lower_plane.w));
        if (0.0 > quotient && count < kMaxPoints) {
            points[count++] = corner;
        }
    }

    // B9B5A5..B9B6DE: the projector projection, built again after a refresh.
    camera.refresh_world_if_dirty();
    CameraMatrix projector_projection{};
    build_projection_00b642f0(projector_projection, camera.field_of_view_1c4(), camera.aspect_1c8(),
                              near_plane, camera.far_plane_1d8());

    // B9B6E7..B9B774: two position copies and the forward row, each after a refresh.
    camera.refresh_world_if_dirty();
    Float3 eye = camera.position_120();
    camera.refresh_world_if_dirty();
    const Float3 anchor = camera.position_120();
    camera.refresh_world_if_dirty();
    Float3 forward = camera.forward_110();

    // B9B77C..B9B93A: nudge a near-horizontal forward vector and renormalise.
    float forward_up = static_cast<float>(up_dot(forward));
    if (!(forward_up > 0.0f)) {
        forward_up = -0.0f - forward_up;  // SUBSS from [00D7A208]
    }
    const float cos_limit = static_cast<float>(std::cos(static_cast<double>(1.55334306f)));  // FCOS [00CEC400]
    if (cos_limit >= forward_up) {  // FCOMIP / JB 00B9B7E0: unordered (NaN) skips the nudge
        const double nudge = static_cast<double>(0.01f);  // [00D7A358]
        Float3 t;
        t.x = forward.x + 0.0f;
        t.y = static_cast<float>(forward.y < 0.0f ? forward.y - nudge : forward.y + nudge);
        t.z = 0.0f + forward.z;
        const float yy = t.y * t.y;
        const float xx = t.x * t.x;
        const float zz = t.z * t.z;
        const float length_squared = static_cast<float>((static_cast<double>(yy) + xx) + zz);
        const float length = static_cast<float>(std::sqrt(length_squared));  // 00BF7030
        float inverse = 0.0f;
        if (length > 0.0f) {
            inverse = static_cast<float>(1.0 / length);
        }
        forward = {t.x * inverse, inverse * t.y, inverse * t.z};
    }

    // B9B93E..B9BA28: keep the projector at least h + 7 above the water.
    const float eye_up = static_cast<float>(up_dot(eye));
    const bool eye_below = 0.0f > eye_up;
    const double lift_limit = static_cast<double>(h) + 7.0;  // [00CED5D8]
    if (lift_limit > eye_up) {
        const float lift = static_cast<float>(eye_below ? lift_limit - eye_up * 2.0  // [00D7A308]
                                                        : lift_limit - eye_up);
        const float lift0 = lift * 0.0f;
        eye = {lift0 + eye.x, eye.y + lift, eye.z + lift0};
    }

    // B9BA2A..B9BB65: the forward ray from the camera, reflected in y = 0 when
    // camera and forward lie on the same side, cut by the water plane.
    // B9BA48 / B9BA74: FCOMI then JB to "flag = 0", so an unordered (NaN) dot gives 0: the
    // flags are the ordered d <= 0, not !(0 < d).
    const bool anchor_not_above = static_cast<float>(up_dot(anchor)) <= 0.0f;
    float forward_level = static_cast<float>(
        (static_cast<double>(forward.y) + static_cast<double>(forward.x) * 0.0) +
        static_cast<double>(forward.z) * 0.0);
    const bool forward_not_up = forward_level <= 0.0f;
    Float3 ray_end;
    if (forward_not_up == anchor_not_above) {
        const float d = static_cast<float>(static_cast<double>(forward.z) * 0.0 +
                                           (static_cast<double>(forward.x) * 0.0 + forward.y));
        const float twice_up_x = 0.0f;  // (0, 1, 0) * 2.0 spilled at B9BAB7
        const float twice_up_y = 2.0f;
        const Float3 reflected{forward.x - twice_up_x * d, forward.y - twice_up_y * d,
                               forward.z - d * twice_up_x};
        ray_end = {anchor.x + reflected.x, reflected.y + anchor.y, reflected.z + anchor.z};
    } else {
        ray_end = {anchor.x + forward.x, anchor.y + forward.y, forward.z + anchor.z};
    }
    ray_end = {ray_end.x + 0.0f, ray_end.y + 0.0f, ray_end.z + 0.0f};
    const Float3 water_hit = plane_intersect_segment_00b999f0(water_plane, anchor, ray_end);

    // B9BB8D..B9BD3D: blend with the flattened point 10 units ahead.
    if (!(forward_level > 0.0f)) {
        forward_level = -0.0f - forward_level;
    }
    Float3 ahead{static_cast<float>(forward.x * 10.0), static_cast<float>(forward.y * 10.0),
                 static_cast<float>(10.0 * forward.z)};  // [00CE3DC0]
    ahead = {ahead.x + anchor.x, ahead.y + anchor.y, ahead.z + anchor.z};
    ahead = flatten_to_water_plane(ahead);
    const float rest = static_cast<float>(1.0 - forward_level);
    ahead = {ahead.x * rest, rest * ahead.y, rest * ahead.z};
    Float3 aim{water_hit.x * forward_level, forward_level * water_hit.y, forward_level * water_hit.z};
    aim = {aim.x + ahead.x, aim.y + ahead.y, aim.z + ahead.z};
    aim = {aim.x - eye.x, aim.y - eye.y, aim.z - eye.z};  // aim - eye + eye, as stored
    aim = {aim.x + eye.x, eye.y + aim.y, eye.z + aim.z};

    // B9BD41: projector view.
    CameraMatrix view{};
    build_camera_look_at_00b63f10(view, {eye.x, eye.y, eye.z}, {aim.x, aim.y, aim.z}, {0.0f, 1.0f, 0.0f},
                                  crt);

    // B9BE20..B9C10D: flatten the points to y = 0.
    for (int i = 0; i < count; ++i) {
        points[i] = flatten_to_water_plane(points[i]);
    }
    bool ok = false;
    if (count != 0) {  // B9C117
        // B9C130..B9C23D: view, divide, projection, divide (both stored back).
        for (int i = 0; i < count; ++i) {
            points[i] = transform_homogeneous_divide_00b99bf0(points[i], view.data());
            points[i] = transform_homogeneous_divide_00b99bf0(points[i], projector_projection.data());
        }
        // B9C24B..B9C4CE: strict min/max; NaN updates nothing.
        float max_x = points[0].x, min_x = points[0].x;
        float max_y = points[0].y, min_y = points[0].y;
        for (int i = 1; i < count; ++i) {
            const float x = points[i].x;
            if (x > max_x) max_x = x;
            if (min_x > x) min_x = x;
            const float y = points[i].y;
            if (y > max_y) max_y = y;
            if (min_y > y) min_y = y;
        }
        // B9C4D0..B9C585: range matrix.
        CameraMatrix range{};
        range[0] = max_x - min_x;
        range[5] = max_y - min_y;
        range[10] = 1.0f;
        range[12] = min_x;
        range[13] = min_y;
        range[15] = 1.0f;
        // B9C58E..B9C6F3: the outputs.
        CameraMatrix view_projection{};
        multiply_camera_matrices_00413920(view_projection, view, projector_projection);
        CameraMatrix inverse_view_projection{};
        invert_camera_general_00b632d0(inverse_view_projection, view_projection);
        multiply_camera_matrices_00413920(out.range_174, range, inverse_view_projection);
        invert_camera_general_00b632d0(out.range_inverse_1b4, out.range_174);
        CameraMatrix inverse_range{};
        invert_camera_affine_00b63b30(inverse_range, range);
        multiply_camera_matrices_00413920(view_projection, view, projector_projection);
        multiply_camera_matrices_00413920(out.view_projection_range_1f4, view_projection, inverse_range);
        out.view_418 = view;
        out.projection_458 = projector_projection;
        ok = true;
    }
    // B9C9A5..B9C9BF and B9C9D3..B9C9EB: restore.
    camera.set_position_vtable_30(saved_position);
    camera.set_field_of_view_00b6fbb0(saved_fov);
    return ok;
}

OceanGridCell average_grid_quad_00b9a990(const OceanGridCell& p0, const OceanGridCell& p1,
                                         const OceanGridCell& p2, const OceanGridCell& p3, float scale,
                                         const CameraAxesCrtAccess& crt) {
    OceanGridCell out;
    // B9A997..B9AA55: centre, every partial sum spilled, * double 0.25.
    const float sx = p3.x + (p2.x + (p0.x + p1.x));
    const float sy = p3.height + (p2.height + (p0.height + p1.height));
    const float sz = p3.z + (p2.z + (p0.z + p1.z));
    out.x = static_cast<float>(sx * 0.25);  // [00D7A348]
    out.height = static_cast<float>(sy * 0.25);
    out.z = static_cast<float>(0.25 * sz);
    // B9AA58..B9AAB8: normal sum, p1 first.
    std::array<float, 3> normal_sum{};
    for (int k = 0; k < 3; ++k) {
        normal_sum[k] = p3.normal[k] + (p2.normal[k] + (p1.normal[k] + p0.normal[k]));
    }
    // B9AABC..B9AACA: lerp towards (0, 4, 0) by scale ([00CE3D34]).
    const std::array<float, 3> blended = main_menu_map_lerp_005803e0(normal_sum, {0.0f, 4.0f, 0.0f}, scale);
    // B9AAD3..B9AB2F: normalise; zero length gives zero.
    const float length = camera_vector_length_00419440(blended.data(), &crt);
    float inverse = 0.0f;
    if (length > 0.0f) {
        inverse = static_cast<float>(1.0 / length);
    }
    out.normal[0] = blended[0] * inverse;
    out.normal[1] = blended[1] * inverse;
    out.normal[2] = inverse * blended[2];
    return out;
}

namespace {

float abs_by_negation(float v) {
    return v > 0.0f ? v : -0.0f - v;  // COMISS + SUBSS from [00D7A208]
}

}  // namespace

void refresh_grid_tiles_00b9d7b0(OceanGridTilesField& field, const float stack_garbage[3],
                                 const CameraAxesCrtAccess& crt, void (*invalid_parameter)(),
                                 void (*upload)(void* context), void* upload_context) {
    if (field.level_count != 0 && field.levels != nullptr) {
        OceanGridLevel& base = field.levels[0];
        for (std::uint32_t y = 0; y < base.height; ++y) {
            for (std::uint32_t x = 0; x < base.width; ++x) {
                const std::uint32_t i = (x & 0x3fu) + (y & 0x3fu) * 0x40u;
                bool in_range = true;
                if (field.choppy_30 == nullptr || i >= field.choppy_count) {
                    invalid_parameter();
                    in_range = false;
                }
                if (field.heights_20 == nullptr || i >= field.heights_count) {
                    invalid_parameter();
                    in_range = false;
                }
                if (!in_range) {
                    continue;  // native reads the element anyway
                }
                const float choppy_x = field.choppy_30[i * 2];
                const float choppy_z = field.choppy_30[i * 2 + 1];
                const float height = -0.0f - field.heights_20[i * 2];
                // B9D8A9..B9D8E4: +104h, the scaled height maximum.
                const float scaled = static_cast<float>(
                    static_cast<double>(field.height_scale_b8) * abs_by_negation(height));
                const float old_104 = field.extents_104[0];
                field.extents_104[0] = old_104 > scaled ? old_104 : scaled;
                // B9D8E1..B9D998: +108h, +10Ch, +110h.
                const float values[3] = {abs_by_negation(choppy_x), abs_by_negation(height),
                                         abs_by_negation(choppy_z)};
                for (int k = 0; k < 3; ++k) {
                    const float old = field.extents_104[1 + k];
                    field.extents_104[1 + k] = values[k] > old ? values[k] : old;
                }
                // B9D9A0..B9D9EB.
                OceanGridCell& cell = base.cells[base.width * y + x];
                cell.height = height;
                cell.x = choppy_x;
                cell.z = choppy_z;
                cell.normal[0] = stack_garbage[0];
                cell.normal[1] = stack_garbage[1];
                cell.normal[2] = stack_garbage[2];
            }
        }
    }
    // B9DA0D..B9DB2F: the pyramid.
    static constexpr float kScales[5] = {0.0f, 0.2f, 0.5f, 0.9f, 1.0f};
    for (std::uint32_t level = 0; level + 1 < field.level_count && level < 5; ++level) {
        const OceanGridLevel& fine = field.levels[level];
        OceanGridLevel& coarse = field.levels[level + 1];
        for (std::uint32_t y = 0; y < coarse.height; ++y) {
            for (std::uint32_t x = 0; x < coarse.width; ++x) {
                const OceanGridCell* top = fine.cells + (fine.width * y + x) * 2;
                const OceanGridCell* bottom = fine.cells + (fine.width * (2 * y + 1) + x * 2);
                coarse.cells[coarse.width * y + x] =
                    average_grid_quad_00b9a990(top[0], top[1], bottom[0], bottom[1], kScales[level], crt);
            }
        }
    }
    // B9DB35: JMP [vtable+0Ch] (00BA5030).
    upload(upload_context);
}

namespace {

// BA3270..BA3299 and the two other draw sites: FILD, +2^32 when negative
// ([00CE3978]), * 2^-32 ([00D63B80]), spilled to a float.
float unit_draw(OceanBeamHost& host) {
    const std::uint32_t bits = host.next_private_random_00ba2c20();
    double value = static_cast<double>(static_cast<std::int32_t>(bits));
    if (static_cast<std::int32_t>(bits) < 0) {
        value += 4294967296.0;
    }
    return static_cast<float>(value * 2.3283064365386963e-10);
}

}  // namespace

void update_ocean_beams_00ba2ff0(OceanBeamSystem& beams, OceanBeamHost& host, float time,
                                 const CameraAxesCrtAccess& crt) {
    if (beams.enabled_2c == 0) {
        return;
    }
    // BA3003..BA3062.
    host.refresh_camera_world_if_dirty();
    const Float3 camera = host.camera_position_120();
    host.camera_trivial_00b6fc60();
    if (!(0.0f > camera.y)) {  // COMISS / JBE: camera not under water
        host.set_visibility_factor_00b6da70(0.0f);
        return;
    }
    host.set_visibility_factor_00b6da70(1.0f);

    // BA3067..BA30BA: clock and frame delta.
    beams.previous_clock_ec = beams.clock_f0;
    const float clock = static_cast<float>(static_cast<double>(beams.speed_30) * time);
    beams.clock_f0 = clock;
    if (0.0f > beams.previous_clock_ec) {
        beams.previous_clock_ec = clock;
    }
    const float delta = static_cast<float>(static_cast<double>(clock) - beams.previous_clock_ec);

    // BA30A5..BA31F8: where the light ray through the camera meets y = 0.
    Float3 raw;
    if (!host.first_light_vector(raw)) {
        host.invalid_parameter_00bf6713();
    }
    float light[3];
    const float raw_v[3] = {raw.x, raw.y, raw.z};
    camera_vector_normalize_00419510(light, raw_v, &crt);
    const float ny = -light[1];
    const float nx = -0.0f - light[0];
    const float nz = -0.0f - light[2];
    const float horizontal_sq = static_cast<float>(1.0 - static_cast<double>(ny) * ny);
    const float s = static_cast<float>(std::sqrt(horizontal_sq));  // 00BF7030
    float flat[2] = {nx, nz};
    const float reciprocal = native_vector2_reciprocal_length_00419260(flat, &crt);
    flat[0] = reciprocal * nx;
    flat[1] = reciprocal * nz;
    const float hx = flat[0] * s;
    const float hz = s * flat[1];
    const float vertical_sq = static_cast<float>(1.0 - horizontal_sq);
    const float ly = -static_cast<float>(std::sqrt(vertical_sq));
    const float k = static_cast<float>(static_cast<double>(camera.y) / ly);
    const Float3 origin{static_cast<float>(camera.x - static_cast<double>(k) * hx),
                        static_cast<float>(camera.y - static_cast<double>(ly) * k),
                        static_cast<float>(camera.z - static_cast<double>(k) * hz)};

    // BA3202..BA3363: respawn and death scheduling.
    for (std::uint32_t i = 0; i < beams.count_dc; ++i) {
        OceanBeam& beam = *beams.beams_e8[i];
        bool respawn = false;
        // COMISS / JC at BA3216 and BA3226: below zero or unordered takes the arm.
        if (!(beam.birth_0c >= 0.0f)) {
            respawn = true;
        } else if (!(beam.death_10 >= 0.0f)) {
            // BA33B1..BA34B4.
            const float dx = beam.position[0] - origin.x;
            const float dz = beam.position[2] - origin.z;
            const float d2 = static_cast<float>(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
            float rate;
            if (beams.death_band_60[0] > d2) {
                rate = beams.death_rate_6c[0];
            } else if (beams.death_band_60[1] > d2) {
                rate = beams.death_rate_6c[1];
            } else if (beams.death_band_60[2] > d2) {
                rate = beams.death_rate_6c[2];
            } else {
                rate = beams.death_rate_6c[3];
            }
            float survival = static_cast<float>(1.0 - rate);
            survival = static_cast<float>(std::log(static_cast<double>(survival)));  // 00BFA910
            survival = survival * delta;
            survival = static_cast<float>(std::exp(static_cast<double>(survival)));  // 00BFEEEC
            const float u3 = unit_draw(host);  // BA346E
            if (u3 > survival) {
                beam.death_10 = beams.fade_out_time_d0 + beams.clock_f0;
            }
        } else if (beams.clock_f0 > beam.death_10) {
            respawn = true;
        }
        if (respawn) {
            // BA3240..BA3357.
            beam.birth_0c = beams.clock_f0;
            beam.death_10 = -1.0f;  // [00D7A260]
            const float u1 = unit_draw(host);  // BA3270
            const float radius = static_cast<float>(static_cast<double>(beams.radius_range_54) * u1 +
                                                    beams.radius_base_50);
            const float u2 = unit_draw(host);  // BA32BF
            const float angle = static_cast<float>(u2 * 6.2831854820251465);  // [00CE3828]
            const float c = static_cast<float>(std::cos(static_cast<double>(angle)));
            const float sn = static_cast<float>(std::sin(static_cast<double>(angle)));
            const float ox = c * radius;
            const float oz = radius * sn;
            beam.position[0] = ox + origin.x;
            beam.position[1] = static_cast<float>(origin.y + 0.0);  // [00D7A258]
            beam.position[2] = oz + origin.z;
        }
    }

    // BA3369..BA33A4.
    host.set_draw_counts(beams.count_dc * 4, beams.count_dc * 2);
    if (beams.count_dc == 0) {
        host.set_visibility_factor_00b6da70(0.0f);
        return;
    }
    // BA34B9..BA3A4A: vertices.
    host.refresh_camera_world_if_dirty();
    OceanBeamVertex* vertex = host.lock_vertices();
    for (std::uint32_t i = 0; i < beams.count_dc; ++i) {
        const OceanBeam& beam = *beams.beams_e8[i];
        const float bx = beam.position[0];
        const float bz = beam.position[2];
        const float tx = origin.x - bx;
        const float tz = origin.z - bz;
        const float distance_sq = static_cast<float>(static_cast<double>(tx) * tx + static_cast<double>(tz) * tz);
        const float distance = static_cast<float>(std::sqrt(distance_sq));
        float px = 1.0f;
        float pz = 0.0f;
        if (distance > static_cast<double>(0.05f)) {  // [00D7A270]
            const float inverse = static_cast<float>(1.0 / distance);
            px = inverse * tz;
            pz = inverse * -tx;
        }
        const double w = beams.half_width_58;
        const float right_x = static_cast<float>(bx + w * px);
        const float right_z = static_cast<float>(bz + w * pz);
        const float left_x = static_cast<float>(bx - w * px);
        const float left_z = static_cast<float>(bz - w * pz);
        const float right_y = host.water_height_0078c890(right_x, right_z);
        const float left_y = host.water_height_0078c890(left_x, left_z);
        float displacement[2];
        host.horizontal_displacement_00ba2b50(bx, bz, displacement);
        // BA36D0..BA3766: normal of (dx, 64, dz); 4096 is [00D63B90].
        const float dxx = displacement[0] * displacement[0];
        const float dzz = displacement[1] * displacement[1];
        const float normal_len = static_cast<float>(
            std::sqrt(static_cast<float>((static_cast<double>(dxx) + 4096.0) + dzz)));
        float normal_inverse = 0.0f;
        if (normal_len > 0.0f) {
            normal_inverse = static_cast<float>(1.0 / normal_len);
        }
        const float nnx = normal_inverse * displacement[0];
        const float nny = static_cast<float>(64.0 * normal_inverse);  // [00D638F0]
        const float nnz = normal_inverse * displacement[1];
        // BA376D..BA37CA: fades.
        // Both differences stay in extended precision (not spilled).
        float alpha = 1.0f;
        const double age = static_cast<double>(beams.clock_f0) - beam.birth_0c;
        if (beams.fade_in_time_cc > age) {
            alpha = static_cast<float>(age * beams.fade_in_rate_d4);
        }
        const float death = beam.death_10;
        if (death >= 0.0f) {  // [00D7A218] = 0.0, COMISS / JC at BA37AA skips below zero or unordered
            const double left_time = static_cast<double>(death) - beams.clock_f0;
            if (beams.fade_out_time_d0 > left_time) {
                alpha = static_cast<float>(left_time * beams.fade_out_rate_d8 * alpha);
            }
        }
        // BA37CA..BA38D5: the beam axis.
        const float sx = nnx * beams.axis_scale_5c;
        const float sy = nny * beams.axis_scale_5c;
        const float sz = beams.axis_scale_5c * nnz;
        const float ax = sx + hx;
        const float ay = sy + ly;
        const float az = sz + hz;
        const float axx = ax * ax;
        const float ayy = ay * ay;
        const float azz = az * az;
        const float axis_len = static_cast<float>(
            std::sqrt(static_cast<float>((static_cast<double>(axx) + ayy) + azz)));
        float axis_inverse = 0.0f;
        if (axis_len > 0.0f) {
            axis_inverse = static_cast<float>(1.0 / axis_len);
        }
        const float axis[3] = {ax * axis_inverse, ay * axis_inverse, axis_inverse * az};
        // BA38E3..BA3A16: four vertices.
        for (int corner = 0; corner < 4; ++corner) {
            float pos[3];
            float u;
            if (corner == 1 || corner == 3) {
                pos[0] = right_x; pos[1] = right_y; pos[2] = right_z;
                u = 0.0f;
            } else {
                pos[0] = left_x; pos[1] = left_y; pos[2] = left_z;
                u = 1.0f;
            }
            float v = 0.0f;
            if (corner >= 2) {
                const float depth = beams.length_4c;
                pos[0] = axis[0] * depth + pos[0];
                pos[1] = axis[1] * depth + pos[1];
                pos[2] = depth * axis[2] + pos[2];
                v = 1.0f;
            }
            const std::uint32_t argb = host.color_argb_004fb850(beams.color_3c);
            vertex->x = pos[0];
            vertex->y = pos[1];
            vertex->z = pos[2];
            vertex->argb = argb;
            vertex->u = u;
            vertex->v = v;
            vertex->alpha = alpha;
            ++vertex;
        }
    }
    host.unlock_vertices();
}

GridResolution initial_grid_resolution_00ba5610(bool simple_ocean_f9) {
    return simple_ocean_f9 ? GridResolution{0x20, 0x20} : GridResolution{0x20, 0x40};
}

GridResolution quality_grid_resolution_00ba6bd0(bool simple_ocean_f9) {
    return simple_ocean_f9 ? GridResolution{0x20, 0x20} : GridResolution{0x80, 0x100};
}

WaterRingRecord make_water_ring_00b9e3f0(const Float3& position, float a, float b,
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
