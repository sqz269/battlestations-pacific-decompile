#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/app_frame.hpp"
#include "bsp/game_frame_control.hpp"

// The game's render entry as the per-frame maps reach it: the begin/render/finish
// triple 004c6c30, 004ca440 and 004ca1f0. docs/GAME_RENDER_FRAME.md records the
// evidence behind every offset and every ordering claim named here. Nothing in
// this header is a recovered symbol; the names are hypotheses.
//
// The three routines are the bodies behind host methods other packets already
// declare: bsp::BlockingScreenHost::try_begin_render_frame and
// ::finish_render_frame, bsp::RenderTailHost::game_render and
// ::finish_render_frame, and bsp::FrontEndStateHost::game_render. Render types
// owned by the render-queue packets are referenced by name only, never copied:
// the queue submission point is bsp::execute_render_command_queue_00b1ebe0 in
// bsp/render_command_queue.hpp, and the frame-active predicate is
// bsp::is_render_frame_active_00b1fe20 in bsp/render_command_execution.hpp.
namespace bsp {

// ---------------------------------------------------------------------------
// Frame phase, game+34h
// ---------------------------------------------------------------------------

// The single field the three routines hand between each other. 004c6c30 moves
// 0 to 1, 004ca440 requires 1 and moves it to 2, 004ca1f0 requires 2 and moves
// it back to 0. Every transition is a plain store; nothing is interlocked.
enum class GameRenderPhase : std::uint32_t {
    kIdle = 0, // 004c6c30 is the only routine that leaves this state
    kBegun = 1, // renderer BeginFrame has run, BSP_Game_Render may proceed
    kRendering = 2, // callbacks have been issued, the frame awaits its present
};

// ---------------------------------------------------------------------------
// View selection, 004ca4e7..004ca5ce
// ---------------------------------------------------------------------------

// The four exits of the scene-versus-interface branch. The native code does not
// name them; these are the four control-flow destinations.
enum class GameRenderViewPath {
    // 004ca5ce. No interface view is selected, so the world view runs inline on
    // the render thread through 00735b50. This is the front-end and attract
    // path, and also the split-screen in-mission path.
    kInlineWorldView,
    // 004ca529. Interface level 1: the world view and the camera update are
    // enqueued on the frame job pool and dispatched, and 00735b50 is skipped.
    kJobPoolScene,
    // 004ca5b2. Interface level 2: 0059d7b0 on interface manager +54h, then the
    // inline world view.
    kInterfaceOrthoView54,
    // 004ca5c1. Interface level 3: 00535430 on interface manager +5Ch, then the
    // inline world view.
    kInterfaceOrthoView5C,
};

// Inputs of the branch, in the order 004ca4e7 reads them. Every field is one
// native load; the document gives the address of each.
struct GameRenderViewGate {
    std::uint32_t local_player_mode{}; // game+1FE4h, the same field as bsp::GameFrameControlState
    bool split_view{}; // game+2058h
    std::uint32_t game_state{}; // game+5D4h
    bool interface_manager_present{}; // 00e198c4, the in-mission interface manager
    bool mission_end_suspended{}; // game+7184h
    // 00e08310, latched by 004f7620 as the highest active interface level.
    // docs/GAME_SIMULATION_GATE.md establishes the writer; what the levels rank
    // is not established, so only the three values this branch tests are named.
    std::uint32_t interface_level{};
};

// 004ca4e7..004ca5cc. The first three loads form one conjunction that skips the
// whole interface block; the interface manager and the suspend byte skip it too.
GameRenderViewPath select_view_path_004ca4e7(const GameRenderViewGate& gate) noexcept;

// True for every path except kJobPoolScene. Level 1 is the only one that jumps
// over 00735b50 at 004ca5b0; levels 2 and 3 fall through to it at 004ca5bf and
// 004ca5c9, and so does every skipped path.
bool runs_inline_world_view(GameRenderViewPath path) noexcept;

// ---------------------------------------------------------------------------
// Frame job pool, singleton 004c1130
// ---------------------------------------------------------------------------

// The two jobs BSP_Game_Render enqueues. Each is a two-word slot holding the
// singleton pointer and a zero argument; the drain calls the object's first
// virtual with that argument.
enum class GameRenderJob {
    // Singleton 004c0960 at 00e18ad8, primary virtual 004bbd00. It builds an
    // identity matrix, runs 0068a670 on the interface manager and hands the
    // result to 00f8bbd8 virtual +4h. Same body as the inline 00735b50, minus
    // that routine's null guard on the interface manager.
    kWorldView004bbd00,
    // Singleton 004c0a30 at 00e18adc, primary virtual 004b4820, which forwards
    // to 0068a0d0 on the interface manager: 0078cff0(camera, global time),
    // 00b101c0(game+19ECh, camera), 004c7e60().
    kCameraUpdate004b4820,
};

// 00be2fa0, the drain. It decrements the count at pool+24h and indexes the slot
// array with the post-decrement value, so jobs run last in first out: the camera
// update runs before the world view even though it is enqueued second. Worker
// threads woken by 00be3150 drain the same array, so with more than one runner
// the interleaving is not fixed; this returns the order a single drainer sees.
std::vector<GameRenderJob> frame_job_drain_order(const std::vector<GameRenderJob>& enqueued);

// pool+24h is a 10000-entry array of two-word slots, zeroed by the constructor
// 00be3040 and never bounds checked by 00be3020.
inline constexpr std::size_t kFrameJobCapacity = 10000;

// ---------------------------------------------------------------------------
// Debug toggles, 004ca5d9 and 004ca699
// ---------------------------------------------------------------------------

// Input action indices. Both sites inline the 004c43c0 rising-edge test against
// a record in the input singleton's array at instance+4h, whose stride is 30h
// (bsp/game_frame_control.hpp). The inline offsets 570h and 7B0h divide out to
// these indices.
inline constexpr int kScreenshotToggleAction = 0x1D; // record at +570h
// Record at +7B0h. Its toggle target 00e18b35 is read and written only inside
// BSP_Game_Render, so this action currently changes nothing observable.
inline constexpr int kUnusedDebugToggleAction = 0x29;

// 004ca61e..004ca639 and 004ca6de..004ca6ee. The native sequence is the MSVC
// expansion of a bool negation: SETZ, a compare that can only fail when the byte
// holds neither 0 nor 1, then the store. Returns the new value.
bool toggle_debug_flag(bool& flag) noexcept;

// ---------------------------------------------------------------------------
// Screenshot capture, 004bfbf0 and 004c9a00
// ---------------------------------------------------------------------------

// The globals the screenshot pair shares with BSP_Game_Render and
// BSP_Game_FinishRenderFrame.
struct ScreenshotState {
    bool capturing{}; // 00e188ad, toggled by action 1Dh, read by 004ca1f0
    int sequence{}; // 00e08210, the numbered directory
    int frame{}; // 00e188b8, the image index inside that directory
    // 00e1899c when non-null, otherwise the buffer at 00e18b1c. Both routines
    // re-read it, so a base directory changed mid-sequence takes effect at once.
    std::string base_directory{};
};

// 004bfc0c and 004c9a1e. Both native sites format into a 512-byte stack buffer
// through 004b7f70, so a base directory long enough to overflow it is a native
// defect; the reconstruction truncates instead.
inline constexpr std::size_t kScreenshotPathCapacity = 512;
std::string screenshot_directory_004bfbf0(const std::string& base, int sequence);
std::string screenshot_frame_path_004c9a00(const std::string& base, int sequence, int frame);

// ---------------------------------------------------------------------------
// Owned state
// ---------------------------------------------------------------------------

// Projection of the fields these three routines read or write. Not the native
// layout: the offsets are in the comments and everything between them belongs
// to other packets.
struct GameRenderFrameState {
    GameRenderPhase phase{GameRenderPhase::kIdle}; // game+34h
    float scaled_delta{}; // game+21F0h, the dilated delta 004c6e30 produced
    std::uint32_t game_state{}; // game+5D4h
    std::uint32_t local_player_mode{}; // game+1FE4h
    bool split_view{}; // game+2058h
    bool mission_end_suspended{}; // game+7184h
    bool profiler_overlay{}; // game+2191h
    bool interface_manager_present{}; // 00e198c4
    std::uint32_t interface_level{}; // 00e08310
    // Bit 0 of 00e18b3c, the MSVC static-local guard on the one-shot profiler
    // zone registration at 004ca490.
    bool profiler_zone_registered{};
    bool unused_debug_toggle{}; // 00e18b35
    ScreenshotState screenshot{};
};

// The literal at 00ce76ec, the only string BSP_Game_Render references.
inline constexpr const char* kGameRenderProfilerZone = "GGame::Render";

// The colour 004ca20d stores into the profiler slot array before opening the
// finish counter, as a little-endian dword.
inline constexpr std::uint32_t kFinishRenderFrameSlotColor = 0xFFFFFFFFu;

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------

// One method per native call site, declared in frame order. Nothing has a
// default implementation: none of these stands in for unrecovered behaviour.
// A method name ending in an address names the native routine it reaches; the
// rest describe a small inlined sequence.
struct GameRenderFrameHost {
    virtual ~GameRenderFrameHost() = default;

