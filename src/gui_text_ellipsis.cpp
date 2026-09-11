#include "bsp/gui_text_ellipsis.hpp"

#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
//00AB8F18..00AB8F63. Retain the x87 product through signed32 conversion;
// do not round it to float/double before truncation. FISTP also preserves
// the native masked-invalid integer-indefinite result, whose low16 is zero.
std::uint16_t target_width(float normalized) {
    const double multiplier = 960.0;
    std::uint16_t saved_control, truncation_control;
    std::int32_t converted;
    __asm {
        fnstcw saved_control
        fld normalized
        fmul multiplier
        mov ax, saved_control
        or ax, 0c00h
        mov truncation_control, ax
        fldcw truncation_control
        fistp converted
        fldcw saved_control
    }
    return static_cast<std::uint16_t>(converted);
}

//00AB8FC4..00AB9004 accumulation,00AB9032..00AB9073 dot reservation,
//00AB90AB..00AB90F0 suffix subtraction. The native multiply/add/subtract
// occurs at current x87 precision; only FISTP uses temporary truncation.
std::uint16_t change_width(std::uint16_t current, std::uint16_t advance,
    float scale, bool subtract, bool three_dots = false) {
    const std::int32_t unsigned_current = current;
    const std::int32_t unsigned_advance = advance;
    const double three = 3.0;
    std::uint16_t saved_control, truncation_control;
    std::int32_t converted;
    __asm {
        fnstcw saved_control
        cmp subtract, 0
        jne width_subtract
        fild unsigned_advance
        fmul scale
        fild unsigned_current
        faddp st(1), st(0)
        jmp width_convert
    width_subtract:
        fild unsigned_current
        fild unsigned_advance
        fmul scale
        cmp three_dots, 0
        je width_single_advance
        fmul three
    width_single_advance:
        fsubp st(1), st(0)
    width_convert:
        mov ax, saved_control
        or ax, 0c00h
        mov truncation_control, ax
        fldcw truncation_control
        fistp converted
        fldcw saved_control
    }
    return static_cast<std::uint16_t>(converted);
}
} // namespace

std::u16string ellipsize_gui_text_00ab8f00(const GuiTextWidget& widget,
    const FontData& font, const LocaleTables& locale,
    LocaleTextRuntimeHost& locale_runtime, std::u16string_view source,
    float normalized_width) {
    auto target = target_width(normalized_width); // Before copy/locale callbacks.
    if (source.size() > static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)()) ||
        source.find(u'\0') != std::u16string_view::npos)
        throw std::invalid_argument("Text ellipsis requires null-free signed32-length UTF16");
    std::u16string output(source);
    if (!widget.font)
        throw std::invalid_argument("Text ellipsis requires the widget's resolved font");
    if (widget.font->uppercase_only) {
        //00A9EC30 uses the same existing locale mapping for every code unit.
        for (auto& unit : output)
            unit = locale_uppercase_00a9eba0(locale, locale_runtime, unit);
    }
    if (output.find(u'\0') != std::u16string::npos)
        throw std::invalid_argument("Text ellipsis uppercase mapping produced an embedded NUL");
    std::uint16_t measured = 0;
    for (const auto unit : output) {
        const auto& glyph = select_font_glyph_00ad4480(font, unit);
        measured = change_width(measured, glyph.scaled_field_12, widget.font_scale, false);
    }
    if (measured <= target) return output;
    const auto& dot = select_font_glyph_00ad4480(font, u'.');
    target = change_width(target, dot.scaled_field_12, widget.font_scale, true, true);
    auto retained = output.size();
    while (retained != 0 && measured > target) {
        const auto& glyph = select_font_glyph_00ad4480(font, output[retained - 1]);
        --retained;
        measured = change_width(measured, glyph.scaled_field_12, widget.font_scale, true);
    }
    if (retained > static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)()) - 3)
        throw std::out_of_range("Text ellipsis result exceeds signed32 length domain");
    output.resize(retained);
    output += u"...";
    return output;
}

std::optional<std::u16string> prepare_gui_text_ellipsis_00abb000_fragment(
    GuiTextWidget& widget, GuiTextHost& host, const FontData& font,
    const LocaleTables& locale, LocaleTextRuntimeHost& locale_runtime,
    const std::string& source, float normalized_width, bool localize) {
    if (!source_text_changed_00abaed0(widget, source)) return std::nullopt;
    if (source.size() > static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)()) ||
        source.find('\0') != std::string::npos)
        throw std::invalid_argument("Text ellipsis requires a null-free signed32-length source");
    widget.source = source;
    //00D7A260 =00 00 80 BF. UCOMISS/LAHF/TEST/JP selects ONLY ordered -1.
    // Retain the captured width if the resolver reenters and changes the size.
    if (normalized_width == -1.0f) normalized_width = widget.size.width;
    const auto converted = localize ? host.resolve_localised(source)
                                    : host.widen_source(source);
    return ellipsize_gui_text_00ab8f00(widget, font, locale, locale_runtime,
        converted, normalized_width);
}
} // namespace bsp
