#include "bsp/native_land_moveto_scalar_lifetime.hpp"
#include <stdexcept>

namespace bsp {

void* scalar_delete_native_land_moveto_009c3d10(
    const NativeLandMoveToConstructorView& state, const std::uint32_t flags,
    NativeObserverLifetime& lifetime) {
    void* const identity = state.actual_root;
    destroy_native_land_moveto_007b65e0(native_land_moveto_cleanup_view(state), lifetime);
    // Volatile local retains the observed helper-before-flag-test order in the
    // emitted new SOURCE interface; no original TEST/RET4 encoding claim.
    const volatile bool release_self = (static_cast<std::uint8_t>(flags) & 1u) != 0;
    if (release_self) singleton_lifetime_free(identity);
    return identity;
}

void* scalar_delete_native_land_moveto_callback_adjustor_009c2b70(
    const NativeLandMoveToConstructorView& state, NativeObserverOwnerStorage* callback,
    const std::uint32_t flags, NativeObserverLifetime& lifetime) {
    // Address-only validation; never read the represented profile/fields or
    // translate a semantic entity pointer into an invented callback receiver.
    const std::uintptr_t adjusted = reinterpret_cast<std::uintptr_t>(callback) - 0x18u;
    if (!callback || callback != &state.callback_18 ||
        adjusted != reinterpret_cast<std::uintptr_t>(state.actual_root))
        throw std::logic_error("native MoveTo callback adjustor requires actual ROOT+18");
    return scalar_delete_native_land_moveto_009c3d10(state, flags, lifetime);
}

} // namespace bsp
