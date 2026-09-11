#include "bsp/main_menu_path.hpp"

#include "bsp/frontend_entry.hpp"
#include "bsp/press_start_screen.hpp"

namespace bsp {
namespace {

// 00685938 and 004DA9CA both express "clear this level" as a call with only the
// varargs terminator. An empty list is that call.
constexpr std::size_t kEmptyList = 0;

} // namespace

// ---------------------------------------------------------------------------
// Level ownership
// ---------------------------------------------------------------------------

const FrontEndScreenSetLevelOwner* front_end_screen_set_level_owner(int level) noexcept {
    for (const FrontEndScreenSetLevelOwner& owner : kFrontEndScreenSetLevelOwners) {
        if (owner.level == level) {
            return &owner;
        }
    }
    return nullptr;
}

bool front_end_screen_set_level_populated(const std::vector<int>& level) noexcept {
    // 0068AB81: MOV EAX,[00E18D1C]; TEST EAX,EAX; JZ. The first pointer null
    // means the vector never allocated.
    if (level.data() == nullptr) {
        return false;
    }
    // 0068AB8D: MOV ECX,[00E18D20]; SUB ECX,EAX; SAR ECX,2; JZ. An emptied
    // vector keeps its buffer, so the size has to be tested separately.
    return !level.empty();
}

void clear_all_screen_set_levels_004da780(FrontEndScreenSetStack& screens,
    FrontEndScreenTable& table, FrontEndScreenSetHostBindings& bindings,
    GameInputContextSetStack& contexts) {
    // 004DA99D, 004DA9A4, 004DA9AB, 004DA9B2: the four input-context levels
    // first, each with the game object and a bare terminator. Every setter
    // stores its empty list and then runs 004C4300 for its own level.
    for (int level = 1; level <= kGameInputContextSetLevels; ++level) {
        contexts.levels[static_cast<std::size_t>(level - 1)].clear();
        apply_input_context_levels_004c4300(contexts, level);
    }

    // 004DA9B8, 004DA9BE, 004DA9C4, 004DA9CA: screen-set levels 1 through 4.
    // Level 5 is not cleared because nothing ever sets it.
    for (int level = kFrontEndScreenSetLowestLevel; level <= kGameInputContextSetLevels; ++level) {
        set_front_end_screen_set_level(screens, table, bindings, level, nullptr, kEmptyList);
    }
}

// ---------------------------------------------------------------------------
// Pump site
// ---------------------------------------------------------------------------

FrontEndScreenPumpSite front_end_screen_pump_site(std::int32_t game_state) noexcept {
    for (std::int32_t branch_state : kFrontEndBranchGameStates) {
        if (game_state == branch_state) {
            return FrontEndScreenPumpSite::FrontEndStateTail;
        }
    }
    // Every other state, the shell-ready 5 included, falls through to the full
    // update path and is pumped by 004C40F0 at 004E53B6.
    return FrontEndScreenPumpSite::GameInterfaceOnly;
}

// ---------------------------------------------------------------------------
// Step naming
// ---------------------------------------------------------------------------

std::string_view main_menu_path_step_name(MainMenuPathStep step) noexcept {
    switch (step) {
    case MainMenuPathStep::PressStartPoll: return "PressStartPoll";
    case MainMenuPathStep::TitleSkip: return "TitleSkip";
    case MainMenuPathStep::RequestShellState: return "RequestShellState";
    case MainMenuPathStep::DrainStateRequest: return "DrainStateRequest";
    case MainMenuPathStep::EnterShell: return "EnterShell";
    case MainMenuPathStep::PushInterfaceRequest: return "PushInterfaceRequest";
    case MainMenuPathStep::ActivateManager: return "ActivateManager";
    case MainMenuPathStep::PublishShellReady: return "PublishShellReady";
    case MainMenuPathStep::ServiceInterfaceRequest: return "ServiceInterfaceRequest";
    case MainMenuPathStep::ApplyInterfaceRequest: return "ApplyInterfaceRequest";
    case MainMenuPathStep::PublishScreenSet: return "PublishScreenSet";
    case MainMenuPathStep::EnterScreen: return "EnterScreen";
    case MainMenuPathStep::ScreenVisible: return "ScreenVisible";
    }
    return "";
}

std::uint32_t main_menu_path_step_address(MainMenuPathStep step) noexcept {
    switch (step) {
    case MainMenuPathStep::PressStartPoll: return 0x0067D2DDu;
    case MainMenuPathStep::TitleSkip: return 0x0068D8A0u;
    case MainMenuPathStep::RequestShellState: return 0x0068D8BCu;
    case MainMenuPathStep::DrainStateRequest: return 0x004E4430u;
    case MainMenuPathStep::EnterShell: return 0x004E4000u;
    case MainMenuPathStep::PushInterfaceRequest: return 0x004E4259u;
    case MainMenuPathStep::ActivateManager: return 0x004E4269u;
    case MainMenuPathStep::PublishShellReady: return 0x004E4279u;
    case MainMenuPathStep::ServiceInterfaceRequest: return 0x004E5442u;
    case MainMenuPathStep::ApplyInterfaceRequest: return 0x00685826u;
    case MainMenuPathStep::PublishScreenSet: return 0x006858BAu;
    case MainMenuPathStep::EnterScreen: return 0x004F88F4u;
    case MainMenuPathStep::ScreenVisible: return 0x004F8925u;
    }
    return 0u;
}

// ---------------------------------------------------------------------------
// Publishing one interface
// ---------------------------------------------------------------------------

void publish_main_menu_interface(int interface_id, MainMenuPathHost& host,
    MainMenuPathState& state) {
    const int screen_id = host.map_interface_to_screen(interface_id);
    state.published_screen_id = screen_id;

    if (screen_id == kFrontEndScreenIdNone) {
        // 00685938: PUSH 0; CALL 004F8710; ADD ESP,4; MOV AL,1; RET 8. One
        // argument, so the list is empty, and 004D8C00 is never reached. The
        // input contexts of the previous interface survive.
        host.publish_screen_set_level4(nullptr, kEmptyList);
        return;
    }

    // 006858BA: PUSH 0; PUSH id; CALL 004F8710.
    const int screen_ids[1] = {screen_id};
    host.publish_screen_set_level4(screen_ids, 1);

    // 006858E6: PUSH 0; PUSH 1; PUSH game; CALL 004D8C00.
    const int context_ids[1] = {kMainMenuPathInputContext};
    host.publish_input_context_set_level4(context_ids, 1);
}

// ---------------------------------------------------------------------------
// The sequence
// ---------------------------------------------------------------------------

MainMenuPathStep advance_main_menu_path(MainMenuPathStep step, MainMenuPathState& state,
    MainMenuPathHost& host, float raw_delta) {
    switch (step) {
    case MainMenuPathStep::PressStartPoll: {
        // 0067D2DD. Until the action goes down the screen only pulses its
        // prompt, which bsp::update_press_start_screen_0067cfb0 owns.
        if (!host.input_action_pressed(kPressStartInputAction)) {
            return MainMenuPathStep::PressStartPoll;
        }
        return MainMenuPathStep::TitleSkip;
    }

    case MainMenuPathStep::TitleSkip: {
        // 0068D8A6, unconditional.
        host.reset_storage_availability();
        // 0068D8B1: with a request already in the ring the skip does nothing at
        // all, not even release the hold byte, and the frame repeats.
        if (state.state_request_pending) {
            return MainMenuPathStep::TitleSkip;
        }
        return MainMenuPathStep::RequestShellState;
    }

    case MainMenuPathStep::RequestShellState: {
        // 0068D8BA: PUSH 4; CALL 004D7920.
        host.request_game_state(kGameStateFrontEndRequest);
        state.state_request_pending = true;
        // 0068D8C6: game+5ECh = 0, which is what lets the drain run.
        host.release_state_request_hold();
        state.state_requests_held = false;
        return MainMenuPathStep::DrainStateRequest;
    }

    case MainMenuPathStep::DrainStateRequest: {
        // 004E4D07: the drain is gated on the hold byte being clear.
        if (state.state_requests_held) {
            return MainMenuPathStep::DrainStateRequest;
        }
        const std::int32_t request = host.drain_state_request();
        state.state_request_pending = false;
        if (request != kGameStateFrontEndRequest) {
            // Some other request was at the head of the ring. The native keeps
            // draining; this routine only tracks the main-menu path.
            return MainMenuPathStep::DrainStateRequest;
        }
        return MainMenuPathStep::EnterShell;
    }

    case MainMenuPathStep::EnterShell: {
        // 004E4000 runs to completion: the title object is destroyed, the
        // GVMainMenu block loads and 00686380 Init builds the seven main-menu
        // screens. On return game+5D4h is still the 3 that 004E3AA0 wrote,
        // unless the platform poll aborted the shell.
        host.enter_front_end_shell();
        state.game_state = kGameStateFrontEndInit;
        state.manager_mode = host.main_menu_manager_mode();
        return MainMenuPathStep::PushInterfaceRequest;
    }

    case MainMenuPathStep::PushInterfaceRequest: {
        // 004E4250: CMP [ECX+4],4; JZ. The rewards entry already sits on the
        // applied record, so the shell leaves it alone.
        if (state.manager_mode != kMainMenuPathModeSkipPush) {
            host.push_interface_request(kMainMenuPathInterfaceId, nullptr);
            // 004CC460 writes the pending record only. Nothing is visible yet.
            state.pending_interface_id = kMainMenuPathInterfaceId;
        }
        return MainMenuPathStep::ActivateManager;
    }

    case MainMenuPathStep::ActivateManager: {
        // 004E4269, vtable +8h = 00684700: the mutual-exclusion activate that
        // deactivates every other manager and sets manager+3Ch.
        host.activate_main_menu_manager();
        state.manager_active = true;
        // 004E4274, 005884A0 on the 578h screen object at manager+58h.
        host.start_main_menu_screen_object();
        return MainMenuPathStep::PublishShellReady;
    }

    case MainMenuPathStep::PublishShellReady: {
        // 004E4279. From here OnMove stops taking the front-end branch.
        host.set_game_state(kGameStateFrontEndShellReady);
        state.game_state = kGameStateFrontEndShellReady;
        return MainMenuPathStep::ServiceInterfaceRequest;
    }

    case MainMenuPathStep::ServiceInterfaceRequest: {
        // 004E5442, 006840F0. Only reached because the state is no longer one
        // of 1, 2 or 4; the front-end branch returns before this point.
        if (front_end_screen_pump_site(state.game_state)
            != FrontEndScreenPumpSite::GameInterfaceOnly) {
            return MainMenuPathStep::ServiceInterfaceRequest;
        }
        // The manager is skipped entirely while manager+3Ch is clear.
        if (!state.manager_active) {
            return MainMenuPathStep::ServiceInterfaceRequest;
        }
        // 00684121: the virtual runs only while pending differs from applied.
        if (state.pending_interface_id == state.applied_interface_id) {
            return MainMenuPathStep::ServiceInterfaceRequest;
        }
        if (!host.service_pending_interface_requests()) {
            return MainMenuPathStep::ServiceInterfaceRequest;
        }
        return MainMenuPathStep::ApplyInterfaceRequest;
    }

    case MainMenuPathStep::ApplyInterfaceRequest: {
        // 00685826, 00684600. It rewrites the pending record from its own
        // arguments and then syncs applied = pending, which is what stops
        // 006840F0 firing again on the next pass.
        if (!host.apply_interface_request(state.pending_interface_id, nullptr)) {
            // 0068582E: AL is zero and the override returns without publishing.
            return MainMenuPathStep::ServiceInterfaceRequest;
        }
        state.applied_interface_id = state.pending_interface_id;
        return MainMenuPathStep::PublishScreenSet;
    }

    case MainMenuPathStep::PublishScreenSet: {
        publish_main_menu_interface(state.applied_interface_id, host, state);
        if (state.published_screen_id == kFrontEndScreenIdNone) {
            // The empty set hides everything and there is nothing to enter.
            return MainMenuPathStep::ServiceInterfaceRequest;
        }
        return MainMenuPathStep::EnterScreen;
    }

    case MainMenuPathStep::EnterScreen: {
        // 004C4165, the pump. Pass B finds the screen wanted but not active.
        const FrontEndScreenPhase phase = host.pump_front_end_screens(raw_delta);
        if (phase != FrontEndScreenPhase::Entering) {
            return MainMenuPathStep::EnterScreen;
        }
        // 004F88E4 sets +5h, 004F88E8 commits, and only then does 004F88F4
        // call the enter virtual, so the enter body observes itself visible.
        host.commit_screen_visibility(state.published_screen_id, true);  // +5h was set to 1 at 004F88E4
        host.enter_screen(state.published_screen_id);
        return MainMenuPathStep::ScreenVisible;
    }

    case MainMenuPathStep::ScreenVisible: {
        // 004F8925, pass C. The delta is loaded with FLD [ESP+0Ch] at
        // 004F891A and passed as a float, so it is the raw delta.
        host.update_screen(state.published_screen_id, raw_delta);
        return MainMenuPathStep::ScreenVisible;
    }
    }
    return step;
}

MainMenuPathStep run_main_menu_path(MainMenuPathState& state, MainMenuPathHost& host,
    float raw_delta, int max_steps) {
    MainMenuPathStep step = MainMenuPathStep::PressStartPoll;
    for (int i = 0; i < max_steps; ++i) {
        const MainMenuPathStep next = advance_main_menu_path(step, state, host, raw_delta);
        if (next == step) {
            // The guard did not open. The native repeats the frame; the caller
            // decides whether to run another one.
            return step;
        }
        step = next;
        if (step == MainMenuPathStep::ScreenVisible) {
            return step;
        }
    }
    return step;
}

} // namespace bsp
