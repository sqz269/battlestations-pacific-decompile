#include "bsp/gui_geometry.hpp"

namespace bsp {
bool gui_write_cropped_quad_00ab1860(GuiQuadParameters& parameters,
    std::array<GuiQuadVertex, 4>& vertices) noexcept {
    // COMISS/JBE at00ab186e..1885 leaves unordered and signed zero unchanged.
    if (parameters.ratio < 0.0f) parameters.ratio = 0.0f;
    else if (parameters.ratio > 1.0f) parameters.ratio = 1.0f;
    // CMP mode,4 / JA at00ab18f3/193d is unsigned.
    if (parameters.mode > 4) return false;

    // Semantic float32 arithmetic, deliberately not claimed x87 bit parity.
    const float source_u_span = parameters.uv.right - parameters.uv.left;
    const float source_v_span = parameters.uv.bottom - parameters.uv.top;
    float u_left = parameters.uv.left + parameters.crop.left * source_u_span;
    float u_right = parameters.uv.left + parameters.crop.right * source_u_span;
    float v_top = parameters.uv.top + parameters.crop.top * source_v_span;
    float v_bottom = parameters.uv.top + parameters.crop.bottom * source_v_span;
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
        v_bottom = v_top + v_span * ratio;
        break;
    case 2: // Move top edge toward bottom.
        y_top = remainder * y_bottom;
        v_top = v_top + remainder * v_span;
        break;
    case 3: // 00ab1c96: move right edge toward left.
        x_right = parameters.width * ratio;
        u_right = u_left + u_span * ratio;
        break;
    case 4: // 00ab1dc5: move left edge toward right.
        x_left = remainder * parameters.width;
        u_left = u_left + remainder * u_span;
        break;
    }
    // Native writes 0,1,3,2 through mapped bytes/stride/semantic offsets. Here
    // positions and UVs are projected fields; native white-color writes are in
    // the caller00ab3cb0 and do not belong to this geometry writer.
    vertices[0] = {x_left, y_top, 0.0f, u_left, v_top};
    vertices[1] = {x_right, y_top, 0.0f, u_right, v_top};
    vertices[3] = {x_right, y_bottom, 0.0f, u_right, v_bottom};
    vertices[2] = {x_left, y_bottom, 0.0f, u_left, v_bottom};
    return true;
}
}
