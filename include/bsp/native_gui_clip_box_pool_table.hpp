#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {

// Actual ClipBox pool F8C13C+28h Mit vector, shared by CRT CD76D0 and
// the existing ACEA10 factory. Names are hypotheses; these C++ interfaces
// preserve storage/order, not the original thiscall stack or native FH3 ABI.
// Evidence: bsp.py show ACE310/ACE340/ACE390/ACE520/ACE5B0, ACE750/ACE8D0/
// ACE3E0; ACE750's CB9043 unwind tail-jumps to ACE310.
void free_native_gui_clip_box_pool_table_00ace310(void* actual_table) noexcept;
void set_native_gui_clip_box_pool_table_capacity_00ace340(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void grow_native_gui_clip_box_pool_table_00ace390(NativeGuiPoolSlabTable& table);
void push_native_gui_clip_box_pool_slab_00ace520(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void reserve_native_gui_clip_box_pool_table_00ace5b0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

template<> void NativeGuiWidgetPool<GuiClipBoxPoolTraits>::initialize();
template<> void* NativeGuiWidgetPool<GuiClipBoxPoolTraits>::allocate();
template<> void NativeGuiWidgetPool<GuiClipBoxPoolTraits>::destroy();

} // namespace bsp
