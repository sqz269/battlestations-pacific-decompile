#include "bsp/game_native_resource_pools.hpp"
#include "bsp/native_point_light_pool_table.hpp"

#include <array>
#include <cstdio>

// Standalone Win32 fixture; it never launches the game. Link against bsp_core
// with /MANIFEST:EMBED. Exercise the production canonical owner and real native
// allocator, including automatic slab-table growth through the live slot path.
int main() {
    auto& process = bsp::game::game_native_resource_pool_process();
    const int status = process.initialize_point_light_once_00cd8060();
    auto& pool = process.point_light_pool_0109011c();
    auto& storage = pool.storage();
    auto** const initial = storage.slabs_28;
    if (!initial || storage.slab_count_2c != 0 || storage.table_capacity_30 != 32
        || storage.first_free_slab_34 != 0xffffffffu || storage.recursion_24 != 0)
        return 1;
    if (process.initialize_point_light_once_00cd8060() != status
        || storage.slabs_28 != initial) return 2;

    // Cross the initial 32-slab capacity: the 1025th slot appends slab 33,
    // exercising 00B7B410 -> 00B7AE30 and retaining all earlier slab entries.
    std::array<void*, 1025> slots;
    for (auto& slot : slots) slot = pool.allocate_raw_slot_00b7b810();
    if (storage.slab_count_2c != 33 || storage.table_capacity_30 != 66
        || storage.slabs_28 == initial) return 3;
    for (std::size_t i = 0; i < slots.size(); ++i) {
        if (slots[i] != storage.slabs_28[i / 32] + (i % 32) * 0x200) return 7;
    }
    auto* const first_slab = storage.slabs_28[0];
    auto* const second_slab = storage.slabs_28[1];
    auto** const automatic = storage.slabs_28;
    bsp::reserve_native_point_light_pool_table_00b7b4a0({&storage.slabs_28}, 128);
    if (storage.slabs_28 == automatic || storage.table_capacity_30 != 128
        || storage.slab_count_2c != 33 || storage.slabs_28[0] != first_slab
        || storage.slabs_28[1] != second_slab) return 4;
    auto** const grown = storage.slabs_28;
    bsp::reserve_native_point_light_pool_table_00b7b4a0({&storage.slabs_28}, 32);
    if (storage.slabs_28 != grown || storage.table_capacity_30 != 128) return 5;
    for (auto* slot : slots) pool.return_raw_slot_00b7b1d0(slot);
    pool.trim_empty_slabs_00b7b770();
    if (storage.slab_count_2c != 0 || storage.first_free_slab_34 != 0xffffffffu
        || storage.recursion_24 != 0) return 6;
    // CE0EA0 performs the remaining empty-table/section/list teardown at exit.
    std::puts("PointLight canonical startup, automatic 32-to-66 slab-table growth, reserve and trim passed");
}
