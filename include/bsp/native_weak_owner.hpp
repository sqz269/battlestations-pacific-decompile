#pragma once
#include "bsp/allocator_list.hpp"
#include "bsp/gui_camera_store_owner.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>

namespace bsp {

// Actual 0Ch weak object; the pool's slab index is a separate word at +0C.
struct NativeWeakHandle {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    void* target_08;
};
struct NativeWeakPoolStorage {
    AllocatorListElement allocator_00;
    alignas(4) std::byte critical_section_0c[24];
    std::int32_t recursion_24;
    std::byte** slabs_28;
    std::uint32_t slab_count_2c;
    std::uint32_t table_capacity_30;
    std::uint32_t first_free_slab_34;
};

// Companion over caller-owned actual 0109CE94 storage and the shared E188B4
// allocator list. No additional slab table, freelist, or allocator-list head.
// Explicit native lifecycle; the C++ destructor does not destroy the pool.
class NativeWeakHandlePool final {
public:
    static constexpr std::uint32_t native_vtable = 0x00d5dcbc;
    static constexpr std::uint32_t native_virtual0 = 0x00b003a0;
    static constexpr std::size_t slot_bytes = 0x10;
    static constexpr std::size_t slab_bytes = 0x904;
    static constexpr std::uint32_t slots_per_slab = 128;
    NativeWeakHandlePool(AllocatorListDomain&, NativeWeakPoolStorage&);
    NativeWeakHandlePool(const NativeWeakHandlePool&) = delete;
    NativeWeakHandlePool& operator=(const NativeWeakHandlePool&) = delete;
    void initialize_00b004b0();
    void* allocate_raw_slot_009242f0();
    void return_raw_slot_00924420(void*);
    void trim_empty_slabs_00b003a0(); // native virtual zero has no internal lock
    void destroy_00b002c0();         // frees slabs without object destructors
    NativeWeakPoolStorage& storage() noexcept { return storage_; }
    static std::uint32_t live_slab_index(const void*) noexcept;
private:
    AllocatorListDomain& list_;
    NativeWeakPoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_section_00402f70() noexcept;
};

using NativeWeakPoolAtexit = int (*)(void (*)());
void bind_static_native_weak_pool_0109ce94(NativeWeakHandlePool&);
int initialize_static_native_weak_pool_00cd8a60(
    NativeWeakPoolAtexit register_atexit = &std::atexit);
void destroy_static_native_weak_pool_00ce1040();

struct NativeWeakMutexOwner {
    std::uint32_t vtable_00;
    TrackedCriticalSection* section_04;
};

// Borrow fields of the SAME native object. This view contributes no storage
// and avoids pretending differently typed owners are a common C++ base.
struct NativeWeakOwnerView {
    void* identity;
    std::uint32_t& vtable_00;
    std::atomic<std::int32_t>& references_04;
    void*& weak_handle_08;
};

// Concrete NativeGuiSceneWeakBase adapter. All consumers share the supplied
// singleton lifetime domain, actual 0109CE90 publication and actual pool owner.
// The manager's destroy_registered callback must dispatch D190B4/925430 here.
// Keep this companion alive until every weak handle and registered mutex die.
class NativeWeakOwnerDomain final : public NativeGuiSceneWeakBase {
public:
    NativeWeakOwnerDomain(SingletonLifetimeDomain&,
        NativeWeakMutexOwner* volatile& global_0109ce90, NativeWeakHandlePool&) noexcept;
    NativeWeakOwnerDomain(const NativeWeakOwnerDomain&) = delete;
    NativeWeakOwnerDomain& operator=(const NativeWeakOwnerDomain&) = delete;
    void construct_00925490(NativeGuiSceneStorage&) override;
    void destroy_00925540(NativeGuiSceneStorage&) noexcept override;
    void construct_00925490(NativeWeakOwnerView);
    void destroy_00925540(NativeWeakOwnerView) noexcept;
    NativeWeakMutexOwner* lock_owner_00924480();
    NativeWeakMutexOwner* initialize_lock_owner_00924050(void*);
    NativeWeakMutexOwner* delete_lock_owner_00925430(
        NativeWeakMutexOwner*, std::uint32_t flags) noexcept;
    NativeWeakHandle* delete_handle_00925470(NativeWeakHandle*, std::uint32_t flags) noexcept;
    // Host equivalents of the established base refcount operations, using
    // only this pool object's actual +04 and known BD30E0 -> 925470 dispatch.
    void retain_handle(NativeWeakHandle&) noexcept;
    void release_handle(NativeWeakHandle&) noexcept;
private:
    SingletonLifetimeDomain& lifetime_;
    NativeWeakMutexOwner* volatile& global_0109ce90_;
    NativeWeakHandlePool& pool_;
};

} // namespace bsp
