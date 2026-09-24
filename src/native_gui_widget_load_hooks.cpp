#include "bsp/native_gui_widget_load_hooks.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI load hooks require MSVC Win32.
#endif
namespace bsp {
__declspec(naked) void __fastcall native_gui_widget_base_post_construct_00a9ac00(
    void*, void*) noexcept {
    __asm { ret }
}
__declspec(naked) void __fastcall set_native_gui_widget_active_00aa6a30(
    void*, void*, std::uint8_t) noexcept {
    __asm {
        mov al,byte ptr [esp+4]
        mov byte ptr [ecx+85h],al
        ret 4
    }
}
void native_gui_widget_loaded_00aa7170(void* widget,
    NativeGuiWidgetLoadedDispatch& dispatch, GuiWidgetBounds& scratch,
    const volatile double& half) {
    const auto profile = *static_cast<const volatile std::uint32_t*>(widget);
    const auto target = *reinterpret_cast<const volatile std::uint32_t*>(profile + 0x60u);
    if (target == 0x00aa6a30u)
        set_native_gui_widget_active_00aa6a30(widget, nullptr, 0);
    else dispatch.call_active_60(target, widget, 0);
    refresh_native_gui_widget_local_bounds_00aa70e0(widget, scratch, half);
}
} // namespace bsp
