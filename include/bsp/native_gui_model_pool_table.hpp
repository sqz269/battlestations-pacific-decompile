#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {

// Source interfaces over the actual F8C09C+28h slab table, not native ABI
// thunks. Evidence: bsp.py show 00ACBE00/00ACBE50/00ACC0D0/00ACC160;
// 00ACC1D0 reserves 20h entries and 00ACC350 appends under the pool lock.
// Those inlined copies publish capacity before allocation, retain partial
// state on failure, and publish count only after the null-tested slot store.
// Native: __thiscall(ECX=table, stack unsigned capacity), RET 4.
void native_gui_model_pool_table_set_capacity_00acbe00(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

// Replaces the former SweepCHost entry for this address; retain one body.
// Native: __fastcall(ECX=table), plain RET.
void native_gui_model_pool_table_grow_00acbe50(NativeGuiPoolSlabTable& table);

// Native: __thiscall(ECX=table, stack const slab-pointer reference), RET 4.
void native_gui_model_pool_table_push_back_00acc0d0(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);

// Native: __thiscall(ECX=table, stack unsigned capacity), RET 4.
void native_gui_model_pool_table_reserve_00acc160(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

} // namespace bsp
