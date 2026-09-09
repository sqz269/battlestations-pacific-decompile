#pragma once
#include "bsp/font_data.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
struct FontGeometryParameters {
    float x{}, y{};
    float width_scale{1.0f}; // Native text context+1D8h.
    float vertical_scale{1.0f}; // Explicit input for mutable global00e12fd4.
    std::uint16_t height{}; // Native stack argument8, low word.
    std::uint32_t quad_index{}; // Native stack argument7; index base = 4*this.
};
struct FontGeometryLayout {
    std::uint32_t stride{};
    std::uint32_t position_offset{}; // Native writer+10h, three floats.
    std::uint32_t uv_offset{}; // Native writer+28h, two floats.
    std::int32_t packed_color_offset{-1}; // Native writer+34h; negative selects floats.
    std::array<std::uint32_t, 4> float_color_offsets{}; // Writer+38h..44h.
};
// Projects00ab98f0..00ab9c5d: TL,TR,BR,BL positions/UVs, white color and six
// triangle indices. Native context ECX, ten stack arguments, RET28h.
// Does not execute the later optional child-GUI/UV-blanking branch.
// Buffer must not alias inputs/indices. Host bounds rejection leaves outputs
// unchanged; native writer does not validate. No original writer-object ABI.
bool write_font_quad_00ab98f0_fragment(const FontGlyphData& glyph,
    const FontGeometryParameters& parameters, const FontGeometryLayout& layout,
    std::uint8_t* vertices, std::size_t vertex_bytes, std::uint32_t first_vertex,
    std::array<std::uint16_t, 6>& indices) noexcept;
}
