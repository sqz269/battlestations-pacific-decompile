#include "bsp/observer_event_producer.hpp"
#include "bsp/observer_dispatch_schedule.hpp"
#include <cstring>

namespace bsp {
namespace {
void depth_add(TrackedCriticalSection& section, std::uint32_t amount) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &section.depth, 4);
    bits += amount;
    std::memcpy(&section.depth, &bits, 4);
}
void selected04(void* context, NativeObserverOwnerStorage& first,
    NativeObserverOwnerStorage& callback_owner) {
    auto& delivery = *static_cast<ObserverEventDeliveryContext*>(context);
    invoke_observer_slot04_00693550(first, callback_owner, delivery.callbacks);
}
void selected08(void* context, NativeObserverOwnerStorage& first,
    NativeObserverOwnerStorage& callback_owner) {
    auto& delivery = *static_cast<ObserverEventDeliveryContext*>(context);
    invoke_observer_slot08_00693560(first, callback_owner, delivery.callbacks);
}
} // namespace

void invoke_observer_slot04_00693550(NativeObserverOwnerStorage& first,
    NativeObserverOwnerStorage& callback_owner, const ObserverEndpointCallbackAccess& access) {
    const std::uint32_t table = callback_owner.native_vtable_00;
    access.callback_virtual_04(access.context, callback_owner, table, first);
}
void invoke_observer_slot08_00693560(NativeObserverOwnerStorage& first,
    NativeObserverOwnerStorage& callback_owner, const ObserverEndpointCallbackAccess& access) {
    const std::uint32_t table = callback_owner.native_vtable_00;
    access.callback_virtual_08(access.context, callback_owner, table, first);
}

void notify_observer_slot04_00696330(NativeObserverOwnerStorage& first,
    ObserverEventDeliveryContext& delivery) {
    dispatch_observer_edges_00695f90(delivery.lifetime, delivery.global_00e198e4,
        first, selected04, &delivery);
}
void notify_observer_slot08_00696340(NativeObserverOwnerStorage& first,
    ObserverEventDeliveryContext& delivery) {
    dispatch_observer_edges_00695f90(delivery.lifetime, delivery.global_00e198e4,
        first, selected08, &delivery);
}

bool sample_observer_endpoint_presence_00925c45(
    NativeObserverOwnerStorage& first, NativeObserverLifetime& lifetime) {
    auto* const section = lifetime.lock_owner_00694280()->section_04;
    if (section) {
        EnterCriticalSection(&section->native);
        depth_add(*section, 1u);
    }
    const bool present = first.edges_04.count_04 > 0u;
    if (section) {
        depth_add(*section, 0xffffffffu);
        LeaveCriticalSection(&section->native);
    }
    return present;
}

void notify_observer_slot04_if_present_00925c40(
    NativeObserverOwnerStorage& first, ObserverEventDeliveryContext& delivery) {
    if (sample_observer_endpoint_presence_00925c45(first, delivery.lifetime))
        notify_observer_slot04_00696330(first, delivery);
}
void notify_observer_slot08_if_present_00925c90(
    NativeObserverOwnerStorage& first, ObserverEventDeliveryContext& delivery) {
    if (sample_observer_endpoint_presence_00925c45(first, delivery.lifetime))
        notify_observer_slot08_00696340(first, delivery);
}

NativeObserverOwnerStorage* observer_endpoint_identity_0042b970(
    NativeObserverOwnerStorage* first) noexcept { return first; }
NativeObserverOwnerStorage* observer_endpoint_null_00522e90(
    NativeObserverOwnerStorage*) noexcept { return nullptr; }

} // namespace bsp
