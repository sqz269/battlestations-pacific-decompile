#pragma once
#include "bsp/gui_widget_owner.hpp"

namespace bsp {
// Required aliases of the live GUI constants. No state or resource copies.
struct GuiWidgetRelativeBoundsConstants {
    const volatile double& half_00d7a280;
    const volatile double& width_divisor_00cec380;
    const volatile float& zero_00d7a218;
    const volatile float& padding_x_00ce9bac;
    const volatile double& depth_offset_00d7a210;
};
// AC06C0: ECX source, EDX size output, stack offset/padding/width, RET0C.
// Both outputs and padding remain live, including their native alias ordering.
// Type3 requires the same actual Text lifetime and completed content domain.
// Native bottom adjustment tests saved HORIZONTAL align==2, not vertical==2.
void measure_gui_widget_padded_00ac06c0(GuiWidgetOwner& source,
    GuiWidgetSize& size, GuiWidgetSize& offset, const GuiWidgetSize& padding,
    float width, const GuiWidgetRelativeBoundsConstants&);
// AC0820: ECX source, EDX destination, float width stack, RET4. Calls actual
// current58 before reading both live pivots/sizes and resolved source position.
void fit_gui_widget_to_source_00ac0820(GuiWidgetOwner& source,
    GuiWidgetOwner& destination, float width, const GuiWidgetRelativeBoundsConstants&);
void set_gui_widget_current_size58(GuiWidgetOwner&, const GuiWidgetSize&);
// Whole established AA8240 composition through this owner's actual scene.
void set_gui_widget_resolved_position_00aa8240(GuiWidgetOwner&, const GuiWidgetPoint&);
// 580820: ECX same menu screen, selected objective stack, RET4. Reload screen
// +294 after every callback. Height adds live double CEE4E8 after fitting.
void position_main_menu_highlight_00580820(GuiWidgetOwner& selected_objective,
    GuiWidgetOwnerRuntime&, GuiLayoutWidget*& highlight_294,
    const GuiWidgetRelativeBoundsConstants&, const volatile double& extra_height_00cee4e8);
// Caller retains all owners, type profiles and borrowed storage across
// callbacks. New C++ interfaces, not original callback/SEH/native object ABI.
// Text current58 must complete synchronously. Its existing pending exception
// stops this caller; resuming Text alone does not resume this outer operation.
} // namespace bsp
