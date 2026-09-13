#pragma once

#include "bsp/random_threads.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/sound_lifetime_access.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

struct NativeObserverEdgeStorage;
// Actual game storage. No constructors, owned sidecars, or default field values.
struct NativeObserverEdgeSlots {
    NativeObserverEdgeStorage** data_00;
    std::uint32_t count_04;
    std::uint32_t capacity_08;
};
// Both endpoint bases have this prefix. Pass the actual base address: native
// VoiceAttachedEntry+18 is the callback owner; its entity pointer is an already
// adjusted first-endpoint address. A semantic voice object still needs an
// explicit alias to its actual embedded owner; never reinterpret its metadata.
struct NativeObserverOwnerStorage {
    volatile std::uint32_t native_vtable_00;
    NativeObserverEdgeSlots edges_04;
};
struct NativeObserverEdgeStorage {
    volatile std::uint32_t native_vtable_00;
    NativeObserverOwnerStorage* first_04;
    NativeObserverOwnerStorage* callback_owner_08;
    std::uint32_t references_0c;
};
struct NativeObserverLockOwner {
    volatile std::uint32_t native_vtable_00;
    TrackedCriticalSection* section_04;
};
// The live 00E198E4 object is a checked pointer-vector owner, distinct from
// endpoint count/capacity arrays. Reuse the canonical +4/+8/+C slot projection.
struct NativeObserverDispatchStorage {
    void* unconsumed_00;
    SingletonPointerSlots slots_04;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeObserverEdgeSlots) == 0x0c);
static_assert(sizeof(NativeObserverOwnerStorage) == 0x10);
static_assert(offsetof(NativeObserverOwnerStorage, edges_04) == 4);
static_assert(sizeof(NativeObserverEdgeStorage) == 0x10);
static_assert(offsetof(NativeObserverEdgeStorage, first_04) == 4);
static_assert(offsetof(NativeObserverEdgeStorage, callback_owner_08) == 8);
static_assert(offsetof(NativeObserverEdgeStorage, references_0c) == 0x0c);
static_assert(sizeof(NativeObserverLockOwner) == 8);
static_assert(offsetof(NativeObserverLockOwner, section_04) == 4);
static_assert(sizeof(NativeObserverDispatchStorage) == 0x10);
static_assert(offsetof(NativeObserverDispatchStorage, slots_04) == 4);

class ObserverLifetimeServices {
public:
    virtual ~ObserverLifetimeServices() = default;
    // Dispatch slot zero of this freshly read native table on this actual edge.
    // The callee owns flag1 deletion. No fake edge life or host-side reference
    // count is allowed. Called after removal from both endpoint arrays.
    virtual void delete_edge_virtual_00(NativeObserverEdgeStorage&,
        std::uint32_t native_vtable, std::uint32_t flags) = 0;
    // Native validation can return; callers continue with their saved iterator
    // and owner identities. No synthesized restart or repaired default storage.
    virtual void invalid_parameter_00bf6713() = 0;
};

// The following operations mutate the actual three-word count/capacity array.
// 006944C0 ECX=left, EDX=right, RET: exchange contents through a temporary copy,
// retaining independent allocations/capacities. This is not a pointer swap.
void exchange_observer_edge_arrays_006944c0(
    NativeObserverEdgeSlots& left, NativeObserverEdgeSlots& right);
// 00694F60 ECX=array, EDX=edge, RET: std::remove then native resize. 00694D30 is
// the standard remove specialization (ECX=first, EDX=end, stack=&value, RET4).
void erase_observer_edge_00694f60(NativeObserverEdgeSlots&, NativeObserverEdgeStorage*);

class NativeObserverLifetime final {
public:
    // Reuse the existing borrowed lifetime access: the application passes its
    // actual01090AA0 publication; existing fixtures may pass their semantic
    // domain through the implicit access constructor. No second domain exists.
    // The historical SoundLifetimeAccess name does not restrict its raw manager
    // operations to sound. See OBSERVER_RAW_LIFETIME.md.
    NativeObserverLifetime(SoundLifetimeAccess,
        NativeObserverLockOwner* volatile& global_00e198e0,
        NativeObserverDispatchStorage* volatile& global_00e198e4,
        ObserverLifetimeServices&) noexcept;

    // No native inputs, EAX=singleton, RET. Captured manager lock, double-check,
    // allocation, publication, second manager lookup/register, unlock, reload.
    NativeObserverLockOwner* lock_owner_00694280();
    NativeObserverLockOwner* initialize_lock_owner_00694200(void*);
    void unwind_lock_owner_00693c50(NativeObserverLockOwner&) noexcept;
    // Preserve existing CG_scalar_deleting_dtor identity; ECX=owner, flags on
    // stack, EAX=original address, RET4. Bind this in singleton shutdown dispatch.
    NativeObserverLockOwner* delete_lock_owner_00694ea0(
        NativeObserverLockOwner*, std::uint32_t flags);

    // Native ECX=first endpoint, EDX=callback owner; lookup returns EAX=edge.
    // All three methods acquire the current shared recursive lock themselves.
    NativeObserverEdgeStorage* find_pair_006949d0(
        NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& callback_owner);
    void unregister_pair_006952a0(
        NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& callback_owner);
    // Native ECX=callback-owner base, RET. Clear matching dispatch slots, move
    // array contents out, then remove/destroy all captured edges regardless of
    // their reference counts. Callbacks see the owner's detached count.
    void detach_all_00695530(NativeObserverOwnerStorage&);
    // Native ECX=callback-owner base, RET. Includes base vtable write, nested
    // locked count query, detach-all, unlock, and array free. Does not free owner.
    void destroy_callback_owner_00695870(NativeObserverOwnerStorage&);

private:
    void remove_from_endpoints_and_delete(NativeObserverEdgeStorage&);
    void invalidate_dispatch_slots(NativeObserverEdgeStorage*, NativeObserverOwnerStorage*);

    SoundLifetimeAccess domain_;
    NativeObserverLockOwner* volatile& global_00e198e0_;
    NativeObserverDispatchStorage* volatile& global_00e198e4_;
    ObserverLifetimeServices& services_;
};

// Valid storage requires allocated pointer spans, counts<=capacities and a
// live published dispatch owner. Native32 count arithmetic is retained; these
// preconditions are not new bounds checks or corruption recovery behavior.
} // namespace bsp
