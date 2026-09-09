#include "bsp/font_geometry.hpp"
#include <cstring>

namespace bsp {
namespace {
void coordinates(const FontGlyphData& glyph, const FontGeometryParameters& p,
    float& left, float& top, float& right, float& bottom) noexcept {
    const std::int32_t bearing = glyph.field_10 < 0x8000u
        ? static_cast<std::int32_t>(glyph.field_10)
        : static_cast<std::int32_t>(glyph.field_10) - 0x10000;
    const std::int32_t width = glyph.scaled_field_14;
    const std::int32_t height = p.height;
    const float x = p.x, y = p.y, width_scale = p.width_scale;
    const float vertical_scale = p.vertical_scale;
    const double horizontal_divisor = 960.0, vertical_divisor = 720.0;
    float raw_left, raw_right, raw_bottom;
    float result_left, result_top, result_right, result_bottom;
    // Exact x87 arithmetic/spill sequence00ab9927..00ab99ec. In particular
    // raw_left is stored without popping, so right retains the extended sum.
    __asm {
        fild bearing
        fadd x
        fst raw_left
        fld width_scale
        fild width
        fmulp st(1), st(0)
        faddp st(1), st(0)
        fstp raw_right
        fld y
        fild height
        faddp st(1), st(0)
        fld1
        fadd st(1), st(0)
        fsubp st(1), st(0)
        fstp raw_bottom
        fld raw_left
        fld horizontal_divisor
        fdiv st(1), st(0)
        fxch st(1)
        fstp result_left
        fld y
        fld vertical_divisor
        fdiv st(1), st(0)
        fld vertical_scale
        fld st(0)
        fmulp st(3), st(0)
        fxch st(2)
        fstp result_top
        fld raw_right
        fdivrp st(3), st(0)
        fxch st(2)
        fstp result_right
        fld raw_bottom
        fdivrp st(2), st(0)
        fmulp st(1), st(0)
        fstp result_bottom
    }
    std::memcpy(&left, &result_left, sizeof(float));
    std::memcpy(&top, &result_top, sizeof(float));
    std::memcpy(&right, &result_right, sizeof(float));
    std::memcpy(&bottom, &result_bottom, sizeof(float));
}
void store_float_bits(std::uint8_t* destination, const float& value) noexcept {
    std::memcpy(destination, &value, sizeof(value));
}
void store_float_x87(std::uint8_t* destination, const float& value) noexcept {
    const float* source = &value;
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
}

bool write_font_quad_00ab98f0_fragment(const FontGlyphData& glyph,
    const FontGeometryParameters& parameters, const FontGeometryLayout& layout,
    std::uint8_t* vertices, std::size_t vertex_bytes, std::uint32_t first_vertex,
    std::array<std::uint16_t, 6>& indices) noexcept {
    if (!vertices || layout.stride == 0 || first_vertex > 0xfffffffcu) return false;
    // Validate in widened arithmetic rather than reproducing unsafe pointer wrap.
    for (std::uint64_t i = 0; i < 4; ++i) {
        const auto base = (static_cast<std::uint64_t>(first_vertex) + i) * layout.stride;
        const auto fits = [&](std::uint32_t offset, std::uint32_t bytes) {
            return base + offset + bytes <= static_cast<std::uint64_t>(vertex_bytes);
        };
        if (!fits(layout.position_offset, 12) || !fits(layout.uv_offset, 8)) return false;
        if (layout.packed_color_offset >= 0) {
            if (!fits(static_cast<std::uint32_t>(layout.packed_color_offset), 4)) return false;
        } else {
            for (const auto offset : layout.float_color_offsets)
                if (!fits(offset, 4)) return false;
        }
    }
    float left, top, right, bottom;
    coordinates(glyph, parameters, left, top, right, bottom);
    const float* xs[] = {&left, &right, &right, &left};
    const float* ys[] = {&top, &top, &bottom, &bottom};
    const unsigned u_indices[] = {1, 3, 3, 1};
    const unsigned v_indices[] = {0, 0, 2, 2};
    const float zero = 0.0f, white = 1.0f;
    const std::uint32_t packed_white = 0xffffffffu;
    for (std::uint32_t i = 0; i < 4; ++i) {
        auto* base = vertices + static_cast<std::size_t>(
            (static_cast<std::uint64_t>(first_vertex) + i) * layout.stride);
        auto* position = base + layout.position_offset;
        if (i == 0) {
            // Native first vertex uses x87 loads/stores; subsequent ones MOVSS.
            store_float_bits(position + 8, zero);
            store_float_x87(position, *xs[i]);
            store_float_x87(position + 4, *ys[i]);
        } else {
            store_float_bits(position, *xs[i]);
            store_float_bits(position + 4, *ys[i]);
            store_float_bits(position + 8, zero);
        }
        store_float_bits(base + layout.uv_offset, glyph.fields_00_0c[u_indices[i]]);
        store_float_bits(base + layout.uv_offset + 4, glyph.fields_00_0c[v_indices[i]]);
        if (layout.packed_color_offset >= 0)
            std::memcpy(base + layout.packed_color_offset, &packed_white, sizeof(packed_white));
        else
            for (const auto offset : layout.float_color_offsets)
                store_float_bits(base + offset, white);
    }
    const auto index = parameters.quad_index * 4u;
    // Preserve native store order and low16 wrapping, independent of first_vertex.
    indices[1] = static_cast<std::uint16_t>(index + 1u);
    indices[0] = static_cast<std::uint16_t>(index);
    indices[3] = static_cast<std::uint16_t>(index);
    indices[2] = static_cast<std::uint16_t>(index + 2u);
    indices[4] = static_cast<std::uint16_t>(index + 2u);
    indices[5] = static_cast<std::uint16_t>(index + 3u);
    return true;
}
}
