#include "bsp/native_pending_entity_lock.hpp"

#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pending entity lock requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t pending_lock_profile = 0x00d190c4u;
constexpr std::uint32_t singleton_base_profile = 0x00ce3818u;
NativePendingEntityLockOwner* volatile pending_lock_publication_00f899e8 = nullptr;
} // namespace

NativePendingEntityLockOwner* construct_native_pending_entity_lock_00924180(
    void* storage, NativePendingEntityLockOwner* volatile& publication) {
    auto* const owner = ::new (storage) NativePendingEntityLockOwner;
    owner->native_vtable_00 = pending_lock_profile;
    try {
        owner->section_04 = create_native_tracked_critical_section_00bd1860();
    } catch (...) {
        unwind_native_pending_entity_lock_00923660(*owner, publication);
        throw;
    }
    return owner;
}

void unwind_native_pending_entity_lock_00923660(NativePendingEntityLockOwner& owner,
    NativePendingEntityLockOwner* volatile& publication) noexcept {
    publication = nullptr;
    owner.native_vtable_00 = singleton_base_profile;
}

NativePendingEntityLockOwner* get_native_pending_entity_lock_009248d0(
    void* volatile& manager_publication,
    NativePendingEntityLockOwner* volatile& publication) {
    if (auto* const current = publication) return current;
    const SoundLifetimeAccess lifetime(manager_publication);
    {
        CapturedSoundLifetimeSection guard(lifetime);
        if (!publication) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, 8});
            NativePendingEntityLockOwner* owner = nullptr;
            try {
                if (allocation)
                    owner = construct_native_pending_entity_lock_00924180(allocation, publication);
            } catch (...) {
                singleton_lifetime_free(allocation); // CA6B68, before guard cleanupCA6B60
                throw;
            }
            publication = owner; // original EH allocation state1 has ended
            auto manager = lifetime.get_manager_00415350();
            manager->register_object(publication); // reread AFTER the second lookup
        }
    }
    return publication; // reload follows release of the first captured section
}

void destroy_native_pending_entity_lock_009256a0(NativePendingEntityLockOwner& owner,
    NativePendingEntityLockOwner* volatile& publication) noexcept {
    owner.native_vtable_00 = pending_lock_profile;
    release_native_tracked_critical_section_0041cc80(&owner.section_04);
    unwind_native_pending_entity_lock_00923660(owner, publication);
}

NativePendingEntityLockOwner* delete_native_pending_entity_lock_009256d0(
    NativePendingEntityLockOwner* owner, std::uint32_t flags,
    NativePendingEntityLockOwner* volatile& publication) noexcept {
    destroy_native_pending_entity_lock_009256a0(*owner, publication);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

NativePendingEntityLockOwner* volatile& process_native_pending_entity_lock_00f899e8() noexcept {
    return pending_lock_publication_00f899e8;
}

NativePendingEntityLockOwner* get_process_native_pending_entity_lock_009248d0(
    void* volatile& manager_publication) {
    return get_native_pending_entity_lock_009248d0(
        manager_publication, process_native_pending_entity_lock_00f899e8());
}
} // namespace bsp
