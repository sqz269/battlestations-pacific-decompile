#pragma once

#include "bsp/native_gui_widget_resolved_position.hpp"
#include "bsp/native_gui_widget_bounds.hpp"
#include "bsp/native_gui_widget_transform.hpp"

namespace bsp {
namespace game { class GameNativeGuiTextState; }
class NativeGuiFrameBoxConstructedTransformDispatch;

// Borrowed Source metadata, not an Original widget/owner or native layout.
// Require the SAME initialized authenticated data service, retained process
// GUI state and concrete mapped B6DB10 dispatch/profiles used by the caller's
// actual GUI graph. No new process owner, scalar copy, callback or default is
// supplied. All of them must outlive this binding, its copies and every call.
class NativeGuiWidgetResolvedSetterBindings final {
public:
    NativeGuiWidgetResolvedSetterBindings(
        const game::GameNativeReadOnlyData& actual_data,
        game::GameNativeGuiTextState& actual_gui_state,
        NativeGuiFrameBoxConstructedTransformDispatch& actual_dispatch);
private:
    NativeGuiWidgetResolvedPositionData resolved_;
    NativeGuiWidgetTransformBindings transform_;
    const volatile double* const actual_d7a258_;
    const volatile double* const actual_half_00d7a280_;
    friend void set_native_gui_widget_resolved_position_00aa8240(
        void*, const void*, NativeGuiWidgetResolvedSetterBindings&,
        NativeGuiWidgetTransformScratch&, GuiWidgetBounds&);
};

// Complete AA8240 normal parent. Original ECX raw widget, stacked XYZ pointer,
// RET4. This new C++ interface borrows explicit metadata and actual callee
// scratch; it is not the Original/private ABI or exception transport.
// Actual +70 parent chain/storage and three input words must be readable;
// +0C/+10/+14 writable. Preserve input/widget aliases. Explicit scratch,
// binding metadata and private frames must be stable, mutually disjoint and
// disjoint from the input/widget/transitive provider graph. Retain that graph
// under the real callee contracts and AA7220 scratch's initialized preimage.
// Each AA7220 reached captured target must be genuine B6DB10; its CURRENT
// receiver/reached Node/Model/Group notification graph must satisfy the held
// mapped-profile contract. AA70E0 tests CURRENT +74 after transform returns;
// if enabled its CURRENT model/geometry/element0 storage must be valid then.
// No semantic widget, forced target/gate, fallback or null/cycle guard.
// Ambient x87/SSE instruction/spill ordering is retained; native ABI/liveouts,
// physical private-stack aliases, C++/SEH/unmasked-FP/fault transport and actual
// Game/pause adoption are unproven. A throwing provider is not a proven unwind
// path through the private naked helper. No catch/rollback is introduced.
void set_native_gui_widget_resolved_position_00aa8240(
    void* actual_widget, const void* actual_xyz,
    NativeGuiWidgetResolvedSetterBindings&,
    NativeGuiWidgetTransformScratch&, GuiWidgetBounds&);
} // namespace bsp
