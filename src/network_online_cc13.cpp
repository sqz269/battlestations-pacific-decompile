// XLive online system, voice chat and session helpers (packet cc13-sweepA1).
// Evidence and ABI per function: docs/CC13_BAND_00A1_00A4.md. Each body below
// was written from the live decompile and checked against the listing where the
// comment says so. Reconstructed and build-tested only; nothing here is wired
// into the game hosts, fixture-tested or game-validated.
#include "bsp/network_online_cc13.hpp"

#include <utility>

namespace bsp::xlive_cc13 {

// 00994100, slot +198h of vtable 00D24878: CMP [ECX+60h],0; SETE AL; RET.
bool is_live_mode_00994100(NetworkMode mode) { return mode == NetworkMode::Live; }

// 00994110, slot +19Ch: CMP [ECX+60h],1; SETE AL; RET.
bool is_system_link_mode_00994110(NetworkMode mode) { return mode == NetworkMode::SystemLink; }

// 00A47900, slot +180h: the low byte is 1 for states 2, 3 and 4 (the upper bytes
// of EAX carry the state's own bits, which no caller reads).
bool is_session_active_00a47900(std::int32_t session_state_21f0) {
    return session_state_21f0 == 2 || session_state_21f0 == 3 || session_state_21f0 == 4;
}

// 00A45010, slot +CCh. Listing 00A45010..00A4507D: EBP runs 0..2Ch in steps of 4
// over [EBP+00D24734]; STL_LegacySboString_CompareSubstring_004BEB60(0, size,
// text, strlen(text)) == 0 assigns the argument into the slot record through
// BSP_LegacySboString_AssignSubstring 00408120; the loop never breaks; AL = 0.
VoiceMaskLookup set_voice_mask_by_name_00a45010(bool voice_active, std::string_view name) {
    VoiceMaskLookup result;
    if (!voice_active) return result;
    for (std::size_t i = 0; i < kVoiceMaskNames.size(); ++i) {
        if (name == kVoiceMaskNames[i]) result.matched_index = static_cast<std::int32_t>(i);
    }
    return result;
}

// 00A44EC0, slot +74h. Listing 00A44F04..00A44FD7 and jump table 00A44FF4
// (seven entries for 80151000h..80151006h): 0 and 4 connection lost, 1 cannot
// access service, 2 unknown, 3 servers too busy, 5 duplicate logon, 6 invalid user.
std::string_view last_error_text_00a44ec0(bool flag_124, std::uint32_t error_1bd8) {
    if (flag_124 && error_1bd8 != 0) return "live_err_update_required";
    const auto signed_error = static_cast<std::int32_t>(error_1bd8);
    if (signed_error <= static_cast<std::int32_t>(0x80155100u)) {  // JG 00A44F9E is signed
        if (error_1bd8 == 0x80155100u) return "live_err_invalid_session_id";
        switch (error_1bd8 - 0x80151000u) {
        case 0:
        case 4: return "live_err_connection_lost";
        case 1: return "live_err_cannot_access_service";
        case 3: return "live_err_servers_too_busy";
        case 5: return "live_err_duplicate_logon";
        case 6: return "live_err_invalid_user";
        default: return "live_err_unknown";  // case 2 and JA 00A44FB0
        }
    }
    if (error_1bd8 == 0x80155109u) return "Session Match overflow";
    if (error_1bd8 == 0) return "";  // 00A44840 erases the "Unknown" seed
    if (error_1bd8 == 0x60192009u) return "sys_link_no_cable_err";
    return "live_err_unknown";
}

// ---------------------------------------------------------------------------
// Voice chat wrapper
// ---------------------------------------------------------------------------

// 00A49340: linear search over talker_count records, stride 38h.
std::int32_t find_remote_talker_00a49340(const VoiceState& voice, std::uint64_t xuid,
                                         std::uint32_t* index) {
    for (std::uint32_t i = 0; i < voice.talker_count; ++i) {
        if ((*voice.talkers)[i].xuid == xuid) {
            if (index) *index = i;
            return 0;
        }
    }
    return kEFail;
}

// 00A49110, listing 00A49110..00A4916F: nothing when +84h is 0; with +C4h set
// the priority is -1; else record+8, or -1 when record+18h or record+28h is set.
// Only user slot 0 is consulted and passed (PUSH 0 at 00A49142).
std::int32_t apply_talker_priority_00a49110(VoiceState& voice, const VoiceTalkerRecord& record) {
    if (voice.local_registered[0] == 0) return 0;
    std::int32_t priority = -1;
    if (voice.mute_all[0] == 0) {
        priority = record.priority[0];
        if (record.mute_list[0] != 0 || record.game_mute[0] != 0) priority = -1;
    }
    voice.engine->set_playback_priority(record.xuid, 0, priority);
    return 0;
}

// 00A49180: stores +C4h[slot], then repeats 00A49110's body inline for every
// talker (the inline copy also skips the call when +84h is 0).
std::int32_t set_mute_all_and_reapply_00a49180(VoiceState& voice, std::uint32_t slot,
                                               std::uint32_t value) {
    voice.mute_all[slot] = value;
    for (std::uint32_t i = 0; i < voice.talker_count; ++i) {
        apply_talker_priority_00a49110(voice, (*voice.talkers)[i]);
    }
    return 0;
}

namespace {
template <typename Field>
std::int32_t set_talker_field(VoiceState& voice, std::uint64_t xuid, std::uint32_t slot,
                              std::int32_t value, Field field) {
    std::uint32_t index = 0;
    if (find_remote_talker_00a49340(voice, xuid, &index) < 0) return kEFail;
    VoiceTalkerRecord& record = (*voice.talkers)[index];
    (record.*field)[slot] = value;
    return apply_talker_priority_00a49110(voice, record);
}
}  // namespace

// 00A49710 returns ((result >= 0) - 1) & 80004005h, i.e. 0 because 00A49110
// always returns 0; 00A49780 and 00A497E0 return 0 outright.
std::int32_t set_talker_priority_00a49710(VoiceState& voice, std::uint64_t xuid,
                                          std::uint32_t slot, std::int32_t value) {
    const std::int32_t hr = set_talker_field(voice, xuid, slot, value, &VoiceTalkerRecord::priority);
    if (hr == kEFail) return kEFail;
    return hr >= 0 ? 0 : kEFail;
}

std::int32_t set_talker_mute_list_flag_00a49780(VoiceState& voice, std::uint64_t xuid,
                                                std::uint32_t slot, std::int32_t value) {
    const std::int32_t hr = set_talker_field(voice, xuid, slot, value, &VoiceTalkerRecord::mute_list);
    return hr == kEFail ? kEFail : 0;
}

std::int32_t set_talker_game_mute_flag_00a497e0(VoiceState& voice, std::uint64_t xuid,
                                                std::uint32_t slot, std::int32_t value) {
    const std::int32_t hr = set_talker_field(voice, xuid, slot, value, &VoiceTalkerRecord::game_mute);
    return hr == kEFail ? kEFail : 0;
}

// 00A49680: moves the last record over the found one (REP MOVSD of 0Eh dwords),
// decrements the count, then XHV2 +2Ch; returns the HRESULT only when negative.
std::int32_t remove_remote_talker_00a49680(VoiceState& voice, std::uint64_t xuid) {
    std::uint32_t index = 0;
    if (find_remote_talker_00a49340(voice, xuid, &index) < 0) return kEFail;
    (*voice.talkers)[index] = (*voice.talkers)[voice.talker_count - 1];
    --voice.talker_count;
    const std::int32_t hr = voice.engine->unregister_remote_talker(xuid);
    return hr < 0 ? hr : 0;
}

// 00A49070, listing 00A49070..00A490BE: XHV2 +2Ch for each record, count = 0.
std::int32_t unregister_all_remote_talkers_00a49070(VoiceState& voice) {
    for (std::uint32_t i = 0; i < voice.talker_count; ++i) {
        voice.engine->unregister_remote_talker((*voice.talkers)[i].xuid);
    }
    voice.talker_count = 0;
    return 0;
}

// 00A49270, listing 00A49270..00A492CE: unregister all, _free (00BF6989) the
// array, UnregisterLocalTalker(0) when +84h (no engine null check), Release
// when the engine exists, then +78h, +7Ch and +104h = 0.
std::int32_t shutdown_00a49270(VoiceState& voice, VoiceHost& host) {
    unregister_all_remote_talkers_00a49070(voice);
    if (voice.talkers) {
        host.free_talker_array(voice.talkers);
        voice.talkers = nullptr;
    }
    if (voice.local_registered[0] != 0) {
        voice.local_registered[0] = 0;
        voice.engine->unregister_local_talker(0);
    }
    if (voice.engine) {
        voice.engine->release();
        voice.engine = nullptr;
    }
    voice.talker_count = 0;
    voice.field_104 = 0;
    return 0;
}

// ---------------------------------------------------------------------------
// XSession wrapper
// ---------------------------------------------------------------------------

// 00A49A00: JMP [state*4+00A49A34] for states 0..4; 0 and 3 share the default.
std::int32_t flag_restart_00a49a00(SessionState& session) {
    switch (session.state) {
    case 1:
    case 2: session.restart_flag = 1; return 0;
    case 4: session.state = 2; return 0;
    default: return kEUnexpected;
    }
}

// 00A49B00, listing 00A49B00..00A49B2B: once per +C0h; returns XSessionStart's
// result (EAX of the import), else 0.
std::int32_t start_00a49b00(SessionState& session, SessionHost& host) {
    if (session.started != 0) return 0;
    session.started = 1;
    session.ended = 0;
    return host.x_session_start();
}

// 00A49B30, listing 00A49B30..00A49B8B: nothing once +C4h is set; starts first
// when needed (result dropped), then XSessionEnd and returns its result.
std::int32_t end_00a49b30(SessionState& session, SessionHost& host) {
    if (session.ended != 0) return 0;
    if (session.started == 0) {
        session.started = 1;
        session.ended = 0;
        host.x_session_start();
    }
    session.started = 0;
    session.ended = 1;
    return host.x_session_end();
}

// 00A4ADC0, listing 00A4ADC0..00A4AE60: End only when the state is non-zero;
// states <= 0 and > 4 return 0; 1..3 cancel the overlapped and reset; 1..4
// then clear +15Ch and the overlapped, log, and unless +C5h delete with state 3.
std::int32_t delete_00a4adc0(SessionState& session, SessionHost& host) {
    if (session.state != 0) end_00a49b30(session, host);
    const std::int32_t state = session.state;
    if (state <= 0 || state > 4) return 0;
    if (state <= 3) {
        host.x_cancel_overlapped();
        session.state = 0;
        session.restart_flag = 0;
    }
    session.key_registered = 0;
    session.overlapped.fill(0);
    host.trace(" - - ONLINE - - CSesssion Delete ");
    if (session.keep_session != 0) return 0;
    const std::int32_t result = host.x_session_delete_overlapped();
    session.state = 3;
    return result;
}

// 00A4B1F0: states 0, 2, 4 return 0; 1 completes the create; 3 stops the QoS
// listener and completes the delete; others 8000FFFFh; a failure terminates XNet.
std::int32_t poll_overlapped_00a4b1f0(SessionState& session, SessionHost& host) {
    std::int32_t result = 0;
    switch (session.state) {
    case 0:
    case 2:
    case 4: return 0;
    case 1: result = host.complete_create_00a4b030(); break;
    case 3:
        host.stop_qos_listen_00a49c80();
        result = host.complete_delete_00a4af20();
        break;
    default: return kEUnexpected;
    }
    if (result < 0) host.terminate_xnet_00a4a790();
    return result;
}

}  // namespace bsp::xlive_cc13
