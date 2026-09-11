#pragma once
#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace bsp {

// The actual storage representing native 0108FFF8, in the same shared
// AllocatorListDomain as the other native pools. No second list or slot state.
struct NativeMeshPoolStorage {
    AllocatorListElement allocator_00;
    alignas(4) std::byte critical_section_0c[24];
    std::int32_t recursion_24;
    std::byte** slabs_28;
    std::uint32_t slab_count_2c;
    std::uint32_t table_capacity_30;
    std::uint32_t first_free_slab_34;
};

// Explicit native lifecycle: destruction of this host companion does not
// destroy or unlink storage. Slot payloads belong to NativeMeshOwner; this
// owner changes only slab metadata and never invokes a mesh destructor.
class NativeMeshPool {
public:
    static constexpr std::uint32_t native_vtable = 0x00d62d50;
    static constexpr std::uint32_t native_virtual0 = 0x00b73970;
    static constexpr std::size_t object_bytes = 0xbc;
    static constexpr std::size_t slot_bytes = 0xc0;
    static constexpr std::size_t slot_slab_index_offset = 0xbc;
    static constexpr std::size_t slot_chunk_index_offset = slot_slab_index_offset;
    static constexpr std::size_t slab_bytes = 0x1844;
    static constexpr std::uint32_t slots_per_slab = 32;

    NativeMeshPool(AllocatorListDomain&, NativeMeshPoolStorage&);
    ~NativeMeshPool() = default;
    NativeMeshPool(const NativeMeshPool&) = delete;
    NativeMeshPool& operator=(const NativeMeshPool&) = delete;

    void initialize_00b73890();
    void* allocate_slot_00b73a10();
    void return_slot_00b72da0(void* slot) noexcept;
    void trim_empty_slabs_00b73970(); // does not enter the critical section
    void destroy_00b72ce0();         // does not destroy mesh payloads
    NativeMeshPoolStorage& storage() noexcept { return storage_; }
    const NativeMeshPoolStorage& storage() const noexcept { return storage_; }
    static std::uint32_t live_slab_index(const void* slot) noexcept;

private:
    AllocatorListDomain& allocator_list_;
    NativeMeshPoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_critical_section_00402f70() noexcept;
    void free_table_unwind_00b728d0() noexcept;
};

// The storage, companion, and shared allocator-list domain must outlive all
// callbacks. An unbound adapter fails explicitly, with no replacement pool.
using NativeMeshPoolAtexit = int (*)(void (*)());
void bind_static_native_mesh_pool_0108fff8(NativeMeshPool&);
int initialize_static_native_mesh_pool_00cd7e40(
    NativeMeshPoolAtexit register_atexit = &std::atexit);
void destroy_static_native_mesh_pool_00ce0e40();
void* allocate_native_mesh_slot_00b73b60();
// Native 00B72F70 receives the slot in ECX and supplies one stack argument to
// the actual pool at 0108FFF8. This exposes an ordinary new C++ interface.
void return_native_mesh_slot_00b72f70(void* slot);

} // namespace bsp
