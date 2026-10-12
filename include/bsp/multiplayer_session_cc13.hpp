#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "bsp/native_smoothed_remainder_triplet.hpp"
#include "bsp/session_message_dispatch.hpp"

// Multiplayer session plumbing in partition segment 43 (0076AA00..00787040):
// the session receive ladder 00777850, the message-type name table 00E0AB68,
// the voice-chat mute table at session+190h, the voice/player-slot/lifecycle
// receive handlers, and their host-side broadcasts. Packet cc13-net1.
// Evidence and per-function ABI: docs/CC13_MULTIPLAYER_SESSION.md.
//
// The session object is the MMultiPlayer instance embedded at game+1EF0h
// (00E188A8 is the game; strings "MMultiPlayer::SetMuted()" 00771630 and
// "__MMultiPlayer::ServerSendSceneScoring()" 0076D1B0 name the class). Every
// handler here is __thiscall with ECX = session. Names are hypotheses, except
// the MT_* identifiers, which are read from the image's own table 00E0AB68.
//
// These are valid-state projections over a host interface, not native layouts
// or drop-in ABI replacements; only VoiceMuteTable is a byte-exact layout.
namespace bsp::mp_cc13 {

// ---------------------------------------------------------------------------
// Message types (00E0AB68)
// ---------------------------------------------------------------------------

// 00E0AB68: 235 char* entries, MT_INVALID (0) .. MT_LAST (EAh), static data
// (no writer; read at 0076C68F, 0077839E and 00788280 as [type*4+00E0AB68]
// in the "multi msg (%s) from tick (%d) is dropped" logs).
inline constexpr std::uint32_t kMessageNameTableAddress = 0x00E0AB68u;
inline constexpr std::uint8_t kMessageTypeLast = 0xEA;  // "MT_LAST"

// The session-level types this file handles. Values are the image's.
enum class MessageType : std::uint8_t {
    kAddPlayer = 0x04, kServerInfo = 0x05, kRemovePlayer = 0x06, kCreated = 0x07,
    kSceneName = 0x08, kSceneIdx = 0x09, kRefreshScene = 0x0A, kSceneLoad = 0x0B,
    kSceneLoaded = 0x0C, kCheckSceneLoad = 0x0D, kLoadingInfo = 0x0E, kConnTest = 0x0F,
    kConnState = 0x10, kLetsPlay = 0x11, kSceneTerm = 0x12, kQuit = 0x13, kKicked = 0x14,
    kSceneScoring = 0x15, kSceneObjective = 0x16, kChangeMultiSettings = 0x17,
    kSubScene = 0x18, kSelectPlayer = 0x19, kSwapSlotRequest = 0x1A,
    kSwapSlotAccept = 0x1B, kSwapSlotRefuse = 0x1C, kReadyPlayer = 0x1D,
    kAiEnable = 0x1E, kAiSkillLevel = 0x1F, kPlayerSurrender = 0x20, kChat = 0x21,
    kHint = 0x22, kCounter = 0x23, kPlayerPing = 0x24, kMyPlayer = 0x25,
    kAddRemoteVoice = 0x26, kRemoveRemoteVoice = 0x27, kMuteRemoteVoice = 0x28,
    kVoiceData = 0x29, kMultiSelectUnit = 0x4D, kPlayerUnlockUnit = 0xDD,
    kSessionManager = 0xE4, kVoiceChange = 0xE8,
};

// [type*4 + 00E0AB68]; nullptr above MT_LAST (the table's next words are
// unrelated data, not names).
const char* message_type_name_00e0ab68(std::uint8_t type) noexcept;

// ---------------------------------------------------------------------------
// The receive ladder 00777850
// ---------------------------------------------------------------------------

// How the ladder calls a handler. ESI = message, EDI = session, and the
// status word is the stack local [ESP+14h], set to 2 before each message.
enum class HandlerArgs : std::uint8_t {
    kNone,               // no call; the arm is inline in 00777850
    kSessionMsgStatus,   // ECX=session, push &status, push msg, RET 8
    kSessionMsg,         // ECX=session, push msg, RET 4
    kSessionPeer,        // ECX=session, push [msg+8] (0Ah)
    kSessionTwoDwords,   // ECX=session, push [msg+1Ch], push [msg+18h]
    kSessionPayloadWord, // ECX=session, push [msg+20h], push msg+18h (18h)
    kMessageThis,        // ECX=msg, no arguments
    kOther,              // a callee outside this segment; see the doc table
};

// The arm tested at the given site, and the handler(s) it calls. When the
// arm splits on session+188h (the server transport), `host_handler` is the
// non-null arm and `client_handler` the null arm; otherwise both are equal.
struct ReceiveRoute {
    bool matched{false};          // false: no is-a test answered (status stays 2)
    std::uint32_t test_site{0};   // the CALL [vtable+0Ch] for this type
    std::uint32_t host_handler{0};
    std::uint32_t client_handler{0};
    HandlerArgs host_args{HandlerArgs::kNone};
    HandlerArgs client_args{HandlerArgs::kNone};
};

// Inline arms (HandlerArgs::kNone): 0Eh MT_LOADINGINFO stores
// (msg+18h == 0) into session+274h in client mode and 0 otherwise; 07h
// MT_CREATED stores 8 into [msg+8]+0Ch; 0Fh MT_CONNTEST and C2h
// MT_PLANE_SYNC_ACK do nothing; E9h MT_METRICSSESSION forwards msg+18h to
// the metrics object [00E1AED4] when present. 0Ch's client arm calls
// 004DA6C0 only when game+624h is 0; 15h's arm clears [00F8A2FC]+74h after
// its call; 36h's arm sets status 0 when msg->vtable[10h]() is false.

// Routing only. The native tests msg->vtable[0Ch](type) in a fixed order;
// this assumes a message class answers its own type byte (true for every
// class this packet inspected; a class answering several types would take
// the earliest arm in ladder order, which the doc lists).
ReceiveRoute route_receive_00777850(std::uint8_t type) noexcept;

// What the queue walk does after the arm (0077836B..0077841E).
enum class QueueOutcome : std::uint8_t {
    kKeepAndAdvance,     // status 0 and no drop log: stays queued (0077843D)
    kUnlink,             // status 1: removed, not destroyed
    kUnlinkAndDestroy,   // status 2: removed, then vtable[0](1)
};

struct DropLogInputs {
    bool game_present{false};  // [00E188A8] != 0
    std::int32_t game_state{0};  // game+5D4h
    std::int32_t message_tick{0};  // msg+0Ch
};

// Status 0 is logged as "multi msg (%s) from tick (%d) is dropped in games"
// (string at 00D03868) and destroyed only when the game exists, its state is
// below 0Ah and the message tick is positive; otherwise it is kept. Any other
// status than 0 or 2 unlinks without destroying.
QueueOutcome receive_queue_outcome_00777850(QueuedMessageStatus status,
    const DropLogInputs& inputs, bool& logs_drop) noexcept;

// 0077788D..007778D6: a null game or a null queue entry is unlinked and, when
// non-null, destroyed without being routed.
inline bool receive_skips_routing_00777850(bool game_present, bool message_present) noexcept {
    return !game_present || !message_present;
}

// ---------------------------------------------------------------------------
// The voice mute table, session+190h (game+2080h)
// ---------------------------------------------------------------------------

// Byte-exact: constructed and reset by 00778610/00778590 (already
// reconstructed as reset_native_game_peer_pairs_00778590). muted[a][b] = 1
// means participant a muted b. `xuid` is the 64-bit online id and `active`
// marks a used row. 007786E0 is its only reader outside this file
// (005D0770 chat filter, see include/bsp/hud_widgets.hpp ChatMuteHost).
struct VoiceMuteTable {
    std::uint8_t muted[8][8]{};   // +00h
    std::uint64_t xuid[8]{};      // +40h, stored as lo/hi dwords
    std::uint8_t active[8]{};     // +80h
};
static_assert(sizeof(VoiceMuteTable) == 0x88);
static_assert(offsetof(VoiceMuteTable, xuid) == 0x40);
static_assert(offsetof(VoiceMuteTable, active) == 0x80);
inline constexpr std::size_t kSessionVoiceMuteTableOffset = 0x190;

// 007785D0, __thiscall(table, xuid_lo, xuid_hi), RET 8: first active row
// whose xuid matches, else -1.
int voice_mute_find_007785d0(const VoiceMuteTable& table, std::uint64_t xuid) noexcept;
// 00778620, RET 8: no-op when present or when all eight rows are active;
// else the first inactive row takes the xuid, is marked active, and both its
// row and its column are cleared.
void voice_mute_add_00778620(VoiceMuteTable& table, std::uint64_t xuid) noexcept;
// 00778690, RET 8: clears the row's xuid, active byte, row and column.
void voice_mute_remove_00778690(VoiceMuteTable& table, std::uint64_t xuid) noexcept;
// 00778780 (value 1) / 007787C0 (value 0), RET 10h: muted[find(a)][find(b)]
// when both are found (a == b allowed).
void voice_mute_set_00778780(VoiceMuteTable& table, std::uint64_t a, std::uint64_t b) noexcept;
void voice_mute_clear_007787c0(VoiceMuteTable& table, std::uint64_t a, std::uint64_t b) noexcept;
// 007786E0, RET 10h: both found, distinct, and either direction muted. The
// false return leaves the high bytes of EAX as the second index (AL = 0).
bool voice_mute_is_muted_007786e0(const VoiceMuteTable& table, std::uint64_t a,
                                  std::uint64_t b) noexcept;

// ---------------------------------------------------------------------------
// Session, game and player-record projections
// ---------------------------------------------------------------------------

inline constexpr std::size_t kPlayerRecordBase = 0x748;    // game+748h, 8 records
inline constexpr std::size_t kPlayerRecordStride = 0x118;
inline constexpr std::size_t kCurrentRecordTable = 0x18CC; // game+18CCh, 8 pointers
inline constexpr std::size_t kLocalPlayerIndex = 0x18C8;   // game+18C8h
inline constexpr std::size_t kLocalSlotIndex = 0x18EC;     // game+18ECh

// Fields of one 118h record that this packet reads or writes. Roles are
// hypotheses taken from the handlers named in each comment.
struct PlayerRecord {
    std::uint8_t claimed_08{0};       // +08h, 004BB440 sets it (session_participant_pools.hpp)
    std::uint8_t ai_held_09{0};       // +09h
    std::uint8_t ai_enabled_0a{0};    // +0Ah, MT_AIENABLE 0076CB90
    std::uint8_t ready_0c{0};         // +0Ch, MT_READYPLAYER 0076CA40
    std::uint8_t ready_aux_0d{0};     // +0Dh, MT_READYPLAYER second byte
    std::uint8_t loaded_0e{0};        // +0Eh, MT_SCENELOADED host arm 0076D090
    std::uint16_t state_10{0};        // +10h, FFFDh on ready/loaded, 1 on MT_LETSPLAY
    std::uint8_t byte_18{0};          // +18h, cleared with +10h
    std::uint8_t byte_19{0};          // +19h, cleared by 007712F0
    std::uint8_t surrender_1b{0};     // +1Bh, MT_PLAYERSURRENDER 00770750
    std::int32_t owner_player_1c{-1}; // +1Ch, read by MT_SWAPSLOTACCEPT on the slot record
    std::int32_t slot_20{-1};         // +20h, the bound slot index (-1 none)
    std::int32_t team_28{0};          // +28h, team for chat channels 9/10 (00770D40)
    std::uint32_t ai_skill_2c{0};     // +2Ch, MT_AISKILLLEVEL 0076CC40
    void* peer_50{nullptr};           // +50h, transport peer, null for the local player
    std::string name_58;              // +58h, inline char array in the image
    std::uint64_t xuid_80{0};         // +80h/+84h
    std::uint8_t voice_registered_89{0}; // +89h, MT_ADD/REMOVEREMOTEVOICE, MT_VOICECHANGE
    std::uint32_t last_voice_time_8c{0}; // +8Ch, 007F7490 stores game+64Ch
    std::uint8_t byte_90{0};             // +90h, MT_ADDPLAYER msg+60h
    std::uint32_t word_94{0};            // +94h, MT_ADDPLAYER msg+61h zero-extended
    std::uint32_t unlock_class_110{0};   // +110h (game+858h + i*118h), MT_PLAYERUNLOCKUNIT
};

struct GameView {
    PlayerRecord players[8];          // game+748h
    PlayerRecord* current[8]{};       // game+18CCh, into players[] or the mission pool
    std::int32_t local_player{0};     // game+18C8h
    std::int32_t local_slot{0};       // game+18ECh
    std::int32_t state{0};            // game+5D4h
    std::int32_t mode{0};             // game+1FE4h (== session+F4h)
    std::uint32_t game_624{0};        // game+624h
    std::uint32_t time_64c{0};        // game+64Ch
    std::uint8_t local_view_1ee3{0};  // game+1EE3h
    std::uint32_t scene_2060{0};      // game+2060h
    std::uint32_t scene_2064{0};      // game+2064h
    bool has_scene_5fc{false};        // game+5FCh != 0
};

struct SessionState {
    SessionMode mode{SessionMode::kLocal};   // +F4h
    bool server_transport{false};            // +188h != 0
    bool client_transport{false};            // +18Ch != 0
    std::uint32_t client_stage_9c{0};  // [+18Ch]+9Ch
    std::uint8_t client_scene_match_c4{0};   // [+18Ch]+C4h
    std::int32_t phase_248{0};               // +248h
    std::uint64_t nonce_268{0};              // +268h/+26Ch
    std::uint8_t loading_274{0};             // +274h
    std::uint32_t movcam_count_284{0};       // +284h
    VoiceMuteTable mute;                     // +190h
};

// A message this file builds on the stack. Base contract 0075B430 (vtable
// written over by the caller, +4h = 1 after construction, +10h = type,
// +14h = selected local slot record); payload holds bytes +18h..+2Bh.
struct BuiltMessage {
    std::uint8_t type{0};
    std::uint32_t vtable{0};
    std::array<std::uint8_t, 0x14> payload{};
    void put32(std::size_t offset, std::uint32_t value) noexcept;  // offset from msg+0
    void put8(std::size_t offset, std::uint8_t value) noexcept;
    std::uint32_t get32(std::size_t offset) const noexcept;
    std::uint8_t get8(std::size_t offset) const noexcept;
};

// The received message as the handlers read it.
struct ReceivedMessage {
    std::uint8_t type{0};
    std::int32_t tick_0c{0};
    void* peer_08{nullptr};            // msg+8h, the sending transport peer
    PlayerRecord* source_14{nullptr};  // msg+14h
    std::array<std::uint8_t, 0x28> payload{};  // msg+18h..+3Fh
    std::uint32_t get32(std::size_t offset) const noexcept;
    std::uint8_t get8(std::size_t offset) const noexcept;
    std::uint16_t get16(std::size_t offset) const noexcept;
    std::uint64_t get64(std::size_t offset) const noexcept;
};

// Vtables the builders install (scalar deleting destructors named in Ghidra).
inline constexpr std::uint32_t kVtableRefreshScene0A = 0x00D03138u;
inline constexpr std::uint32_t kVtableCheckSceneLoad0D = 0x00D03034u;
inline constexpr std::uint32_t kVtableKicked14 = 0x00D03174u;
inline constexpr std::uint32_t kVtableSelectPlayer19 = 0x00D03070u;
inline constexpr std::uint32_t kVtableSwapRequest1A = 0x00D03084u;
inline constexpr std::uint32_t kVtableSwapAccept1B = 0x00D03098u;
inline constexpr std::uint32_t kVtableSwapRefuse1C = 0x00D030ACu;
inline constexpr std::uint32_t kVtableReadyPlayer1D = 0x00D030D4u;
inline constexpr std::uint32_t kVtableAiEnable1E = 0x00D030FCu;
inline constexpr std::uint32_t kVtableAiSkill1F = 0x00D03110u;
inline constexpr std::uint32_t kVtableSurrender20 = 0x00D03124u;
inline constexpr std::uint32_t kVtableAddVoice26 = 0x00D0323Cu;
inline constexpr std::uint32_t kVtableRemoveVoice27 = 0x00D03250u;
inline constexpr std::uint32_t kVtableMuteVoice28 = 0x00D03264u;
inline constexpr std::uint32_t kVtableCreated07 = 0x00D03188u;

// 0076BFE0, __thiscall(msg, a_lo, a_hi, b_lo, b_hi, mute), RET 14h:
// MT_MUTEREMOTEVOICE, +18h a, +20h b, +28h byte.
BuiltMessage build_mute_remote_voice_0076bfe0(std::uint64_t a, std::uint64_t b, bool mute) noexcept;
// 0076BA80, __thiscall(msg, player, slot), RET 8: MT_SWAPSLOTREFUSE.
BuiltMessage build_swap_slot_refuse_0076ba80(std::int32_t player, std::int32_t slot) noexcept;

// ---------------------------------------------------------------------------
// Host
// ---------------------------------------------------------------------------

// XPRIVILEGE_COMMUNICATIONS / _FRIENDS_ONLY, pushed at 00771654 / 00771675.
inline constexpr std::uint32_t kPrivilegeCommunications = 0xFC;
inline constexpr std::uint32_t kPrivilegeCommunicationsFriendsOnly = 0xFB;

struct SessionHost {
    virtual ~SessionHost() = default;
    // 004254B0, the debug print (formatting not modelled).
    virtual void trace(const char* text) = 0;
    // 00784790 __thiscall(transport, msg, exclude_peer), RET 8: serialize to
    // every peer of the transport except `exclude_peer` (null = all). The
    // callers here pass ECX = the server transport (session+188h).
    virtual void broadcast_except_00784790(const void* message, void* exclude_peer) = 0;
    // 00770AF0 __thiscall(session, msg): first peer of +18Ch.
    virtual void send_to_server_00770af0(const BuiltMessage& message) = 0;
    // 00770B50 __thiscall(session, peer, msg).
    virtual void send_to_peer_00770b50(void* peer, const void* message) = 0;
    // 00784B10 __thiscall(server transport, peer, flag), RET 8: MT_KICKED with
    // +18h = flag to `peer` when it is in the transport's list.
    virtual void kick_peer_00784b10(void* peer, std::uint8_t flag) = 0;
    // [00F8A2FC] online manager virtuals.
    virtual void online_add_remote_talker_ac(std::uint64_t xuid) = 0;    // +ACh
    virtual void online_remove_remote_talker_b0(std::uint64_t xuid) = 0; // +B0h
    virtual void online_submit_voice_a8(const void* voice_payload) = 0;  // +A8h, msg+18h
    virtual void online_scene_load_54() = 0;                             // +54h
    // 007F7490 __thiscall(record, 1): record+8Ch = game+64Ch.
    virtual void stamp_voice_time_007f7490(PlayerRecord& record) = 0;
    // 00A3EB00 on [00F8ABE8]: the signed-in user's xuid.
    virtual std::uint64_t local_xuid_00a3eb00() = 0;
    // XUserCheckPrivilege(user, privilege, &result): returns the Win32 code.
    virtual std::uint32_t check_privilege(std::uint32_t privilege, bool& allowed) = 0;
    // XUserAreUsersFriends(user, &xuid, 1, &result, 0).
    virtual std::uint32_t are_friends(std::uint64_t xuid, bool& friends) = 0;
    // 00985920(record+20h, "surrender"/"notsurrender") when [00F8A0C4] != 0.
    virtual bool script_events_present_00f8a0c4() = 0;
    virtual void post_slot_event_00985920(std::int32_t slot, const char* event) = 0;
    // Game-side participant routines (outside this segment).
    virtual void restore_mission_record_004bb660(std::int32_t slot) = 0;
    virtual void bind_record_to_slot_004bb630(PlayerRecord& record, std::int32_t slot) = 0;
    virtual void select_local_slot_004b4740(std::int32_t slot) = 0;
    // GUI refreshes guarded by [00E198B4]+48h / [00E198AC]&&[00E198B4].
    virtual void refresh_lobby_005d9d60() = 0;
    virtual void leave_tactical_library_if_open() = 0;  // 00687860(0Bh,0) then 00687830
    virtual void swap_request_local_005d8800() = 0;
    virtual void swap_refused_local_005d8950() = 0;
    virtual void swap_slot_names_005cf320(const std::string& slot_name,
                                          const std::string& player_name) = 0;
    // 0076D030 tail: 004D7920(10), BSP_Game_RequestState.
    virtual void request_game_state_004d7920(std::int32_t state) = 0;
    // 0076CEA0: 0052CF60 on the HUD root when [00E198C4]+C8h is set.
    virtual bool hud_counter_present() = 0;
    virtual void hud_counter_0052cf60() = 0;
    // 0076CFD0: [[00E198C4]+40h]+78h; false when the HUD root is absent.
    virtual void set_ship_screen_selection(std::uint32_t value) = 0;
    // 0076C7B0: [game+207Ch]+9A8h/+9ACh.
    virtual void publish_nonce_207c(std::uint64_t nonce) = 0;
    // MT_KICKED client arm: [session+18Ch]->vtable[3Ch]().
    virtual void client_transport_3c() = 0;
    // MT_SCENETERM host arm: [msg+8]+D50h, null-checked; +1Ah = 0.
    virtual void clear_peer_metadata_1a(void* peer) = 0;
};

// ---------------------------------------------------------------------------
// Voice handlers
// ---------------------------------------------------------------------------

// 0076D7D0, __thiscall(session, lo, hi), RET 8 (ECX unused): the first
// claimed (+8h) player record whose +80h xuid matches, else null.
PlayerRecord* find_player_by_xuid_0076d7d0(GameView& game, std::uint64_t xuid) noexcept;

// 0076D840 / 0076D890 (client arm) and 00772D20 / 00772D80 (server arm) of
// MT_ADDREMOTEVOICE / MT_REMOVEREMOTEVOICE, (session, msg, &status), RET 8.
void client_receive_remote_voice_0076d840(GameView& game, const ReceivedMessage& msg,
                                          bool add, SessionHost& host);
void server_receive_remote_voice_00772d20(SessionState& session, GameView& game,
                                          const ReceivedMessage& msg, bool add,
                                          SessionHost& host);
// 00772BB0, __thiscall(session, lo, hi, add), RET 0Ch: in host mode,
// broadcast type 26h/27h excluding the owner's peer (+50h); otherwise send it
// to the server. Then the record's +89h = add.
void set_remote_voice_00772bb0(SessionState& session, GameView& game, std::uint64_t xuid,
                               bool add, SessionHost& host);
// 0076D8E0, MT_MUTEREMOTEVOICE (session, msg, &status): set/clear
// mute[a][b]; with a server transport and a != b, rebroadcast to all.
void receive_mute_remote_voice_0076d8e0(SessionState& session, const ReceivedMessage& msg,
                                        SessionHost& host);
// 0076D9C0, MT_VOICEDATA (session, msg, &status).
void receive_voice_data_0076d9c0(GameView& game, const ReceivedMessage& msg, SessionHost& host);
// 0076DA10, MT_VOICECHANGE (session, msg, &status): +89h = msg+18h for the
// record of xuid msg+20h; host mode rebroadcasts excluding that record's peer.
void receive_voice_change_0076da10(const SessionState& session, GameView& game,
                                   const ReceivedMessage& msg, SessionHost& host);
// 00771630, MMultiPlayer::SetMuted(target, mute), RET 0Ch.
void set_muted_00771630(SessionState& session, std::uint64_t target, bool mute,
                        SessionHost& host);

// ---------------------------------------------------------------------------
// Player-slot bookkeeping
// ---------------------------------------------------------------------------

// 0076CA40 (player, ready, aux), RET 0Ch; MT_READYPLAYER receiver 0076CAF0
// calls it unless host mode outside game state 8.
void set_player_ready_0076ca40(const SessionState& session, GameView& game, std::int32_t player,
                               std::uint8_t ready, std::uint8_t aux, SessionHost& host);
void receive_ready_player_0076caf0(const SessionState& session, GameView& game,
                                   const ReceivedMessage& msg, SessionHost& host);
// 0076CB90 (slot, enabled) / 0076CC40 (slot, level), RET 8, on the current
// record; receivers 0076CC20 / 0076CCD0 unpack msg+18h/+1Ch.
void set_slot_ai_enabled_0076cb90(const SessionState& session, GameView& game, std::int32_t slot,
                                  std::uint8_t enabled, SessionHost& host);
void set_slot_ai_skill_0076cc40(const SessionState& session, GameView& game, std::int32_t slot,
                                std::uint32_t level, SessionHost& host);
// 00770750 (player, surrender), RET 8; receiver 00770850 passes msg+1Ch.
void set_player_surrender_00770750(const SessionState& session, GameView& game,
                                   std::int32_t player, bool surrender, SessionHost& host);
// 0076CB30, MT_PLAYERUNLOCKUNIT (session, msg), RET 4.
void receive_player_unlock_0076cb30(GameView& game, const ReceivedMessage& msg,
                                    const void* raw_message, SessionHost& host);
// 0076C840 (player, slot), RET 8: the host's select-player. slot -1 unbinds.
void host_select_player_0076c840(GameView& game, std::int32_t player, std::int32_t slot,
                                 SessionHost& host);
// 007705A0 (player, slot), RET 8: the client's apply of MT_SELECTPLAYER.
void client_select_player_007705a0(GameView& game, std::int32_t player, std::int32_t slot,
                                   SessionHost& host);
// 00771990 / 00771AC0 / 00771CC0 (player, slot), RET 8: swap request,
// accept, refuse.
void swap_slot_request_00771990(const SessionState& session, GameView& game,
                                std::int32_t player, std::int32_t slot, SessionHost& host);
void swap_slot_accept_00771ac0(const SessionState& session, GameView& game,
                               std::int32_t player, std::int32_t slot, SessionHost& host);
void swap_slot_refuse_00771cc0(const SessionState& session, GameView& game,
                               std::int32_t player, std::int32_t slot, SessionHost& host);

// ---------------------------------------------------------------------------
// Lifecycle receivers
// ---------------------------------------------------------------------------

// 0076C7B0, MT_MYPLAYER: game+18C8h = msg+18h; nonce = (msg+1Ch << 32) +
// msg+20h (00BF9640 __allshl with EDX = 0, then ADD/ADC).
void receive_my_player_0076c7b0(SessionState& session, GameView& game,
                                const ReceivedMessage& msg, SessionHost& host);
// 0076CFD0, MT_MULTISELECTUNIT: for the local slot, the u16 at msg+1Ch.
void receive_multi_select_unit_0076cfd0(const GameView& game, const ReceivedMessage& msg,
                                        SessionHost& host);
// 0076D030 MT_SCENELOAD, 0076D090 MT_SCENELOADED (host arm),
// 0076D140 MT_QUIT (host arm), 0076D170 MT_KICKED, 0076D260 MT_SCENETERM
// (host arm), 0076CEA0 MT_COUNTER, 00771FC0 MT_LETSPLAY.
void receive_scene_load_0076d030(GameView& game, SessionHost& host);
void host_receive_scene_loaded_0076d090(const GameView& game, const ReceivedMessage& msg);
void host_receive_quit_0076d140(const SessionState& session, const ReceivedMessage& msg,
                                SessionHost& host);
void receive_kicked_0076d170(SessionState& session, const ReceivedMessage& msg,
                             SessionHost& host);
void host_receive_scene_term_0076d260(const ReceivedMessage& msg, SessionHost& host);
void receive_counter_0076cea0(const SessionState& session, SessionHost& host);

struct LetsPlayHost {
    virtual ~LetsPlayHost() = default;
    virtual void reset_mission_clock_00874640(std::uint32_t arg) = 0;
    virtual void store_ingame_field_1c_00644220(std::uint32_t value) = 0;
    virtual void send_to_server_00770af0(const BuiltMessage& message) = 0;
};
// 007712F0, __fastcall session: once per raise of [+18Ch]+9Ch to 8, clear
// current[local slot]+19h (slot 0..7 only), game+193Ch, the in-game field,
// and send MT_CREATED to the server.
void client_enter_play_007712f0(SessionState& session, GameView& game,
                                std::uint8_t& game_193c, LetsPlayHost& host);
// 00771FC0, MT_LETSPLAY (session, msg, &status).
void receive_lets_play_00771fc0(SessionState& session, GameView& game,
                                const ReceivedMessage& msg, std::uint8_t& game_193c,
                                LetsPlayHost& host);

// ---------------------------------------------------------------------------
// Batch 2: ready request, chat relay, scene selection, player removal, scoring
// ---------------------------------------------------------------------------

// 00771DD0 (player, ready, aux), RET 0Ch: host applies 0076CA40; a client
// sends MT_READYPLAYER (vtable 00D030D4) to the server; local mode does nothing.
void request_player_ready_00771dd0(const SessionState& session, GameView& game,
                                   std::int32_t player, std::uint8_t ready, std::uint8_t aux,
                                   SessionHost& host);

// A chat line as 005D0770 receives it (msg+18h sender, +20h channel, +24h
// word, +28h text, +30h second text, +38h/+3Ch sender xuid).
struct ChatLine {
    std::string sender;        // NativeString at msg+18h
    std::int32_t channel{0};   // msg+20h: 0..7 player, 8 all, 9/10 team 0/1
    std::uint32_t word_24{0};  // msg+24h
    std::string text;          // msg+28h
    std::string second;        // msg+30h
    std::uint64_t sender_xuid{0};  // msg+38h/+3Ch
};

struct ChatHost {
    virtual ~ChatHost() = default;
    virtual void trace(const char* text) = 0;
    // 00770B50 (session, peer, msg).
    virtual void send_to_peer_00770b50(void* peer, const void* message) = 0;
    // 005D0770 on the chat screen; `channel_arg` is 0 for channel 0 and the
    // channel otherwise. [00E198B4] null: not reachable on the client arm.
    virtual bool chat_screen_present_00e198b4() = 0;
    virtual void add_chat_line_005d0770(const ChatLine& line, std::int32_t channel_arg) = 0;
};

// 00770C50 (msg, sender lo, sender hi, exclude peer), RET 10h: in host or
// client mode, every claimed, non-AI player record whose peer is not
// `exclude_peer`, that has not muted or been muted by the sender (007786E0)
// and is not the sender, gets the message; a record without a peer traces
// "###%d#Nem_kuldom_el mert nincs peerInfoja!_4 %x" (Hungarian: not sent,
// no peer info).
void send_to_unmuted_00770c50(const SessionState& session, GameView& game, const void* message,
                              std::uint64_t sender, void* exclude_peer, ChatHost& host);
// 00770D40 (team, msg, sender lo, sender hi, exclude peer), RET 14h: the same
// filter restricted to records with a peer and record+28h == team, silent
// when the peer is null.
void send_to_team_unmuted_00770d40(const SessionState& session, GameView& game,
                                   std::int32_t team, const void* message, std::uint64_t sender,
                                   void* exclude_peer, ChatHost& host);
// 00772020 (msg), RET 4: the host's relay by channel. team_of_local is
// current[local slot]+28h.
void host_relay_chat_00772020(const SessionState& session, GameView& game, const ChatLine& line,
                              const void* raw_message, std::int32_t team_of_local,
                              ChatHost& host);
// 007722E0 MT_CHAT (session, msg, &status).
void receive_chat_007722e0(const SessionState& session, GameView& game, const ChatLine& line,
                           const void* raw_message, std::int32_t team_of_local, ChatHost& host);

// Scene selection. A scene record is a node of the list at game+5F4h
// (count game+5F0h) with its name at +90Ch and a second string at +920h;
// game+5FCh is the selected record and game+60Ch its index.
struct SceneHost {
    virtual ~SceneHost() = default;
    // 004E1D70 BSP_Game_BuildSceneRecord(name, 1, msg+28h, 0): true on success.
    virtual bool build_scene_record(const std::string& name, std::uint32_t arg) = 0;
    // 004C6890 BSP_Game_SelectSceneRecord(index).
    virtual void select_scene_record_004c6890(std::uint32_t index) = 0;
    // The selected record's +90Ch name, empty when game+5FCh is null.
    virtual bool selected_scene_name(std::string& out) = 0;
    // 00449AF0 / 00435C40 case-insensitive compares (not equal / equal).
    virtual bool names_equal_ci(const std::string& a, const std::string& b) = 0;
    virtual void request_player_ready(std::int32_t player, std::uint8_t ready,
                                      std::uint8_t aux) = 0;  // 00771DD0
    virtual void leave_tactical_library_if_open() = 0;
    virtual void refresh_lobby_scene_005e5ae0() = 0;           // when [00E198B4]+48h
    virtual void send_to_server_00770af0(const BuiltMessage& message) = 0;
    virtual void set_client_scene_match_c4(std::uint8_t value) = 0;  // [+18Ch]+C4h if +18Ch
};

// 0076E710 MT_SCENENAME (session, msg, &status): builds a scene record from
// the name at msg+18h ("" when null) and counts successes in [00F1AF2C].
void receive_scene_name_0076e710(const std::string& name, std::uint32_t arg_28,
                                 std::uint32_t& built_count_f1af2c, SceneHost& host);
// 007724A0 MT_SCENEIDX (session, msg, &status).
void receive_scene_index_007724a0(GameView& game, const std::string& scene_name,
                                  std::uint32_t index_20, std::uint32_t word_28,
                                  std::uint32_t word_2c, SceneHost& host);

// 00773420 MT_REMOVEPLAYER (session, msg, &status), msg+18h = player.
struct RemovePlayerHost {
    virtual ~RemovePlayerHost() = default;
    virtual bool online_present_00f8a2fc() = 0;
    virtual void online_remove_remote_talker_b0(std::uint64_t xuid) = 0;
    virtual void online_remove_player_15c(std::uint64_t xuid, const std::string& name) = 0;
    virtual void destroy_peer(void* peer) = 0;                 // peer->vtable[0](1)
    virtual void restore_mission_record_004bb660(std::int32_t slot) = 0;
    // 004BB690(name): the current record of the slot whose name matches, or null.
    virtual PlayerRecord* find_slot_record_by_name_004bb690(const std::string& name) = 0;
    virtual void clear_player_record_004d0fc0(PlayerRecord& record) = 0;
};
void receive_remove_player_00773420(GameView& game, VoiceMuteTable& mute, std::int32_t player,
                                    RemovePlayerHost& host);

// 0076D1B0 __MMultiPlayer::ServerSendSceneScoring, ECX = session, RET.
struct ScoringHost {
    virtual ~ScoringHost() = default;
    virtual void trace(const char* text) = 0;
    // 00905370 on the scoring object [game+21A0h] (body is a plain RET in
    // this image: TRIV_body_00905370).
    virtual void scoring_prepare_00905370() = 0;
    // 0090ABB0 on [game+21A0h]+4h+i*284h, i = 0..7.
    virtual void scoring_send_row_0090abb0(int row) = 0;
    // 007848F0 on +188h, or +18Ch when +188h is null.
    virtual void flush_peer_buffers_007848f0(bool server_transport) = 0;
};
inline constexpr std::size_t kScoringRowStride = 0x284;
inline constexpr int kScoringRowCount = 8;  // 1420h / 284h
void server_send_scene_scoring_0076d1b0(const SessionState& session, ScoringHost& host);

// ---------------------------------------------------------------------------
// Batch 3: the MultiScore net table (MT_MNETTABLESYNCENTITY_SYNC, E0h)
// ---------------------------------------------------------------------------

// The MultiScore entity (constructor 00780DB0, primary vtable 00D040B8, the
// per-peer sync slot at vtable 00D04230 +10h) replicates the Lua global
// table MultiScore[p] from the server to the peer whose index p is
// peer+20h. Message E0h: builder 00781F60 (vtable 00D03824), writer 00781590
// (vtable +4h), reader 00781910 (vtable +8h). The class name comes from the
// MT_ name; "MultiScore" and "MultiScore_save" are the Lua globals.

// A list node of the E0h message (writer 00781590 reads +0Ch, +18h, +20h,
// +24h of each node). Key: string when is_string (wire bool 1), else a
// 32-bit number. Value: string when is_string, else a float32.
struct NetTableKey {
    bool is_string{false};
    std::string text;
    std::uint32_t number{0};
};
struct NetTableValue {
    bool is_string{false};
    std::string text;
    float number{0.0f};  // 008869F0 stores (float)int
};
struct NetTableEntry {
    NetTableKey key;
    NetTableValue value;
};

// Wire order (00781590 / 00781910), after the extended message header:
// count in 5 bits (so at most 31 entries survive; the writer does not clamp),
// then per entry: bool key_is_string, key (owned string, or 32 bits), bool
// value_is_string, value (owned string, or numeric float (0, 1, [00D7A248],
// 32 bits)). Bit-level encoding is left to the bit-cursor reconstructions.
inline constexpr std::uint32_t kNetTableCountBits = 5;
inline constexpr std::uint8_t kNetTableSyncMessageType = 0xE0;
inline constexpr std::uint32_t kNetTableSyncMessageVtable = 0x00D03824u;

// 00781910's text: for each entry
//   "MultiScore" "[" p "][" key "]=" value ";"
// with an integer key in decimal, a string key or value quoted with '"'
// (no escaping), and a float value as sprintf "%f" of the float32 (0043BDF0).
// p is current[local slot]+20h on the receiving side. The string replaces
// msg+28h; the entry list at msg+20h is emptied first.
std::string net_table_sync_lua_source_00781910(std::int32_t party,
                                               const NetTableEntry* entries, std::size_t count);

// One Lua value as 00782020 classifies it. Integer means 00B66A60 holds (a
// number whose float32 spill is an exact int32); Number is any other number.
struct NetTableLuaValue {
    enum class Kind : std::uint8_t { kNil, kInteger, kNumber, kString, kOther };
    Kind kind{Kind::kNil};
    double number{0.0};
    std::string text;
    // Lua type code (GetType). kInteger and kNumber share LUA_TNUMBER.
    int lua_type() const noexcept;
};
struct NetTablePair {
    NetTableLuaValue key;
    NetTableLuaValue value;
    // save[key] when MultiScore_save[p] is a table and the key is an
    // integer or string; nil otherwise.
    NetTableLuaValue saved;
};

// 00782020's change rule for one pair. Unchanged when old and new have the
// same Lua type and, for numbers, compare equal (x87, so NaN is always
// changed) or, for strings, are the same interned string; any other
// same-type pair (tables, booleans) is unchanged. A changed pair is sent only
// when the key is an integer or a string and the value is an integer or a
// string: fractional numbers, booleans and tables never replicate.
bool net_table_pair_changed_00782020(const NetTablePair& pair) noexcept;
bool net_table_entry_from_pair_00782020(const NetTablePair& pair, NetTableEntry& out);

struct NetTableSyncHost {
    virtual ~NetTableSyncHost() = default;
    // 00B65EC0: luaL_loadbuffer(text, "DoBuffer") then lua_call.
    virtual void lua_do_buffer_00b65ec0(const std::string& text) = 0;
    // 00779FC0 on the entity: msg+18h = entity+174h, then 00770B50(peer, msg).
    virtual void send_to_peer_00779fc0(void* peer, const NetTableEntry* entries,
                                       std::size_t count) = 0;
};
// 00782020, __thiscall(entity, peer_holder) RET 4: nothing for a null peer
// or p == 8, or when MultiScore or MultiScore[p] is not a table. Otherwise
// the changed pairs in iteration order, then always
// "MultiScore_save[p]=copy_table(MultiScore[p])" through 00B65EC0, then one
// E0h message to the peer when any entry was collected.
void net_table_sync_to_peer_00782020(void* peer, std::int32_t party, bool tables_present,
                                     const NetTablePair* pairs, std::size_t count,
                                     NetTableSyncHost& host);
std::string net_table_snapshot_statement_00782020(std::int32_t party);

// ---------------------------------------------------------------------------
// Batch 4: MT_ADDPLAYER, MT_CONNSTATE, MT_HINT, MT_CHECKSCENELOAD (client)
// ---------------------------------------------------------------------------

// MT_ADDPLAYER payload fields 00773120 reads (offsets from msg+0).
struct AddPlayerMessage {
    std::int32_t player_18{0};
    std::uint64_t xuid_20{0};
    std::string name_28;             // inline char array at +28h
    std::uint32_t word_48_address{0};  // msg+48h passed by address to 004BB550
    std::uint8_t byte_54{0};
    std::uint8_t byte_58{0};
    std::uint32_t word_5c{0};
    std::uint8_t byte_60{0};         // -> record+90h
    std::uint8_t byte_61{0};         // -> record+94h (zero-extended)
    std::array<std::uint8_t, 36> xnaddr_62{};  // XNADDR, copied by REP MOVSD (9 dwords)
    std::uint8_t byte_9e{0};         // second argument of [00F8A2FC]->vtable[158h]
    std::uint32_t words_b0[4]{};
};

// sockaddr_in written to target+D74h..D83h: AF_INET, port htons(1000)
// (00A4D494, imported by ordinal; Ghidra labels it XSocketNTOHS, the same
// byte swap), address from XNetXnAddrToInAddr.
struct TargetSockAddr {
    std::uint16_t family{2};
    std::uint16_t port_network{0};
    std::uint32_t address{0};
    std::uint8_t zero[8]{};
};
inline constexpr std::uint16_t kPeerVoicePort = 1000;  // PUSH 3E8h at 0077325B

struct AddPlayerHost {
    virtual ~AddPlayerHost() = default;
    // 004BB550 __thiscall(game, player, name, &msg+48h, xuid lo, xuid hi,
    // msg+58h, msg+54h, msg+5Ch, msg+B0h..BCh): fills the player record.
    virtual void fill_player_record_004bb550(const AddPlayerMessage& msg) = 0;
    // Player 0: the first node's +8h of the client transport list
    // ([+18Ch]+0Ch), or null when [+18Ch]+10h is 0. No +18Ch null check.
    virtual void* client_first_peer() = 0;
    // operator new(0DC0h); 007839D0(0, 0); vtable 00D03860; then 007827D0
    // with ECX = target+D6Ch. Allocation failure is not handled downstream.
    virtual void* create_peer_target() = 0;
    virtual std::uint16_t htons_00a4d494(std::uint16_t value) = 0;
    // [00F8A2FC]->vtable[198h](), called without a null check.
    virtual bool online_uses_network_addresses_198() = 0;
    // XNetXnAddrToInAddr(&xnaddr, [game+207Ch]+96Ch, &sin_addr); the result
    // code is ignored.
    virtual std::uint32_t xnaddr_to_inaddr(const std::array<std::uint8_t, 36>& xnaddr) = 0;
    virtual void set_target_address_d74(void* target, const TargetSockAddr& address) = 0;
    virtual void set_target_record_d50(void* target, PlayerRecord& record) = 0;
    virtual bool online_present_00f8a2fc() = 0;
    virtual void online_add_player_158(std::uint64_t xuid, const std::string& name,
                                       std::uint8_t flag) = 0;
    virtual void online_add_remote_talker_ac(std::uint64_t xuid) = 0;
    virtual void online_refresh_28() = 0;
    // 007F8690 __thiscall(record, &{game+734h, game+730h, 0}).
    virtual void place_local_record_007f8690(PlayerRecord& record, std::uint32_t game_734,
                                             std::uint32_t game_730) = 0;
};

// 00773120 MT_ADDPLAYER (session, msg, &status), RET 8. The local player
// index is read after the record is filled (007731D1). Player 0's record peer is the
// client transport's first peer; every other player gets a new peer target
// whose +D50h points back at the record (which is why the peer metadata at
// peer+D50h elsewhere is a player record).
void receive_add_player_00773120(SessionState& session, GameView& game,
                                 const AddPlayerMessage& msg, std::uint32_t game_734,
                                 std::uint32_t game_730, AddPlayerHost& host);

// 0076CD20 MT_CONNSTATE (session, msg, &status): for i = 0..7 the triplet
// at session+0Ch+i*1Ch is updated (007862C0) with the float pair at
// msg+18h+i*0Ch; the third float of each 0Ch group is unused.
struct ConnStateRows {
    NativeSmoothedRemainderTriplet* rows[8]{};
};
void receive_conn_state_0076cd20(ConnStateRows& rows, const std::uint32_t (&float_bits)[8][3],
                                 const NativeTripletSmoothingContext& context);

// 00777820 MT_HINT (session, msg), RET 4: outside host mode,
// 00690FD0 on 004E1CA0(msg+18h, 1) (award-tracker hint).
struct HintHost {
    virtual ~HintHost() = default;
    virtual void trigger_hint(const void* hint_payload_18) = 0;
};
void receive_hint_00777820(const SessionState& session, const void* hint_payload_18,
                           HintHost& host);

// 00772710 MT_CHECKSCENELOAD client arm (session, msg, &status): r =
// [00F8A2FC]->vtable[168h](); 005D6CD0(); send 0Dh with +18h = (r == 0)
// to the server; session+4h = 0.
struct CheckSceneLoadHost {
    virtual ~CheckSceneLoadHost() = default;
    virtual std::uint32_t online_scene_check_168() = 0;
    virtual void gui_005d6cd0() = 0;
    virtual void send_to_server_00770af0(const BuiltMessage& message) = 0;
};
void client_check_scene_load_00772710(std::uint8_t& session_04, CheckSceneLoadHost& host);

// ---------------------------------------------------------------------------
// Batch 5: MT_REFRESHSCENE (server) and the scene message builders
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kVtableSceneName08 = 0x00D036D0u;
inline constexpr std::uint32_t kVtableSceneIdx09 = 0x00D036F8u;

// 00770FF0 __thiscall(msg, index, &name, &second), RET 0Ch: MT_SCENENAME,
// strings at +18h and +20h, index at +28h (0076E710 passes +28h on).
struct SceneNameMessage {
    std::uint32_t vtable{kVtableSceneName08};
    std::string name_18;
    std::string second_20;
    std::uint32_t index_28{0};
};
SceneNameMessage build_scene_name_00770ff0(std::uint32_t index, const std::string& name,
                                           const std::string& second);

// 00771190 __thiscall(msg, index, &name, word_28, word_2c), RET 10h:
// MT_SCENEIDX, name at +18h, index at +20h, words at +28h/+2Ch.
struct SceneIndexMessage {
    std::uint32_t vtable{kVtableSceneIdx09};
    std::string name_18;
    std::uint32_t index_20{0};
    std::uint32_t word_28{0};
    std::uint32_t word_2c{0};
};
SceneIndexMessage build_scene_index_00771190(std::uint32_t index, const std::string& name,
                                             std::uint32_t word_28, std::uint32_t word_2c);

// The scene list as 007717A0 walks it: game+5F0h entries of the list at
// game+5F4h (node+4h next, node+8h record), record+90Ch name, +920h second.
struct SceneListView {
    struct Scene {
        std::string name_90c;
        std::string second_920;
    };
    const Scene* scenes{nullptr};
    std::int32_t count_5f0{0};
    const Scene* selected_5fc{nullptr};
    std::uint32_t selected_index_60c{0};
    std::uint32_t word_2060{0};
    std::uint32_t word_2064{0};
};
struct RefreshSceneHost {
    virtual ~RefreshSceneHost() = default;
    // 00770B50 then the inline destructor (BSP_SessionMessage08/09_Destroy).
    virtual void send_scene_name(void* peer, const SceneNameMessage& message) = 0;
    virtual void send_scene_index(void* peer, const SceneIndexMessage& message) = 0;
};
// 007717A0 (session, peer) RET 4, the server arm of MT_REFRESHSCENE (the
// ladder passes [msg+8]): nothing without a game; every scene name to the
// asking peer, then the selected index when a scene is selected. The node
// walk stops at the last node when a next pointer is null (so it clamps
// rather than running off), and only a null head yields a null record; this
// projection assumes count_5f0 matches the list length.
void host_refresh_scene_007717a0(bool game_present, const SceneListView& scenes, void* peer,
                                 RefreshSceneHost& host);

}  // namespace bsp::mp_cc13
