#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {
// Original ECX is the SAME Scrollbar pool's +28h vector (00F8C02C):
// data/count/capacity at +0/+4/+8. SetCapacity/Reserve are __thiscall,
// capacity on the stack, RET4; Grow is __fastcall/RET; PushBack is
// __thiscall, pointer-to-slab on the stack, RET4. These source interfaces
// borrow that actual header; they are not binary ABI replacement thunks.
// Evidence: bsp.py show 00AC7F40/00AC7F90/00AC8110/00AC81A0, plus the
// inlined reserve(32) in AC8240 and append under the AC83C0 pool lock.
void native_gui_scrollbar_pool_table_set_capacity_00ac7f40(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void native_gui_scrollbar_pool_table_grow_00ac7f90(NativeGuiPoolSlabTable& table);
void native_gui_scrollbar_pool_table_push_back_00ac8110(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void native_gui_scrollbar_pool_table_reserve_00ac81a0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

template<> void NativeGuiWidgetPool<GuiScrollbarPoolTraits>::initialize();
template<> void* NativeGuiWidgetPool<GuiScrollbarPoolTraits>::allocate();
} // namespace bsp
