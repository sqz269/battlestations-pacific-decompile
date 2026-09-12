#pragma once
#include "bsp/gui_listbox_runtime.hpp"
#include <unordered_map>

namespace bsp {
class GuiListboxListenerDispatch;
// Borrowed association for the actual listener+114 subobject identity. This
// is host dispatch metadata; the Listbox retains its one native listener slot.
class GuiListboxSelectionListener {
public:
    explicit GuiListboxSelectionListener(void* actual_identity);
    virtual ~GuiListboxSelectionListener() noexcept;
    GuiListboxSelectionListener(const GuiListboxSelectionListener&) = delete;
    GuiListboxSelectionListener& operator=(const GuiListboxSelectionListener&) = delete;
    void* selection_identity() const noexcept { return identity_; }
    virtual void call_current08(GuiWidgetOwner* selected_row, GuiWidgetOwner& listbox) = 0;
private:
    friend class GuiListboxListenerDispatch;
    void* identity_;
    GuiListboxListenerDispatch* dispatch_{};
};
// Additional slots of the SAME listener+114 subobject. Selection-only
// listeners remain valid for08; reaching04/0C requires these actual bodies.
class GuiListboxFrameListener : public GuiListboxSelectionListener {
public:
    using GuiListboxSelectionListener::GuiListboxSelectionListener;
    virtual void call_current04(GuiWidgetOwner& row, GuiWidgetOwner& listbox) = 0;
    virtual void call_current10(GuiWidgetOwner& row, GuiWidgetOwner& listbox) = 0;
    virtual void call_current0c(bool first, bool second, GuiWidgetOwner& listbox) = 0;
};
class GuiListboxDeviceActivityCalls {
public:
    virtual ~GuiListboxDeviceActivityCalls() = default;
    // The typed canonical InputDevice domain needs its actual current2C
    // provider. It must never be cast to an unrelated native allocation.
    virtual std::uint8_t device_current2c(InputDevice&) = 0;
};
// Same current08 call reached by A9C220. Bind before publishing a listener;
// unbind before destroying its owner. Reentrant calls are allowed, but active
// listeners cannot be unbound. No callback is supplied for an unknown identity.
class GuiListboxListenerDispatch final : public GuiListboxRuntimeCalls, public GuiListboxFrameCalls {
public:
    explicit GuiListboxListenerDispatch(GuiWidgetOwnerRuntime& owners) : owners_(owners) {}
    GuiListboxListenerDispatch(GuiWidgetOwnerRuntime& owners, GuiListboxDeviceActivityCalls& activity)
        : owners_(owners), activity_(&activity) {}
    ~GuiListboxListenerDispatch() noexcept override;
    GuiListboxListenerDispatch(const GuiListboxListenerDispatch&) = delete;
    GuiListboxListenerDispatch& operator=(const GuiListboxListenerDispatch&) = delete;
    GuiWidgetOwnerRuntime& owners() const noexcept { return owners_; }
    void bind_listener(GuiListboxSelectionListener&);
    void unbind_listener(GuiListboxSelectionListener&);
    void listener_current08(void* listener, GuiWidgetOwner* selected_row,
        GuiWidgetOwner& listbox) override;
    void listener_current04(void*, GuiWidgetOwner& row, GuiWidgetOwner& listbox) override;
    void listener_current10(void*, GuiWidgetOwner& row, GuiWidgetOwner& listbox) override;
    void listener_current0c(void*, bool first, bool second, GuiWidgetOwner& listbox) override;
    std::uint8_t device_current2c(InputDevice&) override;
private:
    struct ActiveCall;
    GuiWidgetOwnerRuntime& owners_;
    std::unordered_map<void*, GuiListboxSelectionListener*> listeners_;
    ActiveCall* active_{};
    GuiListboxDeviceActivityCalls* activity_{};
    GuiListboxFrameListener& frame_listener(void*, GuiWidgetOwner&);
};
} // namespace bsp
