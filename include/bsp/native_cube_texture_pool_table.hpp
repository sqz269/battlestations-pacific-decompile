#pragma once

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {
namespace native_cube_texture_pool_table_detail {

static_assert(sizeof(void*) == 4);

inline std::uint32_t read(const void* base, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(base) + offset);
}
inline void write(void* base, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(base) + offset) = value;
}

} // namespace native_cube_texture_pool_table_detail

// Sole per-address bodies promoted from render_fx_cc13.cpp's shape rows.
// Evidence: bsp.py show 00B3DBB0/00B3DC00/00B3E9C0/00B3EA40, including assembly;
// B3F090 and B3F170 inline the same operations on the real 0108DB70+28h header.
// These unreferenced image COMDATs are source substitutions for those inlined
// operations, not recovered original call edges. Names are descriptive.
// Original ABI: ECX = {data,count,capacity}; exact/append/reserve RET4, grow RET.
// These are new C++ interfaces. They add no table, lock, allocator or rollback.
__declspec(noinline) inline void native_cube_texture_pool_table_set_capacity_00b3dbb0(
    void* table, std::uint32_t capacity) {
    using namespace native_cube_texture_pool_table_detail;
    write(table, 8, capacity);
    const auto bytes = capacity * 4u; // native DWORD multiplication wraps
    void* const replacement = singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes});
    auto destination = reinterpret_cast<std::uintptr_t>(replacement);
    // New-handler processing can change the current count/table. Read them
    // after allocation and on each copy, exactly as B3DBCC..B3DBE7 does.
    for (std::uint32_t index = 0; index < read(table, 4); ++index) {
        if (destination != 0) {
            const auto value = read(reinterpret_cast<void*>(read(table)), index * 4u);
            write(reinterpret_cast<void*>(destination), 0, value);
        }
        destination += 4u;
    }
    void* const old_table = reinterpret_cast<void*>(read(table));
    if (old_table) singleton_lifetime_free(old_table);
    // Publish the captured replacement only after the current old table's free.
    write(table, 0, reinterpret_cast<std::uintptr_t>(replacement));
}

__declspec(noinline) inline void native_cube_texture_pool_table_grow_00b3dc00(void* table) {
    using namespace native_cube_texture_pool_table_detail;
    native_cube_texture_pool_table_set_capacity_00b3dbb0(table, read(table, 8) * 2u + 2u);
}

__declspec(noinline) inline void native_cube_texture_pool_table_push_back_00b3e9c0(
    void* table, const void* slab_value) {
    using namespace native_cube_texture_pool_table_detail;
    if (read(table, 4) == read(table, 8))
        native_cube_texture_pool_table_grow_00b3dc00(table);
    const auto destination = read(table) + read(table, 4) * 4u;
    // B3EA24 reads the supplied value after growth, and only for nonnull dest.
    if (destination != 0)
        write(reinterpret_cast<void*>(destination), 0, read(slab_value));
    write(table, 4, read(table, 4) + 1u);
}

__declspec(noinline) inline void native_cube_texture_pool_table_reserve_00b3ea40(
    void* table, std::uint32_t capacity) {
    using namespace native_cube_texture_pool_table_detail;
    if (read(table, 8) < capacity)
        native_cube_texture_pool_table_set_capacity_00b3dbb0(table, capacity);
}

} // namespace bsp
