#include "bsp/native_observer_member_initializer.hpp"

#include "bsp/observer_edges.hpp"
#include "bsp/observer_lifetime.hpp"

namespace bsp {
namespace {

class ObserverMemberInitializationGuard final {
public:
    ObserverMemberInitializationGuard(NativeObserverOwnerStorage& actual_owner,
        NativeObserverLifetime& retained_lifetime) noexcept
        : owner_(actual_owner), lifetime_(retained_lifetime) {}

    ObserverMemberInitializationGuard(const ObserverMemberInitializationGuard&) = delete;
    ObserverMemberInitializationGuard& operator=(const ObserverMemberInitializationGuard&) = delete;

    ~ObserverMemberInitializationGuard() noexcept {
        if (armed_) lifetime_.destroy_callback_owner_00695870(owner_);
    }

    void disarm() noexcept { armed_ = false; }

private:
    NativeObserverOwnerStorage& owner_;
    NativeObserverLifetime& lifetime_;
    bool armed_{true};
};

} // namespace

NativeObserverOwnerStorage& initialize_native_observer_member_007f0f80(
    NativeObserverOwnerStorage& actual_owner,
    volatile std::uint8_t& actual_flag_10,
    NativeObserverOwnerStorage* volatile& actual_endpoint_cell_14,
    NativeObserverOwnerStorage* const volatile& actual_incoming_endpoint_cell,
    NativeObserverLifetime& retained_lifetime) {
    // Existing Win32 storage gives three distinct volatile DWORD stores.
    volatile NativeObserverEdgeSlots& actual_slots = actual_owner.edges_04;
    actual_slots.data_00 = nullptr;
    actual_slots.count_04 = 0u;
    actual_slots.capacity_08 = 0u;
    auto* const captured_endpoint = actual_incoming_endpoint_cell;

    ObserverMemberInitializationGuard cleanup(actual_owner, retained_lifetime);
    actual_owner.native_vtable_00 = 0x00cf6494u;
    actual_endpoint_cell_14 = captured_endpoint;
    actual_flag_10 = 1u;
    if (captured_endpoint != nullptr) {
        register_observer_pair_00694a60(
            *captured_endpoint, actual_owner, retained_lifetime);
    }
    cleanup.disarm();
    return actual_owner;
}

} // namespace bsp
