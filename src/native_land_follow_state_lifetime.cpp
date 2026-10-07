#include "bsp/native_land_follow_state_lifetime.hpp"
#include <stdexcept>

namespace bsp {

NativeLandFollowStateCleanupView native_land_follow_state_cleanup_view(
    void* const actual_root, const std::size_t actual_backing_bytes,
    const NativeLandStateCleanupView& shared, const NativeLandFollowEntryView& entry) {
    const auto root = reinterpret_cast<std::uintptr_t>(actual_root);
    const auto aliases = [root](const volatile void* field, const std::size_t offset) {
        return reinterpret_cast<std::uintptr_t>(field) == root + offset;
    };
    if (!actual_root || actual_backing_bytes < 0x98
        || !aliases(&shared.profile_00, 0)
        || !aliases(&shared.elements_0c, 0x0c)
        || !aliases(&entry.approach_04, 4)
        || !aliases(&entry.callback_18, 0x18)
        || !aliases(&entry.watched_leader_2c, 0x2c)
        || !aliases(&entry.parameters_6c, 0x6c)
        || !aliases(&entry.flag_84, 0x84)
        || !aliases(&entry.flag_85, 0x85)
        || !aliases(&entry.amplitude_88, 0x88)
        || !aliases(&entry.field_8c, 0x8c)
        || !aliases(&entry.field_90, 0x90)
        || !aliases(&entry.field_94, 0x94))
        throw std::logic_error("incoherent actual native Follow state view");
    return {actual_root, shared, native_land_follow_observer_cleanup_view(entry)};
}

void destroy_native_land_follow_state_007b6630(
    const NativeLandFollowStateCleanupView& state, NativeObserverLifetime& lifetime) {
    destroy_native_land_follow_observer_006cdd70(state.observer, lifetime);
    destroy_native_land_state_007b45f0(state.shared, lifetime);
}

void* scalar_delete_native_land_follow_state_009c2a60(
    const NativeLandFollowStateCleanupView& state, const std::uint32_t flags,
    NativeObserverLifetime& lifetime) {
    void* const identity = state.actual_root;
    destroy_native_land_follow_state_007b6630(state, lifetime);
    const volatile bool release_self = (static_cast<std::uint8_t>(flags) & 1u) != 0;
    if (release_self) singleton_lifetime_free(identity);
    return identity;
}

} // namespace bsp
