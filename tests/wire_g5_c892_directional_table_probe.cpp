// Focused Win32 integration probe: real directional-pool growth and the
// table's allocation-failure/publication ordering. Compile without NDEBUG.
#include "bsp/directional_light_pool.hpp"
#include "bsp/directional_light_pool_table.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cassert>
#include <cstdio>
#include <new>
#include <new.h>
#include <vector>

namespace {
bsp::DirectionalLightPoolStorage* observed;
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
    bsp::DirectionalLightPoolStorage storage{};
    bsp::DirectionalLightPool pool(domain, storage);
    pool.initialize_00b7b940();
    assert(head == &storage.allocator_00 && storage.table_capacity_30 == 32);
    std::vector<void*> slots;
    for (unsigned i = 0; i < 1024; ++i) slots.push_back(pool.allocate_raw_slot_00b7bac0());
    assert(storage.slab_count_2c == 32 && storage.table_capacity_30 == 32);
    const std::vector<std::byte*> slabs(storage.slabs_28, storage.slabs_28 + 32);
    slots.push_back(pool.allocate_raw_slot_00b7bac0());
    assert(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66);
    for (unsigned i = 0; i < 32; ++i) assert(storage.slabs_28[i] == slabs[i]);
    for (unsigned i = 0; i < slots.size(); ++i)
        assert(bsp::DirectionalLightPool::live_slab_index(slots[i]) == i / 32);

    observed = &storage;
    old_table = storage.slabs_28;
    const auto old_handler = _set_new_handler(&allocation_failure);
    bool threw = false;
    try {
        bsp::reserve_directional_light_pool_table_00b7b5a0(
            {reinterpret_cast<std::byte*>(&storage) + 0x28}, 0x3fffffffu);
    } catch (const std::bad_alloc&) { threw = true; }
    _set_new_handler(old_handler);
    assert(threw && handler_saw_live_header && storage.slabs_28 == old_table);
    assert(storage.table_capacity_30 == 0x3fffffffu && storage.slab_count_2c == 33);
    // Probe-only repair after the intentionally failed reserve, before teardown.
    storage.table_capacity_30 = 66;
    for (void* slot : slots) pool.return_raw_slot_00b7b2f0(slot);
    pool.trim_empty_slabs_00b7ba20();
    assert(storage.slab_count_2c == 0 && storage.first_free_slab_34 == 0xffffffffu);
    pool.destroy_00b7b230();
    assert(head == nullptr);

    // The input cell aliases the table header: it must be read AFTER growth
    // publishes the new data pointer, rather than captured before allocation.
    struct Header { std::byte* data; std::uint32_t count, capacity; } header{};
    const bsp::DirectionalLightPoolTableView view{reinterpret_cast<std::byte*>(&header)};
    bsp::reserve_directional_light_pool_table_00b7b5a0(view, 1);
    auto* const value = reinterpret_cast<std::byte*>(&header);
    bsp::append_directional_light_pool_table_00b7b510(view, &value);
    bsp::append_directional_light_pool_table_00b7b510(view, &header.data);
    assert(header.count == 2 && header.capacity == 4);
    std::byte* copied;
    std::memcpy(&copied, header.data + 4, 4);
    assert(copied == header.data);
    bsp::singleton_lifetime_free(header.data);

    // Native skips a null destination without reading its input cell.
    header = {nullptr, 0, 1};
    bsp::append_directional_light_pool_table_00b7b510(view, nullptr);
    assert(header.data == nullptr && header.count == 1 && header.capacity == 1);
    std::puts("directional table: real growth, slab identities, failure ordering, late input and null destination passed");
}
