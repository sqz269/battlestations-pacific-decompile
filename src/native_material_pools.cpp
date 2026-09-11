#include "bsp/native_material_pools.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstdlib>
#include <cstring>
#include <new>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Native material pools target MSVC Win32.");
static_assert(sizeof(CRITICAL_SECTION) == 24);
static_assert(sizeof(NativeMaterialPoolStorage) == 0x38);
static_assert(offsetof(NativeMaterialPoolStorage, critical_section_0c) == 0x0c);
static_assert(offsetof(NativeMaterialPoolStorage, recursion_24) == 0x24);
static_assert(offsetof(NativeMaterialPoolStorage, slabs_28) == 0x28);
static_assert(offsetof(NativeMaterialPoolStorage, slab_count_2c) == 0x2c);
static_assert(offsetof(NativeMaterialPoolStorage, table_capacity_30) == 0x30);
static_assert(offsetof(NativeMaterialPoolStorage, first_free_slab_34) == 0x34);
static_assert(sizeof(NativeMaterialStorage) == NativeMaterialPool::object_bytes);
constexpr std::uint32_t no_free_slab = 0xffffffffu;

CRITICAL_SECTION* section(NativeMaterialPoolStorage& storage) noexcept {
    return reinterpret_cast<CRITICAL_SECTION*>(storage.critical_section_0c);
}
template<class T> T read(const std::byte* address) noexcept {
    T value;
    std::memcpy(&value, address, sizeof(value));
    return value;
}
template<class T> void write(std::byte* address, T value) noexcept {
    std::memcpy(address, &value, sizeof(value));
}
void adjust_recursion(NativeMaterialPoolStorage& storage, std::uint32_t delta) noexcept {
    const auto bits = static_cast<std::uint32_t>(storage.recursion_24) + delta;
    std::memcpy(&storage.recursion_24, &bits, sizeof(bits));
}
std::byte** allocate_table(std::uint32_t capacity) {
    // BF55BE is a trampoline to the existing BF681B new-handler allocator.
    const auto bytes = capacity * 4u;
    return static_cast<std::byte**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes}));
}
void replace_table(NativeMaterialPoolStorage& storage, std::byte** replacement) {
    for (std::uint32_t i = 0; i < storage.slab_count_2c; ++i)
        ::new (replacement + i) std::byte*(storage.slabs_28[i]);
    if (storage.slabs_28) singleton_lifetime_free(storage.slabs_28);
    storage.slabs_28 = replacement;
}
// B17630/B17670/B402B0 receive ECX=pool+28 and free only the current pointer. They
// preserve pointer/count/capacity fields, including during constructor unwind.
void free_table(NativeMaterialPoolStorage& storage) noexcept {
    if (storage.slabs_28) singleton_lifetime_free(storage.slabs_28);
}
void destroy_section(NativeMaterialPoolStorage& storage) noexcept {
    while (storage.recursion_24 > 0) {
        --storage.recursion_24;
        LeaveCriticalSection(section(storage));
    }
    DeleteCriticalSection(section(storage));
}
template<class Pool> void initialize_pool(
    AllocatorListDomain& list, NativeMaterialPoolStorage& storage) {
    list.prepend_base_element(storage.allocator_00);
    storage.allocator_00.native_vtable_00 = Pool::native_vtable;
    unsigned unwind_state = 0;
    // B17FA0/B18340/B41090: states 0/1/2 unwind list, section, pointer table. As in
    // the actual constructor, allocation failure does not reset native fields.
    __try {
        ::new (storage.critical_section_0c) CRITICAL_SECTION;
        InitializeCriticalSection(section(storage));
        storage.recursion_24 = 0;
        unwind_state = 1;
        storage.slabs_28 = nullptr;
        storage.slab_count_2c = 0;
        storage.table_capacity_30 = 0;
        storage.first_free_slab_34 = no_free_slab;
        unwind_state = 2;
        if (storage.table_capacity_30 < 32) {
            storage.table_capacity_30 = 32;
            replace_table(storage, allocate_table(32));
        }
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2) free_table(storage);
            if (unwind_state >= 1) destroy_section(storage);
            list.unlink_base_element_00403970(storage.allocator_00);
        }
    }
}
template<class Pool> struct SlabBytes { std::byte bytes[Pool::slab_bytes]; };
// B17440/B17510/B401D0 initialize metadata only: free stack in descending order,
// identical slab ID in every slot, untouched object payload and tail padding.
template<class Pool> std::byte* initialize_slab(void* raw, std::uint32_t index) noexcept {
    auto* bytes = (::new (raw) SlabBytes<Pool>)->bytes;
    write<std::uint16_t>(bytes + Pool::free_count_offset,
        static_cast<std::uint16_t>(Pool::slots_per_slab));
    for (std::uint32_t i = 0; i < Pool::slots_per_slab; ++i) {
        write<std::uint16_t>(bytes + Pool::free_indices_offset + i * 2u,
            static_cast<std::uint16_t>(Pool::slots_per_slab - 1u - i));
        write<std::uint32_t>(bytes + i * Pool::slot_bytes + Pool::slot_slab_index_offset, index);
    }
    return bytes;
}
template<class Pool> void* allocate_slot(NativeMaterialPoolStorage& storage) {
    EnterCriticalSection(section(storage));
    adjust_recursion(storage, 1);
    // These allocators have no unwind region. Preserve early first-free and
    // capacity publication, without rollback or automatic unlock on failure.
    if (storage.first_free_slab_34 == no_free_slab) {
        storage.first_free_slab_34 = storage.slab_count_2c;
        auto* raw = singleton_lifetime_allocate({
            SingletonAllocationKind::object, Pool::slab_bytes, Pool::slab_bytes});
        auto* slab = raw ? initialize_slab<Pool>(raw, storage.first_free_slab_34) : nullptr;
        if (storage.slab_count_2c == storage.table_capacity_30) {
            storage.table_capacity_30 = storage.table_capacity_30 * 2u + 2u;
            replace_table(storage, allocate_table(storage.table_capacity_30));
        }
        ::new (storage.slabs_28 + storage.slab_count_2c) std::byte*(slab);
        ++storage.slab_count_2c;
    }
    auto* slab = storage.slabs_28[storage.first_free_slab_34];
    const auto count = static_cast<std::uint16_t>(
        read<std::uint16_t>(slab + Pool::free_count_offset) - 1u);
    write<std::uint16_t>(slab + Pool::free_count_offset, count);
    auto* slot = slab + read<std::uint16_t>(
        slab + Pool::free_indices_offset + count * 2u) * Pool::slot_bytes;
    if (count == 0) {
        auto index = storage.first_free_slab_34 + 1u;
        storage.first_free_slab_34 = no_free_slab;
        for (; index < storage.slab_count_2c; ++index) {
            if (read<std::uint16_t>(storage.slabs_28[index] + Pool::free_count_offset) != 0) {
                storage.first_free_slab_34 = index;
                break;
            }
        }
    }
    adjust_recursion(storage, 0xffffffffu);
    LeaveCriticalSection(section(storage));
    return slot;
}
template<class Pool> void return_slot(NativeMaterialPoolStorage& storage, void* slot) noexcept {
    EnterCriticalSection(section(storage));
    adjust_recursion(storage, 1);
    // The material return, B193FA parameter fragment and B40A40 load the live ID
    // here, under the real lock. Trimming may have moved its slab table entry.
    const auto slab_index = read<std::uint32_t>(
        static_cast<const std::byte*>(slot) + Pool::slot_slab_index_offset);
    auto* slab = storage.slabs_28[slab_index];
    const auto offset_bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(slot)
        - reinterpret_cast<std::uintptr_t>(slab));
    std::int32_t offset;
    std::memcpy(&offset, &offset_bits, sizeof(offset));
    // Signed IMUL/SAR/SHR sequences implement signed division toward zero by
    // 114h (magic 76B981DB), 88h (78787879) or 8Ch (EA0EA0EB).
    // Valid slots lie in this slab.
    const auto index = static_cast<std::uint16_t>(offset / static_cast<std::int32_t>(Pool::slot_bytes));
    const auto count = read<std::uint16_t>(slab + Pool::free_count_offset);
    write<std::uint16_t>(slab + Pool::free_indices_offset + count * 2u, index);
    write<std::uint16_t>(slab + Pool::free_count_offset,
        static_cast<std::uint16_t>(read<std::uint16_t>(slab + Pool::free_count_offset) + 1u));
    if (slab_index < storage.first_free_slab_34) storage.first_free_slab_34 = slab_index;
    adjust_recursion(storage, 0xffffffffu);
    LeaveCriticalSection(section(storage));
}
template<class Pool> void trim_empty_slabs(NativeMaterialPoolStorage& storage) {
    std::uint32_t index = 0;
    while (index < storage.slab_count_2c) {
        auto* slab = storage.slabs_28[index];
        if (read<std::uint16_t>(slab + Pool::free_count_offset) != Pool::slots_per_slab) {
            ++index;
            continue;
        }
        singleton_lifetime_free(slab);
        // Full free continuations B18186/B18527/B41196 copy even the last entry before
        // decrementing count, rewrite EVERY moved slot ID, then retry the index.
        storage.slabs_28[index] = storage.slabs_28[storage.slab_count_2c - 1u];
        --storage.slab_count_2c;
        if (index < storage.slab_count_2c) {
            auto* moved = storage.slabs_28[index];
            for (std::uint32_t slot = 0; slot < Pool::slots_per_slab; ++slot)
                write<std::uint32_t>(moved + slot * Pool::slot_bytes + Pool::slot_slab_index_offset, index);
        }
    }
    storage.first_free_slab_34 = no_free_slab;
    for (std::uint32_t i = 0; i < storage.slab_count_2c; ++i) {
        if (read<std::uint16_t>(storage.slabs_28[i] + Pool::free_count_offset) != 0) {
            storage.first_free_slab_34 = i;
            break;
        }
    }
}
template<class Pool> void destroy_pool(AllocatorListDomain& list, NativeMaterialPoolStorage& storage) {
    storage.allocator_00.native_vtable_00 = Pool::native_vtable;
    for (std::uint32_t i = 0; i < storage.slab_count_2c; ++i)
        singleton_lifetime_free(storage.slabs_28[i]);
    free_table(storage);
    destroy_section(storage);
    list.unlink_base_element_00403970(storage.allocator_00);
}
} // namespace

