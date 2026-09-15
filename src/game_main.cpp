// bsp_game.exe, milestones 1 through 2c: the reconstructed startup spine running as a
// Win32 process, with the phase-2 virtual file system and hardware probe, the phase-5
// settings load, the phase-6 locale tables and parser registrations, the phase-7 fonts
// and GUI startup, the phase-9 decal definitions, the title bring-up 004c9a70 and the
// front-end screen registry all doing real work.
//
// Entry sequence 008f81f0 (docs/WINMAIN_STARTUP.md) drives the whole run. Everything this
// file adds on top of run_win_main is process plumbing: the option parsing for --frames,
// --log, --game-root and --vfs-probe, the console attachment that lets a windowed process
// report to the terminal that started it, and the exit code. Evidence:
// docs/GAME_EXECUTABLE.md.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#include <wincrypt.h>

#include <cfloat>
#include <array>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include "bsp/game_native_mutable_crt_data.hpp"
#include "bsp/winmain_startup.hpp"

namespace {

// A WIN32-subsystem process has no console of its own. When it was started from a shell,
// borrow that console so the run log is visible where the command was typed.
void attach_parent_console() {
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) return;
    FILE* stream = nullptr;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
}

// These explicit spans select the four original read-only 64-KB bands consumed by
// the raw VFS entry path. The mapper verifies the entire original PE before it
// admits any table/literal read in those bands.
constexpr std::array<bsp::game::GameNativeDataSpan, 4> native_data_spans{{
    {0x00cf0000u, 1}, {0x00d10000u, 1}, {0x00d50000u, 1}, {0x00d60000u, 1}
}};
constexpr char handoff_prefix[] = "--bsp-native-data-handoff=";

// The bootstrap appends this token last. Exclude it before the public parser
// sees argv, including when a public option is missing its required value.
bool is_handoff_child() {
    if (__argc < 2) return false;
    const char* const token = __argv[__argc - 1];
    const std::size_t prefix_size = sizeof(handoff_prefix) - 1;
    if (std::strncmp(token, handoff_prefix, prefix_size) != 0 ||
        std::strlen(token + prefix_size) != 8) return false;
    unsigned long value = 0;
    for (const char* digit = token + prefix_size; *digit; ++digit) {
        const unsigned nibble = (*digit >= '0' && *digit <= '9') ? *digit - '0' :
            (*digit >= 'A' && *digit <= 'F') ? *digit - 'A' + 10 :
            (*digit >= 'a' && *digit <= 'f') ? *digit - 'a' + 10 : 16;
        if (nibble == 16) return false;
        value = (value << 4) | nibble;
    }
    DWORD flags = 0;
    return GetHandleInformation(reinterpret_cast<HANDLE>(value), &flags) &&
        (flags & HANDLE_FLAG_INHERIT);
}

// Windows command-line quoting: double backslashes before quotes and before
// the closing quote. Rebuild the public argument vector without changing its
// token boundaries, then let the bootstrap append its private handle token.
std::wstring quote_argument(const std::wstring& argument) {
    std::wstring quoted(1, L'"');
    std::size_t slashes = 0;
    for (const wchar_t ch : argument) {
        if (ch == L'\\') { ++slashes; continue; }
        if (ch == L'"') {
            quoted.append(slashes * 2 + 1, L'\\');
            quoted.push_back(ch);
        } else {
            quoted.append(slashes, L'\\');
            quoted.push_back(ch);
        }
        slashes = 0;
    }
    quoted.append(slashes * 2, L'\\');
    quoted.push_back(L'"');
    return quoted;
}

