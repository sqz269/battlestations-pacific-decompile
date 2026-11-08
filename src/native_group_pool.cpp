#include "bsp/native_group_pool.hpp"
#include "bsp/native_group_pool_table.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Native group pool storage targets MSVC Win32.");
static_assert(sizeof(CRITICAL_SECTION) == 24);
static_assert(sizeof(NativeGroupPoolStorage) == 0x38);
static_assert(offsetof(NativeGroupPoolStorage, critical_section_0c) == 0x0c);
static_assert(offsetof(NativeGroupPoolStorage, recursion_24) == 0x24);
static_assert(offsetof(NativeGroupPoolStorage, slabs_28) == 0x28);
static_assert(offsetof(NativeGroupPoolStorage, slab_count_2c) == 0x2c);
static_assert(offsetof(NativeGroupPoolStorage, table_capacity_30) == 0x30);
static_assert(offsetof(NativeGroupPoolStorage, first_free_slab_34) == 0x34);
constexpr std::size_t free_indices = 0x3180;
constexpr std::size_t free_count = 0x31c0;
constexpr std::uint32_t no_free_slab = 0xffffffffu;
struct SlabBytes { std::byte bytes[NativeGroupPool::slab_bytes]; };

CRITICAL_SECTION* section(NativeGroupPoolStorage& storage) noexcept {
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
void adjust_recursion(NativeGroupPoolStorage& storage, std::uint32_t delta) noexcept {
    const auto bits = static_cast<std::uint32_t>(storage.recursion_24) + delta;
    std::memcpy(&storage.recursion_24, &bits, sizeof(bits));
}
std::byte* initialize_slab_00b8e7d0(void* raw, std::uint32_t index) noexcept {
    auto* bytes = (::new (raw) SlabBytes)->bytes;
    write<std::uint16_t>(bytes + free_count, 32);
    for (std::uint32_t i = 0; i < 32; ++i) {
        write<std::uint16_t>(bytes + free_indices + i * 2, static_cast<std::uint16_t>(31 - i));
        write<std::uint32_t>(bytes + i * 0x18c + 0x188, index);
    }
    return bytes;
}
NativeGroupPool* static_pool_010902f4;
NativeGroupPool& require_static_pool() {
    if (!static_pool_010902f4)
        throw std::logic_error("native 010902f4 Native group pool has no actual storage binding");
    return *static_pool_010902f4;
}
} // namespace

// Chain 887: specialize the existing shared-shape grow/append bodies over the
// live pool header. 00B8F190 and 00B8F310 inline these reserve/append operations;
// replacing those copies keeps one execution of each operation. See bsp.py show
// 00B8E8E0/00B8E930/00B8EDC0/00B8EE50 and their listings: capacity is stored
// before allocation, the old table survives copying, and data is published last.
__declspec(noinline) void allocate_native_group_pool_table_00b8e8e0(
    NativeGroupPoolTableView table, std::uint32_t capacity) {
    table.capacity(capacity);
    const std::uint32_t bytes = capacity * 4u;
    auto* fresh = static_cast<std::byte*>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes}));
    // 00BF55BE is the throwing 00BF681B new-handler allocator, so fresh is
    // nonnull on return. Reload the live count/data after possible reentrancy.
    for (std::uint32_t i = 0; i < table.count(); ++i)
        std::memcpy(fresh + i * 4u, table.data() + i * 4u, 4);
    if (table.data()) singleton_lifetime_free(table.data());
    table.data(fresh);
}

__declspec(noinline) void grow_native_group_pool_table_00b8e930(
    NativeGroupPoolTableView table) {
    allocate_native_group_pool_table_00b8e8e0(table, table.capacity() * 2u + 2u);
}

__declspec(noinline) void append_native_group_pool_table_00b8edc0(
    NativeGroupPoolTableView table, const void* pointer_cell) {
    if (table.count() == table.capacity()) grow_native_group_pool_table_00b8e930(table);
    // The native placement-copy tests the computed destination, then reads the
    // source cell AFTER any growth. Do not snapshot an aliased source earlier.
    auto* destination = reinterpret_cast<void*>(
        reinterpret_cast<std::uintptr_t>(table.data()) + table.count() * 4u);
    if (destination) std::memcpy(destination, pointer_cell, 4);
    table.count(table.count() + 1u);
}

