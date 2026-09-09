#include "bsp/font_layout.hpp"
#include <cstring>
#include <limits>
#include <utility>

namespace bsp {
namespace {
bool finite_float(float value) noexcept {
    std::uint32_t bits;
    static_assert(sizeof(bits) == sizeof(value));
    std::memcpy(&bits, &value, sizeof(bits));
    return (bits & 0x7f800000u) != 0x7f800000u;
}

bool container_width(float normalized_width, std::uint32_t& output) noexcept {
    const double multiplier = 960.0;
    const double upper = 9223372036854775808.0;
    const double lower = -9223372036854775808.0;
    std::uint16_t saved_control, truncation_control;
    std::int64_t converted;
    std::uint32_t supported;
    //00aba055..00aba0a9: retain the extended FMUL result through FISTP.
    // Added domain comparisons do not spill or round that result. Reject an
    // out-of-range conversion instead of interpreting the indefinite integer.
    __asm {
        fnstcw saved_control
        fld normalized_width
        fmul multiplier
        fld upper
        fcomip st(0), st(1)
        jbe conversion_unsupported
        fld lower
        fcomip st(0), st(1)
        ja conversion_unsupported
        mov ax, saved_control
        or ax, 0c00h
        mov truncation_control, ax
        fldcw truncation_control
        fistp converted
        fldcw saved_control
        mov supported, 1
        jmp conversion_done
    conversion_unsupported:
        fstp st(0)
        mov supported, 0
    conversion_done:
    }
    if (!supported) return false;
    output = static_cast<std::uint32_t>(converted);
    return true;
}

bool add_advance(float current, std::uint16_t advance, float scale,
    float& output) noexcept {
    const std::int32_t unsigned_advance = advance;
    const double upper = static_cast<double>((std::numeric_limits<float>::max)());
    const double lower = -upper;
    float result;
    std::uint32_t supported;
    //00aba0cf..00aba0ea and00aba1fd..00aba22c: unsigned advance, x87
    // multiply/add, then one float32 spill per code unit. Range comparisons
    // impose a host finite domain without changing the accepted arithmetic.
    __asm {
        fild unsigned_advance
        fmul scale
        fadd current
        fld upper
        fcomip st(0), st(1)
        jb advance_unsupported
        fld lower
        fcomip st(0), st(1)
        ja advance_unsupported
        fstp result
        mov supported, 1
        jmp advance_done
    advance_unsupported:
        fstp st(0)
        mov supported, 0
    advance_done:
    }
    if (!supported) return false;
    std::memcpy(&output, &result, sizeof(result));
    return true;
}

bool aligned_origin(std::uint32_t width, float measured_width,
    std::uint32_t alignment, float& output) noexcept {
    if (alignment != 1 && alignment != 2) {
        output = 0.0f;
        return true;
    }
    std::int32_t signed_width;
    std::memcpy(&signed_width, &width, sizeof(width));
    const float unsigned_correction = 4294967296.0f;
    const double half = 0.5;
    const double upper = static_cast<double>((std::numeric_limits<float>::max)());
    const double lower = -upper;
    float result;
    std::uint32_t supported;
    //00aba100..00aba148: convert the low DWORD as unsigned in x87;
    // subtract measured width before the optional double0.5 multiplication.
    __asm {
        fild signed_width
        cmp signed_width, 0
        jge origin_unsigned_ready
        fadd unsigned_correction
    origin_unsigned_ready:
        fsub measured_width
        cmp alignment, 1
        jne origin_scale_ready
        fmul half
    origin_scale_ready:
        fld upper
        fcomip st(0), st(1)
        jb origin_unsupported
        fld lower
        fcomip st(0), st(1)
        ja origin_unsupported
        fstp result
        mov supported, 1
        jmp origin_done
    origin_unsupported:
        fstp st(0)
        mov supported, 0
    origin_done:
    }
    if (!supported) return false;
    std::memcpy(&output, &result, sizeof(result));
    return true;
}
}

bool build_font_single_line_00ab9fd0_fragment(const FontData& font,
    std::u16string_view transformed_text, const FontSingleLineParameters& parameters,
    FontSingleLineLayout& output, std::string& error) {
    error.clear();
    if (!finite_float(parameters.normalized_width) || !finite_float(parameters.width_scale)) {
        error = "Single-line layout requires finite width and scale.";
        return false;
    }
    const auto nul = transformed_text.find(u'\0');
    const auto count = nul == std::u16string_view::npos ? transformed_text.size() : nul;
    if (count > parameters.max_glyphs || count > 16384u) {
        error = "Single-line glyph count exceeds the supplied capacity or 16-bit quad index range.";
        return false;
    }

    FontSingleLineLayout candidate;
    candidate.height = font.scaled_height;
    if (!container_width(parameters.normalized_width, candidate.container_width)) {
        error = "Single-line container width is outside the signed 64-bit conversion domain.";
        return false;
    }
    for (std::size_t i = 0; i < count; ++i) {
        const auto key = static_cast<std::uint16_t>(transformed_text[i]);
        const auto& glyph = select_font_glyph_00ad4480(font, key);
        if (!add_advance(candidate.measured_width, glyph.scaled_field_12,
            parameters.width_scale, candidate.measured_width)) {
            error = "Single-line measured advance exceeds the finite float domain.";
            return false;
        }
    }
    if (!aligned_origin(candidate.container_width, candidate.measured_width,
        parameters.alignment, candidate.initial_x)) {
        error = "Single-line aligned origin exceeds the finite float domain.";
        return false;
    }
    candidate.final_x = candidate.initial_x;
    candidate.placements.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const auto key = static_cast<std::uint16_t>(transformed_text[i]);
        const auto& glyph = select_font_glyph_00ad4480(font, key);
        candidate.placements.push_back({key, candidate.final_x, 0.0f});
        if (!add_advance(candidate.final_x, glyph.scaled_field_12,
            parameters.width_scale, candidate.final_x)) {
            error = "Single-line pen position exceeds the finite float domain.";
            return false;
        }
    }
    output = std::move(candidate);
    return true;
}
}
