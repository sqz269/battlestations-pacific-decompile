// bsp_game.exe, milestone 1: the reconstructed startup spine running as a Win32 process.
//
// Entry sequence 008f81f0 (docs/WINMAIN_STARTUP.md) drives the whole run. Everything this
// file adds on top of run_win_main is process plumbing: the option parsing for --frames
// and --log, the console attachment that lets a windowed process report to the terminal
// that started it, and the exit code. Evidence: docs/GAME_EXECUTABLE.md.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdio>
#include <string>

#include "bsp/game_hosts.hpp"
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
        std::fprintf(stderr, "usage: bsp_game.exe [--frames N] [--log <path>]\n");
        return 2;
    }

    bsp::game::GameHostLog log;
    if (!log.open(options.log_path)) {
        std::fprintf(stderr, "bsp_game: cannot write log %s\n", options.log_path.c_str());
        return 2;
    }
    log.notef("bsp_game milestone 1, frames=%ld log=%s", options.frame_limit,
        options.log_path.empty() ? "(stdout only)" : options.log_path.c_str());

    bsp::game::GameStartupHost host(log, instance, options);

    // 008f81f0 reads none of its four arguments; they are recorded for completeness.
    bsp::WinMainArguments arguments;
    arguments.instance = instance;
    arguments.previous_instance = previous_instance;
    arguments.command_line = command_line;
    arguments.show_command = show_command;

    const int result = bsp::run_win_main(arguments, host);

    bsp::game::GameRunSummary summary = host.summary();
    summary.exit_code = result;
    // A run that asked for a frame count only succeeds when the device presented them.
    if (options.frame_limit > 0
        && summary.frames_presented < static_cast<unsigned long long>(options.frame_limit)) {
        summary.exit_code = 1;
    }
    report_summary(log, summary);
    log.close();
    return summary.exit_code;
}
