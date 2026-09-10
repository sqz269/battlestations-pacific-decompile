#include "bsp/game_render_frame.hpp"

#include <cstdio>

namespace bsp {
// 004b7f70 wraps a bounded sprintf into the 512-byte stack buffer both callers
// declare. The native buffer length is not checked against the base directory;
// the reconstruction truncates rather than reproduce that overflow.

GameRenderViewPath select_view_path_004ca4e7(const GameRenderViewGate& gate) noexcept
{
    // 004ca4e7..004ca500. The three loads are one conjunction: only a
    // single-local-player split view inside the mission state skips the block.
    const bool split_screen_in_mission = gate.local_player_mode == 1 && gate.split_view
        && gate.game_state == static_cast<std::uint32_t>(GameStateId::kInMission);
    // 004ca506 and 004ca514. Either of these skips the block on its own.
    if (split_screen_in_mission || !gate.interface_manager_present || gate.mission_end_suspended) {
        return GameRenderViewPath::kInlineWorldView;
    }
    // 004ca521. The compare chain tests 1, then 2, then 3, and falls through to
    // the inline path for every other level, zero included.
    switch (gate.interface_level) {
    case 1:
        return GameRenderViewPath::kJobPoolScene;
    case 2:
        return GameRenderViewPath::kInterfaceOrthoView54;
    case 3:
        return GameRenderViewPath::kInterfaceOrthoView5C;
    default:
        return GameRenderViewPath::kInlineWorldView;
    }
}

bool runs_inline_world_view(GameRenderViewPath path) noexcept
{
    // 004ca5b0 is the only jump over 004ca5ce.
    return path != GameRenderViewPath::kJobPoolScene;
}

std::vector<GameRenderJob> frame_job_drain_order(const std::vector<GameRenderJob>& enqueued)
{
    // 00be2fa0: InterlockedDecrement on the count, then index the slot array
    // with the value it returned. The first job drained is therefore the last
    // one 00be3020 wrote.
    return std::vector<GameRenderJob>(enqueued.rbegin(), enqueued.rend());
}

bool toggle_debug_flag(bool& flag) noexcept
{
    flag = !flag;
    return flag;
}

std::string screenshot_directory_004bfbf0(const std::string& base, int sequence)
{
    char buffer[kScreenshotPathCapacity] = {};
    std::snprintf(buffer, sizeof(buffer), "%s%04d", base.c_str(), sequence);
    return std::string(buffer);
}

std::string screenshot_frame_path_004c9a00(const std::string& base, int sequence, int frame)
{
    char buffer[kScreenshotPathCapacity] = {};
    std::snprintf(buffer, sizeof(buffer), "%s%04d\\scr%04d.png", base.c_str(), sequence, frame);
    return std::string(buffer);
}

void begin_screenshot_sequence_004bfbf0(ScreenshotState& state, GameRenderFrameHost& host)
{
    // 004bfbf5. The increment is the first statement of the loop body, so the
    // sequence the caller arrives with is never probed.
    do {
        state.sequence += 1;
    } while (!host.create_screenshot_directory(
        screenshot_directory_004bfbf0(state.base_directory, state.sequence)));
    // 004bfc3c stores the loop result, which is zero on the exiting iteration.
    state.frame = 0;
}

std::string next_screenshot_path_004c9a00(ScreenshotState& state)
{
    std::string path
        = screenshot_frame_path_004c9a00(state.base_directory, state.sequence, state.frame);
    state.frame += 1; // 004c9a4a, after the format
    return path;
}

bool try_begin_render_frame_004c6c30(GameRenderFrameState& state, GameRenderFrameHost& host)
{
    // 004c6c36. Any non-idle phase reports success and touches nothing, which
    // is why the in-mission frame can call this early and BSP_Game_Render can
    // still run its own gate later.
    if (state.phase != GameRenderPhase::kIdle) {
        return true;
    }
    if (host.render_queue_retained()) {
        return false;
    }
    host.renderer_begin_frame();
    state.phase = GameRenderPhase::kBegun;
    return true;
}

void run_game_render(GameRenderFrameState& state, GameRenderFrameHost& host)
{
    // 004ca45b. The only entry gate; the phase advances before any callback so
    // a re-entrant call cannot render twice.
    if (state.phase != GameRenderPhase::kBegun) {
        return;
    }
    state.phase = GameRenderPhase::kRendering;

    // 004ca465..004ca48e. The delta is loaded before the phase store and passed
    // to the first virtual as a stack float.
    host.hud_render(state.scaled_delta);
    host.hud_post_render();

    // 004ca490. MSVC static-local guard: the zone name is built once.
    if (!state.profiler_zone_registered) {
        state.profiler_zone_registered = true;
        host.register_profiler_zone(kGameRenderProfilerZone);
    }

    const GameRenderViewGate gate{state.local_player_mode, state.split_view, state.game_state,
        state.interface_manager_present, state.mission_end_suspended, state.interface_level};
    const GameRenderViewPath path = select_view_path_004ca4e7(gate);

    if (path == GameRenderViewPath::kJobPoolScene) {
        // 004ca52f. The suspend byte is tested again here; the outer gate
        // already proved it clear, so the term is dead. It is kept because the
        // native code evaluates it.
        if (host.shadow_map_owner_present() && host.render_camera_present()
            && !state.mission_end_suspended) {
            host.update_shadow_map_view_00a8f3b0();
        }
        // 004ca569..004ca5ad. Two enqueues, then one dispatch. The drain is
        // last in first out, so the camera update runs before the world view.
        host.enqueue_frame_job(GameRenderJob::kWorldView004bbd00);
        host.enqueue_frame_job(GameRenderJob::kCameraUpdate004b4820);
        host.dispatch_frame_jobs();
    } else {
        if (path == GameRenderViewPath::kInterfaceOrthoView54) {
            host.render_interface_ortho_view_0059d7b0();
        } else if (path == GameRenderViewPath::kInterfaceOrthoView5C) {
            host.render_interface_ortho_view_00535430();
        }
        host.submit_world_view_00735b50();
    }

    // 004ca5d9. The screenshot toggle, suppressed on the front-end states.
    if (!is_front_end_game_state(static_cast<int>(state.game_state))
        && host.input_action_pressed(kScreenshotToggleAction)) {
        if (toggle_debug_flag(state.screenshot.capturing)) {
            // 004ca636 runs only on the rising half of the toggle, so turning
            // capture off never opens a directory.
            begin_screenshot_sequence_004bfbf0(state.screenshot, host);
        }
    }

    if (host.front_end_preview_active()) {
        host.render_front_end_preview_00503510();
    }
    host.draw_gui_00aa45a0();
    if (host.debug_overlay_present()) {
        host.draw_debug_overlay_0078a430();
    }
    // 004ca673. Everything queued above is submitted here in one pass.
    host.execute_render_command_queue();
    host.flush_diagnostic_sink_004c14c0();
    if (state.profiler_overlay) {
        host.draw_profiler_overlay_00be3bf0();
    }

    // 004ca699. The second toggle. Its target is never read outside this body.
    if (!is_front_end_game_state(static_cast<int>(state.game_state))
        && host.input_action_pressed(kUnusedDebugToggleAction)) {
        toggle_debug_flag(state.unused_debug_toggle);
    }

    // 004ca6ee. The camera and the suspend byte are not retested here.
    if (host.shadow_map_owner_present()) {
        host.shadow_map_frame_end_00a8ac00();
    }
    if (host.scene_present()) {
        host.scene_post_render();
    }
    host.flush_render_resources_00b0d190();
    host.queue_camera_finalize_00b1ef60();
}

void finish_render_frame_004ca1f0(GameRenderFrameState& state, GameRenderFrameHost& host)
{
    // 004ca1fd..004ca224, outside the phase gate.
    host.profiler_set_finish_slot_color(kFinishRenderFrameSlotColor);
    host.profiler_begin_finish_counter();

    if (state.phase == GameRenderPhase::kRendering && !host.render_queue_retained()) {
        if (!state.screenshot.capturing) {
            host.renderer_end_frame_default();
        } else {
            host.renderer_end_frame_capture(next_screenshot_path_004c9a00(state.screenshot));
        }
        // 004ca2c9. Only a frame that actually presented returns to idle; a
        // retained queue leaves the phase at kRendering for the next attempt.
        state.phase = GameRenderPhase::kIdle;
    }

    host.profiler_end_finish_counter();
}
}
