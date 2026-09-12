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
// Same current08 call reached by A9C220. Bind before publishing a listener;
// unbind before destroying its owner. Reentrant calls are allowed, but active
// listeners cannot be unbound. No callback is supplied for an unknown identity.
class GuiListboxListenerDispatch final : public GuiListboxRuntimeCalls {
public:
    explicit GuiListboxListenerDispatch(GuiWidgetOwnerRuntime& owners) : owners_(owners) {}
    ~GuiListboxListenerDispatch() noexcept override;
    GuiListboxListenerDispatch(const GuiListboxListenerDispatch&) = delete;
    GuiListboxListenerDispatch& operator=(const GuiListboxListenerDispatch&) = delete;
    GuiWidgetOwnerRuntime& owners() const noexcept { return owners_; }
    void bind_listener(GuiListboxSelectionListener&);
    void unbind_listener(GuiListboxSelectionListener&);
    void listener_current08(void* listener, GuiWidgetOwner* selected_row,
        GuiWidgetOwner& listbox) override;
private:
    struct ActiveCall;
    GuiWidgetOwnerRuntime& owners_;
    std::unordered_map<void*, GuiListboxSelectionListener*> listeners_;
    ActiveCall* active_{};
};
} // namespace bsp
