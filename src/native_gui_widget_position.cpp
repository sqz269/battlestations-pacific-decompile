#include "bsp/native_gui_widget_position.hpp"
#include <cstdint>

namespace bsp {
void set_native_gui_widget_local_position_00aa7dc0(void* widget,
    const void* xyz, NativeGuiWidgetTransformScratch& transform_scratch,
    NativeGuiWidgetTransformBindings& transform_bindings,
    GuiWidgetBounds& bounds_scratch, const volatile double& actual_half_00d7a280) {
    const auto* const source = static_cast<const volatile std::uint32_t*>(xyz);
    const std::uint32_t x = source[0];
    const std::uint32_t y = source[1];
    const std::uint32_t z = source[2];
    auto* const destination = reinterpret_cast<volatile std::uint32_t*>(
        static_cast<std::byte*>(widget) + 0x0c);
    destination[0] = x;
    destination[1] = y;
    destination[2] = z;
    recompose_native_gui_widget_transform_00aa7220(widget,
        transform_scratch, transform_bindings);
    refresh_native_gui_widget_local_bounds_00aa70e0(widget,
        bounds_scratch, actual_half_00d7a280);
}
} // namespace bsp
