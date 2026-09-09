#pragma once
#include "bsp/font_data.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {
struct FontSingleLineParameters {
    float normalized_width{}; // Native context+20h; multiplied by double 960.
    float width_scale{1.0f}; // Native context+1D8h, after DAT metric scaling.
    std::uint32_t alignment{}; // 1=center, 2=right; all other values start at 0.
    std::size_t max_glyphs{16384}; // Host capacity; hard limit is also 16384.
};

struct FontGlyphPlacement {
    std::uint16_t code_unit{}; // Select its scalar glyph using00ad4480.
    float x{}, y{}; // Pixel-like coordinates supplied to the quad writer.
};

struct FontSingleLineLayout {
    std::vector<FontGlyphPlacement> placements; // Owns values, no glyph pointers.
    float measured_width{}; // Native context+114h, scaled advance sum.
    float initial_x{}; // Initial pen; native+18Ch is written only if nonempty.
    float final_x{}; // Pen after the final glyph's advance, not a quad bound.
    std::uint32_t container_width{}; // Low DWORD after native truncating int64 conversion.
    std::uint16_t height{}; // Unsigned native font+14h for the quad writer.
};

// Scalar fragment of00ab9fd0; native ECX=context, two stack slots, RET8.
// Input has already undergone any required uppercase transformation. Process
// code units through the first NUL or span end; no shaping, kerning, wrapping,
// ellipsis or LF/CR line breaks. Requires successfully decoded FontData.
//
// Finite width/scale, signed-int64 container conversion and finite intermediate
// pen/width values are required. At most min(max_glyphs,16384) placements keeps
// quad indices starting at zero within16 bits. Unsupported inputs return false
// with error and leave output unchanged; allocation exceptions propagate.
// x87 precision/rounding follow the caller. Only integer conversion temporarily
// selects truncation and restores the control word. Floating exception flags
// and trap timing are not an equivalence contract. No native renderer lifetime,
// material binding or optional child-UI behavior is represented here.
bool build_font_single_line_00ab9fd0_fragment(const FontData& font,
    std::u16string_view transformed_text, const FontSingleLineParameters& parameters,
    FontSingleLineLayout& output, std::string& error);
}
