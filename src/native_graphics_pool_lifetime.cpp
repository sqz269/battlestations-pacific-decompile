#include "bsp/native_graphics_pool_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeGraphicsPoolStorage) == 0x38);
static_assert(sizeof(CRITICAL_SECTION) == 24);
static_assert(offsetof(NativeGraphicsPoolStorage, recursion_24) == 0x24);
static_assert(offsetof(NativeGraphicsPoolStorage, slabs_28) == 0x28);
constexpr NativeGraphicsPoolProfile profiles[] = {
    {0x00b48480, 0x00b47df0, 0x00b47ed0, 0x00d61d08, 0xd4, 0xd0, 0x1ac0},
    {0x00b486a0, 0x00b47f70, 0x00b48050, 0x00d61d0c, 0x44, 0x40, 0x08c0},
    {0x00b4ab80, 0x00b4a190, 0x00b4a270, 0x00d61d60, 0x30, 0x2c, 0x0640},
    {0x00b4ada0, 0x00b4a310, 0x00b4a3f0, 0x00d61d64, 0x78, 0x74, 0x0f40},
    {0x00b4afc0, 0x00b4a490, 0x00b4a570, 0x00d61d68, 0x28, 0x24, 0x0540}
};
NativeGraphicsPoolLifetime* static_pools[6]{};
constexpr NativeGraphicsPoolKind global_kinds[] = {
    NativeGraphicsPoolKind::vertex_declaration, NativeGraphicsPoolKind::hardware_layout,
    NativeGraphicsPoolKind::physical_buffer, NativeGraphicsPoolKind::physical_buffer,
    NativeGraphicsPoolKind::logical_vertex, NativeGraphicsPoolKind::logical_index
};
CRITICAL_SECTION* section(NativeGraphicsPoolStorage& storage) noexcept {
    return reinterpret_cast<CRITICAL_SECTION*>(storage.critical_section_0c);
}
std::uint16_t free_count(const std::byte* slab, std::uint32_t offset) noexcept {
    std::uint16_t value;
    std::memcpy(&value, slab + offset, sizeof(value));
    return value;
}
NativeGraphicsPoolLifetime& require_static(unsigned index) {
    if (!static_pools[index])
        throw std::logic_error("native graphics pool has no actual storage binding");
    return *static_pools[index];
}
int initialize_static(unsigned index, NativeGraphicsPoolAtexit registration, void (*shutdown)()) {
    if (!registration) throw std::invalid_argument("CRT atexit registration is required");
    require_static(index).initialize();
    // Native returns the CRT result, including failure, without undoing startup.
    return registration(shutdown);
}
} // namespace

