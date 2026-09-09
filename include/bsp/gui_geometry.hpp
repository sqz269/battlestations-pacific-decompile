#pragma once
#include <array>
#include <cstdint>

namespace bsp {
struct GuiUvRect {
    float left{}, top{}, right{1.0f}, bottom{1.0f};
};
struct GuiQuadVertex { float x{}, y{}, z{}, u{}, v{}; };
struct GuiQuadParameters {
    std::uint32_t mode{}; // Native object+110h, valid values 0..4.
    float ratio{1.0f};    // Native object+114h; clamped and written back.
    float y_scale{0.75f}; // Explicit projection of global00e12fd0, not a viewport rule.
    float width{}, height{};
    GuiUvRect uv;
    GuiUvRect crop;       // Native object+118h..124h, not clamped.
};

// Native ECX object; seven stack arguments; RET1Ch. New typed output interface,
// not an original stream layout/ABI. Produces slots TL,TR,BL,BR for a strip.
// Modes 1/2 retain the top/bottom ratio; modes 3/4 retain the left/right ratio.
// Invalid modes return false after ratio clamp, leaving all vertices unchanged.
// Ordered ratio values clamp to [0,1]; NaN remains NaN. Coordinates/UVs use
// float32 intermediates: x87 extended precision, status flags and exceptions
// are not reproduced. See docs/GUI_GEOMETRY_DISPATCH.md for evidence boundaries.
bool gui_write_cropped_quad_00ab1860(GuiQuadParameters&,
    std::array<GuiQuadVertex, 4>& vertices) noexcept;
}
