#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {
// 00ACEEE0: __fastcall(ECX=table), plain RET. Capture [table] once and
// free it when nonnull without changing the data/count/capacity words.
// Source interface over the genuine F8C188+28h table, not a native ABI thunk.
void free_native_gui_frame_box_pool_table_00aceee0(void* actual_table) noexcept;

// Original ECX is the retained FrameBox pool F8C188+28h slab table. These
// source interfaces preserve its three DWORD fields; they are not ABI thunks.
// ACEF70/ACF860: __thiscall(capacity), RET 4. ACF7E0: __thiscall(&slab), RET 4.
// ACEFC0 (existing cc13_sweepC body): ECX=table, RET, no EDX input.
void native_gui_frame_box_pool_table_set_capacity_00acef70(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void native_gui_frame_box_pool_table_push_back_00acf7e0(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void native_gui_frame_box_pool_table_reserve_00acf860(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

template<> void NativeGuiWidgetPool<GuiFrameBoxPoolTraits>::initialize();
template<> void* NativeGuiWidgetPool<GuiFrameBoxPoolTraits>::allocate();
template<> void NativeGuiWidgetPool<GuiFrameBoxPoolTraits>::destroy();
} // namespace bsp
