#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {

// 00AC7F10: original ECX is the retained Scrollbar pool F8C004+28h table,
// plain RET. Releases only its current data pointer without clearing the
// pointer/count/capacity fields. AC7FE0 inlines this after freeing the slabs.
// Evidence: bsp.py show AC7F10/AC7FE0. Source interface, not a register-ABI thunk.
void free_native_gui_scrollbar_pool_table_00ac7f10(void* actual_table) noexcept;

template<> void NativeGuiWidgetPool<GuiScrollbarPoolTraits>::destroy();

} // namespace bsp
