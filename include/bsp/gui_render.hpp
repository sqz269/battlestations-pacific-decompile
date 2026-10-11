#pragma once
// Geometry and layout kernels of four GUI widget classes the Lua page loader
// builds from a key suffix (00AA2490 maps the suffix to a type id, 00AA6560
// allocates): `Curve` (type 0Dh), `Line` (type 4), `Scrollbar` (type 7) and
// `Progbar` (type 5).
//
// Addresses (all hypotheses, not recovered symbols; evidence in
// docs/CC13_GUI_RENDER.md):
//   Curve      00ABE030 ctor, vtable 00D5C7F8, 00ABE1D0 property reader (+18h),
//              00ABC7A0 property writer (+1Ch), 00ABDD80 set texture,
//              00ABC920 clear points, 00ABC970 add point, 00ABCA10 rebuild
//              control points, 00ABCED0 tessellate (+7Ch)
//   Line       00AC2D00 ctor, vtable 00D5C9E8, 00AC2DE0 property reader,
//              00AC2950 reset (+7Ch), 00AC2690 grow buffers, 00AC29D0 flush
//              (+80h)
//   Scrollbar  00AC7C20 ctor, vtable 00D5CCF8, 00AC85A0 property reader,
//              00AC8770 property writer, 00AC7D00 / 00AC7D80 / 00AC7DE0 value,
//              minimum and maximum setters, 00AC8890 loaded hook (+78h),
//              00AC8A80 child layout (+7Ch)
//   Progbar    00AC77C0 ctor, vtable 00D5CC18, 00AC7A90 / 00AC7B00 property
//              reader / writer, 00AC7870 set value (+A8h), 00AC7820 get value
//              (+ACh), 00AC7930 quad writer (+B0h)
//
// None of this is binary compatible. The native routines are __thiscall /
// __fastcall members of widget objects with vtables, std::vector storage and
// locked renderer vertex streams; the functions here take the fields they
// read as arguments and return what the native code writes. Renderer objects
// (vertex/index buffer creation, locking, draw sections, materials) are not
// reproduced: the kernels produce the values the native code stores into a
// locked stream. Arithmetic follows the native operation order, but x87
// extended-precision intermediates are not reproduced bit for bit.
#include <array>
#include <cstdint>
#include <vector>

#include "bsp/gui_geometry.hpp"

