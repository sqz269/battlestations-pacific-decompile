// bsp_game.exe, milestones 1, 2a and 2b: the reconstructed startup spine running as a Win32
// process, with the phase-2 virtual file system, the phase-5 settings load, the phase-6
// locale tables and the phase-7 fonts and GUI startup all doing real work.
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

#include <cstdio>
#include <string>
#include <exception>

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
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

void report_summary(bsp::game::GameHostLog& log, const bsp::game::GameRunSummary& summary) {
    log.notef("summary window_created=%d device_created=%d device_hr=0x%08lx "
        "back_buffer=%ux%u frames_presented=%llu loop_finished=%d exit_code=%d",
        summary.window_created ? 1 : 0, summary.device_created ? 1 : 0,
        static_cast<unsigned long>(summary.device_result), summary.back_buffer_width,
        summary.back_buffer_height, summary.frames_presented,
        summary.loop_finished ? 1 : 0, summary.exit_code);
    log.notef("summary vfs_ready=%d mounts=%zu/%zu package_entries=%zu package_mounts=%zu "
        "cachedload=%d probes=%zu/%zu", summary.vfs_ready ? 1 : 0, summary.mounts_created,
        summary.mounts_requested, summary.package_entries, summary.package_mounts,
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

    bsp::game::GameExecutableOptions options;
    std::string error;
    if (!options.parse(__argc, __argv, error)) {
        std::fprintf(stderr, "bsp_game: %s\n", error.c_str());
        std::fprintf(stderr, "usage: bsp_game.exe [--frames N] [--log <path>]"
            " [--game-root <dir>] [--settings-personal-root <dir>] [--vfs-probe <virtual path>]\n");
        return 2;
    }

    bsp::game::GameHostLog log;
    if (!log.open(options.log_path)) {
        std::fprintf(stderr, "bsp_game: cannot write log %s\n", options.log_path.c_str());
        return 2;
    }
    log.notef("bsp_game milestone 2b, frames=%ld log=%s", options.frame_limit,
        options.log_path.empty() ? "(stdout only)" : options.log_path.c_str());

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

    bsp::game::GameStartupHost host(log, instance, options);

    // 008f81f0 reads none of its four arguments; they are recorded for completeness.
    bsp::WinMainArguments arguments;
    arguments.instance = instance;
    arguments.previous_instance = previous_instance;
    arguments.command_line = command_line;
    arguments.show_command = show_command;

    int result = 1;
    try {
        result = bsp::run_win_main(arguments, host);
    } catch (const std::exception& error) {
        log.notef("startup failed: %s", error.what());
    }

    bsp::game::GameRunSummary summary = host.summary();
    summary.exit_code = result;
    // A run that asked for a frame count only succeeds when the device presented them.
    if (options.frame_limit > 0
        && summary.frames_presented < static_cast<unsigned long long>(options.frame_limit)) {
        summary.exit_code = 1;
    }
    // A --vfs-probe that did not read bytes fails the run, so a scripted check needs only the
    // exit code. The three probes the milestone always performs do not affect it.
    if (summary.exit_code == 0 && host.vfs() != nullptr) {
        for (const std::string& requested : options.vfs_probes) {
            for (const auto& probe : host.vfs()->probes()) {
                if (probe.requested == requested && !probe.opened) summary.exit_code = 3;
            }
        }
    }
    report_summary(log, summary);
    log.close();
    return summary.exit_code;
}
