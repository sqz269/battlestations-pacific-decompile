#pragma once
// bsp_game.exe milestone 2h: the in-mission HUD screens and their GUI pages,
// as process bindings.
//
// Addresses: 0068a990 and 0068cc70 (the in-mission interface manager and its
// Init, neither reconstructed: both stay unimplemented records), 004f7180 /
// 004f71d0 (the screen base and the registration every one of the 42 HUD
// screens' +10h override calls), 00aa5840 (the page loader each register or
// layout virtual runs), 00aa7e00 (the direct-child lookup those virtuals bind
// their widgets with), 004cc460 (the INTF_SCENE3D request Init pushes at
// 0068cd44), 0068aca0 (ApplyPendingInterface, which turns that id into a
// level-1 screen set), 004f8530 and 004d8a50 (the level-1 setters), 004f7620
// (the recompute they run) and 004f83b0 (the per-screen visibility commit the
// pump makes).
//
// Nothing in this file is a reconstruction of native code. Every type is an
// integration binding over bsp/ingame_interface.hpp, bsp/hud_screens.hpp and
// bsp/frontend_screen_sets.hpp, satisfied either by a reconstruction already on
// main or by the explicit unimplemented policy in GameHostLog. The 42 screen
// classes are not reconstructed, so each screen is the executable's own
// registry record built from the recovered slot table, exactly the substitution
// milestone 2c makes for the seven main-menu screen classes.
//
// Evidence: docs/IN_MISSION_INTERFACE_MANAGER.md,
// docs/IN_GAME_INTERFACE_SCREEN_SETS.md, docs/HUD_SCREEN_PAGES.md,
// docs/GUI_LAYOUT_LOADER.md, docs/FRONTEND_SCREEN_SETS.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace bsp::game {

class GameHostLog;
class GameMenuHost;

// One of the 42 screens the manager's Init constructs, as this run built it.
struct GameHudScreenRecord {
    int registry_slot{-1};
    std::uint16_t manager_offset{0};
    std::uint32_t register_virtual{0};
    std::uint32_t layout_virtual{0};
    std::vector<std::string> pages;   // the page names its virtuals ask for
    std::size_t pages_loaded{0};
    std::size_t widgets_requested{0};
    std::size_t widgets_bound{0};
    bool in_level1_set{false};        // the applied interface lists this slot
};

struct GameHudSummary {
    bool manager_built{false};       // the executable's registry records exist
    bool manager_global_published{false};  // 0068ccfe, the second write of 00e198c4
    std::size_t screen_constructors{0};    // the 42 leaf constructors, all records
    std::size_t screens_built{0};    // of kInGameHudScreenCount
    std::size_t pages_requested{0};
    std::size_t pages_loaded{0};
    std::size_t widgets_requested{0};
    std::size_t widgets_bound{0};
    int pushed_interface_id{0};      // 004cc460 at 0068cd44
    int applied_interface_id{0};     // what 0068aca0 was run with
    bool interface_applied{false};
    std::size_t level1_screens{0};
    std::size_t level1_contexts{0};
    std::vector<int> level1_screen_ids;
    std::vector<int> level1_context_ids;
    std::vector<std::string> level1_pages;  // the pages those screens hold
    unsigned long long pump_frames{0};
    // Step 17 of the in-mission frame, 0068c1f0 at 004e5252.
    unsigned long long update_frames{0};
    std::string audio_environment;   // what the update's tail chose, 00a7b710
    std::vector<GameHudScreenRecord> screens;
};

// The in-mission HUD, owned by the front-end host because the 95-slot registry
// at 00e18b60 the HUD screens register into is the same one the front-end
// screens use.
class GameHudHost {
public:
    GameHudHost(GameHostLog& log, GameMenuHost& menu);
    ~GameHudHost();
    GameHudHost(const GameHudHost&) = delete;
    GameHudHost& operator=(const GameHudHost&) = delete;

    // The `create_hud_manager` row of the load walk, 0068a990 at 004e046c, and
    // the Init 0068cc70 its caller runs. The construction itself is a record;
    // what runs is the part Init performs through recovered code: each screen's
    // +10h registration into the registry, the page each one loads through
    // 00aa5840 and the widget names it binds through 00aa7e00, then the
    // INTF_SCENE3D request 004cc460 pushes at the end of Init.
    void build_manager_0068a990();

    // 006840f0 services the pushed request through 00684600, which reaches the
    // manager's own virtual +10h, 0068aca0. With no local player unit the
    // payload is null, which is the one arm of the id 20h body that publishes.
    void apply_pending_interface_0068aca0();

    // The `release_main_menu_manager` row of the load walk, 004dfd96: the
    // manager's vtable slot 0 (00687300) calls BSP_MainMenu_Destroy 00686c90,
    // which calls BSP_FrontEndManager_Deactivate 00683aa0, whose tail publishes
    // the empty level-4 screen set and input-context set. That is what takes the
    // front-end pages off the screen when a mission starts.
    void release_main_menu_manager_00686c90();

    // Milestone 2i. The load's `select_front_end_layout` row, 004c1ac0(3,0)
    // then 00518250(3,0): a non-committing call that loads set 3's pause pair
    // and releases nothing, so the front-end frame layouts the title bring-up
    // acquired stay loaded through the mission.
    void select_front_end_layout_00518250();

    // Milestone 2j. BSP_Game_ApplyInGameInterface 004c9ca0, both arms. The byte
    // argument is tested at 004c9cc0 and the routine is two routines sharing a
    // frame: the load calls it with 1 at 004e1873 and puts the loading element
    // up, and the mission-state entry calls it with 0 at 004da746 and tears that
    // element down, releases the front-end atlas, and runs the **committing**
    // 00518250(3, 1) at 004c9e06. That committing call is what releases the
    // front-end frame layouts the title bring-up acquired, which the load's own
    // non-committing row could not do.
    void apply_in_game_interface_004c9ca0(bool loading);

    // One in-mission frame of BSP_Game_UpdateInterfaceOnly 004c40f0, which is
    // where the screen pump runs while the game state is not 1, 2 or 4.
    void update_interface_only_004c40f0(float raw_delta);

    // Step 17 of the in-mission branch of 004e4a40, 0068c1f0 at 004e5252. The
    // frame's own gate is `00e198c4 != 0 && [00e198c4]+3Ch != 0`, which is what
    // manager_active() answers.
    bool manager_active() const noexcept;
    void update_in_game_interface_0068c1f0();

    // One summary line for the run log.
    void report();

    const GameHudSummary& summary() const noexcept;

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
