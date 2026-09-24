#pragma once
#include "bsp/native_gui_widget_bounds.hpp"
#include "bsp/native_gui_widget_transform.hpp"

namespace bsp {
// Complete AA7DC0 normal body. Original ECX widget, stack xyz pointer, RET4.
// Capture all three source DWORDs BEFORE writing widget+0C/+10/+14, preserving
// MOVSS bit copies and overlapping source/destination behavior. Then recompose
// the actual transform and refresh actual bounds, in that order. The explicit
// scratch/bindings extend the native ABI and keep both callee contracts.
// In particular, transform dispatch must execute the actual captured target
// with the current receiver; the bounds provider sees any resulting mutations.
// No logical widget, fallback, cleanup, parent-chain traversal or game binding.
void set_native_gui_widget_local_position_00aa7dc0(void* actual_widget,
    const void* actual_xyz, NativeGuiWidgetTransformScratch&,
    NativeGuiWidgetTransformBindings&, GuiWidgetBounds& bounds_scratch,
    const volatile double& actual_half_00d7a280);
} // namespace bsp
