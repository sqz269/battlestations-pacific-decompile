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
    std::array<float, 4> fields_00_0c{}; // UV edges used by 00ab98f0.
    std::uint16_t field_10{}; // Raw bits; quad writer uses signed horizontal offset.
    std::uint16_t scaled_field_12{}; // Unsigned horizontal advance in layout.
    std::uint16_t scaled_field_14{}; // Unsigned quad width, distinct from advance.
};

struct FontData {
    std::uint16_t scaled_height{}; // Low word stored at native font+14h.
    std::uint32_t source_record_count{};
    // Owning ordered-map projection; native tree/iterator ABI is not reproduced.
    std::map<std::uint16_t, FontGlyphData> glyphs;
    FontGlyphData space_lf_glyph; // Native embedded payload at font+4Ch.
    FontGlyphData missing_glyph; // Known-field snapshot of key 0091h, font+6Ch.
    FontGlyphData carriage_return_glyph; // Native embedded payload at font+8Ch.
};

// Complete-input DAT population fragment of 00ad4c30. Reads at current position;
// accepts trailing bytes. Requires native post-load fallback key 0091h. Output
// is unchanged on rejection, but header/record reads may advance the stream.
// Populates special scalar records. Texture/resource fields, native payload
// padding and font lifetime remain separate.
bool decode_font_data_00ad4c30_fragment(MemoryStream& stream, float scale,
    FontData& output, std::string& error);

// Semantic projections of native unsigned-16 tree membership and byte acceptance.
bool font_has_glyph_00ad4500(const FontData& font, std::uint16_t key) noexcept;
bool font_accepts_text_byte_00ab6d00(const FontData& font,
    std::uint8_t value) noexcept;
// Special keys take precedence over map entries. Requires successfully decoded
// FontData for native meaning; this does not uppercase or perform text layout.
const FontGlyphData& select_font_glyph_00ad4480(const FontData& font,
    std::uint16_t key) noexcept;
}
