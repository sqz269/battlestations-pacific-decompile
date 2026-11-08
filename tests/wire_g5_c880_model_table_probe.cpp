// Focused real-pool table growth/retention check; standalone MSVC Win32 probe.
// Link the built bsp_core library, including its real allocation services.
#include "bsp/native_model_pool.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
void require(bool condition) { if (!condition) std::abort(); }
}

int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain list(head);
    bsp::NativeModelPoolStorage storage;
    std::memset(&storage, 0xa5, sizeof(storage));
    bsp::NativeModelPool pool(list, storage);
    pool.initialize_00b74b80();
    require(storage.slabs_28 && storage.table_capacity_30 == 32);
    require(storage.slab_count_2c == 0 && storage.first_free_slab_34 == 0xffffffffu);
    void* slots[1025];
    for (std::uint32_t i = 0; i < 1025; ++i) {
        slots[i] = pool.allocate_raw_slot_00b74d00();
        std::memcpy(slots[i], &i, sizeof(i));
    }
    require(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66);
    require(storage.recursion_24 == 0 && head == &storage.allocator_00);
    for (std::uint32_t i = 0; i < 1025; ++i) {
        std::uint32_t value;
        std::memcpy(&value, slots[i], sizeof(value));
        require(value == i && bsp::NativeModelPool::live_slab_index(slots[i]) == i / 32);
    }
    pool.return_raw_slot_00b74750(slots[1024]);
    list.trim_all_004b46b0();
    require(storage.slab_count_2c == 32 && storage.table_capacity_30 == 66);
    for (std::uint32_t i = 0; i < 1024; ++i) pool.return_raw_slot_00b74750(slots[i]);
    list.trim_all_004b46b0();
    require(storage.slab_count_2c == 0 && storage.table_capacity_30 == 66);
    require(storage.slabs_28 && storage.first_free_slab_34 == 0xffffffffu);
    pool.destroy_00b74690();
    require(!head);
    std::puts("Model table probe PASS: real allocation services, 1025 slots, 33 slabs, cap32->66, preserved payload/trailers, trim retains table/capacity, teardown unlinks owner");
}
