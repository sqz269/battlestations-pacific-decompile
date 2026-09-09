#include "bsp/font_data.hpp"
#include "bsp/stream_scalars.hpp"
#include <utility>

namespace bsp {
namespace {
// 00ad4f3a..00ad4f4a and metric sequences at 00ad4fcd..00ad5017.
// Preserve CVTSI2SS -> MULSS -> CVTTSS2SI, including indefinite conversion
// results under the caller's SSE environment, and retain only the low word.
std::uint16_t scale_word(std::int32_t input, float scale) noexcept {
    std::int32_t converted;
    __asm {
        mov eax, input
        cvtsi2ss xmm0, eax
        mulss xmm0, scale
        cvttss2si eax, xmm0
        mov converted, eax
    }
    return static_cast<std::uint16_t>(converted);
}
}

bool decode_font_data_00ad4c30_fragment(MemoryStream& stream, float scale,
    FontData& output, std::string& error) {
    error.clear();
    if (!stream.has_backing() || !stream.fully_initialized()) {
        error = "Font DAT requires fully initialized stream backing.";
        return false;
    }
    const auto position = stream.position_00bef580();
    const auto size = stream.size_00bef600();
    if (position < 0 || position > size || size - position < 6) {
        error = "Font DAT header is incomplete.";
        return false;
    }

    FontData decoded;
    decoded.source_record_count = stream_read_u32_00be4300(stream);
    const auto raw_height = stream_read_word_00be4320(stream);
    const std::int32_t signed_height = raw_height < 0x8000u
        ? static_cast<std::int32_t>(raw_height)
        : static_cast<std::int32_t>(raw_height) - 0x10000;
    decoded.scaled_height = scale_word(signed_height, scale);
    const auto required = static_cast<std::uint64_t>(decoded.source_record_count) * 24u;
    if (required > static_cast<std::uint64_t>(size - position - 6)) {
        error = "Font DAT record extent exceeds the remaining stream.";
        return false;
    }

    for (std::uint32_t i = 0; i < decoded.source_record_count; ++i) {
        const auto key = stream_read_word_00be4320(stream);
        FontGlyphData glyph;
        for (auto& value : glyph.fields_00_0c)
            value = stream_read_float_00be4360(stream);
        glyph.field_10 = stream_read_word_00be4340(stream);
        glyph.scaled_field_12 = scale_word(stream_read_word_00be4340(stream), scale);
        glyph.scaled_field_14 = scale_word(stream_read_word_00be4320(stream), scale);
        // Native checks after reading/scaling and frees a duplicate payload.
        // map::emplace likewise retains the earlier entry; every record is read.
        decoded.glyphs.emplace(key, glyph);
    }

    if (!font_has_glyph_00ad4500(decoded, 0x0091u)) {
        error = "Font DAT lacks the native fallback glyph key 0091h.";
        return false;
    }
    output = std::move(decoded);
    return true;
}

bool font_has_glyph_00ad4500(const FontData& font, std::uint16_t key) noexcept {
    return font.glyphs.find(key) != font.glyphs.end();
}

bool font_accepts_text_byte_00ab6d00(const FontData& font,
    std::uint8_t value) noexcept {
    if (value == 0x20u) return true;
    const auto key = static_cast<std::uint16_t>(value < 0x80u
        ? static_cast<unsigned>(value)
        : 0xff00u | static_cast<unsigned>(value));
    return font_has_glyph_00ad4500(font, key);
}
}
