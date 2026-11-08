#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {

// 00AAB7F0: ECX points at the actual pool+28h table header, plain RET.
// Free its nonnull data pointer without clearing the header. This address
// entry reuses msvc9_containers::free_pointer's existing algorithm; it is a
// source interface, not a native register-ABI thunk.
void free_native_gui_button_pool_table_00aab7f0(void* actual_table) noexcept;

// Button's Mit vector takes its actual pool+28h {data,count,capacity} header.
// Names are hypotheses. AAB8E0/AABC70/AABD00 use ECX=header, one stack
// argument, RET4; AAB930 uses ECX=header, RET. These C++ entries reuse the
// reconstructed vector operations; they do not implement the register ABI.
// Keep the appended source alive across growth (the real allocator supplies
// its local slab pointer). The caller owns locking and allocation failures.
void set_native_gui_button_pool_table_capacity_00aab8e0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void grow_native_gui_button_pool_table_00aab930(NativeGuiPoolSlabTable& table);
void push_native_gui_button_pool_slab_00aabc70(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void reserve_native_gui_button_pool_table_00aabd00(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

} // namespace bsp
