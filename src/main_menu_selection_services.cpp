#include "bsp/main_menu_selection_services.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include <stdexcept>

namespace bsp {
MainMenuObjectiveBindings make_main_menu_objective_bindings(
    MainMenuSelectionListenerBindings& selection, MainMenuObjectiveProviders& providers) {
    auto& command = selection.command;
    auto& layout = command.widget.layout;
    return {command.widget.owners, command.strings, selection.compare_names_00bf7fbf,
        providers, selection.field_64, selection.field_68, selection.field_6c,
        layout.objective_page_2c8, layout.objective_groups_2cc,
        layout.objective_companions_2dc, layout.objective_background_2ec};
}
MainMenuCanonicalSelectionServices::MainMenuCanonicalSelectionServices(
    MainMenuCommandListenerBindings& command,
    MainMenuMedalServices& scores, MainMenuDateBindings& date,
    const GuiWidgetRelativeBoundsConstants& relative, const volatile double& extra_height)
    : command_(command), scores_(scores), date_(date),
      relative_(relative), extra_height_(extra_height) {}
GuiWidgetOwner& MainMenuCanonicalSelectionServices::require_owner(GuiWidgetOwner& owner) {
    if (&owner.runtime() != &command_.widget.owners)
        throw std::logic_error("Menu selection provider requires the same canonical widget owner domain");
    return owner;
}
void MainMenuCanonicalSelectionServices::call_00ab2690(GuiWidgetOwner& owner,
    std::uint32_t state, void* texture, const GuiUvRect& uv) {
    auto* icon = dynamic_cast<GuiIconTypeImplementation*>(&require_owner(owner).implementation());
    if (!icon) throw std::logic_error("Menu texture replacement requires its actual Icon companion");
    icon->runtime().set_state_texture_00ab2690(state, texture, uv);
}
void MainMenuCanonicalSelectionServices::call_00ab27a0(GuiWidgetOwner& owner,
    GuiWidgetSize& output, std::uint32_t state) {
    auto* icon = dynamic_cast<GuiIconTypeImplementation*>(&require_owner(owner).implementation());
    if (!icon) throw std::logic_error("Menu texture dimensions require its actual Icon companion");
    icon->runtime().state_texture_size_00ab27a0(output, state);
}
void MainMenuCanonicalSelectionServices::call_00ac0820(
    GuiWidgetOwner& source, GuiWidgetOwner& destination, float width) {
    require_owner(source); require_owner(destination);
    fit_gui_widget_to_source_00ac0820(source, destination, width, relative_);
}
void MainMenuCanonicalSelectionServices::call_0043bc30(NativeString& output,
    std::uint32_t year, std::uint32_t month, std::uint32_t day, NativeStringStorage& storage) {
    build_main_menu_date_0043bc30(output, year, month, day, date_, storage);
}
void MainMenuCanonicalSelectionServices::call_00580820(GuiWidgetOwner& selected) {
    position_main_menu_highlight_00580820(require_owner(selected), command_.widget.owners,
        command_.widget.layout.highlight_background_294, relative_, extra_height_);
}
void MainMenuCanonicalSelectionServices::call_00594b60() {
    MainMenuMedalBindings bindings{command_.widget, command_.services, scores_};
    update_main_menu_medal_00594b60(bindings);
}
void MainMenuCanonicalSelectionServices::call_00b6da70(NativeNodeBinding& node, float factor, bool recurse) {
    command_.widget.owners.set_node_visibility_factor_00b6da70(node, factor, recurse);
}
void MainMenuCanonicalSelectionServices::framebox_current58(GuiWidgetOwner& owner,
    const GuiWidgetSize& size) {
    auto* frame = dynamic_cast<GuiFrameBoxTypeImplementation*>(&require_owner(owner).implementation());
    if (!frame) throw std::logic_error("Menu frame sizing requires its actual FrameBox companion");
    frame->set_size58(owner, size);
}
GuiWidgetOwner& MainMenuCanonicalSelectionServices::call_00519dc0(
    std::uint32_t offset, std::uint32_t index) {
    auto& layout = command_.widget.layout;
    auto* vector = offset == 0x2cc ? &layout.objective_groups_2cc
        : offset == 0x2dc ? &layout.objective_companions_2dc : nullptr;
    if (!vector) throw std::logic_error("Menu objective selection requires actual2CC/2DC vector");
    auto* widget = objective_vector_element_00519dc0(*vector, index);
    if (!widget) throw std::logic_error("Menu objective selection dereferences a null native payload");
    return command_.widget.owners.owner(*widget);
}
} // namespace bsp
