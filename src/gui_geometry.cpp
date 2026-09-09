#include "bsp/gui_geometry.hpp"

namespace bsp {
namespace {
float interpolate(float begin, float end, float fraction) noexcept {
    float result;
    __asm {
        fld dword ptr [end]
        fsub dword ptr [begin]
        fmul dword ptr [fraction]
        fadd dword ptr [begin]
        fstp dword ptr [result]
    }
    return result;
}
float multiply_add(float span, float fraction, float begin) noexcept {
    float result;
    __asm {
        fld dword ptr [span]
        fmul dword ptr [fraction]
        fadd dword ptr [begin]
        fstp dword ptr [result]
    }
    return result;
}
float multiply_three(float a, float b, float c) noexcept {
    float result;
    __asm {
        fld dword ptr [a]
        fmul dword ptr [b]
        fmul dword ptr [c]
        fstp dword ptr [result]
    }
    return result;
}
}

bool gui_write_cropped_quad_00ab1860(GuiQuadParameters& parameters,
    std::array<GuiQuadVertex, 4>& vertices) noexcept {
    // COMISS/JBE at00ab186e..1885 leaves unordered and signed zero unchanged.
    if (parameters.ratio < 0.0f) parameters.ratio = 0.0f;
    else if (parameters.ratio > 1.0f) parameters.ratio = 1.0f;
    // CMP mode,4 / JA at00ab18f3/193d is unsigned.
    if (parameters.mode > 4) return false;

    // Native crop expressions retain x87 intermediates until the final stores.
    float u_left = interpolate(parameters.uv.left, parameters.uv.right, parameters.crop.left);
    float u_right = interpolate(parameters.uv.left, parameters.uv.right, parameters.crop.right);
    float v_top = interpolate(parameters.uv.top, parameters.uv.bottom, parameters.crop.top);
    float v_bottom = interpolate(parameters.uv.top, parameters.uv.bottom, parameters.crop.bottom);
    const float u_span = u_right - u_left;
    const float v_span = v_bottom - v_top;
    const float ratio = parameters.ratio;
    const float remainder = 1.0f - ratio;
    float x_left = 0.0f, x_right = parameters.width;
    float y_top = 0.0f, y_bottom = parameters.y_scale * parameters.height;

    switch (parameters.mode) {
    case 0: // 00ab194b: full rectangle, regardless of ratio.
        break;
    case 1: // 00ab1a46: move bottom edge toward top.
        y_bottom = ratio * y_bottom;
        v_bottom = multiply_add(v_span, ratio, v_top);
        break;
    case 2: // Move top edge toward bottom.
        y_top = remainder * y_bottom;
        v_top = multiply_add(v_span, remainder, v_top);
        break;
    case 3: // 00ab1c96: move right edge toward left.
        x_right = parameters.width * ratio;
        u_right = multiply_add(u_span, ratio, u_left);
        break;
    case 4: // 00ab1dc5: move left edge toward right.
        x_left = remainder * parameters.width;
        u_left = multiply_add(u_span, remainder, u_left);
        break;
    }
    // Native writes 0,1,3,2 through mapped bytes/stride/semantic offsets. Here
    // positions and UVs are projected fields; native white-color writes are in
    // the caller00ab3cb0 and do not belong to this geometry writer.
    vertices[0] = {x_left, y_top, 0.0f, u_left, v_top};
    vertices[1] = {x_right, y_top, 0.0f, u_right, v_top};
    vertices[3] = {x_right, y_bottom, 0.0f, u_right, v_bottom};
    vertices[2] = {x_left, y_bottom, 0.0f, u_left, v_bottom};
    // Native opposite corners can use different multiplication order; do not
    // merge these expressions into a shared pre-rounded height.
    if (parameters.mode == 1) {
        vertices[3].y = multiply_three(parameters.y_scale, parameters.height, ratio);
        vertices[2].y = multiply_three(ratio, parameters.height, parameters.y_scale);
    } else if (parameters.mode == 2) {
        vertices[0].y = multiply_three(parameters.y_scale, parameters.height, remainder);
        vertices[1].y = multiply_three(parameters.height, parameters.y_scale, remainder);
    }
    return true;
}
}
