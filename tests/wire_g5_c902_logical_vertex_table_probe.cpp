// Focused Win32 integration probe, not gameplay or Original-ABI evidence.
#include "bsp/native_graphics_pool_lifetime.hpp"
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_logical_vertex_pool_return.hpp"
#include "bsp/native_logical_vertex_pool_table.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <new>
#include <new.h>
#include <vector>

namespace {
bsp::NativeGraphicsPoolStorage* observed;
std::byte** old_table;
bool saw_published_capacity;
int __cdecl allocation_failure(std::size_t bytes) {
    saw_published_capacity = bytes == 0xfffffffcu
        && observed->table_capacity_30 == 0x3fffffffu
        && observed->slabs_28 == old_table && observed->slab_count_2c == 33;
    return 0;
}
}

int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain domain(head);
    bsp::NativeGraphicsPoolStorage storage{};
    bsp::NativeGraphicsPoolLifetime pool(domain, storage,
        bsp::NativeGraphicsPoolKind::logical_vertex);
    pool.initialize();
    assert(head == &storage.allocator_00 && storage.table_capacity_30 == 32);
    std::vector<void*> slots;
    for (unsigned i = 0; i < 1024; ++i)
        slots.push_back(bsp::allocate_native_logical_vertex_slot_00b4ae80(&storage));
    assert(storage.slab_count_2c == 32 && storage.table_capacity_30 == 32);
    const std::vector<std::byte*> slabs(storage.slabs_28, storage.slabs_28 + 32);
    slots.push_back(bsp::allocate_native_logical_vertex_slot_00b4ae80(&storage));
    assert(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66);
    for (unsigned i = 0; i < 32; ++i) assert(storage.slabs_28[i] == slabs[i]);
    for (unsigned i = 0; i < slots.size(); ++i) {
        std::uint32_t index;
        std::memcpy(&index, static_cast<std::byte*>(slots[i]) + 0x74, 4);
        assert(index == i / 32);
    }
    assert(storage.recursion_24 == 0);

    observed = &storage;
    old_table = storage.slabs_28;
    const auto previous_handler = _set_new_handler(&allocation_failure);
    bool threw = false;
    try {
        bsp::reserve_native_logical_vertex_pool_table_00b4a850(
            &storage.slabs_28, 0x3fffffffu);
    } catch (const std::bad_alloc&) { threw = true; }
    _set_new_handler(previous_handler);
    assert(threw && saw_published_capacity && storage.slabs_28 == old_table);
    assert(storage.slab_count_2c == 33 && storage.table_capacity_30 == 0x3fffffffu);
    storage.table_capacity_30 = 66; // Probe repair after intentional failure.
    for (void* slot : slots)
        bsp::return_native_logical_vertex_pool_slot_00b49570(&storage, slot);
    pool.trim_empty_slabs();
    assert(storage.slab_count_2c == 0 && storage.first_free_slab_34 == 0xffffffffu);
    pool.destroy();
    assert(head == nullptr);

    // Append must consume an aliased source cell after growth publication.
    struct Header { void* data; std::uint32_t count, capacity; } header{};
    bsp::reserve_native_logical_vertex_pool_table_00b4a850(&header, 1);
    void* const value = &header;
    bsp::append_native_logical_vertex_pool_table_00b4a7d0(&header, &value);
    bsp::append_native_logical_vertex_pool_table_00b4a7d0(&header, &header.data);
    assert(header.count == 2 && header.capacity == 4);
    void* copied;
    std::memcpy(&copied, static_cast<std::byte*>(header.data) + 4, 4);
    assert(copied == header.data);
    bsp::singleton_lifetime_free(header.data);
    header = {nullptr, 0, 1};
    bsp::append_native_logical_vertex_pool_table_00b4a7d0(&header, nullptr);
    assert(header.data == nullptr && header.count == 1 && header.capacity == 1);
    std::puts("logical vertex table: real growth, identities, failure ordering, late source and null destination passed");
}
