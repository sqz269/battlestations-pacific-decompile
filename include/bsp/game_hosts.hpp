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

#include <cstdio>
#include <string>
#include <vector>

#include "bsp/app_bootstrap.hpp"
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
struct NativeRendererParametersOwner;
}

namespace bsp::game {

// Milestone 2a, defined in bsp/game_hosts_vfs.hpp. Held by pointer so the milestone-1 header
// stays independent of the VFS types.
class GameVfsHost;
class GameSettingsBinding;
class GameScriptHost;
class GameLocaleHost;
class GameFontHost;

// One host method, or one initialize phase, observed during a run.
struct GameHostMethodRecord {
    std::string method;          // "Interface::method" or "phase N <name>"
    std::string native_address;  // the native call site the method stands for
    unsigned long long calls{};
    bool implemented{};          // false marks the unimplemented-host policy
};

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
// which the bootstrap phase parses separately from GetCommandLineA.
struct GameExecutableOptions {
    long frame_limit{-1};   // --frames N; negative runs until the window is closed
    std::string log_path;   // --log <path>
    // --game-root <path>: SetCurrentDirectoryA before Init runs, so the phase-2 mount system
    // path stays the GetCurrentDirectoryA call at 0073d697 rather than an injected path.
    std::string game_root;
    // Explicit CSIDL_PERSONAL substitute for isolated settings runs.
    std::string settings_personal_root;
    // --vfs-probe <virtual path>, repeatable: resolve and read one path after phase 2 and
    // print its byte count.
    std::vector<std::string> vfs_probes;
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
    D3DPRESENT_PARAMETERS parameters_{};
    DWORD behavior_flags_{};
    HRESULT creation_result_{E_FAIL};
    unsigned long long presented_{};
};

// ApplicationFrameHost for 00737a50. The frame clock, the close policy and the two exit
// flags are concrete; the profiler, the game state, the input edge test, the game update,
// the VFS tick and the loading queue are the unimplemented policy.
class GameFrameHost final : public ApplicationFrameHost {
public:
    GameFrameHost(GameHostLog& log, FrameClock& clock, Win32PlatformState& platform,
        PlatformLoopState& loop)
        : log_(log), clock_(clock), platform_(platform), loop_(loop) {}

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

private:
    GameHostLog& log_;
    FrameClock& clock_;
    Win32PlatformState& platform_;
    PlatformLoopState& loop_;
    bool global_exit_{};
};

// PlatformLoopCallbacks for 00bec1a0. The frame callback is the reconstructed application
// frame followed by the device clear and present; pretranslation stands in for
// XLivePreTranslateMessage.
class GameLoopCallbacks final : public PlatformLoopCallbacks {
public:
    GameLoopCallbacks(GameHostLog& log, ApplicationFrameState& frame_state,
        FrameMarkerColor& color, GameFrameHost& frame_host, GameDeviceHost& device,
        PlatformLoopState& loop, long frame_limit)
        : log_(log), frame_state_(frame_state), color_(color), frame_host_(frame_host),
          device_(device), loop_(loop), frame_limit_(frame_limit) {}

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
};

// StartupHost for 008f81f0 plus everything the milestone runs inside
// application_initialize (0073d410) and application_shutdown (00737f30).
class GameStartupHost final : public StartupHost {
public:
    GameStartupHost(GameHostLog& log, HINSTANCE instance, const GameExecutableOptions& options);
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
    const ObjectHandleResolverSlots& object_handle_resolvers() const noexcept { return object_resolvers_; }
    NativeRendererParametersOwner* renderer_parameters() const noexcept { return renderer_parameters_; }

private:
    void run_initialize_phases();
    void release_platform_window() noexcept;

    GameHostLog& log_;
    HINSTANCE instance_{};
    GameExecutableOptions options_;

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
