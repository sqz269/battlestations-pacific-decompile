#pragma once

#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {
// Address entries for the existing Widget pool engine. The actual F8BC94
// header stays in its canonical process owner on the E188B4 allocator list.
// AA79A0: ECX=pool, EAX=pool, RET; AA7470: ECX=pool, RET. These source
// interfaces discard the constructor result and do not supply the native ABI.
void construct_native_gui_widget_pool_00aa79a0(NativeGuiWidgetPool<GuiWidgetPoolTraits>&);
void destroy_native_gui_widget_pool_00aa7470(NativeGuiWidgetPool<GuiWidgetPoolTraits>&);
} // namespace bsp