NativeMaterialPool::NativeMaterialPool(AllocatorListDomain& list, NativeMaterialPoolStorage& storage)
    : allocator_list_(list), storage_(storage) {
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}
void NativeMaterialPool::initialize_00b17fa0() { initialize_pool<NativeMaterialPool>(allocator_list_, storage_); }
void* NativeMaterialPool::allocate_00b18200() { return allocate_slot<NativeMaterialPool>(storage_); }
void NativeMaterialPool::return_slot_00b17a80(void* slot) noexcept { return_slot<NativeMaterialPool>(storage_, slot); }
void NativeMaterialPool::trim_empty_slabs_00b18160() { trim_empty_slabs<NativeMaterialPool>(storage_); }
void NativeMaterialPool::destroy_00b180d0() { destroy_pool<NativeMaterialPool>(allocator_list_, storage_); }
void NativeMaterialPool::invoke_trim(void* context) {
    static_cast<NativeMaterialPool*>(context)->trim_empty_slabs_00b18160();
}

NativeMaterialParameterPool::NativeMaterialParameterPool(
    AllocatorListDomain& list, NativeMaterialParameterPoolStorage& storage)
    : allocator_list_(list), storage_(storage) {
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}
void NativeMaterialParameterPool::initialize_00b18340() {
    initialize_pool<NativeMaterialParameterPool>(allocator_list_, storage_);
}
void* NativeMaterialParameterPool::allocate_slot_00b185a0() {
    return allocate_slot<NativeMaterialParameterPool>(storage_);
}
void NativeMaterialParameterPool::return_slot_00b193fa_fragment(void* slot) noexcept {
    return_slot<NativeMaterialParameterPool>(storage_, slot);
}
void NativeMaterialParameterPool::trim_empty_slabs_00b18500() {
    trim_empty_slabs<NativeMaterialParameterPool>(storage_);
}
void NativeMaterialParameterPool::destroy_00b18470() {
    destroy_pool<NativeMaterialParameterPool>(allocator_list_, storage_);
}
void NativeMaterialParameterPool::invoke_trim(void* context) {
    static_cast<NativeMaterialParameterPool*>(context)->trim_empty_slabs_00b18500();
}