__declspec(noinline) void reserve_native_group_pool_table_00b8ee50(
    NativeGroupPoolTableView table, std::uint32_t capacity) {
    if (table.capacity() < capacity)
        allocate_native_group_pool_table_00b8e8e0(table, capacity);
}

NativeGroupPool::NativeGroupPool(AllocatorListDomain& list,
    NativeGroupPoolStorage& storage) : allocator_list_(list), storage_(storage) {
    // Establish the host dispatch binding before native construction makes this
    // element visible; a real allocation new-handler may traverse that list.
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}

void NativeGroupPool::initialize_00b8f190() {
    allocator_list_.prepend_base_element(storage_.allocator_00);
    storage_.allocator_00.native_vtable_00 = native_vtable;
    unsigned unwind_state = 0;
    // Native constructor has these three unwind leaves. Allocation itself has
    // no such region. MSVC finally also preserves cleanup during Win32 unwind.
    __try {
        ::new (storage_.critical_section_0c) CRITICAL_SECTION;
        InitializeCriticalSection(section(storage_));
        storage_.recursion_24 = 0;
        unwind_state = 1;
        storage_.slabs_28 = nullptr;
        storage_.slab_count_2c = 0;
        storage_.table_capacity_30 = 0;
        storage_.first_free_slab_34 = no_free_slab;
        unwind_state = 2;
        reserve_native_group_pool_table_00b8ee50(
            {reinterpret_cast<std::byte*>(&storage_.slabs_28)}, 32);
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2) free_table_unwind_00b8e8b0();
            if (unwind_state >= 1) destroy_critical_section_00402f70();
            allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
        }
    }
}

void* NativeGroupPool::allocate_raw_slot_00b8f310() {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    // Keep the native publication and failure semantics: no catch, scope guard,
    // rollback or automatic unlock if either real allocation throws.
    if (storage_.first_free_slab_34 == no_free_slab) {
        storage_.first_free_slab_34 = storage_.slab_count_2c;
        auto* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, slab_bytes, slab_bytes});
        auto* slab = raw ? initialize_slab_00b8e7d0(raw, storage_.first_free_slab_34) : nullptr;
        append_native_group_pool_table_00b8edc0(
            {reinterpret_cast<std::byte*>(&storage_.slabs_28)}, &slab);
    }
    auto* slab = storage_.slabs_28[storage_.first_free_slab_34];
    const auto count = static_cast<std::uint16_t>(read<std::uint16_t>(slab + free_count) - 1u);
    write<std::uint16_t>(slab + free_count, count);
    auto* slot = slab + read<std::uint16_t>(slab + free_indices + count * 2u) * slot_bytes;
    if (count == 0) {
        auto index = storage_.first_free_slab_34 + 1u;
        storage_.first_free_slab_34 = no_free_slab;
        for (; index < storage_.slab_count_2c; ++index) {
            if (read<std::uint16_t>(storage_.slabs_28[index] + free_count) != 0) {
                storage_.first_free_slab_34 = index;
                break;
            }
        }
    }
    adjust_recursion(storage_, 0xffffffffu);
    LeaveCriticalSection(section(storage_));
    return slot;
}

std::uint32_t NativeGroupPool::live_slab_index(const void* slot) noexcept {
    return read<std::uint32_t>(static_cast<const std::byte*>(slot) + slot_slab_index_offset);
}

