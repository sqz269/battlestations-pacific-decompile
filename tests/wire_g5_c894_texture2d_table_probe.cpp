// One actual-owner scenario: cross the retained table's first growth boundary,
// then interrupt a reserve to inspect the native capacity-before-allocation rule.
#include "bsp/d3d9_texture2d_pool.hpp"
#include "bsp/d3d9_texture2d_pool_table.hpp"

#include <cassert>
#include <cstdio>
#include <new>
#include <new.h>
#include <vector>

namespace {
bsp::D3D9Texture2DPoolStorage* observed;
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
    bsp::D3D9Texture2DPoolStorage storage{};
    bsp::D3D9Texture2DPool pool(domain, storage);
    pool.initialize_00b3ee80();
    assert(head == &storage.allocator_00 && storage.table_capacity_30 == 32);
    std::vector<void*> slots;
    for (unsigned i = 0; i < 1024; ++i) slots.push_back(pool.allocate_raw_slot_00b3ef60());
    assert(storage.slab_count_2c == 32 && storage.table_capacity_30 == 32);
    const std::vector<std::byte*> slabs(storage.slabs_28, storage.slabs_28 + 32);
    slots.push_back(pool.allocate_raw_slot_00b3ef60());
    assert(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66);
    for (unsigned i = 0; i < 32; ++i) assert(storage.slabs_28[i] == slabs[i]);
    for (unsigned i = 0; i < slots.size(); ++i)
        assert(bsp::D3D9Texture2DPool::live_slab_index(slots[i]) == i / 32);

    observed = &storage;
    old_table = storage.slabs_28;
    const auto old_handler = _set_new_handler(&allocation_failure);
    bool threw = false;
    try {
        bsp::reserve_d3d9_texture2d_pool_table_00b3e960(
            {reinterpret_cast<std::byte*>(&storage.slabs_28)}, 0x3fffffffu);
    } catch (const std::bad_alloc&) { threw = true; }
    _set_new_handler(old_handler);
    assert(threw && handler_saw_live_header && storage.slabs_28 == old_table);
    assert(storage.table_capacity_30 == 0x3fffffffu && storage.slab_count_2c == 33);
    storage.table_capacity_30 = 66; // fixture-only repair after observing failure
    for (void* slot : slots) pool.return_raw_slot_00b3d8d0(slot);
    pool.trim_empty_slabs_00b3e510();
    assert(storage.slab_count_2c == 0 && storage.first_free_slab_34 == 0xffffffffu);
    pool.destroy_00b3e430();
    assert(head == nullptr);
    std::puts("texture2d table: growth, slot identities, failure publication and teardown passed");
}
