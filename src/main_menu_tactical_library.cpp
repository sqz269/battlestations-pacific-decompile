#include "bsp/main_menu_tactical_library.hpp"
#include <stdexcept>

namespace bsp {
namespace {
MainMenuManager& current_manager(MainMenuTacticalLibraryBindings& b) {
    auto* manager = b.manager_00e198ac;
    if (!manager) throw std::logic_error("Tactical-library request requires the current main-menu manager");
    return *manager;
}
const MissionRecordData& selected_mission(MainMenuTacticalLibraryBindings& b) {
    auto* const identity = current_manager(b).screens[1];
    if (!identity || !b.mission_tables)
        throw std::logic_error("Tactical-library selection requires the actual current mission-tree tables");
    const auto& tables = b.mission_tables(identity);
    const auto group = b.selected_group_00e194d8;
    const auto mission = b.selected_mission_00e194dc;
    const auto* record = selected_mission_005806a0(tables, group, mission);
    if (!record) throw std::logic_error("Tactical-library selection is outside the native valid-iterator domain");
    return *record;
}
MainMenuTacticalLibraryFields current_library(MainMenuTacticalLibraryBindings& b) {
    auto* const identity = current_manager(b).screens[6];
    if (!identity || !b.library_fields)
        throw std::logic_error("Tactical-library request requires the actual current library screen");
    auto fields = b.library_fields(identity);
    if (fields.screen_identity != identity)
        throw std::logic_error("Tactical-library fields must belong to the same published screen");
    return fields;
}
}

void request_tactical_library_with_selection_005885d0(MainMenuTacticalLibraryBindings& b) {
    const auto side = static_cast<std::uint32_t>(
        mission_side_index_005c27e0(selected_mission(b))); //5885D4/DB
    const auto* record = &selected_mission(b); //5885E4, fresh actual globals/table
    auto selection = current_library(b); //5885E9/EF
    selection.selected_mission_9c = record;
    selection.side_index_a0 = side;
    selection.flag_a4 = 0;
    auto mode = current_library(b); //588605/0B reloads both pointers
    mode.mode_94 = 5;
    mode.selector_98 = 0x63;
    push_interface_request_004cc460(current_manager(b).base, b.interface_lock,
        0x0b, nullptr, b.payloads); //58862C, current manager after field stores
}

void request_tactical_library_005886c0(MainMenuTacticalLibraryBindings& b) {
    push_interface_request_004cc460(current_manager(b).base, b.interface_lock,
        0x0b, nullptr, b.payloads); //5886CA, before ANY library field access
    auto mode = current_library(b); //5886CF/D4, reload after payload callbacks
    mode.mode_94 = 4;
    mode.selector_98 = 0x63;
    // No selection or flag store: retain the current screen's existing values.
}
} // namespace bsp
