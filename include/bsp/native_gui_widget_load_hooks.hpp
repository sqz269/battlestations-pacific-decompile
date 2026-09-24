#pragma once
#include "bsp/native_gui_widget_bounds.hpp"
#include <cstdint>

namespace bsp {
// Exact one-byte RET at A9AC00, base widget virtual+74. Only this target is
// a proven no-op; never substitute it for an unknown derived load hook.
void __fastcall native_gui_widget_base_post_construct_00a9ac00(
    void* widget, void* unused_edx) noexcept;
// Complete13B AA6A30. ECX widget, low byte of stack DWORD, RET4. Copies the
// byte unchanged to actual+85, including noncanonical values such as80.
void __fastcall set_native_gui_widget_active_00aa6a30(
    void* widget, void* unused_edx, std::uint8_t active) noexcept;

class NativeGuiWidgetLoadedDispatch {
public:
    virtual ~NativeGuiWidgetLoadedDispatch() = default;
    // Exact current +60 numeric target and actual receiver. Required for any
    // target other than recovered AA6A30; real returning implementation only.
    virtual void call_active_60(std::uint32_t target, void* widget,
        std::uint8_t active) = 0;
};
// Complete20B AA7170, original ECX widget, tail JMP AA70E0. Resolve current
// widget profile/+60, call with zero, then raw bounds over the SAME widget
// identity and its CURRENT fields after callback return. Existing bounds
// scratch and live-half contracts apply. Explicit source context/scratch ABI;
// no logical scene adapter, unknown-target fallback or application binding.
void native_gui_widget_loaded_00aa7170(void* widget,
    NativeGuiWidgetLoadedDispatch&, GuiWidgetBounds& bounds_scratch,
    const volatile double& actual_half_00d7a280);
} // namespace bsp