std::wstring child_arguments() {
    int count = 0;
    LPWSTR* const arguments = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!arguments) throw std::runtime_error("Cannot read bsp_game command line");
    std::wstring result;
    try {
        for (int index = 1; index < count; ++index) {
            if (std::wcsncmp(arguments[index], L"--bsp-native-data-handoff=", 26) == 0)
                throw std::invalid_argument("The --bsp-native-data-handoff= prefix is reserved for the child bootstrap");
            if (!result.empty()) result.push_back(L' ');
            result += quote_argument(arguments[index]);
        }
    } catch (...) { LocalFree(arguments); throw; }
    LocalFree(arguments);
    return result;
}

std::filesystem::path own_executable() {
    std::wstring path(MAX_PATH, L'\0');
    for (;;) {
        const DWORD size = GetModuleFileNameW(nullptr, path.data(),
            static_cast<DWORD>(path.size()));
        if (!size) throw std::runtime_error("Cannot resolve bsp_game executable");
        if (size < path.size()) { path.resize(size); return path; }
        if (path.size() > 32768) throw std::runtime_error("bsp_game executable path is too long");
        path.resize(path.size() * 2);
    }
}

std::filesystem::path original_executable(const bsp::game::GameExecutableOptions& options) {
    const auto root = options.game_root.empty() ? std::filesystem::current_path()
        : std::filesystem::absolute(std::filesystem::path(options.game_root));
    const auto image = root / "battlestationspacific.exe";
    if (!std::filesystem::is_regular_file(image))
        throw std::runtime_error("Supported original battlestationspacific.exe is absent from game root: "
            + image.string());
    return image;
}

