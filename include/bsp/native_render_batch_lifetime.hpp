#pragma once

#include "bsp/random_threads.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/sound_lifetime_access.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace bsp {
struct InstanceRenderEntry;

// Actual Win32 storage, with no field defaults or second count/array. A dead
// batch remains typed raw storage in the free list until reused or freed.
struct NativeRenderBatchStorage {
    volatile std::uint32_t native_vtable_00;
    std::atomic<std::int32_t> references_04;
    std::uint32_t mode_08;
    InstanceRenderEntry** entries_0c;
    std::int32_t count_10;
    std::int32_t capacity_14;
};
struct NativeRenderBatchFreeSlots {
    NativeRenderBatchStorage** data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
struct NativeRenderBatchPoolStorage {
    volatile std::uint32_t native_vtable_00;
    NativeRenderBatchFreeSlots slots_04;
};
struct NativeRenderBatchLockOwner {
    volatile std::uint32_t native_vtable_00;
    TrackedCriticalSection* section_04;
};
static_assert(sizeof(NativeRenderBatchStorage) == 0x18);
static_assert(std::atomic<std::int32_t>::is_always_lock_free);
static_assert(offsetof(NativeRenderBatchStorage, references_04) == 4);
static_assert(offsetof(NativeRenderBatchStorage, mode_08) == 8);
static_assert(offsetof(NativeRenderBatchStorage, entries_0c) == 0x0c);
static_assert(offsetof(NativeRenderBatchStorage, count_10) == 0x10);
static_assert(offsetof(NativeRenderBatchStorage, capacity_14) == 0x14);
static_assert(sizeof(NativeRenderBatchPoolStorage) == 0x10);
static_assert(offsetof(NativeRenderBatchPoolStorage, slots_04) == 4);
static_assert(sizeof(NativeRenderBatchLockOwner) == 8);
static_assert(offsetof(NativeRenderBatchLockOwner, section_04) == 4);

// Native ECX=raw batch, EAX=same, RET. Start/restart the storage's typed
// lifetime without value initialization. Preserve mode_08, set count1 and the
// base-batch vtable D62064, clear only +0C/+10/+14. Does not allocate.
NativeRenderBatchStorage* initialize_native_render_batch_00b51d20(void*) noexcept;
// ECX=embedded entry array (+0C), stack signed capacity/count, RET4. Host takes
// its actual containing batch. Reserve grows only, with native minimum256.
void reserve_native_render_batch_entries_00b51b50(NativeRenderBatchStorage&, std::int32_t);
void resize_native_render_batch_entries_00b51c60(NativeRenderBatchStorage&, std::int32_t);
// Destroy borrowed pointer storage; never release InstanceRenderEntry objects.
// Clear count then free, retaining the dangling pointer/capacity native bytes.
void destroy_native_render_batch_entries_00b51d00(NativeRenderBatchStorage&);
void destroy_native_render_batch_00b51d50(NativeRenderBatchStorage&);
// ECX=batch, stack flags, EAX=original address even after free, RET4.
NativeRenderBatchStorage* delete_native_render_batch_00b1c630(
    NativeRenderBatchStorage*, std::uint32_t flags);

// ECX=actual pool+4 list; reserve/resize RET4, destruction RET. Reserve's
// minimum is1. Destruction scalar-frees already-dead slots, then the array.
void reserve_native_render_batch_free_slots_00b1c830(NativeRenderBatchFreeSlots&, std::int32_t);
void resize_native_render_batch_free_slots_00b1ce50(NativeRenderBatchFreeSlots&, std::int32_t);
void destroy_native_render_batch_free_slots_00b1d8a0(NativeRenderBatchFreeSlots&);

// Borrowed actual01090AA0 manager publication (or the existing semantic
// fixture domain) and live pool/lock publications. The immutable
// table is the real D5E5AC profile (>=4 words); recycle reads its current +4
// slot after reading the owner's current table identity. This concrete profile
// requires B1C630. Unsupported/unbound tables fail before batch destruction.
// No companion destructor implicitly drains, unregisters or clears owners.
class NativeRenderBatchLifetime final {
public:
    // Caller keeps the manager and both publication cells alive through drain.
    // Implicit SoundLifetimeAccess conversion retains existing domain callers.
    NativeRenderBatchLifetime(SoundLifetimeAccess,
        NativeRenderBatchPoolStorage* volatile& global_0108fe8c,
        NativeRenderBatchLockOwner* volatile& global_0109dbbc,
        const volatile std::uint32_t* vtable_00d5e5ac);

    NativeRenderBatchPoolStorage* pool_00b1e870();
    NativeRenderBatchLockOwner* lock_owner_00b1cd90();
    NativeRenderBatchLockOwner* initialize_lock_owner_00b1c9d0(void*);
    void unwind_lock_owner_00b1c3a0(NativeRenderBatchLockOwner&) noexcept;
    NativeRenderBatchPoolStorage* delete_pool_00b1e930(
        NativeRenderBatchPoolStorage*, std::uint32_t flags);
    NativeRenderBatchLockOwner* delete_lock_owner_00b1d530(
        NativeRenderBatchLockOwner*, std::uint32_t flags);

    // Native ECX=pool+4, EAX=batch, RET. Lock owner is looked up every time;
    // capture its actual section before entering. Pop leaves the old cell.
    NativeRenderBatchStorage* acquire_00b1d5b0(NativeRenderBatchFreeSlots&);
    // Native ECX=pool+4, stack raw dead-slot address, RET4. No retain.
    void append_dead_slot_00b555e0(NativeRenderBatchFreeSlots&, NativeRenderBatchStorage*);
    // Concrete D5E5AC virtual0, ECX=batch, RET. Count has ALREADY reached zero;
    // this does not decrement. Dispatch current virtual+4(flags0), then reload
    // the current pool and append the now-dead address under its current lock.
    void recycle_zero_reference_00b55680(NativeRenderBatchStorage&);

private:
    SoundLifetimeAccess lifetime_;
    NativeRenderBatchPoolStorage* volatile& global_0108fe8c_;
    NativeRenderBatchLockOwner* volatile& global_0109dbbc_;
    const volatile std::uint32_t* vtable_00d5e5ac_;
};

// Valid native storage domain: nonnegative count/capacity, count<=capacity,
// allocated pointer spans, requested count/capacity fitting signed32 and the
// native four-byte allocation product; doubling must also fit. Locks obey
// Win32 ownership, with shutdown on the owning/quiescent thread. These are
// preconditions, not new clamps/checks or corruption recovery semantics.
} // namespace bsp
