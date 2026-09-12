#pragma once
#include "bsp/gui_widget_listener_binding.hpp"
#include "bsp/main_menu_layout_binding.hpp"
#include "bsp/main_menu_screen.hpp"

namespace bsp {
// References to one existing main-menu screen's storage, including its actual
// layout/scrollers and +56C input gate. No new screen state or widget tree.
// Native callback ECX is screen+40, so native [ECX+offset] maps to offset+40.
struct MainMenuWidgetListenerBindings {
    GuiWidgetOwnerRuntime& owners;
    MainMenuLayoutBindings& layout;
    MainMenuScreenState& screen;
    float& zoom_delta_19c;
    const std::uint8_t& arrow_top_enabled_1c5;
    const std::uint8_t& arrow_bottom_enabled_1c4;
    const volatile std::int32_t& page_00e08874;
    const volatile float& one_00d7a24c;
    const volatile float& dim_alpha_00ce3800;
};

// Full normal current0C/current18 bodies over the canonical supported widget
// profiles. Names identify slots, not inferred mouse-button meanings. Original
// ABI: ECX screen+40, widget stack (+ Boolean for18), RET4/RET8 respectively.
// The caller keeps the screen, bindings, widget and its resources alive through
// callbacks. Original string-pointer/SEH ABI is not reproduced. Empty canonical
// Text has byte zero, matching the installed E19508 sentinel; compare LOW BYTE
// of the current UTF-16 code unit, not the full character or cached source text.
void main_menu_listener_current0c_00581970(
    MainMenuWidgetListenerBindings&, GuiWidgetOwner&);
void main_menu_listener_current18_00581b20(
    MainMenuWidgetListenerBindings&, GuiWidgetOwner&, bool inside);

// CEFC04 at main-menu+40 (59033A). The recovered handler pair and inherited
// empty10/14 are usable by the same listener owner. Current04=5993A0 remains
// REQUIRED: its menu/listbox/prompt command chain is not supplied by a base
// no-op. A concrete screen must implement it before this adapter can be bound.
class MainMenuWidgetListener : public GuiBaseWidgetListener {
public:
    MainMenuWidgetListener(void* screen_plus_40, MainMenuWidgetListenerBindings);
    void call_current04(GuiWidgetOwner&) override = 0;
    void call_current0c(GuiWidgetOwner&) override;
    void call_current18(GuiWidgetOwner&, bool inside) override;
private:
    MainMenuWidgetListenerBindings bindings_;
};
} // namespace bsp
