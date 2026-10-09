#include "bsp/native_observer_member_cleanup.hpp"

#include "bsp/observer_lifetime.hpp"

namespace bsp {
namespace {

// Source exception policy only; this is not the Native EH frame or state slot.
class ObserverMemberUnregisterGuard final {
public:
    ObserverMemberUnregisterGuard(NativeObserverOwnerStorage& actual_owner,
        NativeObserverLifetime& retained_lifetime) noexcept
        : owner_(actual_owner), lifetime_(retained_lifetime) {}

    ObserverMemberUnregisterGuard(const ObserverMemberUnregisterGuard&) = delete;
    ObserverMemberUnregisterGuard& operator=(const ObserverMemberUnregisterGuard&) = delete;

    ~ObserverMemberUnregisterGuard() noexcept {
        if (armed_) lifetime_.destroy_callback_owner_00695870(owner_);
    }

    void disarm() noexcept { armed_ = false; }

private:
    NativeObserverOwnerStorage& owner_;
    NativeObserverLifetime& lifetime_;
    bool armed_{true};
};

} // namespace

void cleanup_native_observer_member_00653390(
    NativeObserverOwnerStorage& actual_member_owner,
    NativeObserverOwnerStorage* volatile& actual_endpoint_cell,
    NativeObserverLifetime& retained_lifetime) {
    actual_member_owner.native_vtable_00 = 0x00cf6494u;
    auto* const captured_endpoint = actual_endpoint_cell;
    ObserverMemberUnregisterGuard cleanup(actual_member_owner, retained_lifetime);
    if (captured_endpoint) {
        retained_lifetime.unregister_pair_006952a0(
            *captured_endpoint, actual_member_owner);
    }
    cleanup.disarm();
    retained_lifetime.destroy_callback_owner_00695870(actual_member_owner);
}

} // namespace bsp
