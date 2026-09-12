#pragma once
#include "bsp/gui_manager_frame.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {
struct Win32PlatformState;
// AA2F10 must execute its actual page walk/recursive AA8BD0 hit tests and
// publish the SAME F8BC70/F8BC74/F8BC78 globals used by widget frames.
struct GuiPointerHitCalls {
    virtual ~GuiPointerHitCalls() = default;
    virtual void hit_test_00aa2f10(GuiResourceOwner&) = 0;
};
struct GuiPointerRuntimeServices {
    GuiWidgetFrameRuntime& frames;
    GuiPointerHitCalls& hits;
    volatile std::uint32_t& latch_bits_00f8bc6c;
    volatile float& latch_x_00f8bc64;
    volatile float& latch_y_00f8bc68;
    GuiLayoutWidget* volatile& hovered_00f8bc70;
    Win32PlatformState* volatile& platform_0109cf04;
    const CameraAxesCrtAccess& crt;
    const volatile double& scale_00d5bec8;
    const volatile double& scale_00d7a308;
    const volatile float& lower_00d7a238;
    const volatile float& upper_00ce4e0c;
    const volatile float& wide_lower_00d5bec0;
    const volatile float& wide_upper_00d5bebc;
    const volatile double& vertical_upper_00ced5d0;
};
// AA5DE4/5DF9..5E06 only: current CE3800 captured once for both coordinates,
// enabled byte zero and exclusive page null. Delta fields remain unwritten.
void initialize_gui_pointer_fields_00aa5d70_fragment(GuiResourceOwner&,
    const volatile float& center_00ce3800);

// Full normal AA3910 caller, ECX manager, RET. Uses the SAME actual input
// publication as widget/Listbox frames and the manager's sole pointer fields.
// Cursor must retain its actual Icon profile across reached calls. Actual
// AA2F10 remains a required provider; no successful hit-test fallback exists.
// Original SEH/allocation failure behavior and native C++ ABI are not claimed.
class GuiPointerRuntime final : public GuiManagerPointerCalls {
public:
    explicit GuiPointerRuntime(GuiPointerRuntimeServices);
    void update_pointer_00aa3910(GuiResourceOwner&) override;
private:
    GuiPointerRuntimeServices services_;
    GuiWidgetOwner& cursor(GuiResourceOwner&) const;
};
} // namespace bsp
