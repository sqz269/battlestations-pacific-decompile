#pragma once
// Routing and local delivery of a session message.
//
// Packet cc2_session_dispatch, worker agent/cc2-session-dispatch, 2026-09-11 UTC.
// Ghidra was read-only for this packet. Every descriptive name here is a
// hypothesis, not a recovered symbol. Evidence and coverage are in
// docs/SESSION_MESSAGE_DISPATCH.md; the addresses in the comments are the
// instructions the rule was read from.
//
// The native chain is
//   0077C2A0 route -> 0076E520 enqueue -> 00778450 pump -> 0076C600 drain
//   -> 00780670 gate -> 0077C710 / 00780120 -> the receiver.
// Only the routing decision, the queue discipline and the kind mapping are
// modelled here. The serialisation, the id tables, the network transport and
// every receiver body are contracts on the host.

#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// Session mode and route flags
// ---------------------------------------------------------------------------

// *(00E188A8)+1FE4h. game+1EF0h is the session object (docs/FIXED_STEP_FANOUT.md
// rows 9 and 13), so the router's world displacement 1FE4h and the pump's
// session displacement F4h (007784F8) are the same field.
inline constexpr std::size_t kSessionModeWorldOffset = 0x1FE4;
inline constexpr std::size_t kSessionModeSessionOffset = 0x00F4;

enum class SessionMode : int {
    // 007784FF takes the drain-only pump; 0077C32F forces local-only routing.
    kLocal = 0,
    // 0077C3BE broadcasts to the peer list; 0076FB60 sets the default flags to 1.
    kHost = 1,
    // 0077C3A2 sends to the host; 0076FCD7 sets the default flags to 2.
    kClient = 2,
};

// Bit values of the route-flag word, read at 0077C337, 0077C35E, 0077C3AB,
// 0077C3C6 and 0077C43A.
inline constexpr int kRouteFlagLocal = 0x1;  // 0076E520, the loopback queue
inline constexpr int kRouteFlagToHost = 0x2;  // 00779F90 -> 00770AF0
inline constexpr int kRouteFlagToPeers = 0x4;  // 00770B50 per peer

// [00E0AF1C], the process-wide default used when the call site passes 0.
// 0076FAD0 is its only writer: 1 for mode 1 (0076FBAF), 2 for mode 2 (0076FD2C)
// and 1 again when the session is torn down to mode 0 (0076FE26).
inline constexpr std::uint32_t kDefaultRouteFlagsAddress = 0x00E0AF1Cu;
int default_route_flags_for_mode(SessionMode mode) noexcept;

// 0077C2B7: the router does nothing below this game state, and 00780670
// dispatches nothing unless the state is exactly 0Dh (00780675).
inline constexpr int kRouteMinimumGameState = 0x0A;
inline constexpr int kDispatchGameState = 0x0D;

// What 0077C2A0 does with one message.
struct SessionRouteDecision {
    bool routed{false};        // false = the early returns at 0077C463/0077C460
    int flags{0};              // the resolved flag word after 0077C32F..0077C37C
    bool deliver_local{false};  // 0077C43D
    bool send_to_host{false};   // 0077C3A9 and the flag test at 0077C3AB
    bool send_to_peers{false};  // 0077C3C4 and the flag test at 0077C3C6
    bool mark_relay{false};     // msg+1Ah = 1 at 0077C36B
};

// Inputs of the decision. `privileged_kind` is 00779FF0: the message type is
// 81h, 82h, A7h, A8h or A9h, or msg->vtable[0Ch](4Ah) answers true.
struct SessionRouteInputs {
    bool session_object_present{true};  // [00E188A8] != 0 at 0077C2A8
    int game_state{kDispatchGameState};  // [game+5D4h] at 0077C2B7
    SessionMode mode{SessionMode::kLocal};
    int flags_override{0};         // the second argument, 0 = take the default
    int default_flags{1};          // [00E0AF1C]
    bool privileged_kind{false};   // 00779FF0
    bool audit_latch{false};       // [00E18DB7] at 0077C37F
};

