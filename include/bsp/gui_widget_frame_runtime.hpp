#pragma once
#include "bsp/gui_widget_owner.hpp"
#include "bsp/gui_timed_entry_types.hpp"
#include "bsp/input_action_classifier.hpp"
#include <unordered_map>

namespace bsp {
class MouseInputDevice;

// An adapter for ONE existing listener owner/subobject identity. These names
// deliberately identify native slots: individual listener target bodies are
// not reconstructed here. Every reached call must invoke that actual owner's
// implementation; there are no default/no-op event handlers or listener state.
class GuiWidgetFrameListenerOwner {
public:
    explicit GuiWidgetFrameListenerOwner(void* actual_identity);
    virtual ~GuiWidgetFrameListenerOwner() = default;
    void* actual_identity() const noexcept { return identity_; }
    virtual void call_current04(GuiWidgetOwner&) = 0;
    virtual void call_current0c(GuiWidgetOwner&) = 0;
    virtual void call_current10(GuiWidgetOwner&) = 0;
    virtual void call_current14(GuiWidgetOwner&) = 0;
    virtual void call_current18(GuiWidgetOwner&, bool) = 0;
private:
    void* identity_;
};

struct GuiWidgetFrameServices {
    GuiWidgetOwnerRuntime& widgets;
    const GuiTimedEntryConstants& timed;
    const volatile float& zero_00d7a218;
    const volatile float& pointer_x_00f8bc74;
    const volatile float& pointer_y_00f8bc78;
    // Reference to the actual current backend's canonical device-group binding.
    // Re-read after current18, at AA8972. No copied input sample/vector.
    InputBindingDeviceGroups* const& input_groups_00f8bbf4;
};

bool gui_widget_uses_base_frame40_profile(GuiWidgetType) noexcept;
// Actual A9A380: FLD [ECX+238h]; RET, established from the seven image bytes.
float gui_mouse_double_click_seconds_00a9a380(const MouseInputDevice&) noexcept;
// Actual A9E120: RET10h, all four borrowed bounds remain untouched.
void gui_widget_base_bounds64_00a9e120(float&, float&, float&, float&) noexcept;

// Complete AA87B0 normal sequence on the documented canonical-owner domain.
// Child count is captured once; child identities use the SAME owning layout
// and transform lists, whose membership/order and owners must survive callbacks.
// Current40 profiles with derived tails (including Icon AB1150) fail explicitly.
// Listener targets are mandatory bound adapters, not recovered native bodies.
// Callback exceptions propagate after completed effects; there is no suspended
// native EH frame, automatic retry, rollback, or resume-after-callback protocol.
class GuiWidgetFrameRuntime final {
public:
    explicit GuiWidgetFrameRuntime(GuiWidgetFrameServices);
    ~GuiWidgetFrameRuntime() noexcept;
    GuiWidgetFrameRuntime(const GuiWidgetFrameRuntime&) = delete;
    GuiWidgetFrameRuntime& operator=(const GuiWidgetFrameRuntime&) = delete;
    GuiWidgetOwnerRuntime& widgets() const noexcept { return services_.widgets; }
    void bind_listener(GuiWidgetFrameListenerOwner&);
    void unbind_listener(GuiWidgetFrameListenerOwner&);
    bool operation_active(const GuiWidgetOwner&) const noexcept;
    void update40(GuiWidgetOwner&, float seconds);
    // Direct base call for proven derived continuations; it does not dispatch40.
    void update_base_00aa87b0(GuiWidgetOwner&, float seconds);
    bool contains_pointer_00aa6a40(GuiWidgetOwner&);
private:
    struct ActiveFrame;
    GuiWidgetFrameServices services_;
    // Only borrowed identity associations and call-lifetime metadata.
    std::unordered_map<void*, GuiWidgetFrameListenerOwner*> listeners_;
    ActiveFrame* active_{};
    GuiWidgetFrameListenerOwner& listener(GuiWidgetOwner&) const;
    void align_bounds64(GuiWidgetOwner&, float&, float&, float&, float&);
    void listener_tail(GuiWidgetOwner&);
};
} // namespace bsp
