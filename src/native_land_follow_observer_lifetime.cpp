#include "bsp/native_land_follow_observer_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

NativeLandFollowObserverCleanupView native_land_follow_observer_cleanup_view(
    const NativeLandFollowEntryView& state) noexcept {
    return {state.callback_18, state.watched_leader_2c};
}

void destroy_native_land_follow_observer_006cdd70(
    const NativeLandFollowObserverCleanupView& component,
    NativeObserverLifetime& lifetime) {
    component.callback_00.native_vtable_00 = 0x00cf8900u; // CDD8D
    const void* const captured_first = component.first_endpoint_14; // CDD93
    if (captured_first != nullptr) {
        // PURE same-address actual prefix alias; no view provider callback,
        // table read, endpoint-offset translation or fresh identity reload.
        auto& first = *static_cast<NativeObserverOwnerStorage*>(
            const_cast<void*>(captured_first));
        lifetime.unregister_pair_006952a0(first, component.callback_00); // CDDA4
    }
    // CDDB3. Preserve the complete provider's final profile/array fields. In
    // particular it can restamp CE3CD4 and free a still-published array.
    lifetime.destroy_callback_owner_00695870(component.callback_00);
}

// Reused unchanged for active scalar006CEEB0's complete normal contract:
// same actual COMPONENT capture, 006CDD70 call, post-helper low-byte test,
// optional BF65AC component free and COMPONENT return. It is not an adjustor
// to Follow root+0; only a separate actual CRT component admits flags1.
NativeObserverOwnerStorage* scalar_delete_native_land_follow_observer_006cddf0(
    const NativeLandFollowObserverCleanupView& component, std::uint32_t flags,
    NativeObserverLifetime& lifetime) {
    auto* const captured_identity = &component.callback_00;
    destroy_native_land_follow_observer_006cdd70(component, lifetime); // CDDF3
    // CDDF8 native TEST low byte bit0 follows the complete ordinary call.
    // Volatile SOURCE capture retains evaluation order without a binary ABI
    // or instruction-encoding claim.
    const volatile bool release_self = (static_cast<std::uint8_t>(flags) & 1u) != 0;
    if (release_self) {
        singleton_lifetime_free(captured_identity); // CDE00
    }
    return captured_identity; // CDE08; no access after receiver free.
}

} // namespace bsp