void NativeGroupPool::return_raw_slot_00b8ed40(void* slot) {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    const auto slab_index = live_slab_index(slot);
    auto* slab = storage_.slabs_28[slab_index];
    // Assembly's signed magic multiply is exact signed division by 0x18c.
    // Preserve the low-32-bit subtraction; valid slots belong to this slab.
    const auto offset_bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(slot)
        - reinterpret_cast<std::uintptr_t>(slab));
    std::int32_t offset;
    std::memcpy(&offset, &offset_bits, sizeof(offset));
    const auto index = static_cast<std::uint16_t>(offset / static_cast<std::int32_t>(slot_bytes));
    const auto count = read<std::uint16_t>(slab + free_count);
    write<std::uint16_t>(slab + free_indices + count * 2u, index);
    // Native increments the memory word after the index store. Read it again
    // rather than imposing different alias behavior on malformed free stacks.
    write<std::uint16_t>(slab + free_count,
        static_cast<std::uint16_t>(read<std::uint16_t>(slab + free_count) + 1u));
    if (slab_index < storage_.first_free_slab_34) storage_.first_free_slab_34 = slab_index;
    adjust_recursion(storage_, 0xffffffffu);
    LeaveCriticalSection(section(storage_));
}

void NativeGroupPool::trim_empty_slabs_00b8f270() {
    std::uint32_t index = 0;
    while (index < storage_.slab_count_2c) {
        auto* slab = storage_.slabs_28[index];
        if (read<std::uint16_t>(slab + free_count) != 32) {
            ++index;
            continue;
        }
        singleton_lifetime_free(slab);
        // Native continuation 00B8F296 copies even when removing the last item.
        storage_.slabs_28[index] = storage_.slabs_28[storage_.slab_count_2c - 1u];
        --storage_.slab_count_2c;
        if (index < storage_.slab_count_2c) {
            auto* moved = storage_.slabs_28[index];
            for (std::uint32_t slot = 0; slot < 32; ++slot)
                write<std::uint32_t>(moved + slot * slot_bytes + slot_slab_index_offset, index);
        }
        // Retry this same index: the replacement can also be fully empty.
    }
    storage_.first_free_slab_34 = no_free_slab;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i) {
        if (read<std::uint16_t>(storage_.slabs_28[i] + free_count) != 0) {
            storage_.first_free_slab_34 = i;
            break;
        }
    }
}

void NativeGroupPool::invoke_trim(void* context) {
    static_cast<NativeGroupPool*>(context)->trim_empty_slabs_00b8f270();
}

void NativeGroupPool::destroy_critical_section_00402f70() noexcept {
    while (storage_.recursion_24 > 0) {
        --storage_.recursion_24;
        LeaveCriticalSection(section(storage_));
    }
    DeleteCriticalSection(section(storage_));
}

void NativeGroupPool::free_table_unwind_00b8e8b0() noexcept {
    if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
    // Native leaf does not clear table/count/capacity fields.
}

void NativeGroupPool::destroy_00b8ec80() {
    storage_.allocator_00.native_vtable_00 = native_vtable;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
        singleton_lifetime_free(storage_.slabs_28[i]);
    free_table_unwind_00b8e8b0();
    destroy_critical_section_00402f70();
    allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
}

void bind_static_native_group_pool_010902f4(NativeGroupPool& pool) {
    if (static_pool_010902f4 && static_pool_010902f4 != &pool)
        throw std::logic_error("native 010902f4 already has a different native-group-pool binding");
    static_pool_010902f4 = &pool;
}

void NativeGroupPool::require_static_binding() const {
    if (&require_static_pool() != this)
        throw std::invalid_argument("group return requires the SAME canonical 010902f4 pool");
}

int initialize_static_native_group_pool_00cd8460(NativeGroupPoolAtexit register_atexit) {
    if (!register_atexit) throw std::invalid_argument("CRT atexit registration is required");
    require_static_pool().initialize_00b8f190();
    // Native returns CRT registration's result; failure does not undo the pool.
    return register_atexit(&destroy_static_native_group_pool_00ce0f00);
}

void destroy_static_native_group_pool_00ce0f00() {
    require_static_pool().destroy_00b8ec80();
}
void* allocate_native_group_slot_00b8f450() {
    return require_static_pool().allocate_raw_slot_00b8f310();
}
void return_native_group_slot_00b8eeb0(void* slot) {
    require_static_pool().return_raw_slot_00b8ed40(slot);
}
} // namespace bsp
