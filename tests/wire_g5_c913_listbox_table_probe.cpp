// One actual-owner scenario: table growth preserves slabs and a failed reserve
// publishes capacity before the new-handler sees the live pool header.
#include "bsp/cc13_seg87.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <new>
#include <new.h>
#include <vector>

namespace {
bsp::NativeGuiWidgetPoolStorage* observed;
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
    bsp::NativeGuiWidgetPoolStorage storage{};
    bsp::NativeGuiListboxPool pool(domain, storage);
    pool.initialize();
    assert(head == &storage.allocator_00 && storage.table_capacity_30 == 32);
    std::vector<void*> slots;
    for (unsigned i = 0; i < 512; ++i) slots.push_back(pool.allocate());
    assert(storage.slab_count_2c == 32 && storage.table_capacity_30 == 32);
    const std::vector<std::byte*> slabs(storage.slabs_28, storage.slabs_28 + 32);
    slots.push_back(pool.allocate());
    assert(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66);
    for (unsigned i = 0; i < 32; ++i) assert(storage.slabs_28[i] == slabs[i]);
    for (unsigned i = 0; i < slots.size(); ++i) {
        std::uint32_t index;
        std::memcpy(&index, static_cast<std::byte*>(slots[i]) +
            bsp::GuiListboxPoolTraits::slot_slab_index_offset, sizeof index);
        assert(index == i / 16);
    }

    auto& table = bsp::native_gui_pool_slab_table(storage);
    old_table = storage.slabs_28;
    bsp::reserve_native_listbox_pool_table_00a9b2d0(table, 65);
    assert(storage.slabs_28 == old_table && storage.table_capacity_30 == 66);
    observed = &storage;
    const auto old_handler = _set_new_handler(&allocation_failure);
    bool threw = false;
    try {
        bsp::reserve_native_listbox_pool_table_00a9b2d0(table, 0x3fffffffu);
    } catch (const std::bad_alloc&) { threw = true; }
    _set_new_handler(old_handler);
    assert(threw && handler_saw_live_header && storage.slabs_28 == old_table);
    assert(storage.table_capacity_30 == 0x3fffffffu && storage.slab_count_2c == 33);
    // Fixture-only repair of the deliberately interrupted reserve.
    storage.table_capacity_30 = 66;
    for (void* slot : slots) pool.free(slot);
    pool.trim();
    assert(storage.slab_count_2c == 0 && storage.first_free_slab_34 == 0xffffffffu);
    pool.destroy();
    assert(head == nullptr && storage.slabs_28 == old_table && storage.table_capacity_30 == 66);
    std::puts("c913 listbox table: actual reserve32, grow32->66, slab identities, "
        "no-op reserve, failure publication and teardown passed");
}
