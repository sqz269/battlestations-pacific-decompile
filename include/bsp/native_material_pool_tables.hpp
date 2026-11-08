#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {
// The existing 0Ch slab-table view also describes the actual Material F8D3AC
// and MaterialParameter F8D3E4 owners at +28h. Borrow that same header; neither
// routine creates a shadow table. Names describe hypotheses, not symbols.
// B17B70/B17C50: ECX=table, stack pointer-to-slab, RET 4. Equality of count
// and capacity grows to 2*capacity+2 before appending the current source word.
// B17BF0/B17CD0: ECX=table, stack capacity, RET 4. Unsigned reserve only grows.
// Both publish capacity before allocation, copy the current count, free/adopt
// the pointer block, and leave count unchanged until the append completes.
// These source interfaces do not provide the original binary calling ABI.
void append_native_material_pool_slab_00b17b70(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void reserve_native_material_pool_table_00b17bf0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void append_native_material_parameter_pool_slab_00b17c50(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void reserve_native_material_parameter_pool_table_00b17cd0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
} // namespace bsp
