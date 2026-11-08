#include "bsp/directional_light_pool.hpp"
#include "bsp/directional_light_pool_table.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Directional pool storage targets MSVC Win32.");
static_assert(sizeof(CRITICAL_SECTION) == 24);
static_assert(sizeof(DirectionalLightPoolStorage) == 0x38);
static_assert(offsetof(DirectionalLightPoolStorage, critical_section_0c) == 0x0c);
static_assert(offsetof(DirectionalLightPoolStorage, recursion_24) == 0x24);
static_assert(offsetof(DirectionalLightPoolStorage, slabs_28) == 0x28);
static_assert(offsetof(DirectionalLightPoolStorage, slab_count_2c) == 0x2c);
static_assert(offsetof(DirectionalLightPoolStorage, table_capacity_30) == 0x30);
static_assert(offsetof(DirectionalLightPoolStorage, first_free_slab_34) == 0x34);
constexpr std::size_t free_indices = 0x3e00;
constexpr std::size_t free_count = 0x3e40;
constexpr std::uint32_t no_free_slab = 0xffffffffu;
struct SlabBytes { std::byte bytes[DirectionalLightPool::slab_bytes]; };

CRITICAL_SECTION* section(DirectionalLightPoolStorage& storage) noexcept {
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
void adjust_recursion(DirectionalLightPoolStorage& storage, std::uint32_t delta) noexcept {
    const auto bits = static_cast<std::uint32_t>(storage.recursion_24) + delta;
    std::memcpy(&storage.recursion_24, &bits, sizeof(bits));
}
std::byte* initialize_slab_00b7ac90(void* raw, std::uint32_t index) noexcept {
    auto* bytes = (::new (raw) SlabBytes)->bytes;
    write<std::uint16_t>(bytes + free_count, 32);
    for (std::uint32_t i = 0; i < 32; ++i) {
        write<std::uint16_t>(bytes + free_indices + i * 2, static_cast<std::uint16_t>(31 - i));
        write<std::uint32_t>(bytes + i * 0x1f0 + 0x1ec, index);
    }
    return bytes;
}
std::byte* allocate_table(std::uint32_t capacity) {
    // Native 00BF55BE jumps to the same 00BF681B new-handler allocator.
    const auto native_bytes = capacity * 4u;
    return static_cast<std::byte*>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, native_bytes,
        static_cast<std::size_t>(capacity) * sizeof(std::byte*)}));
}
DirectionalLightPoolTableView table_view(DirectionalLightPoolStorage& storage) noexcept {
    return {reinterpret_cast<std::byte*>(&storage.slabs_28)};
}
DirectionalLightPool* static_pool_01090154;
DirectionalLightPool& require_static_pool() {
    if (!static_pool_01090154)
        throw std::logic_error("native 01090154 directional pool has no actual storage binding");
    return *static_pool_01090154;
}
} // namespace

// Chain 892 specializes the existing scene_graph2 address-dispatched bodies.
// Keep one body per address, also used by the legacy dispatcher below.
__declspec(noinline) void allocate_directional_light_pool_table_00b7ae80(
    DirectionalLightPoolTableView table, std::uint32_t capacity) {
    table.capacity(capacity); // 00B7AE87, before 00BF55BE can reenter or throw.
    auto* const replacement = allocate_table(capacity);
    // 00B7AEA3..B7: test each destination, reload live source/count, and
    // advance DWORD addresses modulo32. No snapshot or added null recovery.
    for (std::uint32_t i = 0, destination = reinterpret_cast<std::uint32_t>(replacement);
            i < table.count(); ++i, destination += 4u) {
        if (destination != 0) {
            std::uint32_t value;
            std::memcpy(&value, table.data() + i * 4u, sizeof(value));
            std::memcpy(reinterpret_cast<void*>(destination), &value, sizeof(value));
        }
    }
    if (auto* const previous = table.data()) singleton_lifetime_free(previous);
    table.data(replacement); // 00B7AEC8, after the old table's release.
}

__declspec(noinline) void grow_directional_light_pool_table_00b7aed0(
    DirectionalLightPoolTableView table) {
    allocate_directional_light_pool_table_00b7ae80(table, table.capacity() * 2u + 2u);
}

__declspec(noinline) void append_directional_light_pool_table_00b7b510(
    DirectionalLightPoolTableView table, const void* pointer_cell) {
    if (table.count() == table.capacity()) grow_directional_light_pool_table_00b7aed0(table);
    const auto destination = reinterpret_cast<std::uint32_t>(table.data()) + table.count() * 4u;
    if (destination != 0) {
        // 00B7B574..7A reads the caller's cell AFTER growth/free/publication.
        std::uint32_t value;
        std::memcpy(&value, pointer_cell, sizeof(value));
        std::memcpy(reinterpret_cast<void*>(destination), &value, sizeof(value));
    }
    table.count(table.count() + 1u); // even if the destination was null.
}