namespace bsp {

struct GuiVec2f {
    float x{}, y{};
};

// ---------------------------------------------------------------------------
// Curve (type 0Dh, 140h bytes)
// ---------------------------------------------------------------------------

inline constexpr std::int32_t kGuiCurveTypeId = 0x0D;     // 00ABE030 base ctor arg
inline constexpr std::size_t kGuiCurveNativeSize = 0x140; // 00AA6560 case 0Dh

// 00ABCF0A..00ABCF24: ten samples per span and three vertices across the
// ribbon, except when the padded control array holds exactly four points
// (one authored span): then four samples and two vertices across.
inline constexpr std::uint32_t kGuiCurveSamplesPerSpan = 10;
inline constexpr std::uint32_t kGuiCurveCrossVertices = 3;
inline constexpr std::uint32_t kGuiCurveShortSamplesPerSpan = 4;
inline constexpr std::uint32_t kGuiCurveShortCrossVertices = 2;

// 00ABD27D..00ABD2C8: the per-sample arc-length weights of the one-span curve
// (four samples plus the closing one), read from 00D21A94, 00D5C7F0, 00D7A2F0,
// 00CE38B8 and a literal zero. 00D5C7E8 is the double 4.78 the chord is
// divided by before blending toward each weight.
inline constexpr std::array<float, 5> kGuiCurveShortArcWeights{
    1.89999998f, 0.0975f, 0.1f, 10.0f, 0.0f};
inline constexpr double kGuiCurveShortArcChordScale = 4.78;

// 00CE3800 (vertex Z) and 00CEC9D8 (double 0.75, the Y aspect compression).
inline constexpr float kGuiCurveVertexZ = 0.5f;
inline constexpr double kGuiCurveYScale = 0.75;

// 00ABCA10: the control array the tessellator walks. Fewer than two authored
// points leave it empty (the native routine returns before the virtual +7Ch
// call). Otherwise the authored points are framed by two phantom points, one
// mirrored before the first (p0 - (p1 - p0)) and one after the last
// (pn + (pn - pn-1)), the usual Catmull-Rom end condition.
std::vector<GuiVec2f> gui_curve_control_points_00abca10(
    const std::vector<GuiVec2f>& authored);

// One Catmull-Rom evaluation as 00ABCED0 spells it out (00ABD308..):
// 0.5 * (2p1 + (p2 - p0)t + (2p0 - 5p1 + 4p2 - p3)t^2 + (-p0 + 3p1 - 3p2 + p3)t^3),
// with the powers of t applied as repeated multiplications in that order.
GuiVec2f gui_curve_catmull_rom_00abced0(const GuiVec2f& p0, const GuiVec2f& p1,
    const GuiVec2f& p2, const GuiVec2f& p3, float t) noexcept;

struct GuiCurveMesh {
    std::uint32_t samples_per_span{};
    std::uint32_t cross_vertices{};
    std::uint32_t rows{};              // (control - 3) * samples + 1
    std::vector<GuiQuadVertex> vertices; // rows * cross, row-major
    std::vector<std::uint16_t> indices;  // 16-bit, (cross - 1) * (rows - 1) * 6
    std::uint32_t triangle_count{};      // draw section +18h
    float arc_length{};                  // the U denominator
};

// 00ABCED0, the virtual +7Ch tessellator, for a control array of at least four
// points (fewer: the native routine does nothing). `widths` is the per-authored
// point ribbon width list (+F8h, the Z of each `PosArray` entry or the second
// argument of 00ABC970); span i (control points i-1..i+2) interpolates
// widths[i-1]..widths[i] by t, so it must hold control.size() - 2 entries.
// `uv` is the atlas rectangle at +11Ch..+128h.
//
// `stale_arc_weight` stands in for a stack slot the native code never assigns
// for the ten-sample case: 00ABDC73 multiplies every arc-length step by
// [ESP+18h], which only the four-sample branch (00ABD2F2) writes. When the
// buffers are rebuilt in the same call the slot holds the index-row countdown,
// which ends at zero (00ABD10B), so the native arc length and with it every U
// numerator and the denominator are zero and U comes out as 0/0. Pass 0.0f for
// that case; when the vertex count is unchanged the slot is uninitialized.
GuiCurveMesh gui_curve_tessellate_00abced0(const std::vector<GuiVec2f>& control,
    const std::vector<float>& widths, const GuiUvRect& uv,
    float stale_arc_weight = 0.0f);

// ---------------------------------------------------------------------------
// Line (type 4, 104h bytes)
// ---------------------------------------------------------------------------

inline constexpr std::int32_t kGuiLineTypeId = 4;          // 00AC2D00 base ctor arg
inline constexpr std::size_t kGuiLineNativeSize = 0x104;   // 00AA6560 case 4
inline constexpr std::int32_t kGuiLineMinimumCapacity = 4; // 00AC2690, +100h
inline constexpr float kGuiLineYScale = 0.75f;             // 00E12FF4

struct GuiLineFlush {
    std::uint32_t first_vertex{};          // the committed count before (+FCh)
    std::vector<GuiQuadVertex> vertices;   // written at first_vertex onward
    std::vector<std::uint16_t> indices;    // written at index first_vertex * 2
    std::uint32_t element_vertex_count{};  // draw section +10h
    std::uint32_t element_primitive_count{}; // draw section +18h
    std::uint32_t committed_after{};       // new +FCh
};

// 00AC29D0, virtual +80h. `pending` is the point vector at +ECh (Vec2 `Points`
// entries), `pivot` the widget's +18h/+1Ch pair, subtracted unscaled, and
// `committed` the vertex count already in the stream (+FCh). The native loop
// runs pending.size() - committed times and reads pending[k] for vertex
// committed + k, then empties the vector and stores pending.size() as the new
// committed count: a second flush therefore only appends when the vector has
// grown past the old count, and then from its front. Reproduced as observed.
GuiLineFlush gui_line_flush_00ac29d0(const std::vector<GuiVec2f>& pending,
    const GuiVec2f& pivot, std::uint32_t committed);

// 00AC2690's capacity rule: requests below five become four.
std::int32_t gui_line_capacity_00ac2690(std::int32_t requested) noexcept;

// ---------------------------------------------------------------------------
// Scrollbar (type 7, 0FCh bytes)
// ---------------------------------------------------------------------------

inline constexpr std::int32_t kGuiScrollbarTypeId = 7;
inline constexpr std::size_t kGuiScrollbarNativeSize = 0xFC;

struct GuiScrollbarState {
    float value{};          // +ECh
    float minimum{};        // +F0h, "MinValue"
    float maximum{};        // +F4h, "MaxValue"
    bool horizontal{true};  // +F8h: 0 when "Alignment" is "Vertical", else 1
};

// Each setter ends in the virtual +7Ch layout call; callers rerun
// gui_scrollbar_layout_00ac8a80 after them.
void gui_scrollbar_set_value_00ac7d00(GuiScrollbarState&, float) noexcept;
void gui_scrollbar_set_minimum_00ac7d80(GuiScrollbarState&, float) noexcept;
void gui_scrollbar_set_maximum_00ac7de0(GuiScrollbarState&, float) noexcept;

// 00AC85A0 after the base reader: `alignment_is_vertical` is the __stricmp of
// "Alignment" against "Vertical" (a missing key reads as "" and is horizontal).
void gui_scrollbar_apply_properties_00ac85a0(GuiScrollbarState&,
    bool alignment_is_vertical, float min_value, float max_value) noexcept;

struct GuiScrollbarChildren {
    GuiVec2f back_arrow_size;     // "_BackArrow_Icon"
    GuiVec2f forward_arrow_size;  // "_ForwArrow_Icon"
    bool has_background{false};   // "_Background_Icon"
    bool has_slider{false};       // "_Slider_Icon"
    GuiVec2f slider_size;
};

struct GuiScrollbarLayout {
    GuiVec2f back_arrow;
    GuiVec2f forward_arrow;
    GuiVec2f background;   // valid when has_background
    GuiVec2f slider;       // valid when has_slider
};

// 00AC8A80, virtual +7Ch: child local positions. `size` is the scrollbar's
// +20h/+24h. Along the bar axis: back arrow at 0, forward arrow at
// size - forward, background at back, slider at
// back + (size - (slider + 2 * back)) * (value - min) / (max - min), or at back
// when max == min. The cross-axis coordinate is always 0.
GuiScrollbarLayout gui_scrollbar_layout_00ac8a80(const GuiScrollbarState&,
    const GuiVec2f& size, const GuiScrollbarChildren&) noexcept;

// ---------------------------------------------------------------------------
// Progbar (type 5, 140h bytes, derived from the Icon class)
// ---------------------------------------------------------------------------

inline constexpr std::int32_t kGuiProgbarTypeId = 5;   // 00AC77C0 stores 5 at +60h
inline constexpr float kGuiProgbarDefaultValue = 1.0f; // 00D7A24C
inline constexpr float kGuiProgbarYScale = 0.75f;      // 00E13004

// 00AC7870, virtual +A8h: the stored value is (width * v) / width, evaluated on
// the x87 stack and rounded to float. Returns true when it differs from
// `current` (then the native code stores it and calls the Icon virtuals +8Ch
// and +88h(0, 3, value)).
bool gui_progbar_set_value_00ac7870(float width, float requested,
    float& current) noexcept;

// 00AC7930, virtual +B0h: the four-vertex strip, slots TL, TR, BL, BR, filled
// left to right to `value` of the width. Positions are (0,0), (w*v,0),
// (0,0.75h), (w*v,0.75h) with Z 0; U is lerped to `value` across the atlas
// rectangle.
std::array<GuiQuadVertex, 4> gui_progbar_quad_00ac7930(const GuiUvRect& uv,
    float width, float height, float value) noexcept;

} // namespace bsp
