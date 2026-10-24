#pragma once
// Band 00760000-0078FFFF, non-gameplay pieces (packet cc13-sweep74,
// docs/CC13_BAND_0074_0078.md): the session peer rows at MMultiPlayer+8h,
// the MNetwork packet tick unwrap, the one-way voice mute query and the
// movie-camera angle windows. New C++ interfaces over the recovered rules;
// none of these is a drop-in binary replacement, and nothing here is wired
// into the game host. Names are hypotheses, not recovered symbols.
#include <cstddef>
#include <cstdint>

#include "bsp/multiplayer_session_cc13.hpp"

namespace bsp::band0076_cc13 {

// ---------------------------------------------------------------------------
// Session peer rows (MMultiPlayer+8h; game+1EF8h)
// ---------------------------------------------------------------------------

// One row per connected peer, eight rows of 1Ch bytes. Row 0 is the first
// peer; 007866B0 keeps each row at the index its peer's player record stores
// at +1Ch. Fields from 007864E0 (add), 00786150 (probes), 007865D0 (quality)
// and 00786390 (MT_CONNSTATE copies +4/+8/+0Ch of every row).
struct PeerRow {
    std::uint32_t peer = 0;        // +00h, peer object (record at peer+D50h)
    float value_04 = 0;            // +04h, sent in MT_CONNSTATE (written as float 0)
    float value_08 = 0;            // +08h, sent in MT_CONNSTATE (written as float 0)
    float quality = 1.0f;          // +0Ch, probe rate and MT_CONNSTATE value
    std::uint8_t probing = 0;      // +10h, MT_CONNTEST probes enabled
    std::uint8_t pad_11[3]{};
    float probe_accum_large = 0;   // +14h, 4Bh-byte probes (message flag 1)
    float probe_accum_small = 0;   // +18h, 8Ch-byte probes (message flag 0)
};
static_assert(sizeof(PeerRow) == 0x1C);

struct PeerRows {
    PeerRow rows[8];               // +00h
    std::uint8_t enabled = 0;      // +E0h, rows are maintained (host)
    std::uint8_t probes = 0;       // +E1h, probe flag copied into new rows
    std::uint8_t pad_e2[2]{};
    float period_timer = 0;        // +E4h, counts down from 0.2 (00D042F8)
    float connstate_accum = 0;     // +E8h, +0.2 per period, MT_CONNSTATE at 0.5
};
static_assert(sizeof(PeerRows) == 0xEC);
static_assert(offsetof(PeerRows, enabled) == 0xE0);
static_assert(offsetof(PeerRows, period_timer) == 0xE4);
inline constexpr std::size_t kSessionPeerRowsOffset = 0x8;

// Image constants.
inline constexpr float kPeerRowInitialQuality = 1.0f;     // 00D7A24C
inline constexpr float kConnStateThreshold = 0.5f;        // 00D042FC
inline constexpr float kConnTestProbeThreshold = 0.09f;   // 00D042F0 (float)
inline constexpr double kConnTestProbeStep = 0.09f;       // 00CF5870 (double of the float)

enum class AddPeerResult { kIgnored, kAlreadyPresent, kAdded, kNoFreeRow };

// 007864E0, __thiscall(rows, peer) RET 4. Ignored for a null peer or when
// rows are disabled; returns early when the peer is present; otherwise the
// first empty row takes the peer. When no row is empty the original writes
// the row at index -1 (the 1Ch bytes before session+8h); this interface
// reports kNoFreeRow and writes nothing.
AddPeerResult peer_rows_add_007864e0(PeerRows& rows, std::uint32_t peer) noexcept;

// 00786BE0, __thiscall(rows, peer) RET 4. Returns true when the peer was
// found. Removing row 0 resets every row through 007868C0 (reconstructed as
// reset_native_game_peer_rows_007868c0; passed in here as a callback),
// another row only loses its peer. Both paths then zero the period timer.
using ResetPeerRowsFn = void (*)(PeerRows& rows, std::uint8_t enabled, void* context);
bool peer_rows_remove_00786be0(PeerRows& rows, std::uint32_t peer,
                               ResetPeerRowsFn reset, void* context) noexcept;

// 00785F90 / 00785FC0, ECX = rows, RET: set or clear +E1h and the probe byte
// of rows 1..7 (row 0 is left alone), only when +E1h changes.
void peer_rows_enable_probes_00785f90(PeerRows& rows) noexcept;
void peer_rows_disable_probes_00785fc0(PeerRows& rows) noexcept;

// 00786010, __thiscall(rows, index) RET 4: rows[index].quality. No bound check.
inline float peer_rows_quality_00786010(const PeerRows& rows, int index) noexcept {
    return rows.rows[index].quality;
}

// 00786150's accumulator rule for one stream: accum += (float)(quality * dt);
// while accum >= 0.09f: accum = (float)(accum - 0.09) and one MT_CONNTEST
// is sent. Returns the number of probes. A NaN accumulator sends none.
int conn_test_probe_step_00786150(float& accum, float quality, float dt) noexcept;

// The 1Ch-byte MT_CONNTEST message 00786150 builds on its stack (stream 1 at
// [ESP+14h], stream 2 at [ESP+30h]): 0075B430(0Fh) fills +00h..+17h, then
// +00h = 00D02FBC (vtable), +04h = the stream flag, word +18h = the size.
struct ConnTestMessage {
    std::uint8_t bytes[0x1C]{};
};
inline constexpr std::uint32_t kConnTestMessageType = 0x0F;        // 007861D3 PUSH 0Fh
inline constexpr std::uint32_t kConnTestMessageVtable = 0x00D02FBC; // 007861A3 MOV EBX
inline constexpr std::uint16_t kConnTestLargeSize = 0x4B;          // 007861EF
inline constexpr std::uint16_t kConnTestSmallSize = 0x8C;          // 00786273

struct ConnTestProbeHost {
    virtual ~ConnTestProbeHost() = default;
    // [row.peer + D50h] != 0 (0078617F), the peer's record.
    virtual bool peer_has_record(std::uint32_t peer) = 0;
    // 0075B430, __thiscall(ECX = message; type), RET 4.
    virtual void session_message_construct_base_0075b430(ConnTestMessage& message,
                                                         std::uint32_t type) = 0;
    // 00783DC0, __thiscall(ECX = [[00E188A8] + 2078h]; peer, &message), RET 8.
    // The owner is re-read before every send (007861F6, 0078627A).
    virtual void session_send_00783dc0(std::uint32_t peer, ConnTestMessage& message) = 0;
};

// 00786150 whole, __thiscall(ECX = row; float dt) RET 4, body
// 00786150..007862BB. Gates on +10h, +00h and the peer record; product =
// (float)(+0Ch * dt) (0078618C..007861AB) is shared by both streams; each
// stream runs conn_test_probe_step_00786150's rule and sends one message per
// subtraction, the subtraction stored before the message is built. The
// message is never destroyed on the normal path (EH state 0 around the send,
// then -1 at 00786222/007862A3, no destructor call). Returns the probes sent.
int conn_test_send_probes_00786150(PeerRow& row, float dt, ConnTestProbeHost& host);

// ---------------------------------------------------------------------------
// MNetwork packet header: tick unwrap (007858C0)
// ---------------------------------------------------------------------------

// 00785A6F..00785AA0: a packet carries the low 16 bits of the sender's tick.
// The receiver takes d = wire - (local & FFFFh), folds d into
// [-8000h, 8000h] (signed compares, d > 8000h -> d - 10000h, d < -8000h ->
// d + 10000h) and stores local + d (modulo 2^32) as the message tick (+0Ch).
std::uint32_t unwrap_packet_tick_007858c0(std::uint16_t wire_tick,
                                          std::uint32_t local_tick) noexcept;

// ---------------------------------------------------------------------------
// Voice mute: one direction (00778740)
// ---------------------------------------------------------------------------

// 00778740, __thiscall(table, a_lo, a_hi, b_lo, b_hi) RET 10h: both xuids
// found, distinct, and muted[a][b]. 007786E0 tests either direction.
bool voice_mute_is_muted_one_way_00778740(const mp_cc13::VoiceMuteTable& table,
                                          std::uint64_t a, std::uint64_t b) noexcept;

// ---------------------------------------------------------------------------
// Movie camera angle windows (0078E2B0 / 0078FEB0)
// ---------------------------------------------------------------------------

// Two angle windows {min, max}; a window with min > max wraps through the
// seam. Both functions take ECX = the four floats.
struct AngleWindows {
    float min0, max0;   // +00h/+04h, first angle
    float min1, max1;   // +08h/+0Ch, second angle
};

// 0078E2B0, __thiscall(windows, a, b) RET 8. Listing-checked: an unordered
// compare against a window bound counts as inside, so NaN bounds or angles
// pass.
bool movcam_angle_windows_contain_0078e2b0(const AngleWindows& windows,
                                           float a, float b) noexcept;

// 0078FEB0, __thiscall(windows, float* a, float* b) RET 8. An angle outside
// its window moves to the nearer edge: |SubtractWrappedAngle(x, max)| <=
// |SubtractWrappedAngle(x, min)| (both rounded to float) picks max, else min.
// Inside/outside uses the same rule as 0078E2B0; an unordered distance
// compare picks max. SubtractWrappedAngle is 00438B10
// (wrapped_angle_subtract_00438b10).
void movcam_angle_windows_clamp_0078feb0(const AngleWindows& windows,
                                         float& a, float& b) noexcept;

} // namespace bsp::band0076_cc13
