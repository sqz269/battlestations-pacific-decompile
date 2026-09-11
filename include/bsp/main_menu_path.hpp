#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "bsp/frontend_screen_sets.hpp"
#include "bsp/frontend_state_machine.hpp"
#include "bsp/frontend_states.hpp"

// Packet cc_main_menu_path. The ordered path from the press-start page to a
// visible main menu, expressed as one sequence routine over an injected host so
// the executable's wiring is mechanical.
//
// Every address below was read from the listing; descriptive names are
// hypotheses, not recovered symbols. Evidence and uncertainty:
// docs/MAIN_MENU_PATH.md.
//
// This header deliberately does not restate work that already exists:
//   - the level setters, the recompute 004F7620, the commit 004F83B0 and the
//     interface-id maps: bsp/frontend_screen_sets.hpp
//   - the manager base, 004CC460, 00684600, 00684700, 00686380:
//     bsp/frontend_managers.hpp
//   - the shell entry 004E4000 and the game-state constants:
//     bsp/frontend_entry.hpp
//   - the press-start body 0067CFB0 and its input action:
//     bsp/press_start_screen.hpp
//   - the 004C43C0 edge rule: bsp::action_pressed_this_frame_004c43c0 in
//     bsp/input_tick.hpp. It is referenced here, not redefined.
//   - the pump 004F8830 and the screen phases: bsp/frontend_state_machine.hpp
namespace bsp {

// ---------------------------------------------------------------------------
// Who drives each screen-set level
// ---------------------------------------------------------------------------

// bsp/frontend_screen_sets.hpp records the five level setters and the shared
// body. It does not record which subsystem calls which level; that was left as
// an open follow-up (`front_end_screen_levels`). The xref sweep settles it.
enum class FrontEndScreenSetLevelRole {
    // Level 1. Every caller but the teardown is 0068ACA0
    // BSP_InGameInterface_ApplyPendingInterface, whose jump table at 0068B390
    // publishes a multi-element HUD set per in-mission interface id.
    InGameInterface,
    // Level 2. 0068AB80 CollapseOverlays, 0068AA90, 0068AC30, 0068B470.
    InGameOverlay,
    // Level 3. The widest caller set, including 0068C1F0
    // BSP_InGameInterface_Update, 005ED5D0, 005F98B0, 005FB080 and 0064DD30.
    InGameTransient,
    // Level 4. The three front-end managers' +10h overrides plus 00683AA0
    // Deactivate. This is the only level the main-menu path touches.
    FrontEndManager,
    // Level 5. No reference of any kind exists to 004F87B0 in the image, so
    // level 5 is never populated and the recompute's first descent step is
    // always over an empty vector.
    Unused,
};

struct FrontEndScreenSetLevelOwner {
    int level;                          // 1..5
    std::uint32_t screen_setter;        // 004F8530..004F87B0
    std::uint32_t input_context_setter; // 004D8A50..004D8C00, or 0 when none
    FrontEndScreenSetLevelRole role;
    std::string_view evidence;
};

// The input-context stack has four levels, not five: there is no level-5
// context setter to pair with 004F87B0.
inline constexpr std::array<FrontEndScreenSetLevelOwner, 5> kFrontEndScreenSetLevelOwners{{
    {1, 0x004F8530u, 0x004D8A50u, FrontEndScreenSetLevelRole::InGameInterface,
     "18 call sites: 004DA9B8 in the teardown and 17 in 0068ACA0, e.g. 0068AFD0 "
     "publishes {29h, 49h, 44h, 27h, 4Dh, 3Eh, 25h, 26h}"},
    {2, 0x004F85D0u, 0x004D8AE0u, FrontEndScreenSetLevelRole::InGameOverlay,
     "5 call sites: 004DA9BE, 0068ABFC, 0068AA9E, 0068AC79, 0068B4E9; every one "
     "of the four outside the teardown pushes only the terminator"},
    {3, 0x004F8670u, 0x004D8B70u, FrontEndScreenSetLevelRole::InGameTransient,
     "17 call sites: 004DA9C4, 0068ABB8, 0068B445, 0068C12C, 0068C185, five in "
     "BSP_InGameInterface_Update, and four outside the interface manager"},
    {4, 0x004F8710u, 0x004D8C00u, FrontEndScreenSetLevelRole::FrontEndManager,
     "5 call sites: 004DA9CA in the teardown and the four manager routines "
     "00683AA0, 00685820, 00687800, 00689820"},
    {5, 0x004F87B0u, 0x00000000u, FrontEndScreenSetLevelRole::Unused,
     "Ghidra reports no references to 004F87B0; the teardown 004DA780 clears "
     "levels 1..4 and skips it"},
}};

const FrontEndScreenSetLevelOwner* front_end_screen_set_level_owner(int level) noexcept;

// 0068AB81..0068AB98, the emptiness test CollapseOverlays runs before it clears
// a level: load the vector's first pointer, bail when it is null, then bail
// again when (last - first) >> 2 is zero. Both halves are needed because a
// vector that has been emptied keeps its buffer.
bool front_end_screen_set_level_populated(const std::vector<int>& level) noexcept;

// 004DA780, the mission teardown, at 004DA99D..004DA9CF. It clears input-context
// levels 1..4 and screen-set levels 1..4, each with a bare terminator, then
// resets every input context it can reach. The front end therefore always runs
// with levels 1, 2 and 3 empty, which is why the level-4 set the manager
// publishes is the whole visible front-end set.
void clear_all_screen_set_levels_004da780(FrontEndScreenSetStack& screens,
    FrontEndScreenTable& table, FrontEndScreenSetHostBindings& bindings,
    GameInputContextSetStack& contexts);

// 004DA9D2..004DA9FD walks contexts 1..1Eh inclusive (MOV EDI,1; CMP EDI,1Eh;
// JLE) and drops each one whose level is above 1. 004C4300's clear pass stops at
// 19h (CMP EDI,1Ah; JL), so contexts 1Ah..1Eh exist but are never dropped by a
// level clear: only the teardown resets them. bsp/frontend_screen_sets.hpp's
// kGameInputContextCount describes 004C4300's bound, which is the narrower of
// the two; it is correct for that routine and is not changed here.
inline constexpr int kGameInputContextTeardownLast = 0x1E; // inclusive

// ---------------------------------------------------------------------------
// Which routine pumps the screens, and when
// ---------------------------------------------------------------------------

// The transition crosses a frame-path boundary, and that is the one part of the
// path a reader is most likely to get wrong. GGame::OnMove takes the front-end
// branch at 004E4B9D only for game states 1, 2 and 4; that branch ends by
// running the pump and returning. The main-menu shell rests at state 5, which is
// not in that set, so from the moment 004E4279 writes 5 the screens are pumped
// somewhere else entirely.
enum class FrontEndScreenPumpSite {
    // 004E4CA3, the shared tail of front-end states 1, 2 and 4. Reached with the
    // raw frame delta.
    FrontEndStateTail,
    // 004C4165 inside 004C40F0 BSP_Game_UpdateInterfaceOnly, which OnMove calls
    // unconditionally at 004E53B6 on the path that does not take the front-end
    // branch. This is what pumps the main menu once the shell is ready.
    GameInterfaceOnly,
    // 004CAC37, reached from the loader 008C8560 with a delta of zero to draw a
    // frame synchronously. Not part of the per-frame path.
    LoaderSynchronous,
};

// Game states that take the front-end branch, read from the compares at
// 004E4B9D..004E4BDE. Any other state falls through to the full update path.
inline constexpr std::array<std::int32_t, 3> kFrontEndBranchGameStates{{1, 2, 4}};

FrontEndScreenPumpSite front_end_screen_pump_site(std::int32_t game_state) noexcept;

// ---------------------------------------------------------------------------
// The path itself
// ---------------------------------------------------------------------------

// Registry slot and interface id the main menu occupies. The identity arm of
// 00685820 means the two are equal, which is a coincidence of this manager and
// not a rule: the multi-menu map at 00687330 is not the identity.
inline constexpr int kMainMenuPathInterfaceId = 0x01; // INTF_MAINMENU
inline constexpr int kMainMenuPathScreenId = 0x01;    // registry slot 1, 00CEFC5C
// 0068584D and every sibling arm: the one-element input-context set the manager
// publishes alongside the screen set.
inline constexpr int kMainMenuPathInputContext = 0x01;
// 00686170 stores 4 at manager+4h for the rewards entry; the shell only pushes
// the main-menu request when the applied id is not that value.
inline constexpr int kMainMenuPathModeSkipPush = 0x04; // 004E4250

// The ordered steps. Each names the native routine that performs it. No native
// routine spans the whole path: it crosses the press-start screen, the title
// object, the state-request ring, the shell entry, the manager and the pump.
enum class MainMenuPathStep {
    // 0067CFB0 pass C. 0067D2DD tests input action 4Eh through 004C43C0.
    PressStartPoll,
    // 0068D8A0 BSP_TitleScreen_Skip. 0068D8A6 resets storage availability;
    // 0068D8B1 refuses to enqueue when game+5E8h is already non-zero.
    TitleSkip,
    // 0068D8BC, 004D7920 with 4, then 0068D8C6 clears the hold byte game+5ECh.
    RequestShellState,
    // 004E4D07: with the hold byte clear, 004E4430 drains the ring and case 4
    // calls 004E4000.
    DrainStateRequest,
    // 004E4000 up to 004E424A: the title object is destroyed, the GVMainMenu
    // block loads and the three managers are constructed and Init'd (00686380).
    EnterShell,
    // 004E4259, 004CC460(1, 0). Writes the pending record at manager+20h/+38h
    // only, so nothing is visible yet.
    PushInterfaceRequest,
    // 004E4269, manager vtable +8h = 00684700, then 004E4274, 005884A0.
    ActivateManager,
    // 004E4279, game+5D4h = 5. The frame path changes here.
    PublishShellReady,
    // 004E5442, 006840F0 sees manager+4h != manager+20h and calls vtable +10h.
    ServiceInterfaceRequest,
    // 00685826, 00684600 accepts and syncs applied = pending; 00685848 maps the
    // id through the identity arm.
    ApplyInterfaceRequest,
    // 006858BA, 004F8710(screenId, 0) then 004D8C00(game, 1, 0). 004F8710 calls
    // 004F7620 itself, so the requested bytes are current when it returns.
    PublishScreenSet,
    // 004C4165, 004F8830 pass B: 004F88E4 sets +5h, 004F88E8 commits through
    // 004F83B0, then 004F88F4 calls the enter virtual 005987F0.
    EnterScreen,
    // 004F8925, pass C, the screen's update virtual 00599DB0.
    ScreenVisible,
};

std::string_view main_menu_path_step_name(MainMenuPathStep step) noexcept;
// The native routine that performs the step, for the call-site table.
std::uint32_t main_menu_path_step_address(MainMenuPathStep step) noexcept;

// The observable state the rule reads. Only the fields a routine on this path
// demonstrably reads or writes are modelled; every other byte of the game
// object and the manager is unrecovered.
struct MainMenuPathState {
    std::int32_t game_state{0};            // game+5D4h
    bool state_request_pending{false};     // game+5E8h != 0
    bool state_requests_held{true};        // game+5ECh
    int pending_interface_id{0};           // manager+20h
    int applied_interface_id{0};           // manager+4h
    int manager_mode{0};                   // manager+4h as the shell reads it
    int published_screen_id{kFrontEndScreenIdNone};
    bool manager_active{false};            // manager+3Ch
};

// One method per native call site the path reaches, in call order. There are no
// default implementations: nothing here stands in for unrecovered behaviour.
struct MainMenuPathHost {
    virtual ~MainMenuPathHost() = default;