__declspec(noinline) void reserve_directional_light_pool_table_00b7b5a0(
    DirectionalLightPoolTableView table, std::uint32_t capacity) {
    if (table.capacity() < capacity) allocate_directional_light_pool_table_00b7ae80(table, capacity);
}

DirectionalLightPool::DirectionalLightPool(AllocatorListDomain& list,
    DirectionalLightPoolStorage& storage) : allocator_list_(list), storage_(storage) {
    // Establish the host dispatch binding before native construction makes this
    // element visible; a real allocation new-handler may traverse that list.
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}

void DirectionalLightPool::initialize_00b7b940() {
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
        // 00B7B940 inlines 00B7B5A0 over the real +28h header.
        reserve_directional_light_pool_table_00b7b5a0(table_view(storage_), 32);
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2) free_table_unwind_00b7adb0();
            if (unwind_state >= 1) destroy_critical_section_00402f70();
            allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
        }
    }
}

void* DirectionalLightPool::allocate_raw_slot_00b7bac0() {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    // Keep the native publication and failure semantics: no catch, scope guard,
    // rollback or automatic unlock if either real allocation throws.
    if (storage_.first_free_slab_34 == no_free_slab) {
        storage_.first_free_slab_34 = storage_.slab_count_2c;
        auto* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, slab_bytes, slab_bytes});
        auto* slab = raw ? initialize_slab_00b7ac90(raw, storage_.first_free_slab_34) : nullptr;
        // 00B7BB08..BB6B inlines 00B7B510 after the native slab producer.
        append_directional_light_pool_table_00b7b510(table_view(storage_), &slab);
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

std::uint32_t DirectionalLightPool::live_slab_index(const void* slot) noexcept {
    return read<std::uint32_t>(static_cast<const std::byte*>(slot) + slot_slab_index_offset);
}

void DirectionalLightPool::return_raw_slot_00b7b2f0(void* slot) {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    const auto slab_index = live_slab_index(slot);
    auto* slab = storage_.slabs_28[slab_index];
    // Assembly's signed magic multiply is exact signed division by 0x1F0.
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

void DirectionalLightPool::trim_empty_slabs_00b7ba20() {
    std::uint32_t index = 0;
    while (index < storage_.slab_count_2c) {
        auto* slab = storage_.slabs_28[index];
        if (read<std::uint16_t>(slab + free_count) != 32) {
            ++index;
            continue;
        }
        singleton_lifetime_free(slab);
        // Native continuation 00B7BA46 copies even when removing the last item.
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

void DirectionalLightPool::invoke_trim(void* context) {
    static_cast<DirectionalLightPool*>(context)->trim_empty_slabs_00b7ba20();
}

void DirectionalLightPool::destroy_critical_section_00402f70() noexcept {
    while (storage_.recursion_24 > 0) {
        --storage_.recursion_24;
        LeaveCriticalSection(section(storage_));
    }
    DeleteCriticalSection(section(storage_));
}

void DirectionalLightPool::free_table_unwind_00b7adb0() noexcept {
    if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
    // Native leaf does not clear table/count/capacity fields.
}

void DirectionalLightPool::destroy_00b7b230() {
    storage_.allocator_00.native_vtable_00 = native_vtable;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
        singleton_lifetime_free(storage_.slabs_28[i]);
    free_table_unwind_00b7adb0();
    destroy_critical_section_00402f70();
    allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
}

void bind_static_directional_light_pool_01090154(DirectionalLightPool& pool) {
    if (static_pool_01090154 && static_pool_01090154 != &pool)
        throw std::logic_error("native 01090154 already has a different directional-pool binding");
    static_pool_01090154 = &pool;
}

bool DirectionalLightPool::is_static_binding() const noexcept {
    return static_pool_01090154 == this;
}

int initialize_static_directional_light_pool_00cd8080(DirectionalPoolAtexit register_atexit) {
    if (!register_atexit) throw std::invalid_argument("CRT atexit registration is required");
    require_static_pool().initialize_00b7b940();
    // Native returns CRT registration's result; failure does not undo the pool.
    return register_atexit(&destroy_static_directional_light_pool_00ce0eb0);
}

void destroy_static_directional_light_pool_00ce0eb0() {
    require_static_pool().destroy_00b7b230();
}
void* allocate_directional_light_slot_00b7bd40() {
    return require_static_pool().allocate_raw_slot_00b7bac0();
}
void return_directional_light_slot_00b7b610(void* slot) {
    require_static_pool().return_raw_slot_00b7b2f0(slot);
}
} // namespace bsp
