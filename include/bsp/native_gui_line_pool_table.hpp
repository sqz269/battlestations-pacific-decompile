#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {

// Source interfaces for the Line pool's retained F8BEFC+28h slab table,
// not binary ABI adapters. Evidence: bsp.py show AC1CE0/AC1D10/AC1D60/
// AC1FA0/AC2020, AC2160 and AC2240. The original members take ECX=table;
// set-capacity/reserve/append pop one stack argument, grow/free pop none.
void set_native_gui_line_pool_table_capacity_00ac1d10(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void grow_native_gui_line_pool_table_00ac1d60(NativeGuiPoolSlabTable& table);
void push_native_gui_line_pool_slab_00ac1fa0(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void reserve_native_gui_line_pool_table_00ac2020(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void free_native_gui_line_pool_table_00ac1ce0(void* actual_table) noexcept;

} // namespace bsp
