#pragma once

#include "bsp/cc13_sweepC.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <new>

namespace bsp {

// Source interfaces for the Sound pool's retained F8C0EC+28h slab table.
// Evidence: bsp.py show ACD200/ACD250/ACD430/ACD4C0 and ACD530/ACD6B0.
// The constructor/allocator inline the reserve/append schedules. These entries
// reuse those bodies without adding a table, lock, allocation or second run.
// 00ACD1D0: ECX points to the actual {data,count,capacity} header, RET.
// bsp.py show 00ACD1D0: capture data, free it when nonnull, leave all words
// unchanged. ACD2F0 inlines this after slab retirement and before CS teardown.
// This is a source substitution for that inline schedule, with a new C++ ABI.
__declspec(noinline) inline void free_native_gui_sound_pool_table_00acd1d0(
    void* actual_table) noexcept {
    void* const captured = *static_cast<void* const volatile*>(actual_table);
    if (captured != nullptr) singleton_lifetime_free(captured);
}

__declspec(noinline) inline void set_native_gui_sound_pool_table_capacity_00acd200(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    // ACD200 has AA6FA0's exact capacity-before-allocation, null-tested
    // copy, free-old, adopt-new schedule, including DWORD byte-size wrapping.
    native_gui_pool_table_set_capacity_00aa6fa0(table, capacity);
}

class NativeGuiSoundTableAllocation final : public cc13_sweepC::PointerTableAllocationHost {
public:
    void* operator_new_00bf55be(std::uint32_t bytes) override {
        return singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    }
    void operator_delete_00bf6989(void* block) override { singleton_lifetime_free(block); }
};

__declspec(noinline) inline void append_native_gui_sound_pool_table_00acd430(
    NativeGuiPoolSlabTable& table, std::byte* const& slab) {
    if (table.count == table.capacity) {
        NativeGuiSoundTableAllocation allocation;
        // Reuse the sole ACD250 body with the live header and real CRT imports.
        cc13_sweepC::pointer_table_grow_00acd250(allocation, &table);
    }
    const auto destination = reinterpret_cast<std::uintptr_t>(table.data) + table.count * 4u;
    if (destination != 0)
        ::new (reinterpret_cast<void*>(destination)) std::byte*(slab);
    ++table.count;
}

__declspec(noinline) inline void reserve_native_gui_sound_pool_table_00acd4c0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    // ACD530 reserves 32 entries on this header inside constructor state 2.
    if (table.capacity < capacity)
        set_native_gui_sound_pool_table_capacity_00acd200(table, capacity);
}

} // namespace bsp
