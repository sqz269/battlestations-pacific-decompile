#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"
namespace bsp {

// Source interfaces borrowing the AnimIcon pool's retained F8C1D4+28h
// {data,count,capacity} header. No second table or allocator domain is owned.
// Evidence: bsp.py show AD2F20/AD2F50/AD2FA0/AD3330/AD33C0 and AD36A0/AD3820.
// The latter two inline the reserve/append algorithms under their existing
// unwind state/critical section. Reuse those recovered entries here.
//
// Original AD2F20/AD2FA0: __fastcall(ECX=header), RET; AD2F50/AD33C0:
// __thiscall(ECX=header, stack DWORD), RET 4; AD3330: __thiscall(ECX=header,
// stack pointer-to-slab-pointer), RET 4. These C++ APIs do not preserve that ABI.
void free_native_gui_anim_icon_pool_table_00ad2f20(void* actual_header) noexcept;
void native_gui_anim_icon_pool_table_set_capacity_00ad2f50(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);
void native_gui_anim_icon_pool_table_grow_00ad2fa0(NativeGuiPoolSlabTable& table);
void native_gui_anim_icon_pool_table_push_back_00ad3330(
    NativeGuiPoolSlabTable& table, std::byte* const& slab);
void native_gui_anim_icon_pool_table_reserve_00ad33c0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity);

// AD36A0 originally takes ECX=pool and returns that pool in EAX; AD31F0
// takes ECX=pool. Both use plain RET. The explicit domain parameter borrows
// the existing process allocator-list owner instead of creating another one.
// These entries expose the existing generic pool reconstruction once; the
// NativeGuiWidgetPool methods only forward to them.
NativeGuiWidgetPoolStorage* initialize_native_gui_anim_icon_pool_00ad36a0(
    AllocatorListDomain& list, NativeGuiWidgetPoolStorage& storage);
void destroy_native_gui_anim_icon_pool_00ad31f0(
    AllocatorListDomain& list, NativeGuiWidgetPoolStorage& storage);

} // namespace bsp
