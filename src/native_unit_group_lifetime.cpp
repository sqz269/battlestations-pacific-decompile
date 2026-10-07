#include "bsp/native_unit_group_lifetime.hpp"
#include "bsp/observer_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
void* delete_native_unit_group_0070d260(
    void* const group, const std::uint32_t flags, NativeObserverLifetime& lifetime) {
    const auto original_address = reinterpret_cast<std::uintptr_t>(group);
    auto& owner = *static_cast<NativeObserverOwnerStorage*>(group);
    owner.native_vtable_00 = 0x00cfd6f8u;
    lifetime.destroy_callback_owner_00695870(owner);
    if ((flags & 1u) != 0) singleton_lifetime_free(group);
    return reinterpret_cast<void*>(original_address);
}
} // namespace bsp
