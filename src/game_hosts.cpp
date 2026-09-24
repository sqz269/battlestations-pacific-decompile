// Process bindings for the reconstructed startup spine. See include/bsp/game_hosts.hpp
// and docs/GAME_EXECUTABLE.md. No native behaviour is invented here: whatever is not
// reconstructed is routed through GameHostLog::unimplemented with its native call site.
#include "bsp/game_hosts.hpp"
#include "bsp/platform_control_messages.hpp"
#include "bsp/platform_text_messages.hpp"
#include "bsp/game_native_lua_globals.hpp"
#include "bsp/game_native_lua_services.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_shader_process.hpp"
#include "bsp/native_renderer_end_frame.hpp"
#include "bsp/native_xlive_device_adapter.hpp"
#include "bsp/native_online_signin.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_renderer_reset_process.hpp"
#include "bsp/native_renderer_reset_readiness.hpp"
#include "bsp/game_native_settings_process.hpp"
#include "bsp/game_native_settings_application.hpp"
#include "bsp/game_native_online_process.hpp"
#include "bsp/lua_runtime_globals.hpp"
#include <stdexcept>

#include <objbase.h>
#include <gameux.h>
#include <shlobj.h>

#include <cfloat>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <filesystem>

#include "bsp/app_bootstrap.hpp"
#include "bsp/d3d9_startup.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_hosts_fonts.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/game_hosts_init_tail.hpp"
#include "bsp/game_hosts_menu.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_observer_runtime.hpp"
#include "bsp/game_pending_entity_runtime.hpp"
#include "bsp/game_hosts_mission.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/fingerprint_payload.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_frame_clock_lifetime.hpp"
#include "bsp/native_frame_clock_publication.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_renderer_parameters.hpp"
#include "bsp/physical_file.hpp"
#include "bsp/renderer_startup.hpp"
#include "bsp/game_platform_services.hpp"
#include "bsp/game_sound_runtime.hpp"
#include "bsp/game_sound_dialog_runtime.hpp"
#include "bsp/game_input_runtime.hpp"
#include "bsp/gui_input_runtime.hpp"
#include "bsp/fmod_configuration_library.hpp"
#include "bsp/sound_alternate_owner.hpp"
#include "bsp/legacy_crt_math.hpp"

