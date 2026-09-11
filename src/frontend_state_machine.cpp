#include "bsp/frontend_state_machine.hpp"

namespace bsp {

// 004f7570 and 004f7580 are each XOR AL,AL / RET, so both base predicates are
// false and the base class neither owns its own visibility nor sets the second,
// still unnamed, flag. 004f7620 reads the first one twice per live slot to
// decide whether it may write +4h at all.
bool front_end_screen_self_managed_004f7570() noexcept {
    return false;
}

bool front_end_screen_predicate_004f7580() noexcept {
    return false;
}

FrontEndScreen& construct_front_end_screen_004f7180(FrontEndScreen& screen) noexcept {
    // 004f7184 stores the vtable, 004f718a and 004f718d clear both bytes with
    // the same zeroed CL. The vtable store has no counterpart here.
    screen.wanted = false;
    screen.active = false;
    return screen;
}

void register_front_end_screen_004f71d0(FrontEndScreenTable& table, FrontEndScreen& screen,
    int screen_id) noexcept {
    // 004f71d9, MOV [EAX*4 + 0xe18b60], ESI. The native store is unchecked; an
    // id outside the array writes past it. The bound is applied here because a
    // reconstruction that reproduced the overrun would corrupt its own caller,
    // and the discarded case is recorded in docs/FRONTEND_STATE_MACHINE.md.
    if (screen_id < 0 || screen_id >= kFrontEndScreenSlotCount) {
        return;
    }
    table.slots[screen_id] = &screen;
}

void unregister_front_end_screen_004f71a0(FrontEndScreenTable& table,
    const FrontEndScreen& screen) noexcept {
    // 004f71b0..004f71c2. The scan covers every slot, not just the screen's own
    // id, and it does not stop at the first match.
    for (int slot = 0; slot < kFrontEndScreenSlotCount; ++slot) {
        if (table.slots[slot] == &screen) {
            table.slots[slot] = nullptr;
        }
    }
}

FrontEndScreenPhase front_end_screen_phase(const FrontEndScreen& screen) noexcept {
    if (screen.wanted) {
        return screen.active ? FrontEndScreenPhase::Shown : FrontEndScreenPhase::Entering;
    }
    return screen.active ? FrontEndScreenPhase::Exiting : FrontEndScreenPhase::Hidden;
}

FrontEndScreenPass front_end_screen_pass(FrontEndScreenPhase phase) noexcept {
    switch (phase) {
    case FrontEndScreenPhase::Exiting:
        return FrontEndScreenPass::Exit;
    case FrontEndScreenPhase::Entering:
        return FrontEndScreenPass::Enter;
    case FrontEndScreenPhase::Shown:
        return FrontEndScreenPass::Update;
    case FrontEndScreenPhase::Hidden:
        break;
    }
    return FrontEndScreenPass::None;
}

void activate_title_screen_0068d8d0(TitleHandoverState& state, FrontEndScreenTable& table,
    TitleHandoverHost& host) {
    if (state.skip_title) {
        // 0068d8f2..0068d918. The storage reset runs before the guard, so it
        // happens even when a request is already queued and nothing else does.
        host.reset_storage_availability();
        if (state.state_request_pending) {
            return;
        }
        host.request_game_state(4);
        state.state_request_pending = true;
        state.requests_held = false;
        return;
    }

    // 0068d92f..0068d954. The allocation result is stored into title+40h before
    // it is tested, and the native code then dereferences it unconditionally at
    // 0068d957. A null allocation is a crash there; this returns instead.
    FrontEndScreen* screen = host.create_press_start_screen();
    state.press_start = screen;
    if (screen == nullptr) {
        return;
    }

    // 0068d966. The leaf override 0067ca80 calls 004f71d0 with the press-start
    // id and then loads the FE_initial layout, so the registry slot is filled
    // inside this call rather than here.
    host.screen_register(*screen);
    static_cast<void>(table);

    // 0068d96b..0068d981. Both bytes are set from the same AL, the visibility
    // commit runs, and only then does the enter virtual. This is the enter edge
    // of the pump open coded, so the screen is already Shown by the time the
    // next pump sweep sees it and pass B never runs for it.
    screen->wanted = true;
    screen->active = true;
    host.screen_commit(*screen);
    host.screen_enter(*screen);
}

void update_title_screen_0068d850(const TitleHandoverState& state, TitleHandoverHost& host,
    float raw_delta) {
    // 0068d857..0068d866. The 00F8BBF4 singleton is updated first and it is not
    // reached through the screen registry.
    host.update_title_owner(raw_delta);
    // 0068d86d loads title+40h into ECX with no null test, so the native body
    // relies on activate having run. The guard here is the difference.
    if (state.press_start == nullptr) {
        return;
    }
    host.update_press_start_screen(raw_delta);
}

FrontEndBootStep next_front_end_boot_step(FrontEndBootStep step, bool press_start_accepted,
    bool state_request_pending) noexcept {
    switch (step) {
    case FrontEndBootStep::TitleInit:
        return FrontEndBootStep::TitleActivate;
    case FrontEndBootStep::TitleActivate:
        return FrontEndBootStep::FrameTitleUpdate;
    case FrontEndBootStep::FrameTitleUpdate:
        return FrontEndBootStep::FramePump;
    case FrontEndBootStep::FramePump:
        return FrontEndBootStep::PressStartUpdate;
    case FrontEndBootStep::PressStartUpdate:
        // 004e4c24 and 004e4c9b run again every frame until the page accepts an
        // input, so the machine loops back through the two frame steps.
        return press_start_accepted ? FrontEndBootStep::TitleSkip
                                    : FrontEndBootStep::FrameTitleUpdate;
    case FrontEndBootStep::TitleSkip:
        // 0068d8bc. A request already in the ring suppresses the enqueue and
        // the page stays up for another frame.
        return state_request_pending ? FrontEndBootStep::FrameTitleUpdate
                                     : FrontEndBootStep::FrontEndShellEntry;
    case FrontEndBootStep::FrontEndShellEntry:
        break;
    }
    return FrontEndBootStep::FrontEndShellEntry;
}

} // namespace bsp