    // --- 004c6c30 and 004ca1f0 share these ---
    // 00b1bf90 through the queue singleton 004c11f0. Non-zero holds the frame
    // closed: BSP_Game_TryBeginRenderFrame refuses to begin and
    // BSP_Game_FinishRenderFrame refuses to present.
    virtual bool render_queue_retained() = 0;
    // Renderer 00f8d394 primary virtual +Ch -> 00b2b200, which reaches
    // IDirect3DDevice9::BeginScene at device vtable +A4h.
    virtual void renderer_begin_frame() = 0;

    // --- 004ca440, in order ---
    // Singleton 00f88c20 virtual +Ch, one float argument: game+21F0h.
    virtual void hud_render(float scaled_delta) = 0;
    // The same singleton, virtual +4h, no arguments.
    virtual void hud_post_render() = 0;
    // 0041e870 builds the zone name and 00e18b38 is cleared; the temporary is
    // released through the sized pool. Runs once per process.
    virtual void register_profiler_zone(const char* name) = 0;

    // Scene block. The light is game+19F8h and its shadow-map owner is
    // 00b7aab0, which returns light+174h.
    virtual bool shadow_map_owner_present() = 0;
    virtual bool render_camera_present() = 0; // game+19FCh
    // 00a8f3b0, __thiscall(owner, camera), RET 4. Builds the shadow view from
    // the render camera through 00b63f10 and 00b63b30.
    virtual void update_shadow_map_view_00a8f3b0() = 0;
    virtual void enqueue_frame_job(GameRenderJob job) = 0; // 00be3020, RET 8
    virtual void dispatch_frame_jobs() = 0; // 00be3150 with a zero argument
    virtual void render_interface_ortho_view_0059d7b0() = 0; // ECX = manager+54h
    virtual void render_interface_ortho_view_00535430() = 0; // ECX = manager+5Ch
    virtual void submit_world_view_00735b50() = 0; // ECX = 00e1ae90, unused by the body