namespace bsp::game {
namespace {

// Verified loader-zero CRT publication. Process lifetime is required because
// other recovered arithmetic can run after the sound aggregate is destroyed.
volatile std::uint32_t application_matherr_bypass_00e16bd0{};
volatile std::uint32_t application_dispatch_bypass_0109dd78{};
const LegacyCrtMathRuntime application_math_runtime{
    &application_matherr_bypass_00e16bd0, &_errno};
const CameraAxesCrtAccess application_axes_crt{
    &application_dispatch_bypass_0109dd78, &legacy_crt_87except_00c27489};

std::wstring selected_library_path(const std::wstring& selected, const wchar_t* name) {
    return selected.empty() ? std::filesystem::absolute(name).wstring() : selected;
}

std::wstring selected_xinput_path(const std::wstring& selected) {
    if (!selected.empty()) return selected;
    wchar_t directory[MAX_PATH];
    const UINT length = GetSystemDirectoryW(directory, MAX_PATH);
    if (!length || length >= MAX_PATH)
        throw std::runtime_error("cannot resolve the system XINPUT1_3.dll directory");
    return (std::filesystem::path(directory) / L"XINPUT1_3.dll").wstring();
}

double input_image_double(std::uint64_t bits) noexcept {
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// BEC3B0 forwards the current platform singleton as the explicit receiver;
// BED3B0 separately reads window-extra offset zero for the resize receiver.
// The application binds both to its projection, preserving their distinct roles.
Win32PlatformState* volatile g_active_platform = nullptr;
PlatformTextInput* g_active_platform_text = nullptr;

// Window title and class name, the temporary string 00becee0 receives as argument 2.
const char kWindowName[] = "Battlestations Pacific";

// Host-selected frontend background; the color is not a recovered game value.
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

const CameraAxesCrtAccess& application_camera_axes_crt() noexcept {
    return application_axes_crt;
}

void set_active_platform_state(Win32PlatformState* state, PlatformTextInput* text) noexcept {
    g_active_platform = state;
    g_active_platform_text = state ? text : nullptr;
}

LRESULT CALLBACK game_window_procedure(HWND window, UINT message, WPARAM wparam,
    LPARAM lparam) {
    class WindowMessages final : public PlatformFocusMessageHost {
    public:
        Win32PlatformState& window_state(HWND hwnd) override {
            return *reinterpret_cast<Win32PlatformState*>(GetWindowLongA(hwnd, 0));
        }
        void store_window_state(HWND hwnd, Win32PlatformState& state) override {
            SetWindowLongA(hwnd, 0, reinterpret_cast<LONG>(&state));
        }
        void set_window_pos(HWND hwnd, HWND after, int x, int y, int width,
            int height, UINT flags) override {
            SetWindowPos(hwnd, after, x, y, width, height, flags);
        }
        void set_focus(HWND hwnd) override { SetFocus(hwnd); }
        void set_foreground(HWND hwnd) override { SetForegroundWindow(hwnd); }
        LRESULT default_message(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) override {
            return DefWindowProcA(hwnd, msg, wp, lp);
        }
    } host;
    LRESULT result;
    if (g_active_platform && handle_platform_control_message_00bed3b0_fragment(
            win32_platform_control_message_imports(), *g_active_platform,
            window, message, wparam, lparam, result))
        return result;
    if (g_active_platform && handle_platform_focus_message_00bed3b0_fragment(host,
            *g_active_platform, window, message, wparam, lparam, result))
        return result;
    // Native text arms use the explicit platform receiver, not window-extra
    // state. This owner persists across messages; enabling/dispatch belongs to
    // the actual text editor, whose frontend binding remains incomplete.
    if (g_active_platform_text && handle_platform_text_message_00bed3b0_fragment(
            *g_active_platform_text, window, message, wparam, lparam, result))
        return result;
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

// Milestone 2l. The three values _MCW_PC can hold, named. docs/X87_CONTROL_WORD.md
// reads the CRT's own setter at 00c0683c asking for _PC_53 under this mask and
// the two CreateDevice sites at 00b2aff9 / 00b298ee passing behaviour flags
// without D3DCREATE_FPU_PRESERVE.
unsigned long x87_precision_field() noexcept {
    unsigned int current = 0;
    if (_controlfp_s(&current, 0, 0) != 0) return 0xffffffffu;
    return static_cast<unsigned long>(current) & static_cast<unsigned long>(_MCW_PC);
}

const char* x87_precision_name(unsigned long precision_field) noexcept {
    switch (precision_field) {
        case static_cast<unsigned long>(_PC_24): return "24-bit (single)";
        case static_cast<unsigned long>(_PC_53): return "53-bit (double)";
        case 0u: return "64-bit (extended)";
        default: break;
    }
    return "unknown";
}

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
        } else if (std::strcmp(argument, "--fmod-dll") == 0
            || std::strcmp(argument, "--fmod-event-dll") == 0
            || std::strcmp(argument, "--xlive-dll") == 0
            || std::strcmp(argument, "--xlive-dependency") == 0
            || std::strcmp(argument, "--xinput-dll") == 0) {
            if (index + 1 >= argc) {
                error = std::string(argument) + " needs a DLL path";
                return false;
            }
            std::error_code path_error;
            const auto path = std::filesystem::absolute(argv[++index], path_error);
            if (path_error) {
                error = std::string("cannot resolve library path: ") + path_error.message();
                return false;
            }
            if (std::strcmp(argument, "--fmod-dll") == 0) fmod_dll = path.wstring();
            else if (std::strcmp(argument, "--fmod-event-dll") == 0) fmod_event_dll = path.wstring();
            else if (std::strcmp(argument, "--xlive-dll") == 0) xlive_dll = path.wstring();
            else if (std::strcmp(argument, "--xinput-dll") == 0) xinput_dll = path.wstring();
            else xlive_dependencies.push_back(path.wstring());
        } else if (std::strcmp(argument, "--vfs-probe") == 0) {
            if (index + 1 >= argc) {
                error = "--vfs-probe needs a virtual path";
                return false;
            }
            vfs_probes.emplace_back(argv[++index]);
        } else if (std::strcmp(argument, "--press-start-frame") == 0) {
            if (index + 1 >= argc) {
                error = "--press-start-frame needs a frame number";
                return false;
            }
            press_start_frame = std::strtol(argv[++index], nullptr, 10);
            if (press_start_frame < 0) {
                error = "--press-start-frame needs a non-negative frame number";
                return false;
            }
        } else if (std::strcmp(argument, "--screenshot") == 0) {
            if (index + 1 >= argc) {
                error = "--screenshot needs a path";
                return false;
            }
            // Resolved here, before --game-root changes the current directory,
            // so a relative path never lands inside the installed game.
            const char* input = argv[++index];
            const DWORD required = GetFullPathNameA(input, 0, nullptr, nullptr);
            if (!required) { error = "cannot resolve the screenshot path"; return false; }
            screenshot_path.resize(required);
            const DWORD length = GetFullPathNameA(input, required, screenshot_path.data(),
                nullptr);
            if (!length || length >= required) {
                error = "cannot resolve the screenshot path";
                return false;
            }
            screenshot_path.resize(length);
        } else if (std::strcmp(argument, "--screenshot-frame") == 0) {
            if (index + 1 >= argc) {
                error = "--screenshot-frame needs a frame number";
                return false;
            }
            screenshot_frame = std::strtol(argv[++index], nullptr, 10);
            if (screenshot_frame < 0) {
                error = "--screenshot-frame needs a non-negative frame number";
                return false;
            }
        } else if (std::strcmp(argument, "--screenshot-mission-frame") == 0) {
            if (index + 1 >= argc) {
                error = "--screenshot-mission-frame needs a frame number";
                return false;
            }
            screenshot_mission_frame = std::strtol(argv[++index], nullptr, 10);
            if (screenshot_mission_frame < 0) {
                error = "--screenshot-mission-frame needs a non-negative frame number";
                return false;
            }
        } else if (std::strcmp(argument, "--menu-select") == 0) {
            if (index + 1 >= argc) {
                error = "--menu-select needs a mission id";
                return false;
            }
            menu_select.assign(argv[++index]);
        } else if (std::strcmp(argument, "--mission-frames") == 0) {
            // Milestone 2f: how many in-mission frames of 004e4a40 to run once
            // the load has finished and 004da6c0 has written game state 0Dh.
            if (index + 1 >= argc) {
                error = "--mission-frames needs a count";
                return false;
            }
            mission_frames = std::strtol(argv[++index], nullptr, 10);
        } else if (std::strcmp(argument, "--mission-complete-frame") == 0) {
            // Milestone 2g: the in-mission frame on which the executable makes
            // the script's end-movie call.
            if (index + 1 >= argc) {
                error = "--mission-complete-frame needs a frame number";
                return false;
            }
            mission_complete_frame = std::strtol(argv[++index], nullptr, 10);
        } else if (std::strcmp(argument, "--order-frame") == 0) {
            // Milestone 2i: the in-mission frame the player order is issued on.
            if (index + 1 >= argc) {
                error = "--order-frame needs a frame number";
                return false;
            }
            order_frame = std::strtol(argv[++index], nullptr, 10);
        } else if (std::strcmp(argument, "--order-unit") == 0) {
            // Milestone 2n: the created instance --order's command form goes to.
            if (index + 1 >= argc) {
                error = "--order-unit needs a unit name";
                return false;
            }
            order_unit = argv[++index];
        } else if (std::strcmp(argument, "--ai-drive") == 0) {
            // Milestone 2o: <name>=<throttle>,<rudder>. The diagnostic stand-in
            // for the state step, engaged on --order-frame.
            if (index + 1 >= argc) {
                error = "--ai-drive needs <unit>=<throttle>,<rudder>";
                return false;
            }
            const std::string text = argv[++index];
            const std::size_t equals = text.find('=');
            const std::size_t comma = text.find(',', equals == std::string::npos ? 0
                                                                                 : equals);
            if (equals == std::string::npos || comma == std::string::npos) {
                error = "--ai-drive needs <unit>=<throttle>,<rudder>";
                return false;
            }
            ai_drive_unit = text.substr(0, equals);
            ai_drive_throttle = static_cast<float>(
                std::atof(text.substr(equals + 1, comma - equals - 1).c_str()));
            ai_drive_rudder = static_cast<float>(std::atof(text.substr(comma + 1).c_str()));
            if (ai_drive_unit.empty()) {
                error = "--ai-drive needs a unit name before the '='";
                return false;
            }
        } else if (std::strcmp(argument, "--order") == 0) {
            // Milestone 2i: throttle=<f>,rudder=<f>, the two parameters
            // 00816a40 publishes into the controlled unit's order ring.
            // Milestone 2l: or the name of one of the 26 command classes, with
            // an optional `:<entity>` target, issued through the same recovered
            // path the authored scene command takes.
            if (index + 1 >= argc) {
                error = "--order needs throttle=<f>,rudder=<f> or a command name";
                return false;
            }
            const std::string text = argv[++index];
            if (text.find("throttle=") == std::string::npos
                && text.find("rudder=") == std::string::npos
                && text.find("speed=") == std::string::npos) {
                const std::size_t equals = text.find('=');
                if (equals != std::string::npos) {
                    // Milestone 2s: `<command>=<x>,<z>`, a fixed world point.
                    // Milestone 2l's refusal here was right about 0046AAB0,
                    // which builds only a named-target or owner-position
                    // descriptor, and wrong to conclude that nothing could
                    // carry a coordinate: the mission script's own navigator
                    // bindings do not go through 0046AAB0 at all. 008A2BC0
                    // NavigatorMoveToPos hands 0077D600 a fixed command object
                    // and the descriptor 0088A810's Vector3 branch built, and
                    // milestone 2m wired that entry. The point is passed on as
                    // the target token and GameUnitsHost::issue_player_command
                    // turns it into that descriptor.
                    order_command = text.substr(0, equals);
                    order_command_target = text.substr(equals + 1);
                    if (order_command.empty()) {
                        error = "--order needs a command name before the '='";
                        return false;
                    }
                    if (order_command_target.find(',') == std::string::npos) {
                        error = "--order <command>=<args> carries a fixed world point and "
                            "nothing else: use --order <command>=<x>,<z>. A named target "
                            "goes through 0046aab0 instead, as --order <command>:<entity>";
                        return false;
                    }
                    continue;
                }
                const std::size_t colon = text.find(':');
                order_command = text.substr(0, colon);
                if (colon != std::string::npos) {
                    order_command_target = text.substr(colon + 1);
                }
                if (order_command.empty()) {
                    error = "--order needs a command name";
                    return false;
                }
                continue;
            }
            std::size_t begin = 0;
            bool seen = false;
            while (begin <= text.size()) {
                const std::size_t comma = text.find(',', begin);
                const std::string field = text.substr(begin,
                    comma == std::string::npos ? std::string::npos : comma - begin);
                const std::size_t equals = field.find('=');
                if (equals != std::string::npos) {
                    const std::string key = field.substr(0, equals);
                    const float value
                        = static_cast<float>(std::atof(field.c_str() + equals + 1));
                    if (key == "throttle") {
                        order_throttle = value;
                        seen = true;
                    } else if (key == "rudder") {
                        order_rudder = value;
                        seen = true;
                    } else if (key == "speed") {
                        // Milestone 2m: the call luaMW_SetShipSpeed 00890d30
                        // makes. It writes no throttle: it stores the pair on
                        // the unit's navigator parameter block at *(unit+73Ch),
                        // which is what makes the weapon director's idle tail
                        // choose `cruise` over `stop` and what 009e12bd then
                        // divides by the reference speed.
                        order_speed = value;
                        order_speed_set = true;
                        seen = true;
                    } else {
                        error = "--order takes throttle=<f>, rudder=<f> and speed=<m/s>";
                        return false;
                    }
                }
                if (comma == std::string::npos) break;
                begin = comma + 1;
            }
            if (!seen) {
                error = "--order takes throttle=<f> and rudder=<f>";
                return false;
            }
        } else if (std::strcmp(argument, "--mission-frame-seconds") == 0) {
            // Milestone 2i: a fixed in-mission frame delta, so a headless run
            // accumulates simulated time deterministically.
            if (index + 1 >= argc) {
                error = "--mission-frame-seconds needs a duration";
                return false;
            }
            mission_frame_seconds = static_cast<float>(std::atof(argv[++index]));
            if (!(mission_frame_seconds >= 0.0f)) {
                error = "--mission-frame-seconds needs a non-negative duration";
                return false;
            }
        } else if (std::strcmp(argument, "--trajectory-csv") == 0) {
            // Milestone 2j: where to write the per-step per-unit trace.
            if (index + 1 >= argc) {
                error = "--trajectory-csv needs a path";
                return false;
            }
            // Resolved here for the same reason --screenshot is: the run enters
            // the installed game's directory, which is read-only, so a relative
            // output path would be refused there.
            const char* input = argv[++index];
            const DWORD required = GetFullPathNameA(input, 0, nullptr, nullptr);
            if (!required) { error = "cannot resolve the trajectory csv path"; return false; }
            trajectory_csv.resize(required);
            const DWORD length = GetFullPathNameA(input, required, trajectory_csv.data(),
                nullptr);
            if (!length || length >= required) {
                error = "cannot resolve the trajectory csv path";
                return false;
            }
            trajectory_csv.resize(length);
        } else if (std::strcmp(argument, "--hardware-probe-commit") == 0) {
            hardware_probe_commit = true;
        } else if (std::strcmp(argument, "--instance-tag") == 0) {
            if (index + 1 >= argc) {
                error = "--instance-tag needs a tag";
                return false;
            }
            instance_tag = argv[++index];
            bool tag_ok = !instance_tag.empty() && instance_tag.size() <= 32;
            for (const char c : instance_tag) {
                const bool alnum = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
                if (!alnum && c != '_' && c != '-') tag_ok = false;
            }
            if (!tag_ok) {
                error = "--instance-tag needs 1 to 32 characters from [A-Za-z0-9_-]";
                return false;
            }
        } else if (std::strcmp(argument, "--affinity-core") == 0) {
            if (index + 1 >= argc) {
                error = "--affinity-core needs a processor index";
                return false;
            }
            const long core = std::strtol(argv[++index], nullptr, 10);
            if (core < 0 || core > 31) {
                error = "--affinity-core needs a processor index from 0 to 31";
                return false;
            }
            affinity_core = static_cast<int>(core);
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
    // Slot +8 is recovered BEBF70. Its saved power-policy producer and native
    // normal-shutdown restore schedule are not yet bound in this application.
    log_.unimplemented("PlatformWindowHost::stop_existing_window", "00bebf70");
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
    renderer_.create_device(request);
    device_=renderer_.device();
    parameters_=renderer_.presentation();
    creation_result_=device_ ? S_OK : E_FAIL; // observation, not the ignored native CreateDevice HRESULT
    log_.notef("device created by full native startup size=%ux%u windowed=%d format=%u depth=%u interval=0x%x",
        parameters_.BackBufferWidth,parameters_.BackBufferHeight,parameters_.Windowed,
        static_cast<unsigned>(parameters_.BackBufferFormat),static_cast<unsigned>(parameters_.AutoDepthStencilFormat),
        parameters_.PresentationInterval);
    return device_!=nullptr;
}
IDirect3D9& GameDeviceHost::renderer_api() { return renderer_.api(); }

bool GameDeviceHost::clear_and_present() {
    if (device_ == nullptr) {
        log_.unimplemented("RendererHost::present_frame", "00b32410+vtable20");
        return false;
    }
    log_.implemented("RendererHost::clear_and_present", "00b2b200/00b21430/00b2f4a0");
    renderer_.begin_frame(kMilestoneClearColor);
    if (overlay_) overlay_(*device_);
    // Diagnostic frontend capture before native EndFrame. Later native debug,
    // XLive and clear-request work can change the finally presented pixels.
    if (capture_) {
        std::function<void(IDirect3DDevice9&)> capture = std::move(capture_);
        capture_ = nullptr;
        capture(*device_);
    }
    const auto present = renderer_.end_frame();
    if (!present.returned) {
        ++presents_skipped_;
        log_.note("native frame skipped Present");
        return false;
    }
    const auto result = static_cast<HRESULT>(present.result);
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

void GameDeviceHost::request_capture(std::function<void(IDirect3DDevice9&)> capture) {
    capture_ = std::move(capture);
}

void GameDeviceHost::release() {
    overlay_ = nullptr;
    capture_ = nullptr;
    // The renderer composition retains the real COM reference through native
    // teardown and its later diagnostic AddRef/Release pairs.
    device_ = nullptr;
}

// ---------------------------------------------------------------------------
// GameFrameHost, 00737a50
// ---------------------------------------------------------------------------

void GameFrameHost::profiler_set_frame_slot_color(std::uint32_t argb) {
    // 00737a9e writes the colour of the frame's own slot into the array at
    // profiler+24h; the value is bsp::frame_marker_color_00737a6c's.
    if (profiler_ == nullptr) {
        log_.unimplemented("ApplicationFrameHost::profiler_set_frame_slot_color", "004c1dd0");
        return;
    }
    profiler_set_slot_color(profiler_->counters(), profiler_->app_update_slot(), argb);
    log_.implemented("ApplicationFrameHost::profiler_set_frame_slot_color", "004c1dd0");
}

void GameFrameHost::profiler_begin_frame_slot() {
    if (profiler_ == nullptr) {
        log_.unimplemented("ApplicationFrameHost::profiler_begin_frame_slot", "00be3640");
        return;
    }
    profiler_begin_frame_slot_00be3640(profiler_->counters(), profiler_->app_update_slot(),
        *profiler_);
    log_.implemented("ApplicationFrameHost::profiler_begin_frame_slot", "00be3640");
}

unsigned long long GameFrameHost::mission_frames_run() const noexcept {
    // Milestone 2h: the count the mission frame host keeps of the in-mission
    // frames of 004e4a40 it has run, so --screenshot-mission-frame can name one.
    if (menu_ == nullptr) return 0ull;
    const GameMissionHost* mission = menu_->mission();
    return mission != nullptr ? mission->summary().mission_frames_run : 0ull;
}

int GameFrameHost::game_state() {
    // 00737acc, repeated at 00737b33: MOV ECX,[00e188a8] then MOV EAX,[ECX+5D4h]. A field
    // load off the GGame singleton pointer, not a call, which is why one frame counts two.
    // Milestone 2c owns that field as bsp::GameStateSlot, written by the title bring-up
    // (2), the shell entry (3 then 5) and the drain's store at 004e449e.
    log_.implemented("ApplicationFrameHost::game_state", "00737acc");
    return read_game_state_00737acc(game_state_);
}

bool GameFrameHost::input_action_pressed(int action) {
    // 004c43c0 at 00737ae7 with action 0Eh. The executable's action table holds one
    // record, the press-start action 4Eh the --press-start-frame switch injects, so
    // every other index has no record and 004c43c0's own gate skips it.
    log_.implemented("ApplicationFrameHost::input_action_pressed", "004c43c0");
    return menu_ != nullptr && menu_->input_action_pressed(action);
}

void GameFrameHost::advance_frame_clock() {
    log_.implemented("ApplicationFrameHost::advance_frame_clock", "00bedc30");
    advance_published_native_frame_clock(clock_);
}

const ClockTimestamp& GameFrameHost::frame_interval() {
    log_.implemented("ApplicationFrameHost::frame_interval", "00bee070");
    std::memcpy(&interval_result_, interval_published_native_frame_clock(clock_), sizeof(interval_result_));
    return interval_result_;
}

void GameFrameHost::game_on_move(float seconds) {
    // 004e4a40's front-end branch at 004e4b9d runs for game states 1, 2 and 4, and
    // BSP_Game_UpdateInterfaceOnly 004c40f0 runs for everything else; that split is
    // bsp::front_end_screen_pump_site. The simulation spine the branch falls through to
    // at 004e4d32 is not reconstructed.
    log_.implemented("ApplicationFrameHost::game_on_move", "004e4a40");
    log_.unimplemented("GameFrameControl::simulation_spine", "004e4d32");
    if (menu_ != nullptr) menu_->frame(seconds, frame_index_);
    ++frame_index_;
    // Milestone 2g: a mission that ended through the debrief path has nothing
    // left to run in a headless process, so the run finishes there instead of
    // on the frame count. Requesting the loop exit is the executable's own
    // decision, through the same byte the close policy and --frames use.
    if (menu_ != nullptr && !global_exit_ && menu_->mission_exit_finished()) {
        log_.notef("the mission ended through the debrief path, so the run requests the "
            "application loop exit");
        global_exit_ = true;
    }
    // One of 004e4a40's callees is reconstructed: the window close processor at 004ca2f0
    // (docs/WINDOW_CLOSE.md), whose front-end branch reads and clears platform+180h and
    // sets global exit 00e1ae75. The confirmation-dialog branch taken for other game
    // states is not reconstructed.
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
}

void GameFrameHost::tick_vfs_providers() {
    vfs_.pump_pending();
    log_.implemented("ApplicationFrameHost::tick_vfs_providers", "00bdb0b0");
}

void GameFrameHost::update_loading_queue() {
    log_.unimplemented("ApplicationFrameHost::update_loading_queue", "004fde20");
}

void GameFrameHost::profiler_end_frame_slot() {
    if (profiler_ == nullptr) {
        log_.unimplemented("ApplicationFrameHost::profiler_end_frame_slot", "00be3660");
        return;
    }
    profiler_end_frame_slot_00be3660(profiler_->counters(), profiler_->app_update_slot(),
        *profiler_);
    log_.implemented("ApplicationFrameHost::profiler_end_frame_slot", "00be3660");
}

void GameFrameHost::profiler_end_frame() {
    if (profiler_ == nullptr) {
        log_.unimplemented("ApplicationFrameHost::profiler_end_frame", "00be34d0");
        return;
    }
    // 00be34d0 closes slots 1 .. registered - 1, converts the accumulated ticks through
    // the scale at 0109db48 and advances the ring index modulo 14h.
    static_cast<void>(profiler_end_frame_00be34d0(profiler_->counters(),
        profiler_->registered_slot_count(), profiler_->app_update_slot(),
        profiler_->tick_scale()));
    log_.implemented("ApplicationFrameHost::profiler_end_frame", "00be34d0");
}

// ---------------------------------------------------------------------------
// GameLoopCallbacks, 00bec1a0
// ---------------------------------------------------------------------------

bool GameLoopCallbacks::pretranslate(MSG& message) {
    // 00bec1d8 CALL 00c2f1d2, inside the PeekMessageA success arm, where 00c2f1d2 is
    // JMP [00ce25dc] to XLivePreTranslateMessage (ordinal 5030).
    log_.implemented("PlatformLoopCallbacks::pretranslate", "00bec1d8");
    return xlive_.pretranslate(message);
}

void GameLoopCallbacks::frame() {
    run_application_frame(frame_state_, color_, frame_host_);
    // BECE70 retains its platform receiver across application virtual+10,
    // then BECB20(false) reloads the shared raw online/input publications.
    input_.update_cursor(platform_, false);
    log_.implemented("PlatformLoopCallbacks::cursor_after_application_frame", "00bece81");
    // --screenshot saves the back buffer of the last frame: the frame the count
    // names, or the frame on which the close request was observed.
    const bool last_frame = frame_limit_ >= 0
        && frames_ + 1 >= static_cast<unsigned long long>(frame_limit_);
    // --screenshot-frame N names a frame instead, so a run can photograph the title
    // page before the injected press-start as well as the menu after it.
    // --screenshot-mission-frame N, milestone 2h: the application frame on which
    // in-mission frame N ran, so the HUD can be photographed over the mission.
    // It takes precedence over --screenshot-frame when both are given.
    const bool wanted_frame = screenshot_mission_frame_ >= 0
        ? frame_host_.mission_frames_run()
            == static_cast<unsigned long long>(screenshot_mission_frame_)
        : (screenshot_frame_ >= 0
            ? frames_ == static_cast<unsigned long long>(screenshot_frame_)
            : (last_frame || frame_host_.global_exit()));
    if (capture_ && !capture_requested_ && wanted_frame) {
        capture_requested_ = true;
        device_.request_capture(capture_);
    }
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

struct GameStartupHost::ClockServices {
    enum class Phase { unattempted, constructing, constructed, failed, drained };
    const NativeFrameClockActualContext methods;
    const NativeFrameClockPublicationContext publication;
    NativeFrameClockLifetimeContext lifetime;
    Phase phase{Phase::unattempted};

    ClockServices(GameNativeReadOnlyData& data, GameSingletonHost& singletons,
                  void* volatile& clock)
        : methods{static_cast<const volatile std::uint32_t*>(data.data_at(0x00d68d50, 44))},
          publication{clock, methods},
          lifetime{singletons.manager_publication_01090aa0(), clock, methods} {
        // Validate before allocation/publication: raw initialization dispatches
        // update/sample internally and must never reach unsupported slot words.
        constexpr std::uint32_t expected[] = {
            0x00bee110, 0x00bedbd0, 0x00bedc30, 0x00bedae0, 0x00beddc0,
            0x00bee050, 0x00bee060, 0x00bee070, 0x00bee080, 0x00bedb20, 0x00bedb60};
        for (std::size_t i = 0; i < std::size(expected); ++i)
            if (methods.profile_d68d50[i] != expected[i])
                throw std::logic_error("application clock requires the verified D68D50 profile");
    }

    void ensure() {
        if (phase == Phase::constructed) {
            try {
                // A repeated local ensure skips allocation only for an admitted
                // current publication. This does not make all startup repeatable.
                (void)current_published_native_frame_clock(publication);
            } catch (...) {
                phase = Phase::failed;
                throw;
            }
            return;
        }
        if (phase != Phase::unattempted)
            throw std::logic_error("application clock construction cannot be retried");
        if (publication.actual_clock_01090ab0 != nullptr) {
            phase = Phase::failed; // ownership of this graph is not established
            throw std::logic_error("unexpected preexisting application clock publication");
        }
        phase = Phase::constructing;
        void* captured = nullptr;
        try {
            captured = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x80, 0x80});
        } catch (...) {
            phase = Phase::failed; // no allocation returned; do not call free
            throw;
        }
        if (!captured) {
            phase = Phase::unattempted; // native defensive skip; no fallback clock
            return;
        }
        try {
            construct_native_frame_clock_00bedfb0(captured, lifetime);
        } catch (...) {
            phase = Phase::failed;
            // Native C86A3E frees the captured allocation only. AB0 may retain
            // stale bits; never inspect, repair or clear it here.
            singleton_lifetime_free(captured);
            throw;
        }
        phase = Phase::constructed;
    }

    const NativeFrameClockPublicationContext& require_publication() const {
        if (phase != Phase::constructed)
            throw std::logic_error("application clock consumers require completed construction");
        return publication;
    }
};

void GameStartupHost::ensure_frame_clock_0073d480() {
    if (!clock_services_) {
        if (!native_data_)
            throw std::logic_error("application clock requires retained original profile data");
        auto pending = std::make_unique<ClockServices>(*native_data_, *singletons_, clock_publication_01090ab0_);
        // Install only a fully constructed persistent context, before the raw
        // owner can register with the existing manager. Preserve all other fields.
        singletons_->native_deletion_bindings().frame_clock = &pending->lifetime;
        clock_services_ = std::move(pending);
    }
    clock_services_->ensure();
}

const NativeFrameClockPublicationContext& GameStartupHost::require_frame_clock_context() const {
    if (!clock_services_)
        throw std::logic_error("application clock context is not established");
    return clock_services_->require_publication();
}

void GameStartupHost::exit_if_frame_clock_failed() noexcept {
    if (!clock_services_ || (clock_services_->phase != ClockServices::Phase::failed &&
                            clock_services_->phase != ClockServices::Phase::constructing)) return;
    try {
        log_.note("raw application clock construction failed; manager graph cleanup is unproved; retaining application and mapped data until process exit");
        log_.close();
    } catch (...) {
        std::fputs("bsp_game: failed raw clock construction requires process exit\n", stderr);
        std::fflush(stderr);
    }
    // Conservative source policy. Do not run manager/host/CRT cleanup against
    // a potentially dangling publication; this is not native FH3 equivalence.
    std::_Exit(1);
}

struct GameStartupHost::SoundServices {
    // Shared actual lifecycle publications. Native online construction occurs
    // at73DC7C and input backend construction at73DD8E, after sound/window.
    // Until those owners exist, these verified loader-zero slots stay null.
    // No independent online/input/action owner is constructed for sound loads.
    GameNativeSettingsProcess& settings_process;
    NativeOnlineManagerStorage* volatile& online_00f8abe8;
    NativeOnlinePumpContext* volatile online_pump{};
    std::uint8_t cursor_shown_0109db8e{}, focus_reset_pending_0109db8f{}, previous_ui_0109db90{};
    XLiveLibrary xlive;
    NativeOnlineSigninRuntime signin;
    NativeXLiveDeviceAdapter device_adapter;
    GamePlatformServices platform;
    void* volatile alternate_00f8bbcc{};
    const std::array<std::uint32_t, 4> format_counts_00e12ef0{0, 1, 2, 6};
    const volatile std::uint32_t one_00d7a24c{0x3f800000};
    const volatile double fade_00ce3dc8{0.30000001192092896};
    char null_integer_format_01090ab4{};
    const CameraAxesCrtAccess& crt{application_camera_axes_crt()};
    GameSoundDialogRuntime dialog;
    GameSoundRuntime core;

    explicit SoundServices(GameStartupHost& app)
        : settings_process(*app.settings_process_),online_00f8abe8(settings_process.online_00f8abe8()),
          xlive(selected_library_path(app.options_.xlive_dll, L"xlive.dll"),
              app.options_.xlive_dependencies),
          signin(xlive, app.require_frame_clock_context()),
          device_adapter(xlive),
          platform(app.input_backend_00f8bbf4_, app.input_runtime_, online_00f8abe8, xlive),
          dialog({alternate_00f8bbcc, format_counts_00e12ef0, one_00d7a24c,
              fade_00ce3dc8, &null_integer_format_01090ab4}),
          core({app.vfs_->context(), app.vfs_->search_registrations(),
              app.scripts_->files(), app.scripts_->runtime(), app.scripts_->globals(),
              app.require_frame_clock_context(), crt_string_storage(), platform.load_events(),
              app.singletons_->sound_lifetime(), crt, dialog,
              [this](void* owner, float seconds) { dialog.update(owner, seconds); },
              &alternate_00f8bbcc}, selected_library_path(app.options_.fmod_dll, L"fmodex.dll"),
              selected_library_path(app.options_.fmod_event_dll, L"fmod_event.dll")) {
        dialog.attach(core, core.fmod());
        settings_process.bind_profile_sdk(&xlive);
        // Registration happens in startup, after the raw deletion dispatcher
        // has this exact core allocation available.
        app.singletons_->bind_sound_runtime(&core);
    }
    ~SoundServices() {
        try{settings_process.bind_profile_sdk(nullptr);}
        catch(...){std::fputs("bsp_game: online owner outlived settings SDK binding\n",stderr);std::fflush(stderr);std::_Exit(1);}
    }
};

NativeOnlineSigninCalls* GameStartupHost::online_signin_calls() const noexcept {
    return sound_ ? &sound_->signin : nullptr;
}

struct GameStartupHost::InputServices {
    // Source storage initialized from the verified image words. Mutable settings
    // producers are still required; these cells are not snapshots of settings_view_.
    NativeInputDeviceSdk sdk;
    XInputLibrary& xinput;
    bool rumble_00e12f2c{true};
    // Source binding to the OS CPU+OS SSE2 capability service. The original CRT
    // initializer C27B7C calls __get_sse2_info, then writes this mode DWORD.
    const bool sse2_available{IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE};
    const volatile std::uint32_t sse2_0109eea4{sse2_available ? 1u : 0u};
    XInputDeviceGlobals tables{rumble_00e12f2c, sse2_available};
    volatile float one_00d7a24c{1.0f}, negative_zero_00d7a208{-0.0f};
    volatile float mouse_scale_00e12fb0{1.0f};
    volatile std::uint8_t invert_y_00f8bc04{};
    volatile double axis_divisor_00d7a220{100.0};
    volatile float unsigned_bias_00ce3978{4294967296.0f};
    volatile double milliseconds_00ce47a0{1000.0};
    const volatile double one_00d7a210{1.0};
    const volatile float trigger_threshold_00ce3800{0.5f}, activity_threshold_00ce3868{0.25f};
    const volatile double deadzone_00ce3d10{input_image_double(0x3fc99999a0000000ULL)};
    const volatile double deadzone_gain_00d5b7e8{input_image_double(0x3ff4000001400000ULL)};
    const volatile double activity_seconds_00ce3d68{60.0};
    const volatile double trigger_maximum_00ce4b48{255.0}, force_scale_00ce4bd8{10000.0};
    const volatile float zero_00d7a218{0.0f}, loading_step_00d7a2f0{0.1f};
    const volatile double infinite_remaining_00d7a278{input_image_double(0x47efffffe0000000ULL)};
    const char empty_00f8bc03{};
    // Explicit source stack baseline for the ignored-HRESULT Xbox joystick
    // branch. Arbitrary native uninitialized-stack failure behavior is unproven.
    const XINPUT_STATE joystick_stack_preimage{};
    NativeInputShowCursorCall const show_cursor{&::ShowCursor};
    GameInputRuntime core;

    explicit InputServices(GameStartupHost& app)
        : xinput(*app.xinput_library_),
          core({{crt_string_storage(), sdk, xinput, tables, g_active_platform,
              app.require_frame_clock_context(),
              {one_00d7a24c, negative_zero_00d7a208, mouse_scale_00e12fb0,
                  invert_y_00f8bc04, axis_divisor_00d7a220, unsigned_bias_00ce3978,
                  milliseconds_00ce47a0},
              {one_00d7a24c, one_00d7a210, trigger_threshold_00ce3800,
                  activity_threshold_00ce3868, deadzone_00ce3d10, deadzone_gain_00d5b7e8,
                  activity_seconds_00ce3d68, trigger_maximum_00ce4b48,
                  force_scale_00ce4bd8, sse2_0109eea4},
              {negative_zero_00d7a208, zero_00d7a218},
              {"Keyboard", "Mouse", "GameController"}, &empty_00f8bc03,
              infinite_remaining_00d7a278, joystick_stack_preimage},
              app.singletons_->sound_lifetime(), app.input_backend_00f8bbf4_,
              app.input_actions_00f8bbf8_,
              // Immutable D7A24C bits already used by sound's storage provider.
              // This is a literal representation, not an aliasing float cast.
              app.sound_->one_00d7a24c, app.input_listener_calls_,
              {app.sound_->cursor_shown_0109db8e, app.sound_->focus_reset_pending_0109db8f,
                  app.sound_->previous_ui_0109db90},
              app.sound_->online_00f8abe8, app.sound_->online_pump, app.sound_->xlive,
              loading_step_00d7a2f0, show_cursor,
              &gui_raw_input_device_004ba6d0, rumble_00e12f2c}) {
        app.singletons_->bind_input_backend(&core.backend_context());
        app.singletons_->bind_input_actions(&core.action_context());
    }
};

GameStartupHost::GameStartupHost(GameHostLog& log, HINSTANCE instance,
    const GameExecutableOptions& options, GameNativeReadOnlyData* native_data)
    : log_(log), instance_(instance), options_(options), native_data_(native_data) {
    auto singletons = std::make_unique<GameSingletonHost>(log_);
    // Represented CRT table order: CE2BAC -> CCD6A0 precedes CE3054 -> CD2D80.
    // Keep context/publication cells alive if later source construction fails.
    singletons->observers().initialize_dispatch_00ccd6a0();
    if(!native_data_)throw std::runtime_error("Native settings require retained verified data");
    try {
        settings_process_=&game_native_settings_process(*native_data_);
        const auto registration=settings_process_->initialize_once();
        copy_native_game_settings_read_view(settings_process_->settings(),{},settings_view_);
        log_.notef("native settings CRT owners initialized: storage=rawBCh/raw0Ch atexit=%d/%d/%d/%d online=process_cell input=process_cell",
            registration[0],registration[1],registration[2],registration[3]);
    }catch(...){
        log_.note("native settings CRT construction interrupted; retaining process contexts and partial ownership");log_.close();std::_Exit(1);
    }
    // The represented CRT entries then reach CE30C0/CD3910 and CE30C4/CD3940.
    // These are process owners, with real CRT shutdown callbacks, not members
    // of GameStartupHost. The native initializer table ignores returned status.
    const int destroy_registration = initialize_game_pending_destroy_owner_00cd3910();
    const int kill_registration = initialize_game_pending_kill_owner_00cd3940();
    const auto& pending = game_pending_entity_owners();
    log_.notef("pending entity CRT owners initialized: destroy=%s kill=%s "
        "counts=%u/%u atexit=%d/%d storage=process_raw24h",
        pending.destroy_00f899a8.head_04 != nullptr ? "present" : "null",
        pending.kill_00f899b4.head_04 != nullptr ? "present" : "null",
        pending.destroy_00f899a8.count_08, pending.kill_00f899b4.count_08,
        destroy_registration, kill_registration);
    // Later represented CRT subset: CD6E16..CD6F03. The owner retains the
    // verified tables, real locks and immutable IPC worker binding until exit.
    const auto online_root = options_.game_root.empty() ? std::filesystem::current_path()
        : std::filesystem::absolute(std::filesystem::path(options_.game_root));
    auto& online_process = game_native_online_process(
        online_root / "battlestationspacific.exe", {0, 0});
    const auto online_registration = online_process.initialize_once();
    log_.notef("native online CRT process initialized: tables=200980 fixed=568 "
        "atexit=%d/%d/%d/%d/%d/%d preimages=current_process ipc_stack=explicit_zero worker_binding=permanent",
        online_registration[0], online_registration[1], online_registration[2],
        online_registration[3], online_registration[4], online_registration[5]);
    singletons_ = singletons.release();
}

void GameStartupHost::exit_if_native_vfs_interrupted() noexcept {
    if (!vfs_ || !vfs_->requires_process_retention()) return;
    try {
        log_.notef("native application service interrupted; VFS failure site or resource operation entry=%08x (0=not recorded); cleanup is unrecovered; retaining application and mapped data until process exit",
            vfs_->failure_site());
        log_.close();
    } catch (...) {
        std::fputs("bsp_game: interrupted native application service requires process exit\n", stderr);
        std::fflush(stderr);
    }
    // std::exit would run the native physical-pool CRT cleanup against the
    // retained graph. _Exit performs no CRT cleanup; the OS reclaims it.
    std::_Exit(1);
}

void GameStartupHost::exit_if_native_lua_interrupted() noexcept {
    if (!lua_services_ || !lua_services_->native_operation_interrupted()) return;
    try {
        log_.note("native Lua fundamentals getter interrupted; partial native ownership is retained until process exit");
        log_.close();
    } catch (...) {
        std::fputs("bsp_game: interrupted native Lua getter requires process exit\n", stderr);
        std::fflush(stderr);
    }
    std::_Exit(1); // no guessed raw-manager rollback or CRT cleanup of partial state
}

void GameStartupHost::exit_if_native_renderer_incomplete() noexcept {
    if (!native_renderer_ || !native_renderer_->requires_process_retention()) return;
    try {
        log_.note("native renderer incomplete; actual destructor requires completed device/default surfaces, retaining native ownership until exit");
        log_.close();
    } catch (...) { std::fputs("bsp_game: incomplete native renderer requires process exit\n", stderr); }
    std::_Exit(1);
}

GameStartupHost::~GameStartupHost() {
    if(settings_host_&&settings_host_->requires_process_retention()) {
        try{log_.notef("native settings loader interrupted at %08x; retaining actual settings/catalog/VFS/renderer graph",settings_host_->failure_site());log_.close();}
        catch(...){std::fputs("bsp_game: native settings require process retention\n",stderr);std::fflush(stderr);}
        std::_Exit(1);
    }
    if(scripts_&&scripts_->input().requires_process_retention()) {
        try {log_.note("native input settings interrupted; retaining shared Lua/VFS/manager bindings until process exit");log_.close();}
        catch(...) {std::fputs("bsp_game: native input settings require process retention\n",stderr);std::fflush(stderr);}
        std::_Exit(1);
    }
    exit_if_native_renderer_incomplete();
    exit_if_native_lua_interrupted();
    exit_if_frame_clock_failed();
    exit_if_native_vfs_interrupted();
    delete loop_callbacks_;
    delete frame_host_;
    // The sprite bridge holds textures created on the device, so it goes before the
    // device and before the font host whose registry its pages reference.
    if (device_) device_->set_overlay(nullptr);
    delete menu_;
    delete frontend_;
    frontend_ = nullptr;
    // Retain sound callbacks, DLLs, VFS/Lua and lifetime publication cells
    // through the actual raw singleton drain, including exceptional startup.
    if (singletons_) {
        if (native_renderer_) native_renderer_->drain_singletons();
        else singletons_->shutdown();
        if(scripts_)scripts_->input().after_singleton_drain();
        if (clock_services_) clock_services_->phase = ClockServices::Phase::drained;
    }
    if (input_) {
        try { input_->core.release_sdk_after_native_drain(); }
        catch (const std::exception& error) {
            // Native registration failure can leave an unpublished-to-manager
            // allocation (or a freed backend publication). Do not dereference
            // or repair that cell, and do not terminate exceptional startup
            // merely because the explicit SDK-release precondition fails.
            log_.notef("input SDK final release incomplete: %s", error.what());
        }
    }
    input_runtime_ = nullptr;
    input_.reset();
    sound_.reset();
    delete singletons_;
    singletons_ = nullptr;
    clock_services_.reset(); // after GameSingletonHost's fallback destructor/drain
    delete profiler_;
    delete decals_;
    delete frontend_;
    delete locale_;
    delete fonts_;
    delete scripts_;
    lua_services_.reset(); // after every Lua close/shared drain, before VFS bindings die
    delete settings_host_;
    delete device_;
    renderer_parameters_ = nullptr;
    renderer_api_ = nullptr;
    native_renderer_.reset(); // all COM consumers closed; native owner already drained
    release_platform_window();
    delete window_host_;
    xinput_library_.reset();
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
    exit_if_native_renderer_incomplete();
    exit_if_native_lua_interrupted();
    exit_if_frame_clock_failed();
    exit_if_native_vfs_interrupted();
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
    // Harness: --instance-tag gives this run its own mutex name so runs from several
    // worktrees can overlap. Without the option the name is the image's and a second
    // instance takes WinMain's "already running" exit exactly as before.
    std::string tagged_name;
    if (!options_.instance_tag.empty()) {
        tagged_name = std::string(name) + "-" + options_.instance_tag;
        name = tagged_name.c_str();
        log_.notef("harness: single-instance mutex name suffixed with instance tag \"%s\"",
            options_.instance_tag.c_str());
    }
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
    // The image passes mask 1. Harness: --affinity-core moves the pin to another processor so
    // overlapping runs do not all share processor 0; the thread is still pinned to exactly one.
    DWORD_PTR mask = 1;
    if (options_.affinity_core >= 0) {
        mask = static_cast<DWORD_PTR>(1) << options_.affinity_core;
        log_.notef("harness: main thread pinned to processor %d instead of processor 0",
            options_.affinity_core);
    }
    SetThreadAffinityMask(GetCurrentThread(), mask);
}

void GameStartupHost::publish_game_resource_factory() {
    singletons_->publish_game_resource_factory_008f840b();
    log_.implemented("StartupHost::publish_game_resource_factory", "008f840b");
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

    ensure_frame_clock_0073d480();
    log_.implemented("Phase 0 ensure_actual_frame_clock", "0073d480");

    install_object_handle_resolvers_006ad0d0(object_resolvers_);
    log_.implemented("Phase 0 install_object_handle_resolvers", "006ad0d0");

    if(!native_data_)throw std::runtime_error("Native shader startup modes require verified data");
    auto& shader_process=game_native_shader_process();
    shader_process.configure_startup_modes(mode,*native_data_);
    const auto& shader_modes=shader_process.modes();
    log_.notef("native startup shader modes73D4C2: variants=%u source=%u hires=%u reload=%u pooled_copies=%u",
        static_cast<unsigned>(shader_modes.load_variants_0108d6f0),static_cast<unsigned>(shader_modes.source_mode_0108d6f1),
        static_cast<unsigned>(shader_modes.hires_mode_0108d4ba),static_cast<unsigned>(shader_modes.reload_resources_0108d4bb),
        shader_process.mode_operation().completed_copies);

    // Phase 2, VFS, mounts and packages (0073d604-0073d899). The hardware probe at 0073d610
    // is inside this gate, not before it; milestone 1 recorded it one phase early.
    if (!native_data_) throw std::runtime_error("Production VFS requires verified native data");
    vfs_ = new GameVfsHost(log_, *singletons_, *native_data_,
        std::filesystem::current_path() / "battlestationspacific.exe", options_.hardware_probe_commit);
    VfsStartupState vfs_state;
    vfs_->phase2(vfs_state);

    // Phase 3, platform, window and save storage (0073d8c0-0073d988).
    // The native image imports XInputEnable before WinMain. Keep the selected
    // source SDK alive before any CreateWindow activation messages, then lend
    // that same module to InputServices when native input startup is reached.
    xinput_library_ = std::make_unique<XInputLibrary>(selected_xinput_path(options_.xinput_dll));
    construct_win32_platform_00becda0(platform_, nullptr);
    log_.implemented("Phase 3 construct_win32_platform", "00becda0");
    set_active_platform_state(&platform_, &platform_text_);

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
    vfs_->factory_tail(vfs_state, command_line.cached_load);

    summary_.vfs_ready = vfs_->ready();
    summary_.mounts_requested = vfs_->mounts().size();
    summary_.mounts_created = 0;
    for (const GameMountRecord& mount : vfs_->mounts()) {
        if (std::strcmp(mount.status, "created") == 0) ++summary_.mounts_created;
    }
    summary_.package_scans_completed = vfs_->package_scans_completed();
    summary_.cached_load = vfs_->cached_load();
    log_.notef("vfs loose mounts requested=%zu created=%zu package scans=%zu",
        summary_.mounts_requested, summary_.mounts_created, summary_.package_scans_completed);

    // Renderer constructor0073da88, input getter0073da94/loader0073da9b,
    // then settings0073daa5. The same Direct3D API survives into device creation.
    if (!vfs_->ready()) throw std::runtime_error("Script/settings startup requires the mounted VFS");
    lua_services_ = std::make_unique<GameNativeLuaServices>(*singletons_, vfs_->borrow_raw_services());
    bind_legacy_crt_math_runtime(application_math_runtime);
    native_renderer_ = std::make_unique<GameNativeRendererApplication>(log_, *singletons_,
        *vfs_, *lua_services_, *native_data_, clock_publication_01090ab0_, &platform_.native_window_focus());
    native_renderer_->construct();
    renderer_api_ = &native_renderer_->api();
    renderer_parameters_ = &native_renderer_->parameters();
    native_renderer_->copy_settings_capabilities(renderer_capabilities_);
    log_.notef("renderer capabilities from actual owner api=%p pixel_version=0x%04x shader_ceiling=%d resolutions=%zu",
        renderer_api_,renderer_capabilities_.pixel_shader_version_28,renderer_capabilities_.max_shader_model,
        renderer_capabilities_.resolutions.size());

    scripts_ = new GameScriptHost(log_, vfs_->context(), content_suffixes_,
        *lua_services_,*singletons_,*native_data_,vfs_->borrow_raw_services());
    // Retain the source context before native getter/table execution begins.
    scripts_->input().load_tables();
    log_.implemented("Phase 5 input_script_tables", "005547d0/006ab6b0/006a7be0");
    log_.notef("native Lua fundamentals: published=%d getters=%u storage=raw0ch shared_globals=actual0ch",
        lua_services_->fundamentals_published() ? 1 : 0, lua_services_->fundamentals_getter_calls());
    const auto input_tables=scripts_->input().summary();
    summary_.input_scripts_ready = input_tables.tables_started;
    summary_.input_devices = input_tables.devices;
    summary_.input_names = input_tables.input_names;
    summary_.controller_names = input_tables.controller_names;

    // Phase 5, the settings block at 00f88980 filled by 008d8190 at 0073daa5. It runs before
    // window creation at 0073dc0f, which is the ordering constraint the whole phase exists
    // for: arguments 7, 8, 3, 4 and 9 of 00becee0 are read straight out of this block.
    settings_host_ = new GameNativeSettingsApplication(log_,*settings_process_,*singletons_,
        *vfs_,*native_renderer_,*native_data_,content_suffixes_,options_.settings_personal_root,{});
    auto& settings_host = *settings_host_;
    settings_host.load();
    settings_host.copy_read_view(settings_view_);
    if (command_line.fixed_frame_rate)
        enable_fixed_published_native_frame_clock(require_frame_clock_context(), 50);
    log_.implemented("Phase 5 load_game_settings", "008d8190");
    summary_.options_file_present = settings_host.options_file_present();
    summary_.options_path = settings_host.options_path();
    log_.notef("options file %s %s", settings_host.options_path().c_str(),
        summary_.options_file_present ? "loaded" : "absent, initial options write attempted");
    summary_.language = settings_view_.options_file.language;
    summary_.settings_width = settings_view_.options_file.width_14;
    summary_.settings_height = settings_view_.options_file.height_18;
    summary_.settings_fullscreen = settings_view_.options_file.fullscreen_1e;
    summary_.settings_vsync = settings_view_.options_file.vsync_60;
    summary_.settings_antialias = settings_view_.options_file.antialias_58;
    log_.notef("settings resolution=%dx%d index=%d fullscreen=%d vsync=%d antialias=%d "
        "shader_model=%d language=%s", settings_view_.options_file.width_14, settings_view_.options_file.height_18,
        settings_view_.options_file.resolution_index_78, settings_view_.options_file.fullscreen_1e ? 1 : 0,
        settings_view_.options_file.vsync_60 ? 1 : 0, settings_view_.options_file.antialias_58, settings_view_.options_file.shader_model_88,
        settings_view_.options_file.language.empty() ? "(none)" : settings_view_.options_file.language.c_str());

    bind_legacy_crt_math_runtime(application_math_runtime);
    sound_ = std::make_unique<SoundServices>(*this);
    try {
        sound_->core.startup(!settings_view_.audio.enabled_24);
    } catch (...) {
        // Read the existing SDK call journal; do not issue further FMOD calls
        // while reporting a constructor or its recovery failure.
        for (const auto& call : sound_->core.fmod().calls()) {
            if (call.result != FmodResult::ok)
                log_.notef("sound startup FMOD failure: %s result=%u",
                    call.function, static_cast<unsigned>(call.result));
        }
        throw;
    }
    log_.implemented("Phase 5 sound_system_initialize", "0073dafd");
    sound_->dialog.startup();
    log_.implemented("Phase 5 streamed_dialog_initialize", "0073db2b");

    // Native parser registration0073db41..db69 follows settings loading.
    vfs_->phase6(vfs_state);

    // Native73DB7E/73DB8E reload the same current alternate publication for
    // each direct store. Do not maintain separate music/speech gain copies.
    std::memcpy(static_cast<std::byte*>(sound_->alternate_00f8bbcc) + 0x218,
        &settings_view_.audio.music_28, sizeof(float));
    std::memcpy(static_cast<std::byte*>(sound_->alternate_00f8bbcc) + 0x21c,
        &settings_view_.audio.speech_30, sizeof(float));
    const auto sound_start = sound_->core.summary();
    log_.notef("sound startup before window: enabled=%d classes=%zu resources=%zu "
        "resource_bytes=%u opens=%zu closes=%zu fmod_calls=%zu fmod_errors=%zu "
        "load_pretranslations=%llu load_focus_calls=%llu",
        sound_start.sound_enabled ? 1 : 0, sound_start.classes, sound_start.resources,
        sound_start.resource_bytes, sound_start.file_opens, sound_start.file_closes,
        sound_start.fmod_calls, sound_start.fmod_errors,
        static_cast<unsigned long long>(sound_->platform.load_events().pretranslation_calls()),
        static_cast<unsigned long long>(sound_->platform.load_events().focus_calls()));

    // The three VFS reads the later milestones depend on: a GUI script, the locale table the
    // settings language selects, and one texture.
    if (vfs_ != nullptr && vfs_->ready()) {
        std::vector<std::string> probes{"interface/_common.lua"};
        probes.push_back("lockit/"
            + (settings_view_.options_file.language.empty() ? std::string("english") : settings_view_.options_file.language)
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
    request.fullscreen = settings_view_.options_file.fullscreen_1e;
    request.color_depth_selector = settings_view_.options_file.vsync_60;
    request.x = 0;
    request.y = 0;
    request.width = settings_view_.options_file.width_14;
    request.height = settings_view_.options_file.height_18;
    request.renderer_option = static_cast<std::uint32_t>(settings_view_.options_file.antialias_58);
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
        // Full B2AEB0 receives all ten slots, including VSync and multisampling.
        log_.notef("renderer init request %dx%d fullscreen=%d vsync=%d antialias=%u",
            renderer_request_.width, renderer_request_.height,
            renderer_request_.fullscreen ? 1 : 0,
            renderer_request_.color_depth_selector ? 1 : 0, renderer_request_.option);
        const auto* native_focus = &platform_.native_window_focus();
        log_.notef("platform native focus cells: hwnd_matches=%d active=%d focused=%u",
            native_platform_window_00bec230(native_focus) == platform_.window ? 1 : 0,
            platform_.byte_041 ? 1 : 0, native_platform_has_focus_00b20c50(native_focus));
    } else {
        log_.note("window creation failed");
    }

    // Device creation is inside BECEE0, before it returns to 0073DC27 and
    // before the online constructor at 0073DC7C. The separate 0073DD12 calls
    // material preloading (0073BF80), whose application composition is pending.
    native_renderer_->bind_platform_services(sound_->platform.load_events(),
        reinterpret_cast<const volatile std::uint32_t*>(&sound_->online_00f8abe8), &sound_->device_adapter,sound_->xlive);
    device_ = new GameDeviceHost(log_, *native_renderer_);
    if (summary_.window_created && renderer_request_.requested) {
        summary_.device_created = device_->create(renderer_request_);
        // Complete BED1E8..BED222 follows the native device call in BECEE0.
        // Its actual cache joins the same manager as the renderer and is
        // retired by the manager's current-profile scalar-deletion dispatch.
        native_renderer_->initialize_window_render_entry_cache();
        // BED223..BED276 is recovered, but production policy mutation awaits
        // the native restore/lifetime schedule; BECE30 only enables screensaver.
        log_.unimplemented("PlatformWindowHost::initialize_power_policy", "00bed223");
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
    // Native online construction follows the complete BECEE0 window/device/
    // cache/power sequence. The parent-process named-pipe peer and application
    // IPC owner composition remain unresolved. Keep its publication null.
    log_.unimplemented("Phase 5 online_manager_initialize", "0073dc7c");
    log_.unimplemented("Phase 4 renderer_resources", "00b14a10");
    input_ = std::make_unique<InputServices>(*this);
    input_runtime_ = &input_->core;
    input_runtime_->startup();
    log_.implemented("Phase 5 input_backend_initialize", "0073dd8e");
    log_.implemented("Phase 5 input_backend_callback_and_reset", "0073dd98");

    // Locale construction and exact setter/register/reload order0073e057..e135.
    locale_ = new GameLocaleHost(log_);
    locale_->initialize(settings_host.locale_source(), language_name_008d4870(
        settings_host.language_catalog(), static_cast<std::size_t>(settings_view_.gameplay.language_index_04)));
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
        settings_host_->language_catalog(), settings_view_.gameplay.language_index_04));
    // 0073E14B follows the retail-stubbed After InitGui checkpoint. Borrow the
    // canonical host cell and its shared raw lifetime domain; an earlier
    // diagnostic path may already have made this warm.
    (void)native_diagnostic_sink_get_or_create_004c14c0(
        singletons_->diagnostic_publication_0109cf14(), singletons_->sound_lifetime());
    summary_.fonts_loaded = fonts_->registry().fonts().size();
    summary_.font_resource_opens = fonts_->resource_opens();
    summary_.fingerprint_defined_bytes = fonts_->fingerprint().defined_size();

    // Phase 8, 00af0b10 at 0073e02b. The foliage group manager is reconstructed in
    // src/world_effects_startup.cpp; its critical section and its publication are not
    // bound here, so the phase itself is still a record.
    log_.unimplemented("Phase 8 world_effects_startup", "00af0b10");

    // Phase 9, 00740840 at 0073de8c. Milestone 2a's host table calls this "game_entry";
    // it is the decal definition loader, as docs/APP_INIT_TAIL.md establishes.
    decals_ = new GameDecalTable(log_, *vfs_);
    decals_->run();
    summary_.decal_definitions = decals_->summary().definitions;

    // GGame::OnInitTitle 004c9a70, reached from BSP_Game_BeginStartupSequence 004e5753.
    // It writes game state 2, selects front-end frame set 0 (FE_frame and
    // FE_frame_title through 00518250) and activates the title screen, whose handover
    // 0068d8d0 builds the press-start screen, registers it into registry slot 5Ch, marks
    // it wanted and active, commits it through 004f83b0 and enters it. The register
    // override 0067ca80 is what loads FE_initial, so all three title pages are now owned
    // rather than loaded directly as milestone 2b did.
    profiler_ = new GameFrameProfiler(log_, 8);
    menu_ = new GameMenuHost(log_, *frontend_, game_state_, *singletons_, options_.press_start_frame,
        *vfs_, *scripts_, locale_->tables(), options_.menu_select, options_.mission_frames,
        profiler_, summary_.language, options_.mission_complete_frame, options_.order_frame,
        options_.order_throttle, options_.order_rudder, options_.mission_frame_seconds,
        options_.trajectory_csv, options_.order_command, options_.order_command_target,
        options_.order_speed, options_.order_speed_set);
    // Milestone 2n, --order-unit <name>. It is a setter rather than another
    // constructor argument so the menu host, which this packet does not own,
    // keeps its signature.
    if (!options_.order_unit.empty() && menu_->mission() != nullptr) {
        menu_->mission()->set_order_unit(options_.order_unit);
    }
    // Milestone 2o, --ai-drive <name>=<throttle>,<rudder>, the same way.
    if (!options_.ai_drive_unit.empty() && menu_->mission() != nullptr) {
        menu_->mission()->set_ai_drive(options_.ai_drive_unit, options_.ai_drive_throttle,
            options_.ai_drive_rudder);
    }
    // The first-time OnInitOnce input fragment precedes OnInitTitle. A917E0
    // requests one wildcard slot per class and clears its active/filter ranges;
    // it does not activate devices. The remaining Lua/UI OnInitOnce calls are
    // separate initialization work, not implied by this fragment's log entry.
    input_runtime_->initialize_classes_004dd6a8();
    log_.implemented("OnInitOnce::configure_input_classes_fragment", "004dd6b4");
    menu_->run_title_init_004c9a70();
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
        // Milestone 2d: the text half. The locale tables phase 6 loaded are the last
        // input the reconstructed Text path needs, so every visible Text widget now
        // resolves its string id, lays it out and emits glyph quads.
        frontend_->open_text_bridge(locale_->tables());
        GameFrontendHost* frontend_host = frontend_;
        device_->set_overlay([frontend_host](IDirect3DDevice9& device) {
            frontend_host->draw_bridge(device);
        });
    }

    // The Init tail and the front end, for the run summary.
    summary_.hardware_probe_ran = vfs_->hardware_probe().ran;
    summary_.hardware_profile_values = vfs_->hardware_probe().stored_values_read;
    summary_.provider_factories = vfs_->registered_factories();
    summary_.resource_parsers = vfs_->registered_parsers();
    summary_.pak_registry = vfs_->pak_registry_published();
    summary_.pak_lock = vfs_->pak_lock_published();
    summary_.press_start_frame = options_.press_start_frame;
    summary_.screenshot_path = options_.screenshot_path;
    summary_.screenshot_frame = options_.screenshot_frame;

    frame_host_ = new GameFrameHost(log_, require_frame_clock_context(), platform_, loop_, game_state_, profiler_,
        menu_, *vfs_);
    std::function<void(IDirect3DDevice9&)> capture;
    if (!options_.screenshot_path.empty()) {
        GameFrontendHost* frontend_host = frontend_;
        std::string path = options_.screenshot_path;
        GameRunSummary* summary = &summary_;
        capture = [frontend_host, path, summary](IDirect3DDevice9& device) {
            summary->screenshot_written = frontend_host->save_back_buffer(device, path);
        };
    }
    loop_callbacks_ = new GameLoopCallbacks(log_, frame_state_, frame_color_, *frame_host_,
        *device_, loop_, options_.frame_limit, sound_->xlive, platform_, *input_runtime_, std::move(capture),
        options_.screenshot_frame, options_.screenshot_mission_frame);
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
    summary_.presents_skipped = device_ != nullptr ? device_->presents_skipped() : 0ull;
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
    exit_if_native_renderer_incomplete();
    exit_if_native_lua_interrupted();
    exit_if_frame_clock_failed();
    exit_if_native_vfs_interrupted();
    // Native00737f30's full singleton teardown remains unbound. Retained C++
    // input/locale/settings owners close later in dependency order at destruction.
    log_.implemented("StartupHost::application_shutdown", "00737f30");
    log_.unimplemented("ApplicationShutdownHost::singleton_teardown", "00737f80");
    // The sprite bridge owns managed-pool textures created on this device, so it is torn
    // down before the device is released rather than at destruction.
    if (menu_ != nullptr) {
        const GameMenuSummary& menu = menu_->summary();
        summary_.title_init_ran = menu.title_init_ran;
        summary_.press_start_registered = menu.press_start_registered;
        summary_.final_game_state = menu.game_state;
        summary_.pump_frames = menu.pump_frames;
        summary_.screen_enters = menu.screen_enters;
        summary_.screen_exits = menu.screen_exits;
        summary_.visibility_commits = menu.visibility_commits;
        summary_.press_start_injected = menu.press_start_injected;
        summary_.shell_entered = menu.shell_entered;
        summary_.main_menu_manager_active = menu.main_menu_manager_active;
        summary_.published_screen_id = menu.published_screen_id;
        summary_.path_step = menu.path_step;
        summary_.screens_registered = menu.screens_registered;
        const GameMissionHost* mission = menu_->mission();
        if (mission != nullptr) {
            const GameMissionSummary& path = mission->summary();
            summary_.menu_select = path.requested_id;
            summary_.mission_tree_loaded = path.tree_loaded;
            summary_.mission_tree_groups = path.tree_groups;
            summary_.mission_tree_missions = path.tree_missions;
            summary_.mission_selected_id = path.selected_id;
            summary_.mission_list_page = path.list_page;
            summary_.mission_detail_built = path.detail_built;
            summary_.mission_start_requested = path.load_requested;
            summary_.mission_scene_record = path.scene_record_built;
            summary_.mission_scene_path = path.scene_path;
            summary_.mission_scene_entities = path.scene_entities;
            summary_.mission_scene_classes = path.scene_distinct_classes;
            summary_.mission_load_host_steps = path.load_host_steps;
            summary_.mission_load_stopped_at = path.load_stopped_at;
            summary_.mission_step = game_mission_step_name(path.step);
            // Milestone 2f
            summary_.mission_load_finished = path.mission_load_finished;
            summary_.mission_load_concrete = path.mission_load_concrete;
            summary_.mission_load_records = path.mission_load_records;
            summary_.mission_game_state = path.mission_game_state;
            summary_.mission_entered = path.mission_entered;
            summary_.mission_frames_requested = path.mission_frames_requested;
            summary_.mission_frames_run = path.mission_frames_run;
            summary_.mission_frames_simulated = path.mission_frames_simulated;
            summary_.mission_lua_bindings = path.lua_bindings;
            summary_.mission_lua_natives = path.lua_natives;
            summary_.mission_lua_native_calls = path.lua_native_calls;
            summary_.mission_script_path = path.lua_script_path;
            summary_.mission_exit_note = path.mission_exit_note;
            summary_.mission_exit_frames = path.mission_exit_frames;
            summary_.mission_exit_completed = path.mission_exit_completed;
            summary_.mission_complete_injected = path.mission_complete_injected;
        }
    }
    if (frontend_ != nullptr) {
        const GameFrontendSummary& frontend = frontend_->summary();
        summary_.gui_bridge_open = frontend.bridge_open;
        summary_.gui_bridge_atlas = frontend.bridge_atlas;
        summary_.gui_bridge_atlas_items = frontend.bridge_atlas_items;
        summary_.gui_bridge_textures = frontend.bridge_textures;
        summary_.gui_bridge_quads = frontend.bridge_quads;
        summary_.gui_bridge_frames = frontend.bridge_frames;
        summary_.gui_pages_loaded = frontend.pages_loaded;
        summary_.gui_pages_requested = frontend.pages_requested;
        summary_.gui_widgets = frontend.widgets;
        summary_.gui_widgets_with_texture = frontend.widgets_with_texture;
        summary_.screen_owned_pages = frontend.screen_owned_pages;
        summary_.text_bridge_open = frontend.text_bridge_open;
        summary_.text_widgets = frontend.text_widgets;
        summary_.text_runs = frontend.text_runs;
        summary_.text_glyphs = frontend.text_glyphs;
        summary_.text_quads = frontend.text_quads;
    }
    // The menu host holds the front-end host by reference, so it goes first.
    delete menu_;
    menu_ = nullptr;
    delete frontend_;
    frontend_ = nullptr;
    if (device_ != nullptr) device_->release();
    // Native 0073830F is step 19 of the broader 00737F30 teardown. The other
    // recovered steps remain unbound above, but this explicit owner retirement
    // must precede application destruction and the shared raw-manager drain.
    destroy_native_diagnostic_sink_007363b0(
        singletons_->diagnostic_publication_0109cf14(), singletons_->sound_lifetime());
}

void GameStartupHost::application_destruct() {
    log_.implemented("StartupHost::application_destruct", "008f8444");
    constructed_ = false;
}

void GameStartupHost::destroy_singleton_lifetime_manager() {
    exit_if_native_renderer_incomplete();
    exit_if_native_lua_interrupted();
    exit_if_frame_clock_failed();
    exit_if_native_vfs_interrupted();
    log_.implemented("StartupHost::destroy_singleton_lifetime_manager", "008f8449");
    if (native_renderer_) native_renderer_->drain_singletons();
    else singletons_->shutdown();
    if (lua_services_)
        log_.notef("native Lua after raw singleton drain: fundamentals=%s getters=%u",
            lua_services_->fundamentals_published() ? "non-null" : "null",
            lua_services_->fundamentals_getter_calls());
    if (clock_services_) clock_services_->phase = ClockServices::Phase::drained;
    if (input_) input_->core.release_sdk_after_native_drain();
    if (sound_) {
        const auto state = sound_->core.summary();
        log_.notef("sound after raw singleton drain: started=%d samples=%zu resources=%zu "
            "classes=%zu opens=%zu sdk_closes=%zu host_reclaims=%zu pending=%zu fmod_errors=%zu",
            state.started ? 1 : 0, state.samples, state.resources, state.classes,
            state.file_opens, state.file_closes, state.file_reclaims,
            state.file_handles_pending, state.fmod_errors);
    }
    release_platform_window();
}

}  // namespace bsp::game
