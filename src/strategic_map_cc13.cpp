#include "bsp/strategic_map_cc13.hpp"

#include <cmath>

namespace bsp {

void init_map_scene_zoom_0093ccc0(MapSceneZoomState& scene)
{
    scene.shadow_fade_18 = 1.0f; // 00D7A24C
    scene.field_20 = 1.0f;
    scene.smooth_zoom_level_14 = static_cast<float>(static_cast<double>(scene.level_24) + 1.0);
    // The native indexes 00E0CF40 without a bound check.
    const float extent = kMapZoomExtents[static_cast<std::size_t>(scene.level_24)];
    scene.extent_28 = extent;
    scene.extent_scaled_2c = static_cast<float>(static_cast<double>(extent) * kMapExtentAspect);
}

std::array<MapQuadVertex, 4> build_map_quad_vertices_0093de10(float width, float height)
{
    // Halves are exact; 00D7A280 = 0.5.
    const float hw = width * 0.5f;
    const float hh = height * 0.5f;
    return {{
        {-hw, 0.0f, hh, 0.0f, 0.0f},
        {hw, 0.0f, hh, 1.0f, 0.0f},
        {-hw, 0.0f, -hh, 0.0f, 1.0f},
        {hw, 0.0f, -hh, 1.0f, 1.0f},
    }};
}

std::vector<MapShaderBinding> map_quad_shader_bindings_0093de10(std::int32_t mode)
{
    switch (mode) {
    case 0x0F: return {{"cBaseSize", MapShaderTarget::SceneBaseSize30}};               // 00D19820
    case 0x00: return {{"cSmoothMapZoomLevel", MapShaderTarget::SceneSmoothZoom14}};   // 00D19770
    case 0x0D: return {{"cZoomLerp", MapShaderTarget::SceneSmoothZoom14},              // 00D19814
                       {"cAspectRatio", MapShaderTarget::AspectRatioGlobal}};          // 00D19804
    case 0x0C: return {{"cShadowFade", MapShaderTarget::SceneShadowFade18},            // 00CF0100
                       {"cStaticShot_Size_OffsetX_OffsetY", MapShaderTarget::StaticShotGlobal}}; // 00D0E634
    default: return {};
    }
}

std::array<float, 3> map_quad_position_0093de10(const MapBounds& b, float y_offset)
{
    // FLD/FADD/FSTP dword per axis, then FMUL qword 0.5, then FADD (0 for x and z).
    const float sx = b.x0 + b.x1;
    const float sy = b.y1 + b.y0;
    const float sz = b.z1 + b.z0;
    return {
        static_cast<float>(static_cast<double>(sx) * 0.5 + 0.0),
        static_cast<float>(static_cast<double>(sy) * 0.5 + y_offset),
        static_cast<float>(static_cast<double>(sz) * 0.5 + 0.0),
    };
}

std::array<float, 2> map_overlay_world_to_screen(const MapOverlayTransform& t, float wx, float wy)
{
    const double x = static_cast<double>(t.screen_origin_x_18)
        + static_cast<double>(t.scale_x_00) * (static_cast<double>(wx) - t.world_origin_x_20);
    const double y = static_cast<double>(t.screen_origin_y_1c)
        - static_cast<double>(t.scale_y_04) * (static_cast<double>(wy) - t.world_origin_y_24);
    return {static_cast<float>(x), static_cast<float>(y)};
}

std::vector<MapOverlaySegment> map_overlay_ellipse_0093e9a0(float cx, float cy, float rx, float ry)
{
    std::vector<MapOverlaySegment> out;
    out.reserve(32);
    float prev_cos = 1.0f; // 00D7A24C
    float prev_sin = 0.0f;
    for (int i = 1; i < 0x21; ++i) {
        // FILD, FMUL qword pi, FMUL qword 0.0625, FSTP dword; FSIN/FCOS of that float,
        // each stored as a float.
        const float angle = static_cast<float>(static_cast<double>(i) * 3.1415927410125732 * 0.0625);
        const float s = static_cast<float>(std::sin(static_cast<double>(angle)));
        const float c = static_cast<float>(std::cos(static_cast<double>(angle)));
        out.push_back({
            static_cast<float>(static_cast<double>(rx) * prev_cos + cx),
            static_cast<float>(static_cast<double>(prev_sin) * ry + cy),
            static_cast<float>(static_cast<double>(cx) + static_cast<double>(rx) * c),
            static_cast<float>(static_cast<double>(cy) + static_cast<double>(ry) * s),
        });
        prev_sin = s;
        prev_cos = c;
    }
    return out;
}

} // namespace bsp
