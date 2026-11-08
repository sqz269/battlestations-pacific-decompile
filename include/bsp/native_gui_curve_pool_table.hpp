#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {
// Original ECX is the retained Curve pool+28h header (F8BE84), not a copy.
// SetCapacity/Reserve: __thiscall, stack capacity, RET 4; Grow: __fastcall,
// RET; PushBack: __thiscall, stack pointer-to-slab, RET 4. Source interfaces
// below retain the Win32 fields but are not binary ABI replacement thunks.
void native_gui_curve_pool_table_set_capacity_00abc1e0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void native_gui_curve_pool_table_grow_00abc230(NativeGuiPoolSlabTable& table);
void native_gui_curve_pool_table_push_back_00abc480(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void native_gui_curve_pool_table_reserve_00abc500(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
// ECX points to the original pool+28h table. Releases only its current data;
// the pointer, count and capacity remain unchanged, including during unwind.
void free_native_gui_curve_pool_table_00abc180(void* actual_table) noexcept;

template<> void NativeGuiWidgetPool<GuiCurvePoolTraits>::initialize();
template<> void* NativeGuiWidgetPool<GuiCurvePoolTraits>::allocate();
template<> void NativeGuiWidgetPool<GuiCurvePoolTraits>::destroy();
} // namespace bsp
