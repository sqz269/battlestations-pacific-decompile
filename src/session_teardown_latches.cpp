#include "bsp/session_teardown_latches.hpp"

namespace bsp {
namespace {
SessionTeardownTransport& flush_transport(SessionTeardownState& state) noexcept {
    return *(state.primary != nullptr ? state.primary : state.secondary);
}

void finish_stack_message(SessionLifecycleMessage& message) noexcept {
    // The normal-path destructor is this store, with no call or heap release.
    message.vtable_address = 0x00CE4974u;
}
} // namespace

bool session_game_closed_suppresses_prompt(const SessionGameClosedContext& context) noexcept {
    return context.owner_present && context.nested_present && context.nested_flag_04;
}

bool session_game_closed_marks_drop(std::uint8_t message_flag_18) noexcept {
    return message_flag_18 == 0;
}

void receive_session_game_closed_0076d0e0(SessionTeardownState& state,
    std::uint8_t message_flag_18, const SessionGameClosedContext& context,
    SessionTeardownHost& host) {
    if (message_flag_18 != 0) {
        state.game_field_624 = 2; // 0076D0F0: no prompt and no drop write
        return;
    }
    host.show_game_closed_004d7d50(session_game_closed_suppresses_prompt(context));
    state.one_shots.session_dropped = true; // 0076D12C, after the call
}

void broadcast_session_end_scene_007727a0(SessionTeardownState& state,
    std::uint8_t peer_flag_1a, SessionTeardownHost& host) {
    auto* const transport = state.primary; // 007727C0, before +290h
    state.session_field_290 = peer_flag_1a;
    bool first = true;
    for (auto* peer = transport->first_peer; peer != nullptr; peer = peer->next) {
        if (!first) {
            auto message = host.construct_message_0075b430(0x12u);
            message.field_04 = 1;
            message.vtable_address = 0x00D03160u;
            host.send_to_peer_00770b50(*peer, message);
            finish_stack_message(message);
        }
        // 00772836 reloads peer+D50h after dispatch; do not cache it before.
        if (peer->metadata != nullptr) {
            peer->metadata->field_1a = peer_flag_1a;
        }
        first = false;
    }
    host.flush_peer_buffers_007848f0(flush_transport(state));
    state.one_shots.session_dropped = true;
}

void broadcast_session_game_closed_007728b0(SessionTeardownState& state,
    std::uint8_t message_flag_18, SessionTeardownHost& host) {
    for (auto* peer = state.primary->first_peer->next; peer != nullptr; peer = peer->next) {
        auto message = host.construct_message_0075b430(0x13u);
        message.field_04 = 1;
        message.vtable_address = 0x00CE74DCu;
        message.payload_width = SessionLifecyclePayloadWidth::kByte;
        message.payload_18 = message_flag_18;
        host.send_to_peer_00770b50(*peer, message);
        finish_stack_message(message);
    }
    host.flush_peer_buffers_007848f0(flush_transport(state));
    if (session_game_closed_marks_drop(message_flag_18)) {
        state.one_shots.session_dropped = true; // nonzero argument preserves it
    }
}

void receive_session_end_scene_00772990(
    SessionTeardownState& state, SessionTeardownHost& host) {
    state.session_field_274 = 1;
    auto message = host.construct_message_0075b430(0x12u);
    message.field_04 = 1;
    message.vtable_address = 0x00D03160u;
    host.send_to_secondary_first_peer_00770af0(message);
    finish_stack_message(message);
    host.flush_peer_buffers_007848f0(flush_transport(state));
    host.end_scene_004d7970(false);
    state.one_shots.session_dropped = true;
}

void begin_session_mission_reload_00772610(
    SessionTeardownState& state, SessionTeardownHost& host) {
    state.one_shots.session_was_networked = state.local_view_mode != 0;
    state.one_shots.not_enough_players = false;
    if (state.primary->field_9c != 4) {
        state.primary->field_9c = 4;
    }
    for (auto* peer = state.primary->first_peer->next; peer != nullptr; peer = peer->next) {
        auto message = host.construct_message_0075b430(0x0Bu);
        message.field_04 = 1;
        message.vtable_address = 0x00D0314Cu;
        message.payload_width = SessionLifecyclePayloadWidth::kDword;
        message.payload_18 = 0;
        host.send_to_peer_00770b50(*peer, message);
        finish_stack_message(message);
    }
    host.flush_peer_buffers_007848f0(flush_transport(state));
}

} // namespace bsp
