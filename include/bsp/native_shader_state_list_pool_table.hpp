#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {

// Original 00B621E0/00B62230/00B622E0/00B62360: the pointer-array members
// used by the ShaderStateList pool at 0108FEE4. ECX addresses its actual
// +28h data/count/capacity header. SetCapacity, PushBack and Reserve RET 4;
// Grow has no stack arguments. These C++ interfaces do not preserve that ABI.
// Names are descriptive hypotheses; evidence is each address's Ghidra export.
void native_shader_state_list_pool_table_set_capacity_00b621e0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void native_shader_state_list_pool_table_grow_00b62230(NativeGuiPoolSlabTable& table);
void native_shader_state_list_pool_table_push_back_00b622e0(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void native_shader_state_list_pool_table_reserve_00b62360(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

} // namespace bsp
