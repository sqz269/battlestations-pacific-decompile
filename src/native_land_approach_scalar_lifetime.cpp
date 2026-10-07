#include "bsp/native_land_approach_scalar_lifetime.hpp"
#include <stdexcept>

namespace bsp {

void* scalar_delete_native_land_approach_009b3220(
    const NativeLandApproachCleanupView& approach, const std::uint32_t flags,
    NativeObserverLifetime& lifetime) {
    void* const identity = approach.actual_root;
    destroy_native_land_approach_009b2c80(approach, lifetime);
    // Volatile local preserves helper-before-flag-test order in the emitted
    // new SOURCE interface; no original TEST/RET4 encoding is claimed.
    const volatile bool release_self = (static_cast<std::uint8_t>(flags) & 1u) != 0;
    if (release_self) singleton_lifetime_free(identity);
    return identity;
}

void* scalar_delete_native_land_approach_from_registry_009b3030(
    const NativeLandApproachCleanupView& approach, NativeBotStateRegistryStorage* registry,
    const std::uint32_t flags, NativeObserverLifetime& lifetime) {
    // Unsigned native Win32 address subtraction only: no represented reads,
    // profile invocation, translated receiver, callback or sidecar identity.
    const std::uintptr_t adjusted = reinterpret_cast<std::uintptr_t>(registry) - 0xb8u;
    if (!registry || registry != &approach.registry_b8 ||
        adjusted != reinterpret_cast<std::uintptr_t>(approach.actual_root))
        throw std::logic_error("native approach registry adjustor requires actual ROOT+B8");
    return scalar_delete_native_land_approach_009b3220(approach, flags, lifetime);
}

} // namespace bsp
