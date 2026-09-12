#pragma once
#include "bsp/main_menu_selection_listener.hpp"
#include "bsp/main_menu_profile_display.hpp"
#include "bsp/gui_widget_relative_bounds.hpp"
#include "bsp/main_menu_objective_runtime.hpp"

namespace bsp {
// Bind58F5B0's producer to5966F0's existing counts and its same layout slots.
// This factory creates references only; it owns no separate screen state.
MainMenuObjectiveBindings make_main_menu_objective_bindings(
    MainMenuSelectionListenerBindings&, MainMenuObjectiveProviders&);
// Concrete recovered operations over the same screen and resource owners.
// Mutable mission-tree owner, command-bar owner/host/environment and mission
// picture ownership remain abstract until their real producers bind
// them. This class does not substitute another menu, profile or GUI tree.
class MainMenuCanonicalSelectionServices : public MainMenuSelectionServices {
public:
    MainMenuCanonicalSelectionServices(MainMenuCommandListenerBindings&,
        MainMenuMedalServices&, MainMenuDateBindings&,
        const GuiWidgetRelativeBoundsConstants&, const volatile double& extra_height_00cee4e8);
    void call_00ab2690(GuiWidgetOwner&, std::uint32_t, void*, const GuiUvRect&) override;
    void call_00ab27a0(GuiWidgetOwner&, GuiWidgetSize&, std::uint32_t) override;
    void call_00ac0820(GuiWidgetOwner&, GuiWidgetOwner&, float) override;
    void call_0043bc30(NativeString&, std::uint32_t, std::uint32_t, std::uint32_t,
        NativeStringStorage&) override;
    void call_00580820(GuiWidgetOwner&) override;
    void call_00594b60() override;
    void call_00b6da70(NativeNodeBinding&, float, bool) override;
    void framebox_current58(GuiWidgetOwner&, const GuiWidgetSize&) override;
    GuiWidgetOwner& call_00519dc0(std::uint32_t screen_vector_offset,
        std::uint32_t index) override;
private:
    MainMenuCommandListenerBindings& command_;
    MainMenuMedalServices& scores_;
    MainMenuDateBindings& date_;
    GuiWidgetRelativeBoundsConstants relative_;
    const volatile double& extra_height_;
    GuiWidgetOwner& require_owner(GuiWidgetOwner&);
};
} // namespace bsp
