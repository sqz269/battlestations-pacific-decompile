#include "bsp/native_gui_widget_bounds.hpp"
#include "bsp/gui_widget_owner.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI widget bounds require MSVC Win32.
#endif

namespace bsp {

void refresh_native_gui_widget_local_bounds_00aa70e0(void* widget,
    GuiWidgetBounds& scratch, const volatile double& actual_half_00d7a280) noexcept {
    auto* const bytes = static_cast<std::byte*>(widget);
    if (bytes[0x74] == std::byte{0}) return;
    void* model;
    std::memcpy(&model, bytes + 0x4c, sizeof(model));
    const auto& tail = *reinterpret_cast<const NativeModelTailStorage*>(
        static_cast<const std::byte*>(model) + 0x174);
    void* const geometry = gui_model_geometry_00b74640(tail, 0);
    void* const element = gui_geometry_element_00b732c0(geometry, 0);
    auto* const output = &scratch;
    const volatile double* const half = &actual_half_00d7a280;
    float width_spill, height_spill;
    __asm {
        mov esi, widget
        mov edx, output
        mov eax, half
        fld dword ptr [esi + 20h]
        fld qword ptr [eax]
        xorps xmm0, xmm0
        // AA710B is DC C9: ST1 *= ST0, retaining the double half in ST0.
        fmul st(1), st(0)
        fxch st(1)
        movss dword ptr [edx + 8], xmm0
        fstp dword ptr [edx]
        fld dword ptr [esi + 24h]
        // AA711E is D8 C9: ST0 *= ST1, again retaining the half below it.
        fmul st(0), st(1)
        fstp dword ptr [edx + 4]
        fld dword ptr [esi + 20h]
        fstp width_spill
        fld dword ptr [esi + 24h]
        fstp height_spill
        fld height_spill
        fld width_spill
        fcomip st(0), st(1)
        fstp st(0)
        jbe select_height
        movss xmm0, width_spill
        jmp selected
    select_height:
        movss xmm0, height_spill
    selected:
        movss height_spill, xmm0
        fmul height_spill
        fstp dword ptr [edx + 0ch]
    }
    set_gui_element_bounds_00b855b0(element, scratch);
}

} // namespace bsp
