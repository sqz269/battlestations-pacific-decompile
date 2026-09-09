#include "bsp/font_wrapped_layout.hpp"
#include <cstring>
#include <utility>

namespace bsp {
namespace {
bool finite_float(float value) noexcept {
    std::uint32_t bits;
    static_assert(sizeof(bits) == sizeof(value));
    std::memcpy(&bits, &value, sizeof(bits));
    return (bits & 0x7f800000u) != 0x7f800000u;
}

bool greater_than(float left, float right) noexcept {
    std::uint8_t result;
    // Bounds/metric branches use x87 FCOMIP, not SSE comparisons that could
    // apply a caller's MXCSR denormals-are-zero setting to these float values.
    __asm {
        fld right
        fld left
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}

std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

bool convert_container(float normalized, std::uint32_t& output) noexcept {
    const double multiplier = 960.0;
    const double upper = 9223372036854775808.0;
    const double lower = -9223372036854775808.0;
    std::uint16_t saved_control, truncation_control;
    std::int64_t converted;
    std::uint32_t supported;
    //00aba32a..00aba39d. Domain checks keep the actual x87 product unspilled.
    __asm {
        fnstcw saved_control
        fld normalized
        fmul multiplier
        fld upper
        fcomip st(0), st(1)
        jbe unsupported_container
        fld lower
        fcomip st(0), st(1)
        ja unsupported_container
        mov ax, saved_control
        or ax, 0c00h
        mov truncation_control, ax
        fldcw truncation_control
        fistp converted
        fldcw saved_control
        mov supported, 1
        jmp container_done
    unsupported_container:
        fstp st(0)
        mov supported, 0
    container_done:
    }
    if (!supported) return false;
    output = static_cast<std::uint32_t>(converted);
    return true;
}

bool truncate_scan_width(float width, std::int32_t& output) noexcept {
    //00aba4a2/00aba534 use CVTTSS2SI independently of x87 rounding control.
    if (!finite_float(width) || width < -2147483648.0f || width >= 2147483648.0f)
        return false;
    std::int32_t result;
    __asm {
        cvttss2si eax, width
        mov result, eax
    }
    output = result;
    return true;
}

enum class ScanStep { accepted, overflow, unsupported };

ScanStep scan_advance(float previous, float scale, std::uint16_t advance,
    std::uint32_t container, float& accepted) noexcept {
    const std::int32_t metric = advance;
    const auto signed_container = signed_bits(container);
    const float unsigned_correction = 4294967296.0f;
    float result;
    std::uint32_t fits;
    //00aba465..00aba4c6 compares the extended candidate BEFORE float spilling.
    // FSTP ST1 discards the container while retaining the candidate and flags.
    __asm {
        fld scale
        fild metric
        fmulp st(1), st(0)
        fadd previous
        fild signed_container
        cmp signed_container, 0
        jge scan_container_ready
        fadd unsigned_correction
    scan_container_ready:
        fxch st(1)
        fcomi st(0), st(1)
        fstp st(1)
        ja scan_overflow
        fstp result
        mov fits, 1
        jmp scan_done
    scan_overflow:
        fstp st(0)
        mov fits, 0
    scan_done:
    }
    if (!fits) return ScanStep::overflow;
    if (!finite_float(result)) return ScanStep::unsupported;
    std::memcpy(&accepted, &result, sizeof(result));
    return ScanStep::accepted;
}

bool line_alignment(std::uint32_t container, std::int32_t saved_width,
    std::uint32_t spaces, bool natural_end, std::uint32_t mode,
    std::uint16_t natural_space, float& origin, float& space_step) noexcept {
    origin = 0.0f;
    space_step = static_cast<float>(natural_space); // Native CVTSI2SS; exact u16.
    if (mode == 0 || (mode == 3 && natural_end)) return true;
    if (mode == 3 && spaces == 0) {
        space_step = 0.0f;
        return true;
    }
    if (mode == 3 && spaces == 1) return false; // Native FIDIV zero, unsupported.
    const auto slack = signed_bits(container - static_cast<std::uint32_t>(saved_width));
    const float unsigned_correction = 4294967296.0f;
    const double half = 0.5;
    const std::int32_t space_metric = natural_space;
    const std::int32_t divisor = spaces == 0 ? 0 : static_cast<std::int32_t>(spaces - 1);
    float result;
    //00aba564..00aba62f: DWORD subtraction precedes unsigned x87 conversion.
    __asm {
        fild slack
        cmp slack, 0
        jge slack_ready
        fadd unsigned_correction
    slack_ready:
        cmp mode, 3
        jne nonjustified
        fidiv divisor
        fild space_metric
        faddp st(1), st(0)
        jmp alignment_store
    nonjustified:
        cmp mode, 1
        jne alignment_store
        fmul half
    alignment_store:
        fstp result
    }
    if (!finite_float(result)) return false;
    if (mode == 3) std::memcpy(&space_step, &result, sizeof(result));
    else std::memcpy(&origin, &result, sizeof(result));
    return true;
}

bool advance_emission(float x, float emitted_width, std::uint16_t advance,
    bool is_space, float scale, float space_step, float& next_x,
    float& next_width) noexcept {
    const std::int32_t metric = advance;
    const std::uint32_t space = is_space ? 1u : 0u;
    float result_x, result_width;
    //00aba73d..00aba777 retains unscaled metric beneath the scaled x increment.
    __asm {
        cmp space, 0
        jne emit_space
        fild metric
        fld scale
        fmul st(0), st(1)
        jmp emit_add
    emit_space:
        fld space_step
        fld st(0)
    emit_add:
        fadd x
        fstp result_x
        fadd emitted_width
        fstp result_width
    }
    if (!finite_float(result_x) || !finite_float(result_width)) return false;
    std::memcpy(&next_x, &result_x, sizeof(result_x));
    std::memcpy(&next_width, &result_width, sizeof(result_width));
    return true;
}

bool line_bottom(float y, float height, float& output) noexcept {
    float result;
    __asm {
        fld y
        fadd height
        fstp result
    }
    if (!finite_float(result)) return false;
    std::memcpy(&output, &result, sizeof(result));
    return true;
}

bool advance_line(float y, float height, float distance, float& output) noexcept {
    const double one = 1.0;
    const double reduction = 0.1500000059604644775390625;
    float result;
    //00aba7a3..00aba7ca; do not simplify distance+1-reduction to distance+0.85.
    __asm {
        fld distance
        fadd one
        fsub reduction
        fmul height
        fadd y
        fstp result
    }
    if (!finite_float(result)) return false;
    std::memcpy(&output, &result, sizeof(result));
    return true;
}

bool vertical_metrics(float minimum, float maximum, float container_height,
    float vertical_scale, std::uint32_t mode, float& height,
    float& normalized_height, float& vertical_offset) noexcept {
    const double divisor = 720.0;
    const double half = 0.5;
    float result_height, result_normalized, result_offset = 0.0f;
    //00aba7dc..00aba841: raw extent spills, division stays extended through
    // the vertical-scale multiply, normalized extent spills before subtraction.
    __asm {
        fld maximum
        fsub minimum
        fstp result_height
        fld result_height
        fdiv divisor
        fld vertical_scale
        fld st(0)
        fmulp st(2), st(0)
        fxch st(1)
        fstp result_normalized
        cmp mode, 0
        je vertical_top
        fmul container_height
        fsub result_normalized
        cmp mode, 1
        jne vertical_store
        fmul half
    vertical_store:
        fstp result_offset
        jmp vertical_done
    vertical_top:
        fstp st(0)
    vertical_done:
    }
    if (!finite_float(result_height) || !finite_float(result_normalized) ||
        !finite_float(result_offset)) return false;
    std::memcpy(&height, &result_height, sizeof(result_height));
    std::memcpy(&normalized_height, &result_normalized, sizeof(result_normalized));
    std::memcpy(&vertical_offset, &result_offset, sizeof(result_offset));
    return true;
}
}

bool build_font_wrapped_00aba270_fragment(const FontData& font,
    std::u16string_view transformed_text, const FontWrappedParameters& p,
    FontWrappedLayout& output, std::string& error) {
    error.clear();
    if (!finite_float(p.normalized_width) || !finite_float(p.normalized_height) ||
        !finite_float(p.width_scale) || !finite_float(p.vertical_scale) ||
        !finite_float(p.distance_between_lines)) {
        error = "Wrapped layout requires finite scalar inputs.";
        return false;
    }
    if (p.horizontal_alignment > 3 || p.vertical_alignment > 2) {
        error = "Wrapped layout alignment mode is outside the supported native branches.";
        return false;
    }
    const auto nul = transformed_text.find(u'\0');
    const auto count = nul == std::u16string_view::npos ? transformed_text.size() : nul;
    if (count == 0) {
        error = "Empty text uses the caller's empty-geometry route, outside wrapped layout.";
        return false;
    }
    if (count > p.max_glyphs || count > 16384u) {
        error = "Wrapped input exceeds the supplied glyph capacity or 16-bit quad range.";
        return false;
    }
    const auto code_unit = [&](std::size_t index) -> std::uint16_t {
        return index == count ? 0u : static_cast<std::uint16_t>(transformed_text[index]);
    };

    FontWrappedLayout candidate;
    candidate.height = font.scaled_height;
    const std::int32_t signed_height = font.scaled_height < 0x8000u
        ? static_cast<std::int32_t>(font.scaled_height)
        : static_cast<std::int32_t>(font.scaled_height) - 0x10000;
    const float height = static_cast<float>(signed_height); //00aba284/00aba294.
    if (!convert_container(p.normalized_width, candidate.container_width)) {
        error = "Wrapped container width is outside the signed 64-bit conversion domain.";
        return false;
    }
    const auto space_advance = select_font_glyph_00ad4480(font, 0x20).scaled_field_12;
    candidate.placements.reserve(count);
    float y = 0.0f, minimum_y = 10000000000.0f, maximum_y = -10000000000.0f;
    std::size_t cursor = 0;
    while (cursor != count) {
        if (candidate.lines.size() >= p.max_lines || candidate.lines.size() >= 16384u) {
            error = "Wrapped line count exceeds the supplied capacity.";
            return false;
        }
        const auto previous_cursor = cursor;
        //00aba3e3..00aba427: preserve initial/explicit-line leading spaces only
        // for left alignment; soft-wrap spaces and other alignments skip them.
        if (code_unit(cursor) == 0x20 && (p.horizontal_alignment != 0 ||
            (cursor != 0 && code_unit(cursor - 1) != 0x0a))) {
            do { ++cursor; } while (code_unit(cursor) == 0x20);
        }
        const auto line_begin = cursor;
        auto scan = cursor;
        auto last_space = count;
        float scan_width = 0.0f;
        std::int32_t saved_width = 0;
        std::uint32_t spaces = 0;
        bool only_spaces = true;
        while (code_unit(scan) != 0 && code_unit(scan) != 0x0a) {
            const auto key = code_unit(scan);
            const auto advance = select_font_glyph_00ad4480(font, key).scaled_field_12;
            float next_width;
            const auto step = scan_advance(scan_width, p.width_scale, advance,
                candidate.container_width, next_width);
            if (step == ScanStep::unsupported) {
                error = "Wrapped scan width exceeds the finite float domain.";
                return false;
            }
            if (step == ScanStep::overflow) {
                if (key == 0x20) {
                    if (scan == 0) {
                        error = "Native wrapped scan would step before the input string.";
                        return false;
                    }
                    --scan;
                }
                break;
            }
            if (key == 0x20) {
                ++spaces;
                if (!truncate_scan_width(scan_width, saved_width)) {
                    error = "Wrapped saved width is outside the signed 32-bit conversion domain.";
                    return false;
                }
                last_space = scan;
            } else {
                only_spaces = false;
            }
            ++scan;
            scan_width = next_width;
        }
        //00aba4cd..00aba4f4 changes scan/width/count but NOT last_space.
        if (!only_spaces && code_unit(scan) == 0x20) {
            do {
                if (scan == 0 || spaces == 0) {
                    error = "Native wrapped space rewind leaves the supported endpoint domain.";
                    return false;
                }
                --scan;
                --spaces;
                saved_width = signed_bits(static_cast<std::uint32_t>(saved_width) - space_advance);
            } while (code_unit(scan) == 0x20);
        }
        const auto stop = code_unit(scan);
        std::size_t next = scan;
        if (stop != 0) {
            if (spaces == 0 || stop == 0x0a) next = scan + 1;
            else {
                if (last_space == count) {
                    error = "Wrapped last-space endpoint is unavailable.";
                    return false;
                }
                next = last_space + 1;
            }
        }
        if (next < line_begin || next > count || next <= previous_cursor) {
            error = "Native wrapped endpoint would not make forward progress.";
            return false;
        }
        if (saved_width == 0 || stop == 0 || stop == 0x0a) {
            if (!truncate_scan_width(scan_width, saved_width)) {
                error = "Wrapped alignment width is outside the signed 32-bit conversion domain.";
                return false;
            }
        }
        FontWrappedLine line;
        line.first_placement = candidate.placements.size();
        line.placement_count = next - line_begin;
        line.y = y;
        float space_step;
        if (!line_alignment(candidate.container_width, saved_width, spaces,
            stop == 0 || stop == 0x0a, p.horizontal_alignment, space_advance,
            line.initial_x, space_step)) {
            error = p.horizontal_alignment == 3 && spaces == 1 && stop != 0 && stop != 0x0a
                ? "Native justified soft wrap divides by zero with one accepted space."
                : "Wrapped alignment exceeds the finite float domain.";
            return false;
        }
        float x = line.initial_x;
        if (line.placement_count != 0) {
            float bottom;
            if (!line_bottom(y, height, bottom)) {
                error = "Wrapped line bottom exceeds the finite float domain.";
                return false;
            }
            if (!greater_than(y, minimum_y)) minimum_y = y;
            if (!greater_than(maximum_y, bottom)) maximum_y = bottom;
            for (; cursor != next; ++cursor) {
                const auto key = code_unit(cursor);
                const auto advance = select_font_glyph_00ad4480(font, key).scaled_field_12;
                candidate.placements.push_back({key, x, y});
                if (!advance_emission(x, line.emitted_width, advance, key == 0x20,
                    p.width_scale, space_step, x, line.emitted_width)) {
                    error = "Wrapped emitted pen or width exceeds the finite float domain.";
                    return false;
                }
            }
        }
        if (greater_than(line.emitted_width, candidate.measured_width))
            candidate.measured_width = line.emitted_width;
        candidate.lines.push_back(line);
        if (!advance_line(y, height, p.distance_between_lines, y)) {
            error = "Wrapped line step exceeds the finite float domain.";
            return false;
        }
    }
    if (candidate.placements.empty()) {
        error = "Text without emitted glyphs uses an unsupported native sentinel extent.";
        return false;
    }
    if (!vertical_metrics(minimum_y, maximum_y, p.normalized_height,
        p.vertical_scale, p.vertical_alignment, candidate.measured_height,
        candidate.normalized_height, candidate.normalized_vertical_offset)) {
        error = "Wrapped height or vertical alignment exceeds the finite float domain.";
        return false;
    }
    output = std::move(candidate);
    return true;
}

bool apply_font_wrapped_vertical_offset_00aba860_fragment(float normalized_y,
    float vertical_offset, float& output) noexcept {
    if (!finite_float(normalized_y) || !finite_float(vertical_offset)) return false;
    float spill, result;
    __asm {
        fld normalized_y
        fadd vertical_offset
        fstp spill
        fld spill
        fstp result
    }
    if (!finite_float(result)) return false;
    std::memcpy(&output, &result, sizeof(result));
    return true;
}
}
