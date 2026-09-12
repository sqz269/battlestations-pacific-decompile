#include "bsp/gui_listbox_listener_binding.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
GuiListboxSelectionListener::GuiListboxSelectionListener(void* identity) : identity_(identity) {
    if (!identity) throw std::invalid_argument("Listbox listener requires its actual subobject identity");
}
GuiListboxSelectionListener::~GuiListboxSelectionListener() noexcept {
    if (dispatch_) std::terminate();
}
struct GuiListboxListenerDispatch::ActiveCall {
    GuiListboxListenerDispatch& dispatch;
    GuiListboxSelectionListener& listener;
    ActiveCall* previous;
    ActiveCall(GuiListboxListenerDispatch& owner, GuiListboxSelectionListener& target)
        : dispatch(owner), listener(target), previous(owner.active_) { owner.active_ = this; }
    ~ActiveCall() { dispatch.active_ = previous; }
};
GuiListboxListenerDispatch::~GuiListboxListenerDispatch() noexcept {
    if (active_ || !listeners_.empty()) std::terminate();
}
void GuiListboxListenerDispatch::bind_listener(GuiListboxSelectionListener& listener) {
    if (listener.dispatch_ || listeners_.count(listener.identity_))
        throw std::logic_error("Listbox listener identity is already bound");
    listeners_.emplace(listener.identity_, &listener);
    listener.dispatch_ = this;
}
void GuiListboxListenerDispatch::unbind_listener(GuiListboxSelectionListener& listener) {
    if (listener.dispatch_ != this)
        throw std::logic_error("Listbox listener is not bound to this dispatch owner");
    for (auto* call = active_; call; call = call->previous)
        if (&call->listener == &listener)
            throw std::logic_error("Cannot unbind a Listbox listener during its callback");
    const auto found = listeners_.find(listener.identity_);
    if (found == listeners_.end() || found->second != &listener)
        throw std::logic_error("Listbox listener identity no longer matches its bound owner");
    listeners_.erase(found);
    listener.dispatch_ = nullptr;
}
void GuiListboxListenerDispatch::listener_current08(void* identity,
    GuiWidgetOwner* selected_row, GuiWidgetOwner& listbox) {
    if (&listbox.runtime() != &owners_ || listbox.layout().type != GuiWidgetType::Listbox ||
        (selected_row && &selected_row->runtime() != &owners_))
        throw std::logic_error("Listbox callback requires the same canonical widget owners");
    const auto found = listeners_.find(identity);
    if (found == listeners_.end())
        throw std::logic_error("Current Listbox listener08 has no bound implementation");
    auto& listener = *found->second;
    ActiveCall call(*this, listener);
    listener.call_current08(selected_row, listbox);
}
} // namespace bsp
