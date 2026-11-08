// One actual-owner scenario covers table growth and an interrupted reserve.
#include "bsp/native_mesh_pool.hpp"
#include "bsp/native_mesh_pool_table.hpp"

#include <cassert>
#include <cstdio>
#include <new>
#include <new.h>
#include <vector>

namespace {
bsp::NativeMeshPoolStorage* observed;
std::byte** old_table;
bool handler_saw_live_header;
int __cdecl allocation_failure(std::size_t bytes) {
    handler_saw_live_header = bytes == 0xfffffffcu
        && observed->table_capacity_30 == 0x3fffffffu
        && observed->slabs_28 == old_table && observed->slab_count_2c == 33;
    return 0;
}
}

int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain domain(head);
    bsp::NativeMeshPoolStorage storage{};
    bsp::NativeMeshPool pool(domain, storage);
    pool.initialize_00b73890();
    assert(head == &storage.allocator_00 && storage.table_capacity_30 == 32);
    std::vector<void*> slots;
    for (unsigned i = 0; i < 1024; ++i) slots.push_back(pool.allocate_slot_00b73a10());
    assert(storage.slab_count_2c == 32 && storage.table_capacity_30 == 32);
    const std::vector<std::byte*> slabs(storage.slabs_28, storage.slabs_28 + 32);
    slots.push_back(pool.allocate_slot_00b73a10());
    assert(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66);
    for (unsigned i = 0; i < 32; ++i) assert(storage.slabs_28[i] == slabs[i]);
    for (unsigned i = 0; i < slots.size(); ++i)
        assert(bsp::NativeMeshPool::live_slab_index(slots[i]) == i / 32);

    observed = &storage;
    old_table = storage.slabs_28;
    const auto old_handler = _set_new_handler(&allocation_failure);
    bool threw = false;
    try {
        bsp::reserve_native_mesh_pool_table_00b72f10(
            {reinterpret_cast<std::byte*>(&storage.slabs_28)}, 0x3fffffffu);
    } catch (const std::bad_alloc&) { threw = true; }
    _set_new_handler(old_handler);
    assert(threw && handler_saw_live_header && storage.slabs_28 == old_table);
    assert(storage.table_capacity_30 == 0x3fffffffu && storage.slab_count_2c == 33);
    // Fixture-only repair after checking the intentionally failed operation.
    storage.table_capacity_30 = 66;
    for (void* slot : slots) pool.return_slot_00b72da0(slot);
    pool.trim_empty_slabs_00b73970();
    assert(storage.slab_count_2c == 0 && storage.first_free_slab_34 == 0xffffffffu);
    pool.destroy_00b72ce0();
    assert(head == nullptr);
    std::puts("mesh table: live growth, slot identities, failure publication and teardown passed");
}
