#pragma once
#include "bsp/gui_listbox_listener_binding.hpp"
#include "bsp/main_menu_selection_listener.hpp"
#include "bsp/main_menu_tactical_library.hpp"
#include "bsp/main_menu_vehicle_unlock.hpp"
#include "bsp/main_menu_activation_runtime.hpp"

namespace bsp {
// C8/CC alias the selection listener; D0/D4 borrow their existing globals.
MainMenuActivationBindings make_main_menu_activation_bindings(
    MainMenuSelectionListenerBindings&, MainMenuActivationServices&,
    MainMenuTacticalLibraryBindings&, volatile std::uint32_t& selected_00e194d0,
    volatile std::uint32_t& selected_00e194d4);
// Actual implementations of established command services. The remaining
// page/profile/checkpoint/transform/prompt methods stay abstract until their
// real providers are supplied; this class never supplies successful defaults.
// All storage associations must describe the same current game/menu globals.
class MainMenuCanonicalCommandServices : public MainMenuCommandServices {
public:
    MainMenuCanonicalCommandServices(GuiWidgetOwnerRuntime&, MainMenuVehicleUnlockStorage&,
        NativeStringStorage&, MainMenuTacticalLibraryBindings&);
    GuiListboxRuntime& listbox_runtime(GuiWidgetOwner&) override;
    ProfileResetState& profile_00e188a8_650() override;
    const MissionRecordData& selected_mission_005806a0() override;
    bool call_00584750(bool first_argument, std::int32_t vehicle_class) override;
    void call_005885d0() override;
    void call_005886c0() override;
    void tactical_selection_fields(const MissionRecordData&, std::int32_t side, bool flag) override;
    // Late binding closes the screen/service construction cycle. The binding
    // must outlive this association and cannot be replaced during a callback.
    void bind_activation(MainMenuActivationBindings&);
    void unbind_activation(MainMenuActivationBindings&);
    void call_00598b60(GuiWidgetOwner*, GuiWidgetOwner&) override;
private:
    GuiWidgetOwnerRuntime& owners_;
    MainMenuVehicleUnlockStorage& game_;
    NativeStringStorage& strings_;
    MainMenuTacticalLibraryBindings& tactical_;
    MainMenuActivationBindings* activation_{};
    std::uint32_t active_activation_calls_{};
};

// Two native subobjects, one canonical screen binding: CEFC04 at+40 supplies
// widget04/0C/18; CEFC48 at+8 supplies Listbox04/08/0C. No duplicate screen state.
// Register these borrowed interfaces in the existing frame/Listbox dispatchers
// before the layout binder publishes their respective actual identities.
class MainMenuRuntimeListeners final : public MainMenuWidgetListener,
    public GuiListboxFrameListener {
public:
    MainMenuRuntimeListeners(void* screen_plus_40, void* screen_plus_8,
        MainMenuSelectionListenerBindings&);
    ~MainMenuRuntimeListeners() noexcept override;
    void bind(GuiWidgetFrameRuntime&, GuiListboxListenerDispatch&);
    void unbind();
    void call_current04(GuiWidgetOwner&) override;
    void call_current0c(GuiWidgetOwner&) override;
    void call_current18(GuiWidgetOwner&, bool) override;
    void call_current08(GuiWidgetOwner* selected_row, GuiWidgetOwner& listbox) override;
    void call_current04(GuiWidgetOwner& selected_row, GuiWidgetOwner& listbox) override;
    void call_current0c(bool first, bool second, GuiWidgetOwner& listbox) override;
private:
    class Operation;
    MainMenuSelectionListenerBindings& selection_;
    GuiWidgetFrameRuntime* frames_{};
    GuiListboxListenerDispatch* listboxes_{};
    std::uint32_t active_calls_{};
};
} // namespace bsp
