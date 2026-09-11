#include "bsp/session_polls.hpp"

namespace bsp {
namespace {
// 004db4eb..004db8f0 always passes the empty string at 00ce3a0c as the title, so
// only the message keys appear here.
constexpr const char* kKeyProfileChanged = "FE_xbox.xsm_profilechanged"; // 00ce7c38
constexpr const char* kKeyStorageRemoved = "FE_xbox.xsm_storageremovedauto"; // 00ce7c18
constexpr const char* kKeyCrossPlatform = "FE.crossplatform_notsupported"; // 00ce7bf8
constexpr const char* kKeySameConsole = "FE_xbox.xsm_invitesameconsole"; // 00ce7bd8
constexpr const char* kKeyJoinWarning = "FE_xbox.xsm_joinwarning|.\n|"; // 00ce7b9c
constexpr const char* kKeyWillEndSession = "globals.willendsession|.\n|"; // 00ce7b80
constexpr const char* kKeyAreYouSure = "globals.areyousure"; // 00ce7b6c
constexpr const char* kKeyInviteOtherPlayer = "FE_xbox.xsm_inviteotherplayer"; // 00ce7b4c
constexpr const char* kKeyJoinOtherPlayer = "FE_xbox.xsm_joinotherplayer"; // 00ce7b30
}

const PlatformNotificationArm kPlatformNotificationArms[12] = {
    {0x00000009u, PlatformNotificationEffect::kSetSystemUiVisible},
    {0x0000000Au, PlatformNotificationEffect::kRefreshSignInState},
    {0x0000000Bu, PlatformNotificationEffect::kStorageHook},
    {0x0000000Eu, PlatformNotificationEffect::kProfileSettingChanged},
    {0x00000015u, PlatformNotificationEffect::kTitleUpdate},
    {0x00000016u, PlatformNotificationEffect::kSystemUpdateAndExit},
    {0x02000001u, PlatformNotificationEffect::kLiveConnectionChanged},
    {0x02000002u, PlatformNotificationEffect::kAcceptInvite},
    {0x02000003u, PlatformNotificationEffect::kLogOnly},
    {0x02000007u, PlatformNotificationEffect::kSetContentInstalled},
    {0x04000002u, PlatformNotificationEffect::kNotifyOnlineClient},
    {0x04000003u, PlatformNotificationEffect::kNotifyOnlineClient},
};

const PlatformNotificationArm* find_platform_notification_arm(std::uint32_t id) noexcept {
    for (const PlatformNotificationArm& arm : kPlatformNotificationArms) {
        if (arm.id == id) return &arm;
    }
    return nullptr;
}

void apply_platform_notification_00a40110(
    PlatformManagerFlags& flags, std::uint32_t id, std::uint32_t parameter) noexcept {
    const PlatformNotificationArm* arm = find_platform_notification_arm(id);
    if (arm == nullptr) return;
    switch (arm->effect) {
    case PlatformNotificationEffect::kSetSystemUiVisible:
        flags.system_ui_visible = parameter != 0u;
        break;
    case PlatformNotificationEffect::kAcceptInvite:
        flags.invite_accepted = true;
        break;
    case PlatformNotificationEffect::kSetContentInstalled:
        flags.content_installed = true;
        break;
    case PlatformNotificationEffect::kLiveConnectionChanged:
        // 00a403a6: only 80151005h writes a flag. 001510F0h calls 00a3fa70(1),
        // which is a call-out, and every other value falls through to the next
        // XNotifyGetNext.
        if (parameter == 0x80151005u) flags.link_failure = true;
        break;
    default:
        // The remaining arms only call out; they touch none of these bytes.
        break;
    }
}

bool profile_changed_pending_00a3e3b0(const PlatformManagerFlags& flags) noexcept {
    return flags.profile_changed &&
        flags.profile_overlapped_3c0.words[0] != kAsyncStatusPending;
}

bool invite_already_in_session_004db509(const InviteDecisionInputs& in) noexcept {
    // 004db509: the whole check is skipped when game+610h is set.
    return !in.suppressed && in.signed_in && in.invitee_slot == in.active_slot && in.session_active
        && in.already_in_invited_session; // 004db55b
}

InviteOutcome classify_accepted_invite_004db4eb(const InviteDecisionInputs& in) noexcept {
    if (invite_already_in_session_004db509(in)) return InviteOutcome::kIgnored;
    // 004db574. A different title's invite is dropped without a prompt, after
    // the dialog layers have already been dismissed at 004db56d.
    if (in.title_id != kTitleId) return InviteOutcome::kIgnored;
    // 004db596: XNetXnAddrToPlatform. The prompt is raised only when the call
    // succeeded and reported something other than the supported platform; a
    // failed call falls through to the normal path.
    if (in.host_platform_known && in.host_platform != kSupportedHostPlatform) {
        return InviteOutcome::kCrossPlatformNotSupported;
    }
    // 004db620: the inviter resolves to a local slot, so the invite came from
    // this console.
    if (in.inviter_slot != kNoLocalSlot) return InviteOutcome::kInviteSameConsole;
    // 004db69d: the multiplayer front-end accepts without asking.
    if (in.game_state == 2) return InviteOutcome::kAcceptImmediately;
    // 004db6c0: the invitee is the active local profile.
    if (in.signed_in && in.invitee_slot == in.active_slot) {
        return InviteOutcome::kJoinWarningPrompt;
    }
    return InviteOutcome::kOtherPlayerPrompt; // 004db7fe
}

const char* invite_outcome_text_key(InviteOutcome outcome, bool from_game_invite) noexcept {
    switch (outcome) {
    case InviteOutcome::kCrossPlatformNotSupported:
        return kKeyCrossPlatform;
    case InviteOutcome::kInviteSameConsole:
        return kKeySameConsole;
    case InviteOutcome::kJoinWarningPrompt:
        return kKeyJoinWarning;
    case InviteOutcome::kOtherPlayerPrompt:
        // 004db805 tests info+50h: an invite the player sent through the guide
        // versus a join-session-in-progress.
        return from_game_invite ? kKeyInviteOtherPlayer : kKeyJoinOtherPlayer;
    default:
        return nullptr;
    }
}

InvitePromptText compose_join_warning_text_004db6e7(bool would_end_session) noexcept {
    InvitePromptText text{};
    // 004db6e7 assigns FE_xbox.xsm_invitewarning|.\n| and 004db718 replaces it
    // with the join variant when 00a3e470 and 00a3eac0 agree. Reaching 004db6e7
    // already required that they agree (004db6df), so the invite variant is
    // unreachable and only the join key is emitted here.
    text.parts[text.count++] = kKeyJoinWarning;
    if (would_end_session) text.parts[text.count++] = kKeyWillEndSession; // 004db731
    text.parts[text.count++] = kKeyAreYouSure; // 004db75f
    return text;
}

OnlineStatsWriteStep online_stats_write_step_004caa90(
    OnlineStatsWriteState& state, std::uint32_t overlapped_result, std::size_t& pending_writes) noexcept {
    if (!state.write_in_flight) return OnlineStatsWriteStep::kSkipped; // 004caab5
    if (state.overlapped_internal_low == kAsyncStatusPending) {
        return OnlineStatsWriteStep::kSkipped; // 004caac6
    }
    // XGetOverlappedResult and XGetOverlappedExtendedError, then the log line
    // " - - ONLINE - - Update Stats Write Status code %x and %x" and 004cab0f.
    state.write_in_flight = false;
    if (overlapped_result == 0u) {
        // 004cab1b: nothing queued means the run is finished.
        if (pending_writes == 0u) return OnlineStatsWriteStep::kQueueDrained;
        --pending_writes; // 004cab33, the front entry is erased
        if (pending_writes == 0u) return OnlineStatsWriteStep::kQueueDrained;
        return OnlineStatsWriteStep::kSubmitNext; // 004cab9f resubmits through 004c04c0
    }
    // A failed write does not pop, so the same front entry is submitted again.
    if (pending_writes == 0u) return OnlineStatsWriteStep::kQueueDrained;
    return OnlineStatsWriteStep::kRetryFront;
}

bool menu_channel_has_pending_request(const MenuRequestChannel& channel) noexcept {
    if (!channel.present || !channel.active) return false;
    return channel.current_a != channel.target_a || channel.current_b != channel.target_b;
}

bool service_pending_menu_requests_006840f0(
    MenuInterfaceState& menus, MenuRequestServicer& servicer) {
    bool serviced = false;
    for (std::size_t i = 0; i < kMenuRequestChannelCount; ++i) {
        MenuRequestChannel& channel = menus.channels[i];
        if (!menu_channel_has_pending_request(channel)) continue;
        servicer.service_menu_channel(i, channel.target_a, channel.target_b);
        serviced = true;
    }
    return serviced;
}

InviteOutcome poll_platform_session_events_004db290(SessionPollHost& host) {
    host.pump_platform_manager_00a409f0(); // 004db2bc
    if (host.system_ui_flag_00e188ae()) host.on_system_ui_raised_004ceb40(); // 004db2ca

    // 004db2d8. The state test is part of the same condition, so a profile change
    // that lands on the title screen falls through to the storage arm instead.
    if (host.profile_changed_pending() && host.game_state() != 2) {
        if (host.game_flag_620()) host.game_620_handler_004d94f0(); // 004db2fa
        if (host.game_flag_61f()) host.game_61f_handler_004d95f0(); // 004db30c
        if (host.game_state() == 4) {
            host.profile_lost_in_state4_004d7f90(); // 004db31c
        } else {
            host.teardown_menu_objects_004db190(); // 004db323
            host.teardown_session_004cccc0(); // 004db32a
            host.clear_game_flag_2180(); // 004db331
            host.on_init_title_004c9a70(); // 004db338
        }
        PromptRequest prompt{};
        prompt.text.parts[prompt.text.count++] = kKeyProfileChanged;
        host.show_prompt_00531b00(prompt); // 004db3a6
        return InviteOutcome::kIgnored; // 004db3fa, the arm returns
    }

    // 004db411. The flag is consumed whether or not the prompt is raised.
    if (host.take_storage_removed() && host.storage_owner_idle_0109cecc()) {
        host.storage_removed_side_effect_00bd3450(); // 004db431
        PromptRequest prompt{};
        prompt.text.parts[prompt.text.count++] = kKeyStorageRemoved;
        prompt.kind = kPromptKindStorage; // 004db497 is the only site passing 0
        host.show_prompt_00531b00(prompt); // 004db4a0
        // 004db4eb falls through into the invite arm; there is no return here.
    }

    // 004db4eb. Below state 2 the invite flag is not read, so an invite that
    // arrives during the logo sequence stays pending.
    if (host.game_state() < 2) return InviteOutcome::kIgnored;
    if (!host.take_invite_accepted()) return InviteOutcome::kIgnored; // 004db4fc

    InviteDecisionInputs inputs{};
    host.invite_decision_inputs(inputs);
    if (invite_already_in_session_004db509(inputs)) return InviteOutcome::kIgnored;

    // 004db561: every path that gets past the in-session check dismisses the
    // dialog layers, including the two that then drop the invite silently.
    host.dismiss_dialog_layers_00530650();

    const InviteOutcome outcome = classify_accepted_invite_004db4eb(inputs);
    switch (outcome) {
    case InviteOutcome::kIgnored:
        break; // 004db57b, the title id did not match
    case InviteOutcome::kAcceptImmediately:
        host.accept_invite_004d8000(); // 004db6a8
        break;
    case InviteOutcome::kJoinWarningPrompt: {
        PromptRequest prompt{};
        prompt.text = compose_join_warning_text_004db6e7(host.leaving_ends_session_004bb8a0());
        prompt.style = kPromptStyleConfirm;
        prompt.callback = kJoinWarningCallback;
        prompt.flag = 1; // 004db7cc, the only site that passes 1
        host.show_prompt_00531b00(prompt); // 004db7e3
        break;
    }
    case InviteOutcome::kOtherPlayerPrompt: {
        PromptRequest prompt{};
        prompt.text.parts[prompt.text.count++]
            = invite_outcome_text_key(outcome, inputs.from_game_invite);
        prompt.style = kPromptStyleConfirm;
        prompt.callback = kInviteAcceptCallback;
        host.show_prompt_00531b00(prompt); // 004db877 / 004db8e6
        break;
    }
    default: {
        PromptRequest prompt{};
        prompt.text.parts[prompt.text.count++]
            = invite_outcome_text_key(outcome, inputs.from_game_invite);
        host.show_prompt_00531b00(prompt); // 004db60a / 004db68e
        break;
    }
    }
    return outcome;
}

void run_session_polls(SessionPollHost& host) {
    poll_platform_session_events_004db290(host); // 004e4b91
    host.update_online_stats_write_004caa90(); // 004e4b98
}

void run_multiplayer_tick_00778560(SessionPollHost& host, float seconds) {
    host.multiplayer_pre_tick_00778450(seconds); // 0077856b
    host.multiplayer_post_tick_0076ffc0(seconds); // 0077857c, second argument 0
}

int run_menu_interface_drain_004e5434(SessionPollHost& host) {
    host.update_multiplayer_interface_004d80d0(); // 004e5434

    MenuInterfaceState menus{};
    host.read_menu_interface_state(menus);
    bool pending = service_pending_menu_requests_006840f0(menus, host); // 004e5442
    host.clear_menu_transition_latch_00e18cdc(); // 004e544c

    int iterations = 0;
    while (pending) { // 004e5453 / 004e5488
        host.pump_peer_messages_00776230(); // 004e5462
        host.update_interface_only_004c40f0(); // 004e5469
        ++iterations;
        host.read_menu_interface_state(menus);
        pending = service_pending_menu_requests_006840f0(menus, host); // 004e5477
        host.clear_menu_transition_latch_00e18cdc(); // 004e5481
    }
    return iterations;
}
}
