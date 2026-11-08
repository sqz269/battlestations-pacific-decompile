#include "bsp/d3d9_surface_pool.hpp"
#include "bsp/d3d9_surface_pool_table.hpp"
#include "bsp/render_fx_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "D3D9 surface pool storage targets MSVC Win32.");
static_assert(sizeof(CRITICAL_SECTION) == 24);
static_assert(sizeof(D3D9SurfacePoolStorage) == 0x38);
static_assert(offsetof(D3D9SurfacePoolStorage, critical_section_0c) == 0x0c);
static_assert(offsetof(D3D9SurfacePoolStorage, recursion_24) == 0x24);
static_assert(offsetof(D3D9SurfacePoolStorage, slabs_28) == 0x28);
static_assert(offsetof(D3D9SurfacePoolStorage, slab_count_2c) == 0x2c);
static_assert(offsetof(D3D9SurfacePoolStorage, table_capacity_30) == 0x30);
static_assert(offsetof(D3D9SurfacePoolStorage, first_free_slab_34) == 0x34);
constexpr std::size_t free_indices = 0x700;
constexpr std::size_t free_count = 0x740;
constexpr std::uint32_t no_free_slab = 0xffffffffu;
struct SlabBytes { std::byte bytes[D3D9SurfacePool::slab_bytes]; };

CRITICAL_SECTION* section(D3D9SurfacePoolStorage& storage) noexcept {
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
void adjust_recursion(D3D9SurfacePoolStorage& storage, std::uint32_t delta) noexcept {
    const auto bits = static_cast<std::uint32_t>(storage.recursion_24) + delta;
    std::memcpy(&storage.recursion_24, &bits, sizeof(bits));
}
std::byte* initialize_slab_00b3d120(void* raw, std::uint32_t index) noexcept {
    auto* bytes = (::new (raw) SlabBytes)->bytes;
    write<std::uint16_t>(bytes + free_count, 32);
    for (std::uint32_t i = 0; i < 32; ++i) {
        write<std::uint16_t>(bytes + free_indices + i * 2, static_cast<std::uint16_t>(31 - i));
        write<std::uint32_t>(bytes + i * 0x38 + 0x34, index);
    }
    return bytes;
}
std::byte** allocate_table(std::uint32_t capacity) {
    // Native 00BF55BE jumps to the same 00BF681B new-handler allocator.
    const auto native_bytes = capacity * 4u;
    return static_cast<std::byte**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, native_bytes,
        static_cast<std::size_t>(capacity) * sizeof(std::byte*)}));
}
D3D9SurfacePoolTableView table_view(D3D9SurfacePoolStorage& storage) noexcept {
    return {reinterpret_cast<std::byte*>(&storage) + 0x28};
}
D3D9SurfacePool* static_pool_0108db00;
D3D9SurfacePool& require_static_pool() {
    if (!static_pool_0108db00)
        throw std::logic_error("native 0108db00 D3D9 surface pool has no actual storage binding");
    return *static_pool_0108db00;
}
} // namespace

// Promote the four render_fx template rows to one address-specific body each.
// Capacity publication precedes allocation; count and source are reloaded after
// a real new-handler callback. The old table is freed before data publication.
// Null placement destinations are skipped, as in the original DWORD loop.
__declspec(noinline) void set_d3d9_surface_pool_table_capacity_00b3da70(
    D3D9SurfacePoolTableView table, std::uint32_t capacity) {
    table.capacity(capacity);
    auto* const replacement = reinterpret_cast<std::byte*>(allocate_table(capacity));
    for (std::uint32_t i = 0; i < table.count(); ++i) {
        const auto destination = reinterpret_cast<std::uintptr_t>(replacement) + i * 4u;
        if (destination != 0) {
            std::uint32_t value;
            const auto source = reinterpret_cast<std::uintptr_t>(table.data()) + i * 4u;
            std::memcpy(&value, reinterpret_cast<const void*>(source), sizeof(value));
            std::memcpy(reinterpret_cast<void*>(destination), &value, sizeof(value));
        }
    }
    if (auto* const old = table.data()) singleton_lifetime_free(old);
    table.data(replacement);
}

