#pragma once
// bsp_game.exe milestone 2c: the frame's game state and profiler, the title
// pump and the main menu, as process bindings.
//
// Addresses: 00737acc / 00737b33 the game-state field the frame reads twice,
// 004c1dd0 / 00be3260 / 00be3640 / 00be3660 / 00be34d0 the profiler counter
// pair with its clock 00ce2270; 004c9a70 GGame::OnInitTitle with the frame
// layout selector 00518250; 0068d8d0 / 0068d850 the title handover; 004f7180 /
// 004f71d0 / 004f71a0 / 004f71f0 the screen registry, 004f8830 the pump and
// 004f83b0 the visibility commit; 0067c840 / 0067ca80 / 0067cb40 / 0067cfb0 the
// press-start screen; 004d7920 / 004e4430 the state-request ring; 004e4000 the
// front-end shell entry; 004cc460 / 00684600 / 00684700 / 00683aa0 / 00683e90 /
// 006840f0 / 00686380 the front-end managers; 004f8710 / 004d8c00 / 004f7620
// the level-4 screen set; 004c40f0 BSP_Game_UpdateInterfaceOnly and 004c43c0
// the input edge.
//
// Nothing in this file is a reconstruction of native code. Every type is an
// integration binding over the reconstructions listed in
// docs/APP_FRAME_GAME_STATE.md, docs/FRONTEND_STATE_MACHINE.md,
// docs/MAIN_MENU_PATH.md, docs/MAIN_MENU_SCREENS.md, docs/GAME_TITLE_INIT.md,
// docs/PRESS_START_SCREEN.md, docs/FRONTEND_SCREEN_SETS.md,
// docs/FRONTEND_MANAGERS.md and docs/GAME_FRONTEND_ENTRY.md, satisfying their
// host interfaces with either a concrete implementation or the explicit
// unimplemented policy in GameHostLog.

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/app_frame_game_state.hpp"
#include "bsp/main_menu_path.hpp"

namespace bsp {
class LocaleTables;
}  // namespace bsp

namespace bsp::game {

class GameHostLog;
class GameFrontendHost;
class GameVfsHost;
class GameScriptHost;
class GameMissionHost;

// ---------------------------------------------------------------------------
// The profiler counter pair the application frame brackets itself with
// ---------------------------------------------------------------------------

// Storage for bsp::ProfilerCounters, which mirrors the native object's six
// allocations rather than owning them. 00be3820 sizes all six from one
// capacity, the static counter count at 00e15118 plus 32h spare slots; that
// static count is not in the image, so the executable picks its own capacity
// and says so.
class GameFrameProfiler final : public ProfilerClockHost {
public:
    GameFrameProfiler(GameHostLog& log, int slot_count);

    // 00ce2270, the indirect QueryPerformanceCounter the counter pair samples
    // only on its outermost entry and exit.
    std::int64_t query_performance_counter() override;

    ProfilerCounters& counters() noexcept { return counters_; }
    // PERF_APP_UPDATE, held at 0109d014 and written at run time by the counter
    // registration 00408720. Slot 0 is skipped by 00be3640 and 00be3660, so the
    // executable registers the frame in slot 1 and records the substitution.
    int app_update_slot() const noexcept { return 1; }
    int registered_slot_count() const noexcept { return slot_count_; }
    // 0109db48, zero in the image and filled at run time. Its unit is unproven
    // (docs/APP_FRAME_GAME_STATE.md), and a zero scale makes every converted
    // value non finite, so the executable supplies seconds per performance
    // counter tick and marks it as its own substitute.
    double tick_scale() const noexcept { return tick_scale_; }
    unsigned long long samples() const noexcept { return samples_; }
    const ProfilerCounterRecord& frame_record() const noexcept;

private:
    GameHostLog& log_;
    int slot_count_{};
    std::vector<ProfilerCounterRecord> records_;
    std::vector<float> history_;
    std::vector<float> start_offsets_;
    std::vector<float> current_;
    std::vector<std::uint32_t> colors_;
    std::vector<ProfilerDisplayRecord> display_;
    ProfilerCounters counters_{};
    double tick_scale_{};
    unsigned long long samples_{};
};

// ---------------------------------------------------------------------------
// The front-end runtime
// ---------------------------------------------------------------------------

// What one run's front end did, for the summary and the report.
struct GameMenuSummary {
    bool title_init_ran{false};
    bool press_start_registered{false};
    int press_start_slot{-1};
    int game_state{0};
    unsigned long long pump_frames{0};
    std::size_t screen_enters{0};
    std::size_t screen_exits{0};
    unsigned long long screen_updates{0};
    std::size_t visibility_commits{0};
    long press_start_frame{-1};
    bool press_start_injected{false};
    bool shell_entered{false};
    bool main_menu_manager_active{false};
    int published_screen_id{0};
    std::string path_step;
    std::size_t main_menu_pages_loaded{0};
    std::size_t screens_registered{0};
};

// Milestone 2e: the scripted mission selection, once the shell has published
// the main-menu screen. Null when --menu-select named nothing.
class GameMissionHost;

// The front-end half of the frame: the registry at 00e18b60, the title object
// at 00e198c8, the press-start screen at registry slot 5Ch, the state-request
// ring at game+5D8h, the three front-end managers and the level-4 screen set.
//
// run_title_init_004c9a70 is the bring-up the startup sequence runs after Init;
// frame() is the front-end branch of GGame::OnMove at 004e4b9d for game states
// 1, 2 and 4, and BSP_Game_UpdateInterfaceOnly 004c40f0 for everything else,
// which is the split bsp::front_end_screen_pump_site names.
class GameMenuHost {
public:
    // press_start_frame is --press-start-frame N: the frame on which the input
    // edge for action 4Eh is injected through
    // bsp::action_pressed_this_frame_004c43c0's rule. Negative injects nothing.
    //
    // Milestone 2e: `menu_select` is --menu-select, the mission id the
    // mission-tree screen's loader asks the shell for at 00586150. An empty id
    // leaves the run exactly where milestone 2d left it.
    //
    // Milestone 2f: `mission_frames` is --mission-frames, the number of
    // in-mission frames of 004e4a40 to run once the load has finished, and
    // `profiler` is the counter pair the frame brackets itself with.
    GameMenuHost(GameHostLog& log, GameFrontendHost& frontend, GameStateSlot& state,
        long press_start_frame, GameVfsHost& vfs, GameScriptHost& scripts,
        LocaleTables& locale, std::string menu_select, long mission_frames = 0,
        GameFrameProfiler* profiler = nullptr, std::string language = {});
    ~GameMenuHost();
    GameMenuHost(const GameMenuHost&) = delete;
    GameMenuHost& operator=(const GameMenuHost&) = delete;

    // 004c9a70, reached from BSP_Game_BeginStartupSequence 004e5753.
    void run_title_init_004c9a70();
    // One frame of the front end. frame_index counts presented frames, so the
    // injected press lands on the frame the command line names.
    void frame(float raw_delta, unsigned long long frame_index);
    // 004c43c0 for the application frame's own edge test at 00737ae7 and for
    // the press-start poll at 0067d2dd.
    bool input_action_pressed(int action);

    const GameMenuSummary& summary() const noexcept;
    // Milestone 2e. Null when --menu-select named nothing.
    GameMissionHost* mission() const noexcept;

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
