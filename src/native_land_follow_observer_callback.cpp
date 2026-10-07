#include "bsp/native_land_follow_observer_callback.hpp"
#include <stdexcept>

namespace bsp {

void native_land_follow_observer_callback_006ceed0(
    const NativeLandFollowEntryView& state, NativeObserverOwnerStorage& first,
    NativeLandFollowObserverCallbackBindings& bindings) {
    auto* const identity = bindings.observed_virtual_04(first);
    if (identity == state.watched_leader_2c) {
        state.watched_leader_2c = nullptr;
        // The ordinary admitted match is a live actual nonnull FIRST prefix.
        // No null guard substitutes for native invalid NULL==NULL behavior.
        bindings.observer_lifetime().unregister_pair_006952a0(*identity, state.callback_18);
    }
}

namespace {
void callback04(void* context, NativeObserverOwnerStorage& callback,
    std::uint32_t table, NativeObserverOwnerStorage& first) {
    auto& bindings = *static_cast<NativeLandFollowObserverCallbackBindings*>(context);
    if (table != 0x00cf89b4u) {
        bindings.other_callback_virtual_04(callback, table, first);
        return;
    }
    auto state = bindings.callback_fields(callback);
    if (&state.callback_18 != &callback ||
        reinterpret_cast<std::uintptr_t>(&state.watched_leader_2c) !=
            reinterpret_cast<std::uintptr_t>(&callback) + 0x14u)
        throw std::logic_error("native Follow slot04 requires actual callback18/watch2C");
    native_land_follow_observer_callback_006ceed0(state, first, bindings);
}
void callback08(void* context, NativeObserverOwnerStorage& callback,
    std::uint32_t table, NativeObserverOwnerStorage& first) {
    static_cast<NativeLandFollowObserverCallbackBindings*>(context)->
        callback_virtual_08(callback, table, first);
}
} // namespace

ObserverEndpointCallbackAccess native_land_follow_observer_callback_access(
    NativeLandFollowObserverCallbackBindings& bindings) noexcept {
    return {&bindings, &callback04, &callback08};
}

} // namespace bsp
