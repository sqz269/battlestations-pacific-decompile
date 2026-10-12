#pragma once

// HUD map screen zoom (cc13-warnings1): the wheel-driven zoom transition and the smooth zoom
// level of BSP_HudMapScreen_Update (005B45C0, __thiscall ECX = screen, float dt, RET 4).
// The screen keeps its Simple3dScene at screen+8h, so screen+1Ch is the scene's
// cSmoothMapZoomLevel (+14h) and screen+2Ch..+34h are the scene's level and extents
// (bsp/strategic_map_cc13.hpp). Reconstruct-only. docs/CC13_MAP_SCREEN_UPDATE.md.

#include <cstdint>

namespace bsp {

struct MapScreenZoomState {
    // Scene fields seen from the screen (scene = screen+8h).
    float smooth_zoom_1c{1.0f};      // scene+14h "cSmoothMapZoomLevel" / "cZoomLerp"
    float shadow_fade_20{1.0f};      // scene+18h "cShadowFade"
    float fade_24{0.0f};             // scene+1Ch
    float mid_fade_28{1.0f};         // scene+20h
    std::int32_t level_2c{0};        // scene+24h
    float extent_30{0.0f};           // scene+28h, world metres across
    float extent_v_34{0.0f};         // scene+2Ch
    bool zooming_in_40{false};
    bool zooming_out_41{false};
    float pan_x_54c{0.0f};
    float pan_y_550{0.0f};
    float cursor_world_x_554{0.0f};
    float cursor_world_y_558{0.0f};
    float remaining_5ac{0.0f};       // transition time left
    float target_extent_5b0{0.0f};
    float target_extent_v_5b4{0.0f};
    float scale_5b8{0.0f};           // lerps to +5BCh
    float scale_target_5bc{0.0f};
    float table_e0cf50_5c4{0.0f};    // lerps to +5C8h
    float table_e0cf50_target_5c8{0.0f};
    float out_weight_5d0{0.0f};      // 1 -> 0 over the transition
    float in_weight_5d4{1.0f};       // 0 -> 1
    float level_float_5d8{0.0f};     // lerps to float(level)
    float value_5dc{0.0f};           // lerps to +5E0h
    float value_target_5e0{0.0f};
    float pan_target_x_5e8{0.0f};
    float pan_target_y_5ec{0.0f};
    std::int32_t min_level_5fc{0};
};

// 005B46E8..005B476C. The cursor in normalised GUI units (GuiManager+5Ch/+60h, or 0.5/0.5 when
// game+634h is set) mapped to world: x = (mx - 0.5) * extent + pan_x, y = (0.5 - my) * extent_v
// + pan_y, each stored as a float. 00E08890/00E08894 (0.5) are .data and could be rewritten.
void map_cursor_to_world_005b46e8(MapScreenZoomState&, float mouse_x, float mouse_y,
    float centre_x = 0.5f, float centre_y = 0.5f);

// 005B4772..005B496C: one frame of the zoom transition.
void step_map_zoom_005b4772(MapScreenZoomState&, float dt);

// Per-level inputs of the zoom start that are not constants of the image.
struct MapZoomLevelInputs {
    float scale_f89ae4[4]{};          // BSS table 00F89AE4, filled at run time
    float config_scale_2bc{1.0f};     // GlobalConfig+2BCh
    float config_duration_2b8{0.0f};  // GlobalConfig+2B8h
    float value_0059abc0{0.0f};       // 0059ABC0() after the level change
};

// 005B4E3C..005B5177: the wheel trigger and the transition start. `wheel_ready` is input
// +24E8h set and +24E0h clear; `wheel_delta` is input +24E4h. A negative delta zooms in while
// level < 3, a positive one zooms out while level > +5FCh; zoom-in wins. Returns false when
// nothing starts. After a start the native clamps the pan target through
// 0059E120(ECX = screen, &pan_target, extent, extent_v); that call is left to the host.
bool begin_map_zoom_005b4e3c(MapScreenZoomState&, bool wheel_ready, float wheel_delta,
    const MapZoomLevelInputs&);

// 005B58DF..005B59FE: the shader values. While a direction flag is set, smooth_zoom_1c moves
// between level+0 and level+1 (zoom in) or level+2 and level+1 (zoom out), and mid_fade_28 =
// |2(t - 0.5)|^3 with t = remaining / duration. Then shadow_fade_20 = 1 - clamp(z - 2, 0, 1)
// and fade_24 = 1 - (1 - (z - 1) * 0.5)^3, with z = smooth_zoom_1c.
void update_map_zoom_shader_values_005b58df(MapScreenZoomState&, float config_duration_2b8);

} // namespace bsp
