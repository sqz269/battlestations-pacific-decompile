#include "bsp/native_projectile_pending_entity_services.hpp"

#include <cstdint>

namespace bsp {
static_assert(sizeof(int) == sizeof(std::uint32_t), "Native cause is a Win32 DWORD");

NativeProjectilePendingEntityServices::NativeProjectilePendingEntityServices(
    NativePendingEntityOwners& owners, NativePendingEntityProducerAccess& access) noexcept
    : owners_(owners), access_(access) {}

NativeObserverOwnerStorage& NativeProjectilePendingEntityServices::observed_prefix(
    const void* identity) const noexcept {
    return *static_cast<NativeObserverOwnerStorage*>(const_cast<void*>(identity));
}

void NativeProjectilePendingEntityServices::release_projectile_00926d90(
    const void* identity, int code) {
    native_pending_entity_kill_00926d90(
        owners_, const_cast<void*>(identity), static_cast<std::uint32_t>(code), access_);
}
} // namespace bsp
