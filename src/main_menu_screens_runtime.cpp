#include "bsp/main_menu_screens_runtime.hpp"

#include "bsp/frontend_managers.hpp"

namespace bsp {
namespace {

// The overlay index order is the order 004C40F0 reads the three offsets in, not
// the numeric order of the offsets themselves.
InterfaceOnlyScreenRef overlay_ref(std::size_t index) noexcept {
    switch (index) {
    case 0:
        return InterfaceOnlyScreenRef::MissionOverlayC8;
    case 1:
        return InterfaceOnlyScreenRef::MissionOverlay9C;
    default:
        return InterfaceOnlyScreenRef::MissionOverlayA4;
    }
}

// 004C419B..004C41B8 and 004C41E5..004C41FB: 004B6E50 inlined. The guard on the
// active byte is re-tested inside the inlined copy exactly as the callee does,
// and it is always true at both sites because the caller just tested it.
void close_screen_inline(FrontEndScreen& screen, InterfaceOnlyScreenRef ref, InterfaceOnlyHost& host) {
    if (screen.active) {
        host.screen_exit_virtual(ref);
    }
    screen.wanted = false;
    screen.active = false;
    host.screen_commit_004f83b0(ref);
}

// The shared shape of the modal arm: an active screen is either updated or
// closed, on its wanted byte. 004C416F for the menu command screen and 004C41BD
// for the gamepad prompt are the same six instructions over different objects.
void update_or_close(FrontEndScreen& screen, InterfaceOnlyScreenRef ref, float seconds,
    InterfaceOnlyHost& host) {
    if (!screen.active) {
        return;
    }
    if (screen.wanted) {
        host.screen_update_004f71f0(ref, seconds);
    } else {
        close_screen_inline(screen, ref, host);
    }
}
}

bool interface_only_takes_system_ui_arm(
    bool system_ui_raised, bool system_ui_previous, bool suppress_arm) noexcept {
    // 004C40F5 tests 00E188AE, 004C4100 tests the latch, 004C4108 tests the
    // suppression byte. The suppression byte wins over both.
    if (suppress_arm) {
        return false;
    }
    return system_ui_raised || system_ui_previous;
}

float interface_only_pump_delta(const InterfaceOnlyGameFields& game) noexcept {
    // 004C413D: game+635h selects 004C4145 (+21ECh) over 004C414F (+21F0h).
    return game.still_simulate ? game.unclamped_delta : game.scaled_delta;
}

InterfaceOnlyResult run_interface_only_update_004c40f0(
    InterfaceOnlyWorld& world, InterfaceOnlyGameFields& game, InterfaceOnlyHost& host) {
    InterfaceOnlyResult result{};

    // 004C40F5..004C412A. The arm is decided from the pre-call values, then both
    // side effects run unconditionally, before either arm.
    const bool system_ui_arm = interface_only_takes_system_ui_arm(
        world.system_ui_raised, world.system_ui_previous, game.suppress_system_ui_arm);
    game.suppress_system_ui_arm = false;               // 004C4118
    world.system_ui_previous = world.system_ui_raised; // 004C411E..004C4124

    result.arm = system_ui_arm ? InterfaceOnlyArm::SystemUiOverlay : InterfaceOnlyArm::FrontEndPump;

    if (system_ui_arm) {
        // 004C4205. The whole arm is skipped when the in-mission manager is
        // absent, which is the case for every front-end frame.
        if (world.in_mission_manager_present) {
            for (std::size_t i = 0; i < kInterfaceOnlyOverlayCount; ++i) {
                if (!world.overlay_present[i]) {
                    continue;
                }
                FrontEndScreen& screen = world.overlay[i];
                if (!screen.active) {
                    continue;
                }
                const InterfaceOnlyScreenRef ref = overlay_ref(i);
                if (screen.wanted) {
                    // 004C422A / 004C425D / 004C4290. Always the scaled delta:
                    // the +635h choice exists only at the pump call site.
                    host.screen_update_004f71f0(ref, game.scaled_delta);
                } else {
                    // 004C423B / 004C426E / 004C42A1, the out-of-line 004B6E50,
                    // which runs the exit virtual, clears both bytes and commits.
                    host.screen_close_004b6e50(ref);
                    screen.wanted = false;
                    screen.active = false;
                }
            }
        }
    } else if (!world.menu.modal_dialog_active) {
        // 004C4130..004C4165. The main-menu frame. This one call is the entire
        // per-frame life of the front-end screens at game state 5.
        result.pump_delta = interface_only_pump_delta(game);
        host.run_screen_pump_004f8830(result.pump_delta);
        result.screen_pump_ran = true;
    } else {
        // 004C416F. A modal dialog is up, so the registry pump is skipped and
        // only these two objects are serviced.
        update_or_close(world.menu.base, InterfaceOnlyScreenRef::MenuCommand, game.scaled_delta, host);
        // 004C41BD. Reached whether or not the menu command screen was active,
        // and the global is dereferenced without a null test.
        update_or_close(
            world.gamepad_prompt, InterfaceOnlyScreenRef::GamepadPrompt, game.scaled_delta, host);
    }

    // 004C42A6, the tail, joined by both arms.
    if (game.termination_reason != 0 && world.in_mission_manager_present
        && world.overlay_present[kInterfaceOnlyOverlayCount - 1]) {
        FrontEndScreen& screen = world.overlay[kInterfaceOnlyOverlayCount - 1];
        screen.wanted = true; // 004C42C7
        screen.active = true; // 004C42CB
        // 004C42CF then 004C42DB: the commit runs before the enter virtual, the
        // same order the registry pump uses and the reverse of the exit path.
        host.screen_commit_004f83b0(InterfaceOnlyScreenRef::MissionOverlayA4);
        host.screen_enter_virtual(InterfaceOnlyScreenRef::MissionOverlayA4);
        host.mission_overlay_finalize_005b6960(); // 004C42EF, a tail jump
        result.overlay_forced_visible = true;
    }
    return result;
}

InterfaceLockOutcome classify_interface_request_00683e90(
    bool lock_engaged, int interface_id) noexcept {
    if (!lock_engaged) {
        return InterfaceLockOutcome::PassedLockClear; // 00683E97
    }
    if (interface_id == kMovieCameraNewInterface) {
        return InterfaceLockOutcome::PassedExempt; // 00683E9D
    }
    if (interface_id >= kFirstInGameInterface) {
        return InterfaceLockOutcome::Rejected; // 00683EA5 then 00683EB3
    }
    return InterfaceLockOutcome::PassedAndReleased; // 00683EA7
}

bool interface_lock_released_by(InterfaceLockOutcome outcome) noexcept {
    return outcome == InterfaceLockOutcome::PassedAndReleased;
}
}
