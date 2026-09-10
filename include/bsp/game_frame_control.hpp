#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "bsp/frame_clock.hpp"

// Control spine of the game update BSP_Game_OnMove (004e4a40): the per-frame
// delta scaling at 004c6e30, the state-request queue embedded at game+5D8h and
// drained by 004e4430, and the call sites the frame reaches unconditionally.
// docs/GAME_FRAME_CONTROL.md records the evidence. The phases between these
// ones belong to other packets and are deliberately absent.
namespace bsp {

// ---------------------------------------------------------------------------
// State request queue, game+5D8h
// ---------------------------------------------------------------------------

// The queue is an MSVC std::deque<int>. 004d3ed0 allocates one block with a
// 10h-byte malloc and 004e4430 indexes it as map[off / 4][off % 4], so the
// block holds 10h / sizeof(int) elements. Native fields, relative to game+5D8h:
// +0h allocator slot (untouched by push_back and pop_front), +4h block map,
// +8h block count, +Ch head offset in elements, +10h element count.
inline constexpr std::size_t kStateRequestsPerBlock = 4;

struct GameStateRequestBlock {
    std::uint32_t entries[kStateRequestsPerBlock]{};
};

struct GameStateRequestQueue {
    // Native +4h. A null slot is an unallocated block: 004d3f13 allocates one
    // on demand, and no path in the drain or the producer ever frees it.
    std::vector<std::unique_ptr<GameStateRequestBlock>> blocks;
    std::size_t head_offset{}; // +Ch, in elements, always < blocks.size() * 4
    std::size_t count{}; // +10h
};

// 004e4486..004e449b and 004d3eff..004d3f08. One conditional subtraction is
// enough because both callers keep the offset below two map lengths.
std::size_t state_request_block_index(const GameStateRequestQueue& queue,
    std::size_t offset) noexcept;

// 004e4450..004e449e. The native body first runs two _SECURE_SCL range
// validations that call the non-returning CRT handler 00bf6713 when the queue
// is empty; the precondition is documented rather than checked here because
// every native call site tests the count first.
std::uint32_t front_state_request(const GameStateRequestQueue& queue) noexcept;

// 004e44a4..004e44e4. Advances the head, wraps it at block_count * 4, and
// resets it to zero when the queue empties. Blocks are never released.
void pop_front_state_request(GameStateRequestQueue& queue) noexcept;

// 004d20d0. Grows the block map by max(count, max(block_count / 2, 8)) slots
// and rotates it so the head block keeps its index, which is why the head
// offset is left alone (004d2211 only adds to the block count). The native
// body copies the two halves with memmove; only the resulting order is
// reproduced here, not the instruction-level copy sequence.
void grow_state_request_map_004d20d0(GameStateRequestQueue& queue, std::size_t count);

// 004d3ed0, __thiscall, ECX = game+5D8h, one stack argument, RET 4. The
// argument is a pointer to the int to copy (004d3f35 loads through it), so
// call sites that appear to push a literal are pushing the address of a stack
// slot holding it.
void enqueue_state_request_004d3ed0(GameStateRequestQueue& queue, std::uint32_t request);

// ---------------------------------------------------------------------------
// Frame delta scaling, 004c6e30
// ---------------------------------------------------------------------------

// Image constants. The .rdata cell is named after each value; the two step
// clamps and the cinematic step are 32-bit, the four scales are 64-bit.
inline constexpr float kFrameDeltaMultiplayerMaxStep = 0.25f; // 00ce3868
inline constexpr float kFrameDeltaSingleMaxStep = 0.05f; // 00ce7638
// 00d7a270 holds 3FA99999A0000000h, which is the double promotion of the float
// above rather than the nearest double to 0.05, so the cast is load bearing.
inline constexpr double kFrameDeltaSingleStepThreshold
    = static_cast<double>(kFrameDeltaSingleMaxStep);
inline constexpr float kFrameDeltaCinematicStep = 0.0333333351f; // 00ce7628, 3D088889h
inline constexpr double kFrameDeltaFastForwardScale = 10.0; // 00ce3dc0
inline constexpr double kFrameDeltaTurboScale = 30.0; // 00ce7630
// 00ce3dc8 holds 3FD3333340000000h, again the promotion of a float constant.
inline constexpr double kFrameDeltaSlowMotionScale = static_cast<double>(0.3f);

// Time-scale multiplier at 00e0b6c8. The image ships 1.0f; nothing in the
// packet writes it, so the split-screen path currently scales by one.
inline constexpr float kFrameDeltaSplitScreenScale = 1.0f; // 00e0b6c8

// Input action indices. 004c43c0 indexes the input singleton's record array at
// instance+4h with a 30h stride, so the inline record addresses in 004c6e30
// and 004e4430 divide out to these. "Held" is the first half of the 004c43c0
// test (+28h set and +24h greater than zero); "pressed" adds its edge test
// (+20h zero, or +1Ch not greater than zero).
inline constexpr int kMissionStepAction = 8; // instance+4h record at +180h
inline constexpr int kFastForwardAction = 9; // +1B0h
inline constexpr int kTurboAction = 10; // +1E0h
inline constexpr int kCinematicStepAction = 11; // +210h
inline constexpr int kSlowMotionAction = 12; // +240h

// Local player slots. 004c6e50 walks eight pointers starting at game+18CCh in
// two passes of four, and 007713a0 indexes the same array with game+18ECh.
inline constexpr std::size_t kLocalPlayerSlotCount = 8;

struct LocalPlayerSlot {
    bool present{}; // the slot pointer is non-null
    bool joined{}; // slot+8h nonzero
    bool inactive{}; // slot+9h nonzero, which excludes the slot
};

// 004c6e50..004c6ec5.
int count_active_local_players_004c6e50(
    const LocalPlayerSlot (&slots)[kLocalPlayerSlotCount]) noexcept;

struct FrameDeltaScaleInputs {
    float raw_delta{}; // the OnMove stack argument
    std::uint32_t local_player_mode{}; // game+1FE4h
    int active_local_players{}; // count_active_local_players_004c6e50
    std::uint32_t game_state{}; // game+5D4h
    // 007713a0 with ECX = game+1EF0h, applied only in state Dh. It returns its
    // argument unchanged unless the mission object's mode at +F4h is set, so a
    // host that has not reconstructed the replay modes passes raw_delta back.
    float mission_filtered_delta{};
    bool max_step_clamp_disabled{}; // 00f876b0 nonzero
    bool cinematic{}; // game+634h
    bool fast_forward_held{}; // action 9
    bool turbo_held{}; // action 10
    bool slow_motion_held{}; // action 12
    bool cinematic_step_pressed{}; // action 11, full 004c43c0 edge test
};

struct FrameDeltaScaleResult {
    float delta{}; // written back through the argument pointer
    float step{}; // game+21ECh
    float scaled{}; // game+21F0h, the value the simulation reads
    float elapsed_increment{}; // added to game+64Ch
};

// 004c6e30, __thiscall, ECX = game, one stack argument holding a float*, RET 4.
// The routine rewrites the caller's delta slot, so the raw delta the rest of
// the frame sees is the clamped one, not the argument the application passed.
FrameDeltaScaleResult scale_frame_delta_004c6e30(const FrameDeltaScaleInputs& inputs) noexcept;

// ---------------------------------------------------------------------------
// Mission completion poll, 004d7ea0
// ---------------------------------------------------------------------------

struct MissionResult {
    bool present{}; // game+7188h non-null
    bool requests_debrief{}; // result+21h
    float score{}; // result+8h, handed to the result GUI at 004d7f1d
};

// ---------------------------------------------------------------------------
// Owned state
// ---------------------------------------------------------------------------

// Projection of the game fields this packet establishes. Not the native
// layout: the offsets are in the comments and the intervening fields belong to
// other packets.
struct GameFrameControlState {
    std::uint32_t state{}; // +5D4h
    GameStateRequestQueue requests{}; // +5D8h
    bool drain_suspended{}; // +5ECh, gates the drain at 004e4d02
    float raw_delta{}; // the OnMove argument slot, rewritten by 004c6e30
    float step{}; // +21ECh
    float scaled_delta{}; // +21F0h
    float elapsed{}; // +64Ch, accumulates the undilated step
    std::uint32_t local_player_mode{}; // +1FE4h
    bool cinematic{}; // +634h
    bool mission_end_suspended{}; // +7184h
    // +1EE1h. Read only through the sete at 004e508e; its meaning is not
    // established, so it is carried as a raw byte.
    bool mission_flag_1ee1{};
    MissionResult mission_result{}; // +7188h
    // 00e1ae75, the byte the application frame reads as its exit request
    // (bsp::ApplicationFrameHost::exit_requested).
    bool application_exit_requested{};
};

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------

// One method per native call site this packet reaches, declared in frame
// order. Nothing has a default implementation: none of these stands in for
// unrecovered game behaviour. Method names ending in an address name the
// native routine; the rest describe a small inlined sequence.
struct GameFrameControlHost {
    virtual ~GameFrameControlHost() = default;

