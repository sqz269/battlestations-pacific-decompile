// One focused c858 check: the actual Group pool crosses its 32-slab table.
#include "bsp/native_gui_group_storage.hpp"

#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain list(head);
    bsp::NativeGuiGroupPoolStorage storage;
    std::memset(&storage, 0xa5, sizeof storage);
    bsp::NativeGuiGroupPool pool(list, storage);
    pool.initialize_00ac7410();
    require(head == &storage.allocator_00 && storage.slab_count_2c == 0
        && storage.table_capacity_30 == 32 && storage.recursion_24 == 0,
        "Group initialization did not reserve its actual table");

    std::array<void*, 1025> slots;
    std::array<std::byte*, 32> original_slabs;
    for (std::size_t i = 0; i < slots.size(); ++i) {
        if (i == 1024)
            std::memcpy(original_slabs.data(), storage.slabs_28, sizeof original_slabs);
        slots[i] = pool.allocate_00ac7590();
        std::uint32_t slab_id;
        std::memcpy(&slab_id, static_cast<std::byte*>(slots[i]) + pool.slot_slab_index_offset, 4);
        require(slab_id == i / pool.slots_per_slab, "Group hidden slab ID changed");
    }
    require(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66
        && storage.first_free_slab_34 == 32 && storage.recursion_24 == 0,
        "Group append/grow did not preserve count, capacity or lock state");
    for (std::size_t i = 0; i < original_slabs.size(); ++i)
        require(storage.slabs_28[i] == original_slabs[i], "Group growth lost a live slab");
    std::uint16_t free_slots;
    std::memcpy(&free_slots, storage.slabs_28[32] + pool.free_count_offset, 2);
    require(free_slots == 31, "Group append did not retain the initialized new slab");

    for (void* slot : slots) pool.return_00ac7260(slot);
    pool.trim_00ac74f0();
    require(storage.slab_count_2c == 0, "Group retirement left empty slabs");
    pool.destroy_00ac71a0();
    require(head == nullptr, "Group destruction left its allocator-list link");
    std::puts("c858 Group pool: 1025 slots, 33 slabs, capacity 32 -> 66; identities/IDs/lock/retirement passed");
}
