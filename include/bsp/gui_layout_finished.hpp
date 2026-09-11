#pragma once
#include "bsp/gui_widget_owner.hpp"

namespace bsp {
//00A9E0A0, ECX widget ignored, RET/1. This is the actual current+24 target
//in Base/Screen/Group/Icon/FrameBox tables, not an unresolved callback stub.
void base_gui_layout_finished24_00a9e0a0(GuiWidgetOwner&) noexcept;

// Recovered live-profile+24 for Screen1/Group2/Icon6/ClipBox16/FrameBox18
// companions. Type tags must agree with the SAME owner's concrete companion.
// ClipBox calls its existing00ACE120 over its one layout/derived field set;
// unknown types or mismatched companions fail explicitly.
// Call only during a serialized live layout pass, before any widget teardown.
// GuiWidgetOwner has no actual widget-vtable word; this is not raw ABI dispatch
// or support for a native table changing during construction/destruction.
void dispatch_gui_layout_finished24(GuiWidgetOwner&);

// Install on an existing actual transform host for one00AA8710 pass. All
// platform/listener/node operations keep their original host and widget;
// only the terminal current+24 is supplied here. In particular, the wrapped
// host's on_layout_finished is not called a second time.
// The runtime and wrapped host must outlive this adapter and the pass. This
// borrows existing owners; it does not establish material/widget retention.
class GuiLayoutFinishedHost final : public GuiWidgetTransformHost {
public:
    GuiLayoutFinishedHost(GuiWidgetOwnerRuntime&, GuiWidgetTransformHost&) noexcept;
    bool widescreen_enabled() override;
    void publish_local_transform(GuiWidgetTransform&, const GuiWidgetLocalTransform&) override;
    void publish_local_bounds(GuiWidgetTransform&, const GuiWidgetBounds&) override;
    void notify_layout_changed(GuiWidgetTransform&) override;
    void on_layout_finished(GuiWidgetTransform&) override;
private:
    GuiWidgetOwnerRuntime& owners_;
    GuiWidgetTransformHost& native_host_;
};
} // namespace bsp
