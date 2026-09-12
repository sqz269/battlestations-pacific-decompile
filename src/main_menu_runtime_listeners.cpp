#include "bsp/main_menu_runtime_listeners.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
MainMenuCanonicalCommandServices::MainMenuCanonicalCommandServices(
    GuiWidgetOwnerRuntime& owners, MainMenuVehicleUnlockStorage& game,
    NativeStringStorage& strings, MainMenuTacticalLibraryBindings& tactical)
    : owners_(owners), game_(game), strings_(strings), tactical_(tactical) {}
GuiListboxRuntime& MainMenuCanonicalCommandServices::listbox_runtime(GuiWidgetOwner& owner) {
    auto* implementation = dynamic_cast<GuiListboxTypeImplementation*>(&owner.implementation());
    if (&owner.runtime() != &owners_ || !implementation || &implementation->runtime().owner() != &owner)
        throw std::logic_error("Menu command requires the same actual Listbox companion");
    return implementation->runtime();
}
ProfileResetState& MainMenuCanonicalCommandServices::profile_00e188a8_650() {
    return game_.profile_00e188a8_650();
}
const MissionRecordData& MainMenuCanonicalCommandServices::selected_mission_005806a0() {
    auto* manager = tactical_.manager_00e198ac;
    if (!manager || !manager->screens[1] || !tactical_.mission_tables)
        throw std::logic_error("Menu command requires current mission-tree storage");
    const auto& tables = tactical_.mission_tables(manager->screens[1]);
    const auto group = tactical_.selected_group_00e194d8;
    const auto mission = tactical_.selected_mission_00e194dc;
    const auto* record = bsp::selected_mission_005806a0(tables, group, mission);
    if (!record) throw std::logic_error("Menu command selected mission is outside the valid-iterator domain");
    return *record;
}
bool MainMenuCanonicalCommandServices::call_00584750(bool first, std::int32_t vehicle_class) {
    return main_menu_vehicle_class_unlocked_00584750(first, vehicle_class, game_, strings_);
}
void MainMenuCanonicalCommandServices::call_005885d0() {
    request_tactical_library_with_selection_005885d0(tactical_);
}
void MainMenuCanonicalCommandServices::call_005886c0() {
    request_tactical_library_005886c0(tactical_);
}
void MainMenuCanonicalCommandServices::tactical_selection_fields(
    const MissionRecordData& record, std::int32_t side, bool flag) {
    auto* manager = tactical_.manager_00e198ac;
    if (!manager || !manager->screens[6] || !tactical_.library_fields)
        throw std::logic_error("Menu command requires current TacticalLibrary storage");
    void* identity = manager->screens[6];
    auto fields = tactical_.library_fields(identity);
    if (fields.screen_identity != identity)
        throw std::logic_error("Menu command TacticalLibrary field identity differs");
    fields.selected_mission_9c = &record;
    fields.side_index_a0 = static_cast<std::uint32_t>(side);
    fields.flag_a4 = static_cast<std::uint8_t>(flag);
}

class MainMenuRuntimeListeners::Operation {
public:
    explicit Operation(MainMenuRuntimeListeners& owner) : owner_(owner) { ++owner_.active_calls_; }
    ~Operation() { --owner_.active_calls_; }
private:
    MainMenuRuntimeListeners& owner_;
};
MainMenuRuntimeListeners::MainMenuRuntimeListeners(void* screen_plus_40, void* screen_plus_8,
    MainMenuSelectionListenerBindings& selection)
    : MainMenuWidgetListener(screen_plus_40, selection.command.widget),
      GuiListboxSelectionListener(screen_plus_8), selection_(selection) {
    if (screen_plus_40 == screen_plus_8 ||
        &selection.arrow_bottom_enabled_1c4 != &selection.command.widget.arrow_bottom_enabled_1c4 ||
        &selection.arrow_top_enabled_1c5 != &selection.command.widget.arrow_top_enabled_1c5)
        throw std::invalid_argument("Menu listeners require distinct identities and the same screen field aliases");
}
MainMenuRuntimeListeners::~MainMenuRuntimeListeners() noexcept {
    if (frames_ || listboxes_ || active_calls_) std::terminate();
}
void MainMenuRuntimeListeners::bind(GuiWidgetFrameRuntime& frames, GuiListboxListenerDispatch& listboxes) {
    if (frames_ || listboxes_ || active_calls_ ||
        &frames.widgets() != &selection_.command.widget.owners ||
        &listboxes.owners() != &selection_.command.widget.owners)
        throw std::logic_error("Menu listener registration requires its idle canonical owner domain");
    // Listbox rollback permits removing this newly registered, inactive
    // listener even when a different frame callback is active. Frame rollback
    // would reject that domain and could leave an untracked borrowed pointer.
    listboxes.bind_listener(*this);
    try { frames.bind_listener(*this); }
    catch (...) { listboxes.unbind_listener(*this); throw; }
    frames_ = &frames;
    listboxes_ = &listboxes;
}
void MainMenuRuntimeListeners::unbind() {
    if (active_calls_ || !frames_ || !listboxes_)
        throw std::logic_error("Menu listeners must be bound and idle before unbinding");
    // Both callbacks increment active_calls_; dispatchers also enforce their
    // own active-call contracts before erasing either borrowed association.
    frames_->unbind_listener(*this);
    listboxes_->unbind_listener(*this);
    frames_ = nullptr;
    listboxes_ = nullptr;
}
void MainMenuRuntimeListeners::call_current04(GuiWidgetOwner& widget) {
    Operation operation(*this);
    main_menu_listener_current04_005993a0(selection_.command, widget);
}
void MainMenuRuntimeListeners::call_current0c(GuiWidgetOwner& widget) {
    Operation operation(*this);
    MainMenuWidgetListener::call_current0c(widget);
}
void MainMenuRuntimeListeners::call_current18(GuiWidgetOwner& widget, bool inside) {
    Operation operation(*this);
    MainMenuWidgetListener::call_current18(widget, inside);
}
void MainMenuRuntimeListeners::call_current08(GuiWidgetOwner* row, GuiWidgetOwner& listbox) {
    Operation operation(*this);
    main_menu_listener_current08_005966f0(selection_, row, listbox);
}
} // namespace bsp
