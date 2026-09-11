#pragma once
#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace bsp {

// Caller supplies the one canonical owner representing native 010902f4.
// The critical-section bytes hold an actual Win32 CRITICAL_SECTION initialized
// by the implementation. No field initializers replace native storage preimages.
struct NativeGroupPoolStorage {
    AllocatorListElement allocator_00;
    alignas(4) std::byte critical_section_0c[24];
    std::int32_t recursion_24;
    std::byte** slabs_28;
    std::uint32_t slab_count_2c;
    std::uint32_t table_capacity_30;
    std::uint32_t first_free_slab_34;
};

// A host companion over that storage. It contributes no second list, slab table
// or slot state. Explicit initialize/destroy calls reproduce native lifecycle;
// the C++ destructor does not destroy, unregister or unlock the native owner.
class NativeGroupPool {
public:
    static constexpr std::uint32_t native_vtable = 0x00d634d4;
    static constexpr std::uint32_t native_virtual0 = 0x00b8f270;
    // A native group occupies [0, 0x188); the trailing DWORD belongs to this
    // pool and must survive every group construction/destruction operation.
    static constexpr std::size_t slot_bytes = 0x18c;
    static constexpr std::size_t slot_slab_index_offset = 0x188;
    static constexpr std::size_t slab_bytes = 0x31c4;
    static constexpr std::uint32_t slots_per_slab = 32;

    NativeGroupPool(AllocatorListDomain&, NativeGroupPoolStorage&);
    ~NativeGroupPool() = default;
    NativeGroupPool(const NativeGroupPool&) = delete;
    NativeGroupPool& operator=(const NativeGroupPool&) = delete;

    void initialize_00b8f190();
    void* allocate_raw_slot_00b8f310();
    void return_raw_slot_00b8ed40(void* slot);
    void trim_empty_slabs_00b8f270(); // no internal critical-section entry
    void destroy_00b8ec80();         // does not invoke any group destructor
    NativeGroupPoolStorage& storage() noexcept { return storage_; }
    const NativeGroupPoolStorage& storage() const noexcept { return storage_; }
    static std::uint32_t live_slab_index(const void* slot) noexcept;

private:
    AllocatorListDomain& allocator_list_;
    NativeGroupPoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_critical_section_00402f70() noexcept;
    void free_table_unwind_00b8e8b0() noexcept;
};

// Explicit adapter binding for the actual static owner. Storage, list/domain
// and companion must outlive every registered callback. An unbound adapter fails;
// it never creates another pool or allocator-list head. Startup is called once.
using NativeGroupPoolAtexit = int (*)(void (*)());
void bind_static_native_group_pool_010902f4(NativeGroupPool&);
int initialize_static_native_group_pool_00cd8460(
    NativeGroupPoolAtexit register_atexit = &std::atexit);
void destroy_static_native_group_pool_00ce0f00();
void* allocate_native_group_slot_00b8f450();
// Native 00B8EEB0 receives the slot in ECX, then calls the pool with a stack
// argument. This function exposes an ordinary new C++ interface to that action.
void return_native_group_slot_00b8eeb0(void* slot);

} // namespace bsp
