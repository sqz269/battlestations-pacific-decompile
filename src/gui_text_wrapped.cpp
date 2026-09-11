#include "bsp/gui_text_wrapped.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
std::uint32_t word(const void* object, std::size_t offset) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(object) + offset, 4);
    return value;
}
std::int32_t signed_bits(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, 4);
    return result;
}
std::uint32_t length(const GuiTextWidget& text) noexcept {
    return static_cast<std::uint32_t>(text.text.size());
}
NativeFontResources& current_font(GuiTextLifetime& lifetime, GuiTextWrappedServices& services) {
    auto& font = services.fonts.resolve(lifetime.text().font);
    if (&font.actual_owners() != &services.actual_owners)
        throw std::logic_error("Text font and material require the same actual retained owners");
    return font;
}
bool greater_than(float left, float right) noexcept {
    unsigned char result;
    __asm {
        fld right
        fld left
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
float add(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fadd right
        fstp result
    }
    return result;
}
std::uint32_t container_width(float normalized, std::uint16_t saved_control) noexcept {
    const double multiplier = 960.0;
    const std::uint16_t truncation_control = saved_control | 0x0c00;
    std::int64_t converted;
    __asm {
        fld normalized
        fmul multiplier
        fldcw truncation_control
        fistp converted
        fldcw saved_control
    }
    return static_cast<std::uint32_t>(converted);
}
std::uint32_t truncate_scan(float width) noexcept {
    std::uint32_t result;
    __asm {
        cvttss2si eax, width
        mov result, eax
    }
    return result;
}
bool scan_advance(float previous, float scale, std::uint16_t advance,
    std::uint32_t container, float& output) noexcept {
    const std::int32_t metric = advance;
    const auto signed_container = signed_bits(container);
    const float unsigned_correction = 4294967296.0f;
    float result;
    unsigned int fits;
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
    if (!fits) return false;
    output = result;
    return true;
}
bool align_line(GuiTextWrappedContinuation& frame) noexcept {
    const auto mode = static_cast<std::uint32_t>(frame.lifetime->text().align);
    if (mode > 3) {
        frame.pending = GuiTextWrappedPending::unsupported_alignment;
        return false;
    }
    const auto natural_space = frame.space_glyph->scaled_field_12;
    if (mode == 0 || (mode == 3 && frame.natural_end)) {
        frame.space_step = static_cast<float>(natural_space);
        frame.placement.x = 0.0f;
        return true;
    }
    if (mode == 3 && frame.spaces == 0) {
        frame.space_step = 0.0f;
        frame.placement.x = 0.0f;
        return true;
    }
    const auto slack = signed_bits(frame.container_width - frame.saved_width);
    const auto divisor = signed_bits(frame.spaces - 1u);
    const std::int32_t space_metric = natural_space;
    const float correction = 4294967296.0f;
    const double half = 0.5;
    float result;
    __asm {
        fild slack
        cmp slack, 0
        jge slack_ready
        fadd correction
    slack_ready:
        cmp mode, 3
        jne ordinary_alignment
        fidiv divisor
        fild space_metric
        faddp st(1), st(0)
        jmp alignment_store
    ordinary_alignment:
        cmp mode, 1
        jne alignment_store
        fmul half
    alignment_store:
        fstp result
    }
    if (mode == 3) {
        frame.space_step = result;
        frame.placement.x = 0.0f;
    } else {
        frame.space_step = static_cast<float>(natural_space);
        frame.placement.x = result;
    }
    return true;
}
bool select_line(GuiTextWrappedContinuation& frame) {
    auto& text = frame.lifetime->text();
    if (*frame.text_cursor == u' ' &&
        (text.align != GuiTextAlign::Left ||
            (frame.text_cursor != text.text.c_str() &&
                (frame.text_cursor == text.text.c_str() || frame.text_cursor[-1] != u'\n')))) {
        do { ++frame.text_cursor; } while (*frame.text_cursor == u' ');
    }
    frame.scan_width = 0.0f;
    frame.saved_width = 0;
    frame.spaces = 0;
    bool only_spaces = true;
    const char16_t* last_space = nullptr;
    const char16_t* scan = frame.text_cursor;
    while (*scan && *scan != u'\n') {
        const auto& glyph = current_font(*frame.lifetime, *frame.services).glyph(*scan);
        float accepted;
        if (!scan_advance(frame.scan_width, text.font_scale,
                glyph.scaled_field_12, frame.container_width, accepted)) {
            if (*scan == u' ') --scan;
            break;
        }
        if (*scan == u' ') {
            ++frame.spaces;
            frame.saved_width = truncate_scan(frame.scan_width);
            last_space = scan;
        } else only_spaces = false;
        ++scan;
        frame.scan_width = accepted;
    }
    if (!only_spaces && *scan == u' ') {
        const auto advance = frame.space_glyph->scaled_field_12;
        do {
            --scan;
            --frame.spaces;
            frame.saved_width -= advance;
        } while (*scan == u' ');
    }
    const char16_t stop = *scan;
    frame.line_end = stop == 0 ? scan :
        (frame.spaces == 0 || stop == u'\n' ? scan + 1 : last_space + 1);
    if (frame.saved_width == 0 || stop == 0 || stop == u'\n')
        frame.saved_width = truncate_scan(frame.scan_width);
    frame.natural_end = stop == 0 || stop == u'\n';
    if (!align_line(frame)) return false;
    frame.emitted_width = 0.0f;
    if (frame.text_cursor != frame.line_end) {
        frame.line_bottom = add(frame.pen_y, frame.height_float);
        (*frame.native_position)[1] = frame.pen_y;
        (*frame.native_position)[2] = 0.0f;
    }
    return true;
}
void advance_after_glyph(GuiTextWrappedContinuation& frame) noexcept {
    frame.indices = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(frame.indices) + 12u);
    frame.first_vertex += 4;
    const auto is_space = *frame.text_cursor == u' ';
    const std::int32_t metric = is_space ? 0 : frame.glyph->scaled_field_12;
    const float scale = is_space ? 0.0f : frame.lifetime->text().font_scale;
    const float step = frame.space_step;
    const float x = frame.placement.x;
    const float width = frame.emitted_width;
    float next_x, next_width;
    __asm {
        cmp is_space, 0
        jne emit_space
        fild metric
        fld scale
        fmul st(0), st(1)
        jmp emit_add
    emit_space:
        fld step
        fld st(0)
    emit_add:
        fadd x
        fstp next_x
        fadd width
        fstp next_width
    }
    ++frame.quad_index;
    ++frame.text_cursor;
    frame.placement.x = next_x;
    frame.emitted_width = next_width;
}
void finish_line(GuiTextWrappedContinuation& frame) noexcept {
    auto& text = frame.lifetime->text();
    if (greater_than(frame.emitted_width, text.measured_width))
        text.measured_width = frame.emitted_width;
    const float distance = text.distance_between_lines;
    text.line_count = signed_bits(static_cast<std::uint32_t>(text.line_count) + 1u);
    const float height = frame.height_float;
    const float y = frame.pen_y;
    const double one = 1.0;
    const double reduction = 0.1500000059604644775390625;
    float result;
    __asm {
        fld distance
        fadd one
        fsub reduction
        fmul height
        fadd y
        fstp result
    }
    frame.pen_y = result;
}
void vertical_metrics(GuiTextWrappedContinuation& frame) noexcept {
    const float maximum = frame.maximum_y, minimum = frame.minimum_y;
    const auto mode = static_cast<std::uint32_t>(frame.lifetime->text().vertical_align);
    const double divisor = 720.0, half = 0.5;
    float* const raw_height = &frame.lifetime->fields().field_178;
    const volatile float* const scale = &frame.services->vertical_scale_00e12fd4;
    const float* const container = &frame.lifetime->content_binding().widget.layout().transform.size.height;
    float spilled_height, normalized, result_offset = 0.0f;
    __asm {
        fld maximum
        fsub minimum
        fstp spilled_height
        fld spilled_height
        mov eax, raw_height
        fst dword ptr [eax]
        fdiv divisor
        mov eax, scale
        fld dword ptr [eax]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(1)
        fstp normalized
        cmp mode, 1
        je vertical_center
        cmp mode, 2
        jne vertical_other
        mov eax, container
        fmul dword ptr [eax]
        fsub normalized
        fstp result_offset
        jmp vertical_done
    vertical_center:
        mov eax, container
        fmul dword ptr [eax]
        fsub normalized
        fmul half
        fstp result_offset
        jmp vertical_done
    vertical_other:
        fstp st(0)
    vertical_done:
    }
    frame.vertical_offset = result_offset;
}
std::optional<GuiTextWrappedContinuation> finalize(GuiTextWrappedContinuation frame) {
    vertical_metrics(frame);
    const auto quads = static_cast<std::uint16_t>(frame.quad_index);
    frame.section->range_words_0c[1] = static_cast<std::uint32_t>(quads) * 4u;
    frame.section->range_words_0c[3] = static_cast<std::uint32_t>(quads) * 2u;
    for (; frame.position_index < frame.section->range_words_0c[1]; ++frame.position_index) {
        float position[3];
        if (!read_gui_text_float3_position_004768d0_fragment(
                frame.vertex_stream, frame.position_index, position)) {
            frame.pending = GuiTextWrappedPending::decoded_position;
            return frame;
        }
        position[1] = add(position[1], frame.vertical_offset);
        const auto address = word(frame.vertex_stream, 0x0c) * frame.position_index +
            word(frame.vertex_stream, 0x10) + word(frame.vertex_stream, 0x08);
        float* const destination = reinterpret_cast<float*>(static_cast<std::uintptr_t>(address));
        const float x = position[0], y = position[1], z = position[2];
        __asm {
            mov eax, destination
            movss xmm0, x
            fld y
            movss dword ptr [eax], xmm0
            fstp dword ptr [eax + 4]
            movss xmm0, z
            movss dword ptr [eax + 8], xmm0
        }
    }
    unlock_native_logical_index_stream_00b49c70(frame.index_stream, frame.services->mapping);
    unlock_native_logical_vertex_stream_00b49a80(frame.vertex_stream, frame.services->mapping);
    return std::nullopt;
}
std::optional<GuiTextWrappedContinuation> emit(GuiTextWrappedContinuation frame, bool start_line) {
    auto& lifetime = *frame.lifetime;
    auto& services = *frame.services;
    for (;;) {
        if (start_line && !select_line(frame)) return frame;
        while (frame.text_cursor != frame.line_end) {
            frame.glyph = &current_font(lifetime, services).glyph(*frame.text_cursor);
            if (frame.first_resources) {
                void* const gfx = frame.glyph->gfx_texture_18;
                set_native_material_texture_00b189f0(
                    *static_cast<NativeMaterialStorage*>(frame.section->material_20),
                    0, gfx, services.actual_owners);
                void* const alpha = frame.glyph->alpha_texture_1c;
                set_native_material_texture_00b189f0(
                    *static_cast<NativeMaterialStorage*>(frame.section->material_20),
                    1, alpha, services.actual_owners);
                frame.first_resources = false;
                lifetime.fields().field_18c = frame.placement.x;
            }
            if (!greater_than(frame.pen_y, frame.minimum_y)) frame.minimum_y = frame.pen_y;
            if (!greater_than(frame.maximum_y, frame.line_bottom)) frame.maximum_y = frame.line_bottom;
            frame.placement.code_unit = *frame.text_cursor;
            (*frame.native_position)[0] = frame.placement.x;
            frame.placement.y = (*frame.native_position)[1];
            const auto result = write_gui_text_quad_00ab98f0_fragment(lifetime,
                *frame.glyph, frame.placement, frame.quad_index,
                static_cast<std::uint16_t>(frame.signed_height), frame.first_vertex,
                frame.vertex_stream, frame.indices, services.vertical_scale_00e12fd4);
            if (result == GuiTextGlyphWriteResult::needs_glyph_child) {
                frame.pending = GuiTextWrappedPending::glyph_child;
                return frame;
            }
            advance_after_glyph(frame);
        }
        finish_line(frame);
        if (!*frame.text_cursor) return finalize(std::move(frame));
        start_line = true;
    }
}
} // namespace

