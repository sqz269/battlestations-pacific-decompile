#include "bsp/gui_text_glyph_child_runtime.hpp"
#include "bsp/gui_widget_attach.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Text glyph child continuation requires MSVC Win32.
#endif

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
GuiWidgetOwner& parent_owner(GuiTextGlyphChildTailContinuation& frame) {
    return frame.call.parent_ecx.content_binding().widget;
}
GuiWidgetOwner& child_owner(GuiTextGlyphChildTailContinuation& frame) {
    require(frame.child_identity != nullptr, "glyph child has not been constructed");
    return frame.services.buffers.widgets.owner(*frame.child_identity);
}
GuiTextLifetime& child_text(GuiTextGlyphChildTailContinuation& frame) {
    return frame.implementation().lifetime();
}
GuiTextLifetime& current_reference(GuiTextGlyphChildTailContinuation& frame) {
    auto* const reference = resolve_gui_text_glyph_reference(
        frame.call.parent_ecx, frame.services.buffers.widgets);
    require(reference != nullptr, "glyph child requires the current borrowed reference Text");
    return *reference;
}
float copy_float_x87(const volatile float& source) noexcept {
    const volatile float* from = &source;
    float result;
    __asm {
        mov eax, from
        fld dword ptr [eax]
        fstp dword ptr [result]
    }
    return result;
}
void validate_services(GuiTextGlyphChildTailContinuation& frame) {
    auto& s = frame.services;
    auto& b = s.buffers;
    auto& actual = b.geometry.actual_owners();
    require(&b.widgets.owner(parent_owner(frame).layout()) == &parent_owner(frame) &&
        parent_owner(frame).text_lifetime() == &frame.call.parent_ecx &&
        frame.call.position_2 != nullptr,
        "glyph child tail requires the same live parent Text and original position backing");
    require(&s.strings == &b.strings && &s.fonts.names.widgets == &b.widgets &&
        &s.style.widgets == &b.widgets && &s.fonts.names.actual_owners == &actual &&
        &s.style.actual_owners == &actual && &b.materials.retained_owners == &actual &&
        &b.widgets.environment().models.retained_owners == &actual &&
        &b.widgets.environment().models.nodes == &b.parenting.nodes,
        "glyph child tail requires the same actual string, widget, render and node domains");
    require(s.shader_name_00cefd78 != nullptr &&
        &s.one_00d7a24c == &s.fonts.one_00d7a24c,
        "glyph child tail requires live shader-name and constant storage");
}
GuiTextGlyphChildTailStatus domain_failure(GuiTextGlyphChildTailContinuation& frame) noexcept {
    frame.failure = std::current_exception();
    frame.status = GuiTextGlyphChildTailStatus::domain_required;
    return frame.status;
}
void finish_after_content(GuiTextGlyphChildTailContinuation& frame) {
    auto& s = frame.services;
    frame.pending_native_address = 0x00ab9e73;
    // Read current header data, then current length. Native cleanup leaves the
    // eight header bytes untouched; these booleans are caller-lifetime metadata.
    if (auto* const data = frame.code_unit.data) {
        const auto bytes = frame.code_unit.length * 2u + 2u;
        s.strings.release(reinterpret_cast<char*>(data), bytes);
    }
    frame.code_unit_live = false;
    frame.pending_native_address = 0x00ab9e9b;
    append_gui_widget_child_00aaa5a0(s.buffers.widgets, s.buffers.parenting,
        parent_owner(frame).layout(), frame.child);
    frame.pending_native_address = 0x00ab9ea0;
    // One MOVSS load initializes both native pivot cells before current30.
    const float pivot = s.pivot_00ce3800;
    const float pair[]{pivot, pivot};
    set_gui_widget_pivot_00a9e0b0(child_owner(frame), pair);
    frame.pending_native_address = 0x00ab9ec2;
    position_gui_text_glyph_child_00ab98f0_fragment(frame.call.parent_ecx,
        child_text(frame), *frame.call.position_2, s.buffers.widgets, s.position);
    frame.pending_native_address = 0x00ab9fae;
    frame.status = GuiTextGlyphChildTailStatus::complete;
}
} // namespace

GuiTextGlyphChildTailContinuation::GuiTextGlyphChildTailContinuation(
    const GuiTextGlyphChildCallFrame& original, GuiTextGlyphChildTailServices& supplied)
    : call(original), services(supplied) {
    static_assert(sizeof(Utf16Header) == 8 && offsetof(Utf16Header, data) == 4);
}
GuiTextGlyphChildTailContinuation::~GuiTextGlyphChildTailContinuation() noexcept {
    if (status != GuiTextGlyphChildTailStatus::ready &&
        status != GuiTextGlyphChildTailStatus::complete) std::terminate();
}
GuiTextRuntimeImplementation& GuiTextGlyphChildTailContinuation::implementation() {
    auto& owner = child_owner(*this);
    auto* const implementation = dynamic_cast<GuiTextRuntimeImplementation*>(&owner.implementation());
    require(implementation && owner.text_lifetime() == &implementation->lifetime(),
        "glyph child requires its one canonical runtime Text implementation");
    return *implementation;
}
GuiTextRuntimeContentContinuation* GuiTextGlyphChildTailContinuation::pending_content() {
    if (status != GuiTextGlyphChildTailStatus::pending_content) return nullptr;
    return implementation().pending_content();
}

