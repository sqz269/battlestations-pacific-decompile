#include "bsp/gui_widget_relative_bounds.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void initial_measure(const GuiWidgetSize& source, GuiWidgetSize& size,
    GuiWidgetSize& offset, const GuiWidgetSize& padding, const volatile double& half) {
    const auto* input = &source;
    const auto* pad = &padding;
    auto* output = &size;
    auto* position = &offset;
    const volatile double* factor = &half;
    float w, h, negative_w, negative_h, x, y;
    __asm {
        mov eax, input
        mov ecx, pad
        fld dword ptr [eax]
        fadd dword ptr [ecx]
        fstp w
        fld dword ptr [eax + 4]
        fadd dword ptr [ecx + 4]
        fstp h
        mov edx, output
        fld w
        fstp dword ptr [edx]
        fld h
        fstp dword ptr [edx + 4]
        fld dword ptr [ecx]
        fchs
        fstp negative_w
        fld dword ptr [ecx + 4]
        fchs
        fstp negative_h
        fld negative_w
        mov eax, factor
        fld qword ptr [eax]
        fmul st(1), st(0)
        fxch st(1)
        fstp x
        fmul negative_h
        fstp y
        mov edx, position
        fld x
        fstp dword ptr [edx]
        fld y
        fstp dword ptr [edx + 4]
    }
}
bool native_equal_zero(float width, const volatile float& zero) {
    const volatile float* address = &zero;
    std::uint32_t equal;
    __asm {
        mov eax, address
        movss xmm0, width
        ucomiss xmm0, dword ptr [eax]
        lahf
        test ah, 44h
        setnp al
        movzx eax, al
        mov equal, eax
    }
    return equal != 0;
}
bool native_positive(float width, const volatile float& zero) {
    const volatile float* address = &zero;
    std::uint32_t positive;
    __asm {
        mov eax, address
        movss xmm0, width
        comiss xmm0, dword ptr [eax]
        seta al
        movzx eax, al
        mov positive, eax
    }
    return positive != 0;
}
void text_width(float measured, const volatile double& divisor, const float& padding,
    float& output) {
    const volatile double* scale = &divisor;
    const float* pad = &padding;
    float* result = &output;
    float normalized;
    __asm {
        mov eax, scale
        fld measured
        fdiv qword ptr [eax]
        fstp normalized
        mov eax, pad
        fld normalized
        fadd dword ptr [eax]
        mov eax, result
        fstp dword ptr [eax]
    }
}
void store_float(float value, float& output) {
    auto* result = &output;
    __asm {
        mov eax, result
        fld value
        fstp dword ptr [eax]
    }
}
void add_float(float value, const float& increment, float& output) {
    const auto* addend = &increment;
    auto* result = &output;
    __asm {
        fld value
        mov eax, addend
        fadd dword ptr [eax]
        mov eax, result
        fstp dword ptr [eax]
    }
}
void add_double(float value, const volatile double& increment, float& output) {
    const volatile double* addend = &increment;
    auto* result = &output;
    __asm {
        fld value
        mov eax, addend
        fadd qword ptr [eax]
        mov eax, result
        fstp dword ptr [eax]
    }
}
void adjust_alignment(const float& source_extent, const float& measured_extent,
    const float& padding, float& offset, bool centered, const volatile double& half) {
    const auto* source = &source_extent;
    const auto* measured = &measured_extent;
    const auto* pad = &padding;
    auto* result = &offset;
    const volatile double* factor = &half;
    __asm {
        mov eax, source
        fld dword ptr [eax]
        mov eax, measured
        fld dword ptr [eax]
        mov eax, pad
        fsub dword ptr [eax]
        fsubp st(1), st(0)
        cmp centered, 0
        je alignment_add
        mov eax, factor
        fmul qword ptr [eax]
    alignment_add:
        mov eax, result
        fadd dword ptr [eax]
        fstp dword ptr [eax]
    }
}
void pivot_difference(const GuiWidgetTransform& source, const GuiWidgetTransform& destination,
    GuiWidgetSize& offset) {
    const auto* source_size = &source.size;
    const auto* destination_size = &destination.size;
    const auto* source_x = &source.pivot_x;
    const auto* source_y = &source.pivot_y;
    const auto* destination_x = &destination.pivot_x;
    const auto* destination_y = &destination.pivot_y;
    auto* offset_output = &offset;
    float source_width, source_height, destination_width, destination_height, difference_x, difference_y;
    __asm {
        mov eax, source_size
        fld dword ptr [eax]
        mov ecx, source_x
        fmul dword ptr [ecx]
        fstp source_width
        fld dword ptr [eax + 4]
        mov ecx, source_y
        fmul dword ptr [ecx]
        fstp source_height
        mov eax, destination_size
        fld dword ptr [eax]
        mov ecx, destination_x
        fmul dword ptr [ecx]
        fstp destination_width
        fld dword ptr [eax + 4]
        mov ecx, destination_y
        fmul dword ptr [ecx]
        fstp destination_height
        fld destination_width
        fsub source_width
        fstp difference_x
        fld destination_height
        fsub source_height
        fstp difference_y
        mov eax, offset_output
        fld difference_x
        fadd dword ptr [eax]
        fstp dword ptr [eax]
        fld difference_y
        fadd dword ptr [eax + 4]
        fstp dword ptr [eax + 4]
    }
}
} // namespace

