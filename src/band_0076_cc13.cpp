// Band 00760000-0078FFFF non-gameplay reconstructions (cc13-sweep74).
// Evidence, ABI and uncertainty per function in include/bsp/band_0076_cc13.hpp
// and docs/CC13_BAND_0074_0078.md. Build-tested only.
#include "bsp/band_0076_cc13.hpp"

#include <cmath>
#include <cstring>

#include "bsp/unit_rudder.hpp"

namespace bsp::band0076_cc13 {

// 007864E0. Listing: 007864FC..00786520 scans all eight rows, keeping the
// first empty index (ESI from -1) and returning at 0078658B on a match;
// 00786522..00786588 fill the row and copy +E1h when +E0h is clear or the
// index is above 0 (+E0h is always set on this path).
AddPeerResult peer_rows_add_007864e0(PeerRows& rows, std::uint32_t peer) noexcept {
    if (peer == 0 || rows.enabled == 0) return AddPeerResult::kIgnored;
    int free_index = -1;
    for (int i = 0; i < 8; ++i) {
        if (rows.rows[i].peer == peer) return AddPeerResult::kAlreadyPresent;
        if (free_index < 0 && rows.rows[i].peer == 0) free_index = i;
    }
    // The original writes rows[-1] here; see the header.
    if (free_index < 0) return AddPeerResult::kNoFreeRow;
    PeerRow& row = rows.rows[free_index];
    row.peer = peer;
    row.probing = 0;
    row.probe_accum_small = 0.0f;
    row.probe_accum_large = 0.0f;
    row.value_04 = 0.0f;
    row.value_08 = 0.0f;
    row.quality = kPeerRowInitialQuality;
    rows.period_timer = 0.0f;
    rows.connstate_accum = kConnStateThreshold;
    if (rows.probes != 0 && (rows.enabled == 0 || free_index > 0)) row.probing = rows.probes;
    return AddPeerResult::kAdded;
}

// 00786BE0. Listing: the reset call at 00786C11 receives the +E0h byte and
// ECX survives it (00786C2B writes +E4h through ECX on both paths).
bool peer_rows_remove_00786be0(PeerRows& rows, std::uint32_t peer,
                               ResetPeerRowsFn reset, void* context) noexcept {
    if (peer == 0 || rows.enabled == 0) return false;
    int index = 0;
    while (rows.rows[index].peer != peer) {
        if (++index >= 8) return false;
    }
    if (index == 0) {
        reset(rows, rows.enabled, context);
    } else {
        rows.rows[index].peer = 0;
    }
    rows.period_timer = 0.0f;
    return true;
}

void peer_rows_enable_probes_00785f90(PeerRows& rows) noexcept {
    if (rows.probes != 0) return;
    rows.probes = 1;
    for (int i = 1; i < 8; ++i) rows.rows[i].probing = 1;
}

void peer_rows_disable_probes_00785fc0(PeerRows& rows) noexcept {
    if (rows.probes == 0) return;
    rows.probes = 0;
    for (int i = 1; i < 8; ++i) rows.rows[i].probing = 0;
}

// 0078618C..00786226: the product and the sum are each stored to float;
// the loop entry test is FCOMPI/JB (unordered skips) and the repeat test is
// COMISS/JAE (unordered exits); the step is FSUB QWORD [00CF5870].
namespace {
template <class Send>
int conn_test_probe_stream(float& accum, float product, Send&& send) {
    accum = accum + product;
    int sent = 0;
    if (!(accum >= kConnTestProbeThreshold)) return 0;
    do {
        accum = static_cast<float>(static_cast<double>(accum) - kConnTestProbeStep);
        send();
        ++sent;
    } while (accum >= kConnTestProbeThreshold);
    return sent;
}

void put_u32(ConnTestMessage& message, std::size_t offset, std::uint32_t value) noexcept {
    std::memcpy(message.bytes + offset, &value, sizeof(value));
}
} // namespace

int conn_test_probe_step_00786150(float& accum, float quality, float dt) noexcept {
    const float product = quality * dt;
    return conn_test_probe_stream(accum, product, [] {});
}

int conn_test_send_probes_00786150(PeerRow& row, float dt, ConnTestProbeHost& host) {
    // 0078616B..00786186: probing byte, peer, peer record.
    if (row.probing == 0 || row.peer == 0 || !host.peer_has_record(row.peer)) return 0;
    const float product = row.quality * dt; // 0078618C..007861AB, stored as float
    auto sender = [&](std::uint32_t flag, std::uint16_t size) {
        return [&row, &host, flag, size] {
            ConnTestMessage message;
            host.session_message_construct_base_0075b430(message, kConnTestMessageType);
            put_u32(message, 0x00, kConnTestMessageVtable);
            put_u32(message, 0x04, flag);
            std::memcpy(message.bytes + 0x18, &size, sizeof(size));
            host.session_send_00783dc0(row.peer, message); // peer re-read per send
        };
    };
    // 007861AF..00786226: +14h, flag 1, 4Bh. 00786228..007862A7: +18h, flag 0, 8Ch.
    int sent = conn_test_probe_stream(row.probe_accum_large, product,
                                      sender(1, kConnTestLargeSize));
    sent += conn_test_probe_stream(row.probe_accum_small, product,
                                   sender(0, kConnTestSmallSize));
    return sent;
}

std::uint32_t unwrap_packet_tick_007858c0(std::uint16_t wire_tick,
                                          std::uint32_t local_tick) noexcept {
    std::int32_t delta = static_cast<std::int32_t>(wire_tick) -
                         static_cast<std::int32_t>(local_tick & 0xFFFFu);
    if (delta > 0x8000) {
        delta -= 0x10000;
    } else if (delta < -0x8000) {
        delta += 0x10000;
    }
    return local_tick + static_cast<std::uint32_t>(delta);
}

bool voice_mute_is_muted_one_way_00778740(const mp_cc13::VoiceMuteTable& table,
                                          std::uint64_t a, std::uint64_t b) noexcept {
    const int row = mp_cc13::voice_mute_find_007785d0(table, a);
    const int column = mp_cc13::voice_mute_find_007785d0(table, b);
    if (row < 0 || column < 0 || row == column) return false;
    return table.muted[row][column] != 0;
}

namespace {

// 0078E2B0 window test, decoded from the listing (0078E2BF..0078E32C).
// Ordered window (max >= min): outside when min > x or x > max. Otherwise,
// when min > max (wrapped): outside when min > x and x > max. Every other
// case, including unordered compares, is inside.
bool angle_in_window(float min, float max, float x) noexcept {
    if (max >= min) {
        return !(min > x) && !(x > max);
    }
    if (!(min > max)) return true;
    if (!(min > x)) return true;
    return !(x > max);
}

// 0078FF9E..00790004: distances rounded to float, absolute value by masking
// the sign bit, FCOMPI/JBE picks max on <= or unordered.
float snap_to_nearer_edge(float min, float max, float x) noexcept {
    const float to_min = std::fabs(wrapped_angle_subtract_00438b10(x, min));
    const float to_max = std::fabs(wrapped_angle_subtract_00438b10(x, max));
    if (!(to_max > to_min)) return max;
    return min;
}

} // namespace

bool movcam_angle_windows_contain_0078e2b0(const AngleWindows& windows,
                                           float a, float b) noexcept {
    return angle_in_window(windows.min0, windows.max0, a) &&
           angle_in_window(windows.min1, windows.max1, b);
}

void movcam_angle_windows_clamp_0078feb0(const AngleWindows& windows,
                                         float& a, float& b) noexcept {
    if (!angle_in_window(windows.min0, windows.max0, a)) {
        a = snap_to_nearer_edge(windows.min0, windows.max0, a);
    }
    if (!angle_in_window(windows.min1, windows.max1, b)) {
        b = snap_to_nearer_edge(windows.min1, windows.max1, b);
    }
}

} // namespace bsp::band0076_cc13
