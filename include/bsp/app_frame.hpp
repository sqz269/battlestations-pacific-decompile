#pragma once
#include <cstdint>

#include "bsp/frame_clock.hpp"

namespace bsp {
// Projection of the two per-frame flag bytes of the native application object.
// Constructor 00737970 stores 1 at +18h and 0 at +19h and +1Ah. This is not the
// full native layout; +4h..+14h hold unreconstructed subsystem pointers and
// +14h is the game object passed to the update in ECX.
struct ApplicationFrameState {
    bool first_update{true}; // +18h, cleared after the game update at 00737b2b
    bool input_action_latch{false}; // +19h, cleared at 00737ac0 and set at 00737af2
};

// One-time initialisation of the PERF_APP_UPDATE bar colour at 00e1ae94, guarded
// by bit 0 of 00e1ae98 (00737a6c..00737a98). Bytes 94h/95h/96h are cleared and
// 97h becomes FFh, so the little-endian dword is FF000000h.
struct FrameMarkerColor {
    bool initialized{false};
    std::uint32_t argb{0};
};
std::uint32_t frame_marker_color_00737a6c(FrameMarkerColor& color) noexcept;

// Game states 1, 2 and 4 read from *(00e188a8)+5D4h. The same three values gate
// the close policy in 004ca2f0. docs/APP_INIT_GAME_ENTRY.md establishes 1 as the
// logo sequence and 2 as the title screen, so this is the front-end set; the
// input edge test in the frame runs only outside it (in-mission states).
bool is_front_end_game_state(int game_state) noexcept;

// Input action index pushed to 004c43c0 at 00737ae7. The action table entry is
// not identified, so this is a raw index rather than a named control.
inline constexpr int kApplicationFrameInputAction = 0x0E;

// Integration boundary for the subsystems the application frame reaches. Each
// method is one native call site, listed in frame order. There are no default
// implementations: nothing here is a stand-in for unrecovered game behaviour.
struct ApplicationFrameHost {
    virtual ~ApplicationFrameHost() = default;
    // Profiler singleton 004c1dd0; colour array at instance+24h indexed by the
    // PERF_APP_UPDATE slot held in 0109d014.
    virtual void profiler_set_frame_slot_color(std::uint32_t argb) = 0;
    virtual void profiler_begin_frame_slot() = 0; // 004c1dd0 then 00be3640
    virtual int game_state() = 0; // *(00e188a8)+5D4h
    virtual bool input_action_pressed(int action) = 0; // 004c43c0
    virtual void advance_frame_clock() = 0; // clock 01090ab0 virtual +8h -> 00bedc30
    virtual const ClockTimestamp& frame_interval() = 0; // virtual +1Ch -> 00bee070
    virtual void game_on_move(float seconds) = 0; // 004e4a40, ECX = application+14h
    virtual bool exit_requested() = 0; // 00e1ae75
    virtual void request_loop_exit() = 0; // platform 0109cf04 +181h = 1
    virtual void tick_vfs_providers() = 0; // 00bdb0b0, ECX = 0109ceec
    virtual void update_loading_queue() = 0; // 004fde20 then 00509190
    virtual void profiler_end_frame_slot() = 0; // 004c1dd0 then 00be3660
    virtual void profiler_end_frame() = 0; // 004c1dd0 then 00be34d0
};

// Application frame 00737a50. Native ECX = application, no stack arguments, RET,
// no return value; the caller 00bece70 ignores it and the message loop observes
// exit through platform byte +181h instead. The frame interval is returned here
// only so a host can see what the game update received. The native body also
// wraps the profiler slot in an SEH scope guard (handler 00c86228); that unwind
// path is not modelled.
float run_application_frame(ApplicationFrameState& state, FrameMarkerColor& color,
    ApplicationFrameHost& host);
}
