#pragma once
#include "bsp/font_layout.hpp"

namespace bsp {
struct FontWrappedParameters {
    float normalized_width{}; // Native context+20h.
    float normalized_height{}; // Native context+24h.
    float width_scale{1.0f}; // Native context+1D8h.
    float vertical_scale{1.0f}; // Explicit mutable global00e12fd4 input.
    float distance_between_lines{}; // Native context+10Ch.
    std::uint32_t horizontal_alignment{}; // 0=left,1=center,2=right,3=justify.
    std::uint32_t vertical_alignment{}; // 0=top,1=center,2=bottom.
    std::size_t max_glyphs{16384};
    std::size_t max_lines{16384};
};

struct FontWrappedLine {
    std::size_t first_placement{};
    std::size_t placement_count{}; // Includes LF if its interval ends at LF.
    float initial_x{}, y{}; // Raw pen before quad normalization.
    float emitted_width{}; // Unscaled nonspaces plus chosen space increments.
};

struct FontWrappedLayout {
    std::vector<FontGlyphPlacement> placements; // Owns values, no font pointers.
    std::vector<FontWrappedLine> lines; // A processed line can have zero placements.
    std::uint32_t container_width{};
    std::uint16_t height{}; // Quad writer consumes unsigned low16 font height.
    float measured_width{}; // Max emitted_width, initialized0 like native caller+114h.
    float measured_height{}; // Native max_y-min_y, context+178h.
    float normalized_height{}; // measured_height/720 * vertical_scale, float spill.
    float normalized_vertical_offset{}; // Add AFTER quad coordinate normalization.
};

// Scalar fragment of00aba270, native ECX=context; UTF16 wrapper/draw section
// stack arguments, RET8. Input is already transformed; first NUL or span end
// terminates it. LF ends a scan but is emitted; CR is not a line boundary.
// Spaces retain the native scanning/emission scale asymmetry and endpoint rules.
//
// Requires decoded FontData, nonempty input, at least one emitted code unit,
// finite scalars/results, signed64 width and signed32 scan conversions, modes
// shown above, and counts within supplied capacities and the16384 hard cap.
// Unsafe backward endpoints, non-progress and justified count1 division are
// rejected. False leaves output unchanged; allocation exceptions propagate.
// The caller's x87 and SSE environment remains active; temporary x87 truncation
// is restored. Floating exception flags/trap timing are not a parity promise.
// Native buffer ownership, string mutation, child UI and material binding are
// outside this interface. See docs/FONT_WRAPPED_LAYOUT.md.
bool build_font_wrapped_00aba270_fragment(const FontData& font,
    std::u16string_view transformed_text, const FontWrappedParameters& parameters,
    FontWrappedLayout& output, std::string& error);

//00aba86d..00aba896 y arithmetic: add offset to an already-normalized vertex y,
// spill/reload/store through x87. False for nonfinite input/result, output intact.
// Apply to each of the four y values AFTER write_font_quad00ab98f0_fragment.
bool apply_font_wrapped_vertical_offset_00aba860_fragment(float normalized_y,
    float vertical_offset, float& output) noexcept;
}
