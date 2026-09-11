#pragma once
#include "bsp/allocator_list.hpp"
#include "bsp/native_material_owner.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual 38h layout shared by the DISTINCT material F8D3AC, parameter
// F8D3E4 and pass 0108FBF8 pools. Supply each global's canonical storage and the
// SAME allocator list domain used by other native pools. These are the only pool fields.
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
using NativeMaterialPassPoolStorage = NativeMaterialPoolStorage;

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

// Actual 0108FBF8 pool: 32 slots of 8Ch, with 88h pass payload and a DWORD slab ID.
// It shares the existing implementation's recovered shape-dependent algorithm,
// while its list element, critical section, table and slabs remain DISTINCT.
class NativeMaterialPassPool final {
public:
    static constexpr std::uint32_t native_global = 0x0108fbf8;
    static constexpr std::uint32_t native_vtable = 0x00d61a1c;
    static constexpr std::uint32_t native_virtual0 = 0x00b41170;
    static constexpr std::size_t object_bytes = 0x88;
    static constexpr std::size_t slot_bytes = 0x8c;
    static constexpr std::size_t slot_slab_index_offset = 0x88;
    static constexpr std::size_t slab_bytes = 0x11c4;
    static constexpr std::size_t free_indices_offset = 0x1180;
    static constexpr std::size_t free_count_offset = 0x11c0;
    static constexpr std::uint32_t slots_per_slab = 32;

    NativeMaterialPassPool(AllocatorListDomain&, NativeMaterialPassPoolStorage&);
    NativeMaterialPassPool(const NativeMaterialPassPool&) = delete;
    NativeMaterialPassPool& operator=(const NativeMaterialPassPool&) = delete;
    // ECX actual 38h pool, EAX same pool, RET. Prepend same allocator list,
    // initialize real CS, clear fields, firstFree=-1, allocate 32-pointer table.
    void initialize_00b41090();
    // ECX actual pool, EAX slot, RET. Real CS/recursion; LIFO slot metadata,
    // geometric table growth, no allocator-failure rollback/unlock in native.
    void* allocate_slot_00b41210();
    // ECX actual pool, stack slot, RET4. Load actual +88 slab ID under the lock.
    void return_slot_00b40a40(void*) noexcept;
    // ECX actual pool, RET; trim does not lock. Copy last slab into each removed
    // table entry and rewrite all 32 moved IDs before rescanning current index.
    void trim_empty_slabs_00b41170();
    // Caller already destroyed every pass payload. Free slabs/table, drain
    // positive recursion, delete CS, unlink actual list; metadata stays stale.
    void destroy_00b40980();
    NativeMaterialPassPoolStorage& storage() noexcept { return storage_; }
    const NativeMaterialPassPoolStorage& storage() const noexcept { return storage_; }
private:
    AllocatorListDomain& allocator_list_;
    NativeMaterialPassPoolStorage& storage_;
    static void invoke_trim(void*);
};
// ECX fresh 11C4h slab, stack slab index, EAX same address, RET4. Initializes
// only 32 descending free indices, freecount 32 and each slot+88 ID. Payloads and
// final padding remain untouched. Also used by the shape-templated allocator.
void* initialize_native_material_pass_slab_00b401d0(void* raw, std::uint32_t index) noexcept;
// ECX actual pointer-table header at pool+28, RET. Free its current nonnull
// data pointer only; keep pointer/count/capacity untouched during unwind.
void destroy_native_material_pass_pool_table_00b402b0(void* actual_header) noexcept;

// Bind the canonical companion for actual 0108FBF8 before startup. The caller
// must keep this binding unchanged; storage/list/companion must outlive the real atexit
// callback, with every payload destroyed beforehand. No additional pool state,
// initialization guard or private exit registry is created by these wrappers.
void bind_static_native_material_pass_pool_0108fbf8(NativeMaterialPassPool&) noexcept;
void* allocate_static_native_material_pass_slot_00b41820(); // no args, EAX slot, RET
void return_static_native_material_pass_slot_00b41040(void*); // original ECX slot, RET
int initialize_static_native_material_pass_pool_00cd7bc0(); // actual init then CRT atexit result
void destroy_static_native_material_pass_pool_00ce0cd0() noexcept; // actual fixed-global shutdown

} // namespace bsp
