#pragma once
#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace bsp {

// Caller supplies the one canonical owner representing native 0108ffb0.
// The critical-section bytes hold an actual Win32 CRITICAL_SECTION initialized
// by the implementation. No field initializers replace native storage preimages.
struct NativeCameraPoolStorage {
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
class NativeCameraPool {
public:
    static constexpr std::uint32_t native_vtable = 0x00d62ce0;
    static constexpr std::uint32_t native_virtual0 = 0x00b716d0;
    static constexpr std::size_t slot_bytes = 0x45c;
    static constexpr std::size_t slot_slab_index_offset = 0x458;
    static constexpr std::size_t slab_bytes = 0x8bc4;
    static constexpr std::uint32_t slots_per_slab = 32;

    NativeCameraPool(AllocatorListDomain&, NativeCameraPoolStorage&);
    ~NativeCameraPool() = default;
    NativeCameraPool(const NativeCameraPool&) = delete;
    NativeCameraPool& operator=(const NativeCameraPool&) = delete;

    void initialize_00b715f0();
    void* allocate_raw_slot_00b71770();
    void return_raw_slot_00b711e0(void* slot);
    void trim_empty_slabs_00b716d0(); // no internal critical-section entry
    void destroy_00b71120();         // does not invoke any camera destructor
    NativeCameraPoolStorage& storage() noexcept { return storage_; }
    const NativeCameraPoolStorage& storage() const noexcept { return storage_; }
    static std::uint32_t live_slab_index(const void* slot) noexcept;

private:
    AllocatorListDomain& allocator_list_;
    NativeCameraPoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_critical_section_00402f70() noexcept;
    void free_table_unwind_00b6ffc0() noexcept;
};

// Explicit adapter binding for the actual static owner. Storage, list/domain
// and companion must outlive every registered callback. An unbound adapter fails;
// it never creates another pool or allocator-list head. Startup is called once.
using NativeCameraPoolAtexit = int (*)(void (*)());
void bind_static_native_camera_pool_0108ffb0(NativeCameraPool&);
int initialize_static_native_camera_pool_00cd7dd0(
    NativeCameraPoolAtexit register_atexit = &std::atexit);
void destroy_static_native_camera_pool_00ce0e30();
void* allocate_native_camera_slot_00b71930();
// Native 00B71350 takes the slot in ECX, pushes it, then calls 00B711E0;
// its own RET pops no arguments. This host function exposes an ordinary C++ API.
void return_native_camera_slot_00b71350(void* slot);

} // namespace bsp
