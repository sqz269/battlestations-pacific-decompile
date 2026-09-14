#pragma once
// Process bindings for the reconstructed startup spine, milestone 1 of bsp_game.exe.
//
// Addresses: 008f81f0 WinMain, 0073d410 cSkeletonAppMidway::Init, 00becda0/00becee0 the
// Win32 platform object and its window, 00beb2c0 save storage, 00b2aeb0 the D3D9 device,
// 00bec1a0 the platform message loop, 00737a50 the application frame, 004ca2f0 the window
// close policy, 00737f30 the shutdown order.
//
// Nothing in this file is a reconstruction of native code. Every type is an integration
// binding that satisfies one of the host interfaces declared in bsp/winmain_startup.hpp,
// bsp/platform_window.hpp, bsp/platform_loop.hpp and bsp/app_frame.hpp with either a
// concrete Win32 / Direct3D 9 implementation or the explicit unimplemented policy in
// GameHostLog. An unimplemented method records its own name and the native call site it
// stands for, returns a neutral value, and never invents game behaviour.
//
// Evidence: docs/GAME_EXECUTABLE.md, docs/WINMAIN_STARTUP.md, docs/APP_INITIALIZE_MAP.md,
// docs/PLATFORM_LOOP.md, docs/WINDOW_CREATION.md, docs/WINDOW_CLOSE.md,
// docs/APP_RUN_FRAME.md, docs/D3D9_STARTUP.md, docs/APP_SHUTDOWN.md.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d9.h>

struct IGameExplorer;

#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "bsp/app_bootstrap.hpp"
#include "bsp/app_frame_game_state.hpp"
#include "bsp/game_settings.hpp"
#include "bsp/settings_capabilities.hpp"
#include "bsp/renderer_capabilities.hpp"
#include "bsp/profile_manager.hpp"
#include "bsp/app_frame.hpp"
#include "bsp/frame_clock.hpp"
#include "bsp/platform_loop.hpp"
#include "bsp/platform_window.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/winmain_startup.hpp"

namespace bsp {
struct NativeInputActionRecordCalls;
struct NativeRendererParametersOwner;
struct CameraAxesCrtAccess;
class XLiveLibrary;
}

namespace bsp::game {
class GameInputRuntime;

// One process-owned CRT dispatch state, shared by sound and recovered geometry.
const CameraAxesCrtAccess& application_camera_axes_crt() noexcept;

// Milestone 2a, defined in bsp/game_hosts_vfs.hpp. Held by pointer so the milestone-1 header
// stays independent of the VFS types.
class GameVfsHost;
class GameNativeReadOnlyData;
class GameSettingsBinding;
class GameScriptHost;
class GameLocaleHost;
class GameFontHost;
// Milestone 2b, defined in bsp/game_hosts_frontend.hpp.
class GameFrontendHost;
class GameSingletonHost;
// Milestone 2c, defined in bsp/game_hosts_menu.hpp and
// bsp/game_hosts_init_tail.hpp.
class GameMenuHost;
class GameFrameProfiler;
class GameDecalTable;

// One host method, or one initialize phase, observed during a run.
struct GameHostMethodRecord {
    std::string method;          // "Interface::method" or "phase N <name>"
    std::string native_address;  // the native call site the method stands for
    unsigned long long calls{};
    bool implemented{};          // false marks the unimplemented-host policy
};

// Milestone 2l: the x87 precision field as `_controlfp(0, 0) & _MCW_PC` reports
// it, named. docs/X87_CONTROL_WORD.md establishes statically that the CRT
// startup asks for 53 bits and that the Direct3D 9 device is created without
// D3DCREATE_FPU_PRESERVE, so d3d9.dll is expected to drop the field to 24; the
// executable reads it once before Direct3D exists and once at the first fixed
// simulation step so the claim rests on an observation rather than on the API's
// documentation. Neither read changes any arithmetic.
unsigned long x87_precision_field() noexcept;
const char* x87_precision_name(unsigned long precision_field) noexcept;

// The run log. Every host method the process reaches is recorded once with a call count,
// so a finished run states exactly which parts of the spine are concrete and which are
// standing in. Writes to a file when one is requested and always mirrors to stdout.
class GameHostLog {
public:
    GameHostLog() = default;
    GameHostLog(const GameHostLog&) = delete;
    GameHostLog& operator=(const GameHostLog&) = delete;
    ~GameHostLog();

