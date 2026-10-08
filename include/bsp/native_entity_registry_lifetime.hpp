#pragma once

#include "bsp/native_entity_id_tables.hpp"
#include "bsp/native_pending_entity_owners.hpp"
#include "bsp/observer_lifetime.hpp"
#include <type_traits>

namespace bsp {

using NativeEntityRegistryLockOwner = NativeObserverLockOwner;

struct NativeEntityRegistryIdPair {
    NativeEntityIdTableStorage primary_00f89a08;
    NativeEntityIdTableStorage alternate_00f89a5c;
};
static_assert(sizeof(NativeEntityRegistryIdPair) == 0xa8);
static_assert(offsetof(NativeEntityRegistryIdPair, alternate_00f89a5c) == 0x54);

struct NativeEntityRegistryCrtStatuses {
    int pending_init;
    int id_pair;
};

// One permanent owner of the actual Source cells, initially loader-zero. This
// owns no entity and has no implicit native initialization or exit destructor.
// The ID headers are adjacent actual bytes, not a pair of conceptual tables.
class NativeEntityRegistryProcess final {
public:
    NativeEntityRegistryProcess(const NativeEntityRegistryProcess&) = delete;
    NativeEntityRegistryProcess& operator=(const NativeEntityRegistryProcess&) = delete;
    NativePendingEntityListStorage& pending_00f899cc() noexcept { return pending_; }
    NativeEntityRegistryIdPair& ids() noexcept { return ids_; }
    NativeEntityRegistryLockOwner* volatile& pending_lock_00f899e4() noexcept { return pending_lock_; }
    NativeEntityRegistryLockOwner* volatile& scene_lock_00f899fc() noexcept { return scene_lock_; }
    ObjectHandleTables handle_tables() const noexcept;

    // Explicit SINGLE-THREADED startup only. Native table order is CD39A0
    // (CE30CC), then CD3A70 (CE3120). Both real atexit statuses are retained;
    // nonzero status does not stop the second initializer. A partial exception
    // prohibits replay, preserving native partial allocation/registration effects.
    // Success may be queried again without duplicate initialization/registration.
    NativeEntityRegistryCrtStatuses initialize_crt_once();
private:
    friend NativeEntityRegistryProcess& native_entity_registry_process() noexcept;
    constexpr NativeEntityRegistryProcess() noexcept = default;
    NativePendingEntityListStorage pending_{};
    NativeEntityRegistryIdPair ids_{};
    NativeEntityRegistryLockOwner* volatile pending_lock_{};
    NativeEntityRegistryLockOwner* volatile scene_lock_{};
    enum class Startup { cold, attempted, complete } startup_{};
    NativeEntityRegistryCrtStatuses statuses_{};
};
static_assert(std::is_trivially_destructible_v<NativeEntityRegistryProcess>);
NativeEntityRegistryProcess& native_entity_registry_process() noexcept;

// Each constructor consumes actual raw8 storage, stamps its own native profile,
// creates the actual malloc-backed BD1860 section, then stores +4. Ordinary
// construction failure clears the supplied cell and stamps CE3818; +4 is not
// overwritten and the section is not released by the base unwind.
NativeEntityRegistryLockOwner* construct_native_pending_init_lock_00924100(
    void*, NativeEntityRegistryLockOwner* volatile& publication_00f899e4);
void unwind_native_pending_init_lock_00923640(NativeEntityRegistryLockOwner&,
    NativeEntityRegistryLockOwner* volatile& publication_00f899e4) noexcept;
NativeEntityRegistryLockOwner* construct_native_scene_registry_lock_00928080(
    void*, NativeEntityRegistryLockOwner* volatile& publication_00f899fc);
void unwind_native_scene_registry_lock_00927a60(NativeEntityRegistryLockOwner&,
    NativeEntityRegistryLockOwner* volatile& publication_00f899fc) noexcept;

// Fast path returns the captured first publication. Slow path captures the
// first manager section, double-checks, allocates/constructs, publishes, performs
// a second manager lookup, then rereads publication for registration. Return
// reload follows unlocking. Failed registration keeps owner/publication; only
// failed construction frees its captured raw8. The Game facade supplies the
// canonical GameNativeStringProcess manager cell and the same process cells.
NativeEntityRegistryLockOwner* get_native_pending_init_lock_00924810(
    void* volatile& manager_01090aa0, NativeEntityRegistryLockOwner* volatile& publication_00f899e4);
NativeEntityRegistryLockOwner* get_native_scene_registry_lock_00928240(
    void* volatile& manager_01090aa0, NativeEntityRegistryLockOwner* volatile& publication_00f899fc);

// Full scalar bodies: own-profile stamp, release actual +4, unconditional cell
// clear, base-profile stamp, flags bit0 free, captured owner-address return.
// Neither checks current-publication identity nor unregisters the owner.
NativeEntityRegistryLockOwner* delete_native_pending_init_lock_00925660(
    NativeEntityRegistryLockOwner*, std::uint32_t,
    NativeEntityRegistryLockOwner* volatile& publication_00f899e4) noexcept;
NativeEntityRegistryLockOwner* delete_native_scene_registry_lock_009285f0(
    NativeEntityRegistryLockOwner*, std::uint32_t,
    NativeEntityRegistryLockOwner* volatile& publication_00f899fc) noexcept;

// Complete926BE0: actual borrowed entity-root bits, with no dereference, flags,
// deduplication or ownership. Lock E4 is distinct from the parent's scene FC
// lock. Capture head/previous, allocate node from a pointer local, grow count
// BEFORE links. A count exception leaks the unlinked node and only unlocks.
void enqueue_native_pending_init_00926be0(NativeEntityRegistryProcess&,
    void* actual_entity_root, void* volatile& manager_01090aa0);

// Complete CRT bodies, intended for the process's explicit once-only startup.
// Actual std::atexit callbacks always target this same permanent process owner.
// Allocation failure publishes/registers nothing; registration failure retains
// storage and its real status. Pending-list destruction never deletes payloads.
int initialize_native_pending_init_owner_00cd39a0(NativeEntityRegistryProcess&);
void destroy_native_pending_init_owner_00cdf4c0(NativeEntityRegistryProcess&) noexcept;
int initialize_native_entity_id_pair_00cd3a70(NativeEntityRegistryProcess&);
void destroy_native_entity_id_pair_00cdf500(NativeEntityRegistryProcess&) noexcept;
// Complete9516B0 plain destructor: capture +4C, stamp the actual typed Source
// D19B88 profile, free slots, preserve every other field and the owner header.
void destroy_native_entity_id_table_009516b0(void* actual_owner) noexcept;

// New C++ ABI and ordinary exceptions only. Literal native profiles are data
// identities for concrete Source dispatch, never called as function pointers.
// No Native FH3/SEH, hardware-fault cleanup, entity/World/InitAll production,
// application startup wiring, full CRT ordering or game execution is supplied.
} // namespace bsp