void check_original_identity(const std::filesystem::path& image) {
    constexpr std::uintmax_t supported_size = 12223752;
    constexpr std::array<BYTE, 32> supported_sha256{{
        0xb6, 0x82, 0xa8, 0x2c, 0x52, 0xf8, 0x1f, 0x95,
        0x7b, 0x2c, 0x70, 0x22, 0x20, 0x77, 0x30, 0x5a,
        0x93, 0x3f, 0x72, 0x48, 0x16, 0x86, 0xc8, 0x88,
        0x43, 0x07, 0x7f, 0x71, 0x4b, 0x95, 0x6d, 0xd6
    }};
    if (std::filesystem::file_size(image) != supported_size)
        throw std::runtime_error("Original executable has an unsupported size: " +
            image.string());
    std::ifstream file(image, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot read original executable: " + image.string());
    struct HashContext {
        HCRYPTPROV provider = 0;
        HCRYPTHASH hash = 0;
        ~HashContext() {
            if (hash) CryptDestroyHash(hash);
            if (provider) CryptReleaseContext(provider, 0);
        }
    } context;
    if (!CryptAcquireContextW(&context.provider, nullptr, nullptr, PROV_RSA_AES,
            CRYPT_VERIFYCONTEXT) ||
        !CryptCreateHash(context.provider, CALG_SHA_256, 0, 0, &context.hash))
        throw std::runtime_error("Cannot initialize original executable SHA-256 check");
    std::array<char, 65536> bytes{};
    for (;;) {
        file.read(bytes.data(), bytes.size());
        const std::streamsize count = file.gcount();
        if (count > 0 && !CryptHashData(context.hash,
                reinterpret_cast<const BYTE*>(bytes.data()), static_cast<DWORD>(count), 0))
            throw std::runtime_error("Cannot hash original executable");
        if (count < static_cast<std::streamsize>(bytes.size())) {
            if (!file.eof()) throw std::runtime_error("Cannot read entire original executable");
            break;
        }
    }
    std::array<BYTE, 32> digest{};
    DWORD size = static_cast<DWORD>(digest.size());
    if (!CryptGetHashParam(context.hash, HP_HASHVAL, digest.data(), &size, 0) ||
        size != digest.size())
        throw std::runtime_error("Cannot finish original executable SHA-256 check");
    if (digest != supported_sha256)
        throw std::runtime_error("Original executable SHA-256 is unsupported: " +
            image.string());
}

int run_bootstrap_parent(const bsp::game::GameExecutableOptions& options) {
    check_original_identity(original_executable(options));
    const auto child_path = own_executable();
    bsp::game::GameNativeDataBootstrapChild child(child_path, child_arguments(),
        native_data_spans.data(), native_data_spans.size(),
        bsp::game::GameNativeDataPlan::CanonicalCrtV2);
    child.reserve_and_resume();
    child.wait_for_mapping(30000);
    const HANDLE process = static_cast<HANDLE>(child.process_handle());
    if (WaitForSingleObject(process, INFINITE) != WAIT_OBJECT_0)
        throw std::runtime_error("Cannot wait for native-data child exit");
    DWORD exit_code = 0;
    if (!GetExitCodeProcess(process, &exit_code))
        throw std::runtime_error("Cannot read native-data child exit code");
    return static_cast<int>(exit_code);
}

void report_summary(bsp::game::GameHostLog& log, const bsp::game::GameRunSummary& summary) {
    log.notef("summary window_created=%d device_created=%d device_hr=0x%08lx "
        "back_buffer=%ux%u frames_presented=%llu loop_finished=%d exit_code=%d",
        summary.window_created ? 1 : 0, summary.device_created ? 1 : 0,
        static_cast<unsigned long>(summary.device_result), summary.back_buffer_width,
        summary.back_buffer_height, summary.frames_presented,
        summary.loop_finished ? 1 : 0, summary.exit_code);
    log.notef("summary vfs_ready=%d loose_mounts=%zu/%zu package_scans=%zu "
        "cachedload=%d probes=%zu/%zu", summary.vfs_ready ? 1 : 0, summary.mounts_created,
        summary.mounts_requested, summary.package_scans_completed,
        summary.cached_load ? 1 : 0, summary.probes_resolved, summary.probes_requested);
    log.notef("summary options_file=%d path=%s language=%s resolution=%dx%d fullscreen=%d "
        "vsync=%d antialias=%d", summary.options_file_present ? 1 : 0,
        summary.options_path.empty() ? "(none)" : summary.options_path.c_str(),
        summary.language.empty() ? "(none)" : summary.language.c_str(),
        summary.settings_width, summary.settings_height,
        summary.settings_fullscreen ? 1 : 0, summary.settings_vsync ? 1 : 0,
        summary.settings_antialias);
    log.notef("summary input_scripts_ready=%d devices=%zu input_names=%zu controller_names=%zu "
        "renderer_api_shared=%d locale_keys=%zu locale_files=%zu",
        summary.input_scripts_ready ? 1 : 0, summary.input_devices, summary.input_names,
        summary.controller_names, summary.renderer_api_shared ? 1 : 0,
        summary.locale_keys, summary.locale_files);
    log.notef("summary fonts_loaded=%zu font_resource_opens=%zu fingerprint_defined_bytes=%zu",
        summary.fonts_loaded, summary.font_resource_opens, summary.fingerprint_defined_bytes);
    log.notef("summary gui_resources=%zu pages=%zu/%zu widgets=%zu widgets_with_texture=%zu",
        summary.gui_resources_acquired, summary.gui_pages_loaded, summary.gui_pages_requested,
        summary.gui_widgets, summary.gui_widgets_with_texture);
    log.notef("summary bridge_open=%d atlas=%s atlas_items=%zu textures=%zu quads=%zu "
        "frames=%llu", summary.gui_bridge_open ? 1 : 0,
        summary.gui_bridge_atlas.empty() ? "(none)" : summary.gui_bridge_atlas.c_str(),
        summary.gui_bridge_atlas_items, summary.gui_bridge_textures, summary.gui_bridge_quads,
        summary.gui_bridge_frames);
    log.notef("summary init_tail hardware_probe=%d stored_values=%d factories=%zu "
        "parsers=%zu pak_registry=%d pak_lock=%d decals=%zu",
        summary.hardware_probe_ran ? 1 : 0, summary.hardware_profile_values,
        summary.provider_factories, summary.resource_parsers,
        summary.pak_registry ? 1 : 0, summary.pak_lock ? 1 : 0,
        summary.decal_definitions);
    log.notef("summary frontend title_init=%d press_start_slot=%d screens=%zu "
        "screen_pages=%zu pump_frames=%llu enters=%zu exits=%zu commits=%zu",
        summary.title_init_ran ? 1 : 0, summary.press_start_registered ? 0x5C : -1,
        summary.screens_registered, summary.screen_owned_pages, summary.pump_frames,
        summary.screen_enters, summary.screen_exits, summary.visibility_commits);
    log.notef("summary mainmenu press_start_frame=%ld injected=%d shell=%d manager=%d "
        "screen=%d step=%s state=%d", summary.press_start_frame,
        summary.press_start_injected ? 1 : 0, summary.shell_entered ? 1 : 0,
        summary.main_menu_manager_active ? 1 : 0, summary.published_screen_id,
        summary.path_step.empty() ? "PressStartPoll" : summary.path_step.c_str(),
        summary.final_game_state);
    log.notef("summary text bridge=%d widgets=%zu runs=%zu glyphs=%zu quads=%zu",
        summary.text_bridge_open ? 1 : 0, summary.text_widgets, summary.text_runs,
        summary.text_glyphs, summary.text_quads);
    if (!summary.menu_select.empty()) {
        log.notef("summary mission select=%s tree=%d groups=%zu missions=%zu selected=%s "
            "list_page=%d detail=%d requested=%d step=%s", summary.menu_select.c_str(),
            summary.mission_tree_loaded ? 1 : 0, summary.mission_tree_groups,
            summary.mission_tree_missions,
            summary.mission_selected_id.empty() ? "(none)"
                                                : summary.mission_selected_id.c_str(),
            summary.mission_list_page, summary.mission_detail_built ? 1 : 0,
            summary.mission_start_requested ? 1 : 0,
            summary.mission_step.empty() ? "Idle" : summary.mission_step.c_str());
        log.notef("summary mission scene record=%d path=%s entities=%zu classes=%zu "
            "load_steps=%zu stopped_at=%s", summary.mission_scene_record ? 1 : 0,
            summary.mission_scene_path.empty() ? "(none)"
                                               : summary.mission_scene_path.c_str(),
            summary.mission_scene_entities, summary.mission_scene_classes,
            summary.mission_load_host_steps,
            summary.mission_load_stopped_at.empty() ? "(nothing)"
                                                    : summary.mission_load_stopped_at.c_str());
        log.notef("summary mission load finished=%d concrete=%zu records=%zu state=0x%02X "
            "entered=%d script=%s", summary.mission_load_finished ? 1 : 0,
            summary.mission_load_concrete, summary.mission_load_records,
            summary.mission_game_state, summary.mission_entered ? 1 : 0,
            summary.mission_script_path.empty() ? "(none)"
                                                : summary.mission_script_path.c_str());
        log.notef("summary mission lua bindings=%zu natives=%zu calls=%llu",
            summary.mission_lua_bindings, summary.mission_lua_natives,
            summary.mission_lua_native_calls);
        log.notef("summary mission frames requested=%ld ran=%llu simulated=%llu exit=%s",
            summary.mission_frames_requested, summary.mission_frames_run,
            summary.mission_frames_simulated,
            summary.mission_exit_note.empty() ? "(not reached)"
                                              : summary.mission_exit_note.c_str());
        log.notef("summary mission exit injected=%d frames=%llu completed=%d",
            summary.mission_complete_injected ? 1 : 0, summary.mission_exit_frames,
            summary.mission_exit_completed ? 1 : 0);
    }
    if (!summary.screenshot_path.empty()) {
        log.notef("summary screenshot=%d frame=%ld path=%s",
            summary.screenshot_written ? 1 : 0, summary.screenshot_frame,
            summary.screenshot_path.c_str());
    }
    log.notef("host methods %zu concrete, %zu unimplemented",
        log.implemented_count(), log.unimplemented_count());
    for (const auto& record : log.records()) {
        log.notef("  %-56s %-20s %-13s calls=%llu", record.method.c_str(),
            record.native_address.c_str(), record.implemented ? "concrete" : "UNIMPLEMENTED",
            record.calls);
    }
}

}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line,
    int show_command) {
    attach_parent_console();

    const bool handoff_child = is_handoff_child();
    bsp::game::GameExecutableOptions options;
    std::string error;
    if (!options.parse(__argc - (handoff_child ? 1 : 0), __argv, error)) {
        std::fprintf(stderr, "bsp_game: %s\n", error.c_str());
        std::fprintf(stderr, "usage: bsp_game.exe [--frames N] [--log <path>]"
            " [--game-root <dir>] [--settings-personal-root <dir>] [--vfs-probe <virtual path>]"
            " [--press-start-frame N] [--menu-select <mission id>] [--mission-frames N]"
            " [--mission-complete-frame N]"
            " [--order throttle=<f>,rudder=<f> | --order <command>[:<entity>]"
            " | --order <command>=<x>,<z>]"
            " [--order-unit <name>] [--ai-drive <unit>=<throttle>,<rudder>]"
            " [--order-frame N] [--mission-frame-seconds S]"
            " [--trajectory-csv <path>]"
            " [--screenshot <path>] [--screenshot-frame N]"
            " [--screenshot-mission-frame N] [--hardware-probe-commit]\n");
        return 2;
    }

    if (!handoff_child) {
        try { return run_bootstrap_parent(options); }
        catch (const std::exception& failure) {
            std::fprintf(stderr, "bsp_game: native-data bootstrap failed: %s\n",
                failure.what());
            return 2;
        }
    }

    std::filesystem::path original_image;
    try { original_image = original_executable(options); }
    catch (const std::exception& failure) {
        std::fprintf(stderr, "bsp_game: native-data source failed: %s\n", failure.what());
        return 2;
    }

    bsp::game::GameHostLog log;
    if (!log.open(options.log_path)) {
        std::fprintf(stderr, "bsp_game: cannot write log %s\n", options.log_path.c_str());
        return 2;
    }

    bsp::game::GameNativeReadOnlyData* native_data = nullptr;
    try {
        auto reservation = bsp::game::accept_native_data_handoff(
            native_data_spans.data(), native_data_spans.size(),
            bsp::game::GameNativeDataPlan::CanonicalCrtV2);
        const auto& owner = bsp::game::GameNativeCanonicalDataOwner::initialize(original_image,
            native_data_spans.data(), native_data_spans.size(), std::move(reservation));
        native_data = &owner.read_only_data();
        log.notef("native-data handoff mapped verified original image %s",
            original_image.string().c_str());
    } catch (const std::exception& failure) {
        std::fprintf(stderr, "bsp_game: native-data handoff failed: %s\n", failure.what());
        log.notef("native-data handoff failed: %s", failure.what());
        log.close();
        return 2;
    }
    // docs/X87_CONTROL_WORD.md establishes statically that the CRT startup sets
    // the x87 precision field to 53 bits (`__setdefaultprecision` asks for
    // _PC_53 under _MCW_PC at 00c0683c) and that no game code changes it again,
    // and that the device is created without D3DCREATE_FPU_PRESERVE so d3d9.dll
    // drops the field to 24 bits for the life of the device. This is the first
    // half of that read taken at run time: the mode before Direct3D exists. The
    // second is taken at the first fixed simulation step. Neither changes any
    // arithmetic; both are observations.
    const unsigned long precision_before_d3d = bsp::game::x87_precision_field();
    log.notef("x87 precision before Direct3D: %s (_controlfp_s & _MCW_PC = 0x%08lx)",
        bsp::game::x87_precision_name(precision_before_d3d), precision_before_d3d);
    log.notef("bsp_game milestone 2l, frames=%ld press_start_frame=%ld screenshot_frame=%ld "
        "screenshot_mission_frame=%ld menu_select=%s mission_frames=%ld "
        "mission_complete_frame=%ld order_frame=%ld order=throttle %.3f rudder %.3f "
        "mission_frame_seconds=%.4f trajectory_csv=%s log=%s",
        options.frame_limit, options.press_start_frame, options.screenshot_frame,
        options.screenshot_mission_frame,
        options.menu_select.empty() ? "(none)" : options.menu_select.c_str(),
        options.mission_frames, options.mission_complete_frame, options.order_frame,
        static_cast<double>(options.order_throttle),
        static_cast<double>(options.order_rudder),
        static_cast<double>(options.mission_frame_seconds),
        options.trajectory_csv.empty() ? "(none)" : options.trajectory_csv.c_str(),
        options.log_path.empty() ? "(stdout only)" : options.log_path.c_str());
    if (!options.ai_drive_unit.empty()) {
        log.notef("--ai-drive %s=%.3f,%.3f: a labelled diagnostic stand-in for the ship AI "
            "state step, engaged on --order-frame. It substitutes nothing after the two "
            "setters 009dbf90 / 009dffb0",
            options.ai_drive_unit.c_str(), static_cast<double>(options.ai_drive_throttle),
            static_cast<double>(options.ai_drive_rudder));
    }

    // The phase-2 mounts use GetCurrentDirectoryA at 0073d697, so pointing the run at an
    // installed game means setting the process current directory, not injecting a path. The
    // log is already open, so a relative --log path stays relative to the invoking shell.
    if (!options.game_root.empty()) {
        if (!SetCurrentDirectoryA(options.game_root.c_str())) {
            log.notef("cannot enter game root %s", options.game_root.c_str());
            log.close();
            return 2;
        }
        log.notef("game root %s", options.game_root.c_str());
    }

    std::unique_ptr<bsp::game::GameStartupHost> host;
    try { host = std::make_unique<bsp::game::GameStartupHost>(
        log, instance, options, native_data); }
    catch (const std::exception& failure) {
        std::fprintf(stderr, "bsp_game: startup host failed: %s\n", failure.what());
        log.notef("startup host failed: %s", failure.what());
        log.close();
        return 1;
    }

    // 008f81f0 reads none of its four arguments; they are recorded for completeness.
    bsp::WinMainArguments arguments;
    arguments.instance = instance;
    arguments.previous_instance = previous_instance;
    arguments.command_line = command_line;
    arguments.show_command = show_command;

    int result = 1;
    try {
        result = bsp::run_win_main(arguments, *host);
    } catch (const std::exception& error) {
        log.notef("startup failed: %s", error.what());
    }

    host->exit_if_native_vfs_interrupted();
    bsp::game::GameRunSummary summary = host->summary();
    summary.exit_code = result;
    // A run that asked for a frame count only succeeds when the device presented them.
    // Milestone 2g is the one exception: a mission that ended through the debrief path
    // finishes the run where the game does, which is before the frame count is reached.
    if (options.frame_limit > 0 && !summary.mission_exit_completed
        && summary.frames_presented < static_cast<unsigned long long>(options.frame_limit)) {
        summary.exit_code = 1;
    }
    // A --vfs-probe that did not read bytes fails the run, so a scripted check needs only the
    // exit code. The three probes the milestone always performs do not affect it.
    if (summary.exit_code == 0 && host->vfs() != nullptr) {
        for (const std::string& requested : options.vfs_probes) {
            for (const auto& probe : host->vfs()->probes()) {
                if (probe.requested == requested && !probe.opened) summary.exit_code = 3;
            }
        }
    }
    report_summary(log, summary);
    host.reset(); // The canonical RO/RW owner remains mapped through process teardown.
    log.close();
    return summary.exit_code;
}