    bool open(const std::string& path);
    void close();

    // Free-form progress line. Not a host method record.
    void note(const char* text);
    void notef(const char* format, ...);

    // A host method backed by a concrete implementation.
    void implemented(const char* method, const char* native_address);
    // The unimplemented-host policy: the method name and the native call site it stands
    // for are recorded, the caller receives a neutral value.
    void unimplemented(const char* method, const char* native_address);

    const std::vector<GameHostMethodRecord>& records() const noexcept { return records_; }
    std::size_t implemented_count() const noexcept;
    std::size_t unimplemented_count() const noexcept;

private:
    GameHostMethodRecord& record(const char* method, const char* native_address,
        bool implemented);
    void emit(const char* text);

    std::vector<GameHostMethodRecord> records_;
    std::FILE* file_{};
};

// Command line of bsp_game.exe itself. Unrelated to the native switch table at 0073ce20,
// which the bootstrap phase parses from WinMain's application mode argument.
struct GameExecutableOptions {
    long frame_limit{-1};   // --frames N; negative runs until the window is closed
    std::string log_path;   // --log <path>
    // --game-root <path>: SetCurrentDirectoryA before Init runs, so the phase-2 mount system
    // path stays the GetCurrentDirectoryA call at 0073d697 rather than an injected path.
    std::string game_root;
    // Explicit CSIDL_PERSONAL substitute for isolated settings runs.
    std::string settings_personal_root;
    // Optional absolute library selections, resolved before --game-root changes
    // CWD. Empty DLL paths select the original names in the current game root.
    std::wstring fmod_dll;
    std::wstring fmod_event_dll;
    std::wstring xlive_dll;
    std::wstring xinput_dll; // --xinput-dll; empty selects system XINPUT1_3.dll
    std::vector<std::wstring> xlive_dependencies;
    // --vfs-probe <virtual path>, repeatable: resolve and read one path after phase 2 and
    // print its byte count.
    std::vector<std::string> vfs_probes;
    // --press-start-frame N: inject the input-action edge the press-start page waits on
    // (action 4Eh through 004c43c0) on frame N. Negative injects nothing.
    long press_start_frame{-1};
    // --screenshot <path>: save the back buffer of the last frame as a PNG.
    std::string screenshot_path;
    // --screenshot-frame N: capture that frame instead of the last one, so a run can
    // photograph the title page before the injected press-start as well as the main
    // menu after it. Negative keeps the last-frame default.
    long screenshot_frame{-1};
    // --screenshot-mission-frame N, milestone 2h: capture the application frame
    // on which in-mission frame N of 004e4a40 ran, so the HUD can be
    // photographed over the mission without counting frames from the press.
    // Negative captures nothing extra; it takes precedence over
    // --screenshot-frame when both are given.
    long screenshot_mission_frame{-1};
    // --menu-select <mission id>: the mission the mission-tree screen's loader asks the
    // shell for at 00586150. Once the main menu is up the run publishes that selection
    // through 00580940, builds the mission-detail page 0058c010, takes the page's play
    // action 005922f0 and stops at the mission load request. Empty selects nothing.
    std::string menu_select;
    // --mission-frames N, milestone 2f: once --menu-select's load has run to
    // its end and 004da6c0 has entered game state 0Dh, run N frames of the
    // in-mission branch of 004e4a40 headless. Zero or negative runs none, which
    // leaves the run exactly where milestone 2e left it.
    long mission_frames{0};
    // --mission-complete-frame N, milestone 2g: on in-mission frame N the
    // executable makes the call a mission script's end-movie binding makes
    // (0089a480 -> 0089a390 -> 004cd390 with the debrief byte set), so the run
    // leaves state 0Dh through the recovered path instead of a frame count.
    // Negative injects nothing.
    long mission_complete_frame{-1};
    // --order-frame N and --order throttle=<f>,rudder=<f>, milestone 2i: on
    // in-mission frame N the executable issues one player order to the
    // controlled unit through the order record 00816a40 builds, which is the
    // same path the authored `Cruise` command takes. Negative issues nothing.
    long order_frame{-1};
    float order_throttle{0.0f};
    float order_rudder{0.0f};
    // Milestone 2m: --order speed=<m/s> makes the store luaMW_SetShipSpeed
    // 00890d30 makes on the controlled unit's navigator parameter block at
    // *(unit+73Ch). It is not a throttle: it is what makes the weapon
    // director's idle tail 00836e59 choose `cruise` over `stop`, and what
    // 009e12bd divides by the reference speed to get one.
    float order_speed{0.0f};
    bool order_speed_set{false};
    // Milestone 2l: --order may also name a command class instead of a pair.
    // `order_command` is the token 0046aab0 resolves against the 26-row registry
    // and `order_command_target` the `CommandTarget` name, which for the
    // position form `moveto=x,z` stays empty and the position is carried here.
    std::string order_command;
    std::string order_command_target;
    // Milestone 2n: --order-unit <name> names which created instance the
    // command form of --order is issued to. Empty keeps the controlled unit,
    // which is what every earlier milestone's run used. It exists because
    // 009f3dd0 replaces the director's current command with `cruise` for a
    // player-controlled unit at 009f3df3, so a command issued to the controlled
    // ship can never put its AI controller into any other state.
    std::string order_unit;
    // Milestone 2o: --ai-drive <name>=<throttle>,<rudder>. A LABELLED
    // DIAGNOSTIC STAND-IN, engaged on --order-frame like the player order.
    // Eight of the nine ship AI state steps have no reconstructed body, so on
    // each re-plan tick of the named unit the executable calls the two
    // recovered setters 009dbf90 and 009dffb0 on its control block with this
    // pair, and the rest of the chain - 009ed6b0, 009f4d10, 009f4da0's tail
    // into 009f3f80, the hop through 0080e170 / 0080e190, 00813020 and
    // 00825f20 - runs as the game's own routines. Empty drives nothing.
    std::string ai_drive_unit;
    float ai_drive_throttle{0.0f};
    float ai_drive_rudder{0.0f};
    bool order_command_position{false};
    float order_command_x{0.0f};
    float order_command_z{0.0f};
    // --mission-frame-seconds S, milestone 2i: run each in-mission frame with a
    // fixed delta instead of the wall clock, so a headless run accumulates
    // simulated time deterministically and the fixed-step driver's own clock
    // does not depend on how fast the machine presents. Zero keeps the wall
    // clock, which is what every earlier milestone's run used.
    float mission_frame_seconds{0.0f};
    // --trajectory-csv <path>, milestone 2j: one row per unit per fixed
    // simulation step, so an external comparison against a trace taken from the
    // running game can be made. Empty writes nothing. The column contract is in
    // include/bsp/game_hosts_trajectory.hpp.
    std::string trajectory_csv;
    // --hardware-probe-commit: let the phase-2 probe 0073c3b0 raise its message box and
    // write the machine profile back to HKLM. Off by default so an unattended run cannot
    // block on a dialog or rewrite a machine's stored profile.
    bool hardware_probe_commit{false};
    bool parse(int argc, char** argv, std::string& error);
};

// PlatformWindowHost, every method a direct Win32 call. 00becee0 issues these itself in
// the original; the reconstruction routes them through the host so the routine can run
// against a recording double, and this binding puts the real calls back.
class GameWindowHost final : public PlatformWindowHost {
public:
    GameWindowHost(GameHostLog& log, HINSTANCE instance) : log_(log), instance_(instance) {}
    void stop_existing_window(Win32PlatformState& state) override;
    HCURSOR load_arrow_cursor() override;
    ATOM register_class(const WNDCLASSA& window_class) override;
    BOOL adjust_window_rect(RECT& rectangle, DWORD style, BOOL menu) override;
    HWND create_window(DWORD extended_style, const char* class_name, const char* title,
        DWORD style, int x, int y, int width, int height, HINSTANCE instance,
        void* parameter) override;
    void set_window_long(HWND window, int index, LONG value) override;
    void set_window_pos(HWND window, HWND insert_after, int x, int y, int width, int height,
        UINT flags) override;
    BOOL desktop_client_rect(RECT& rectangle) override;
    int window_color_depth(HWND window) override;
    void show_window(HWND window, int command) override;
    ATOM registered_class() const noexcept { return registered_class_; }
    void unregister_class(const char* name) noexcept;

private:
    GameHostLog& log_;
    HINSTANCE instance_{};
    ATOM registered_class_{};
};

// SaveStorageHost for 00beb2c0: SHGetSpecialFolderPathA plus CreateDirectoryA.
class GameSaveStorageHost final : public SaveStorageHost {
public:
    explicit GameSaveStorageHost(GameHostLog& log) : log_(log) {}
    bool special_folder_path(int folder, std::string& path) override;
    bool create_directory(const std::string& path) override;

private:
    GameHostLog& log_;
};

// Device ownership for the milestone. Creation is the reconstructed prefix of 00b2aeb0;
// the per-frame clear and present are direct Direct3D 9 calls, because the native
// renderer frame routine behind renderer virtual +20h is not reconstructed.
class GameDeviceHost {
public:
    // The application's renderer API outlives this device, settings queries and
    // any device recreation. This object releases only the device it creates.
    GameDeviceHost(GameHostLog& log, IDirect3D9& api, NativeRendererParametersOwner& parameters)
        : log_(log), api_(api), renderer_parameters_(parameters) {}
    GameDeviceHost(const GameDeviceHost&) = delete;
    GameDeviceHost& operator=(const GameDeviceHost&) = delete;
    ~GameDeviceHost();

