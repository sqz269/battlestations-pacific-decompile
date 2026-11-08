#pragma once

#include "bsp/cc13_sweepC.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <new>

namespace bsp {

// These entries borrow the Movie pool's actual F8BD4C+28h header; no second
// table or allocator is retained. Source interfaces, not original ABI adapters.
// Evidence: bsp.py show 00AACDF0/00AACE40/00AAD450/00AAD4D0 and 00AAD5F0/00AAD6D0.
// Capacity is published before allocation and count after the append. Failure
// retains the image's partial state and the caller's existing lock scope.
__declspec(noinline) inline void native_gui_movie_pool_table_set_capacity_00aacdf0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    // Reuse the existing reconstructed AACDF0 source alias, whose algorithm
    // is shared with AA6FA0; do not add another copy/allocation implementation.
    native_gui_pool_table_set_capacity(table, capacity);
}

class NativeGuiMovieTableAllocation final : public cc13_sweepC::PointerTableAllocationHost {
public:
    void* operator_new_00bf55be(std::uint32_t bytes) override {
        return singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    }
    void operator_delete_00bf6989(void* block) override { singleton_lifetime_free(block); }
};

__declspec(noinline) inline void native_gui_movie_pool_table_push_back_00aad450(
    NativeGuiPoolSlabTable& table, std::byte* const& slab) {
    if (table.count == table.capacity) {
        NativeGuiMovieTableAllocation allocation;
        // Reuse the sole AACE40 body on this same table and real CRT imports.
        cc13_sweepC::pointer_table_grow_00aace40(allocation, &table);
    }
    const auto destination = reinterpret_cast<std::uintptr_t>(table.data) + table.count * 4u;
    if (destination != 0)
        ::new (reinterpret_cast<void*>(destination)) std::byte*(slab);
    ++table.count;
}

__declspec(noinline) inline void native_gui_movie_pool_table_reserve_00aad4d0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    if (table.capacity < capacity)
        native_gui_movie_pool_table_set_capacity_00aacdf0(table, capacity);
}

} // namespace bsp