__declspec(noinline) void grow_d3d9_surface_pool_table_00b3dac0(
    D3D9SurfacePoolTableView table) {
    set_d3d9_surface_pool_table_capacity_00b3da70(table, table.capacity() * 2u + 2u);
}

__declspec(noinline) void append_d3d9_surface_pool_table_00b3e800(
    D3D9SurfacePoolTableView table, const void* pointer_cell) {
    if (table.count() == table.capacity()) grow_d3d9_surface_pool_table_00b3dac0(table);
    const auto destination = reinterpret_cast<std::uintptr_t>(table.data()) + table.count() * 4u;
    if (destination != 0) {
        std::uint32_t value;
        std::memcpy(&value, pointer_cell, sizeof(value));
        std::memcpy(reinterpret_cast<void*>(destination), &value, sizeof(value));
    }
    table.count(table.count() + 1u);
}

__declspec(noinline) void reserve_d3d9_surface_pool_table_00b3e880(
    D3D9SurfacePoolTableView table, std::uint32_t capacity) {
    if (table.capacity() < capacity)
        set_d3d9_surface_pool_table_capacity_00b3da70(table, capacity);
}

D3D9SurfacePool::D3D9SurfacePool(AllocatorListDomain& list,
    D3D9SurfacePoolStorage& storage) : allocator_list_(list), storage_(storage) {
    // Establish the host dispatch binding before native construction makes this
    // element visible; a real allocation new-handler may traverse that list.
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}

void D3D9SurfacePool::initialize_00b3ec60() {
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
        // 00B3EC60 inlines 00B3E880 with the native initial capacity 32.
        reserve_d3d9_surface_pool_table_00b3e880(table_view(storage_), 32);
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2) free_table_unwind_00b3d370();
            if (unwind_state >= 1) destroy_critical_section_00402f70();
            allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
        }
    }
}

void* D3D9SurfacePool::allocate_raw_slot_00b3ed40() {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    // Keep the native publication and failure semantics: no catch, scope guard,
    // rollback or automatic unlock if either real allocation throws.
    if (storage_.first_free_slab_34 == no_free_slab) {
        storage_.first_free_slab_34 = storage_.slab_count_2c;
        auto* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, slab_bytes, slab_bytes});
        auto* slab = raw ? initialize_slab_00b3d120(raw, storage_.first_free_slab_34) : nullptr;
        // 00B3ED40's append uses this newly allocated slab and the retained
        // pool+28h header. No parallel pointer table or duplicate append.
        append_d3d9_surface_pool_table_00b3e800(table_view(storage_), &slab);
    }
    auto* slab = storage_.slabs_28[storage_.first_free_slab_34];
    // 00B3ED40 inlines the same operations as the recovered surface leaves.
    auto* slot = cc13_render1::surface_slab_pop_00b3d160(
        reinterpret_cast<std::uint8_t*>(slab));
    if (cc13_render1::surface_slab_has_no_free_00b3d1d0(
            reinterpret_cast<const std::uint8_t*>(slab))) {
        auto index = storage_.first_free_slab_34 + 1u;
        storage_.first_free_slab_34 = no_free_slab;
        for (; index < storage_.slab_count_2c; ++index) {
            if (!cc13_render1::surface_slab_has_no_free_00b3d1d0(
                    reinterpret_cast<const std::uint8_t*>(storage_.slabs_28[index]))) {
                storage_.first_free_slab_34 = index;
                break;
            }
        }
    }
    adjust_recursion(storage_, 0xffffffffu);
    LeaveCriticalSection(section(storage_));
    return slot;
}

std::uint32_t D3D9SurfacePool::live_slab_index(const void* slot) noexcept {
    return read<std::uint32_t>(static_cast<const std::byte*>(slot) + slot_slab_index_offset);
}

