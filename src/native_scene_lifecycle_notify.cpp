#include "bsp/native_scene_lifecycle_notify.hpp"

namespace bsp {

void notify_native_scene_destroyed_00926390(
    NativeSceneLifecycleView view, NativeSceneLifecycleContext& context) {
    if (view.byte_5d != 0) return;
    view.byte_5d = 1;
    view.byte_60 = 1;
    auto& first = view.alias.prefixes.observed_00;
    notify_observer_slot08_if_present_00925c90(first, context.observers);
    const std::uint32_t tail_table = first.native_vtable_00;
    context.access.call_virtual_7c(view.alias, tail_table);
}

void remove_native_scene_immediate_009263c0(
    NativeSceneLifecycleView view, NativeSceneLifecycleContext& context) {
    auto& first = view.alias.prefixes.observed_00;
    const std::uint32_t first_table = first.native_vtable_00;
    if (context.access.render_virtual_18(view.alias, first_table) != nullptr) {
        const std::uint32_t second_table = first.native_vtable_00;
        void* const captured_listener = context.controlled_listener_00e188dc;
        if (context.access.render_virtual_18(view.alias, second_table) == captured_listener)
            context.access.set_controlled_listener_004bca80(nullptr);
    }
    // Getter and detach callbacks may have changed this byte.
    if (view.byte_5e != 0) return;
    view.byte_5d = 1;
    view.byte_5e = 1;
    view.byte_5f = 1;
    view.byte_5c = 0;
    notify_observer_slot04_if_present_00925c40(first, context.observers);
    const std::uint32_t tail_table = first.native_vtable_00;
    context.access.call_virtual_80(view.alias, tail_table);
}

} // namespace bsp
