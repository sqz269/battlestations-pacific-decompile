#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

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

// AB14F0: MOV EAX,[ECX]; TEST/JZ; free(EAX); RET. CB7A33, the Icon
// constructor's state-2 unwind funclet, tail-jumps here with pool+28h.
// AB1FD0 inlines this same release after freeing the real slabs and before
// critical-section/list teardown. Preserve every header field, including
// the stale data pointer. This source interface does not implement ECX ABI.
__declspec(noinline) inline void free_native_gui_icon_pool_table_00ab14f0(
    void* actual_table) noexcept {
    void* data;
    std::memcpy(&data, actual_table, sizeof(data));
    if (data) singleton_lifetime_free(data);
}

template<> void NativeGuiWidgetPool<GuiIconPoolTraits>::destroy();

} // namespace bsp
