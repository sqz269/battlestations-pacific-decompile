#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {
// Actual F8C050+28h slab table, borrowed without a second header or pool.
// Evidence: bsp.py show AC9320/AC9370/AC97F0/AC9870 and AC9B30/AC9C10.
// SetCapacity/Reserve: __thiscall(ECX=table, stack capacity), RET 4;
// Grow: __fastcall(ECX=table), RET; PushBack: __thiscall(ECX=table,
// stack pointer-to-slab), RET 4. These C++ interfaces do not supply that ABI.
void native_gui_grid_pool_table_set_capacity_00ac9320(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void native_gui_grid_pool_table_grow_00ac9370(NativeGuiPoolSlabTable& table);
void native_gui_grid_pool_table_push_back_00ac97f0(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void native_gui_grid_pool_table_reserve_00ac9870(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

template<> void NativeGuiWidgetPool<GuiGridPoolTraits>::initialize();
template<> void* NativeGuiWidgetPool<GuiGridPoolTraits>::allocate();
} // namespace bsp