    // --- press-start poll --------------------------------------------------
    // 0067D2DD, 004C43C0 with bsp::kPressStartInputAction. The pure rule is
    // bsp::action_pressed_this_frame_004c43c0; this method is the lookup of the
    // action record, not a second copy of the rule.
    virtual bool input_action_pressed(int action) = 0;

    // --- title skip, 0068D8A0 ----------------------------------------------
    // 0068D8A6, 00BD3450 with ECX = *0109CECC.
    virtual void reset_storage_availability() = 0;
    // 0068D8BC, 004D7920 BSP_Game_RequestState.
    virtual void request_game_state(std::int32_t state) = 0;
    // 0068D8C6, game+5ECh = 0.
    virtual void release_state_request_hold() = 0;

    // --- the drain, 004E4D07 -----------------------------------------------
    // 004E4430. Returns the request it popped, or 0 when the ring was empty.
    virtual std::int32_t drain_state_request() = 0;
    // 004E4000, bsp::enter_front_end_shell in bsp/frontend_entry.hpp. Runs to
    // completion on this thread.
    virtual void enter_front_end_shell() = 0;

    // --- the shell tail, 004E424A..004E4279 --------------------------------
    // 004E424A, *(00E198AC)+4h.
    virtual int main_menu_manager_mode() = 0;
    // 004E4259, 004CC460 with ECX = the main-menu manager. The payload is the
    // null 004E4256 pushes.
    virtual void push_interface_request(int interface_id, void* payload) = 0;
    // 004E4269, manager vtable +8h = 00684700.
    virtual void activate_main_menu_manager() = 0;
    // 004E4274, 005884A0 with ECX = *(00E198AC)+58h, the 578h screen object.
    virtual void start_main_menu_screen_object() = 0;
    // 004E4279, game+5D4h = 5.
    virtual void set_game_state(std::int32_t state) = 0;

