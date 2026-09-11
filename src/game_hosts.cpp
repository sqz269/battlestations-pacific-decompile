// Process bindings for the reconstructed startup spine. See include/bsp/game_hosts.hpp
// and docs/GAME_EXECUTABLE.md. No native behaviour is invented here: whatever is not
// reconstructed is routed through GameHostLog::unimplemented with its native call site.
#include "bsp/game_hosts.hpp"
#include "bsp/settings_initial_state.hpp"
#include "bsp/lua_runtime_globals.hpp"
#include <stdexcept>

#include <objbase.h>
#include <gameux.h>
#include <shlobj.h>

#include <cstdarg>
#include <cstdlib>
#include <cstring>

#include "bsp/app_bootstrap.hpp"
#include "bsp/d3d9_startup.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_hosts_fonts.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/fingerprint_payload.hpp"
#include "bsp/native_renderer_parameters.hpp"
#include "bsp/physical_file.hpp"
#include "bsp/renderer_startup.hpp"

namespace bsp::game {
namespace {

// 00bed3b0 keeps the platform object in window-extra offset zero. The extra-bytes layout
// of the native object is not recovered, so the milestone binds one process-wide pointer.
Win32PlatformState* g_active_platform = nullptr;

// Window title and class name, the temporary string 00becee0 receives as argument 2.
const char kWindowName[] = "Battlestations Pacific";

// Clear colour of the milestone frame. Not a recovered value: the native renderer frame
// routine behind renderer virtual +20h is not reconstructed.
const D3DCOLOR kMilestoneClearColor = D3DCOLOR_ARGB(255, 12, 24, 48);

std::string trim_copy(const std::string& text) {
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && (text[begin] == ' ' || text[begin] == '\t' || text[begin] == '\r'
        || text[begin] == '\n')) {
        ++begin;
    }
    while (end > begin && (text[end - 1] == ' ' || text[end - 1] == '\t'
        || text[end - 1] == '\r' || text[end - 1] == '\n')) {
        --end;
    }
    return text.substr(begin, end - begin);
}

// Token split matching the scan at 008f8010..008f808b: whitespace separated words, in
// file order, so the first "Language" consumes the following token.
std::vector<std::string> split_option_tokens(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;
    for (const char character : text) {
        if (character == ' ' || character == '\t' || character == '\r' || character == '\n') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(character);
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

}  // namespace

void set_active_platform_state(Win32PlatformState* state) noexcept {
    g_active_platform = state;
}

LRESULT CALLBACK game_window_procedure(HWND window, UINT message, WPARAM wparam,
    LPARAM lparam) {
    // docs/WINDOW_CLOSE.md step 1: WM_CLOSE records a pending close at platform+180h and
    // returns zero. It does not set the loop exit byte +181h.
    if (message == WM_CLOSE) {
        if (g_active_platform != nullptr) g_active_platform->close_requested = true;
        return 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

// ---------------------------------------------------------------------------
// GameHostLog
// ---------------------------------------------------------------------------

GameHostLog::~GameHostLog() { close(); }

bool GameHostLog::open(const std::string& path) {
    close();
    if (path.empty()) return true;
    file_ = nullptr;
    const errno_t status = fopen_s(&file_, path.c_str(), "w");
    if (status != 0 || file_ == nullptr) {
        file_ = nullptr;
        return false;
    }
    return true;
}

void GameHostLog::close() {
    if (file_ != nullptr) {
        std::fclose(file_);
        file_ = nullptr;
    }
}

void GameHostLog::emit(const char* text) {
    std::printf("%s\n", text);
    if (file_ != nullptr) {
        std::fprintf(file_, "%s\n", text);
        std::fflush(file_);
    }
}

void GameHostLog::note(const char* text) { emit(text); }

void GameHostLog::notef(const char* format, ...) {
    char buffer[1024];
    va_list arguments;
    va_start(arguments, format);
    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, arguments);
    va_end(arguments);
    emit(buffer);
}

GameHostMethodRecord& GameHostLog::record(const char* method, const char* native_address,
    bool implemented) {
    for (auto& entry : records_) {
        if (entry.method == method) {
            ++entry.calls;
            return entry;
        }
    }
    records_.push_back(GameHostMethodRecord{method, native_address, 1ull, implemented});
    return records_.back();
}

void GameHostLog::implemented(const char* method, const char* native_address) {
    const GameHostMethodRecord& entry = record(method, native_address, true);
    if (entry.calls == 1ull) notef("host %s [%s] concrete", method, native_address);
}

void GameHostLog::unimplemented(const char* method, const char* native_address) {
    const GameHostMethodRecord& entry = record(method, native_address, false);
    if (entry.calls == 1ull) {
        notef("host %s [%s] UNIMPLEMENTED, returning a neutral value", method,
            native_address);
    }
}

std::size_t GameHostLog::implemented_count() const noexcept {
    std::size_t total = 0;
    for (const auto& entry : records_) {
        if (entry.implemented) ++total;
    }
    return total;
}

std::size_t GameHostLog::unimplemented_count() const noexcept {
    return records_.size() - implemented_count();
}

// ---------------------------------------------------------------------------
// GameExecutableOptions
// ---------------------------------------------------------------------------

bool GameExecutableOptions::parse(int argc, char** argv, std::string& error) {
    for (int index = 1; index < argc; ++index) {
        const char* argument = argv[index];
        if (std::strcmp(argument, "--frames") == 0) {
            if (index + 1 >= argc) {
                error = "--frames needs a count";
                return false;
            }
            frame_limit = std::strtol(argv[++index], nullptr, 10);
            if (frame_limit < 0) {
                error = "--frames needs a non-negative count";
                return false;
            }
        } else if (std::strcmp(argument, "--log") == 0) {
            if (index + 1 >= argc) {
                error = "--log needs a path";
                return false;
            }
            log_path = argv[++index];
        } else if (std::strcmp(argument, "--game-root") == 0) {
            if (index + 1 >= argc) {
                error = "--game-root needs a directory";
                return false;
            }
            game_root = argv[++index];
        } else if (std::strcmp(argument, "--settings-personal-root") == 0) {
            if (index + 1 >= argc) {
                error = "--settings-personal-root needs a directory";
                return false;
            }
            // Resolve before --game-root changes CWD; an isolated output path
            // must not accidentally become relative to the original installation.
            const char* input = argv[++index];
            const DWORD required = GetFullPathNameA(input, 0, nullptr, nullptr);
            if (!required) { error = "cannot resolve settings personal root"; return false; }
            settings_personal_root.resize(required);
            const DWORD length = GetFullPathNameA(input, required, settings_personal_root.data(), nullptr);
            if (!length || length >= required) { error = "cannot resolve settings personal root"; return false; }
            settings_personal_root.resize(length);
        } else if (std::strcmp(argument, "--vfs-probe") == 0) {
            if (index + 1 >= argc) {
                error = "--vfs-probe needs a virtual path";
                return false;
            }
            vfs_probes.emplace_back(argv[++index]);
        } else {
            error = std::string("unknown option ") + argument;
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// GameWindowHost, the Win32 calls 00becee0 issues inline
// ---------------------------------------------------------------------------

void GameWindowHost::stop_existing_window(Win32PlatformState& state) {
    // Platform vtable +8. The native teardown is not reconstructed; the milestone never
    // reaches this path because it configures the window once.
    log_.unimplemented("PlatformWindowHost::stop_existing_window", "00becda0+vtable08");
    static_cast<void>(state);
}

HCURSOR GameWindowHost::load_arrow_cursor() {
    log_.implemented("PlatformWindowHost::load_arrow_cursor", "00becf0f");
    return LoadCursorA(nullptr, IDC_ARROW);
}

ATOM GameWindowHost::register_class(const WNDCLASSA& window_class) {
    log_.implemented("PlatformWindowHost::register_class", "00becf80");
    registered_class_ = RegisterClassA(&window_class);
    return registered_class_;
}

void GameWindowHost::unregister_class(const char* name) noexcept {
    if (registered_class_ != 0 && UnregisterClassA(name, instance_)) registered_class_ = 0;
}

BOOL GameWindowHost::adjust_window_rect(RECT& rectangle, DWORD style, BOOL menu) {
    log_.implemented("PlatformWindowHost::adjust_window_rect", "00becfd6");
    return AdjustWindowRect(&rectangle, style, menu);
}

HWND GameWindowHost::create_window(DWORD extended_style, const char* class_name,
    const char* title, DWORD style, int x, int y, int width, int height, HINSTANCE instance,
    void* parameter) {
    log_.implemented("PlatformWindowHost::create_window", "00bed01d");
    return CreateWindowExA(extended_style, class_name, title, style, x, y, width, height,
        nullptr, nullptr, instance, parameter);
}

void GameWindowHost::set_window_long(HWND window, int index, LONG value) {
    log_.implemented("PlatformWindowHost::set_window_long", "00bed0a4");
    SetWindowLongA(window, index, value);
}

void GameWindowHost::set_window_pos(HWND window, HWND insert_after, int x, int y, int width,
    int height, UINT flags) {
    log_.implemented("PlatformWindowHost::set_window_pos", "00bed0c9");
    SetWindowPos(window, insert_after, x, y, width, height, flags);
}

BOOL GameWindowHost::desktop_client_rect(RECT& rectangle) {
    log_.implemented("PlatformWindowHost::desktop_client_rect", "00bed0f3");
    return GetClientRect(GetDesktopWindow(), &rectangle);
}

int GameWindowHost::window_color_depth(HWND window) {
    log_.implemented("PlatformWindowHost::window_color_depth", "00bed16b");
    const HDC context = GetDC(window);
    if (context == nullptr) return 0;
    const int depth = GetDeviceCaps(context, BITSPIXEL);
    ReleaseDC(window, context);
    return depth;
}

void GameWindowHost::show_window(HWND window, int command) {
    log_.implemented("PlatformWindowHost::show_window", "00bed19c");
    ShowWindow(window, command);
    UpdateWindow(window);
}

// ---------------------------------------------------------------------------
// GameSaveStorageHost, 00beb2c0
// ---------------------------------------------------------------------------

bool GameSaveStorageHost::special_folder_path(int folder, std::string& path) {
    log_.implemented("SaveStorageHost::special_folder_path", "00beb2f3");
    char buffer[MAX_PATH] = {};
    if (!SHGetSpecialFolderPathA(nullptr, buffer, folder, TRUE)) return false;
    path.assign(buffer);
    return true;
}

bool GameSaveStorageHost::create_directory(const std::string& path) {
    log_.implemented("SaveStorageHost::create_directory", "00beb35e");
    return CreateDirectoryA(path.c_str(), nullptr) != 0
        || GetLastError() == ERROR_ALREADY_EXISTS;
}

// ---------------------------------------------------------------------------
// GameDeviceHost, 00b2aeb0
// ---------------------------------------------------------------------------

GameDeviceHost::~GameDeviceHost() { release(); }

bool GameDeviceHost::create(const RendererInitRequest& request) {
    release();

    RendererDisplaySettings settings{};
    settings.width = static_cast<std::uint32_t>(request.width);
    settings.height = static_cast<std::uint32_t>(request.height);
    settings.fullscreen = request.fullscreen;
    const D3D9StartupOptions options = renderer_present_request_00becee0(settings,
        request.window);

    log_.implemented("RendererHost::create_device", "00b2aeb0");
    creation_result_ = d3d9_create_device_prefix_00b2aeb0(api_, options,
        renderer_parameters_, parameters_, behavior_flags_, device_);
    if (FAILED(creation_result_) || device_ == nullptr) {
        log_.notef("device creation failed hr=0x%08lx",
            static_cast<unsigned long>(creation_result_));
        return false;
    }
    log_.notef("device created hr=0x%08lx flags=0x%lx size=%ux%u windowed=%d format=%u "
        "depth=%u interval=0x%x", static_cast<unsigned long>(creation_result_),
        behavior_flags_, parameters_.BackBufferWidth, parameters_.BackBufferHeight,
        parameters_.Windowed, static_cast<unsigned>(parameters_.BackBufferFormat),
        static_cast<unsigned>(parameters_.AutoDepthStencilFormat),
        parameters_.PresentationInterval);
    return true;
}

bool GameDeviceHost::clear_and_present() {
    if (device_ == nullptr) {
        log_.unimplemented("RendererHost::present_frame", "00b32410+vtable20");
        return false;
    }
    // Direct Direct3D 9 calls: the native renderer frame routine is not reconstructed,
    // so this is the milestone's own clear and present, not a recovered sequence.
    log_.implemented("RendererHost::clear_and_present", "milestone");
    HRESULT result = device_->Clear(0, nullptr,
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, kMilestoneClearColor, 1.0f, 0);
    if (SUCCEEDED(result)) result = device_->BeginScene();
    if (SUCCEEDED(result) && overlay_) overlay_(*device_);
    if (SUCCEEDED(result)) result = device_->EndScene();
    if (SUCCEEDED(result)) result = device_->Present(nullptr, nullptr, nullptr, nullptr);
    if (FAILED(result)) {
        log_.notef("present failed hr=0x%08lx", static_cast<unsigned long>(result));
        return false;
    }
    ++presented_;
    return true;
}

void GameDeviceHost::set_overlay(std::function<void(IDirect3DDevice9&)> overlay) {
    overlay_ = std::move(overlay);
}

void GameDeviceHost::release() {
    overlay_ = nullptr;
    if (device_ != nullptr) {
        device_->Release();
        device_ = nullptr;
    }
}

// ---------------------------------------------------------------------------
// GameFrameHost, 00737a50
// ---------------------------------------------------------------------------

void GameFrameHost::profiler_set_frame_slot_color(std::uint32_t argb) {
    log_.unimplemented("ApplicationFrameHost::profiler_set_frame_slot_color", "004c1dd0");
    static_cast<void>(argb);
}

void GameFrameHost::profiler_begin_frame_slot() {
    log_.unimplemented("ApplicationFrameHost::profiler_begin_frame_slot", "00be3640");
}

int GameFrameHost::game_state() {
    // *(00e188a8)+5D4h. The game object does not exist in this milestone.
    log_.unimplemented("ApplicationFrameHost::game_state", "00e188a8+5d4");
    return 0;
}

bool GameFrameHost::input_action_pressed(int action) {
    log_.unimplemented("ApplicationFrameHost::input_action_pressed", "004c43c0");
    static_cast<void>(action);
    return false;
}

void GameFrameHost::advance_frame_clock() {
    log_.implemented("ApplicationFrameHost::advance_frame_clock", "00bedc30");
    update_frame_clock_00bedc30(clock_);
}

const ClockTimestamp& GameFrameHost::frame_interval() {
    log_.implemented("ApplicationFrameHost::frame_interval", "00bee070");
    return clock_.interval;
}

void GameFrameHost::game_on_move(float seconds) {
    // 004e4a40 itself is not reconstructed. One of its callees is: the window close
    // processor at 004ca2f0 (docs/WINDOW_CLOSE.md), whose front-end branch reads and
    // clears platform+180h and sets global exit 00e1ae75. The confirmation-dialog branch
    // taken for other game states is not reconstructed.
    log_.unimplemented("ApplicationFrameHost::game_on_move", "004e4a40");
    static_cast<void>(seconds);
    if (platform_.close_requested) {
        platform_.close_requested = false;
        log_.implemented("CloseRequestPolicy::front_end_branch", "004ca2f0");
        global_exit_ = true;
    }
}

bool GameFrameHost::exit_requested() {
    log_.implemented("ApplicationFrameHost::exit_requested", "00e1ae75");
    return global_exit_;
}

void GameFrameHost::request_loop_exit() {
    log_.implemented("ApplicationFrameHost::request_loop_exit", "0109cf04+181");
    platform_.exit_requested = true;
    loop_.exit_requested = true;
}

void GameFrameHost::tick_vfs_providers() {
    log_.unimplemented("ApplicationFrameHost::tick_vfs_providers", "00bdb0b0");
}

void GameFrameHost::update_loading_queue() {
    log_.unimplemented("ApplicationFrameHost::update_loading_queue", "004fde20");
}

void GameFrameHost::profiler_end_frame_slot() {
    log_.unimplemented("ApplicationFrameHost::profiler_end_frame_slot", "00be3660");
}

void GameFrameHost::profiler_end_frame() {
    log_.unimplemented("ApplicationFrameHost::profiler_end_frame", "00be34d0");
}

// ---------------------------------------------------------------------------
// GameLoopCallbacks, 00bec1a0
// ---------------------------------------------------------------------------

bool GameLoopCallbacks::pretranslate(MSG& message) {
    // Native pretranslation is XLivePreTranslateMessage. The XLive binding is not
    // reconstructed, so no message is ever consumed here.
    log_.unimplemented("PlatformLoopCallbacks::pretranslate", "00bec20a");
    static_cast<void>(message);
    return false;
}

void GameLoopCallbacks::frame() {
    run_application_frame(frame_state_, color_, frame_host_);
    device_.clear_and_present();
    ++frames_;
    if (frame_limit_ >= 0 && frames_ >= static_cast<unsigned long long>(frame_limit_)) {
        log_.notef("frame limit %ld reached, requesting loop exit", frame_limit_);
        frame_host_.request_loop_exit();
    }
    static_cast<void>(loop_);
}

// ---------------------------------------------------------------------------
// GameStartupHost, 008f81f0
// ---------------------------------------------------------------------------

GameStartupHost::GameStartupHost(GameHostLog& log, HINSTANCE instance,
    const GameExecutableOptions& options) : log_(log), instance_(instance), options_(options) {
    initialize_static_game_settings_00cd2d80(settings_);
}

GameStartupHost::~GameStartupHost() {
    delete loop_callbacks_;
    delete frame_host_;
    // The sprite bridge holds textures created on the device, so it goes before the
    // device and before the font host whose registry its pages reference.
    if (device_) device_->set_overlay(nullptr);
    delete frontend_;
    delete locale_;
    delete fonts_;
    delete scripts_;
    delete settings_host_;
    delete device_;
    delete renderer_parameters_;
    if (renderer_api_) renderer_api_->Release();
    release_platform_window();
    delete window_host_;
    delete vfs_;
    delete random_threads_;
}

long GameStartupHost::com_initialize() {
    static_assert(COINIT_MULTITHREADED == 0 && COINIT_SPEED_OVER_MEMORY == 8);
    log_.implemented("StartupHost::com_initialize", "008f81f8");
    // 008F81F8 PUSH 8: multithreaded apartment (0), speed-over-memory flag (8).
    return CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY);
}

long GameStartupHost::com_initialize_security() {
    static_assert(RPC_C_AUTHN_LEVEL_DEFAULT == 0 && RPC_C_IMP_LEVEL_IMPERSONATE == 3);
    log_.implemented("StartupHost::com_initialize_security", "008f820a");
    return CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
}

void GameStartupHost::com_uninitialize() {
    log_.implemented("StartupHost::com_uninitialize", "008f82cc");
    CoUninitialize();
}

void GameStartupHost::current_directory(char* buffer, unsigned long capacity) {
    log_.implemented("StartupHost::current_directory", "008f8254");
    GetCurrentDirectoryA(capacity, buffer);
}

bool GameStartupHost::game_explorer_create() {
    static_assert(CLSCTX_ALL == 0x17);
    log_.implemented("StartupHost::game_explorer_create", "008f8245");
    game_explorer_ = nullptr; // 008F823D, before the actual COM call.
    const HRESULT result = CoCreateInstance(__uuidof(GameExplorer), nullptr,
        CLSCTX_ALL, __uuidof(IGameExplorer), reinterpret_cast<void**>(&game_explorer_));
    // Native branches on signed HRESULT, not on the returned pointer.
    return SUCCEEDED(result);
}

bool GameStartupHost::game_explorer_verify_access(const wchar_t* gdf_binary_path) {
    log_.implemented("StartupHost::game_explorer_verify_access", "008f82aa");
    // Native supplies its ordinary widened C string to the BSTR-typed method;
    // there is no SysAllocString call or length-prefix conversion at this site.
    // ESP+10 is not initialized before VerifyAccess. Preserve the actual API
    // output/remainder bytes and ignore HRESULT, as the following CMP does.
    BOOL has_access;
    static_cast<void>(game_explorer_->VerifyAccess(
        const_cast<wchar_t*>(gdf_binary_path), &has_access));
    BOOL captured_access;
    __asm {
        mov eax, has_access
        mov captured_access, eax
    }
    return captured_access != FALSE;
}

void GameStartupHost::game_explorer_release() {
    log_.implemented("StartupHost::game_explorer_release", "008f82c4");
    if (game_explorer_ != nullptr) game_explorer_->Release();
    // Original does not clear this local slot. This startup sequence runs once.
}

void GameStartupHost::exit_process(int code) {
    log_.implemented("StartupHost::exit_process", "008f82f0");
    std::exit(code); // 008F82F0 calls genuine CRT exit, including atexit callbacks.
}

void GameStartupHost::random_threads_initialize() {
    log_.implemented("StartupHost::random_threads_initialize", "00bd2e20");
    if (random_threads_ == nullptr) random_threads_ = new RandomThreads();
}

void* GameStartupHost::allocate_thread_slot() {
    log_.implemented("StartupHost::allocate_thread_slot", "00bf681b");
    return new char;
}

void GameStartupHost::random_threads_register_current() {
    log_.implemented("StartupHost::random_threads_register_current", "00bd2fe0");
    if (random_threads_ != nullptr) random_threads_->register_current_00bd2fe0();
}

void GameStartupHost::random_threads_unregister_current() {
    log_.implemented("StartupHost::random_threads_unregister_current", "00bd3050");
    if (random_threads_ != nullptr) random_threads_->unregister_current_00bd3050();
}

void GameStartupHost::release_thread_slot(void* slot) {
    log_.implemented("StartupHost::release_thread_slot", "00bf65ac");
    delete static_cast<char*>(slot);
}

void GameStartupHost::random_threads_shutdown() {
    log_.implemented("StartupHost::random_threads_shutdown", "00bd30d0");
    delete random_threads_;
    random_threads_ = nullptr;
}

SingleInstanceMutex GameStartupHost::create_single_instance_mutex(const char* name) {
    log_.implemented("StartupHost::create_single_instance_mutex", "008f8301");
    SingleInstanceMutex mutex{};
    mutex.handle = CreateMutexA(nullptr, TRUE, name);
    mutex.already_exists = GetLastError() == ERROR_ALREADY_EXISTS;
    return mutex;
}

void GameStartupHost::close_mutex(void* handle) {
    log_.implemented("StartupHost::close_mutex", "008f846e");
    if (handle != nullptr) CloseHandle(handle);
}

std::string GameStartupHost::resolve_language() {
    // 008f7db0: the options file under CSIDL_PERSONAL first, the registry LCID only when
    // the file cannot be opened.
    log_.implemented("StartupHost::resolve_language", "008f7db0");
    std::string language = kStartupLanguageEnglish;

    char personal[MAX_PATH] = {};
    if (!options_.settings_personal_root.empty()
        || SHGetSpecialFolderPathA(nullptr, personal, CSIDL_PERSONAL, TRUE)) {
        std::string path = options_.settings_personal_root.empty()
            ? std::string(personal) : options_.settings_personal_root;
        path += kOptionsDirectory;
        path += kOptionsFileName;
        PhysicalFile file;
        DWORD error = 0;
        if (file.open_read_only_00bf52a0_fragment(path.c_str(), error)) {
            const std::uint64_t size = file.size_00bf4f90();
            std::string text(static_cast<std::size_t>(size), '\0');
            std::uint32_t read = 0;
            if (size != 0 && file.read_00bf5030(text.data(),
                static_cast<std::uint32_t>(size), read, error)) {
                text.resize(read);
                const std::vector<std::string> tokens = split_option_tokens(text);
                std::vector<const char*> pointers;
                pointers.reserve(tokens.size());
                for (const auto& token : tokens) pointers.push_back(token.c_str());
                startup_language_from_options_tokens(pointers.data(), pointers.size(),
                    language);
            }
            file.close_00bf5090_fragment(error);
            return language;
        }
    }

    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, kRegistryKeyPath, 0, KEY_READ, &key)
        == ERROR_SUCCESS) {
        DWORD type = 0;
        DWORD value = 0;
        DWORD size = sizeof(value);
        if (RegQueryValueExA(key, kRegistryLanguageValue, nullptr, &type,
            reinterpret_cast<LPBYTE>(&value), &size) == ERROR_SUCCESS && type == REG_DWORD) {
            language = startup_language_from_registry_lcid(value);
        }
        RegCloseKey(key);
    }
    return language;
}

void GameStartupHost::error_message_box(const wchar_t* text, const wchar_t* caption) {
    log_.implemented("StartupHost::error_message_box", "008f83c5");
    MessageBoxW(nullptr, text, caption, MB_ICONHAND);
}

void GameStartupHost::set_thread_affinity_to_first_processor() {
    log_.implemented("StartupHost::set_thread_affinity_to_first_processor", "008f83fc");
    SetThreadAffinityMask(GetCurrentThread(), 1);
}

void GameStartupHost::publish_game_resource_factory() {
    // BSP_GameResourceFactory_GetSingleton into 00f8d31c. The factory is not reconstructed.
    log_.unimplemented("StartupHost::publish_game_resource_factory", "008f840b");
}

void GameStartupHost::application_construct() {
    // 00737970 stores 1 at +18h and 0 at +19h/+1Ah, which is ApplicationFrameState's
    // default. The remaining subsystem pointers at +4h..+14h are not reconstructed.
    log_.implemented("StartupHost::application_construct", "00737970");
    frame_state_ = ApplicationFrameState{};
    constructed_ = true;
}

void GameStartupHost::application_initialize(int flags, const char* mode) {
    if (mode == nullptr) throw std::invalid_argument("application initialization requires its native mode string");
    log_.implemented("StartupHost::application_initialize", "0073d410");
    log_.notef("initialize flags=%d mode=%s", flags, mode != nullptr ? mode : "(null)");
    run_initialize_phases(mode);
}

void GameStartupHost::run_initialize_phases(const char* mode) {
    // Phase 0, allocator and process identity (0073d43f-0073d4bd).
    char module_name[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, module_name, static_cast<DWORD>(sizeof(module_name)));
    const std::string module_directory = capture_module_directory_00439040(module_name);
    log_.implemented("Phase 0 capture_module_directory", "00439040");
    log_.notef("module directory %s", module_directory.c_str());

    AllocationStatsState allocation_stats{};
    construct_allocation_stats_00be2900(allocation_stats);
    log_.implemented("Phase 0 construct_allocation_stats", "00be2900");

    construct_frame_clock_singleton_00bedfb0(clock_);
    log_.implemented("Phase 0 construct_frame_clock_singleton", "00bedfb0");

    install_object_handle_resolvers_006ad0d0(object_resolvers_);
    log_.implemented("Phase 0 install_object_handle_resolvers", "006ad0d0");

    // Phase 2, VFS, mounts and packages (0073d604-0073d899). The hardware probe at 0073d610
    // is inside this gate, not before it; milestone 1 recorded it one phase early.
    vfs_ = new GameVfsHost(log_, false);
    VfsStartupState vfs_state;
    run_vfs_startup_phase2(vfs_state, *vfs_);

    // Phase 3, platform, window and save storage (0073d8c0-0073d988).
    construct_win32_platform_00becda0(platform_, nullptr);
    log_.implemented("Phase 3 construct_win32_platform", "00becda0");
    set_active_platform_state(&platform_);

    GameSaveStorageHost save_storage(log_);
    if (initialize_save_storage_00beb2c0(save_storage, save_roots_)) {
        log_.implemented("Phase 3 initialize_save_storage", "00beb2c0");
        log_.notef("save directory %s", save_roots_.save_directory.c_str());
    } else {
        log_.note("save storage initialization failed");
    }

    // The command line is parsed at 0073d94a, after the whole phase-2 block, which is why
    // cachedload cannot have influenced any phase-2 mount. Milestone 1 parsed it before
    // phase 2; the native position is used here.
    // WinMain passes "cachedload" as the second 0073d410 argument. Native keeps
    // that pointer in EBP, duplicates it at 0073d933, then parses at 0073d94a.
    const CommandLineOptions command_line = parse_command_line_0073ce20(mode);
    log_.implemented("Phase 3 parse_command_line", "0073ce20");
    log_.notef("command line cached_load=%d fixed_frame_rate=%d file_access_log=%d",
        command_line.cached_load ? 1 : 0, command_line.fixed_frame_rate ? 1 : 0,
        command_line.file_access_log ? 1 : 0);

    // Factory tail 0073d94f-0073d98d follows platform/save and command-line setup.
    run_vfs_startup_factory_tail(vfs_state, *vfs_, command_line.cached_load);

    summary_.vfs_ready = vfs_->ready();
    summary_.mounts_requested = vfs_->mounts().size();
    summary_.mounts_created = 0;
    for (const GameMountRecord& mount : vfs_->mounts()) {
        if (std::strcmp(mount.status, "created") == 0) ++summary_.mounts_created;
    }
    summary_.package_entries = vfs_->package_entries_enumerated();
    summary_.package_mounts = vfs_->package_entries_mounted();
    summary_.cached_load = vfs_->cached_load();
    log_.notef("vfs mounts requested=%zu created=%zu package entries=%zu mounted=%zu",
        summary_.mounts_requested, summary_.mounts_created, summary_.package_entries,
        summary_.package_mounts);

    // Renderer constructor0073da88, input getter0073da94/loader0073da9b,
    // then settings0073daa5. The same Direct3D API survives into device creation.
    if (!vfs_->manager()) throw std::runtime_error("Script/settings startup requires the mounted VFS");
    renderer_api_ = Direct3DCreate9(D3D_SDK_VERSION);
    if (!renderer_api_) throw std::runtime_error("Renderer Direct3DCreate9 failed");
    log_.implemented("RendererHost::direct3d_create", "00b32410");
    renderer_parameters_ = new NativeRendererParametersOwner;
    initialize_native_renderer_parameters_00b32410_fragment(*renderer_parameters_);
    Win32SettingsCapabilityQueries renderer_queries(*renderer_api_);
    enumerate_settings_resolutions_00b27d80(renderer_capabilities_, renderer_queries);
    HRESULT capabilities_result = query_renderer_adapter_identifier_00b32410(
        renderer_full_capabilities_, *renderer_api_);
    if (FAILED(capabilities_result)) throw std::runtime_error("Renderer adapter identification failed");
    capabilities_result = gather_renderer_capabilities_00b2c8e0(
        renderer_full_capabilities_, renderer_capabilities_, *renderer_api_);
    if (FAILED(capabilities_result)) throw std::runtime_error("Renderer capability query failed");
    log_.implemented("RendererHost::gather_capabilities", "00b2c8e0");
    log_.notef("renderer capabilities api=%p pixel_version=0x%04x shader_ceiling=%d "
        "formats=%zu declaration_types=%zu", static_cast<void*>(renderer_api_),
        renderer_capabilities_.pixel_shader_version_28, renderer_capabilities_.max_shader_model,
        renderer_full_capabilities_.texture_formats_1b68.size(),
        renderer_full_capabilities_.declaration_types_1b5c.size());

    scripts_ = new GameScriptHost(log_, vfs_->manager()->context(), content_suffixes_,
        make_initial_lua_runtime_globals_0108ff20());
    summary_.input_scripts_ready = scripts_->input().data_tables_started();
    summary_.input_devices = scripts_->input().settings().devices.size();
    summary_.input_names = scripts_->input().settings().input_names.size();
    summary_.controller_names = scripts_->input().settings().controller_input_names.size();

    // Phase 5, the settings block at 00f88980 filled by 008d8190 at 0073daa5. It runs before
    // window creation at 0073dc0f, which is the ordering constraint the whole phase exists
    // for: arguments 7, 8, 3, 4 and 9 of 00becee0 are read straight out of this block.
    settings_host_ = new GameSettingsBinding(log_, vfs_->manager()->context(),
        vfs_->search_registrations(), content_suffixes_, profile_hints_, *renderer_api_,
        renderer_capabilities_, options_.settings_personal_root);
    auto& settings_host = *settings_host_;
    load_game_settings_008d8190(settings_, settings_host);
    log_.implemented("Phase 5 load_game_settings", "008d8190");
    summary_.options_file_present = settings_host.options_file_present();
    summary_.options_path = settings_host.options_path();
    log_.notef("options file %s %s", settings_host.options_path().c_str(),
        summary_.options_file_present ? "loaded" : "absent, initial options write attempted");
    summary_.language = settings_.options_file.language;
    summary_.settings_width = settings_.options_file.width_14;
    summary_.settings_height = settings_.options_file.height_18;
    summary_.settings_fullscreen = settings_.options_file.fullscreen_1e;
    summary_.settings_vsync = settings_.options_file.vsync_60;
    summary_.settings_antialias = settings_.options_file.antialias_58;
    log_.notef("settings resolution=%dx%d index=%d fullscreen=%d vsync=%d antialias=%d "
        "shader_model=%d language=%s", settings_.options_file.width_14, settings_.options_file.height_18,
        settings_.options_file.resolution_index_78, settings_.options_file.fullscreen_1e ? 1 : 0,
        settings_.options_file.vsync_60 ? 1 : 0, settings_.options_file.antialias_58, settings_.options_file.shader_model_88,
        settings_.options_file.language.empty() ? "(none)" : settings_.options_file.language.c_str());

    // Native parser registration0073db41..db69 follows settings loading.
    run_vfs_startup_phase6(vfs_state, *vfs_);

    // The three VFS reads the later milestones depend on: a GUI script, the locale table the
    // settings language selects, and one texture.
    if (vfs_ != nullptr && vfs_->ready()) {
        std::vector<std::string> probes{"interface/_common.lua"};
        probes.push_back("lockit/"
            + (settings_.options_file.language.empty() ? std::string("english") : settings_.options_file.language)
            + ".lng");
        probes.push_back("effects/a_fiji_terr_atl.dds");
        for (const std::string& extra : options_.vfs_probes) probes.push_back(extra);
        for (const std::string& path : probes) {
            if (vfs_->probe(path).opened) ++summary_.probes_resolved;
            ++summary_.probes_requested;
        }
    }

    // Arguments 3, 4, 7, 8 and 9 of 00becee0, read from the settings block. Argument 4 is the
    // VSync setting and argument 9 the antialias sample count; see the two corrections at the
    // end of docs/APP_INIT_PLATFORM.md. Arguments 5 and 6 have no writer in the image, so the
    // window is always created at 0,0.
    PlatformWindowRequest request{};
    window_class_name_ = kWindowName;
    request.name = window_class_name_.c_str();
    request.fullscreen = settings_.options_file.fullscreen_1e;
    request.color_depth_selector = settings_.options_file.vsync_60;
    request.x = 0;
    request.y = 0;
    request.width = settings_.options_file.width_14;
    request.height = settings_.options_file.height_18;
    request.renderer_option = static_cast<std::uint32_t>(settings_.options_file.antialias_58);
    request.application = this;
    request.instance = instance_;
    request.procedure = &game_window_procedure;
    if (request.width <= 0 || request.height <= 0) {
        // 008d841f's literal pair, the fallback the loader itself uses when a parsed
        // resolution is not in the supported table.
        request.width = kFallbackResolution.width;
        request.height = kFallbackResolution.height;
        log_.notef("settings gave no usable resolution; using the 008d841f fallback %dx%d",
            request.width, request.height);
    }
    log_.notef("window request %dx%d fullscreen=%d vsync=%d antialias=%u", request.width,
        request.height, request.fullscreen ? 1 : 0, request.color_depth_selector ? 1 : 0,
        request.renderer_option);

    window_host_ = new GameWindowHost(log_, instance_);
    summary_.window_created = configure_platform_window_00becee0(*window_host_, request,
        platform_, renderer_request_);
    if (summary_.window_created) {
        log_.implemented("Phase 3 configure_platform_window", "00becee0");
        log_.notef("window created %dx%d at %d,%d color_depth=%d",
            platform_.present_width, platform_.present_height, platform_.x, platform_.y,
            platform_.color_depth);
        // 00bed1b8 builds the renderer init request from the same settings. Both VSync and
        // the antialias sample count reach it, but d3d9_create_device_prefix_00b2aeb0 models
        // neither PresentationInterval nor MultiSampleType, so the device below is created
        // with the recovered constants and these two values stop here.
        log_.notef("renderer init request %dx%d fullscreen=%d vsync=%d antialias=%u "
            "(vsync and antialias are not consumed by the device prefix)",
            renderer_request_.width, renderer_request_.height,
            renderer_request_.fullscreen ? 1 : 0,
            renderer_request_.color_depth_selector ? 1 : 0, renderer_request_.option);
    } else {
        log_.note("window creation failed");
    }

    // Phase 4, renderer (0073d9cc-0073da88): the device creation point.
    device_ = new GameDeviceHost(log_, *renderer_api_, *renderer_parameters_);
    if (summary_.window_created && renderer_request_.requested) {
        summary_.device_created = device_->create(renderer_request_);
        summary_.device_result = device_->creation_result();
        summary_.back_buffer_width = device_->parameters().BackBufferWidth;
        summary_.back_buffer_height = device_->parameters().BackBufferHeight;
        IDirect3D9* device_api = nullptr;
        if (summary_.device_created && SUCCEEDED(device_->device()->GetDirect3D(&device_api))) {
            summary_.renderer_api_shared = device_api == renderer_api_
                && &settings_host.renderer_api() == &device_->renderer_api();
            device_api->Release();
        }
    }
    log_.unimplemented("Phase 4 renderer_resources", "00b14a10");

    // Locale construction and exact setter/register/reload order0073e057..e135.
    locale_ = new GameLocaleHost(log_);
    locale_->initialize(settings_host.locale_source(), language_name_008d4870(
        settings_host.language_catalog(), static_cast<std::size_t>(settings_.gameplay.language_index_04)));
    summary_.locale_keys = locale_->tables().size();
    summary_.locale_files = locale_->tables().loaded_files().size();
    // The tables the phase loaded, and one string id resolved out of them, so a run states
    // that the locale phase produced lookups and not just a key count. 00a9ec70 is the
    // lookup: hash, then walk the chain comparing length and _stricmp.
    for (const std::string& file : locale_->tables().loaded_files()) {
        log_.notef("locale table %s", file.c_str());
    }
    // The ids the title pages themselves author: fe_initial.lua's copyright text and
    // _pleasewait.lua's message, plus the profile reset's globals.newplayer.
    for (const char* key : {"FE.init_legal", "globals.pleasewait", "globals.newplayer"}) {
        const std::u16string* text = locale_->tables().find_00a9ec70(key);
        if (text == nullptr) {
            log_.notef("locale lookup %s -> miss", key);
            continue;
        }
        std::string ascii;
        for (char16_t unit : *text) {
            ascii.push_back(unit >= 0x20 && unit < 0x7F ? static_cast<char>(unit) : '?');
        }
        if (ascii.size() > 96) ascii.resize(96);
        log_.notef("locale lookup %s -> %zu units \"%s\"", key, text->size(), ascii.c_str());
        log_.implemented("Phase 6 locale_lookup", "00a9ec70");
    }

    // Required later owners remain outside the currently runnable spine.
    log_.unimplemented("Phase 5 sound_system_initialize", "00a88770");
    if (!device_->device()) throw std::runtime_error("Font startup requires the renderer device");
    //Constructor00b32769 initializes renderer+1D84 to zero. The later ApplyAll
    //texture-detail setter is not bound by this startup path yet.
    fonts_ = new GameFontHost(log_, *vfs_, *scripts_, *device_->device(), 0);

    // Phase 7, 0073e13c: the whole of 0073bae0 through its reconstruction, which loads the
    // font descriptors, forces the fingerprint payload, creates the GUI manager and walks
    // 00aa5e20's fixed resource list. The two resource groups it names, `_Mouse` and
    // `_Highlight`, are real GUI pages, so this is also the first phase that evaluates
    // page scripts. Evidence: docs/APP_INIT_FONTS_GUI.md.
    frontend_ = new GameFrontendHost(log_, *vfs_, *scripts_, *fonts_, *device_->device(),
        platform_.widescreen);
    frontend_->run_font_and_gui_startup_0073bae0(language_font_path_008d4890(
        settings_host_->language_catalog(), settings_.gameplay.language_index_04));
    summary_.fonts_loaded = fonts_->registry().fonts().size();
    summary_.font_resource_opens = fonts_->resource_opens();
    summary_.fingerprint_defined_bytes = fonts_->fingerprint().defined_size();

    log_.unimplemented("Phase 8 world_effects_startup", "00af0b10");
    log_.unimplemented("Phase 9 game_entry", "00740840");

    // The GUI pages GGame::OnInitTitle brings up (docs/GAME_TITLE_INIT.md): the panel and
    // title layouts 00518250 selects for frame sets 0..2, and the title screen's own
    // FE_initial. The front-end state machine that would request them is packet
    // cc_frontend_states and is not reconstructed here; this loads the pages directly.
    frontend_->load_title_pages({"FE_frame", "FE_frame_title", "FE_initial"});
    log_.unimplemented("Title bring-up GGame::OnInitTitle", "004c9a70");
    const GameFrontendSummary& frontend = frontend_->summary();
    summary_.gui_pages_loaded = frontend.pages_loaded;
    summary_.gui_pages_requested = frontend.pages_requested;
    summary_.gui_widgets = frontend.widgets;
    summary_.gui_widgets_with_texture = frontend.widgets_with_texture;
    summary_.gui_resources_acquired = frontend.gui_resources_acquired;

    // The sprite bridge, drawn inside the milestone's own present. Not the native GUI
    // draw path; see include/bsp/game_hosts_frontend.hpp.
    if (summary_.device_created) {
        frontend_->open_sprite_bridge(summary_.back_buffer_width, summary_.back_buffer_height);
        GameFrontendHost* frontend_host = frontend_;
        device_->set_overlay([frontend_host](IDirect3DDevice9& device) {
            frontend_host->draw_bridge(device);
        });
    }

    loop_.frames_enabled = platform_.frames_enabled;
    frame_host_ = new GameFrameHost(log_, clock_, platform_, loop_);
    loop_callbacks_ = new GameLoopCallbacks(log_, frame_state_, frame_color_, *frame_host_,
        *device_, loop_, options_.frame_limit);
}

void GameStartupHost::platform_run_loop_dispatch() {
    log_.implemented("StartupHost::platform_run_loop_dispatch", "00bec1a0");
    if (loop_callbacks_ == nullptr) {
        log_.note("no loop callbacks; initialize did not complete");
        return;
    }
    if (!loop_.frames_enabled) {
        log_.note("frames are disabled; the loop would never call the frame callback");
    }
    platform_run_loop_00bec1a0(loop_, *loop_callbacks_);
    summary_.loop_finished = loop_.loop_finished;
    summary_.frames_presented = device_ != nullptr ? device_->presented() : 0ull;
    log_.notef("loop finished=%d frames=%llu presented=%llu", summary_.loop_finished ? 1 : 0,
        loop_callbacks_->frames(), summary_.frames_presented);
}

void GameStartupHost::release_platform_window() noexcept {
    // Host cleanup also runs if a required startup service throws. Do not log:
    // WinMain can close the log before this object's destructor executes.
    if (g_active_platform == &platform_) set_active_platform_state(nullptr);
    if (platform_.window != nullptr) {
        DestroyWindow(platform_.window);
        platform_.window = nullptr;
    }
    if (window_host_ != nullptr) window_host_->unregister_class(window_class_name_.c_str());
}

void GameStartupHost::application_shutdown() {
    // Native00737f30's full singleton teardown remains unbound. Retained C++
    // input/locale/settings owners close later in dependency order at destruction.
    log_.implemented("StartupHost::application_shutdown", "00737f30");
    log_.unimplemented("ApplicationShutdownHost::singleton_teardown", "00737f80");
    // The sprite bridge owns managed-pool textures created on this device, so it is torn
    // down before the device is released rather than at destruction.
    if (frontend_ != nullptr) {
        const GameFrontendSummary& frontend = frontend_->summary();
        summary_.gui_bridge_open = frontend.bridge_open;
        summary_.gui_bridge_atlas = frontend.bridge_atlas;
        summary_.gui_bridge_atlas_items = frontend.bridge_atlas_items;
        summary_.gui_bridge_textures = frontend.bridge_textures;
        summary_.gui_bridge_quads = frontend.bridge_quads;
        summary_.gui_bridge_frames = frontend.bridge_frames;
    }
    delete frontend_;
    frontend_ = nullptr;
    if (device_ != nullptr) device_->release();
    release_platform_window();
}

void GameStartupHost::application_destruct() {
    log_.implemented("StartupHost::application_destruct", "008f8444");
    constructed_ = false;
}

void GameStartupHost::destroy_singleton_lifetime_manager() {
    // 008f8449: only when 01090aa0 is set. No singleton lifetime manager exists here.
    log_.unimplemented("StartupHost::destroy_singleton_lifetime_manager", "008f8449");
}

}  // namespace bsp::game
