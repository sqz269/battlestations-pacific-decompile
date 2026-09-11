#include "bsp/gui_text_single_line.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
NativeFontResources& current_font(GuiTextLifetime& lifetime, GuiTextSingleLineServices& services) {
    auto& font = services.fonts.resolve(lifetime.text().font);
    if (&font.actual_owners() != &services.actual_owners)
        throw std::logic_error("Text font and material must share actual retained owners");
    return font;
}
std::uint32_t length(const GuiTextWidget& text) noexcept {
    return static_cast<std::uint32_t>(text.text.size());
}
std::uint32_t container_width(float normalized_width, std::uint16_t saved_control) noexcept {
    const double multiplier = 960.0;
    const std::uint16_t truncation_control = saved_control | 0x0c00;
    std::int64_t converted;
    __asm {
        fld normalized_width
        fmul multiplier
        fldcw truncation_control
        fistp converted
        fldcw saved_control
    }
    return static_cast<std::uint32_t>(converted);
}
void advance(float& current, std::uint16_t glyph_advance, float scale) noexcept {
    const std::int32_t value = glyph_advance;
    float* const target = &current;
    __asm {
        fild value
        fmul scale
        mov eax, target
        fadd dword ptr [eax]
        fstp dword ptr [eax]
    }
}
void aligned_origin(std::uint32_t width, float measured, GuiTextAlign alignment,
    float& output) noexcept {
    if (alignment != GuiTextAlign::Center && alignment != GuiTextAlign::Right) {
        output = 0.0f;
        return;
    }
    std::int32_t signed_width;
    std::memcpy(&signed_width, &width, sizeof(width));
    const float unsigned_correction = 4294967296.0f;
    const double half = 0.5;
    const auto center = alignment == GuiTextAlign::Center;
    float* const target = &output;
    __asm {
        fild signed_width
        cmp signed_width, 0
        jge width_ready
        fadd unsigned_correction
    width_ready:
        fsub measured
        cmp center, 0
        je origin_ready
        fmul half
    origin_ready:
        mov eax, target
        fstp dword ptr [eax]
    }
}
void advance_after_glyph(GuiTextSingleLineContinuation& frame) noexcept {
    // The native keeps the selected glyph across its call; it reloads that
    // glyph's advance and current Text scale even if the font pointer changed.
    const auto glyph_advance = frame.glyph->scaled_field_12;
    frame.first_vertex += 4;
    frame.indices = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(frame.indices) + 12u);
    ++frame.quad_index;
    ++frame.text_cursor;
    advance(frame.placement.x, glyph_advance, frame.lifetime->text().font_scale);
}
std::optional<GuiTextSingleLineContinuation> emit(
    GuiTextSingleLineContinuation frame, bool bind_first_resources) {
    auto& lifetime = *frame.lifetime;
    auto& services = *frame.services;
    while (*frame.text_cursor) {
        frame.glyph = &current_font(lifetime, services).glyph(*frame.text_cursor);
        if (bind_first_resources) {
            void* const gfx = frame.glyph->gfx_texture_18;
            set_native_material_texture_00b189f0(
                *static_cast<NativeMaterialStorage*>(frame.section->material_20),
                0, gfx, services.actual_owners);
            // Retained-texture release can change the current material or the
            // selected glyph's alpha slot. Native reloads both after slot0.
            void* const alpha = frame.glyph->alpha_texture_1c;
            set_native_material_texture_00b189f0(
                *static_cast<NativeMaterialStorage*>(frame.section->material_20),
                1, alpha, services.actual_owners);
            bind_first_resources = false;
            lifetime.fields().field_18c = frame.placement.x;
        }
        frame.placement.code_unit = *frame.text_cursor; // Reload after bindings.
        // ABA1DE/E3/EA use one float3; ABA1F2 copies the separate pen x into
        // it each time. Y/Z were initialized once at ABA168/16E. Preserve any
        // child-tail writes across suspension and subsequent glyph calls.
        (*frame.native_position)[0] = frame.placement.x;
        frame.placement.y = (*frame.native_position)[1];
        const auto result = write_gui_text_quad_00ab98f0_fragment(lifetime,
            *frame.glyph, frame.placement, frame.quad_index, frame.height,
            frame.first_vertex, frame.vertex_stream, frame.indices,
            services.vertical_scale_00e12fd4);
        if (result == GuiTextGlyphWriteResult::needs_glyph_child) return std::move(frame);
        advance_after_glyph(frame);
    }
    unlock_native_logical_index_stream_00b49c70(frame.index_stream, services.mapping);
    unlock_native_logical_vertex_stream_00b49a80(frame.vertex_stream, services.mapping);
    rebuild_native_mesh_section_vertex_layout_00b865a0(*frame.section,
        services.actual_owners, frame.mesh, services.layouts);
    return std::nullopt;
}
} // namespace

std::optional<GuiTextSingleLineContinuation> build_gui_text_single_line_00ab9fd0(
    GuiTextLifetime& lifetime, std::u16string_view, NativeMeshSectionStorage& section,
    GuiTextSingleLineServices& services) {
    auto& text = lifetime.text();
    auto& widget = lifetime.content_binding().widget;
    text.line_count = 0;
    auto* reference = widget.model_reference();
    if (!reference || &reference->model_owner().environment.retained_owners != &services.actual_owners)
        throw std::logic_error("Text single-line geometry requires the same actual primary model");
    auto* mesh = static_cast<NativeMeshStorage*>(
        gui_model_geometry_00b74640(reference->model_owner().storage.model, 0));
    void* const vertices = mesh->vertex_streams_64[0]; // Existing B73260 index0 leaf.
    (void)lock_native_logical_vertex_stream_00b49980(vertices, services.mapping,
        length(text) * 4u, 0, 0);
    void* const index_stream = mesh->index_stream_60; // AFTER vertex Lock callback.
    void* const indices = lock_native_logical_index_stream_00b49b60(index_stream,
        services.mapping, length(text) * 6u, 0, 0);
    std::uint16_t saved_control;
    __asm { fnstcw saved_control }
    section.range_words_0c[1] = length(text) * 4u; // Actual section+10.
    section.range_words_0c[3] = length(text) * 2u; // Actual section+18.
    const float normalized_width = widget.layout().transform.size.width;
    text.measured_width = 0.0f;
    const auto height = current_font(lifetime, services).data().scaled_height;
    const auto width = container_width(normalized_width, saved_control);
    for (const char16_t* cursor = text.text.c_str(); *cursor; ++cursor) {
        const auto& glyph = current_font(lifetime, services).glyph(*cursor);
        advance(text.measured_width, glyph.scaled_field_12, text.font_scale);
    }
    float origin;
    aligned_origin(width, text.measured_width, text.align, origin);
    GuiTextSingleLineContinuation frame{&lifetime, &services, mesh, &section,
        vertices, index_stream, indices, nullptr, text.text.c_str(),
        FontGlyphPlacement{0, origin, 0.0f},
        std::make_unique<std::array<float, 3>>(), 0, 0, height};
    return emit(std::move(frame), true);
}

std::optional<GuiTextSingleLineContinuation>
resume_gui_text_single_line_after_child_00ab9fd0(GuiTextSingleLineContinuation frame) {
    if (!frame.native_position)
        throw std::invalid_argument("single-line child continuation requires its original float3 allocation");
    advance_after_glyph(frame);
    return emit(std::move(frame), false);
}
} // namespace bsp
