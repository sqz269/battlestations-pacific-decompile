#pragma once
#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace bsp {

// Caller supplies the one canonical owner representing native 0109011c.
// The critical-section bytes hold an actual Win32 CRITICAL_SECTION initialized
// by the implementation. No field initializers replace native storage preimages.
struct NativePointLightPoolStorage {
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
class NativePointLightPool {
public:
    static constexpr std::uint32_t native_vtable = 0x00d62efc;
    static constexpr std::uint32_t native_virtual0 = 0x00b7b770;
    static constexpr std::size_t slot_bytes = 0x200;
    static constexpr std::size_t slot_slab_index_offset = 0x1fc;
    static constexpr std::size_t slab_bytes = 0x4044;
    static constexpr std::uint32_t slots_per_slab = 32;

    NativePointLightPool(AllocatorListDomain&, NativePointLightPoolStorage&);
    ~NativePointLightPool() = default;
    NativePointLightPool(const NativePointLightPool&) = delete;
    NativePointLightPool& operator=(const NativePointLightPool&) = delete;

    void initialize_00b7b690();
    void* allocate_raw_slot_00b7b810();
    void return_raw_slot_00b7b1d0(void* slot);
    void trim_empty_slabs_00b7b770(); // no internal critical-section entry
    void destroy_00b7b110();         // does not invoke any light destructor
    NativePointLightPoolStorage& storage() noexcept { return storage_; }
    const NativePointLightPoolStorage& storage() const noexcept { return storage_; }
    static std::uint32_t live_slab_index(const void* slot) noexcept;

private:
    AllocatorListDomain& allocator_list_;
    NativePointLightPoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_critical_section_00402f70() noexcept;
    void free_table_unwind_00b7ad70() noexcept;
};

// Explicit adapter binding for the actual static owner. Storage, list/domain
// and companion must outlive every registered callback. An unbound adapter fails;
// it never creates another pool or allocator-list head. Startup is called once.
using NativePointLightPoolAtexit = int (*)(void (*)());
void bind_static_native_point_light_pool_0109011c(NativePointLightPool&);
int initialize_static_native_point_light_pool_00cd8060(
    NativePointLightPoolAtexit register_atexit = &std::atexit);
void destroy_static_native_point_light_pool_00ce0ea0();
void* allocate_point_light_slot_00b7bd30();
void return_point_light_slot_00b7b600(void* slot);

} // namespace bsp
