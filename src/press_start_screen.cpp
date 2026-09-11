#include "bsp/press_start_screen.hpp"

#include <cmath>
#include <cstdio>

namespace bsp {
namespace {

// 0067d093..0067d0f5 and 0067cd7f..0067cded build the same argument block:
// slot, key, kind, 0, 0, "", 0.0f, 0, "", 1. Only the first three vary.
void raise_screen_prompt(PressStartScreenHost& host, int slot, const char* key, int kind) {
    host.raise_prompt(slot, key, kind);
}

// 0067d3f6 and the six other copies of the same three instructions: the idle arm
// ends by asking the 00E17664 singleton to process event 0.
void run_idle_tail(PressStartScreenHost& host) {
    host.frontend_tail_event(0);
}

// 0067cc9d..0067cd52 and 0067d494..0067d4d5 share the profile-side prologue.
// The gamertag string is built once and handed to both setters.
void reset_profile_and_identity(PressStartScreenHost& host) {
    host.reset_profile_007fdb20();
    host.reset_award_tracker_004374f0();
}

} // namespace

float advance_prompt_pulse_0067cfc8(PromptPulse& pulse, float delta_seconds) noexcept {
    // fld dt; fmul qword 00CE3DE0; fadd dword 00E19888; fstp dword. The store
    // through a 4-byte slot rounds the x87 result back to float before the fmod.
    const float advanced = static_cast<float>(
        static_cast<double>(delta_seconds) * kPromptPulseRate + static_cast<double>(pulse.phase));
    // fld advanced; fld qword 00CE3828; call 00bf857a; fstp dword 00E19888.
    pulse.phase = static_cast<float>(std::fmod(static_cast<double>(advanced), kPromptPulsePeriod));
    // fsin then `and eax, 7FFFFFFFh`: the sign bit is masked off the stored
    // float, so this is fabs of the rounded sine. The native sine is the x87
    // fsin of the 80-bit register value, which is not bit-identical to this.
    const float sine = static_cast<float>(std::sin(static_cast<double>(pulse.phase)));
    return std::fabs(sine);
}

PressStartScreen& construct_press_start_screen_0067c840(PressStartScreen& screen) noexcept {
    // BSP_FrontEndScreen_Construct 004f7180 clears +4h and +5h first.
    screen.base.wanted = false;
    screen.base.active = false;
    screen.sign_in_handled = 0;      // param_1[4], +10h
    screen.close_prompt_pending = false; // byte at +14h
    // *param_1 = &PTR_LAB_00cf6d90 has no reconstruction; the vtable is the
    // dispatch table this file's free functions stand in for.
    return screen;
}

void register_press_start_screen_0067ca80(PressStartScreen& screen, PressStartScreenHost& host) {
    host.register_screen(kPressStartScreenId); // 004f71d0, index from slot +0h
    screen.layout = host.load_layout(kPressStartLayoutName); // 00aa5840 -> +8h
}

void enter_press_start_screen_0067cb40(PressStartScreen& screen, PressStartScreenHost& host) {
    host.clear_page_context(); // 00518d60(0, 0, -1, "")
    screen.prompt_element = host.find_element(screen.layout, kPressStartElementName);
    screen.sign_in_handled = 0;
    screen.close_prompt_pending = false;
}

void exit_press_start_screen_0067c870(PressStartScreen& screen) noexcept {
    (void)screen; // 0067c870 is a bare RET
}

void collect_press_start_layouts_0067d860(const PressStartScreen& screen, void* sink,
    PressStartScreenHost& host) {
    host.publish_layout(sink, screen.layout); // 004d6790(sink, screen+8h)
}

void destruct_press_start_screen_0067c9e0(PressStartScreen& screen, PressStartScreenHost& host) {
    host.release_layout(screen.layout); // 004c12b0 then 00aa31f0
    screen.layout = 0;
    screen.prompt_element = 0;
    // BSP_FrontEndScreen_Destruct 004f71a0 then removes the object from the
    // registry; that walk belongs to bsp::FrontEndScreenTable, not here.
}

bool run_invite_fast_path_0067cf50(PressStartScreen& screen, PressStartGlobals& globals,
    PressStartScreenHost& host) {
    if (!globals.invite_pending) {
        return false; // 0067cf5c, the only false return
    }
    host.set_element_flag(screen.prompt_element, false);
    const SignInPhase phase = host.sign_in_phase();
    if (phase == SignInPhase::Idle) {
        host.request_sign_in(host.invitee_slot()); // 00a3e470 then 00a3f3d0
        return true;
    }
    if (phase == SignInPhase::Completed) {
        (void)apply_sign_in_0067cc60(screen, host);
        host.request_main_menu_state(); // 0068d8a0
    }
    // SignInPhase::Requested falls straight through: the frame is still owned.
    return true;
}

PressStartOutcome apply_sign_in_0067cc60(PressStartScreen& screen, PressStartScreenHost& host) {
    (void)screen; // ECX is the screen but the body only reaches globals

    reset_profile_and_identity(host);

    const char* const gamertag = host.signed_in_gamertag(); // 00a3eae0
    host.set_profile_name(gamertag);         // 007f9290
    host.set_profile_display_name(gamertag); // 007f9340
    // The XUID at manager+110h+index*8 goes to profile+48h, which is game+698h.
    host.set_profile_xuid(host.signed_in_xuid()); // 00a3eb00
    host.reset_save_manager_00bd3450();           // 00bd3450 on 0109CECC

    char save_name[128] = {};
    std::snprintf(save_name, sizeof(save_name), kSaveNameFormat,
        static_cast<unsigned long long>(host.save_id())); // 00a3e5d0

    if (!host.storage_device_required()) { // 0067cd55, [0109CECC]+21h == 0
        host.commit_profile_007fae70();
        if (!host.storage_busy()) { // [0109CECC]+8h == 0
            host.clear_all_prompt_slots();  // 00530650
            host.request_main_menu_state(); // 0068d8a0
            return PressStartOutcome::HandedOffToMainMenu;
        }
        return PressStartOutcome::SignInApplied;
    }

    if (host.storage_query_1c(save_name)) { // manager vtable +1Ch(save_name, 1)
        raise_screen_prompt(host, 0, kSavingDeviceMessageKey, 3);
        host.request_read_007ff100(save_name); // 007ff100 with 0067ca40
    } else if (host.storage_query_14(save_name)) { // manager vtable +14h(save_name)
        host.apply_input_settings();   // 005547d0 then 006ac030
        host.refresh_input_bindings(); // [00F8BBF4] +0Ch, +8h, then 008d44c0
        host.request_write_007fa710(save_name); // 007fa710 with 0067ca40
    }
    host.commit_profile_007fae70(); // 007fae70
    host.commit_input_manager();    // 00698a10 on the input manager + 3Ch
    return PressStartOutcome::SignInApplied;
}

namespace {

// 0067d16d..0067d415, the SignInPhase::Idle arm. ESI is reloaded with the device
// at 0067d1ae, so nothing past that point touches the screen object.
PressStartOutcome run_idle_arm(PressStartGlobals& globals, PressStartScreenHost& host,
    std::uint32_t prompt_element) {
    const bool menu_active = host.menu_screen_active();
    // SETZ AL at 0067d17d: the prompt is shown only while no message screen is up.
    host.set_element_flag(prompt_element, !menu_active);
    if (menu_active) {
        host.clear_prompt_slot(0); // 00532a20(menuCmd, 0)
    }

    void* device = host.primary_device(); // 004ba6d0([00F8BBF4], 2, 0)
    if (host.sign_in_blocked()) {         // manager+3E8h != 0
        run_idle_tail(host);
        return PressStartOutcome::Idle;
    }

    if (globals.primary_binding_valid) { // 00E1987C
        if (device == nullptr) {
            globals.primary_binding_valid = false;
        } else {
            host.rebind_primary_input(); // 0067c970, which sets 00E1987C again
            device = nullptr;            // XOR ESI,ESI: no press this frame
        }
    }

    if (globals.sign_in_blade_was_up) { // 00E19885
        host.rebind_primary_input();
        globals.sign_in_blade_was_up = false;
        run_idle_tail(host);
        return PressStartOutcome::Idle;
    }

    if (device != nullptr) { // 0067d206
        if (host.menu_screen_active()) {
            host.clear_prompt_slot(2);
            host.close_menu_screen(); // 004b6e50
            globals.press_consumed_by_prompt = true;
            run_idle_tail(host);
            return PressStartOutcome::PromptClosed;
        }
        if (!globals.press_consumed_by_prompt) { // 0067d29f
            host.request_sign_in(host.device_pad_index(device)); // device +34h, 00a3f3d0
            (void)host.sign_in_phase(); // 00a3e500, result discarded at 0067d2b0
            run_idle_tail(host);
            return PressStartOutcome::SignInRequested;
        }
        if (!host.device_button_held(device)) { // 0067c880
            host.rebind_primary_input();
            globals.press_consumed_by_prompt = false;
            run_idle_tail(host);
            return PressStartOutcome::Idle;
        }
        run_idle_tail(host); // the button is still down: 0067d26c jumps to the tail
        return PressStartOutcome::Idle;
    }

    if (host.action_pressed_this_frame(kPressStartInputAction)) { // 0067d2d7
        if (host.menu_screen_active()) {
            globals.press_consumed_by_prompt = true;
            if (!host.menu_command_pending()) { // +188h and +218h both zero
                host.clear_prompt_slot(2);
                host.close_menu_screen();
                run_idle_tail(host);
                return PressStartOutcome::PromptClosed;
            }
            run_idle_tail(host);
            return PressStartOutcome::Idle;
        }
        if (globals.press_consumed_by_prompt) { // 0067d35b
            if (!host.input_edge_latch()) {     // 004b43b0
                globals.press_consumed_by_prompt = false;
            }
            run_idle_tail(host);
            return PressStartOutcome::Idle;
        }
        host.request_sign_in(0); // 0067d39e, pad 0
        (void)host.sign_in_phase();
        run_idle_tail(host);
        return PressStartOutcome::SignInRequested;
    }

    if (globals.resume_sign_in && !host.menu_screen_active()) { // 0067d3cf
        host.request_sign_in(0);
        (void)host.sign_in_phase();
        globals.resume_sign_in = false;
    }
    run_idle_tail(host);
    return PressStartOutcome::Idle;
}

// 0067d439..0067d587, the SignInPhase::Completed arm.
PressStartOutcome run_completed_arm(PressStartScreen& screen, PressStartGlobals& globals,
    PressStartScreenHost& host) {
    if (screen.sign_in_handled == 0) {
        if (host.sign_in_blocked()) { // 0067d44d, manager+3E8h
            return PressStartOutcome::WaitingForSignIn;
        }
        screen.sign_in_handled = 1;
        globals.sign_in_blade_was_up = false;
        if (host.has_selected_user()) { // 00a3e510
            const PressStartOutcome outcome = apply_sign_in_0067cc60(screen, host);
            globals.resume_sign_in = true; // 00E08CC0 = 1
            return outcome == PressStartOutcome::HandedOffToMainMenu
                ? PressStartOutcome::HandedOffToMainMenu
                : PressStartOutcome::SignInApplied;
        }
        // 0067d494: no user selected, so the title goes on without a profile.
        host.reset_offline_profile();
        host.apply_input_settings();
        host.refresh_input_bindings();
        host.commit_input_manager();
        host.request_main_menu_state(); // 0068d8a0
        return PressStartOutcome::OfflineStart;
    }

    if (screen.close_prompt_pending && !host.menu_screen_active()) { // 0067d532
        screen.close_prompt_pending = false;
        host.clear_all_prompt_slots();  // 00530650
        host.request_main_menu_state(); // 0068d8a0
        return PressStartOutcome::HandedOffToMainMenu;
    }
    const bool busy = host.user_operation_busy(); // 00a3e540, manager+120h
    if (!screen.close_prompt_pending && !busy && host.menu_screen_active()) {
        screen.close_prompt_pending = true; // 0067d584
    }
    return PressStartOutcome::WaitingForSignIn;
}

} // namespace

PressStartOutcome update_press_start_screen_0067cfb0(PressStartScreen& screen,
    PressStartGlobals& globals, PromptPulse& pulse, float delta_seconds,
    PressStartScreenHost& host) {
    const float alpha = advance_prompt_pulse_0067cfc8(pulse, delta_seconds);
    host.set_element_color(screen.prompt_element, kPromptPulseRgb, kPromptPulseRgb,
        kPromptPulseRgb, alpha); // element vtable +50h

    if (run_invite_fast_path_0067cf50(screen, globals, host)) { // 0067cf50
        return PressStartOutcome::InviteHandled;
    }

    const SignInPhase phase = host.sign_in_phase(); // 00a3e500, read before the flag
    host.set_element_flag(screen.prompt_element, false); // 0067d081, always hides first

    if (host.profile_change_pending()) { // 00a3e3b0
        raise_screen_prompt(host, 2, kProfileChangedMessageKey, 2);
        host.reset_sign_in_state();  // 00a40020
        host.rebind_primary_input(); // 0067c970
        return PressStartOutcome::ProfileChangedPrompt;
    }

    switch (phase) {
    case SignInPhase::Idle:
        return run_idle_arm(globals, host, screen.prompt_element);
    case SignInPhase::Requested:
        globals.sign_in_blade_was_up = true; // 0067d41d
        return PressStartOutcome::WaitingForSignIn;
    case SignInPhase::Completed:
        return run_completed_arm(screen, globals, host);
    }
    return PressStartOutcome::Idle; // 0067d43c, any other value returns
}
}
