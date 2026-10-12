#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

// XLive online system, voice chat and session plumbing in 00A38D80..00A50000
// (packet cc13-sweepA1). Evidence, per-function ABI and the naming ledger rows:
// docs/CC13_BAND_00A1_00A4.md.
//
// The online system is a 2218h-byte object created by 00992A50, constructed by
// 00A47EE0 (vtable 00D24878) and stored at [00F8A2FC]/[00F8A300]. Its voice
// chat wrapper sits at +1C24h, its XSession wrapper at +2098h, matchmaking at
// +184h and a host QoS probe at +1AE8h (constructor call sites 00A480E7,
// 00A481DE, 00A48035, 00A48045).
//
// Names are hypotheses, except the XHV2 engine method names, which follow the
// vtable order of the XHV2 SDK interface and match every call in this band.
// These are valid-state projections over host interfaces, not native layouts or
// drop-in ABI replacements; only VoiceTalkerRecord is a byte-exact layout.
namespace bsp::xlive_cc13 {

// ---------------------------------------------------------------------------
// Constants read from the image
// ---------------------------------------------------------------------------

// 00A39300: XSocketCreate(AF_INET, SOCK_DGRAM, 0FEh) bound to port 1000 for game
// traffic; a second socket on port 3EBh with FIONBIO and SO_BROADCAST answers
// and sends the system-link discovery probes (00A39900, 00A3D540).
inline constexpr std::uint16_t kGamePort = 1000;
inline constexpr std::uint16_t kLanDiscoveryPort = 0x3EB;
inline constexpr std::int32_t kVdpProtocol = 0xFE;
inline constexpr std::size_t kLanProbeBytes = 9;      // 00A39900 sendto size
inline constexpr std::size_t kLanReplyBytes = 0x14C;  // 00A3D540 reply size

// +60h of the online system (base constructor 00994080 writes 2). Slot +198h
// tests 0, slot +19Ch tests 1, slot +1A0h stores it. Mode 1 gates the LAN
// discovery and skips arbitration, so it is read here as system link.
enum class NetworkMode : std::int32_t {
    Live = 0,        // provisional name
    SystemLink = 1,  // provisional name
    Unset = 2,
};

// 00994100, slot +198h: [ECX+60h] == 0.
bool is_live_mode_00994100(NetworkMode mode);
// 00994110, slot +19Ch: [ECX+60h] == 1.
bool is_system_link_mode_00994110(NetworkMode mode);

// 00A47900, slot +180h: session state +21F0h is 2, 3 or 4.
bool is_session_active_00a47900(std::int32_t session_state_21f0);

// The twelve voice mask names of table 00D24734 (pointers to 00D24724..00D24678),
// read from the installed PE. The constructor 00A47EE0 seeds each slot with
// entry 0.
inline constexpr std::array<std::string_view, 12> kVoiceMaskNames = {
    "voice_mask_none", "voice_mask_anonymous", "player_sun",    "player_jed",
    "player_jenna",    "player_sarama",        "player_angul",  "player_machiko",
    "player_jinx",     "player_amran",         "player_roosta", "player_vorsh",
};

// 00A45010, slot +CCh, __thiscall(system, name, slot), RET 8. With voice active
// (+1D28h), each table entry whose text equals `name` (004BEB60 compare over
// the whole argument) is assigned into the slot record +1D5Ch+slot*1Ch. The
// loop does not stop at a match and the function always returns false; the
// index of the last match (or -1) is returned here instead of the assignment.
struct VoiceMaskLookup {
    bool native_result = false;  // AL, always 0
    std::int32_t matched_index = -1;
};
VoiceMaskLookup set_voice_mask_by_name_00a45010(bool voice_active, std::string_view name);

// 00A44EC0, slot +74h, __thiscall(system, out string), RET 4. The output starts
// as "Unknown"; flag +124h with a non-zero error gives "live_err_update_required",
// otherwise the HRESULT at +1BD8h selects the text. Error 0 erases the string
// (00A44840, std::string erase over the whole range), returned here as "".
std::string_view last_error_text_00a44ec0(bool flag_124, std::uint32_t error_1bd8);

// ---------------------------------------------------------------------------
// Voice chat wrapper (online system +1C24h)
// ---------------------------------------------------------------------------

// The XHV2 engine at voice+78h. Only the methods this band calls are declared;
// the comment gives each one's vtable offset. The native engine is called
// __stdcall with an explicit this, so this interface is a host seam, not an ABI.
struct Xhv2Engine {
    virtual ~Xhv2Engine() = default;
    virtual void release() = 0;                                                  // +04h
    virtual std::int32_t unregister_local_talker(std::uint32_t user_index) = 0;  // +24h
    virtual std::int32_t unregister_remote_talker(std::uint64_t xuid) = 0;       // +2Ch
    virtual std::int32_t set_playback_priority(std::uint64_t xuid, std::uint32_t user_index,
                                               std::int32_t priority) = 0;       // +48h
};

// One remote talker, 38h bytes, in the array at voice+80h (operator new 380h in
// 00A493C0 = 16 records). Per local user slot: a playback priority and two mute
// sources; 00A49110 sends -1 (muted) when either mute word is set.
struct VoiceTalkerRecord {
    std::uint64_t xuid;                         // +00h
    std::array<std::int32_t, 4> priority;       // +08h, 00A49710
    std::array<std::int32_t, 4> mute_list;      // +18h, 00A49780 (provisional source)
    std::array<std::int32_t, 4> game_mute;      // +28h, 00A497E0 (provisional source)
};
static_assert(sizeof(VoiceTalkerRecord) == 0x38);
static_assert(offsetof(VoiceTalkerRecord, priority) == 0x08);
static_assert(offsetof(VoiceTalkerRecord, mute_list) == 0x18);
static_assert(offsetof(VoiceTalkerRecord, game_mute) == 0x28);

inline constexpr std::uint32_t kVoiceMaxTalkers = 16;
inline constexpr std::int32_t kEFail = static_cast<std::int32_t>(0x80004005u);

// Projection of the voice fields this band reads and writes.
struct VoiceState {
    Xhv2Engine* engine = nullptr;                           // +78h
    std::uint32_t talker_count = 0;                         // +7Ch
    std::array<VoiceTalkerRecord, kVoiceMaxTalkers>* talkers = nullptr;  // +80h
    std::array<std::uint32_t, 4> local_registered{};        // +84h (slot 0 used)
    std::array<std::uint32_t, 4> mute_all{};                // +C4h
    std::uint32_t field_104 = 0;                            // +104h
};

// Host seam for _free (00BF9DC8) on the talker array.
struct VoiceHost {
    virtual ~VoiceHost() = default;
    virtual void free_talker_array(std::array<VoiceTalkerRecord, kVoiceMaxTalkers>* talkers) = 0;
};

// 00A49340, __thiscall(voice, xuid lo, xuid hi, out index), RET 0Ch.
std::int32_t find_remote_talker_00a49340(const VoiceState& voice, std::uint64_t xuid,
                                         std::uint32_t* index);
// 00A49110, __thiscall(voice, record*), RET 4. Always returns 0.
std::int32_t apply_talker_priority_00a49110(VoiceState& voice, const VoiceTalkerRecord& record);
// 00A49180, __thiscall(voice, slot, value), RET 8. Always returns 0.
std::int32_t set_mute_all_and_reapply_00a49180(VoiceState& voice, std::uint32_t slot,
                                               std::uint32_t value);
// 00A49710 / 00A49780 / 00A497E0, __thiscall(voice, xuid lo, xuid hi, slot, value), RET 10h.
std::int32_t set_talker_priority_00a49710(VoiceState& voice, std::uint64_t xuid,
                                          std::uint32_t slot, std::int32_t value);
std::int32_t set_talker_mute_list_flag_00a49780(VoiceState& voice, std::uint64_t xuid,
                                                std::uint32_t slot, std::int32_t value);
std::int32_t set_talker_game_mute_flag_00a497e0(VoiceState& voice, std::uint64_t xuid,
                                                std::uint32_t slot, std::int32_t value);
// 00A49680, __thiscall(voice, xuid lo, xuid hi), RET 8.
std::int32_t remove_remote_talker_00a49680(VoiceState& voice, std::uint64_t xuid);
// 00A49070, __fastcall(ECX = voice). Always returns 0.
std::int32_t unregister_all_remote_talkers_00a49070(VoiceState& voice);
// 00A49270, __fastcall(ECX = voice). Always returns 0.
std::int32_t shutdown_00a49270(VoiceState& voice, VoiceHost& host);

// ---------------------------------------------------------------------------
// XSession wrapper (online system +2098h)
// ---------------------------------------------------------------------------

// State word +158h: 0 idle, 1 create pending, 2 created, 3 delete pending,
// 4 created and waiting (00A49A00 moves 4 back to 2). Names are provisional.
struct SessionState {
    std::int32_t state = 0;          // +158h
    std::int32_t restart_flag = 0;   // +160h
    std::int32_t started = 0;        // +C0h
    std::uint8_t ended = 0;          // +C4h
    std::uint8_t keep_session = 0;   // +C5h, skips XSessionDelete in 00A4ADC0
    std::int32_t key_registered = 0; // +15Ch
    std::array<std::uint32_t, 7> overlapped{};  // +110h..+128h
};

inline constexpr std::int32_t kEUnexpected = static_cast<std::int32_t>(0x8000FFFFu);

// The XLive imports and sibling routines these state helpers call.
struct SessionHost {
    virtual ~SessionHost() = default;
    virtual std::int32_t x_session_start() = 0;                 // XSessionStart(+12Ch, 0, 0)
    virtual std::int32_t x_session_end() = 0;                   // XSessionEnd(+12Ch, 0)
    virtual std::int32_t x_session_delete_overlapped() = 0;     // XSessionDelete(+12Ch, +110h)
    virtual void x_cancel_overlapped() = 0;                     // XCancelOverlapped(+110h)
    virtual void trace(const char* text) = 0;                   // 004254B0
    virtual std::int32_t complete_create_00a4b030() = 0;
    virtual std::int32_t stop_qos_listen_00a49c80() = 0;
    virtual std::int32_t complete_delete_00a4af20() = 0;
    virtual void terminate_xnet_00a4a790() = 0;
};

// 00A49A00, __fastcall(ECX = session).
std::int32_t flag_restart_00a49a00(SessionState& session);
// 00A49B00, __fastcall(ECX = session).
std::int32_t start_00a49b00(SessionState& session, SessionHost& host);
// 00A49B30, __fastcall(ECX = session).
std::int32_t end_00a49b30(SessionState& session, SessionHost& host);
// 00A4ADC0, __fastcall(ECX = session).
std::int32_t delete_00a4adc0(SessionState& session, SessionHost& host);
// 00A4B1F0, __fastcall(ECX = session).
std::int32_t poll_overlapped_00a4b1f0(SessionState& session, SessionHost& host);

}  // namespace bsp::xlive_cc13