    // --- the service pass, 004E5442 ----------------------------------------
    // 006840F0. True when a channel needed servicing, which is the byte the
    // dropped do/while at 004E5455 loops on.
    virtual bool service_pending_interface_requests() = 0;
    // 00685826, 00684600. False means the request was rejected and the override
    // returns without publishing anything.
    virtual bool apply_interface_request(int interface_id, void* payload) = 0;
    // 00685848 and its sibling arms. bsp::main_menu_screen_id_00685820 is the
    // pure map; this method exists so the sequence can be driven against a host
    // that maps differently, as the multi and options managers do.
    virtual int map_interface_to_screen(int interface_id) = 0;

    // --- publishing, 006858BA ----------------------------------------------
    // 004F8710. An empty list is the native's bare terminator.
    virtual void publish_screen_set_level4(const int* ids, std::size_t count) = 0;
    // 004D8C00 with the game object. The default arm of 00685820 skips this
    // call entirely, so the sequence below skips it for screen id 0 too.
    virtual void publish_input_context_set_level4(const int* ids, std::size_t count) = 0;

    // --- the pump, 004C4165 ------------------------------------------------
    // 004F8830. Returns the phase the screen was in when the pass reached it.
    virtual FrontEndScreenPhase pump_front_end_screens(float raw_delta) = 0;
    // 004F83B0, the visibility commit the enter pass runs before the virtual.
    virtual void commit_screen_visibility(int screen_id) = 0;
    // Screen vtable +18h; 005987F0 for the main-menu screen.
    virtual void enter_screen(int screen_id) = 0;
    // Screen vtable +20h; 00599DB0 for the main-menu screen.
    virtual void update_screen(int screen_id, float raw_delta) = 0;
};

// Performs the step's native action through the host and returns the step that
// follows. A step whose guard is not satisfied returns itself, which is the
// native behaviour: the frame repeats until the guard opens.
//
// `raw_delta` is the unscaled frame delta the pump and the update virtual take;
// the earlier steps ignore it.
MainMenuPathStep advance_main_menu_path(MainMenuPathStep step, MainMenuPathState& state,
    MainMenuPathHost& host, float raw_delta);

// Drives the path from PressStartPoll until it reaches ScreenVisible or stalls.
// `max_steps` bounds the loop; the native has no such bound, it simply runs one
// frame at a time. Returns the step it stopped on.
MainMenuPathStep run_main_menu_path(MainMenuPathState& state, MainMenuPathHost& host,
    float raw_delta, int max_steps);

// 006858BA and the default arm 00685938 as one routine: map the id, publish the
// screen set, and publish the input-context set only when the map produced a
// real screen. Screen id 0 publishes the empty screen set and leaves the input
// contexts of the previous interface in place.
void publish_main_menu_interface(int interface_id, MainMenuPathHost& host,
    MainMenuPathState& state);

} // namespace bsp
