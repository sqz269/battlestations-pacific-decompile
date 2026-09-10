#include "bsp/blocking_screen.hpp"

#include <algorithm>

namespace bsp {

bool attract_suppressed_in_game_state(int game_state) noexcept {
    return std::find(kAttractSuppressedGameStates.begin(), kAttractSuppressedGameStates.end(),
               game_state)
        != kAttractSuppressedGameStates.end();
}

void update_attract_screen_00689cc0(AttractScreenState& state, AttractScreenHost& host,
    float raw_delta) {
    // 00689CC4..00689CDC. Both queries run before anything is tested and their
    // results are combined with OR on a byte, not with a short-circuit.
    const bool dynamic_down = host.any_dynamic_device_button_down();
    const bool joystick_down = host.any_joystick_slot_button_down();
    bool hold = dynamic_down || joystick_down;

    // 00689CDE..00689CF2. Only while the screen is down. The disabled flag and
    // the scene-context byte hold the countdown at its reload value, which is
    // how the screen is kept out of a session without clearing the timer.
    if (!state.active) {
        const bool disabled = !state.enabled;
        const bool scene_blocks = host.scene_context_blocks_attract();
        hold = hold || disabled || scene_blocks;
    }

    if (hold) {
        // 00689CF9..00689D01.
        state.idle_countdown = kAttractIdleSeconds;
    } else {
        // 00689D08..00689D30. COMISS then a conditional store, so a NaN delta
        // is kept rather than clamped; the subtraction is one x87 FLD/FSUB/FSTP
        // on two float32 operands with a single rounding, which is what plain
        // float arithmetic does.
        const float clamped
            = raw_delta > kAttractDeltaClampSeconds ? kAttractDeltaClampSeconds : raw_delta;
        state.idle_countdown -= clamped;
    }

    if (state.active) {
        // 00689D33..00689D59. Any button dismisses the screen. The countdown was
        // already reloaded above, and 00689C00 reloads it a second time.
        if (!hold) {
            return;
        }
        host.deactivate_attract_screen();
        state.active = false;
        state.idle_countdown = kAttractIdleSeconds;
        if (host.front_end_menu_present()) {
            host.front_end_menu_on_attract_dismissed();
        }
        return;
    }

    // 00689D5C..00689D87. Strictly negative, so the frame the countdown reaches
    // exactly zero is not the raising frame.
    if (!(state.idle_countdown < 0.0f)) {
        return;
    }
    if (attract_suppressed_in_game_state(host.game_state())) {
        return;
    }
    host.activate_attract_screen();
    state.active = true;
}

void gui_manager_update_00aa4f80(GuiManagerUpdateState& state, GuiManagerUpdateHost& host,
    float seconds, bool flag) {
    // 00AA4F8E..00AA4FA8.
    state.in_update = flag;
    if (!flag) {
        host.run_pointer_input_pass();
    }

    // 00AA4FAD..00AA4FBC.
    host.reset_highlight_frames();
    const std::vector<GuiScreenHandle> screens = host.snapshot_screens();

    // 00AA4FE0..00AA503B. The checked-iterator bounds calls between the steps
    // are assertions with no effect on a well-formed vector and are not
    // modelled.
    for (const GuiScreenHandle screen : screens) {
        if (host.screen_wants_update(screen) || host.screen_has_live_entries(screen)) {
            host.screen_update(screen, seconds);
        }
    }

    // 00AA503D..00AA504F. The flag is cleared unconditionally, even when the
    // caller passed true, and the snapshot is freed.
    state.in_update = false;
}

void gui_manager_set_enabled_00aa0e00(GuiManagerPointerState& state, GuiPointerHost& host,
    bool enabled) {
    state.enabled = enabled;
    if (!enabled) {
        host.set_front_end_pointer_visible(false);
    }
}

void gui_manager_set_pointer_visible_00aa0e50(GuiPointerHost& host, bool visible) {
    host.set_front_end_pointer_visible(visible);
}

void reset_to_title_004db220(TitleResetHost& host) {
    host.release_session_objects();
    host.release_mission_objects();
    host.clear_game_flag_2180h();
    host.on_init_title();
}

bool run_blocking_screen_frame(BlockingScreenFrameHost& host, float raw_delta) {
    // 004E4B29..004E4B31.
    if (!host.attract_screen_present()) {
        return false;
    }

    // 004E4B33..004E4B3B. The update runs on every frame the screen object
    // exists, whether or not the screen is up: it owns the idle countdown.
    host.update_attract_screen(raw_delta);

    // 004E4B40..004E4B4A. The global is loaded a second time, so a screen the
    // update had just raised is seen on the same frame.
    if (!host.attract_screen_active()) {
        return false;
    }

    // 004E4B4C..004E4B55. A refused device leaves the frame to the normal path.
    if (!host.try_begin_render_frame()) {
        return false;
    }

    // 004E4B57..004E4B75. The manager is fetched first, then becomes ECX.
    void* const manager = host.gui_manager();
    host.gui_manager_update(manager, raw_delta, false);
    host.render();
    host.finish_render_frame();
    return true;
}
}
