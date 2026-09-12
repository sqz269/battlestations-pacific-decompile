#pragma once
#include "bsp/gui_pointer_runtime.hpp"

namespace bsp {
struct GuiPointerHitRuntimeServices {
    GuiWidgetFrameRuntime& frames;
    GuiLayoutWidget* volatile& hovered_00f8bc70;
    volatile float& pointer_x_00f8bc74;
    volatile float& pointer_y_00f8bc78;
    volatile float& hit_depth_00f8bc7c;
    const volatile float& initial_depth_00ce4970;
    const volatile double& z_bias_00d7a258;
};

// Complete normal AA2F10/AA8BD0 callers on the existing canonical GUI owner
// domain. Bind the SAME F8BC70/74/78 publications used by pointer/widget frames.
// The registry and owning layout/transform child vectors are traversed live;
// no second page list, tree, input owner or hit publication is constructed.
// Child membership/order, owners and active iterator storage must survive
// callbacks, matching the existing frame runtime's projected-tree domain.
// Native checked-STL corruption, native EH and binary ABI are not implemented.
class GuiPointerHitRuntime final : public GuiPointerHitCalls {
public:
    explicit GuiPointerHitRuntime(GuiPointerHitRuntimeServices);
    // Native ECX manager, no stack arguments, RET; body AA2F10..AA3054.
    void hit_test_00aa2f10(GuiResourceOwner&) override;
    // Native ECX widget, borrowed three-float origin on stack, RET4;
    // body AA8BD0..AA8E32. Children precede self; lowest resolved Z wins,
    // and equal depths preserve the first hit. No hidden/type fallback.
    void hit_test_widget_00aa8bd0(GuiWidgetOwner&, const GuiWidgetPoint& origin);
private:
    GuiPointerHitRuntimeServices services_;
};
} // namespace bsp