    // Pre-tick 004e2200, ECX = game, first call of the frame. Its own body
    // returns immediately unless the console command list at game+638h is
    // non-empty and the request queue is empty, so console commands can never
    // race a pending state transition.
    virtual void pre_tick_console_commands() = 0;

    // Delta inputs, sampled in the order 004c6e30 reads them.
    // game+18CCh, eight pointer slots walked in two passes of four.
    virtual void copy_local_player_slots(LocalPlayerSlot (&slots)[kLocalPlayerSlotCount]) = 0;
    virtual float mission_time_filter_007713a0(float delta) = 0; // ECX = game+1EF0h
    virtual bool max_step_clamp_disabled() = 0; // 00f876b0
    virtual bool input_action_held(int action) = 0; // first half of 004c43c0
    virtual bool input_action_pressed(int action) = 0; // full 004c43c0

    // Drain dispatch, 004e44e4..004e4866. Every arm is one native call site.
    virtual void request_02_004d8000() = 0;
    virtual void request_04_004e4000() = 0;
    virtual void request_06_notify_00e198ac() = 0; // vtable +Ch
    virtual void request_07_004bfc70() = 0;
    virtual void request_09_notify_00e198b4() = 0; // vtable +Ch
    virtual void request_0a_0b_004dfb70() = 0;
    virtual void request_0e_004c6b00() = 0;
    virtual void request_0f_004d7970() = 0; // one argument, zero
    // 004e458a. The long teardown arm: it leaves state 11h, may enqueue
    // request 4 through the same queue, and has a sub-path at 004e48d7 that
    // returns from the drain without running the tail. The host is handed the
    // owned state because the native handler writes both fields.
    virtual bool request_10_teardown_004e458a(GameFrameControlState& state) = 0;
    virtual void request_12_resume_004cd0f0() = 0; // 004cd0f0(1,1,1) plus 00e198c4 flag
    virtual void request_14_004bac20() = 0;
    virtual void request_16_notify_00e198b8() = 0; // vtable +Ch
    // 004e49fc..004e4a19: state 10h resolves to 8 when the session object at
    // 00e198b4 exists and its +3Ch byte is set, otherwise to 5.
    virtual bool network_session_active() = 0;
    virtual void post_drain_00a95960(float scaled_delta) = 0; // 004e4a24