    bool create(const RendererInitRequest& request);
    // Clears to the milestone background and presents. Counts one presented frame.
    bool clear_and_present();
    // Milestone 2c: runs once, between EndScene and Present of the next frame, so a
    // capture sees the finished back buffer. Executable plumbing, not a native routine.
    void request_capture(std::function<void(IDirect3DDevice9&)> capture);
    // Milestone 2b: what the milestone's own present draws between BeginScene and
    // EndScene. The renderer frame routine behind renderer virtual +20h is still not
    // reconstructed, so anything installed here is an executable-side bridge, not a
    // recovered draw path.
    void set_overlay(std::function<void(IDirect3DDevice9&)> overlay);
    void release();

    bool created() const noexcept { return device_ != nullptr; }
    HRESULT creation_result() const noexcept { return creation_result_; }
    unsigned long long presented() const noexcept { return presented_; }
    const D3DPRESENT_PARAMETERS& parameters() const noexcept { return parameters_; }
    IDirect3DDevice9* device() const noexcept { return device_; }
    IDirect3D9& renderer_api() noexcept { return api_; }

private:
    GameHostLog& log_;
    IDirect3D9& api_;
    NativeRendererParametersOwner& renderer_parameters_;
    IDirect3DDevice9* device_{};
    std::function<void(IDirect3DDevice9&)> overlay_;
    std::function<void(IDirect3DDevice9&)> capture_;
    D3DPRESENT_PARAMETERS parameters_{};
    DWORD behavior_flags_{};
    HRESULT creation_result_{E_FAIL};
    unsigned long long presented_{};
};

// ApplicationFrameHost for 00737a50. Milestone 2c turns six of its methods concrete: the
// game-state field *(00e188a8)+5D4h becomes a real read over the slot the drain advances,
// the four profiler methods run the reconstructed counter pair, and the front-end branch
// of the game update runs. The VFS tick and the loading queue are still the unimplemented
// policy, and so is the rest of GGame::OnMove.
class GameFrameHost final : public ApplicationFrameHost {
public:
    GameFrameHost(GameHostLog& log, FrameClock& clock, Win32PlatformState& platform,
        PlatformLoopState& loop, GameStateSlot& game_state, GameFrameProfiler* profiler,
        GameMenuHost* menu)
        : log_(log), clock_(clock), platform_(platform), loop_(loop),
          game_state_(game_state), profiler_(profiler), menu_(menu) {}

