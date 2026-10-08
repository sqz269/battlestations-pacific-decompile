// Compiled once through native_pending_entity_lock.cpp, a registered core TU.
#include "bsp/native_entity_registry_lifetime.hpp"
#include "bsp/effect_deletion_queue.hpp"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t pending_init_profile = 0x00d190c0u;
constexpr std::uint32_t scene_registry_profile = 0x00d19284u;

template<class T> T& registry_field(void* owner, std::uint32_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::byte*>(owner) + offset);
}
template<class T> const T& registry_field(const void* owner, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const T*>(static_cast<const std::byte*>(owner) + offset);
}

class CapturedRegistryPendingSection final {
public:
    explicit CapturedRegistryPendingSection(TrackedCriticalSection* section) : section_(section) {
        if (section_) {
            EnterCriticalSection(&section_->native);
            change_depth(1u);
        }
    }
    ~CapturedRegistryPendingSection() {
        if (section_) {
            change_depth(0xffffffffu);
            LeaveCriticalSection(&section_->native);
        }
    }
    CapturedRegistryPendingSection(const CapturedRegistryPendingSection&) = delete;
    CapturedRegistryPendingSection& operator=(const CapturedRegistryPendingSection&) = delete;
private:
    void change_depth(std::uint32_t increment) noexcept {
        std::uint32_t bits;
        std::memcpy(&bits, &section_->depth, sizeof bits);
        bits += increment;
        std::memcpy(&section_->depth, &bits, sizeof bits);
    }
    TrackedCriticalSection* section_;
};

void pending_init_shutdown_00cdf4c0() noexcept {
    destroy_native_pending_init_owner_00cdf4c0(native_entity_registry_process());
}
void entity_id_pair_shutdown_00cdf500() noexcept {
    destroy_native_entity_id_pair_00cdf500(native_entity_registry_process());
}
} // namespace

NativeEntityRegistryProcess& native_entity_registry_process() noexcept {
    // Constant initialization and trivial destruction: permanent zero-filled
    // owner, no allocation, implicit CRT registration or duplicated teardown.
    static NativeEntityRegistryProcess process;
    return process;
}

ObjectHandleTables NativeEntityRegistryProcess::handle_tables() const noexcept {
    const auto* primary = &ids_.primary_00f89a08;
    const auto* alternate = &ids_.alternate_00f89a5c;
    return {registry_field<std::int32_t>(primary, 4),
            registry_field<std::int32_t>(primary, 8),
            registry_field<const void*>(primary, 0x4c),
            registry_field<std::int32_t>(alternate, 4),
            registry_field<const void*>(alternate, 0x4c)};
}

NativeEntityRegistryCrtStatuses NativeEntityRegistryProcess::initialize_crt_once() {
    if (startup_ == Startup::complete) return statuses_;
    if (startup_ != Startup::cold)
        throw std::logic_error("native entity registry startup cannot replay a partial initialization");
    startup_ = Startup::attempted;
    statuses_.pending_init = initialize_native_pending_init_owner_00cd39a0(*this);
    statuses_.id_pair = initialize_native_entity_id_pair_00cd3a70(*this);
    startup_ = Startup::complete;
    return statuses_;
}

NativeEntityRegistryLockOwner* construct_native_pending_init_lock_00924100(
    void* storage, NativeEntityRegistryLockOwner* volatile& publication) {
    auto* const owner = ::new (storage) NativeEntityRegistryLockOwner;
    owner->native_vtable_00 = pending_init_profile;
    try {
        owner->section_04 = create_native_tracked_critical_section_00bd1860();
    } catch (...) {
        unwind_native_pending_init_lock_00923640(*owner, publication);
        throw;
    }
    return owner;
}
void unwind_native_pending_init_lock_00923640(NativeEntityRegistryLockOwner& owner,
    NativeEntityRegistryLockOwner* volatile& publication) noexcept {
    publication = nullptr;
    owner.native_vtable_00 = singleton_base_profile;
}
NativeEntityRegistryLockOwner* construct_native_scene_registry_lock_00928080(
    void* storage, NativeEntityRegistryLockOwner* volatile& publication) {
    auto* const owner = ::new (storage) NativeEntityRegistryLockOwner;
    owner->native_vtable_00 = scene_registry_profile;
    try {
        owner->section_04 = create_native_tracked_critical_section_00bd1860();
    } catch (...) {
        unwind_native_scene_registry_lock_00927a60(*owner, publication);
        throw;
    }
    return owner;
}
void unwind_native_scene_registry_lock_00927a60(NativeEntityRegistryLockOwner& owner,
    NativeEntityRegistryLockOwner* volatile& publication) noexcept {
    publication = nullptr;
    owner.native_vtable_00 = singleton_base_profile;
}