NativeMaterialPassPool::NativeMaterialPassPool(AllocatorListDomain& list, NativeMaterialPassPoolStorage& storage)
    : allocator_list_(list), storage_(storage) {
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}
void NativeMaterialPassPool::initialize_00b41090() {
    initialize_pool<NativeMaterialPassPool>(allocator_list_, storage_);
}
void* NativeMaterialPassPool::allocate_slot_00b41210() {
    return allocate_slot<NativeMaterialPassPool>(storage_);
}
void NativeMaterialPassPool::return_slot_00b40a40(void* slot) noexcept {
    return_slot<NativeMaterialPassPool>(storage_, slot);
}
void NativeMaterialPassPool::trim_empty_slabs_00b41170() {
    trim_empty_slabs<NativeMaterialPassPool>(storage_);
}
void NativeMaterialPassPool::destroy_00b40980() {
    destroy_pool<NativeMaterialPassPool>(allocator_list_, storage_);
}
void NativeMaterialPassPool::invoke_trim(void* context) {
    static_cast<NativeMaterialPassPool*>(context)->trim_empty_slabs_00b41170();
}
void* initialize_native_material_pass_slab_00b401d0(void* raw, std::uint32_t index) noexcept {
    return initialize_slab<NativeMaterialPassPool>(raw, index);
}
void destroy_native_material_pass_pool_table_00b402b0(void* header) noexcept {
    std::byte** table;
    std::memcpy(&table, header, sizeof(table));
    if (table) singleton_lifetime_free(table);
}
namespace { NativeMaterialPassPool* canonical_pass_pool; }
void bind_static_native_material_pass_pool_0108fbf8(NativeMaterialPassPool& pool) noexcept {
    canonical_pass_pool = &pool;
}
void* allocate_static_native_material_pass_slot_00b41820() {
    return canonical_pass_pool->allocate_slot_00b41210();
}
void return_static_native_material_pass_slot_00b41040(void* slot) {
    canonical_pass_pool->return_slot_00b40a40(slot);
}
int initialize_static_native_material_pass_pool_00cd7bc0() {
    canonical_pass_pool->initialize_00b41090();
    return std::atexit(&destroy_static_native_material_pass_pool_00ce0cd0);
}
void destroy_static_native_material_pass_pool_00ce0cd0() noexcept {
    canonical_pass_pool->destroy_00b40980();
}
} // namespace bsp