GuiTextGlyphChildTailStatus begin_gui_text_glyph_child_tail_00ab98f0_fragment(
    GuiTextGlyphChildTailContinuation& frame) {
    if (frame.status != GuiTextGlyphChildTailStatus::ready)
        throw std::logic_error("glyph child tail cannot repeat already executed native phases");
    auto& s = frame.services;
    auto& b = s.buffers;
    // A started frame must survive errors even before any allocation is returned.
    frame.status = GuiTextGlyphChildTailStatus::domain_required;
    try {
        validate_services(frame);
        frame.pending_native_address = 0x00ab9d38;
        frame.child = s.factory.construct_unbound_glyph_child();
        frame.child_identity = frame.child.get();
        require(frame.child_identity != nullptr, "native null Text allocation has no successful child tail");
        auto& owner = child_owner(frame);
        require(owner.node_binding() == nullptr, "glyph child factory must return an unbound Text");
        (void)frame.implementation();
        frame.pending_native_address = 0x00ab9d70;
        frame.call.parent_ecx.glyph_children_198().push_back(frame.child_identity);

        frame.pending_native_address = 0x00ab9d75;
        auto& reference_owner = current_reference(frame).content_binding().widget;
        auto* const reference_model = reference_owner.model_reference();
        require(reference_model && reference_owner.node_binding() == &reference_model->model_owner().node,
            "current reference Text requires its canonical actual Model");
        frame.pending_native_address = 0x00ab9d87;
        (void)clone_native_gui_text_model_00b752b0(reference_model->model_owner(),
            b.widgets, b.geometry, b.materials, b.material_vtable_00d5e520,
            s.mesh_vtable_00d62d60, frame.acquired);
        frame.pending_native_address = 0x00ab9d8c;
        require(frame.acquired.model != nullptr, "completed glyph Model clone requires its creator reference");
        owner.bind_scene_00aa6720(&frame.acquired.model->model_owner().node);
        // AA6720 performs no retain/release; the widget now owns that creator.
        frame.acquired.model = nullptr;
        frame.pending_native_address = 0x00ab9d93;
        auto& text = child_text(frame);
        ensure_gui_text_draw_sections_00ab8530(owner, text.text(), text.shadow_slot_188(), b);
        frame.pending_native_address = 0x00ab9da5;
        owner.set_visible34(true);
        frame.pending_native_address = 0x00ab9db2;
        set_gui_widget_listener_00aa6bc0(owner, frame.call.parent_ecx.fields().pointer_1ac, 0);
        owner.layout().transform.mouse_hit = true;
        frame.pending_native_address = 0x00ab9dba;
        // Resolve +1B0 again AFTER visibility/listener phases; never cache the
        // earlier template Text or font name across the clone's callbacks.
        const auto& font_name = current_reference(frame).text().font_name;
        frame.pending_native_address = 0x00ab9dc9;
        set_gui_text_font_name_00ab8c30(text, font_name, s.fonts);
        frame.pending_native_address = 0x00ab9dce;
        const auto offset = copy_float_x87(s.shadow_offset_00d5c5c0);
        const GuiTextColor shadow{0.0f, 0.0f, 0.0f, s.one_00d7a24c};
        auto style = text.style_binding(s.style);
        frame.pending_native_address = 0x00ab9e05;
        configure_gui_text_shadow_00ab6c30(style, 1, GuiTextShadowPos::Behind, offset, shadow);

        frame.pending_native_address = 0x00ab9e13;
        frame.shader_temporary.assign_0041e870(s.strings, s.shader_name_00cefd78);
        frame.shader_temporary_live = true;
        frame.shader_projection.assign(frame.shader_temporary.data(), frame.shader_temporary.length());
        frame.pending_native_address = 0x00ab9e1f;
        set_gui_text_shader_name_00ab8e70(text, frame.shader_projection, s.fonts.names);
        frame.pending_native_address = 0x00ab9e24;
        destroy_native_string_header_0041dd20(&frame.shader_temporary, s.strings);
        frame.shader_temporary_live = false;
        std::string{}.swap(frame.shader_projection);
        frame.pending_native_address = 0x00ab9e51;
        construct_gui_text_code_unit_00ab81c0(&frame.code_unit,
            frame.call.code_unit_word_10, s.strings);
        frame.code_unit_live = true;
        frame.pending_native_address = 0x00ab9e61;
        try {
            // AB6AB0 composes precisely this caller's ABA8D0 then current50.
            // The actual native header is borrowed until BOTH have completed.
            frame.implementation().submit_utf16_00ab6ab0(
                std::u16string_view(frame.code_unit.data, frame.code_unit.length));
        } catch (const GuiTextRuntimePending&) {
            require(frame.implementation().has_pending_operation(),
                "pending glyph content must retain its actual child operation");
            frame.status = GuiTextGlyphChildTailStatus::pending_content;
            return frame.status;
        }
        finish_after_content(frame);
        return frame.status;
    } catch (...) { return domain_failure(frame); }
}

GuiTextGlyphChildTailStatus resume_gui_text_glyph_child_tail_after_content(
    GuiTextGlyphChildTailContinuation& frame) {
    if (frame.status != GuiTextGlyphChildTailStatus::pending_content)
        throw std::logic_error("glyph tail content resume requires its saved pending content phase");
    try {
        if (frame.implementation().has_pending_operation()) return frame.status;
        // The UTF16 header stays unchanged after release. Mark this phase
        // non-resumable BEFORE cleanup/attachment callbacks can reenter it.
        frame.status = GuiTextGlyphChildTailStatus::domain_required;
        finish_after_content(frame);
        return frame.status;
    } catch (...) { return domain_failure(frame); }
}
} // namespace bsp
