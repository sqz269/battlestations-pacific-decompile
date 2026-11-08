#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {
namespace native_gui_layer_pool_table_detail {
static_assert(sizeof(void*) == 4);
inline std::uint32_t read(const void* table, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(table) + offset);
}
inline void write(void* table, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(table) + offset) = value;
}
} // namespace native_gui_layer_pool_table_detail

// Sole address-specific bodies for Layer's retained F8BF50+28h slab table.
// Evidence: bsp.py show AC4250/AC4350/AC43A0/AC4990/AC4A10; AC4D70,
// AC4E50 and AC4670 inline the same reserve, append and free operations.
// Descriptive names are hypotheses. Original ECX=table; set/reserve/append
// RET 4, grow/free RET. These C++ interfaces are not register-ABI thunks.
// No table copy, additional allocation domain or rollback is introduced.
__declspec(noinline) inline void free_native_gui_layer_pool_table_00ac4250(
    void* table) noexcept {
    using namespace native_gui_layer_pool_table_detail;
    void* const data = reinterpret_cast<void*>(read(table));
    if (data) singleton_lifetime_free(data);
    // AC4250 and the AC4670 inline copy leave all three header words intact.
}

__declspec(noinline) inline void set_native_gui_layer_pool_table_capacity_00ac4350(
    NativeGuiPoolSlabTable& actual_table, std::uint32_t capacity) {
    using namespace native_gui_layer_pool_table_detail;
    void* const table = &actual_table;
    write(table, 8, capacity);
    const std::uint32_t bytes = capacity * 4u; // native DWORD wrap
    void* const replacement = singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes});
    auto destination = reinterpret_cast<std::uintptr_t>(replacement);
    // AC4367..4384: reread current count/data after allocation and per copy;
    // the new-handler is allowed to alter the current table before returning.
    for (std::uint32_t index = 0; index < read(table, 4); ++index, destination += 4u) {
        if (destination != 0) {
            const auto value = read(reinterpret_cast<void*>(read(table)), index * 4u);
            write(reinterpret_cast<void*>(destination), 0, value);
        }
    }
    void* const old_data = reinterpret_cast<void*>(read(table));
    if (old_data) singleton_lifetime_free(old_data);
    write(table, 0, reinterpret_cast<std::uintptr_t>(replacement));
}

__declspec(noinline) inline void grow_native_gui_layer_pool_table_00ac43a0(
    NativeGuiPoolSlabTable& table) {
    using namespace native_gui_layer_pool_table_detail;
    set_native_gui_layer_pool_table_capacity_00ac4350(table, read(&table, 8) * 2u + 2u);
}

__declspec(noinline) inline void push_native_gui_layer_pool_slab_00ac4990(
    NativeGuiPoolSlabTable& table, std::byte* const& slab) {
    using namespace native_gui_layer_pool_table_detail;
    if (read(&table, 4) == read(&table, 8)) grow_native_gui_layer_pool_table_00ac43a0(table);
    const auto destination = read(&table) + read(&table, 4) * 4u;
    // AC49F4..49FC: read the supplied slab AFTER growth, only if destination
    // is nonnull, then increment the current count after the optional store.
    if (destination != 0) write(reinterpret_cast<void*>(destination), 0, read(&slab));
    write(&table, 4, read(&table, 4) + 1u);
}

__declspec(noinline) inline void reserve_native_gui_layer_pool_table_00ac4a10(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    using namespace native_gui_layer_pool_table_detail;
    if (read(&table, 8) < capacity)
        set_native_gui_layer_pool_table_capacity_00ac4350(table, capacity);
}
} // namespace bsp
