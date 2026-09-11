#include "bsp/main_menu_map_runtime.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include <stdexcept>

namespace bsp {
MainMenuMapGeometryRuntime::MainMenuMapGeometryRuntime(GuiWidgetOwnerRuntime& owners,
    const MissionTreeTables& missions, const MainMenuMissionSelection& selection,
    const MainMenuMapPointLists& points, GuiLayoutWidget*& backdrop,
    GuiLayoutWidget*& selected_point, GuiLayoutWidget*& selector) noexcept
    : owners_(owners), missions_(missions), selection_(selection), points_(points),
      backdrop_(backdrop), selected_point_(selected_point), selector_(selector) {}

void MainMenuMapGeometryRuntime::update(MainMenuMapGeometryState& state,
    const MainMenuMapGeometryInput& input) {
    auto actual_input = input;
    actual_input.has_selected_point_widget = selected_point_ != nullptr;
    update_main_menu_map_geometry_00588c70(state, actual_input, *this);
}

std::uint32_t MainMenuMapGeometryRuntime::selected_mission_index() {
    return selection_.mission_index; // Reload published00E194DC at each read.
}
std::size_t MainMenuMapGeometryRuntime::current_group_mission_count() {
    // Published00E194D8 is independent of the supplied point-list selector.
    return missions_.groups.at(selection_.group_index).missions.size();
}
std::array<float, 3> MainMenuMapGeometryRuntime::point_at(std::size_t list,
    std::uint32_t index) {
    return points_.at(list).at(index);
}
GuiWidgetOwner& MainMenuMapGeometryRuntime::require_owner(GuiLayoutWidget* widget) const {
    if (!widget) throw std::logic_error("Map geometry requires its bound GUI widget.");
    return owners_.owner(*widget);
}
std::uintptr_t MainMenuMapGeometryRuntime::find(std::uintptr_t root, std::string_view name) {
    auto& parent = require_owner(reinterpret_cast<GuiLayoutWidget*>(root)).layout();
    auto* child = find_child_by_name_00aa7e00(parent, name);
    if (child) require_owner(child);
    return reinterpret_cast<std::uintptr_t>(child);
}
std::uintptr_t MainMenuMapGeometryRuntime::find_map_point(std::uintptr_t root,
    std::string_view name, int) {
    return find(root, name); // 00588F9C; unused final native argument ignored.
}
std::uintptr_t MainMenuMapGeometryRuntime::find_map_flag(std::uintptr_t root,
    std::string_view name, int) {
    return find(root, name); // 005890FE, even when the point lookup missed.
}
void MainMenuMapGeometryRuntime::move(GuiLayoutWidget* widget,
    const std::array<float, 3>& position) {
    auto& owner = require_owner(widget);
    // Complete00AA8240 using the existing inverse-parent arithmetic and the
    // retained owner's actual matrix/bounds publications, in native order.
    const auto local = local_position_for_resolved(owner.layout().transform,
        {position[0], position[1], position[2]});
    owner.set_position_00aa7dc0(local);
}
void MainMenuMapGeometryRuntime::move_map_point(std::uintptr_t widget,
    const std::array<float, 3>& position) {
    move(reinterpret_cast<GuiLayoutWidget*>(widget), position);
}
void MainMenuMapGeometryRuntime::move_map_flag(std::uintptr_t widget,
    const std::array<float, 3>& position) {
    move(reinterpret_cast<GuiLayoutWidget*>(widget), position);
}
void MainMenuMapGeometryRuntime::move_backdrop(const std::array<float, 3>& position) {
    move(backdrop_, position);
}
void MainMenuMapGeometryRuntime::resize_backdrop(const std::array<float, 2>& extent) {
    auto& owner = require_owner(backdrop_); // Reload +328h after the move.
    auto* icon = dynamic_cast<GuiIconTypeImplementation*>(&owner.implementation());
    if (!icon) throw std::logic_error("Map backdrop size58 requires the retained Icon implementation.");
    icon->runtime().set_size58_00ab1ef0(owner, {extent[0], extent[1]});
}
std::array<float, 3> MainMenuMapGeometryRuntime::selected_point_position() {
    const auto position = resolved_position(require_owner(selected_point_).layout().transform);
    return {position.x, position.y, position.z};
}
void MainMenuMapGeometryRuntime::move_selection_companion(const std::array<float, 3>& position) {
    move(selector_, position);
}
} // namespace bsp