NativeEntityRegistryLockOwner* get_native_pending_init_lock_00924810(
    void* volatile& manager_publication, NativeEntityRegistryLockOwner* volatile& publication) {
    if (auto* const current = publication) return current;
    const SoundLifetimeAccess lifetime(manager_publication);
    {
        CapturedSoundLifetimeSection guard(lifetime);
        if (!publication) {
            void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 8, 8});
            NativeEntityRegistryLockOwner* owner = nullptr;
            try {
                if (allocation) owner = construct_native_pending_init_lock_00924100(allocation, publication);
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }
            publication = owner;
            auto manager = lifetime.get_manager_00415350();
            manager->register_object(publication);
        }
    }
    return publication;
}
NativeEntityRegistryLockOwner* get_native_scene_registry_lock_00928240(
    void* volatile& manager_publication, NativeEntityRegistryLockOwner* volatile& publication) {
    if (auto* const current = publication) return current;
    const SoundLifetimeAccess lifetime(manager_publication);
    {
        CapturedSoundLifetimeSection guard(lifetime);
        if (!publication) {
            void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 8, 8});
            NativeEntityRegistryLockOwner* owner = nullptr;
            try {
                if (allocation) owner = construct_native_scene_registry_lock_00928080(allocation, publication);
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }
            publication = owner;
            auto manager = lifetime.get_manager_00415350();
            manager->register_object(publication);
        }
    }
    return publication;
}

NativeEntityRegistryLockOwner* delete_native_pending_init_lock_00925660(
    NativeEntityRegistryLockOwner* owner, std::uint32_t flags,
    NativeEntityRegistryLockOwner* volatile& publication) noexcept {
    owner->native_vtable_00 = pending_init_profile;
    release_native_tracked_critical_section_0041cc80(&owner->section_04);
    unwind_native_pending_init_lock_00923640(*owner, publication);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
NativeEntityRegistryLockOwner* delete_native_scene_registry_lock_009285f0(
    NativeEntityRegistryLockOwner* owner, std::uint32_t flags,
    NativeEntityRegistryLockOwner* volatile& publication) noexcept {
    owner->native_vtable_00 = scene_registry_profile;
    release_native_tracked_critical_section_0041cc80(&owner->section_04);
    unwind_native_scene_registry_lock_00927a60(*owner, publication);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

void enqueue_native_pending_init_00926be0(NativeEntityRegistryProcess& process,
    void* actual_entity_root, void* volatile& manager_publication) {
    const CapturedRegistryPendingSection guard(
        get_native_pending_init_lock_00924810(manager_publication, process.pending_lock_00f899e4())->section_04);
    auto& list = process.pending_00f899cc();
    auto* const head = list.head_04;
    auto* const previous = head->previous_04;
    void* const root_pointer_cell = actual_entity_root;
    auto* const node = create_effect_deletion_node_008665f0(head, previous, &root_pointer_cell);
    grow_effect_deletion_list_count_008675e0(list, 1);
    head->previous_04 = node;
    node->previous_04->next_00 = node;
}

int initialize_native_pending_init_owner_00cd39a0(NativeEntityRegistryProcess& process) {
    volatile auto& owner = process.pending_00f899cc();
    auto* const head = create_native_effect_deletion_sentinel_004c3200();
    owner.head_04 = head;
    owner.count_08 = 0;
    return std::atexit(&pending_init_shutdown_00cdf4c0);
}
void destroy_native_pending_init_owner_00cdf4c0(NativeEntityRegistryProcess& process) noexcept {
    destroy_native_effect_deletion_list_004c5940(process.pending_00f899cc());
}

void destroy_native_entity_id_table_009516b0(void* owner) noexcept {
    void* const slots = registry_field<void* volatile>(owner, 0x4c);
    registry_field<const NativeEntityIdTableProfile* volatile>(owner, 0) =
        &native_entity_id_table_profile_00d19b88();
    singleton_lifetime_free(slots);
}
int initialize_native_entity_id_pair_00cd3a70(NativeEntityRegistryProcess& process) {
    auto& pair = process.ids();
    construct_native_entity_id_table_00951660(&pair.primary_00f89a08, 0, 0x1000);
    try {
        construct_native_entity_id_table_00951660(&pair.alternate_00f89a5c, 0x1000, 0x1000);
        return std::atexit(&entity_id_pair_shutdown_00cdf500);
    } catch (...) {
        destroy_native_entity_id_table_009516b0(&pair.primary_00f89a08);
        throw;
    }
}
void destroy_native_entity_id_pair_00cdf500(NativeEntityRegistryProcess& process) noexcept {
    auto& pair = process.ids();
    destroy_native_entity_id_table_009516b0(&pair.alternate_00f89a5c);
    destroy_native_entity_id_table_009516b0(&pair.primary_00f89a08);
}
} // namespace bsp
