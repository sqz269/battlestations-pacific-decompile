#include "bsp/map_screen_update_cc13.hpp"

#include "bsp/strategic_map_cc13.hpp"

#include <cmath>

namespace bsp {

namespace {

using D = double; // x87 intermediates are 80-bit; double is the closest MSVC Win32 type.

// a + (target - a) * f, one FSTP dword at the end (005B4827..005B4848 pattern).
float lerp_store(float a, float target, D f)
{
    return static_cast<float>(D(a) + (D(target) - a) * f);
}

} // namespace

void map_cursor_to_world_005b46e8(MapScreenZoomState& s, float mouse_x, float mouse_y,
    float centre_x, float centre_y)
{
    s.cursor_world_x_554 = static_cast<float>((D(mouse_x) - centre_x) * s.extent_30 + s.pan_x_54c);
    s.cursor_world_y_558 = static_cast<float>((D(centre_y) - mouse_y) * s.extent_v_34 + s.pan_y_550);
}

void step_map_zoom_005b4772(MapScreenZoomState& s, float dt)
{
    const float r = s.remaining_5ac;
    if (!(r > 0.0f)) return; // COMISS / JBE
    if (!(dt < r)) {
        // 005B4799..005B4818: snap to the targets.
        s.remaining_5ac = 0.0f;
        s.extent_30 = s.target_extent_5b0;
        s.extent_v_34 = s.target_extent_v_5b4;
        s.pan_x_54c = s.pan_target_x_5e8;
        s.pan_y_550 = s.pan_target_y_5ec;
        s.in_weight_5d4 = 1.0f;
        s.scale_5b8 = s.scale_target_5bc;
        s.out_weight_5d0 = 0.0f;
        s.level_float_5d8 = static_cast<float>(s.level_2c); // CVTSI2SS
        s.value_5dc = s.value_target_5e0;
        s.table_e0cf50_5c4 = s.table_e0cf50_target_5c8;
        return;
    }
    // 005B481D: f = dt / r, stored as a float; the stack copy stays 80-bit until the
    // weights below, so the float is used for the lerps and the 80-bit for nothing else.
    const float f = static_cast<float>(D(dt) / r);
    s.extent_30 = lerp_store(s.extent_30, s.target_extent_5b0, f);
    s.extent_v_34 = lerp_store(s.extent_v_34, s.target_extent_v_5b4, f);
    const float dx = s.pan_target_x_5e8 - s.pan_x_54c; // FSUB, FSTP dword
    const float dy = s.pan_target_y_5ec - s.pan_y_550;
    const float mx = static_cast<float>(D(dx) * f);
    const float my = static_cast<float>(D(dy) * f);
    s.pan_x_54c = mx + s.pan_x_54c;
    s.pan_y_550 = my + s.pan_y_550;
    s.scale_5b8 = lerp_store(s.scale_5b8, s.scale_target_5bc, f);
    s.value_5dc = lerp_store(s.value_5dc, s.value_target_5e0, f);
    s.table_e0cf50_5c4 = lerp_store(s.table_e0cf50_5c4, s.table_e0cf50_target_5c8, f);
    s.out_weight_5d0 = static_cast<float>(D(s.out_weight_5d0) - f);
    s.in_weight_5d4 = static_cast<float>(D(s.in_weight_5d4) + f);
    s.level_float_5d8 = static_cast<float>(D(s.level_float_5d8)
        + (D(s.level_2c) - s.level_float_5d8) * f);                 // FILD level
    s.remaining_5ac = static_cast<float>(D(r) - dt);                // FSUBR
}

bool begin_map_zoom_005b4e3c(MapScreenZoomState& s, bool wheel_ready, float wheel_delta,
    const MapZoomLevelInputs& in)
{
    if (!wheel_ready) return false;
    const bool zoom_in = 0.0f > wheel_delta && s.level_2c < 3;               // 005B4E68..005B4E75
    const bool zoom_out = wheel_delta > 0.0f && s.level_2c > s.min_level_5fc; // 005B4E98..005B4EAF
    if (!zoom_in && !zoom_out) return false;

    const std::int32_t old_level = s.level_2c;
    // The native indexes these tables without a bound check.
    s.scale_5b8 = static_cast<float>(D(in.scale_f89ae4[old_level]) * in.config_scale_2bc);
    s.level_float_5d8 = static_cast<float>(old_level);
    s.table_e0cf50_5c4 = kMapZoomCrossfade[static_cast<std::size_t>(old_level)];
    const std::int32_t level = zoom_in ? old_level + 1 : old_level - 1;
    s.zooming_in_40 = zoom_in;
    s.zooming_out_41 = !zoom_in;
    s.level_2c = level;
    s.pan_target_x_5e8 = s.pan_x_54c;
    s.pan_target_y_5ec = s.pan_y_550;
    s.out_weight_5d0 = 1.0f;
    s.in_weight_5d4 = 0.0f;
    s.scale_target_5bc = static_cast<float>(D(in.scale_f89ae4[level]) * in.config_scale_2bc);
    s.table_e0cf50_target_5c8 = kMapZoomCrossfade[static_cast<std::size_t>(level)];
    s.value_target_5e0 = in.value_0059abc0;
    s.remaining_5ac = in.config_duration_2b8;
    const float extent = kMapZoomExtents[static_cast<std::size_t>(level)];
    s.target_extent_5b0 = extent;
    s.target_extent_v_5b4 = static_cast<float>(D(extent) * kMapExtentAspect);
    // (005B4FD6..005B5030 marks every entry of the list at +4F4h: entry+10h -> +9h = 1.)

    // 005B503D..005B5177: keep the point under the cursor fixed. `k` is the extent change
    // through the old extent (00E0CF40[level -/+ 1]), stored as a float; u and v are the cursor
    // in normalised units, each stored as a float.
    const D old_extent = zoom_in ? kMapZoomExtents[static_cast<std::size_t>(level - 1)]
                                 : kMapZoomExtents[static_cast<std::size_t>(level + 1)];
    const float k = zoom_in
        ? static_cast<float>((1.0 - D(s.target_extent_5b0) / old_extent) * old_extent)
        : static_cast<float>((1.0 - old_extent / s.target_extent_5b0) * s.target_extent_5b0);
    const float u = static_cast<float>((D(s.cursor_world_x_554) - s.pan_x_54c) / s.extent_30 + 0.5);
    const float v = static_cast<float>((D(s.pan_y_550) - s.cursor_world_y_558) / s.extent_v_34 + 0.5);
    const D du = (D(u) - 0.5) * k;
    const D dv = D(k) * (D(v) - 0.5) * kMapExtentAspect;
    if (zoom_in) {
        s.pan_target_x_5e8 = static_cast<float>(D(s.pan_x_54c) + du);
        s.pan_target_y_5ec = static_cast<float>(D(s.pan_y_550) - dv);
    } else {
        s.pan_target_x_5e8 = static_cast<float>(D(s.pan_x_54c) - du);
        s.pan_target_y_5ec = static_cast<float>(D(s.pan_y_550) + dv);
    }
    return true;
}

void update_map_zoom_shader_values_005b58df(MapScreenZoomState& s, float duration)
{
    if (s.zooming_in_40 || s.zooming_out_41) {
        const D t = D(s.remaining_5ac) / duration;
        const D next = D(s.level_2c + 1); // FILD / FIADD of level + 1
        s.smooth_zoom_1c = static_cast<float>(s.zooming_in_40 ? next - t : t + next);
        // (t - 0.5) * 2, stored; FABS, stored; cubed, stored.
        const float m = std::fabs(static_cast<float>((t - 0.5) + (t - 0.5)));
        s.mid_fade_28 = static_cast<float>(D(m) * m * m);
    }
    const float z = s.smooth_zoom_1c;
    float a = static_cast<float>(D(z) - 2.0); // 00D7A308
    // Both JBEs route the unordered case to "keep", so NaN passes through unclamped.
    if (0.0f > a) a = 0.0f;                    // FLDZ / FCOMIP / JBE 005B59BB
    else if (a > 1.0f) a = 1.0f;               // COMISS / JBE 005B5A77
    s.shadow_fade_20 = static_cast<float>(1.0 - a);
    const float y = static_cast<float>(1.0 - (D(z) - 1.0) * 0.5);
    s.fade_24 = static_cast<float>(1.0 - static_cast<float>(D(y) * y * y));
}

} // namespace bsp
