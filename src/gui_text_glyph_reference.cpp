#include "bsp/gui_text_glyph_reference.hpp"
#include <stdexcept>

namespace bsp {
namespace {
GuiTextLifetime& canonical_text(GuiWidgetOwner& owner,
    GuiWidgetOwnerRuntime& runtime) {
    if (&runtime.owner(owner.layout()) != &owner ||
        owner.layout().type != GuiWidgetType::Text ||
        owner.layout().transform.type_id != 3)
        throw std::logic_error("glyph reference requires its canonical Text owner domain");
    auto* const text = owner.text_lifetime();
    if (!text || &text->content_binding().widget != &owner)
        throw std::logic_error("glyph reference has no matching canonical Text companion");
    return *text;
}
GuiWidgetOwner& canonical_owner(GuiTextLifetime& text,
    GuiWidgetOwnerRuntime& runtime) {
    auto& owner = text.content_binding().widget;
    if (&canonical_text(owner, runtime) != &text)
        throw std::logic_error("glyph operation requires the owner's same Text companion");
    return owner;
}
void publish_offsets(GuiTextLifetimeFields& fields, float x,
    const volatile float& current_y) noexcept {
    fields.field_1b8 = x;
    fields.field_1bc = current_y; // Reload only AFTER the x store.
    fields.glyph_offsets_written = true; // C++ provenance, no native store.
}
float x87_sum(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fadd b
        fstp result
    }
    return result;
}
float x87_spill(float value) noexcept {
    float result;
    __asm {
        fld value
        fstp result
    }
    return result;
}
float x87_difference(float a, const volatile double& b) noexcept {
    float result;
    const volatile double* operand = &b;
    __asm {
        fld a
        mov eax, operand
        fsub qword ptr [eax]
        fstp result
    }
    return result;
}
float x87_input_x(const float& input, const volatile double& bias,
    const volatile double& divisor, float reference_x) noexcept {
    float result;
    const float* source = &input;
    const volatile double* bias_word = &bias;
    const volatile double* divisor_word = &divisor;
    __asm {
        mov eax, source
        fld dword ptr [eax]
        mov eax, bias_word
        fsub qword ptr [eax]
        mov eax, divisor_word
        fdiv qword ptr [eax]
        fadd reference_x
        fstp result
    }
    return result; // Native AB9F79 spill BEFORE subtracting x_subtract.
}
} // namespace

GuiTextLifetime* resolve_gui_text_glyph_reference(GuiTextLifetime& text,
    GuiWidgetOwnerRuntime& runtime) {
    canonical_owner(text, runtime);
    auto* const reference = text.fields().pointer_1b0;
    return reference ? &canonical_text(*reference, runtime) : nullptr;
}

void set_gui_text_glyph_reference_00531130(GuiTextLifetime& text,
    const std::u16string& source, void* listener,
    GuiWidgetOwner* reference, GuiWidgetOwnerRuntime& runtime) {
    canonical_owner(text, runtime);
    if (reference) canonical_text(*reference, runtime);
    auto& fields = text.fields();
    if (&source != &fields.string_1a4) {
        // Typed owner projection of resize(source length,1), then copy through
        // the terminator. Native pool allocation callbacks are not fabricated.
        if (source.find(u'\0') != std::u16string::npos)
            throw std::invalid_argument("glyph reference source requires its canonical terminator at size");
        fields.string_1a4 = source;
    }
    fields.pointer_1ac = listener; // 53117B/531195/5311AF, before +1B0.
    fields.pointer_1b0 = reference;
}

float begin_gui_text_prompt_glyph_offsets_00531380_fragment(
    GuiTextLifetime& text, const volatile double& width,
    const volatile float& current_y) {
    auto& fields = text.fields();
    float captured_x;
    auto* const mode = &fields.byte_1b4;
    const volatile double* denominator = &width;
    __asm {
        fldz
        mov eax, denominator
        fdiv qword ptr [eax]
        mov eax, mode
        mov byte ptr [eax], 1
        fstp captured_x
    }
    publish_offsets(fields, captured_x, current_y);
    return captured_x;
}
void copy_gui_text_prompt_glyph_offsets_00531380_fragment(
    GuiTextLifetime& text, float captured_x, const volatile float& current_y) {
    auto& fields = text.fields();
    fields.byte_1b4 = 1;
    publish_offsets(fields, captured_x, current_y);
}

void position_gui_text_glyph_child_00ab98f0_fragment(
    GuiTextLifetime& parent, GuiTextLifetime& child,
    const std::array<float, 3>& original_position,
    GuiWidgetOwnerRuntime& runtime, const GuiTextGlyphPositionConstants& constants) {
    auto& child_owner = canonical_owner(child, runtime);
    auto* reference = resolve_gui_text_glyph_reference(parent, runtime);
    if (!reference)
        throw std::logic_error("glyph child position requires a current reference Text");
    const auto reference_position = resolved_position(
        reference->content_binding().widget.layout().transform); // AB9ECD.
    auto& fields = parent.fields();
    GuiWidgetPoint position;
    if (fields.byte_1b4 != 0) {
        if (!fields.glyph_offsets_written)
            throw std::logic_error("glyph offsets were not written by an established producer");
        // Native reads/spills BOTH offsets before beginning the sums.
        const float x_offset = x87_spill(fields.field_1b8);
        const float y_offset = x87_spill(fields.field_1bc);
        position.x = x87_sum(x_offset, reference_position.x);
        position.y = x87_sum(y_offset, reference_position.y);
    } else {
        // AB9F1F reloads the original parent's reference slot for the length.
        reference = resolve_gui_text_glyph_reference(parent, runtime);
        if (!reference)
            throw std::logic_error("glyph reference disappeared before its length read");
        float x = reference_position.x;
        if (reference->text().text.size() != 1u) {
            x = x87_input_x(original_position[0], constants.input_x_bias_00d06880,
                constants.input_x_divisor_00cec380, reference_position.x);
        }
        position.x = x87_difference(x, constants.x_subtract_00d7a358);
        position.y = x87_difference(reference_position.y, constants.y_subtract_00d5c7a8);
    }
    position.z = x87_difference(reference_position.z, constants.z_subtract_00d7a220);
    // AB9FA9 -> existing AA8240 inverse arithmetic, then actual AA7220/AA70E0.
    child_owner.layout().transform.position = local_position_for_resolved(
        child_owner.layout().transform, position);
    child_owner.recompose_00aa7220();
    child_owner.refresh_bounds_00aa70e0();
}
} // namespace bsp