    void profiler_set_frame_slot_color(std::uint32_t argb) override;
    void profiler_begin_frame_slot() override;
    int game_state() override;
    bool input_action_pressed(int action) override;
    void advance_frame_clock() override;
    const ClockTimestamp& frame_interval() override;
    void game_on_move(float seconds) override;
    bool exit_requested() override;
    void request_loop_exit() override;
    void tick_vfs_providers() override;
    void update_loading_queue() override;
    void profiler_end_frame_slot() override;
    void profiler_end_frame() override;

    // Global exit byte 00e1ae75, set by the close policy at 004ca2f0.
    bool global_exit() const noexcept { return global_exit_; }
    unsigned long long frames() const noexcept { return frame_index_; }
    // Milestone 2h, --screenshot-mission-frame N: how many in-mission frames of
    // 004e4a40 have run. Zero while the run is still in the front end.
    unsigned long long mission_frames_run() const noexcept;

private:
    GameHostLog& log_;
    FrameClock& clock_;
    Win32PlatformState& platform_;
    PlatformLoopState& loop_;
    GameStateSlot& game_state_;
    GameFrameProfiler* profiler_{};
    GameMenuHost* menu_{};
    unsigned long long frame_index_{};
    bool global_exit_{};
};

// PlatformLoopCallbacks for 00bec1a0. The frame callback is the reconstructed application
// frame followed by the device clear and present. Pretranslation calls the
// same selected XLive library used by the load-time message service.
class GameLoopCallbacks final : public PlatformLoopCallbacks {
public:
    GameLoopCallbacks(GameHostLog& log, ApplicationFrameState& frame_state,
        FrameMarkerColor& color, GameFrameHost& frame_host, GameDeviceHost& device,
        PlatformLoopState& loop, long frame_limit, XLiveLibrary& xlive,
        std::function<void(IDirect3DDevice9&)> capture = {}, long screenshot_frame = -1,
        long screenshot_mission_frame = -1)
        : log_(log), frame_state_(frame_state), color_(color), frame_host_(frame_host),
          device_(device), loop_(loop), frame_limit_(frame_limit), xlive_(xlive),
          capture_(std::move(capture)), screenshot_frame_(screenshot_frame),
          screenshot_mission_frame_(screenshot_mission_frame) {}

