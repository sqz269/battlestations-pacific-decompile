#include "bsp/main_menu_activation_runtime.hpp"
#include "bsp/gui_listbox_runtime.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
struct StringTemporary {
    NativeString value;
    NativeStringStorage& strings;
    bool live{};
    explicit StringTemporary(NativeStringStorage& storage) : strings(storage) {}
    ~StringTemporary() { destroy(); }
    void construct(const char* text) {
        value.assign_0041e870(strings, text);
        live = true;
    }
    void destroy() noexcept {
        if (live) {
            live = false;
            destroy_native_string_header_0041dd20(&value, strings);
        }
    }
};
struct CheckpointTemporary {
    MainMenuCommandCheckpointValue value;
    NativeStringStorage& strings;
    bool live{};
    explicit CheckpointTemporary(NativeStringStorage& storage) : strings(storage) {}
    ~CheckpointTemporary() {
        if (live) destroy_native_string_header_0041dd20(&value.text_0c, strings);
    }
};
std::uint32_t selected_data(MainMenuActivationBindings& b) {
    auto& command = b.command;
    auto* widget = command.widget.layout.main_listbox_1b8;
    require(widget != nullptr, "Main-menu activation requires its actual current Listbox");
    auto& owner = command.widget.owners.owner(*widget);
    auto& listbox = command.services.listbox_runtime(owner);
    require(&listbox.owner() == &owner,
        "Main-menu activation requires the same canonical Listbox companion");
    return listbox.selected_data_d8_00a9c990();
}
MainMenuManager& current_manager(MainMenuActivationBindings& b) {
    auto* manager = b.tactical.manager_00e198ac;
    require(manager != nullptr, "Main-menu activation requires the current menu manager");
    return *manager;
}
MainMenuTacticalLibraryFields current_library(MainMenuActivationBindings& b) {
    auto* identity = current_manager(b).screens[6];
    require(identity && b.tactical.library_fields,
        "Main-menu activation requires the actual published tactical screen");
    auto fields = b.tactical.library_fields(identity);
    require(fields.screen_identity == identity,
        "Main-menu activation tactical fields must belong to that published screen");
    return fields;
}
void request_interface(MainMenuActivationBindings& b, std::uint32_t id) {
    push_interface_request_004cc460(current_manager(b).base, b.tactical.interface_lock,
        id, nullptr, b.tactical.payloads);
}
void request_library_mode(MainMenuActivationBindings& b, std::int32_t mode) {
    request_interface(b, 0x0b);
    auto fields = current_library(b); // reload manager AND screen after payload callbacks
    fields.mode_94 = mode;
    fields.selector_98 = 0x63;
}
void information_prompt(MainMenuActivationBindings& b, const char* text) {
    auto& command = b.command;
    StringTemporary title(command.strings), message(command.strings), countdown(command.strings);
    title.construct("");
    message.construct(text);
    countdown.construct("");
    auto& screen = command.services.prompt_00425d10();
    auto& host = command.services.prompt_host(screen);
    countdown.live = false; //531B00 owns its by-value8h string
    raise_prompt_00531b00(screen, host, 2, message.value, PromptKind::Accept,
        0, 0, title.value, 0.0f, 0, std::move(countdown.value), true);
    message.destroy();
    title.destroy();
    // These helpers have no focus-navigation call.
}
void checkpoint_prompt(MainMenuActivationBindings& b,
    const MainMenuCommandCheckpointValue& checkpoint, bool available, bool unlocked) {
    auto& command = b.command;
    const bool offer_checkpoint = available && unlocked;
    StringTemporary title(command.strings), suffix(command.strings), countdown(command.strings);
    title.construct("");
    suffix.construct(offer_checkpoint ? "globals.checkpoint_available"
        : "|.\n|globals.continue_without_checkpoint");
    countdown.construct("");
    StringTemporary prefixed(command.strings), combined(command.strings);
    const NativeString* message = &suffix.value;
    if (!offer_checkpoint) {
        {
            StringTemporary prefix(command.strings);
            prefix.construct(available ? "globals.checkpoint_unit_locked|.\n|."
                : "globals.checkpoint_unit_notavailable|.\n|.");
            concatenate_native_string_headers_004261a0(&prefix.value, &prefixed.value,
                &checkpoint.text_0c, command.strings); // complete43C130 normal body
            prefixed.live = true;
        }
        concatenate_native_string_headers_004261a0(&prefixed.value, &combined.value,
            &suffix.value, command.strings);
        combined.live = true;
        message = &combined.value;
    }
    auto& screen = command.services.prompt_00425d10();
    auto& host = command.services.prompt_host(screen);
    countdown.live = false;
    raise_prompt_00531b00(screen, host, 5, *message, PromptKind::YesNo,
        offer_checkpoint ? 0x00592ad0u : 0x0058f540u, 0, title.value,
        0.0f, 0, std::move(countdown.value), true);
    combined.destroy();
    prefixed.destroy();
    suffix.destroy();
    title.destroy();
    auto& focus_screen = command.services.prompt_00425d10();
    auto& focus_host = command.services.prompt_host(focus_screen);
    //592CD/D4 is common to ALL three arms, including available+unlocked.
    focus_last_prompt_navigation_00530c20(focus_screen, focus_host);
}
void campaign_activation(MainMenuActivationBindings& b) {
    auto& command = b.command;
    const auto selected = selected_data(b);
    command.selected_00e194dc = selected; //598F8F before598F94
    command.selected_00e08878 = selected;
    const auto page = command.widget.page_00e08874; // reload after selection callback
    command.widget.screen.us_campaign = page == 5 || page == 7; //screen564, not565
    auto* profile = &command.services.profile_00e188a8_650();
    auto* mission = &command.services.selected_mission_005806a0();
    if (!command.services.call_007f8d60(*profile, *mission)) {
        command.mission_flag_5c = 0;
        command.services.call_0058c010();
        return;
    }
    profile = &command.services.profile_00e188a8_650();
    mission = &command.services.selected_mission_005806a0();
    CheckpointTemporary checkpoint(command.strings);
    command.services.call_007fc490(*profile, checkpoint.value, *mission, command.strings);
    checkpoint.live = true;
    profile = &command.services.profile_00e188a8_650();
    mission = &command.services.selected_mission_005806a0();
    const bool available = command.services.call_007fc370(*profile, *mission);
    bool unlocked = false;
    if (available) {
        std::int32_t vehicle_class;
        std::memcpy(&vehicle_class, &checkpoint.value.words[2], sizeof(vehicle_class));
        const bool first = !command.widget.screen.us_campaign;
        unlocked = command.services.call_00584750(first, vehicle_class);
    }
    checkpoint_prompt(b, checkpoint.value, available, unlocked);
}
int selected_sign_in_state(const OnlineSignInState& state) {
    // Native valid-index domain; do not turn malformed identity into a
    // successful or signed-out synthetic default from the older helper.
    require(state.user_index >= 0 && static_cast<std::size_t>(state.user_index) < kAwardSignInSlotCount,
        "Main-menu account gate requires the actual valid selected sign-in slot");
    return selected_slot_state_00a3ead0(state);
}
} // namespace

