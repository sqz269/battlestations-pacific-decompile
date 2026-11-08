#include "bsp/native_shader_state_list_pool_table.hpp"

#include <new>

namespace bsp {

// Original B621E0/B62230/B622E0/B62360 duplicate the Widget table's
// allocation/copy/free schedule. B623D0 construction and B62630 allocation
// inline these operations on the actual retained 0108FEE4 pool+28h header.
// This is source reuse, not an original CALL edge or register-ABI replacement.
// Keep each address entry out of line for compiled reachability evidence.
__declspec(noinline) void native_shader_state_list_pool_table_set_capacity_00b621e0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    // B621E0 publishes capacity BEFORE allocation; the shared body reloads the
    // live count/data after allocation, null-tests each destination, frees the
    // old backing and only then publishes the replacement. No second copy.
    native_gui_pool_table_set_capacity_00aa6fa0(table, capacity);
}

__declspec(noinline) void native_shader_state_list_pool_table_grow_00b62230(
    NativeGuiPoolSlabTable& table) {
    native_shader_state_list_pool_table_set_capacity_00b621e0(
        table, table.capacity * 2u + 2u);
}

__declspec(noinline) void native_shader_state_list_pool_table_push_back_00b622e0(
    NativeGuiPoolSlabTable& table, std::byte* const& slab) {
    if (table.count == table.capacity)
        native_shader_state_list_pool_table_grow_00b62230(table);
    const auto destination = reinterpret_cast<std::uintptr_t>(table.data) + table.count * 4u;
    if (destination != 0)
        ::new (reinterpret_cast<void*>(destination)) std::byte*(slab);
    ++table.count;
}

__declspec(noinline) void native_shader_state_list_pool_table_reserve_00b62360(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    if (table.capacity < capacity)
        native_shader_state_list_pool_table_set_capacity_00b621e0(table, capacity);
}

} // namespace bsp