    bool pretranslate(MSG& message) override;
    void frame() override;

    unsigned long long frames() const noexcept { return frames_; }

private:
    GameHostLog& log_;
    ApplicationFrameState& frame_state_;
    FrameMarkerColor& color_;
    GameFrameHost& frame_host_;
    GameDeviceHost& device_;
    PlatformLoopState& loop_;
    long frame_limit_{-1};
    XLiveLibrary& xlive_;
    std::function<void(IDirect3DDevice9&)> capture_;
    // --screenshot-frame N, milestone 2d: the frame index to capture. Negative keeps the
    // last-frame (or close-request) rule the switch shipped with.
    long screenshot_frame_{-1};
    // --screenshot-mission-frame N, milestone 2h: the in-mission frame of
    // 004e4a40 to capture, so a run can photograph the HUD over the mission
    // without counting application frames back from the injected press.
    long screenshot_mission_frame_{-1};
    bool capture_requested_{};
    unsigned long long frames_{};
};

// What one run produced, for the report.
struct GameRunSummary {
    bool window_created{};
    bool device_created{};
    HRESULT device_result{E_FAIL};
    unsigned int back_buffer_width{};
    unsigned int back_buffer_height{};
    unsigned long long frames_presented{};
    bool loop_finished{};
    int exit_code{};
    // Milestone 2a. Phase 2 (00beda60, 00be1890, 0073cb10) and phase 5 (008d8190).
    bool vfs_ready{};
    std::size_t mounts_created{};
    std::size_t mounts_requested{};
    std::size_t package_entries{};
    std::size_t package_mounts{};
    bool cached_load{};
    bool options_file_present{};
    std::string options_path;
    std::string language;
    int settings_width{};
    int settings_height{};
    bool settings_fullscreen{};
    bool settings_vsync{};
    int settings_antialias{};
    std::size_t probes_resolved{};
    std::size_t probes_requested{};
    bool input_scripts_ready{};
    std::size_t input_devices{};
    std::size_t input_names{};
    std::size_t controller_names{};
    bool renderer_api_shared{};
    std::size_t locale_keys{};
    std::size_t locale_files{};
    std::size_t fonts_loaded{};
    std::size_t font_resource_opens{};
    std::size_t fingerprint_defined_bytes{};
    // Milestone 2b. Phase 7 (0073bae0, 00aa5e20) and the title pages (00aa5840).
    std::size_t gui_resources_acquired{};
    std::size_t gui_pages_loaded{};
    std::size_t gui_pages_requested{};
    std::size_t gui_widgets{};
    std::size_t gui_widgets_with_texture{};
    // The sprite bridge, which is not a reconstruction of the native GUI draw path.
    bool gui_bridge_open{};
    std::string gui_bridge_atlas;
    std::size_t gui_bridge_atlas_items{};
    std::size_t gui_bridge_textures{};
    std::size_t gui_bridge_quads{};
    unsigned long long gui_bridge_frames{};
    // Milestone 2c. The Init tail (0073c3b0, 0073d94f-0073d98d, 0073db41-0073db69,
    // 00740840) and the front end (004c9a70, 004f8830, 004e4000, 004c40f0).
    bool hardware_probe_ran{};
    int hardware_profile_values{};
    std::size_t provider_factories{};
    std::size_t resource_parsers{};
    bool pak_registry{};
    bool pak_lock{};
    std::size_t decal_definitions{};
    bool title_init_ran{};
    bool press_start_registered{};
    int final_game_state{};
    unsigned long long pump_frames{};
    std::size_t screen_enters{};
    std::size_t screen_exits{};
    std::size_t visibility_commits{};
    long press_start_frame{-1};
    bool press_start_injected{};
    bool shell_entered{};
    bool main_menu_manager_active{};
    int published_screen_id{};
    std::string path_step;
    std::size_t screens_registered{};
    std::size_t screen_owned_pages{};
    bool screenshot_written{};
    std::string screenshot_path;
    long screenshot_frame{-1};
    // Milestone 2e, the mission path. The whole record is in
    // reports/game_executable_milestone_2e.json; these are the summary lines.
    std::string menu_select;
    bool mission_tree_loaded{};
    std::size_t mission_tree_groups{};
    std::size_t mission_tree_missions{};
    std::string mission_selected_id;
    int mission_list_page{};
    bool mission_detail_built{};
    bool mission_start_requested{};
    bool mission_scene_record{};
    std::string mission_scene_path;
    std::size_t mission_scene_entities{};
    std::size_t mission_scene_classes{};
    std::size_t mission_load_host_steps{};
    std::string mission_load_stopped_at;
    std::string mission_step;
    // Milestone 2f, the load past the renderer owners, the mission Lua machine
    // and the headless in-mission frames.
    bool mission_load_finished{};
    std::size_t mission_load_concrete{};
    std::size_t mission_load_records{};
    int mission_game_state{};
    bool mission_entered{};
    long mission_frames_requested{};
    unsigned long long mission_frames_run{};
    unsigned long long mission_frames_simulated{};
    // Milestone 2g: the frames after game state 0Dh and how the path ended.
    unsigned long long mission_exit_frames{};
    bool mission_exit_completed{};
    bool mission_complete_injected{};
    std::size_t mission_lua_bindings{};
    std::size_t mission_lua_natives{};
    unsigned long long mission_lua_native_calls{};
    std::string mission_script_path;
    std::string mission_exit_note;
    // Milestone 2d, the text half of the sprite bridge.
    bool text_bridge_open{};
    std::size_t text_widgets{};
    std::size_t text_runs{};
    std::size_t text_glyphs{};
    std::size_t text_quads{};
};

// StartupHost for 008f81f0 plus everything the milestone runs inside
// application_initialize (0073d410) and application_shutdown (00737f30).
class GameStartupHost final : public StartupHost {
public:
    GameStartupHost(GameHostLog& log, HINSTANCE instance, const GameExecutableOptions& options,
        GameNativeReadOnlyData* native_data = nullptr);
    ~GameStartupHost() override;

