#include "bsp/native_land_moveto_observer_callback.hpp"
#include <stdexcept>

namespace bsp {

void native_land_moveto_observer_callback_009bdeb0(
    const NativeLandMoveToConstructorView& state, NativeObserverOwnerStorage& first,
    NativeLandMoveToObserverCallbackBindings& bindings) {
    auto* const identity = bindings.observed_virtual_04(first);
    if (identity == state.first_endpoint_2c) state.first_endpoint_2c = nullptr;
}

namespace {
void callback04(void* context, NativeObserverOwnerStorage& callback,
    std::uint32_t table, NativeObserverOwnerStorage& first) {
    auto& bindings = *static_cast<NativeLandMoveToObserverCallbackBindings*>(context);
    if (table != 0x00d20ad4u) {
        bindings.other_callback_virtual_04(callback, table, first);
        return;
    }
    auto state = bindings.callback_fields(callback);
    if (&state.callback_18 != &callback ||
        reinterpret_cast<std::uintptr_t>(&callback) - 0x18u !=
            reinterpret_cast<std::uintptr_t>(state.actual_root))
        throw std::logic_error("native MoveTo slot04 requires actual callback18/root2C");
    native_land_moveto_observer_callback_009bdeb0(state, first, bindings);
}
void callback08(void* context, NativeObserverOwnerStorage& callback,
    std::uint32_t table, NativeObserverOwnerStorage& first) {
    static_cast<NativeLandMoveToObserverCallbackBindings*>(context)->
        callback_virtual_08(callback, table, first);
}
} // namespace

ObserverEndpointCallbackAccess native_land_moveto_observer_callback_access(
    NativeLandMoveToObserverCallbackBindings& bindings) noexcept {
    return {&bindings, &callback04, &callback08};
}

} // namespace bsp
