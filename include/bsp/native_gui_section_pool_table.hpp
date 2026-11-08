#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {
// Original ECX is Section's actual pool+28h table (F8BED4). Free is
// __fastcall, RET; SetCapacity/Reserve are __thiscall, stack capacity, RET 4.
// These C++ interfaces preserve the Win32 fields, not the original register ABI.
void free_native_gui_section_pool_table_00abe9d0(void* actual_table) noexcept;
void set_native_gui_section_pool_table_capacity_00abea00(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void reserve_native_gui_section_pool_table_00abed30(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

template<> void NativeGuiWidgetPool<GuiSectionPoolTraits>::initialize();
template<> void NativeGuiWidgetPool<GuiSectionPoolTraits>::destroy();
} // namespace bsp
