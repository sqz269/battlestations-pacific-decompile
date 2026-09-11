#pragma once
#include "bsp/gui_widget_detach.hpp"

namespace bsp {
// AAA5A0 append branch, insert_position=null. Transfer the same detached C++
// allocation into the parent's existing owning list and borrowed GUI list,
// publish GUI parent, then call actual node parenting. Null handle is inert.
// Existing-parent detachment is performed by the caller using AA83A0 and its
// returned handle. No copy or second widget tree is made. Successful C++ vector
// allocations are the domain; native std::list/SEH ABI is not reproduced.
// On a later parenting exception the child stays published under the parent;
// the consumed handle must not be reused. Both actual node owners are required
// at the native call phase. No bounds, visibility or current74/78 is added.
void append_gui_widget_child_00aaa5a0(GuiWidgetOwnerRuntime&,
    NativeNodeParentingRuntime&, GuiLayoutWidget& parent,
    std::unique_ptr<GuiLayoutWidget>& detached_child);

// AA6BC0: ECX widget, borrowed listener and low-byte flag on stack, RET8.
// Publish+DC then+79; no retain, call or propagation.
void set_gui_widget_listener_00aa6bc0(GuiWidgetOwner&, void* listener,
    std::uint8_t raw_flag) noexcept;

// A9E0B0 Text/base current30: sequential x87 pair copy to+18/+1C, then
// actual AA7220; RET4. Source may alias either destination. No bounds refresh.
void set_gui_widget_pivot_00a9e0b0(GuiWidgetOwner&, const float* pair);
} // namespace bsp
