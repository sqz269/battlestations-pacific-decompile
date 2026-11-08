#include "bsp/native_graphics_pool_lifetime.hpp"
#include "bsp/native_logical_index_owner.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// One integration boundary: the actual owner's reserve32 followed by the
// 1025th raw slot, which must preserve all 32 slabs while growing to 66.
// Calls production lifecycle/allocation bodies from bsp_core.lib.
int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain list(head);
    bsp::NativeGraphicsPoolStorage pool{};
    bsp::NativeGraphicsPoolLifetime owner(list, pool,
        bsp::NativeGraphicsPoolKind::logical_index);
    owner.initialize();
    auto check = [](bool condition) { if (!condition) std::abort(); };
    check(pool.table_capacity_30 == 32 && pool.slab_count_2c == 0);
    std::vector<void*> slots;
    for (std::uint32_t i = 0; i < 1025; ++i) {
        void* slot = bsp::allocate_native_logical_index_slot_00b4b0a0(&pool);
        std::memcpy(slot, &i, 4);
        slots.push_back(slot);
    }
    check(pool.slab_count_2c == 33 && pool.table_capacity_30 == 66);
    check(pool.first_free_slab_34 == 32 && pool.recursion_24 == 0);
    for (std::uint32_t i = 0; i < slots.size(); ++i) {
        std::uint32_t value, slab_index;
        std::memcpy(&value, slots[i], 4);
        std::memcpy(&slab_index, static_cast<unsigned char*>(slots[i]) + 0x24, 4);
        check(value == i && slab_index == i / 32);
        check(slots[i] == pool.slabs_28[slab_index] + (i % 32) * 0x28);
    }
    owner.destroy();
    check(head == nullptr);
    std::puts("logical-index table boundary passed: 1025 slots, capacity32->66");
}
