#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
// AA745C uses the slot38 target read from the EARLIER captured vtable and
// the CURRENT widget+4C receiver. The binding must implement that exact target
// on actual storage; no logical owner, fallback target or no-op is supplied.
class NativeGuiWidgetTransformDispatch {
public:
    virtual ~NativeGuiWidgetTransformDispatch() = default;
    virtual void call_virtual38(std::uint32_t actual_target,
        void* actual_receiver,const void* actual_matrix) = 0;
};
struct NativeGuiWidgetTransformBindings {
    const volatile float& one_00d7a24c;
    const volatile float& negative_zero_00d7a208;
    const volatile float& y_scale_00e12fc4;
    NativeGuiWidgetTransformDispatch& dispatch;
};
// Exact1DCh-byte local region, beginning at native ESP+8 after ESI/EDI saves.
// Initialized caller-owned preimage with a stable address; bytes C..F,
// D0..D3 and D8..DB remain unwritten. Seven40h matrices occupy this region.
struct alignas(4) NativeGuiWidgetTransformScratch { std::byte bytes[0x1dc]; };
// Complete AA7220[583] normal body; original ECX actual widget, RET.
// Reads actual widget+0C..2C,48,4C; captures *node before matrix providers,
// reloads widget+4C afterward, then reads captured-vtable+38. Preserves native
// x87/SSE arithmetic, raw matrix aliases and borrowed-global read order.
// New C++ scratch/bindings ABI; no null validation, exception translation,
// rollback, logical GuiWidgetOwner, application installation or game claim.
void recompose_native_gui_widget_transform_00aa7220(void* actual_widget,
    NativeGuiWidgetTransformScratch&,NativeGuiWidgetTransformBindings&);
} // namespace bsp
