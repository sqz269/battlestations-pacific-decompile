#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {

// Mit pointer-vector entries over the actual Icon pool's +28h header:
// {data, count, capacity}. The names are hypotheses, not recovered symbols.
// Original 00AB1580/00AB23A0: ECX=header, one stack argument, RET4;
// 00AB15D0: ECX=header, RET. These are ordinary C++ source interfaces.
// Keep the table and any appended source pointer live; a growth must not
// invalidate that source. The real Icon allocator supplies its local slab.
// No lock, allocation-failure recovery, payload ownership or ABI thunk is added.
void set_native_gui_icon_pool_table_capacity_00ab1580(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void grow_native_gui_icon_pool_table_00ab15d0(NativeGuiPoolSlabTable& table);
void push_native_gui_icon_pool_slab_00ab23a0(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);

} // namespace bsp