bool main_menu_connection_gate_00585b40(MainMenuActivationBindings& b) {
    if (b.services.session_current14_00f8a2fc()) return true;
    information_prompt(b, "FE.multi_cablelost_live");
    return false;
}
bool main_menu_account_gate_00585810(MainMenuActivationBindings& b,
    bool check_multiplayer_privilege) {
    const bool live = live_enabled_account_00a3e520(b.services.sign_in_00f8abe8());
    auto& selected = b.services.sign_in_00f8abe8(); // fresh global at58583F
    if (!live) {
        information_prompt(b, selected.user_selected ? "FE_pc.xsm_requiresmembership"
            : "FE_xbox.xsm_requiresprofile");
        return false;
    }
    if (!selected.user_selected || selected_sign_in_state(selected) != 2) {
        information_prompt(b, "FE_pc.xsm_requireslive");
        return false;
    }
    if (!check_multiplayer_privilege || b.services.call_004bb600()) return true;
    information_prompt(b, "FE_pc.xsm_requiresmultiprivilege");
    return false;
}
bool main_menu_session_request_005e6f70(MainMenuActivationBindings& b) {
    b.services.session_current1a0_00f8a2fc(true);
    b.services.session_selector_00e188a8_218c() = 1;
    enqueue_state_request_004d3ed0(b.services.state_requests_00e188a8_5d8(), 6);
    enqueue_state_request_004d3ed0(b.services.state_requests_00e188a8_5d8(), 7);
    return true;
}
void main_menu_listbox_current04_00598b60(MainMenuActivationBindings& b,
    GuiWidgetOwner* selected_row, GuiWidgetOwner& callback_listbox) {
    (void)selected_row;
    (void)callback_listbox;
    switch (b.command.widget.page_00e08874) {
    case 1: b.command.services.call_00588a80(); return;
    case 4: case 5: case 6: case 7: campaign_activation(b); return;
    case 8: {
        const auto selected = selected_data(b);
        b.command.selected_00e194dc = selected;
        b.command.selected_00e08878 = selected;
        b.command.services.call_005922f0();
        return;
    }
    case 12: b.selected_00e194d4 = selected_data(b); return;
    case 2: {
        const auto selected = selected_data(b);
        b.selected_00e194c8 = selected;
        switch (selected) {
        case 4: b.services.call_00597870(8); return;
        case 0: b.services.call_00597870(4); return;
        case 1: b.services.call_00597870(5); return;
        case 2: b.services.call_00597870(6); return;
        case 3: b.services.call_00597870(7); return;
        case 5: (void)main_menu_session_request_005e6f70(b); return;
        default: return;
        }
    }
    case 3: {
        const auto selected = selected_data(b);
        b.selected_00e194cc = selected;
        if (selected == 1) b.services.call_005e74b0();
        else if (selected == 0 && main_menu_connection_gate_00585b40(b)
            && main_menu_account_gate_00585810(b, true)) b.services.call_005e76d0();
        return;
    }
    case 11: {
        const auto selected = selected_data(b);
        b.selected_00e194d0 = selected;
        switch (selected) {
        case 3: request_library_mode(b, 3); return;
        case 1: {
            auto fields = current_library(b); // before interface request
            fields.selected_mission_9c = nullptr;
            fields.side_index_a0 = 2;
            fields.flag_a4 = 0;
            request_library_mode(b, 2);
            return;
        }
        case 4: request_library_mode(b, 0); return;
        case 2: request_library_mode(b, 1); return;
        case 5:
            if (main_menu_connection_gate_00585b40(b)
                && main_menu_account_gate_00585810(b, false)) request_interface(b, 0x0a);
            return;
        case 6: request_interface(b, 8); return;
        case 0: b.command.services.call_005886c0(); return;
        default: return;
        }
    }
    default: return;
    }
}
} // namespace bsp
