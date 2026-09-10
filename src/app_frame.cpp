#include "bsp/app_frame.hpp"

namespace bsp {
std::uint32_t frame_marker_color_00737a6c(FrameMarkerColor& color) noexcept {
    if (!color.initialized) {
        color.initialized = true;
        color.argb = 0xFF000000u;
    }
    return color.argb;
}

bool is_mission_game_state(int game_state) noexcept {
    return game_state == 1 || game_state == 2 || game_state == 4;
}

float run_application_frame(ApplicationFrameState& state, FrameMarkerColor& color,
    ApplicationFrameHost& host) {
    host.profiler_set_frame_slot_color(frame_marker_color_00737a6c(color));
    host.profiler_begin_frame_slot();

    state.input_action_latch = false;
    if (!is_mission_game_state(host.game_state())
        && host.input_action_pressed(kApplicationFrameInputAction)) {
        state.input_action_latch = true;
    }

    host.advance_frame_clock();
    const float seconds = timestamp_seconds_x87(host.frame_interval());
    host.game_on_move(seconds);
    state.first_update = false;

    // 00737b33 re-reads the game state, but the mission branch at 00737b65 and
    // the other branch at 00737b4e run the same test and store, so the second
    // read is dead. The store is sticky: a cleared request never clears +181h.
    static_cast<void>(host.game_state());
    if (host.exit_requested()) {
        host.request_loop_exit();
    }

    host.tick_vfs_providers();
    host.update_loading_queue();

    host.profiler_end_frame_slot();
    host.profiler_end_frame();
    return seconds;
}
}
