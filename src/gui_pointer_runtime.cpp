#include "bsp/gui_pointer_runtime.hpp"
#include "bsp/gui_listbox_runtime.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include "bsp/native_vector2_math.hpp"
#include "bsp/platform_window.hpp"
#include <stdexcept>

namespace bsp {
namespace {
// All pointer kernels retain the image's x87/SSE boundaries. Float results
// are stored through pointers, avoiding extra x87 C++ return-value spills.
void integer_sample(std::int32_t value, float* destination) noexcept {
    __asm {
        mov eax, value
        cvtsi2ss xmm0, eax
        mov ecx, destination
        movss dword ptr [ecx], xmm0
    }
}
void subtract_sample(std::int32_t value, const volatile float* old, float* destination) noexcept {
    __asm {
        fild dword ptr value
        mov eax, old
        fsub dword ptr [eax]
        mov ecx, destination
        fstp dword ptr [ecx]
    }
}
void scale_pair(float* xy, const volatile double* factor) noexcept {
    __asm {
        mov eax, xy
        mov ecx, factor
        fld dword ptr [eax]
        fld qword ptr [ecx]
        fmul st(1), st(0)
        fxch st(1)
        fstp dword ptr [eax]
        fmul dword ptr [eax+4]
        fstp dword ptr [eax+4]
    }
}
void accumulate_pair(const float* xy, float* delta, float* position) noexcept {
    __asm {
        mov eax, xy
        mov edx, delta
        mov ecx, position
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax+4]
        fstp dword ptr [edx+4]
        fld dword ptr [ecx]
        fadd dword ptr [edx]
        fstp dword ptr [ecx]
        fld dword ptr [edx+4]
        fadd dword ptr [ecx+4]
        fstp dword ptr [ecx+4]
    }
}
bool movement_exceeds(const float* delta, const CameraAxesCrtAccess* crt,
    const volatile float* threshold) {
    bool result;
    __asm {
        mov ecx, delta
        mov edx, crt
        call native_vector2_length_00419210
        mov eax, threshold
        fld dword ptr [eax]
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        seta al
        mov result, al
    }
    return result;
}
void clamp_x(float* x, const float* lower, const float* upper) noexcept {
    __asm {
        push upper
        mov edx, lower
        mov ecx, x
        call clamp_native_float_004155b0
        mov ecx, x
        fstp dword ptr [ecx]
    }
}
void select_y(const float* position, const volatile float* lower,
    const volatile double* ceiling, const volatile float* upper, float* result) noexcept {
    __asm {
        mov eax, position
        movss xmm0, dword ptr [eax+4]
        mov eax, lower
        movss xmm1, dword ptr [eax]
        mov edx, result
        movss dword ptr [edx], xmm0
        comiss xmm1, xmm0
        jbe compare_ceiling
        movss dword ptr [edx], xmm1
        jmp selected
    compare_ceiling:
        mov eax, ceiling
        fld qword ptr [eax]
        fld dword ptr [edx]
        fcomip st(0), st(1)
        fstp st(0)
        jbe selected
        mov eax, upper
        movss xmm0, dword ptr [eax]
        movss dword ptr [edx], xmm0
    selected:
    }
}
void cursor_arguments(float* position, const float* selected_y, float* arguments) noexcept {
    __asm {
        mov eax, selected_y
        mov ecx, position
        mov edx, arguments
        fld dword ptr [eax]
        fstp dword ptr [edx+4]
        movss xmm0, dword ptr [eax]
        movss dword ptr [ecx+4], xmm0
        fld dword ptr [ecx]
        fstp dword ptr [edx]
    }
}
}
void initialize_gui_pointer_fields_00aa5d70_fragment(GuiResourceOwner& resources,
    const volatile float& center) {
    const float captured = center;
    auto& fields = resources.pointer_fields();
    fields.enabled_48 = 0;
    fields.position_5c = std::array<float, 2>{captured, captured};
    fields.exclusive_page_6c = nullptr;
}
GuiPointerRuntime::GuiPointerRuntime(GuiPointerRuntimeServices services) : services_(services) {}
GuiWidgetOwner& GuiPointerRuntime::cursor(GuiResourceOwner& resources) const {
    auto* widget = resources.state().current_cursor;
    if (!widget) throw std::logic_error("GUI pointer requires its current manager50 cursor");
    return services_.frames.widgets().owner(*widget);
}
void GuiPointerRuntime::update_pointer_00aa3910(GuiResourceOwner& resources) {
    const auto mouse = services_.frames.input_source().device(1);
    if (!mouse) return;
    mouse.require_mouse();
    auto publish_sample = [&] {
        float x, y;
        integer_sample(mouse.mouse_accumulated_x(), &x);
        integer_sample(mouse.mouse_accumulated_y(), &y);
        services_.latch_x_00f8bc64 = x; // both stores follow the Y callback
        services_.latch_y_00f8bc68 = y;
    };
    if ((services_.latch_bits_00f8bc6c & 1u) == 0) {
        services_.latch_bits_00f8bc6c = services_.latch_bits_00f8bc6c | 1u;
        publish_sample();
    }
    std::array<float, 2> sampled;
    subtract_sample(mouse.mouse_accumulated_x(), &services_.latch_x_00f8bc64, &sampled[0]);
    subtract_sample(mouse.mouse_accumulated_y(), &services_.latch_y_00f8bc68, &sampled[1]);
    publish_sample();
    auto& fields = resources.pointer_fields();
    if (fields.enabled_48 == 0) return;
    if (!fields.position_5c)
        throw std::logic_error("GUI pointer position requires the actual AA5D70 field producer");
    scale_pair(sampled.data(), &services_.scale_00d5bec8);
    scale_pair(sampled.data(), &services_.scale_00d7a308);
    if (!fields.delta_64) fields.delta_64.emplace();
    auto& position = *fields.position_5c;
    auto& delta = *fields.delta_64;
    accumulate_pair(sampled.data(), delta.data(), position.data());
    auto& visible_cursor = cursor(resources);
    if (!visible_cursor.implementation().is_visible38(visible_cursor) &&
        movement_exceeds(delta.data(), &services_.crt, &services_.lower_00d7a238))
        cursor(resources).set_visible34(true);
    float lower = services_.lower_00d7a238;
    auto* platform = services_.platform_0109cf04;
    if (!platform) throw std::logic_error("GUI pointer requires current Win32 platform");
    const bool wide = platform->widescreen;
    float upper = services_.upper_00ce4e0c;
    if (wide) {
        // AA3B0B pushes the earlier D5BEC0 load as UPPER; AA3B12 then
        // points EDX at the later D5BEBC load as LOWER after ESP changes.
        upper = services_.wide_upper_00d5bec0;
        lower = services_.wide_lower_00d5bebc;
    }
    clamp_x(&position[0], &lower, &upper);
    float selected_y;
    select_y(position.data(), &services_.lower_00d7a238,
        &services_.vertical_upper_00ced5d0, &services_.upper_00ce4e0c, &selected_y);
    auto& positioned_cursor = cursor(resources); // capture50 before argument spills
    std::array<float, 2> arguments;
    cursor_arguments(position.data(), &selected_y, arguments.data());
    set_gui_widget_local_xy_00aa7d00(positioned_cursor, arguments[0], arguments[1]);
    services_.hits.hit_test_00aa2f10(resources);
    auto* hovered = services_.hovered_00f8bc70;
    const std::int16_t state = hovered && !hovered->transform.mouse_block ? 1 : 0;
    auto& state_cursor = cursor(resources); // fresh50/current88 after hit callbacks
    auto* icon = dynamic_cast<GuiIconTypeImplementation*>(&state_cursor.implementation());
    if (!icon) throw std::logic_error("GUI pointer current88 requires its actual Icon companion");
    icon->runtime().select_state_00ab1710(state, 0, 1.0f);
}
} // namespace bsp
