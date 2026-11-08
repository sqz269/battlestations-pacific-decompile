// One integration probe for the live Node slab-table replacement: growth must
// retain slot/slab identities, and a throwing allocator must see published cap.
#include "bsp/native_node_pool_owner.hpp"
#include "bsp/native_node_pool_allocation.hpp"
#include "bsp/native_node_pool_table.hpp"

#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <new>
#include <new.h>
#include <vector>

namespace {
std::array<std::uint32_t, 14>* observed;
std::uint32_t old_table;
bool saw_publication;
int __cdecl allocation_failure(std::size_t bytes) {
    saw_publication = bytes == 0xfffffffcu && (*observed)[0x30 / 4] == 0x3fffffffu
        && (*observed)[0x28 / 4] == old_table && (*observed)[0x2c / 4] == 33;
    return 0;
}
}

int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain domain(head);
    std::array<std::uint32_t, 14> pool{};
    bsp::initialize_native_node_pool_00b6e980(pool.data(), domain);
    assert(head == reinterpret_cast<bsp::AllocatorListElement*>(pool.data()));
    assert(pool[0x30 / 4] == 32 && pool[0x2c / 4] == 0);
    std::vector<void*> slots;
    for (unsigned i = 0; i < 1024; ++i)
        slots.push_back(bsp::allocate_native_node_pool_slot_00b6eb00(pool.data()));
    assert(pool[0x2c / 4] == 32 && pool[0x30 / 4] == 32);
    const auto* table = reinterpret_cast<const std::uint32_t*>(pool[0x28 / 4]);
    const std::vector<std::uint32_t> slabs(table, table + 32);
    slots.push_back(bsp::allocate_native_node_pool_slot_00b6eb00(pool.data()));
    assert(pool[0x2c / 4] == 33 && pool[0x30 / 4] == 66 && pool[0x24 / 4] == 0);
    table = reinterpret_cast<const std::uint32_t*>(pool[0x28 / 4]);
    for (unsigned i = 0; i < 32; ++i) assert(table[i] == slabs[i]);
    for (unsigned i = 0; i < slots.size(); ++i) {
        std::uint32_t id;
        std::memcpy(&id, static_cast<const std::byte*>(slots[i]) + 0x174, 4);
        assert(id == i / 32);
        assert(reinterpret_cast<std::uintptr_t>(slots[i]) == table[id] + (i % 32) * 0x178u);
    }

    observed = &pool;
    old_table = pool[0x28 / 4];
    const auto previous_handler = _set_new_handler(&allocation_failure);
    bool threw = false;
    try {
        bsp::reserve_native_node_pool_table_00b6e610({pool.data() + 0x28 / 4}, 0x3fffffffu);
    } catch (const std::bad_alloc&) { threw = true; }
    _set_new_handler(previous_handler);
    assert(threw && saw_publication && pool[0x28 / 4] == old_table);
    assert(pool[0x30 / 4] == 0x3fffffffu && pool[0x2c / 4] == 33);
    // Fixture-only repair after the deliberately failed reserve.
    pool[0x30 / 4] = 66;
    for (void* slot : slots) bsp::return_native_node_pool_slot_00b6e490(pool.data(), slot);
    bsp::trim_native_node_pool_00b6ea60(pool.data());
    assert(pool[0x2c / 4] == 0 && pool[0x34 / 4] == 0xffffffffu);
    bsp::destroy_native_node_pool_00b6e3d0(pool.data(), domain);
    assert(head == nullptr);
    std::puts("node table: growth, slot identities, failure publication, trim and teardown passed");
}
