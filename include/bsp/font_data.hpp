#pragma once
#include "bsp/memory_stream.hpp"
#include <array>
#include <cstdint>
#include <map>
#include <string>

namespace bsp {
// Field offsets refer to native glyph payloads, not this projected C++ layout.
// Native padding +16h and resource pointers +18h/+1Ch are deliberately absent.
struct FontGlyphData {
    std::array<float, 4> fields_00_0c{};
    std::uint16_t field_10{};
    std::uint16_t scaled_field_12{};
    std::uint16_t scaled_field_14{};
};

struct FontData {
    std::uint16_t scaled_height{}; // Low word stored at native font+14h.
    std::uint32_t source_record_count{};
    // Owning ordered-map projection; native tree/iterator ABI is not reproduced.
    std::map<std::uint16_t, FontGlyphData> glyphs;
};

// Complete-input DAT population fragment of 00ad4c30. Reads at current position;
// accepts trailing bytes. Requires native post-load fallback key 0091h. Output
// is unchanged on rejection, but header/record reads may advance the stream.
// Texture loading, special glyph payloads and native font lifetime are separate.
bool decode_font_data_00ad4c30_fragment(MemoryStream& stream, float scale,
    FontData& output, std::string& error);

// Semantic projections of native unsigned-16 tree membership and byte acceptance.
bool font_has_glyph_00ad4500(const FontData& font, std::uint16_t key) noexcept;
bool font_accepts_text_byte_00ab6d00(const FontData& font,
    std::uint8_t value) noexcept;
}
