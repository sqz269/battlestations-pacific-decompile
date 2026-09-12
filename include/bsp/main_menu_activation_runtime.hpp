#pragma once
#include "bsp/award_grant.hpp"
#include "bsp/game_frame_control.hpp"
#include "bsp/main_menu_command_listener.hpp"
#include "bsp/main_menu_tactical_library.hpp"

namespace bsp {
// Required providers over the SAME current menu/game/session owners. Storage
// accessors only resolve existing associations, with no allocation or callback.
// Address/slot names preserve incompletely reconstructed network contracts.
struct MainMenuActivationServices {
    virtual ~MainMenuActivationServices() = default;
    virtual void call_00597870(std::int32_t page) = 0; // ECX screen, RET4
    virtual void call_005e74b0() = 0; // actual [E198B4]+40; return AL ignored
    virtual void call_005e76d0() = 0; // actual [E198B4]+40; return AL ignored
    // Fresh actual F8A2FC object and its current vtable, no synthesized session.
    virtual bool session_current14_00f8a2fc() = 0; // RET, AL; target unresolved
    virtual void session_current1a0_00f8a2fc(bool) = 0; // RET4; target unresolved
    virtual OnlineSignInState& sign_in_00f8abe8() = 0;
    // Complete callee read: capture actual F8ABE8 for selected/state==2,
    // then fresh F8ABE8 -> A3EB20's byte[118h+selected index]. RET, AL.
    virtual bool call_004bb600() = 0;
    virtual std::uint8_t& session_selector_00e188a8_218c() = 0;
    virtual GameStateRequestQueue& state_requests_00e188a8_5d8() = 0;
};

// All references alias the existing command/selection/tactical bindings.
// There is no alternative screen, selected row, profile or publication slot.
struct MainMenuActivationBindings {
    MainMenuCommandListenerBindings& command;
    MainMenuActivationServices& services;
    MainMenuTacticalLibraryBindings& tactical;
    volatile std::uint32_t& selected_00e194c8;
    volatile std::uint32_t& selected_00e194cc;
    volatile std::uint32_t& selected_00e194d0;
    volatile std::uint32_t& selected_00e194d4;
};

// Complete normal00598B60..00599331 caller sequence. CEFC48+04 at screen+8;
// ECX=screen+8, selected-row and callback-Listbox on stack, RET8. BOTH stack
// operands are unread; selection is fetched from this screen's CURRENT1B8.
// Providers above and existing command services remain required dependencies.
// No original SEH/vtable ABI, complete concrete menu or gameplay claim.
void main_menu_listbox_current04_00598b60(MainMenuActivationBindings&,
    GuiWidgetOwner* selected_row, GuiWidgetOwner& callback_listbox);

// Complete normal helper sequences, using actual sign-in state, session
// provider, prompt/string bodies and the existing game state-request deque.
// 585B40: RET, AL. 585810: CL=check multiplayer privilege, RET, AL.
// 5E6F70: incoming ECX unread, RET, AL=1 after two successful enqueues.
bool main_menu_connection_gate_00585b40(MainMenuActivationBindings&);
bool main_menu_account_gate_00585810(MainMenuActivationBindings&,
    bool check_multiplayer_privilege);
bool main_menu_session_request_005e6f70(MainMenuActivationBindings&);
} // namespace bsp
