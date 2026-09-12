#pragma once
#include "bsp/gui_widget_owner.hpp"

namespace bsp {
// Actual AA68F0/AA6980 over the same live canonical Model, mesh, section0 and
// material. Missing geometry/zero sections uses base color; unsupported node
// profiles are rejected before interpreting their +180 as a mesh pointer.
// Original ECX widget, float4 output / float alpha stack, RET4. New C++ ABI.
// Getter preserves native DWORD load/store order, including valid aliasing.
float* read_gui_widget_color_00aa68f0(GuiWidgetOwner&, float (&output)[4]);
void set_gui_widget_alpha_00aa6980(GuiWidgetOwner&, float alpha);
bool gui_widget_has_base_color54_profile(GuiWidgetType) noexcept;
bool gui_widget_has_base_alpha4c_profile(GuiWidgetType) noexcept;
} // namespace bsp