SessionRouteDecision route_session_message_0077c2a0(const SessionRouteInputs& in) noexcept;

// ---------------------------------------------------------------------------
// The loopback queue at session+24Ch
// ---------------------------------------------------------------------------

// 0076E520 stores into it, 0076C600 (mode 0) and 00777850 (mode 1 and 2) drain it.
inline constexpr std::size_t kSessionQueueBaseOffset = 0x024C;    // 0076E5E6
inline constexpr std::size_t kSessionQueueCountOffset = 0x0250;   // 0076E5D9
inline constexpr std::size_t kSessionQueueCapacityOffset = 0x0254;  // 0076E601
inline constexpr std::size_t kSessionQueueCursorOffset = 0x0258;  // 0076E5D1

// 0076E535 SUB ESP,0x428 with EBP = 400h at 0076E561: the serialisation buffer
// the local path writes the message into before rebuilding it.
inline constexpr std::size_t kSessionLoopbackStreamCapacity = 0x400;

// The status word 00780670 writes through its out-parameter, and what 0076C600
// does with each value (0076C662, 0076C6E5, 0076C721).
enum class QueuedMessageStatus : int {
    kKeep = 0,      // leave it queued and go to the next entry
    kRelease = 1,   // unlink without destroying; ownership moved elsewhere
    kConsume = 2,   // unlink and call vtable[0](1)
};

// ---------------------------------------------------------------------------
// The category gate in 00780670
// ---------------------------------------------------------------------------

// Arguments the gate passes to msg->vtable[0Ch], the message's own is-a test.
inline constexpr int kMessageCategoryEntityCreate = 0x47;  // 0078069F
inline constexpr int kMessageCategorySubObject = 0x61;     // 007806EE
inline constexpr int kMessageCategoryLongWindow = 0x49;    // 00780751
inline constexpr int kMessageCategoryEntitySync = 0x48;    // 00780797
inline constexpr int kMessageCategoryPrivileged = 0x4A;    // 0077A010

// Tick windows compared against msg+0Ch - [00F876B0].
inline constexpr int kDispatchTickWindow = 0x3C;          // 00780755
inline constexpr int kDispatchTickWindowLong = 0x1770;    // 00780760
inline constexpr int kDispatchUnresolvedSenderAge = -0x258;  // 0078072B
inline constexpr int kDispatchMissingEntityAge = -0x0A;   // 007807C6

// Where the gate sends the message.
enum class QueuedMessageRoute {
    kNotDispatching,     // game state is not 0Dh (0078067F)
    kDeferredCreate,     // 47h: spliced onto 00F871A0 for 0077EC20
    kSubObject,          // 61h: 00805C60 on entity+1E8h+34h*msg[+24h]
    kRetryLater,         // status kKeep: too early, or the entity is not ready
    kDrop,               // too old: status kConsume without a receiver
    kEntitySync,         // 48h: 0077C710 -> entity->vtable[18Ch]
    kEntityGeneric,      // otherwise: 00780120
};

struct QueuedMessageInputs {
    int game_state{kDispatchGameState};
    bool is_entity_create{false};   // vtable[0Ch](47h)
    bool is_sub_object{false};      // vtable[0Ch](61h)
    bool is_long_window{false};     // vtable[0Ch](49h)
    bool is_entity_sync{false};     // vtable[0Ch](48h)
    bool sender_resolved{false};    // 00521E30 on msg+18h returned an entity
    bool entity_ready{false};       // entity+BDh != 0 at 00780789
    int tick_delta{0};              // msg+0Ch - [00F876B0]
};

struct QueuedMessageDecision {
    QueuedMessageRoute route{QueuedMessageRoute::kNotDispatching};
    QueuedMessageStatus status{QueuedMessageStatus::kKeep};
};

