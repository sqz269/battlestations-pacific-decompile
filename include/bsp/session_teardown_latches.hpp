#pragma once
#include <cstdint>

#include "bsp/mission_state_entry.hpp"

// Five session lifecycle routes. Names are hypotheses, not recovered symbols.
// Valid-state projections, not native layouts or ABI replacements. See
// docs/SESSION_TEARDOWN_LATCHES.md for call-site evidence and preconditions.
namespace bsp {

struct SessionTeardownPeerMetadata {
    std::uint8_t field_1a{0}; // [peer+D50h]+1Ah
};

struct SessionTeardownPeer {
    // Projection of the transport list: null replaces its end sentinel.
    SessionTeardownPeer* next{nullptr};
    SessionTeardownPeerMetadata* metadata{nullptr}; // peer+D50h, nullable
};

struct SessionTeardownTransport {
    SessionTeardownPeer* first_peer{nullptr}; // first node's +8h value
    std::uint32_t field_9c{0};
};

struct SessionTeardownState {
    MissionOneShots one_shots{};
    std::int32_t local_view_mode{0}; // game+1FE4h
    std::uint32_t game_field_624{0};
    std::uint8_t session_field_274{0};
    std::uint8_t session_field_290{0};
    SessionTeardownTransport* primary{nullptr}; // session+188h
    SessionTeardownTransport* secondary{nullptr}; // session+18Ch
};

struct SessionGameClosedContext {
    bool owner_present{false}; // [00E198AC] != 0
    bool nested_present{false}; // owner+60h != 0
    bool nested_flag_04{false}; // [owner+60h]+4h != 0
};

enum class SessionLifecyclePayloadWidth : std::uint8_t { kNone, kByte, kDword };

// Projection of the stack event, deliberately not a serialized wire packet.
// The constructor contract supplies the base fields, including the source
// local-player pointer. Only payload_width bytes of payload_18 are defined.
struct SessionLifecycleMessage {
    std::uint32_t vtable_address{0};
    std::uint32_t field_04{0};
    std::uint32_t field_08{0};
    std::uint32_t field_0c{0};
    std::uint8_t type{0}; // native +10h
    std::uint32_t source_slot_address{0}; // native +14h
    SessionLifecyclePayloadWidth payload_width{SessionLifecyclePayloadWidth::kNone};
    std::uint32_t payload_18{0};
};

struct SessionTeardownHost {
    virtual ~SessionTeardownHost() = default;
    // 0075B430: this = stack message; +4h=3, +8h/+Ch=0, vtable
    // 00D02C68, +10h=type; +14h=selected local slot or null for index outside 0..7.
    virtual SessionLifecycleMessage construct_message_0075b430(std::uint8_t type) = 0;
    // 00770B50: this = session; stack(peer,message). Host owns serialization
    // and the nonlocal-peer/secondary-transport type-29h dispatch rule.
    virtual void send_to_peer_00770b50(
        SessionTeardownPeer& peer, const SessionLifecycleMessage& message) = 0;
    // 00770AF0: this = session, stack(message); uses the first peer of +18Ch
    // (null target if its list count is zero), and does nothing if +18Ch is null.
    virtual void send_to_secondary_first_peer_00770af0(
        const SessionLifecycleMessage& message) = 0;
    // 007848F0: this = selected transport; flushes three buffers per peer.
    virtual void flush_peer_buffers_007848f0(SessionTeardownTransport& transport) = 0;
    // 004D7D50: this = game, low stack byte selects the prompt suppression arm.
    virtual void show_game_closed_004d7d50(bool suppress_prompt) = 0;
    // 004D7970: this = game, stack(false), before the drop latch write.
    virtual void end_scene_004d7970(bool aborted) = 0;
};

bool session_game_closed_suppresses_prompt(const SessionGameClosedContext& context) noexcept;
bool session_game_closed_marks_drop(std::uint8_t message_flag_18) noexcept;

// The primary pointer is required for the three broadcast routines. Its list
// may be empty for 007727A0, but must contain the first/local node for 007728B0
// and 00772610: the originals invoke the checked-iterator failure on empty.
// A selected transport must exist at flush time. Host calls may update fields
// and metadata, but must preserve the active list nodes until traversal ends.
void receive_session_game_closed_0076d0e0(SessionTeardownState& state,
    std::uint8_t message_flag_18, const SessionGameClosedContext& context,
    SessionTeardownHost& host);
void broadcast_session_end_scene_007727a0(SessionTeardownState& state,
    std::uint8_t peer_flag_1a, SessionTeardownHost& host);
void broadcast_session_game_closed_007728b0(SessionTeardownState& state,
    std::uint8_t message_flag_18, SessionTeardownHost& host);
void receive_session_end_scene_00772990(
    SessionTeardownState& state, SessionTeardownHost& host);
void begin_session_mission_reload_00772610(
    SessionTeardownState& state, SessionTeardownHost& host);

} // namespace bsp
