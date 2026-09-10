#include "bsp/native_renderer_parameters.hpp"
#include "bsp/d3d9_states.hpp"
#include <stdexcept>
#include <type_traits>

namespace bsp {
static_assert(std::is_standard_layout_v<NativeRendererParametersOwner>);
static_assert(sizeof(NativeRendererParametersOwner) == 0x14);
static_assert(alignof(NativeRendererParametersOwner) == 4);
static_assert(offsetof(NativeRendererParametersOwner, flag_00) == 0x00);
static_assert(offsetof(NativeRendererParametersOwner, preserved_01) == 0x01);
static_assert(offsetof(NativeRendererParametersOwner, value_04) == 0x04);
static_assert(offsetof(NativeRendererParametersOwner, flag_08) == 0x08);
static_assert(offsetof(NativeRendererParametersOwner, preserved_09) == 0x09);
static_assert(offsetof(NativeRendererParametersOwner, width_0c) == 0x0c);
static_assert(offsetof(NativeRendererParametersOwner, height_10) == 0x10);

void initialize_native_renderer_parameters_00b32410_fragment(
    NativeRendererParametersOwner& owner) noexcept {
    owner.flag_00 = 0;   // 00B32512
    owner.value_04 = 3;  // 00B32518
    owner.flag_08 = 0;   // 00B32522
    owner.width_0c = 0;  // 00B32528
    owner.height_10 = 0; // 00B3252E
}

NativeViewportRendererParameters NativeD3D9RendererParameterDispatch::parameters_00b1ff60(
    D3D9StateCache& captured_renderer) {
    if (&captured_renderer != &renderer_)
        throw std::logic_error("native renderer parameter binding belongs to another renderer");
    return {owner_.width_0c, owner_.height_10};
}

NativeViewportRendererParameters D3D9ViewportRendererAccess::parameters_00b1ff60(
    D3D9StateCache& captured_renderer) {
    auto* const dispatch = captured_renderer.native_renderer_parameters_dispatch();
    if (!dispatch)
        throw std::logic_error("native renderer parameter dispatch is unbound");
    return dispatch->parameters_00b1ff60(captured_renderer);
}
} // namespace bsp