QueuedMessageDecision gate_queued_message_00780670(const QueuedMessageInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Kind to receiver
// ---------------------------------------------------------------------------

// 00780120 is an ordered ladder of msg->vtable[0Ch] tests, not a table. The
// order is the order of the native tests; the first match wins.
enum class EntityMessageReceiver {
    kNone,                  // 0077FE9D: nothing matched and the message is kept
    kPlayerSlot,            // 4Bh, 00780170..007803D9
    kPlayerState,           // 4Ch, 007803E9
    kSpawnPoint,            // 57h, 0078042C
    kDestroy,               // 4Eh, 00780461: entity+70h = msg+24h, vtable[70h]
    kTeamScore,             // 4Fh, 0078048D
    kRespawnTimer,          // 50h, 007804DF
    kMissionEvent,          // 51h, 0078051E
    kVoice,                 // 52h, 00780548
    kCamera,                // 97h, 00780591
    kUnitCommand,           // 58h, 00780611: entity->vtable[160h] = 00816E30
    kWeaponDirector,        // 59h family, 00780644: 00778820 then 00721A40
    kSessionKindSwitch,     // the fallthrough: 0077FE80
};

// The ladder's arguments, in test order (00780162 .. 00780636).
inline constexpr int kEntityMessageLadder[] = {
    0x4B, 0x4C, 0x57, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x97, 0x58, 0x59,
};

// One entry of the is-a set a message class answers true for. `count` is how
// many of `ids` are used.
struct MessageIsATest {
    std::uint8_t ids[4]{};
    int count{0};
    bool answers(int id) const noexcept;
};

EntityMessageReceiver receiver_for_message_00780120(const MessageIsATest& is_a) noexcept;

// The is-a sets read for this packet.
// 00764D00, the MT_COMMAND class at vtable 00D03630.
inline constexpr MessageIsATest kCommandMessageIsA{{0x58, 0x49, 0x46}, 3};
// 0075B090, the fire-target class at vtable 00D02E98 built by 00835740.
inline constexpr MessageIsATest kFireTargetMessageIsA{{0x5E, 0x59, 0x49, 0x46}, 4};
// 0071C640, quoted by docs/WEAPON_DIRECTOR.md.
inline constexpr MessageIsATest kDirectorMessageIsA{{0x5A, 0x59, 0x49, 0x46}, 4};

// 0077FE80: a second switch on the type byte for kinds 53h..78h, and
// entity->vtable[164h] for everything else (0078002F).
inline constexpr int kSessionKindSwitchFirst = 0x53;   // 0077FEAE ADD EAX,-0x53
inline constexpr int kSessionKindSwitchLast = 0x78;    // 0077FEB1 CMP EAX,0x25
inline constexpr std::size_t kEntityGenericHandlerSlot = 0x164;  // 00780032
inline constexpr std::size_t kEntityCommandHandlerSlot = 0x160;  // 00780613
inline constexpr std::size_t kEntitySyncHandlerSlot = 0x18C;     // 0077C79B
inline constexpr std::size_t kWeaponDirectorHandlerSlot = 0x38;  // 00721A8F

// 00821E80, the unit body of slot 164h: kind - 4Bh indexes the byte table at
// 00822400 and that selects one of 27 targets at 00822394. Kinds outside
// 4Bh..A0h, and the entries that select target 1Ah, call 0095ABE0, the base.
inline constexpr int kUnitKindSwitchFirst = 0x4B;
inline constexpr int kUnitKindSwitchLast = 0xA0;
inline constexpr int kUnitKindDetachPart = 0x99;  // 00821FF0 -> 0080E440
inline constexpr int kUnitKindDeath = 0x9A;       // 00821FD6 -> 00814560

// ---------------------------------------------------------------------------
// Host
// ---------------------------------------------------------------------------

// One virtual per native call site of the route, the enqueue and the drain.
// Nothing has a default body: none of these is a stand-in for game behaviour.
struct SessionMessageDispatchHost {
    virtual ~SessionMessageDispatchHost() = default;

    // 0077C2CC / 0077C2E8 entity->vtable[5Ch](id), the entity class test.
    virtual bool entity_is_kind(void* entity, int class_id) = 0;
    // 0077C2D6 the signed word at entity+528h, the player slot; negative means
    // unassigned and skips the discarded is-a call.
    virtual int entity_player_slot(void* entity) = 0;
    // 0077C2FB msg->vtable[0Ch](0D3h); the result is discarded at the call site.
    virtual void message_is_a_discarded(void* message, int id) = 0;
    // 0077C344 / 0077C36F 00779FF0(message).
    virtual bool message_is_privileged(void* message) = 0;
    // 0077C36B msg+1Ah = 1, the relay marker 00780147 reads on the host.
    virtual void mark_message_for_relay(void* message) = 0;
    // 0077C392 msg->vtable[0Ch](49h) behind the [00E18DB7] latch, then
    // 0077C398 msg+1Ch = 1.
    virtual bool message_is_long_window(void* message) = 0;
    virtual void set_message_audit_flag(void* message) = 0;
    // 0077C3B3 00779F90(entity, message): msg+18h = entity+174h, then
    // 00770AF0(session, message).
    virtual void send_message_to_host(void* entity, void* message) = 0;
    // 0077C3D1..0077C432, one call per node of the list at entity+2A4h.
    virtual void* first_peer(void* entity) = 0;
    virtual void* next_peer(void* entity, void* peer) = 0;
    // 0077C40E msg+18h = entity+174h, then 0077C423
    // 00770B50(session, peer_node+8 -> +50h, message).
    virtual void send_message_to_peer(void* peer, void* message) = 0;
    // 0077C44D 0076E520(session, message, entity).
    virtual void enqueue_local(void* session, void* message, void* sender) = 0;
    // 0077C45A *out = 0 when the local path ran and the out pointer is non-null.
    virtual void clear_caller_message_slot(void** out) = 0;

    // 0076E575 message->vtable[4](stream), the serialiser, and 0076E5A4
    // 00768530(stream), the factory that rebuilds the message from the bytes.
    virtual void serialize_message(void* message, void* stream) = 0;
    virtual void* create_message_from_stream(void* stream) = 0;
    // 0076E5B2 copy+18h = sender+174h, 0076E5C6 00782790(copy, [00F876B0]),
    // 0076E5CB copy+14h = message+14h.
    virtual void stamp_loopback_message(void* copy, void* source, void* sender) = 0;
    // 0076E5E0 [00CE221C](&session+250h), the interlocked append, and the
    // reallocating path at 0076E5F5.
    virtual void append_to_queue(void* session, void* message) = 0;

    // 0076C65D 00780670(message, &status), the gate.
    virtual QueuedMessageStatus gate_message(void* message) = 0;
    // 0076C67E msg+0Ch, the send tick the drop test compares against zero.
    virtual int message_send_tick(void* message) = 0;
    // 0076C69C 004254B0 with the name from [00E0AB68 + 4*msg+10h].
    virtual void log_dropped_message(void* message, int game_state) = 0;
    // 0076C6F8 message->vtable[0](1), the scalar deleting destructor.
    virtual void destroy_message(void* message) = 0;
};

// 0077C2A0 with the host attached. Returns the decision that was executed.
SessionRouteDecision run_route_0077c2a0(SessionMessageDispatchHost& host,
                                        const SessionRouteInputs& in,
                                        void* session,
                                        void* entity,
                                        void* message,
                                        void** out) noexcept;

// 0076E520: serialise, rebuild, stamp, append. Returns the queued copy.
void* run_enqueue_local_0076e520(SessionMessageDispatchHost& host,
                                 void* session,
                                 void* stream,
                                 void* message,
                                 void* sender) noexcept;

// 0076C600, the mode-0 drain. `queue` is the array of message pointers and
// `count` its length; both are updated in place. Returns how many entries were
// removed.
int run_drain_0076c600(SessionMessageDispatchHost& host,
                       void** queue,
                       int& count,
                       int game_state) noexcept;

}  // namespace bsp
