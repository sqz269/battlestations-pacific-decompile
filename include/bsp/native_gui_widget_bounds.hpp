#pragma once
#include "bsp/gui_widget.hpp"
#include <cstddef>

namespace bsp {

static_assert(sizeof(GuiWidgetBounds) == 16);
static_assert(offsetof(GuiWidgetBounds, half_width) == 0);
static_assert(offsetof(GuiWidgetBounds, half_height) == 4);
static_assert(offsetof(GuiWidgetBounds, half_depth) == 8);
static_assert(offsetof(GuiWidgetBounds, radius) == 12);

// Complete AA70E0 normal body. Original ECX raw widget, RET; source adds
// caller-owned float4 scratch and the borrowed live double at D7A280. Gate
// byte+74==0 leaves scratch and the entire provider chain untouched. Otherwise
// current widget+4C names an actual generated model (tail+174, geometry+180),
// whose geometry+54 points to an element array with element0 available.
// Widget dimensions at+20/+24 are loaded/reloaded in native x87 order. The
// final provider writes element+24,+28,+2C,+30 with sequential x87 transfers,
// retaining valid element/widget alias effects. Scratch represents the native
// local frame: provide distinct writable storage, not overlapping widget,
// model, geometry, element or the live half constant. No initialization is
// imposed on it. Enabled calls write z,x,y,radius in that order.
// No logical GuiWidgetOwner, ownership change, validation/fallback, native ABI,
// unmasked FP fault, or game/application binding is supplied by this interface.
void refresh_native_gui_widget_local_bounds_00aa70e0(void* actual_widget,
    GuiWidgetBounds& scratch, const volatile double& actual_half_00d7a280) noexcept;

} // namespace bsp