void D3D9SurfacePool::return_raw_slot_00b3d860(void* slot) {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    const auto slab_index = live_slab_index(slot);
    auto* slab = storage_.slabs_28[slab_index];
    // 00B3D860's inlined push has the same signed quotient and write order.
    cc13_render1::surface_slab_push_00b3d190(reinterpret_cast<std::uint8_t*>(slab),
        static_cast<const std::uint8_t*>(slot));
    if (slab_index < storage_.first_free_slab_34) storage_.first_free_slab_34 = slab_index;
    adjust_recursion(storage_, 0xffffffffu);
    LeaveCriticalSection(section(storage_));
}

void D3D9SurfacePool::trim_empty_slabs_00b3e390() {
    std::uint32_t index = 0;
    while (index < storage_.slab_count_2c) {
        auto* slab = storage_.slabs_28[index];
        if (!cc13_render1::surface_slab_all_free_00b3e360(
                reinterpret_cast<const std::uint8_t*>(slab))) {
            ++index;
            continue;
        }
        singleton_lifetime_free(slab);
        // Native continuation 00B3E3B6 copies even when removing the last item.
        storage_.slabs_28[index] = storage_.slabs_28[storage_.slab_count_2c - 1u];
        --storage_.slab_count_2c;
        if (index < storage_.slab_count_2c) {
            auto* moved = storage_.slabs_28[index];
            cc13_render1::surface_slab_stamp_index_00b3e340(
                reinterpret_cast<std::uint8_t*>(moved), index);
        }
        // Retry this same index: the replacement can also be fully empty.
    }
    storage_.first_free_slab_34 = no_free_slab;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i) {
        if (!cc13_render1::surface_slab_has_no_free_00b3d1d0(
                reinterpret_cast<const std::uint8_t*>(storage_.slabs_28[i]))) {
            storage_.first_free_slab_34 = i;
            break;
        }
    }
}

void D3D9SurfacePool::invoke_trim(void* context) {
    static_cast<D3D9SurfacePool*>(context)->trim_empty_slabs_00b3e390();
}

void D3D9SurfacePool::destroy_critical_section_00402f70() noexcept {
    while (storage_.recursion_24 > 0) {
        --storage_.recursion_24;
        LeaveCriticalSection(section(storage_));
    }
    DeleteCriticalSection(section(storage_));
}

void D3D9SurfacePool::free_table_unwind_00b3d370() noexcept {
    if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
    // Native leaf does not clear table/count/capacity fields.
}

void D3D9SurfacePool::destroy_00b3e2b0() {
    storage_.allocator_00.native_vtable_00 = native_vtable;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
        singleton_lifetime_free(storage_.slabs_28[i]);
    free_table_unwind_00b3d370();
    destroy_critical_section_00402f70();
    allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
}

void bind_static_d3d9_surface_pool_0108db00(D3D9SurfacePool& pool) {
    if (static_pool_0108db00 && static_pool_0108db00 != &pool)
        throw std::logic_error("native 0108db00 already has a different D3D9-surface-pool binding");
    static_pool_0108db00 = &pool;
}

int initialize_static_d3d9_surface_pool_00cd7b40(D3D9SurfacePoolAtexit register_atexit) {
    if (!register_atexit) throw std::invalid_argument("CRT atexit registration is required");
    require_static_pool().initialize_00b3ec60();
    // Native returns CRT registration's result; failure does not undo the pool.
    return register_atexit(&destroy_static_d3d9_surface_pool_00ce0c90);
}

void destroy_static_d3d9_surface_pool_00ce0c90() {
    require_static_pool().destroy_00b3e2b0();
}
void* allocate_d3d9_surface_slot_00b3f2a0() {
    return require_static_pool().allocate_raw_slot_00b3ed40();
}
void return_d3d9_surface_slot_00b3dcc0(void* slot) {
    require_static_pool().return_raw_slot_00b3d860(slot);
}
} // namespace bsp
