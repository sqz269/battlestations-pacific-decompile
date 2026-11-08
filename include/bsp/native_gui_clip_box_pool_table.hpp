#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {

// 00ACE750 returns this in EAX; the existing process interface ignores that
// result. Both native lifetime members take ECX=pool and return with plain RET.
void construct_native_gui_clip_box_pool_00ace750(
    AllocatorListDomain& list, NativeGuiWidgetPoolStorage& storage);
void destroy_native_gui_clip_box_pool_00ace3e0(
    AllocatorListDomain& list, NativeGuiWidgetPoolStorage& storage);

// Source interfaces over the retained ClipBox pool's F8C13C+28h slab table.
// Evidence: bsp.py show 00ACE340/00ACE390/00ACE520/00ACE5B0 and
// 00ACE750/00ACE8D0. The constructor reserves 20h slots in unwind state 2;
// allocation appends its initialized slab inside the existing pool lock.
// These image template bodies are also inlined at those two caller sites.
// Substituting their equivalent address entries adds no second table or run.
// Native ABI: ECX=table; capacity/reserve/append RET4, grow RET.
// New C++ source interfaces, not native ABI adapters. Names are descriptive.
// 00ACE310: ECX=table, RET; free the current nonnull data without clearing it.
void free_native_gui_clip_box_pool_table_00ace310(void* actual_table) noexcept;
void native_gui_clip_box_pool_table_set_capacity_00ace340(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void native_gui_clip_box_pool_table_grow_00ace390(NativeGuiPoolSlabTable& table);
void native_gui_clip_box_pool_table_push_back_00ace520(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void native_gui_clip_box_pool_table_reserve_00ace5b0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

// ACE750's CB9043 unwind passes the SAME pool+28h table to ACE310.
// Explicit member specializations keep one canonical body for each address.
template<> void NativeGuiWidgetPool<GuiClipBoxPoolTraits>::initialize();
template<> void* NativeGuiWidgetPool<GuiClipBoxPoolTraits>::allocate();
template<> void NativeGuiWidgetPool<GuiClipBoxPoolTraits>::destroy();

} // namespace bsp