    long com_initialize() override;
    long com_initialize_security() override;
    void com_uninitialize() override;
    void current_directory(char* buffer, unsigned long capacity) override;
    bool game_explorer_create() override;
    bool game_explorer_verify_access(const wchar_t* gdf_binary_path) override;
    void game_explorer_release() override;
    void exit_process(int code) override;
    void random_threads_initialize() override;
    void* allocate_thread_slot() override;
    void random_threads_register_current() override;
    void random_threads_unregister_current() override;
    void release_thread_slot(void* slot) override;
    void random_threads_shutdown() override;
    SingleInstanceMutex create_single_instance_mutex(const char* name) override;
    void close_mutex(void* handle) override;
    std::string resolve_language() override;
    void error_message_box(const wchar_t* text, const wchar_t* caption) override;
    void set_thread_affinity_to_first_processor() override;
    void publish_game_resource_factory() override;
    void application_construct() override;
    void application_initialize(int flags, const char* mode) override;
    void platform_run_loop_dispatch() override;
    void application_shutdown() override;
    void application_destruct() override;
    void destroy_singleton_lifetime_manager() override;

    const GameRunSummary& summary() const noexcept { return summary_; }
    // Milestone 2a: the phase-2 provider manager, alive for the whole run, and the settings
    // phase 5 loaded. Null and default respectively when initialize did not reach them.
    GameVfsHost* vfs() const noexcept { return vfs_; }
    const GameSettings& settings() const noexcept { return settings_.options_file; }
    const GameSettingsBlock& settings_block() const noexcept { return settings_; }
    GameSettingsBinding* settings_binding() const noexcept { return settings_host_; }
    GameScriptHost* script_host() const noexcept { return scripts_; }
    GameLocaleHost* locale_host() const noexcept { return locale_; }
    GameFontHost* font_host() const noexcept { return fonts_; }
    GameFrontendHost* frontend_host() const noexcept { return frontend_; }
    GameMenuHost* menu_host() const noexcept { return menu_; }
    GameDecalTable* decal_table() const noexcept { return decals_; }
    const ObjectHandleResolverSlots& object_handle_resolvers() const noexcept { return object_resolvers_; }
    NativeRendererParametersOwner* renderer_parameters() const noexcept { return renderer_parameters_; }
    GameInputRuntime* input_runtime() const noexcept { return input_runtime_; }
    void* volatile& input_backend_publication() noexcept { return input_backend_00f8bbf4_; }

private:
    void run_initialize_phases(const char* mode);
    void release_platform_window() noexcept;
    struct SoundServices;
    std::unique_ptr<SoundServices> sound_;
    struct InputServices;
    std::unique_ptr<InputServices> input_;
    void* volatile input_backend_00f8bbf4_{};
    void* volatile input_actions_00f8bbf8_{};
    GameInputRuntime* volatile input_runtime_{}; // nonowning source service publication
    NativeInputActionRecordCalls* volatile input_listener_calls_{};
    FrameClock* volatile clock_publication_01090ab0_{};