bool read_gui_text_float3_position_004768d0_fragment(
    void* stream, std::uint32_t index, float (&output)[3]) {
    if (word(stream, 0x50) != 0) return false;
    const auto address = word(stream, 0x0c) * index + word(stream, 0x10) + word(stream, 0x08);
    const float* const source = reinterpret_cast<const float*>(static_cast<std::uintptr_t>(address));
    float* const destination = output;
    __asm {
        mov ecx, source
        mov eax, destination
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fstp dword ptr [eax + 8]
    }
    return true;
}

std::optional<GuiTextWrappedContinuation> build_gui_text_wrapped_00aba270(
    GuiTextLifetime& lifetime, const std::u16string& source,
    NativeMeshSectionStorage& section, GuiTextWrappedServices& services) {
    auto binding = lifetime.content_binding();
    clear_gui_text_glyph_children_00ab80c0(binding, services.widgets,
        services.parenting, services.children);
    GuiTextWrappedContinuation frame;
    frame.lifetime = &lifetime;
    frame.services = &services;
    frame.section = &section;
    frame.native_position = std::make_unique<std::array<float, 3>>();
    frame.signed_height = current_font(lifetime, services).signed_height_14();
    frame.height_float = static_cast<float>(frame.signed_height);
    auto& text = lifetime.text();
    if (&source != &text.text) text.text.assign(source);
    auto* reference = binding.widget.model_reference();
    if (!reference || &reference->model_owner().environment.retained_owners != &services.actual_owners)
        throw std::logic_error("Wrapped Text requires its actual primary model owner");
    frame.mesh = static_cast<NativeMeshStorage*>(
        gui_model_geometry_00b74640(reference->model_owner().storage.model, 0));
    frame.vertex_stream = frame.mesh->vertex_streams_64[0];
    (void)lock_native_logical_vertex_stream_00b49980(frame.vertex_stream,
        services.mapping, length(text) * 4u, 0, 0);
    frame.index_stream = frame.mesh->index_stream_60;
    frame.indices = lock_native_logical_index_stream_00b49b60(frame.index_stream,
        services.mapping, length(text) * 6u, 0, 0);
    std::uint16_t saved_control;
    __asm { fnstcw saved_control }
    section.range_words_0c[1] = length(text) * 4u;
    section.range_words_0c[3] = length(text) * 2u;
    auto& captured_font = current_font(lifetime, services);
    const float normalized_width = binding.widget.layout().transform.size.width;
    text.line_count = 0;
    frame.container_width = container_width(normalized_width, saved_control);
    frame.space_glyph = &captured_font.glyph(u' ');
    frame.text_cursor = text.text.c_str();
    if (!*frame.text_cursor) return finalize(std::move(frame));
    return emit(std::move(frame), true);
}

std::optional<GuiTextWrappedContinuation>
resume_gui_text_wrapped_after_child_00aba270(GuiTextWrappedContinuation frame) {
    if (frame.pending != GuiTextWrappedPending::glyph_child)
        throw std::logic_error("Wrapped frame is not suspended at the glyph-child call");
    advance_after_glyph(frame);
    return emit(std::move(frame), false);
}
} // namespace bsp
