#pragma once
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_node_parenting.hpp"

namespace bsp {

// Complete normal-path00AA83A0, native ECX parent, stack child, RET4.
// Uses the existing retained widget/node objects. Null child is inert. Reloads
// node bindings after each native callback, then removes matching child-list
// entries and clears the parent pointer. It never destroys/releases the widget.
//
// Native lists borrow pointers; the C++ page owns unique_ptr children. Return
// that ownership to the caller so detachment cannot implicitly delete it.
// This storage has one owning entry per child; its borrowed transform view
// removes every matching entry, like the native list-remove helper.
// An absent child returns empty while preserving the native node/backpointer
// effects. The caller must keep the returned handle alive until reattachment
// or an actual deleting-destructor binding consumes it. Do not drop the handle
// to stand in for Text virtual04: its existing fallback uses page retirement.
// All callback-visible owners and bindings must remain alive during the call.
std::unique_ptr<GuiLayoutWidget> detach_gui_widget_child_00aa83a0(
    GuiWidgetOwnerRuntime&, NativeNodeParentingRuntime&,
    GuiLayoutWidget& parent, GuiLayoutWidget* child);

} // namespace bsp
