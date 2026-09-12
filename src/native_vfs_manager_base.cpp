#include "bsp/native_vfs_manager_base.hpp"

namespace bsp {
namespace {
void profile(void* owner, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(owner) = value;
}
}

void* construct_native_vfs_manager_base_00bda6f0(
    void* owner, void* volatile& actual_published_0109ceec,
    SoundLifetimeAccess lifetime) {
    profile(owner, 0x00d683e4u);
    // State0 is armed before the first getter. State1 is armed only after
    // section entry/increment succeeds, by completion of the captured guard.
    try {
        CapturedSoundLifetimeSection section(lifetime);
        actual_published_0109ceec = owner;
        auto manager = lifetime.get_manager_00415350();
        manager->register_object(actual_published_0109ceec);
    } catch (...) {
        // CC5FD8 releases the captured section before CC5FD0 -> 412430.
        // Publication remains whatever was last written, including callbacks.
        profile(owner, 0x00ce3818u);
        throw;
    }
    return owner;
}

void destroy_native_vfs_manager_base_00bda790(
    void* owner, void* volatile& actual_published_0109ceec,
    SoundLifetimeAccess lifetime) {
    profile(owner, 0x00d683e4u);
    try {
        CapturedSoundLifetimeSection section(lifetime);
        auto manager = lifetime.get_manager_00415350();
        // Removal follows current publication, not the captured owner.
        manager->unregister_object(actual_published_0109ceec);
        actual_published_0109ceec = nullptr;
    } catch (...) {
        // CC5FF8 -> 411EE0 then CC5FF0 -> 412430. A removal failure skips
        // the normal publication clear; profile cleanup still targets owner.
        profile(owner, 0x00ce3818u);
        throw;
    }
    profile(owner, 0x00ce3818u);
}

void* delete_native_vfs_manager_base_00bda8e0(
    void* owner, std::uint32_t flags, void* volatile& actual_published_0109ceec,
    SoundLifetimeAccess lifetime) {
    destroy_native_vfs_manager_base_00bda790(owner, actual_published_0109ceec, lifetime);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

} // namespace bsp
