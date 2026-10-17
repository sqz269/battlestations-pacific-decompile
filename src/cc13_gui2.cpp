// cc13_gui2: GUI Text geometry update and glyph-layout builders (packet
// cc13-gui2, 2026-10-17). Evidence, ABI and uncertainty per routine: the
// comments below and docs/CC13_GUI2.md. Raw native objects; every callee is the
// named hook in include/bsp/cc13_gui2.hpp, invoked at the binary's call point.
#include "bsp/cc13_gui2.hpp"

#include <cstring>
#include <xmmintrin.h>

namespace bsp::cc13_gui2 {
namespace {

template <class T>
T& at(void* base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(base) + offset);
}

constexpr std::uint16_t kSpace = 0x20;
constexpr std::uint16_t kLineFeed = 0x0A;

// FNSTCW; FLD text+20h; FMUL double 960.0 [00CEC380]; FLDCW (saved | 0C00h);
// FISTP qword; FLDCW saved. The caller keeps the low DWORD.
std::uint32_t container_width_960(float normalized_width) noexcept {
    const double scale = 960.0;
    std::uint16_t saved_control;
    std::uint16_t truncating_control;
    std::int64_t converted;
    __asm {
        fnstcw saved_control
        fld normalized_width
        fmul scale
        movzx eax, saved_control
        or eax, 0x0c00
        mov truncating_control, ax
        fldcw truncating_control
        fistp converted
        fldcw saved_control
    }
    return static_cast<std::uint32_t>(converted);
}

// CVTTSS2SI (out-of-range and NaN give 80000000h, as the instruction does).
std::uint32_t truncate_ss(float value) noexcept {
    return static_cast<std::uint32_t>(_mm_cvtt_ss2si(_mm_set_ss(value)));
}

const std::uint16_t* text_or_empty(GuiTextGeometryHooks& host, void* text) {
    const auto* const data = at<const std::uint16_t*>(text, 0xF0);
    return data != nullptr ? data : host.empty_wide_00f8be50();
}

// The glyph advance at glyph+12h, unsigned 16 bits (MOVZX then FILD DWORD).
double advance_of(void* glyph) noexcept {
    return static_cast<double>(at<std::uint16_t>(glyph, 0x12));
}

// The first-glyph binding both builders perform once: section+20h material
// slot 0 <- glyph+18h, slot 1 <- glyph+1Ch (section+20h re-read for the
// second call), then text+18Ch <- the current pen.
void bind_first_glyph(GuiTextGeometryHooks& host, void* text, void* section, void* glyph, float pen) {
    host.material_set_texture_slot_00b189f0(at<void*>(section, 0x20), 0, at<void*>(glyph, 0x18));
    host.material_set_texture_slot_00b189f0(at<void*>(section, 0x20), 1, at<void*>(glyph, 0x1C));
    at<float>(text, 0x18C) = pen;
}

// One shader-parameter registration: 0041E870 into the stack temporary, the
// register call, then the temporary's release (length+1 bytes) when it owns a
// block.
template <class Register>
void register_parameter(GuiTextGeometryHooks& host, const char* name, Register register_call) {
    NativeStringHeader temporary;
    host.native_string_assign_0041e870(&temporary, name);
    register_call(&temporary);
    if (temporary.data != nullptr) {
        void* const pool = host.sized_storage_pool_00419cc0();
        host.pool_return_block_00bd1510(pool, temporary.data, temporary.length + 1, 1);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 00AB9FD0 BSP_TextContext_BuildSingleLineGlyphGeometry. __thiscall(ECX =
// text; source, section) RET 8, frame SUB ESP,38h + four saves (S below = ESP
// after them; arg1 S+4Ch, arg2 S+50h). arg1 is never read: the routine lays
// out text+0F0h. Locals: S+13h first-glyph flag, S+14h first vertex, S+18h
// pen, S+1Ch index pointer, S+20h quad index, S+28h width, S+30h font height
// (MOVZX font+14h), S+34h index stream, S+38h geometry, and the float[3]
// position at S+3Ch (y and z zeroed once before the emit loop at
// 00ABA168/00ABA16E, x stored per glyph at 00ABA1F2). 00AB98F0 receives
// S+3Ch for slots 2, 5 and 6 (LEA at 00ABA1DE/1E3/1EA checked against ESP
// after each push).
// ---------------------------------------------------------------------------
void text_build_single_line_geometry_00ab9fd0(GuiTextGeometryHooks& host, void* text,
    const NativeWideStringHeader* source, void* section) {
    static_cast<void>(source);  // never read by the image
    at<std::uint32_t>(text, 0x110) = 0;
    void* const geometry = host.generated_model_get_geometry_00b74640(at<void*>(text, 0x4C), 0);
    void* const vertex_stream = host.geometry_get_vertex_stream_00b73260(geometry, 0);
    host.vertex_stream_vslot10(vertex_stream, at<std::uint32_t>(text, 0xEC) * 4, 0, 0);
    void* const index_stream = at<void*>(geometry, 0x60);
    std::uint16_t* indices = host.index_stream_vslot0c(index_stream, at<std::uint32_t>(text, 0xEC) * 6, 0, 0);
    at<std::uint32_t>(section, 0x10) = at<std::uint32_t>(text, 0xEC) * 4;
    at<std::uint32_t>(section, 0x18) = at<std::uint32_t>(text, 0xEC) * 2;

    at<float>(text, 0x114) = 0.0f;
    const std::uint32_t font_height = at<std::uint16_t>(at<void*>(text, 0x108), 0x14);
    float pen = 0.0f;
    bool first_glyph = true;
    const std::uint32_t width = container_width_960(at<float>(text, 0x20));
    std::uint32_t first_vertex = 0;
    std::uint32_t quad_index = 0;

    // Measure: text+114h accumulates advance * text+1D8h, stored per unit.
    for (const std::uint16_t* unit = text_or_empty(host, text); *unit != 0; ++unit) {
        void* const glyph = host.font_select_glyph_00ad4480(at<void*>(text, 0x108), *unit);
        at<float>(text, 0x114) = static_cast<float>(
            advance_of(glyph) * static_cast<double>(at<float>(text, 0x1D8)) +
            static_cast<double>(at<float>(text, 0x114)));
    }
    // Alignment text+100h: 2 right, 1 centre (x 0.5 double [00D7A280]), any
    // other value keeps pen 0. The width is FILD + 2^32 [00CE3978] when
    // negative, i.e. the unsigned value.
    const std::int32_t alignment = at<std::int32_t>(text, 0x100);
    if (alignment == 2) {
        pen = static_cast<float>(static_cast<double>(width) - static_cast<double>(at<float>(text, 0x114)));
    } else if (alignment == 1) {
        pen = static_cast<float>(
            (static_cast<double>(width) - static_cast<double>(at<float>(text, 0x114))) * 0.5);
    }

    const std::uint16_t* unit = text_or_empty(host, text);
    if (*unit != 0) {
        float position[3];
        position[1] = 0.0f;
        position[2] = 0.0f;
        do {
            void* const glyph = host.font_select_glyph_00ad4480(at<void*>(text, 0x108), *unit);
            if (first_glyph) {
                bind_first_glyph(host, text, section, glyph, pen);
                first_glyph = false;
            }
            position[0] = pen;
            host.text_write_glyph_quad_00ab98f0(text, glyph, position, vertex_stream, indices, position,
                position, quad_index, font_height, first_vertex, *unit);
            first_vertex += 4;
            indices += 6;
            ++quad_index;
            pen = static_cast<float>(advance_of(glyph) * static_cast<double>(at<float>(text, 0x1D8)) +
                static_cast<double>(pen));
            ++unit;
        } while (*unit != 0);
    }
    host.index_stream_vslot10(index_stream);
    host.vertex_stream_vslot14(vertex_stream);
    host.draw_section_rebuild_vertex_layout_00b865a0(section, geometry);
}

// ---------------------------------------------------------------------------
// 00ABA270 BSP_TextContext_BuildGlyphGeometry (the wrapped builder).
// __thiscall(ECX = text; source, section) RET 8, frame SUB ESP,64h + four
// saves (S = ESP after them; arg1 S+78h, arg2 S+7Ch). Locals: S+13h
// first-glyph flag, S+14h pen, S+18h space count, S+1Ch line advance sum,
// S+20h line top y, S+24h the space glyph, S+28h space step, S+2Ch line
// width, S+30h container width, S+34h quad index, S+38h index pointer, S+3Ch
// max y (-1e10 [00CE4ADC]), S+40h min y (1e10 [00CE4970]), S+44h last space /
// line bottom, S+48h first vertex, S+4Ch font height as float, S+50h vertex
// stream, S+54h..5Ch read-back float[3], S+60h font height (MOVSX font+14h),
// S+64h index stream, S+68h the float[3] glyph position, S+78h the
// all-spaces flag and then the line end (arg1's slot), S+7Ch the vertical
// offset (arg2's slot, after EBP took the section).
// The space step S+28h is written only by alignments 0..3; any other value of
// text+100h reads it uninitialised on the first line (the image reads a stale
// stack slot). This reconstruction starts it at 0; that is the one deviation.
// ---------------------------------------------------------------------------
void text_build_wrapped_geometry_00aba270(GuiTextGeometryHooks& host, void* text,
    const NativeWideStringHeader* source, void* section) {
    host.text_clear_glyph_children_00ab80c0(text);
    const std::int32_t font_height = at<std::int16_t>(at<void*>(text, 0x108), 0x14);
    const float font_height_f = static_cast<float>(font_height);  // CVTSI2SS
    auto* const own = &at<NativeWideStringHeader>(text, 0xEC);
    if (own != source) {
        host.wide_string_resize_004c53e0(own, source->length, 1);
        if (source->length != 0) {
            const std::uint16_t* from = source->data;
            std::uint16_t* to = own->data;
            std::uint16_t copied;
            do {
                copied = *from++;
                *to++ = copied;
            } while (copied != 0);
        }
    }
    void* const geometry = host.generated_model_get_geometry_00b74640(at<void*>(text, 0x4C), 0);
    void* const vertex_stream = host.geometry_get_vertex_stream_00b73260(geometry, 0);
    host.vertex_stream_vslot10(vertex_stream, at<std::uint32_t>(text, 0xEC) * 4, 0, 0);
    std::uint32_t first_vertex = 0;
    void* const index_stream = at<void*>(geometry, 0x60);
    std::uint16_t* indices = host.index_stream_vslot0c(index_stream, at<std::uint32_t>(text, 0xEC) * 6, 0, 0);
    at<std::uint32_t>(section, 0x10) = at<std::uint32_t>(text, 0xEC) * 4;
    at<std::uint32_t>(section, 0x18) = at<std::uint32_t>(text, 0xEC) * 2;

    const std::uint32_t width = container_width_960(at<float>(text, 0x20));
    at<std::uint32_t>(text, 0x110) = 0;
    std::uint32_t quad_index = 0;
    float pen = 0.0f;
    float line_top = 0.0f;
    bool first_glyph = true;
    void* const space_glyph = host.font_select_glyph_00ad4480(at<void*>(text, 0x108), kSpace);
    float min_y = 1e10f;
    float max_y = -1e10f;
    float space_step = 0.0f;  // see the header comment: uninitialised in the image
    float position[3];

    const std::uint16_t* line = text_or_empty(host, text);
    if (*line != 0) {
        do {
            // 00ABA3E3: skip the line's leading spaces, except left-aligned
            // text at the very start or right after a line feed.
            if (*line == kSpace) {
                bool skip = true;
                if (at<std::int32_t>(text, 0x100) == 0) {
                    if (line == text_or_empty(host, text))
                        skip = false;
                    else if (line != text_or_empty(host, text) && line[-1] == kLineFeed)
                        skip = false;
                }
                if (skip) {
                    do {
                        ++line;
                    } while (*line == kSpace);
                }
            }

            // 00ABA429: measure until a line feed, the end, or an overflow.
            float line_width = 0.0f;
            std::uint32_t break_width = 0;  // EBP
            std::int32_t spaces = 0;
            bool only_spaces = true;
            const std::uint16_t* last_space = nullptr;
            const std::uint16_t* scan = line;
            if (*line != 0) {
                for (;;) {
                    if (*scan == kLineFeed) break;
                    void* const glyph = host.font_select_glyph_00ad4480(at<void*>(text, 0x108), *scan);
                    const double candidate = static_cast<double>(at<float>(text, 0x1D8)) * advance_of(glyph) +
                        static_cast<double>(line_width);
                    // FCOMI candidate, width; JA: ordered and above.
                    if (candidate > static_cast<double>(width)) {
                        if (*scan == kSpace) --scan;
                        break;
                    }
                    if (*scan == kSpace) {
                        ++spaces;
                        break_width = truncate_ss(line_width);
                        last_space = scan;
                    } else {
                        only_spaces = false;
                    }
                    ++scan;
                    line_width = static_cast<float>(candidate);
                    if (*scan == 0) break;
                }
                // 00ABA4CD: back over trailing spaces, subtracting the raw
                // (unscaled) space advance from the break width each time.
                if (!only_spaces && *scan == kSpace) {
                    const std::uint32_t space_advance = at<std::uint16_t>(space_glyph, 0x12);
                    std::int32_t remaining = spaces;
                    do {
                        --scan;
                        --remaining;
                        break_width -= space_advance;
                    } while (*scan == kSpace);
                    spaces = remaining;
                }
            }

            // 00ABA4FB: the line end.
            const std::uint16_t stop = *scan;
            const std::uint16_t* line_end;
            if (stop == 0)
                line_end = scan;
            else if (spaces == 0 || stop == kLineFeed)
                line_end = scan + 1;
            else
                line_end = last_space + 1;
            if (break_width == 0 || stop == 0 || stop == kLineFeed)
                break_width = truncate_ss(line_width);

            // 00ABA53A: alignment text+100h; 3 justifies when the line was
            // broken by overflow. The free width is unsigned (FILD + 2^32).
            const std::int32_t alignment = at<std::int32_t>(text, 0x100);
            const float space_advance_f =
                static_cast<float>(static_cast<std::int32_t>(at<std::uint16_t>(space_glyph, 0x12)));
            const double free_width = static_cast<double>(width - break_width);
            if (alignment == 0 || (alignment == 3 && (stop == 0 || stop == kLineFeed))) {
                space_step = space_advance_f;
                pen = 0.0f;
            } else if (alignment == 3) {
                if (spaces != 0) {
                    // FIDIV by (spaces - 1) with no zero guard, then FADDP of
                    // the raw space advance.
                    space_step = static_cast<float>(free_width / static_cast<double>(spaces - 1) +
                        static_cast<double>(at<std::uint16_t>(space_glyph, 0x12)));
                } else {
                    space_step = 0.0f;
                }
                pen = 0.0f;
            } else if (alignment == 2) {
                space_step = space_advance_f;
                pen = static_cast<float>(free_width);
            } else if (alignment == 1) {
                space_step = space_advance_f;
                pen = static_cast<float>(free_width * 0.5);
            }
            // Any other alignment keeps space_step and pen from the last line.

            float line_sum = 0.0f;
            if (line != line_end) {
                const float line_bottom =
                    static_cast<float>(static_cast<double>(line_top) + static_cast<double>(font_height_f));
                position[1] = line_top;
                position[2] = 0.0f;
                std::uint32_t vertex = first_vertex;
                do {
                    void* const glyph = host.font_select_glyph_00ad4480(at<void*>(text, 0x108), *line);
                    if (first_glyph) {
                        bind_first_glyph(host, text, section, glyph, pen);
                        first_glyph = false;
                    }
                    // FCOMIP then JA: assign unless strictly ordered-greater,
                    // so a NaN operand assigns.
                    if (!(line_top > min_y)) min_y = line_top;
                    if (!(max_y > line_bottom)) max_y = line_bottom;
                    position[0] = pen;
                    host.text_write_glyph_quad_00ab98f0(text, glyph, position, vertex_stream, indices, position,
                        position, quad_index, static_cast<std::uint32_t>(font_height), vertex, *line);
                    indices += 6;
                    vertex += 4;
                    if (*line != kSpace) {
                        // D8 C9 FMUL ST0,ST1 at 00ABA755: pen gets advance *
                        // text+1D8h, the line sum gets the raw advance.
                        const double advance = advance_of(glyph);
                        pen = static_cast<float>(static_cast<double>(at<float>(text, 0x1D8)) * advance +
                            static_cast<double>(pen));
                        line_sum = static_cast<float>(advance + static_cast<double>(line_sum));
                    } else {
                        pen = static_cast<float>(static_cast<double>(space_step) + static_cast<double>(pen));
                        line_sum = static_cast<float>(static_cast<double>(space_step) + static_cast<double>(line_sum));
                    }
                    ++quad_index;
                    ++line;
                } while (line != line_end);
                first_vertex = vertex;
            }
            // 00ABA785: FCOMIP then JBE: only an ordered-greater sum replaces
            // text+114h.
            if (line_sum > at<float>(text, 0x114)) at<float>(text, 0x114) = line_sum;
            at<std::uint32_t>(text, 0x110) += 1;
            // ((text+10Ch + 1.0 [00D7A210]) - 0.15000000596 [00CE6618]) * height + top.
            line_top = static_cast<float>(
                ((static_cast<double>(at<float>(text, 0x10C)) + 1.0) - 0.15000000596046448) *
                    static_cast<double>(font_height_f) +
                static_cast<double>(line_top));
        } while (*line != 0);
    }

    // 00ABA7DC: block height and vertical alignment text+104h (1 centre, 2
    // bottom, else no offset). 720.0 [00CEF1B8], 0.75f [00E12FD4], DE CA FMULP
    // ST2 at 00ABA812 checked from the bytes.
    const float span = static_cast<float>(static_cast<double>(max_y) - static_cast<double>(min_y));
    at<float>(text, 0x178) = span;
    const float raised = static_cast<float>(static_cast<double>(span) / 720.0 * static_cast<double>(0.75f));
    float offset = 0.0f;
    const std::int32_t vertical = at<std::int32_t>(text, 0x104);
    if (vertical == 1) {
        offset = static_cast<float>(
            (static_cast<double>(0.75f) * static_cast<double>(at<float>(text, 0x24)) - static_cast<double>(raised)) *
            0.5);
    } else if (vertical == 2) {
        offset = static_cast<float>(
            static_cast<double>(0.75f) * static_cast<double>(at<float>(text, 0x24)) - static_cast<double>(raised));
    }

    // 00ABA843: section counts from the low 16 bits of the quad index, then
    // every vertex's y moved by the offset in place.
    const std::uint32_t quads = static_cast<std::uint16_t>(quad_index);
    at<std::uint32_t>(section, 0x10) = quads * 4;
    at<std::uint32_t>(section, 0x18) = quads * 2;
    for (std::uint32_t index = 0; index < at<std::uint32_t>(section, 0x10); ++index) {
        float read_back[3];
        host.logical_vertex_stream_read_position_004768d0(vertex_stream, read_back, index);
        read_back[1] = static_cast<float>(static_cast<double>(read_back[1]) + static_cast<double>(offset));
        auto* const target = reinterpret_cast<float*>(at<unsigned char*>(vertex_stream, 0x08) +
            at<std::uint32_t>(vertex_stream, 0x10) + at<std::uint32_t>(vertex_stream, 0x0C) * index);
        target[0] = read_back[0];
        target[1] = read_back[1];
        target[2] = read_back[2];
    }
    host.index_stream_vslot10(index_stream);
    host.vertex_stream_vslot14(vertex_stream);
}

// ---------------------------------------------------------------------------
// 00ABA8D0 BSP_TextContext_UpdateUtf16Geometry. __thiscall(ECX = text;
// source) RET 4, SEH 00CB8130, SUB ESP,1Ch. With S = ESP after PUSH EBP/ESI:
// the copied wide string at S+10h, EH state S+2Ch, arg1 S+34h; after PUSH
// EBX/EDI (S' = S-8): the main geometry S'+10h, the material S'+14h, the
// parameter-name temporary S'+20h, the shader-changed byte in arg1's slot
// S'+3Ch (later the font height and its scaled float).
// ---------------------------------------------------------------------------
void text_update_utf16_geometry_00aba8d0(GuiTextGeometryHooks& host, void* text,
    const NativeWideStringHeader* source) {
    host.text_ensure_draw_sections_00ab8530(text);
    NativeWideStringHeader copy;
    host.wide_string_copy_construct_004c8dd0(&copy, source);
    if (at<std::uint8_t>(at<void*>(text, 0x108), 0x48) != 0)
        host.uppercase_utf16_in_place_00a9ec30(host.locale_table_00f8bc4c(), &copy);

    // 00ABA921: unchanged text (case-insensitive) skips everything.
    bool changed;
    if (at<std::uint32_t>(text, 0xEC) == 0)
        changed = copy.length != 0;
    else if (copy.length == 0)
        changed = true;
    else
        changed = host.wcsicmp_00c03a39(at<const std::uint16_t*>(text, 0xF0), copy.data) != 0;

    if (changed) {
        at<float>(text, 0x114) = 0.0f;
        host.text_clear_glyph_children_00ab80c0(text);
        void* const geometry = host.generated_model_get_geometry_00b74640(at<void*>(text, 0x4C), 0);
        void* const section = host.mesh_get_element_00b732c0(geometry, 0);
        host.wide_string_assign_004c5e20(&at<NativeWideStringHeader>(text, 0xEC), &copy);
        if (copy.length == 0) {
            at<std::uint32_t>(section, 0x18) = 0;
            at<std::uint32_t>(section, 0x10) = 0;
            void* const shadow_geometry = host.generated_model_get_geometry_00b74640(at<void*>(text, 0x188), 0);
            void* const shadow_section = host.mesh_get_element_00b732c0(shadow_geometry, 0);
            at<std::uint32_t>(shadow_section, 0x18) = 0;
            at<std::uint32_t>(shadow_section, 0x10) = 0;
        } else {
            host.text_create_glyph_buffers_00ab8400(text, copy.length, geometry);
            void* const material = at<void*>(section, 0x20);
            at<std::uint32_t>(section, 0x08) = 4;
            at<std::uint32_t>(section, 0x0C) = 0;
            at<std::uint32_t>(section, 0x14) = 0;
            at<std::uint32_t>(section, 0x10) = 0;
            at<std::uint32_t>(section, 0x18) = 0;
            const bool shader_changed = host.text_ensure_font_shader_00ab8ce0(text);
            if (shader_changed) {
                host.material_set_shader_00b19210(material, at<void*>(text, 0x1EC));
                at<float>(text, 0x1DC) = at<float>(text, 0x94);
                at<float>(text, 0x1E0) = at<float>(text, 0x1D4);
                register_parameter(host, "cOverbrightAlpha", [&](const NativeStringHeader* name) {
                    host.material_register_float2_00b18b00(material, name, &at<float>(text, 0x1DC));
                });
                register_parameter(host, "cLowColor", [&](const NativeStringHeader* name) {
                    host.material_register_float4_00b18aa0(material, name, &at<float>(text, 0xA4));
                });
                register_parameter(host, "cHighColor", [&](const NativeStringHeader* name) {
                    host.material_register_float4_00b18aa0(material, name, &at<float>(text, 0xB4));
                });
                register_parameter(host, "cBlendFactor", [&](const NativeStringHeader* name) {
                    host.material_register_float_00b18b20(material, name, &at<float>(text, 0xC4));
                });
                host.ui_context_register_clip_parameters_00aa9f10(text, material);
            }
            host.draw_section_rebuild_vertex_layout_00b865a0(section, geometry);
            if (at<std::uint8_t>(text, 0xFC) != 0)
                text_build_wrapped_geometry_00aba270(host, text, &copy, section);
            else
                text_build_single_line_geometry_00ab9fd0(host, text, &copy, section);
            host.text_vslot50(text, &at<unsigned char>(text, 0x50));

            // 00ABAB8A: the shadow (text+188h) shares the main streams and
            // the section counts, then mirrors texture slot 0 and the shader.
            void* const shadow_geometry = host.generated_model_get_geometry_00b74640(at<void*>(text, 0x188), 0);
            void* const shadow_section = host.mesh_get_element_00b732c0(shadow_geometry, 0);
            void* const main_stream = host.geometry_get_vertex_stream_00b73260(geometry, 0);
            host.geometry_set_vertex_stream_00b73bb0(shadow_geometry, 0, main_stream);
            host.geometry_set_index_stream_00b73b70(shadow_geometry, at<void*>(geometry, 0x60));
            at<std::uint32_t>(shadow_section, 0x08) = at<std::uint32_t>(section, 0x08);
            at<std::uint32_t>(shadow_section, 0x0C) = at<std::uint32_t>(section, 0x0C);
            at<std::uint32_t>(shadow_section, 0x14) = at<std::uint32_t>(section, 0x14);
            at<std::uint32_t>(shadow_section, 0x10) = at<std::uint32_t>(section, 0x10);
            at<std::uint32_t>(shadow_section, 0x18) = at<std::uint32_t>(section, 0x18);
            host.draw_section_rebuild_vertex_layout_00b865a0(shadow_section, shadow_geometry);
            void* const shadow_material = at<void*>(shadow_section, 0x20);
            // Signed 16-bit slot count at material+34h (CMP word, JLE).
            void* const texture0 = at<std::int16_t>(material, 0x34) > 0 ? at<void*>(material, 0x10) : nullptr;
            host.material_set_texture_slot_00b189f0(shadow_material, 0, texture0);
            if (shader_changed) {
                host.material_set_shader_00b19210(shadow_material, at<void*>(material, 0x7C));
                host.ui_context_register_clip_parameters_00aa9f10(text, shadow_material);
                register_parameter(host, "cBlendFactor", [&](const NativeStringHeader* name) {
                    host.material_register_float_00b18b20(shadow_material, name, host.zero_block_00f8be54());
                });
                register_parameter(host, "cOverbrightAlpha", [&](const NativeStringHeader* name) {
                    host.material_register_float2_00b18b00(shadow_material, name, &at<float>(text, 0x1DC));
                });
                register_parameter(host, "cLowColor", [&](const NativeStringHeader* name) {
                    host.material_register_float4_00b18aa0(shadow_material, name, &at<float>(text, 0xA4));
                });
                register_parameter(host, "cHighColor", [&](const NativeStringHeader* name) {
                    host.material_register_float4_00b18aa0(shadow_material, name, &at<float>(text, 0xB4));
                });
                register_parameter(host, "cBlendFactor", [&](const NativeStringHeader* name) {
                    host.material_register_float_00b18b20(shadow_material, name, &at<float>(text, 0xC4));
                });
            }

            // 00ABAD99: shadow colour from text+168h..174h, scale and depth.
            void* const colour_geometry = host.generated_model_get_geometry_00b74640(at<void*>(text, 0x188), 0);
            void* const colour_section = host.mesh_get_element_00b732c0(colour_geometry, 0);
            float* const colour = host.material_get_diffuse_color_00b179f0(at<void*>(colour_section, 0x20), 0);
            reinterpret_cast<std::uint32_t*>(colour)[0] = at<std::uint32_t>(text, 0x168);
            reinterpret_cast<std::uint32_t*>(colour)[1] = at<std::uint32_t>(text, 0x16C);
            reinterpret_cast<std::uint32_t*>(colour)[2] = at<std::uint32_t>(text, 0x170);
            reinterpret_cast<std::uint32_t*>(colour)[3] = at<std::uint32_t>(text, 0x174);
            void* const font = at<void*>(text, 0x108);
            const std::int32_t height = font != nullptr ? at<std::int16_t>(font, 0x14) : 0;
            // FILD; FDIV double 720.0 [00CEF1B8]; FMUL float text+164h; FSTP float.
            const float scale = static_cast<float>(
                static_cast<double>(height) / 720.0 * static_cast<double>(at<float>(text, 0x164)));
            // text+160h zero -> 0.5f [00CE3800], else -0.5f [00CE69D0].
            const float depth = at<std::int32_t>(text, 0x160) == 0 ? 0.5f : -0.5f;
            const float shadow_scale[3] = {scale, scale, depth};
            host.model_vslot2c(at<void*>(text, 0x188), shadow_scale);
            void* const parent = at<std::uint8_t>(text, 0x15C) != 0 ? at<void*>(text, 0x4C) : nullptr;
            host.node_set_parent_00b6e680(at<void*>(text, 0x188), parent);
            if (at<std::uint8_t>(text, 0x15C) == 0)
                host.node_propagate_root_registration_00b6d890(at<void*>(text, 0x188), nullptr);
        }
    }
    if (copy.data != nullptr) {
        void* const pool = host.sized_storage_pool_00419cc0();
        host.pool_return_block_00bd1510(pool, copy.data, copy.length * 2 + 2, 1);
    }
}

// ---------------------------------------------------------------------------
// 00AA8610 BSP_GuiWidget_SetAlphaRecursive. __thiscall(ECX = widget; float
// alpha) RET 4; alpha at [ESP+10h] after the three saves. +5Ch = alpha; when
// the model +4Ch has geometry and geometry+58h is nonzero, element 0's
// material diffuse colour +0Ch = alpha (00B74640 is called afresh for the
// element). Then every child of the VS2008 std::list at +64h (proxy +64h,
// head +68h; node +0 next, +8 value) gets the same alpha, recursively. The
// checked iterator's container compare (CMP EDI,EDI at 00AA8675) is always
// equal; the two end-of-list checks around the call compare the node with
// the head re-read from +68h and report through 00BF6713, as the image does.
// ---------------------------------------------------------------------------
void gui_widget_set_alpha_recursive_00aa8610(GuiWidgetAlphaHooks& host, void* widget, float alpha) {
    at<float>(widget, 0x5C) = alpha;
    if (host.generated_model_has_geometry_00b74650(at<void*>(widget, 0x4C)) != 0) {
        void* const geometry = host.generated_model_get_geometry_00b74640(at<void*>(widget, 0x4C), 0);
        if (host.mesh_get_count58_00b72b40(geometry) != 0) {
            void* const element_geometry = host.generated_model_get_geometry_00b74640(at<void*>(widget, 0x4C), 0);
            void* const section = host.mesh_get_element_00b732c0(element_geometry, 0);
            float* const colour = host.material_get_diffuse_color_00b179f0(at<void*>(section, 0x20), 0);
            colour[3] = alpha;
        }
    }
    void* const head = at<void*>(widget, 0x68);
    for (void* node = at<void*>(head, 0); node != head; node = at<void*>(node, 0)) {
        if (node == at<void*>(widget, 0x68)) host.invalid_parameter_00bf6713();
        gui_widget_set_alpha_recursive_00aa8610(host, at<void*>(node, 8), alpha);
        if (node == at<void*>(widget, 0x68)) host.invalid_parameter_00bf6713();
    }
}

namespace {

NativeVariantPair pair_of(std::uint32_t tag, const void* pointer) noexcept {
    return {tag, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer))};
}
NativeVariantPair pair_of_float(std::uint32_t tag, float value) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return {tag, bits};
}

void release_narrow(cc13_gui1::NativePoolHooks& host, const NativeStringHeader& s) {
    if (s.data != nullptr) {
        void* const pool = host.sized_storage_pool_00419cc0();
        host.pool_return_block_00bd1510(pool, s.data, s.length + 1, 1);
    }
}

// The guarded first-use initialisation of the two static colours.
float* white_colour(GuiWidgetReadPropertiesHooks& host) {
    std::uint32_t* const guard = host.white_guard_00f8bcf0();
    float* const white = host.white_00f8bce0();
    if ((*guard & 1) == 0) {
        *guard |= 1;
        white[0] = 1.0f;
        white[1] = 1.0f;
        white[2] = 1.0f;
        white[3] = 1.0f;
    }
    return white;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00AAA710 BSP_GuiWidget_ReadPropertiesAndChildren. __thiscall(ECX = widget;
// visitor) RET 4, SEH 00CB7571, SUB ESP,7F8h; S = ESP after the four saves
// (visitor S+818h, EH state S+810h). Each property is visitor->vtable[+0Ch]
// with three by-value pairs built in the outgoing area (fallback at S-8,
// field S-10h, key S-18h, checked against ESP after each SUB). Locals: Pos
// default float3 S+28h, the reused float2 / NativeString header S+14h..18h,
// the X spill S+1Ch, the child block S+34h..807h, the child name temporary
// S+20h, the node pointer S+10h, the list head S+2Ch.
// The key strings are this file's literals with the image's text ("Pos" at
// 00CF168C, "Size" 00CFF278, "Pivot" 00D5C228, "Scale" 00CE60AC, "Rotate"
// 00D5C220, "Color" 00CE93F8, "LowColor" 00D5C214, "HighColor" 00D5C208,
// "BlendFactor" 00D5C1FC, "WideScreenAlign" 00D5C1EC, "Left" 00CE92E4, "Right"
// 00CE92DC, "Visible" 00D5C1E4, "MouseBlock" 00D5C1D8, "MouseHit" 00D5C1CC,
// "" 00CE3A0C), not the image addresses. The bool fallbacks (tag 3) store only
// a byte (MOV byte [pair+4]); the other three payload bytes are stale stack
// in the image and zero here.
// ---------------------------------------------------------------------------
void gui_widget_read_properties_and_children_00aaa710(GuiWidgetReadPropertiesHooks& host, void* widget,
    void* visitor) {
    auto* const w = static_cast<unsigned char*>(widget);
    float pos_default[3] = {0.0f, 0.0f, 0.0f};
    host.visitor_vslot0c(visitor, pair_of(0, "Pos"), pair_of(5, w + 0x0C), pair_of(5, pos_default));
    float vec2[2] = {0.0f, 0.0f};
    host.visitor_vslot0c(visitor, pair_of(0, "Size"), pair_of(6, w + 0x20), pair_of(6, vec2));
    vec2[0] = 0.0f;
    vec2[1] = 0.0f;
    host.visitor_vslot0c(visitor, pair_of(0, "Pivot"), pair_of(6, w + 0x18), pair_of(6, vec2));
    vec2[0] = 1.0f;  // [00D7A24C]
    vec2[1] = 1.0f;
    host.visitor_vslot0c(visitor, pair_of(0, "Scale"), pair_of(6, w + 0x28), pair_of(6, vec2));
    host.visitor_vslot0c(visitor, pair_of(0, "Rotate"), pair_of(2, w + 0x48), pair_of_float(2, 0.0f));
    host.visitor_vslot0c(visitor, pair_of(0, "Color"), pair_of(8, w + 0x50), pair_of(8, white_colour(host)));
    {
        std::uint32_t* const guard = host.black_guard_00f8bcdc();
        float* const black = host.black_00f8bccc();
        if ((*guard & 1) == 0) {
            *guard |= 1;
            black[0] = 0.0f;
            black[1] = 0.0f;
            black[2] = 0.0f;
            black[3] = 1.0f;
        }
        host.visitor_vslot0c(visitor, pair_of(0, "LowColor"), pair_of(8, w + 0xA4), pair_of(8, black));
    }
    host.visitor_vslot0c(visitor, pair_of(0, "HighColor"), pair_of(8, w + 0xB4), pair_of(8, white_colour(host)));
    host.visitor_vslot0c(visitor, pair_of(0, "BlendFactor"), pair_of(2, w + 0xC4), pair_of_float(2, 0.0f));

    // WideScreenAlign: "Left" (__stricmp, only for a non-null buffer) -> 1,
    // "Right" (00425850) -> 2, else 0, at +0E0h.
    NativeStringHeader align{0, nullptr};
    host.visitor_vslot0c(visitor, pair_of(0, "WideScreenAlign"), pair_of(0, &align), pair_of(0, ""));
    if (align.data != nullptr && host.stricmp_00bf7fbf(align.data, "Left") == 0)
        at<std::uint32_t>(widget, 0xE0) = 1;
    else if (host.native_string_equals_insensitive_00425850(&align, "Right"))
        at<std::uint32_t>(widget, 0xE0) = 2;
    else
        at<std::uint32_t>(widget, 0xE0) = 0;
    release_narrow(host, align);

    // +8 keeps the authored x; with the platform's widescreen byte set, x at
    // +0Ch moves by 0.14166666567 (double [00D5C118]) left or right.
    const float authored_x = at<float>(widget, 0x0C);
    at<float>(widget, 0x08) = authored_x;
    const std::uint32_t align_mode = at<std::uint32_t>(widget, 0xE0);
    if (align_mode != 0 && at<std::uint8_t>(host.platform_0109cf04(), 0x0D) != 0) {
        if (align_mode == 1)
            at<float>(widget, 0x0C) = static_cast<float>(static_cast<double>(authored_x) - 0.1416666656732559);
        else if (align_mode == 2)
            at<float>(widget, 0x0C) = static_cast<float>(static_cast<double>(authored_x) + 0.1416666656732559);
    }

    if (host.widget_vslot5c(widget) != 1) {
        host.visitor_vslot0c(visitor, pair_of(0, "Visible"), pair_of(3, w + 0xE4), NativeVariantPair{3, 1});
        host.widget_vslot34_dword(widget, at<std::uint8_t>(widget, 0xE4));
    }
    host.visitor_vslot0c(visitor, pair_of(0, "MouseBlock"), pair_of(3, w + 0x84), NativeVariantPair{3, 0});
    if (at<std::uint8_t>(widget, 0x84) != 0)
        at<std::uint8_t>(widget, 0x78) = 1;
    else
        host.visitor_vslot0c(visitor, pair_of(0, "MouseHit"), pair_of(3, w + 0x78), NativeVariantPair{3, 0});
    host.recompose_local_transform_00aa7220(widget);

    // 00AAACD4: children.
    NativeVariantChildBlock block;
    for (NativeVariantPair& entry : block.entries) entry.tag = 0xFFFFFFFFu;
    block.count = 0;
    host.visitor_vslot18(visitor, &block);
    for (std::int32_t index = 0; index < block.count; ++index) {
        const NativeVariantPair* const entry = &block.entries[index];
        if (entry->tag != 0) continue;
        const char* const name = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(entry->payload));
        NativeStringHeader key{0, nullptr};
        std::uint32_t name_length = 0;
        while (name[name_length] != '\0') ++name_length;
        host.native_string_resize_0041dd40(&key, name_length, 1);
        if (key.data != nullptr) host.memcpy_00bf7680(key.data, name, key.length + 1);
        const std::uint32_t type = host.widget_type_for_key_00aa2490(&key);
        if (type != 0) {
            void* const child = cc13_gui1::gui_layout_create_widget_of_type_00aa6560(host, type, nullptr);
            void* const model_block = host.model_pool_allocate_00b74eb0(0x184);
            void* const model =
                model_block != nullptr ? host.generated_model_construct_00b75030(model_block, &key) : nullptr;
            at<void*>(child, 0x4C) = model;
            if (model != nullptr) at<std::uint32_t>(model, 0x138) &= 0xFFFFFFFCu;
            if (at<void*>(child, 0x70) != nullptr) host.detach_child_00aa83a0(at<void*>(child, 0x70), child);
            // Inline std::list insert before end() of +64h (head +68h).
            void* const value = child;
            void* const head = at<void*>(widget, 0x68);
            void* const node = host.list_buy_node_00a9b790(w + 0x64, head, at<void*>(head, 4), &value);
            host.list_incsize_00a9d480(w + 0x64, 1);
            at<void*>(head, 4) = node;
            at<void*>(at<void*>(node, 4), 0) = node;
            at<void*>(child, 0x70) = widget;
            host.scene_node_attach_00b6e680(at<void*>(child, 0x4C), at<void*>(widget, 0x4C));
            host.widget_vslot74(child);
            host.visitor_vslot04(visitor, *entry);
            host.widget_vslot18(child, visitor);
            host.visitor_vslot08(visitor);
            host.widget_vslot78(child);
        }
        release_narrow(host, key);
    }
}

}  // namespace bsp::cc13_gui2