void measure_gui_widget_padded_00ac06c0(GuiWidgetOwner& source,
    GuiWidgetSize& size, GuiWidgetSize& offset, const GuiWidgetSize& padding,
    float width, const GuiWidgetRelativeBoundsConstants& constants) {
    initial_measure(source.layout().transform.size, size, offset, padding, constants.half_00d7a280);
    if (source.implementation().type5c(source) != 3) return;
    auto* text = dynamic_cast<GuiTextRuntimeImplementation*>(&source.implementation());
    if (!text || source.text_lifetime() != &text->lifetime())
        throw std::logic_error("Text measurement requires the same canonical Text lifetime");
    if (native_equal_zero(width, constants.zero_00d7a218))
        text_width(text->lifetime().text().measured_width, constants.width_divisor_00cec380,
            padding.width, size.width);
    else store_float(width, size.width);
    const float height = text->normalized_height_00ab6bd0();
    add_float(height, padding.height, size.height);
    const auto horizontal = static_cast<std::int32_t>(text->lifetime().text().align);
    if (horizontal == 1 || horizontal == 2)
        adjust_alignment(source.layout().transform.size.width, size.width, padding.width,
            offset.width, horizontal == 1, constants.half_00d7a280);
    const auto vertical = static_cast<std::int32_t>(text->lifetime().text().vertical_align);
    if (vertical == 1 || horizontal == 2)
        adjust_alignment(source.layout().transform.size.height, size.height, padding.height,
            offset.height, vertical == 1, constants.half_00d7a280);
}
void set_gui_widget_current_size58(GuiWidgetOwner& owner, const GuiWidgetSize& size) {
    owner.implementation().set_size58(owner, size);
}
void set_gui_widget_resolved_position_00aa8240(GuiWidgetOwner& owner, const GuiWidgetPoint& world) {
    owner.layout().transform.position = local_position_for_resolved(owner.layout().transform, world);
    owner.recompose_00aa7220();
    owner.refresh_bounds_00aa70e0();
}
void fit_gui_widget_to_source_00ac0820(GuiWidgetOwner& source,
    GuiWidgetOwner& destination, float width, const GuiWidgetRelativeBoundsConstants& constants) {
    if (&source.runtime() != &destination.runtime())
        throw std::logic_error("Relative GUI placement requires the same canonical owner domain");
    GuiWidgetSize padding;
    float measured_width;
    const volatile float* padding_address = &constants.padding_x_00ce9bac;
    auto* padding_output = &padding;
    __asm {
        fld width
        mov eax, padding_address
        movss xmm0, dword ptr [eax]
        fstp measured_width
        mov eax, padding_output
        movss dword ptr [eax], xmm0
        xorps xmm0, xmm0
        movss dword ptr [eax + 4], xmm0
    }
    GuiWidgetSize size, offset;
    measure_gui_widget_padded_00ac06c0(source, size, offset, padding, measured_width, constants);
    if (native_positive(width, constants.zero_00d7a218)) size.width = width; // MOVSS
    set_gui_widget_current_size58(destination, size);
    pivot_difference(source.layout().transform, destination.layout().transform, offset);
    const auto position = resolved_position(source.layout().transform);
    GuiWidgetPoint output;
    // Native evaluates y, then z, then x after the resolved-position call.
    add_float(position.y, offset.height, output.y);
    add_double(position.z, constants.depth_offset_00d7a210, output.z);
    add_float(position.x, offset.width, output.x);
    set_gui_widget_resolved_position_00aa8240(destination, output);
}
void position_main_menu_highlight_00580820(GuiWidgetOwner& selected,
    GuiWidgetOwnerRuntime& owners, GuiLayoutWidget*& highlight_294,
    const GuiWidgetRelativeBoundsConstants& constants, const volatile double& extra_height) {
    if (&selected.runtime() != &owners)
        throw std::logic_error("Menu highlight requires its actual objective owner domain");
    const auto highlight = [&]() -> GuiWidgetOwner& {
        if (!highlight_294) throw std::logic_error("Menu screen highlight294 is unbound");
        return owners.owner(*highlight_294);
    };
    highlight().set_visible34(true);
    fit_gui_widget_to_source_00ac0820(selected, highlight(), 0.0f, constants);
    const float width = highlight().layout().transform.size.width; // MOVSS
    float height;
    add_double(highlight().layout().transform.size.height, extra_height, height);
    const GuiWidgetSize size{width, height};
    set_gui_widget_current_size58(highlight(), size);
}
} // namespace bsp
