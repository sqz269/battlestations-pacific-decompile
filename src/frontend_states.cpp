#include "bsp/frontend_states.hpp"

namespace bsp {
namespace {
// 004e4bb5..004e4bdb and the identical fallback at 004e4cdd..004e4cff. Returns
// false only on the first form, where a retained queue skips the whole branch.
bool open_render_queue(GameFrontEndFrameState& state, FrontEndFrameHost& host) {
    if (state.render_queue_open) {
        return true;
    }
    if (host.render_queue_retained()) {
        return false;
    }
    host.renderer_begin_frame();
    state.render_queue_open = true;
    return true;
}

// 004e4bf8..004e4c9b, the state 2 body.
void run_title_screen_state(GameFrontEndFrameState& state, FrontEndWorld& world,
    FrontEndFrameHost& host, FrontEndScreenHost& screens, float raw_delta) {
    if (world.title == nullptr) {
        // 004e4c0d. 004db220 runs 004db190 and 004cccc0, clears game+2180h and
        // tail jumps to 004c9a70 GGame::OnInitTitle, which builds 00E198C8.
        host.reinitialize_title_screen();
    }
    if (world.title != nullptr) {
        host.update_title_screen(raw_delta);
    }

    if (world.front_end_gui_suspended) {
        // 004e4c38..004e4c52. Both pushes belong to the GUI calls, not to
        // 004c12b0: the same accessor is called with no push at 004e4ca8.
        host.gui_clear_screens(false);
        host.gui_set_enabled(false);
        return;
    }

    host.gui_set_enabled(true);
    // 004e4c61..004e4c96 re-fetches the 00425d10 singleton for each test.
    if (!world.menu.base.active) {
        return;
    }
    if (world.menu.base.wanted) {
        // 004f71f0, and the only site in the branch that uses game+21F0h
        // instead of the raw delta.
        screens.menu_command_update(state.scaled_delta);
    } else {
        close_menu_command_screen(world.menu, screens);
    }
}
}

void close_front_end_screen(FrontEndScreen& screen, int slot, FrontEndScreenHost& host) {
    if (screen.active) {
        host.screen_exit(slot);
    }
    screen.wanted = false;
    screen.active = false;
    host.screen_commit(slot);
}

void close_menu_command_screen(MenuCommandScreen& menu, FrontEndScreenHost& host) {
    if (menu.base.active) {
        host.menu_command_exit();
    }
    menu.base.wanted = false;
    menu.base.active = false;
    host.menu_command_commit();
}

void run_front_end_screen_pump(FrontEndWorld& world, FrontEndScreenHost& host, float raw_delta) {
    // 004f8830..004f887f. A modal dialog narrows the pump to that one screen
    // unless a mission is loaded and at least one local player is present.
    if (world.menu.modal_dialog_active) {
        if (world.menu.base.active) {
            host.menu_command_update(raw_delta);
        }
        if (world.local_player_count == 0) {
            return;
        }
        if (!world.mission_context_present) {
            return;
        }
    }

    world.screens.pump_sentinel = false; // 004f8881

    // 004f8890: exit every screen whose request was withdrawn. The active byte
    // is re-read after the virtual because the virtual may clear it itself.
    for (int slot = 0; slot < kFrontEndScreenSlotCount; ++slot) {
        FrontEndScreen* screen = world.screens.slots[slot];
        if (screen == nullptr || screen->wanted || !screen->active) {
            continue;
        }
        host.screen_exit(slot);
        if (screen->active) {
            screen->active = false;
            host.screen_commit(slot);
        }
    }

    // 004f88d0: enter every newly requested screen. The commit runs before the
    // enter virtual here, the opposite order from the exit pass.
    for (int slot = 0; slot < kFrontEndScreenSlotCount; ++slot) {
        FrontEndScreen* screen = world.screens.slots[slot];
        if (screen == nullptr || !screen->wanted || screen->active) {
            continue;
        }
        screen->active = true;
        host.screen_commit(slot);
        host.screen_enter(slot);
    }

    // 004f8906: update the screens that are both requested and applied.
    for (int slot = 0; slot < kFrontEndScreenSlotCount; ++slot) {
        FrontEndScreen* screen = world.screens.slots[slot];
        if (screen == nullptr || !screen->wanted || !screen->active) {
            continue;
        }
        host.screen_update(slot, raw_delta);
    }
}

FrontEndFrameOutcome run_front_end_state_frame(GameFrontEndFrameState& state, FrontEndWorld& world,
    FrontEndFrameHost& host, FrontEndScreenHost& screens, float raw_delta) {
    if (is_front_end_game_state(host.game_state())) {
        if (open_render_queue(state, host)) {
            // 004e4bde re-reads game+5D4h after the renderer BeginFrame virtual.
            switch (static_cast<GameFrontEndState>(host.game_state())) {
            case GameFrontEndState::LogoSequence:
                host.update_logo_sequence();
                break;
            case GameFrontEndState::TitleScreen:
                run_title_screen_state(state, world, host, screens, raw_delta);
                break;
            case GameFrontEndState::FrontEndShell:
            default:
                // State 4 has no per-state update: 004e4bfb falls straight to
                // the shared tail at 004e4c9b.
                break;
            }

            // 004e4c9b..004e4cc9, the tail all three states share. The blocking
            // screen path at 004e4b57 runs the same four calls but reaches them
            // through 004c6c30 and never runs the screen pump.
            run_front_end_screen_pump(world, screens, raw_delta);
            host.gui_update(raw_delta, 0);
            host.game_render();
            host.game_finish_render_frame();
        }

        // 004e4ccb reads the count through the 00E188A8 mirror, not through ESI.
        if (!host.state_requests_pending()) {
            return FrontEndFrameOutcome::FrameComplete;
        }
    }

    // 004e4cdd. Reached by every state that is not 1, 2 or 4, and by a front-end
    // frame that still has queued requests. The retained result is discarded.
    static_cast<void>(open_render_queue(state, host));
    if (!state.requests_held) {
        host.drain_state_requests();
    }
    if (is_front_end_game_state(host.game_state())) {
        return FrontEndFrameOutcome::FrameComplete;
    }
    return FrontEndFrameOutcome::ContinueToSimulation;
}
}