    // Timing block, 004e4d3e..004e4d94.
    virtual void update_cutscene_playback_004c6b20(float raw_delta) = 0;
    virtual void mission_hud_update(float scaled_delta) = 0; // 00f88c20 vtable +8h

    // Unconditional per-frame block, 004e4de2..004e4def.
    virtual void accumulate_frame_statistics_0053c510() = 0; // ECX = game+19C8h
    virtual void update_presence_context_004c0170() = 0; // throttled XUserSetContext

    // Frame bookkeeping, 004e503b..004e5097.
    virtual void update_device_wait_screen_004db920() = 0; // state Ch only
    virtual void set_front_end_pending_flag(bool value) = 0; // 00e198b0

    // Mission completion poll, 004d7ea0, reached from the world tick.
    virtual MissionResult mission_result() = 0; // game+7188h
    virtual void world_final_tick(float delta) = 0; // game+19CCh vtable +Ch
    virtual void world_post_tick() = 0; // 00903670, ECX = game+19CCh
    virtual void show_mission_result_gui(float score) = 0; // 004c3cb0, 004f8a20, 004f8970
    virtual void close_mission_result() = 0; // 004cd610(1) then 004cc510
};

// ---------------------------------------------------------------------------
// Routines
// ---------------------------------------------------------------------------

// Game states this packet observes. Values are recovered; the names are
// hypotheses drawn from the console command that requests each one and from
// the handler the drain dispatches to. 1, 2 and 10 are the same field as
// bsp::GameStartupState in bsp/game_entry.hpp.
enum class GameStateId : std::uint32_t {
    kFrontEndRestore = 2,
    kFrontEnd = 4,
    kInMission = 13, // Dh, the simulation state
    kMissionDebrief = 15, // Fh, requested by 004d7ea0
    kMissionTeardown = 16, // 10h, the "term" console command
    kMissionEndWait = 17, // 11h
    kResumeGameplay = 18, // 12h, the "pause" console command
    kQuit = 19, // 13h, the "quit" console command
};

// 004e4430, __thiscall, ECX = game, no arguments, RET. Pops the queue front
// into game+5D4h and dispatches it, first in first out, until the queue is
// empty. Requests enqueued by a handler are therefore serviced in the same
// pass, behind the ones already queued. Two arms leave early: request Dh stops
// the loop with requests still pending, and request 13h returns without the
// tail call after setting the application exit byte.
void drain_state_requests_004e4430(GameFrameControlState& state, GameFrameControlHost& host);

// 004d7ea0, __thiscall, ECX = game, no arguments, RET. Runs only while the
// request queue is empty and a mission result object exists. Returns true when
// it enqueued the debrief request, which is also when it latches the drain
// suspension at +5ECh.
bool check_mission_completion_004d7ea0(GameFrameControlState& state, GameFrameControlHost& host);

// 004e504b..004e5097, the state 11h arm of the frame bookkeeping. The two
// stores to +5ECh are unconditional in the native fall-through order, so this
// clears the suspension whenever the mission-end suspend flag is down.
void update_mission_end_wait(GameFrameControlState& state, GameFrameControlHost& host);

// The control spine of BSP_Game_OnMove in native order. The phases between
// these belong to other packets: input, the blocking screen, the front-end
// states, the render queue, the simulation gate, the world tick and the
// profiler blocks are all absent, and the SEH frame at 004e4a48 is not
// modelled. The clock is the frame clock at 01090ab0, whose +4h field the
// native code accumulates with the scaled delta at 004e4d5e.
void run_game_frame_control(GameFrameControlState& state, FrameClock& clock,
    GameFrameControlHost& host);
}
