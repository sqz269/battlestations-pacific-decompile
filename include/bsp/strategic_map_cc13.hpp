#pragma once

// Strategic-map presentation, segment 61 (cc13-warnings1): the "Simple3dScene" that the HUD
// map, formation and library screens own (vtable 00D19724), the map quad builder with the
// cSmoothMapZoomLevel shader constant, and the 2D map-overlay debug drawer.
// Reconstruct-only: no host calls these. docs/CC13_STRATEGIC_MAP.md.

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace bsp {

// 00E0CF40: map extent per zoom level (metres), read by 0093CCC0, BSP_HudMapScreen_Register
// 005AB190, BSP_HudFormationScreen_Register 00538F80 and the map-screen update 005B45C0.
inline constexpr std::array<float, 4> kMapZoomExtents{48000.0f, 18000.0f, 8000.0f, 3000.0f};
inline constexpr double kMapExtentAspect = 0.75;   // 00CEC9D8 (also float 00D19720 "cAspectRatio")

// The scene fields 0093CCC0 initialises (offsets from the scene; the map screen keeps its
// scene at screen+8h, 005AB1B9).
struct MapSceneZoomState {
    float smooth_zoom_level_14{0.0f}; // bound to "cSmoothMapZoomLevel" (0093D64A, 0093E2FB)
    float shadow_fade_18{0.0f};       // bound to "cShadowFade" (0093DC3E, 0093E366)
    float field_20{0.0f};
    std::int32_t level_24{0};         // 0 from the constructor 0093CB10
    float extent_28{0.0f};
    float extent_scaled_2c{0.0f};
    float base_size_30[2]{0.0f, 0.0f}; // "cBaseSize", mode-0Fh quads only (0093E0DB)
};

// 0093CCC0 (__fastcall ECX = scene), the zoom part at 0093CDDF..0093CE25:
// +18h = +20h = 1.0f; +14h = float(level) + 1.0 (FILD, FADD qword 00D7A210, FSTP dword);
// +28h = kMapZoomExtents[level]; +2Ch = +28h * 0.75 (FMUL qword, FSTP dword).
// The rest of 0093CCC0 builds the scene graph (Simple3dSceneHandler, SceneHandlerCamera,
// MapLights, TestDirectionalLight); not reconstructed.
void init_map_scene_zoom_0093ccc0(MapSceneZoomState& scene);

// 0093DE10: __thiscall(ECX = scene, mode, rgba[4], NativeString effect, y_offset, width,
// height, alpha_ref), RET 2Ch. Builds model "MeshQuad<mode>" with format pf43uf42.mvfm.
struct MapQuadVertex {
    float x, y, z, u, v;
};
// 0093DFEF..0093E0A7: one triangle strip of four vertices on the XZ plane.
std::array<MapQuadVertex, 4> build_map_quad_vertices_0093de10(float width, float height);
// 0093E132..0093E140: draw-section +8h, +10h, +18h.
struct MapQuadDrawSection {
    std::int32_t primitive_8{5};  // triangle strip
    std::int32_t vertices_10{4};
    std::int32_t primitives_18{2};
};

enum class MapShaderTarget {
    SceneSmoothZoom14,     // scene+14h
    SceneShadowFade18,     // scene+18h
    SceneBaseSize30,       // scene+30h (width, height stored first)
    AspectRatioGlobal,     // 00D19720 = 0.75f
    StaticShotGlobal,      // 00E1891C, float3 (BSS)
};
struct MapShaderBinding {
    std::string_view name;
    MapShaderTarget target;
};
// 0093E0BE..0093E457: the constants a quad registers, by mode. 0Fh "cBaseSize";
// 0 "cSmoothMapZoomLevel"; 0Dh "cZoomLerp" (scene+14h, the same field) and "cAspectRatio";
// 0Ch "cShadowFade" and "cStaticShot_Size_OffsetX_OffsetY"; 0Eh and others none.
std::vector<MapShaderBinding> map_quad_shader_bindings_0093de10(std::int32_t mode);

// 0093E1F7..0093E296: the quad node is placed at the centre of the map bounds
// (game+711Ch/7120h/7124h and +7128h/712Ch/7130h), raised by y_offset, through the
// node's vtable +30h. Each sum is stored as a float before the 0.5 multiply.
struct MapBounds {
    float x0, y0, z0;   // game+711Ch, +7120h, +7124h
    float x1, y1, z1;   // game+7128h, +712Ch, +7130h
};
std::array<float, 3> map_quad_position_0093de10(const MapBounds& bounds, float y_offset);

// ---------------------------------------------------------------------------
// 2D map-overlay debug drawer (0093E510..0093F4B0). Only 0093FF10 (the "Lin"/"Log" sound
// overlay, no reference) and 007BAC50 (no reference) reach it.
// ---------------------------------------------------------------------------
struct MapOverlayTransform {
    float scale_x_00{1.0f};
    float scale_y_04{1.0f};
    float screen_origin_x_18{0.0f};
    float screen_origin_y_1c{0.0f};
    float world_origin_x_20{0.0f};
    float world_origin_y_24{0.0f};
};
// 0093EAEA..0093EB2x: x = +18h + +0h * (wx - +20h); y = +1Ch - +4h * (wy - +24h).
std::array<float, 2> map_overlay_world_to_screen(const MapOverlayTransform&, float wx, float wy);

struct MapOverlaySegment {
    float x0, y0, x1, y1;
};
// 0093E9A0 (cdecl, center*, radius*): 32 chords, angle i * pi * 0.0625 for i = 1..32
// (00CE3D28, 00CEF290), each endpoint center + radius * (cos, sin), handed to the world
// line 0093E510. 0093EEB0 calls it with radius / (scale_x, scale_y).
std::vector<MapOverlaySegment> map_overlay_ellipse_0093e9a0(float cx, float cy, float rx, float ry);

} // namespace bsp