const NativeGraphicsPoolProfile& native_graphics_pool_profile(NativeGraphicsPoolKind kind) {
    const auto index = static_cast<unsigned>(kind);
    if (index >= 5) throw std::invalid_argument("unknown native graphics pool profile");
    return profiles[index];
}
NativeGraphicsPoolLifetime::NativeGraphicsPoolLifetime(AllocatorListDomain& list,
    NativeGraphicsPoolStorage& storage, NativeGraphicsPoolKind kind)
    : list_(list), storage_(storage), kind_(kind), profile_(native_graphics_pool_profile(kind)) {
    // Bind canonical virtual dispatch before publication to the native list.
    list_.bind_virtual0(storage_.allocator_00,
        {profile_.vtable, profile_.trim, this, &invoke_trim});
}
NativeGraphicsPoolStorage& NativeGraphicsPoolLifetime::initialize() {
    list_.prepend_base_element(storage_.allocator_00);
    storage_.allocator_00.native_vtable_00 = profile_.vtable;
    unsigned unwind_state = 0;
    __try {
        ::new (storage_.critical_section_0c) CRITICAL_SECTION;
        InitializeCriticalSection(section(storage_));
        storage_.recursion_24 = 0;
        unwind_state = 1;
        storage_.slabs_28 = nullptr;
        storage_.slab_count_2c = 0;
        storage_.table_capacity_30 = 0;
        storage_.first_free_slab_34 = 0xffffffffu;
        unwind_state = 2;
        if (storage_.table_capacity_30 < 32) {
            storage_.table_capacity_30 = 32;
            auto** replacement = static_cast<std::byte**>(singleton_lifetime_allocate(
                {SingletonAllocationKind::pointer_slots, 0x80, 0x80}));
            // A real allocation new-handler may have changed the live table.
            for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
                ::new (replacement + i) std::byte*(storage_.slabs_28[i]);
            free_table();
            storage_.slabs_28 = replacement;
        }
    } __finally {
        if (AbnormalTermination()) {
            // Native FH3 chains table -> critical section -> allocator list.
            if (unwind_state >= 2) free_table();
            if (unwind_state >= 1) destroy_critical_section();
            list_.unlink_base_element_00403970(storage_.allocator_00);
        }
    }
    return storage_;
}
void NativeGraphicsPoolLifetime::trim_empty_slabs() {
    std::uint32_t i = 0;
    while (i < storage_.slab_count_2c) {
        auto* slab = storage_.slabs_28[i];
        if (free_count(slab, profile_.free_count_offset) != 32) { ++i; continue; }
        singleton_lifetime_free(slab);
        storage_.slabs_28[i] = storage_.slabs_28[storage_.slab_count_2c - 1];
        --storage_.slab_count_2c;
        if (i < storage_.slab_count_2c) {
            auto* index = storage_.slabs_28[i] + profile_.slab_index_offset;
            for (unsigned slot = 0; slot < 32; ++slot, index += profile_.slot_bytes)
                std::memcpy(index, &i, sizeof(i));
        }
        // Retry the moved slab at this index, including another empty slab.
    }
    storage_.first_free_slab_34 = 0xffffffffu;
    auto** cursor = storage_.slabs_28;
    for (i = 0; i < storage_.slab_count_2c; ++i, ++cursor) {
        if (free_count(*cursor, profile_.free_count_offset) != 0) {
            storage_.first_free_slab_34 = i;
            break;
        }
    }
}
void NativeGraphicsPoolLifetime::invoke_trim(void* context) {
    static_cast<NativeGraphicsPoolLifetime*>(context)->trim_empty_slabs();
}
void NativeGraphicsPoolLifetime::free_table() noexcept {
    if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
}
void NativeGraphicsPoolLifetime::destroy_critical_section() noexcept {
    while (storage_.recursion_24 > 0) {
        --storage_.recursion_24;
        LeaveCriticalSection(section(storage_));
    }
    DeleteCriticalSection(section(storage_));
}
void NativeGraphicsPoolLifetime::destroy() {
    storage_.allocator_00.native_vtable_00 = profile_.vtable;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
        singleton_lifetime_free(storage_.slabs_28[i]);
    free_table();
    destroy_critical_section();
    list_.unlink_base_element_00403970(storage_.allocator_00);
    // Native retains old links and table/count/capacity/first-free values.
}
void bind_static_native_graphics_pool(NativeGraphicsPoolGlobal global, NativeGraphicsPoolLifetime& pool) {
    const auto index = static_cast<unsigned>(global);
    if (index >= 6 || pool.kind() != global_kinds[index])
        throw std::invalid_argument("graphics global requires its matching native pool profile");
    if (static_pools[index] && static_pools[index] != &pool)
        throw std::logic_error("graphics global already has a different native pool binding");
    for (unsigned other = 0; other < 6; ++other)
        if (other != index && static_pools[other] &&
            &static_pools[other]->storage() == &pool.storage())
            throw std::logic_error("distinct graphics globals require distinct actual storage");
    static_pools[index] = &pool;
}

int initialize_static_native_vertex_declaration_pool_00cd7be0(NativeGraphicsPoolAtexit fn) {
    return initialize_static(0, fn, &destroy_static_native_vertex_declaration_pool_00ce0ce0);
}
int initialize_static_native_hardware_layout_pool_00cd7c00(NativeGraphicsPoolAtexit fn) {
    return initialize_static(1, fn, &destroy_static_native_hardware_layout_pool_00ce0cf0);
}
int initialize_static_native_physical_index_pool_00cd7c20(NativeGraphicsPoolAtexit fn) {
    return initialize_static(2, fn, &destroy_static_native_physical_index_pool_00ce0d00);
}
int initialize_static_native_physical_vertex_pool_00cd7c40(NativeGraphicsPoolAtexit fn) {
    return initialize_static(3, fn, &destroy_static_native_physical_vertex_pool_00ce0d10);
}
int initialize_static_native_logical_vertex_pool_00cd7c60(NativeGraphicsPoolAtexit fn) {
    return initialize_static(4, fn, &destroy_static_native_logical_vertex_pool_00ce0d20);
}
int initialize_static_native_logical_index_pool_00cd7c80(NativeGraphicsPoolAtexit fn) {
    return initialize_static(5, fn, &destroy_static_native_logical_index_pool_00ce0d30);
}
void destroy_static_native_vertex_declaration_pool_00ce0ce0() { require_static(0).destroy(); }
void destroy_static_native_hardware_layout_pool_00ce0cf0() { require_static(1).destroy(); }
void destroy_static_native_physical_index_pool_00ce0d00() { require_static(2).destroy(); }
void destroy_static_native_physical_vertex_pool_00ce0d10() { require_static(3).destroy(); }
void destroy_static_native_logical_vertex_pool_00ce0d20() { require_static(4).destroy(); }
void destroy_static_native_logical_index_pool_00ce0d30() { require_static(5).destroy(); }
} // namespace bsp
