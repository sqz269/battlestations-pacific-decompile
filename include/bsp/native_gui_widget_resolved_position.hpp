#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw resolved widget position requires MSVC Win32.
#endif

namespace bsp {
namespace game { class GameNativeReadOnlyData; }

// Binding metadata only, not a widget or another data owner. Construction
// resolves the actual D7A258 double cell through authenticated data_at; it may
// throw before a binding exists. There is no default/literal-cell provider.
// Retain the SAME initialized data owner/mapping through every binding copy
// and call. Copying this metadata does not acquire or extend that lifetime.
class NativeGuiWidgetResolvedPositionData final {
public:
    explicit NativeGuiWidgetResolvedPositionData(
        const game::GameNativeReadOnlyData& actual_data);
private:
    const volatile double* const actual_d7a258_;
    friend void* get_native_gui_widget_resolved_position_00aa6750(
        void*, void*, const NativeGuiWidgetResolvedPositionData&);
};

// Whole AA6750..AA67E1:146B/47ops. Original ECX=actual widget, stacked xyz
// output, EAX=same output, RET4. This safe C++ interface adds the genuine data
// binding; an internal Source helper carries its cell in otherwise-unused EDX.
// It retains the original field reloads, recursive spills, ambient x87 operand
// directions and output order. No-parent reads/stores X, then Y, then Z rather
// than capturing three DWORDs. Parent sums all spill before the first output.
//
// All reached widget/parent fields must be readable, the actual12-byte output
// writable, and recursion must terminate for normal return. Payload aliases
// follow the native ordered accesses; stable binding metadata and its mapping
// are disjoint from payload/private scratch. No validation, cycle guard, logical
// widget, callback, mode reset or cleanup is added to the getter. No FP occurs
// outside its raw helper. Existing FP stack contents remain subject to native
// stack capacity/exception behavior; no empty-stack/default-PC assumption.
//
// Source/native argument/private-frame identity, original EDX/flags live-outs,
// unmasked-FP/SEH/hardware-fault transport, binary ABI and caller/game/lifetime
// adoption remain separate. This entry neither constructs nor owns a widget.
void* get_native_gui_widget_resolved_position_00aa6750(void* actual_widget,
    void* actual_output_xyz, const NativeGuiWidgetResolvedPositionData&);
} // namespace bsp
