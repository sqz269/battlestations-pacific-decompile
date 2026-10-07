#include "bsp/native_land_approach_lifetime.hpp"
#include <limits>
#include <stdexcept>

namespace bsp {

NativeLandApproachCleanupView native_land_approach_cleanup_view(
    void* actual_root, const std::size_t bytes,
    volatile std::uint32_t& profile, NativeBotStateRegistryStorage& registry,
    const NativeLandMoveToCleanupView& moveto,
    const NativeLandStateCleanupView& follow,
    const NativeLandFollowObserverCleanupView& observer,
    const std::array<NativeLandStateCleanupView, 6>& trailing) {
    const auto root = reinterpret_cast<std::uintptr_t>(actual_root);
    const auto at = [root](const volatile void* field, const std::size_t offset) {
        return reinterpret_cast<std::uintptr_t>(field) == root + offset;
    };
    const auto shared_at = [&at](const NativeLandStateCleanupView& state,
        const std::size_t offset) {
        return at(&state.profile_00, offset)
            && at(&state.elements_0c, offset + 0x0c);
    };
    constexpr std::size_t offsets[]{0x254, 0x228, 0x200, 0x1e0, 0x1c0, 0x1a0};
    bool coherent = actual_root && bytes >= 0x26c
        && root <= (std::numeric_limits<std::uintptr_t>::max)() - 0x26c
        && at(&profile, 0) && at(&registry, 0xb8)
        && shared_at(moveto.state, 0xcc) && at(&moveto.callback_18, 0xe4)
        && shared_at(follow, 0x108) && at(&observer.callback_00, 0x120)
        && at(&observer.first_endpoint_14, 0x134);
    for (std::size_t i = 0; i != trailing.size(); ++i)
        coherent = coherent && shared_at(trailing[i], offsets[i]);
    if (!coherent)
        throw std::logic_error("native approach cleanup requires same-root >=26Ch fields");
    return {actual_root, profile, registry, moveto, follow, observer, trailing};
}

void destroy_native_land_approach_009b2c80(
    const NativeLandApproachCleanupView& approach, NativeObserverLifetime& lifetime) {
    // 2CAE/2CBE/2CCE/2CDE/2CEE/2CFE, on +254/+228/+200/+1E0/+1C0/+1A0.
    for (const auto& state : approach.trailing_states)
        destroy_native_land_state_007b45f0(state, lifetime);
    destroy_native_land_follow_observer_006cdd70(
        approach.follow_observer_120, lifetime); // 2D15, actual Follow+18.
    destroy_native_land_state_007b45f0(approach.follow_108, lifetime); // 2D21.
    destroy_native_land_moveto_007b65e0(approach.moveto_cc, lifetime); // 2D31.
    // Native 2D38..46 NULL placement is outside the admitted nonnull domain.
    destroy_native_bot_state_registry_004116d0(approach.registry_b8); // 2D46.
    approach.profile_00 = 0x00d1fdb8u; // 2D4F, after ALL complete providers.
}

} // namespace bsp
