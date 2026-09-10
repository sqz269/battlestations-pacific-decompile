#include "bsp/d3d9_surface_pool.hpp"
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
D3D9SurfacePool* static_pool_0108db00;
D3D9SurfacePool& require_static_pool() {
    if (!static_pool_0108db00)
        throw std::logic_error("native 0108db00 D3D9 surface pool has no actual storage binding");
    return *static_pool_0108db00;
}
} // namespace

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
        if (storage_.table_capacity_30 < 32) {
            storage_.table_capacity_30 = 32;
            auto** replacement = allocate_table(32);
            for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
                ::new (replacement + i) std::byte*(storage_.slabs_28[i]);
            if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
            storage_.slabs_28 = replacement;
        }
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
        if (storage_.slab_count_2c == storage_.table_capacity_30) {
            storage_.table_capacity_30 = storage_.table_capacity_30 * 2u + 2u;
            auto** replacement = allocate_table(storage_.table_capacity_30);
            for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
                ::new (replacement + i) std::byte*(storage_.slabs_28[i]);
            if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
            storage_.slabs_28 = replacement;
        }
        ::new (storage_.slabs_28 + storage_.slab_count_2c) std::byte*(slab);
        ++storage_.slab_count_2c;
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

std::uint32_t D3D9SurfacePool::live_slab_index(const void* slot) noexcept {
    return read<std::uint32_t>(static_cast<const std::byte*>(slot) + slot_slab_index_offset);
}

void D3D9SurfacePool::return_raw_slot_00b3d860(void* slot) {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    const auto slab_index = live_slab_index(slot);
    auto* slab = storage_.slabs_28[slab_index];
    // Assembly's signed magic multiply is exact signed division by 0x38.
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

void D3D9SurfacePool::trim_empty_slabs_00b3e390() {
    std::uint32_t index = 0;
    while (index < storage_.slab_count_2c) {
        auto* slab = storage_.slabs_28[index];
        if (read<std::uint16_t>(slab + free_count) != 32) {
            ++index;
            continue;
        }
        singleton_lifetime_free(slab);
        // Native continuation 00B3E3B6 copies even when removing the last item.
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
