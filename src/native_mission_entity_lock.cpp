#include "bsp/native_mission_entity_lock.hpp"

#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission entity lock requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t mission_lock_profile = 0x00ce7548u;
constexpr std::uint32_t singleton_base_profile = 0x00ce3818u;
NativeMissionEntityLockOwner* volatile mission_lock_publication_00f878fc = nullptr;
} // namespace

NativeMissionEntityLockOwner* construct_native_mission_entity_lock_004bd150(
    void* storage, NativeMissionEntityLockOwner* volatile& publication) {
    auto* const owner = ::new (storage) NativeMissionEntityLockOwner;
    owner->native_vtable_00 = mission_lock_profile;
    try {
        owner->section_04 = create_native_tracked_critical_section_00bd1860();
    } catch (...) {
        unwind_native_mission_entity_lock_004b7ed0(*owner, publication);
        throw;
    }
    return owner;
}

void unwind_native_mission_entity_lock_004b7ed0(NativeMissionEntityLockOwner& owner,
    NativeMissionEntityLockOwner* volatile& publication) noexcept {
    publication = nullptr;
    owner.native_vtable_00 = singleton_base_profile;
}

NativeMissionEntityLockOwner* get_native_mission_entity_lock_004c1570(
    void* volatile& manager_publication,
    NativeMissionEntityLockOwner* volatile& publication) {
    if (auto* const current = publication) return current;
    const SoundLifetimeAccess lifetime(manager_publication);
    {
        CapturedSoundLifetimeSection guard(lifetime);
        if (!publication) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, 8});
            NativeMissionEntityLockOwner* owner = nullptr;
            try {
                if (allocation)
                    owner = construct_native_mission_entity_lock_004bd150(allocation, publication);
            } catch (...) {
                singleton_lifetime_free(allocation); // C64F88, before guard cleanupC64F80
                throw;
            }
            publication = owner; // original EH allocation state1 has ended
            auto manager = lifetime.get_manager_00415350();
            manager->register_object(publication); // reread AFTER the second lookup
        }
    }
    return publication; // reload follows release of the first captured section
}

void destroy_native_mission_entity_lock_004c4860(NativeMissionEntityLockOwner& owner,
    NativeMissionEntityLockOwner* volatile& publication) noexcept {
    owner.native_vtable_00 = mission_lock_profile;
    release_native_tracked_critical_section_0041cc80(&owner.section_04);
    unwind_native_mission_entity_lock_004b7ed0(owner, publication);
}

NativeMissionEntityLockOwner* delete_native_mission_entity_lock_004c4890(
    NativeMissionEntityLockOwner* owner, std::uint32_t flags,
    NativeMissionEntityLockOwner* volatile& publication) noexcept {
    destroy_native_mission_entity_lock_004c4860(*owner, publication);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

NativeMissionEntityLockOwner* volatile& process_native_mission_entity_lock_00f878fc() noexcept {
    return mission_lock_publication_00f878fc;
}

NativeMissionEntityLockOwner* get_process_native_mission_entity_lock_004c1570(
    void* volatile& manager_publication) {
    return get_native_mission_entity_lock_004c1570(
        manager_publication, process_native_mission_entity_lock_00f878fc());
}
} // namespace bsp
