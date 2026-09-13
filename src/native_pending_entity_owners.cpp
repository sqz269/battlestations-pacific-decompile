#include "bsp/native_pending_entity_owners.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pending entity owners require MSVC Win32.
#endif

namespace bsp {
namespace {
int initialize_pending_owner(NativePendingEntityListStorage& input,
                             const NativePendingEntityCrtRegistration& registration,
                             std::uint32_t shutdown) {
    if (!registration.register_atexit)
        throw std::invalid_argument("pending entity CRT registration is unbound");
    volatile auto& owner = input;
    auto* const head = create_native_effect_deletion_sentinel_004c3200();
    owner.head_04 = head;
    owner.count_08 = 0;
    return registration.register_atexit(registration.context, shutdown);
}
} // namespace

int initialize_native_pending_destroy_owner_00cd3910(
    NativePendingEntityOwners& owners,
    const NativePendingEntityCrtRegistration& registration) {
    return initialize_pending_owner(owners.destroy_00f899a8, registration, 0x00cdf4a0u);
}

int initialize_native_pending_kill_owner_00cd3940(
    NativePendingEntityOwners& owners,
    const NativePendingEntityCrtRegistration& registration) {
    return initialize_pending_owner(owners.kill_00f899b4, registration, 0x00cdf4b0u);
}

void destroy_native_pending_destroy_owner_00cdf4a0(NativePendingEntityOwners& owners) noexcept {
    destroy_native_effect_deletion_list_004c5940(owners.destroy_00f899a8);
}

void destroy_native_pending_kill_owner_00cdf4b0(NativePendingEntityOwners& owners) noexcept {
    destroy_native_effect_deletion_list_004c5940(owners.kill_00f899b4);
}
} // namespace bsp