    // Debug toggles.
    virtual bool input_action_pressed(int action) = 0; // inlined 004c43c0
    // 00bf94c6 on the formatted directory. The loop exits on a zero return, so
    // this returns true then. The CRT helper is not identified; a _mkdir that
    // returns 0 on success fits, but that reading is provisional.
    virtual bool create_screenshot_directory(const std::string& path) = 0;

    // 00e198ac +70h, and that object's +5 byte.
    virtual bool front_end_preview_active() = 0;
    // 00503510, ECX = 00e198ac+70h. Strings ColourRemap.tga and "Rotor %d=0x%x".
    virtual void render_front_end_preview_00503510() = 0;

    virtual void draw_gui_00aa45a0() = 0; // 004c12b0 then 00aa45a0
    virtual bool debug_overlay_present() = 0; // 00f871b4
    virtual void draw_debug_overlay_0078a430() = 0; // ECX = 00f871b4
    // 004c11f0 then 00b1ebe0, the whole of the render command queue. Everything
    // above this line is what the frame queued; the queue's own contract is
    // docs/RENDER_COMMAND_EXECUTION.md and bsp/render_command_queue.hpp.
    virtual void execute_render_command_queue() = 0;
    virtual void flush_diagnostic_sink_004c14c0() = 0; // result discarded
    virtual void draw_profiler_overlay_00be3bf0() = 0; // 004c1dd0 then 00be3bf0
    virtual void shadow_map_frame_end_00a8ac00() = 0; // trivial body in this image
    virtual bool scene_present() = 0; // game+19F0h
    virtual void scene_post_render() = 0; // *(scene+A8h) virtual +8h
    virtual void flush_render_resources_00b0d190() = 0; // ECX = 00f8d39c
    // 004c11f0 then 00b1ef60(camera), one stack argument: game+19FCh.
    virtual void queue_camera_finalize_00b1ef60() = 0;

    // --- 004ca1f0 ---
    // Profiler 004c1dd0, colour array at instance+24h indexed by the slot in
    // 0109db34, then 00be3640 and 00be3660 on that slot.
    virtual void profiler_set_finish_slot_color(std::uint32_t argb) = 0;
    virtual void profiler_begin_finish_counter() = 0;
    virtual void profiler_end_finish_counter() = 0;
    // Renderer virtual +14h -> 00b2f4a0 -> 00b2d8e0 with a zero argument.
    virtual void renderer_end_frame_default() = 0;
    // Renderer virtual +10h -> 00b2f4b0 -> the same 00b2d8e0 with the path.
    virtual void renderer_end_frame_capture(const std::string& path) = 0;
};

// ---------------------------------------------------------------------------
// Routines
// ---------------------------------------------------------------------------

// 004bfbf0, __cdecl, no arguments, RET. Increments the sequence before its
// first probe, so the current directory is never retried, and resets the image
// counter once a directory is taken.
void begin_screenshot_sequence_004bfbf0(ScreenshotState& state, GameRenderFrameHost& host);

// 004c9a00, __fastcall, one argument returned unchanged, RET. Formats the path
// for the current image index, then increments it.
std::string next_screenshot_path_004c9a00(ScreenshotState& state);

// 004c6c30, __thiscall(game), no stack arguments, plain RET, EAX is the result.
// A frame already begun reports success without touching the renderer.
bool try_begin_render_frame_004c6c30(GameRenderFrameState& state, GameRenderFrameHost& host);

// 004ca440, __thiscall(game), no stack arguments, plain RET, void. The whole
// body sits inside an SEH frame whose handler is 00c6557e; that unwind path is
// not modelled. Returns immediately unless the phase is kBegun.
void run_game_render(GameRenderFrameState& state, GameRenderFrameHost& host);

// 004ca1f0, __thiscall(game), no stack arguments, plain RET, void. The profiler
// counter is opened and closed unconditionally, outside the phase gate, so a
// frame that never rendered still pays for the counter. SEH handler 00c65530.
void finish_render_frame_004ca1f0(GameRenderFrameState& state, GameRenderFrameHost& host);
}