    GameHostLog& log_;
    HINSTANCE instance_{};
    GameExecutableOptions options_;
    // The entrypoint retains adopted numeric data beyond this host's drain.
    GameNativeReadOnlyData* native_data_{};

    // 008F823D: actual CoCreateInstance output, released explicitly at 008F82CA.
    // The denied branch calls CRT exit before this release; no destructor cleanup.
    IGameExplorer* game_explorer_{};

    RandomThreads* random_threads_{};
    Win32PlatformState platform_;
    PlatformLoopState loop_;
    FrameClock clock_;
    ObjectHandleResolverSlots object_resolvers_;
    ApplicationFrameState frame_state_;
    FrameMarkerColor frame_color_;
    SaveStorageRoots save_roots_;
    RendererInitRequest renderer_request_;
    GameWindowHost* window_host_{};
    GameDeviceHost* device_{};
    GameFrameHost* frame_host_{};
    GameLoopCallbacks* loop_callbacks_{};
    GameVfsHost* vfs_{};
    GameSettingsBlock settings_;
    ProfileHintsOwner profile_hints_;
    std::vector<std::string> content_suffixes_;
    GameSettingsBinding* settings_host_{};
    GameScriptHost* scripts_{};
    GameLocaleHost* locale_{};
    GameFontHost* fonts_{};
    GameFrontendHost* frontend_{};
    // Milestone 2c.
    GameStateSlot game_state_;          // *(00e188a8)+5D4h
    GameFrameProfiler* profiler_{};     // 004c1dd0's counter arrays
    GameMenuHost* menu_{};              // the front-end registry and the main-menu path
    GameSingletonHost* singletons_{};   // survives menu teardown through 008F8449
    GameDecalTable* decals_{};          // phase 9, 00740840
    IDirect3D9* renderer_api_{};
    NativeRendererParametersOwner* renderer_parameters_{};
    SettingsRendererCapabilities renderer_capabilities_;
    RendererCapabilities renderer_full_capabilities_;
    std::string window_class_name_;
    GameRunSummary summary_;
    bool constructed_{};
};

// The window procedure. Only the WM_CLOSE branch of the native handler 00bed3b0 is
// recovered as a policy (docs/WINDOW_CLOSE.md): it records a pending close at platform
// +180h and returns zero. Every other message goes to DefWindowProcA, because the
// activation, resize, input and sizing-loop branches of 00bed3b0 are not reconstructed.
LRESULT CALLBACK game_window_procedure(HWND window, UINT message, WPARAM wparam,
    LPARAM lparam);

// The platform object the window procedure writes its close request into. 00bed3b0 keeps
// it in window-extra offset zero during WM_CREATE; the milestone uses one process-wide
// binding instead, because the extra-bytes layout of the native object is not recovered.
void set_active_platform_state(Win32PlatformState* state) noexcept;

}  // namespace bsp::game
