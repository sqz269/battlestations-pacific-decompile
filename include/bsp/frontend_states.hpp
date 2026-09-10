#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/app_frame.hpp"

// Front-end branch of GGame::OnMove (004e4a40), covering game states 1, 2 and 4
// read from game+5D4h. Evidence and uncertainties: docs/GAME_FRONTEND_STATES.md.
//
// Reconstructed here: the dispatch at 004e4b9d..004e4d2c, the screen-registry
// pump 004f8830, the screen close helper 004b6e50 and the screen update
// trampoline 004f71f0. Everything reached through FrontEndFrameHost or
// FrontEndScreenHost is a native call site that is not reconstructed.
namespace bsp {

// game+5D4h values that select the front-end branch. 1 and 2 come from
// docs/APP_INIT_GAME_ENTRY.md. 4 is named here from 004e4000, the drain handler
// the request for 4 selects at 004e44f3: it destroys the title-screen singleton
// at 00e198c8, then loads the front-end resource sets whose profiler labels are
// "Textures before mainmenu", "Sounds before mainmenu" and "Effects before
// mainmenu" under the loading label "GILoading::SLM_LOAD_FRONTEND" on a first
// entry or "GILoading::SLM_LOAD_FRONTEND_RETURN" when game+719Ch is already set.
enum class GameFrontEndState : int {
    LogoSequence = 1, // 004e57c1; the 0x80 object at 00e198a4 plays Logos.lua
    TitleScreen = 2, // 004c9a70 GGame::OnInitTitle; the 0x44 object at 00e198c8
    FrontEndShell = 4, // 004e4000; main-menu shell, no per-state update in OnMove
};

// Base of the front-end screen hierarchy. Constructor 004f7180 stores vtable
// 00CEAE54 and clears both bytes; destructor 004f71a0 removes the object from
// the registry. Slot +0 of that vtable is __purecall, and 004f71d0 calls it with
// no arguments and uses EAX as the registry index, so the leaf type supplies its
// own screen id. Only the two flag bytes are recovered.
struct FrontEndScreen {
    bool wanted{false}; // +4h, the requested state
    bool active{false}; // +5h, the state the registry has applied
};

// Fixed screen registry walked by 004f8830. The array runs 00E18B60..00E18CDB
// inclusive, 95 pointer slots; 00E18CDC is a separate byte the pump clears at
// 004f8881 before the first pass and OnMove clears again at 004e544c/004e5481.
inline constexpr int kFrontEndScreenSlotCount = 95;

struct FrontEndScreenTable {
    FrontEndScreen* slots[kFrontEndScreenSlotCount]{}; // 00E18B60
    bool pump_sentinel{false}; // 00E18CDC
};

// Singleton returned by 00425d10: a 0x290 byte object cached at 00E18E6C, built
// by 00533120 through the CRT allocator and registered with the singleton
// lifetime manager at instance+0Ch. 00533120 chains 004f7180, so the object is a
// front-end screen. Its string table (globals.dialog_exitgame,
// globals.dialog_exittomenu, globals.dialog_restartmission, globals.accept,
// globals.yes, globals.no, globals.back) identifies it as the shared menu
// command and confirmation-dialog screen.
struct MenuCommandScreen {
    FrontEndScreen base{}; // +4h, +5h
    bool modal_dialog_active{false}; // +25Ch; gates 004f8830 and the pause gate
    std::int32_t pending_command_a{0}; // +188h; polled by the pause gate only
    std::int32_t pending_command_b{0}; // +218h; polled by the pause gate only
};

// The 0x44 byte title-screen object at 00E198C8. Constructor 0068d760 chains
// 00684e10, stores vtable 00CF7A98 and clears +40h; 0068d7b0 (vtable +8h) fills
// +40h and sets that screen's wanted byte. This is not the screen base type: its
// vtable +4h is 0068d8d0, the routine that leaves the title screen.
struct TitleScreen {
    FrontEndScreen* attached_screen{nullptr}; // +40h
};

// Fields of the game object (00E188A8) that the front-end branch reads or writes.
struct GameFrontEndFrameState {
    bool render_queue_open{false}; // +34h; 1 once the renderer frame is begun
    float scaled_delta{0.0f}; // +21F0h, the time-scaled delta
    bool requests_held{false}; // +5ECh; when set, 004e4430 does not run
};

// Globals the front-end branch consults that are owned by other subsystems.
struct FrontEndWorld {
    FrontEndScreenTable screens{};
    MenuCommandScreen menu{};
    TitleScreen* title{nullptr}; // 00E198C8, null until 004c9a70 builds it
    // *(00F8ABE8)+3E8h. The same byte forces the GUI-enable argument to false at
    // 004c6d96 and makes 004ca327 drop the window close request, so it reads as
    // "front-end GUI suspended". 00F8ABE8 is filled at 00a3f586 in the online and
    // system-manager segment; the exact owner is not established.
    bool front_end_gui_suspended{false};
    std::int32_t local_player_count{0}; // game+1FE4h
    bool mission_context_present{false}; // 00E198C4
};

// One method per native call site the screen registry reaches. Slot is the index
// into FrontEndScreenTable::slots, which is also the native screen id.
struct FrontEndScreenHost {
    virtual ~FrontEndScreenHost() = default;
    virtual void screen_exit(int slot) = 0; // vtable +1Ch
    virtual void screen_enter(int slot) = 0; // vtable +18h
    virtual void screen_update(int slot, float seconds) = 0; // vtable +20h
    virtual void screen_commit(int slot) = 0; // 004f83b0
    // 00425d10 singleton, vtable +20h. Reached from 004f8830 and, wrapped in
    // 004f71f0, from the state 2 branch.
    virtual void menu_command_update(float seconds) = 0;
    virtual void menu_command_exit() = 0; // 00425d10 singleton, vtable +1Ch
    virtual void menu_command_commit() = 0; // 004f83b0 on the same singleton
};

// One method per native call site the front-end dispatch reaches.
struct FrontEndFrameHost {
    virtual ~FrontEndFrameHost() = default;
    virtual int game_state() = 0; // game+5D4h
    // 004c11f0 then 00b1bf90: a nonzero retention control blocks the frame.
    virtual bool render_queue_retained() = 0;
    virtual void renderer_begin_frame() = 0; // 00F8D394 vtable +0Ch
    virtual void update_logo_sequence() = 0; // 00685170, ECX = 00E198A4
    virtual void reinitialize_title_screen() = 0; // 004db220, ECX = game
    virtual void update_title_screen(float raw_delta) = 0; // 0068d850
    virtual void gui_clear_screens(bool flag) = 0; // 004c12b0 then 00aa0e50
    virtual void gui_set_enabled(bool enabled) = 0; // 004c12b0 then 00aa0e00
    virtual void gui_update(float raw_delta, int mode) = 0; // 004c12b0, 00aa4f80
    virtual void game_render() = 0; // 004ca440
    virtual void game_finish_render_frame() = 0; // 004ca1f0
    virtual bool state_requests_pending() = 0; // *(00E188A8)+5E8h
    virtual void drain_state_requests() = 0; // 004e4430
};

// How the front-end branch left GGame::OnMove.
enum class FrontEndFrameOutcome {
    // The branch returned, at 004e4cd7 with no queued requests or at
    // 004e4d1a/004e4d23/004e4d2c after a drain that kept a front-end state.
    FrameComplete,
    // The drain moved to a non front-end state, so the same OnMove call runs on
    // into the timing block at 004e4d32 as the new state.
    ContinueToSimulation,
};

// 004b6e50. __thiscall, ECX = the screen, no stack arguments, RET 0; the body
// ends in a tail jump to 004f83b0. Clears both flag bytes after running the exit
// virtual, so the registry pump will not run the exit path again.
void close_front_end_screen(FrontEndScreen& screen, int slot, FrontEndScreenHost& host);

// The same routine at the only ECX OnMove passes it, the 00425d10 singleton
// (004e4c8f..004e4c96). Split out because that object is addressed directly
// rather than by registry slot.
void close_menu_command_screen(MenuCommandScreen& menu, FrontEndScreenHost& host);

// 004f8830. __stdcall(float), RET 4, no ECX. Three full passes over the registry:
// exit the screens that are no longer wanted, enter the newly wanted ones, then
// update the ones that are both wanted and active. When the menu command screen
// has a modal dialog up, only that screen is updated unless a mission is loaded.
void run_front_end_screen_pump(FrontEndWorld& world, FrontEndScreenHost& host, float raw_delta);

// 004e4b9d..004e4d2c. raw_delta is the OnMove stack float at [esp+48h];
// state.scaled_delta is game+21F0h, which only the menu command update receives.
FrontEndFrameOutcome run_front_end_state_frame(GameFrontEndFrameState& state, FrontEndWorld& world,
    FrontEndFrameHost& host, FrontEndScreenHost& screens, float raw_delta);
}
