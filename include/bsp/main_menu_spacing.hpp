#pragma once

namespace bsp {
struct GuiLayoutWidget;
class GuiWidgetOwnerRuntime;
struct MainMenuLayoutBindings;

// 00582F30 registration, ONLY the after-current14 fragment 00582F45..00582FEB.
// Native containing function: ECX=main-menu screen, no stack args, bare RET.
// Call after current14 (normally005861B0) completes, before current34(false).
// Publishes the containing layout's existing optional +2B0/+2B4 cells at the
// two native stores. Requires the same authored widgets and actual Text
// companions; an unresolved/pending runtime throws at the reached operation.
// Returns the +470 receiver captured at00582FDD for the caller's subsequent
// current34(false). This fragment neither dispatches it nor completes register.
// New C++ interface, not a binary-compatible replacement of00582F30.
GuiLayoutWidget* compute_main_menu_spacing_00582f45_fragment(
    MainMenuLayoutBindings&, GuiWidgetOwnerRuntime&);
} // namespace bsp
