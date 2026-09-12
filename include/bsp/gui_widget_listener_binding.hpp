#pragma once
#include "bsp/gui_widget_frame_runtime.hpp"

namespace bsp {
// CEB110 base listener table. Each reached frame slot is an actual empty
// native body (RET4, or RET8 for current18), not an unresolved event stub.
// identity is the existing listener subobject; this is a new C++ adapter ABI.
class GuiBaseWidgetListener : public GuiWidgetFrameListenerOwner {
public:
    using GuiWidgetFrameListenerOwner::GuiWidgetFrameListenerOwner;
    void call_current00(GuiWidgetOwner&) override; // 004FA100
    void call_current04(GuiWidgetOwner&) override; // 004FA110
    void call_current08(GuiWidgetOwner&) override; // 004FA120
    void call_current0c(GuiWidgetOwner&) override; // 004FA130
    void call_current10(GuiWidgetOwner&) override; // 004FA140
    void call_current14(GuiWidgetOwner&) override; // 004FA150
    void call_current18(GuiWidgetOwner&, bool) override; // 004FA160
};
} // namespace bsp
