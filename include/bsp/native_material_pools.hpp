#pragma once
#include "bsp/allocator_list.hpp"
#include "bsp/native_material_owner.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual 38h layout shared by the DISTINCT material F8D3AC and parameter
// F8D3E4 pools. Supply each global's canonical storage, and the SAME allocator
// list domain used by other native pools. These fields are the only pool state.
struct NativeMaterialPoolStorage {
    AllocatorListElement allocator_00;
    alignas(4) std::byte critical_section_0c[24];
    std::int32_t recursion_24;
    std::byte** slabs_28;
    std::uint32_t slab_count_2c;
    std::uint32_t table_capacity_30;
    std::uint32_t first_free_slab_34;
};
using NativeMaterialParameterPoolStorage = NativeMaterialPoolStorage;

// Host companions do not construct/destroy native storage implicitly. Initialize
// once before use; explicitly destroy only after all payload owners are gone.
// Storage/list/companion must survive callbacks. A second companion for the same
// native element is rejected by the shared AllocatorListDomain binding.
class NativeMaterialPool final : public NativeMaterialSlotPool {
public:
    static constexpr std::uint32_t native_global = 0x00f8d3ac;
    static constexpr std::uint32_t native_vtable = 0x00d5e518;
    static constexpr std::uint32_t native_virtual0 = 0x00b18160;
    static constexpr std::size_t object_bytes = 0x110;
    static constexpr std::size_t slot_bytes = 0x114;
    static constexpr std::size_t slot_slab_index_offset = 0x110;
    static constexpr std::size_t slab_bytes = 0x4584;
    static constexpr std::size_t free_indices_offset = 0x4500;
    static constexpr std::size_t free_count_offset = 0x4580;
    static constexpr std::uint32_t slots_per_slab = 64;

    NativeMaterialPool(AllocatorListDomain&, NativeMaterialPoolStorage&);
    ~NativeMaterialPool() override = default;
    NativeMaterialPool(const NativeMaterialPool&) = delete;
    NativeMaterialPool& operator=(const NativeMaterialPool&) = delete;

    void initialize_00b17fa0();
    void* allocate_00b18200() override;
    void return_slot_00b17a80(void* actual_slot) noexcept override;
    void trim_empty_slabs_00b18160(); // native virtual0; does not lock
    void destroy_00b180d0();         // frees slabs without destroying payloads
    NativeMaterialPoolStorage& storage() noexcept { return storage_; }
    const NativeMaterialPoolStorage& storage() const noexcept { return storage_; }

private:
    AllocatorListDomain& allocator_list_;
    NativeMaterialPoolStorage& storage_;
    static void invoke_trim(void*);
};

class NativeMaterialParameterPool final : public NativeMaterialParameterSlots {
public:
    static constexpr std::uint32_t native_global = 0x00f8d3e4;
    static constexpr std::uint32_t native_vtable = 0x00d5e51c;
    static constexpr std::uint32_t native_virtual0 = 0x00b18500;
    static constexpr std::size_t object_bytes = 0x84;
    static constexpr std::size_t slot_bytes = 0x88;
    static constexpr std::size_t slot_slab_index_offset = 0x84;
    static constexpr std::size_t slab_bytes = 0x4504;
    static constexpr std::size_t free_indices_offset = 0x4400;
    static constexpr std::size_t free_count_offset = 0x4500;
    static constexpr std::uint32_t slots_per_slab = 128;

    NativeMaterialParameterPool(AllocatorListDomain&, NativeMaterialParameterPoolStorage&);
    ~NativeMaterialParameterPool() override = default;
    NativeMaterialParameterPool(const NativeMaterialParameterPool&) = delete;
    NativeMaterialParameterPool& operator=(const NativeMaterialParameterPool&) = delete;

    void initialize_00b18340();
    void* allocate_slot_00b185a0();
    // B193FA..B19463: caller already released the NativeString name. Reads the
    // current slot+84 only AFTER acquiring this actual pool's critical section.
    // Does not release the name again or clear the material's parameter table.
    void return_slot_00b193fa_fragment(void* actual_parameter) noexcept override;
    void trim_empty_slabs_00b18500(); // native virtual0; does not lock
    void destroy_00b18470();         // caller already destroyed parameter names
    NativeMaterialParameterPoolStorage& storage() noexcept { return storage_; }
    const NativeMaterialParameterPoolStorage& storage() const noexcept { return storage_; }

private:
    AllocatorListDomain& allocator_list_;
    NativeMaterialParameterPoolStorage& storage_;
    static void invoke_trim(void*);
};

} // namespace bsp
