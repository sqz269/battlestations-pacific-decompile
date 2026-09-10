#pragma once
#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace bsp {

// Caller supplies the one canonical owner representing native 01090154.
// The critical-section bytes hold an actual Win32 CRITICAL_SECTION initialized
// by the implementation. No field initializers replace native storage preimages.
struct DirectionalLightPoolStorage {
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
class DirectionalLightPool {
public:
    static constexpr std::uint32_t native_vtable = 0x00d62f00;
    static constexpr std::uint32_t native_virtual0 = 0x00b7ba20;
    static constexpr std::size_t slot_bytes = 0x1f0;
    static constexpr std::size_t slot_slab_index_offset = 0x1ec;
    static constexpr std::size_t slab_bytes = 0x3e44;
    static constexpr std::uint32_t slots_per_slab = 32;

    DirectionalLightPool(AllocatorListDomain&, DirectionalLightPoolStorage&);
    ~DirectionalLightPool() = default;
    DirectionalLightPool(const DirectionalLightPool&) = delete;
    DirectionalLightPool& operator=(const DirectionalLightPool&) = delete;

    void initialize_00b7b940();
    void* allocate_raw_slot_00b7bac0();
    void return_raw_slot_00b7b2f0(void* slot);
    void trim_empty_slabs_00b7ba20(); // no internal critical-section entry
    void destroy_00b7b230();         // does not invoke any light destructor
    DirectionalLightPoolStorage& storage() noexcept { return storage_; }
    const DirectionalLightPoolStorage& storage() const noexcept { return storage_; }
    static std::uint32_t live_slab_index(const void* slot) noexcept;

private:
    AllocatorListDomain& allocator_list_;
    DirectionalLightPoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_critical_section_00402f70() noexcept;
    void free_table_unwind_00b7adb0() noexcept;
};

// Explicit adapter binding for the actual static owner. Storage, list/domain
// and companion must outlive every registered callback. An unbound adapter fails;
// it never creates another pool or allocator-list head. Startup is called once.
using DirectionalPoolAtexit = int (*)(void (*)());
void bind_static_directional_light_pool_01090154(DirectionalLightPool&);
int initialize_static_directional_light_pool_00cd8080(
    DirectionalPoolAtexit register_atexit = &std::atexit);
void destroy_static_directional_light_pool_00ce0eb0();
void* allocate_directional_light_slot_00b7bd40();
void return_directional_light_slot_00b7b610(void* slot);

} // namespace bsp
