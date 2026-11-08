#include "bsp/native_graphics_pool_lifetime.hpp"
#include "bsp/native_hardware_layout_owner_table.hpp"

#include <cstdio>
#include <cstdlib>
#include <new>
#include <new.h>

namespace {
void check(bool value) { if (!value) std::abort(); }
bsp::NativeGraphicsPoolStorage* observed;
std::byte** old_table;
unsigned handler_calls;
int __cdecl allocation_failed(std::size_t bytes) {
    ++handler_calls;
    check(bytes == 0xfffffff8u);
    check(observed->table_capacity_30 == 0x3ffffffeu);
    check(observed->slabs_28 == old_table && observed->slab_count_2c == 2);
    return 0;
}
} // namespace

// One production integration boundary: the real hardware-layout profile's
// startup reserve32, then the same borrowed table's exact copy and real CRT
// allocation failure. No game process or invented production slab allocation.
int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain list(head);
    bsp::NativeGraphicsPoolStorage pool{};
    bsp::NativeGraphicsPoolLifetime owner(list, pool,
        bsp::NativeGraphicsPoolKind::hardware_layout);
    owner.initialize();
    check(head == &pool.allocator_00 && pool.table_capacity_30 == 32);
    check(pool.slabs_28 != nullptr && pool.slab_count_2c == 0);
    check(pool.first_free_slab_34 == 0xffffffffu && pool.recursion_24 == 0);
    auto* const header = reinterpret_cast<std::byte*>(&pool) + 0x28;
    auto** const initial = pool.slabs_28;
    bsp::reserve_native_hardware_layout_owner_table_00b482d0(header, 16);
    check(pool.slabs_28 == initial && pool.table_capacity_30 == 32);

    // Fixture slabs have the native allocation extent. Teardown frees these
    // same pointers; no payload or free-index fields are read by this probe.
    auto* const first = static_cast<std::byte*>(std::malloc(0x8c4));
    auto* const second = static_cast<std::byte*>(std::malloc(0x8c4));
    check(first && second);
    pool.slabs_28[0] = first;
    pool.slabs_28[1] = second;
    pool.slab_count_2c = 2;
    bsp::reserve_native_hardware_layout_owner_table_00b482d0(header, 64);
    check(pool.table_capacity_30 == 64 && pool.slab_count_2c == 2);
    check(pool.slabs_28[0] == first && pool.slabs_28[1] == second);

    observed = &pool;
    old_table = pool.slabs_28;
    const auto previous_handler = _set_new_handler(&allocation_failed);
    bool threw = false;
    try {
        bsp::set_native_hardware_layout_owner_table_capacity_00b47b60(header, 0x3ffffffeu);
    } catch (const std::bad_alloc&) { threw = true; }
    _set_new_handler(previous_handler);
    check(threw && handler_calls == 1);
    check(pool.table_capacity_30 == 0x3ffffffeu && pool.slabs_28 == old_table);
    check(pool.slabs_28[0] == first && pool.slabs_28[1] == second);
    owner.destroy();
    check(head == nullptr);
    std::puts("hardware-layout owner table passed: startup reserve32, copy, no shrink, failure publication");
}
