// XLive online system reconstructions, packet cc13-gui2 (docs/CC13_GUI_2.md, Part B).
#include "bsp/online_cc13.hpp"
#include "bsp/engine_seg11_35_66_cc13.hpp"
#include "bsp/online_seg71_cc13.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <string>
#include <initializer_list>

namespace bsp::online_cc13 {
namespace {

template <class T>
T read_field(const void* object, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}

template <class T>
void write_field(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}

const void* at(const void* object, std::size_t offset) noexcept {
    return static_cast<const std::byte*>(object) + offset;
}

} // namespace

bool is_online_started_00a47890(const void* s) noexcept { return read_field<std::uint8_t>(s, 0xEC) != 0; }
bool flag_124_00a478a0(const void* s) noexcept { return read_field<std::uint8_t>(s, 0x124) != 0; }
bool has_last_error_00a478b0(const void* s) noexcept { return read_field<std::uint8_t>(s, 0x1BD5) != 0; }
bool flag_1bdc_00a478c0(const void* s) noexcept { return read_field<std::uint8_t>(s, 0x1BDC) != 0; }
const void* block_128_00a478d0(const void* s) noexcept { return at(s, 0x128); }
bool is_search_active_00a478e0(const void* s) noexcept { return read_field<std::uint8_t>(s, 0xED) != 0; }
bool is_voice_active_00a47920(const void* s) noexcept { return read_field<std::uint32_t>(s, 0x1D28) != 0; }
float float_1d40_00a479f0(const void* s) noexcept { return read_field<float>(s, 0x1D40); }
const void* block_8c_00a47a40(const void* s) noexcept { return at(s, 0x8C); }

bool return_true_00a478f0() noexcept { return true; }
bool return_true_00a479d0() noexcept { return true; }
bool return_true_00a47a00() noexcept { return true; }
bool return_true_00a47a10() noexcept { return true; }
bool return_true_00a47a20() noexcept { return true; }
bool return_true_00a47a50() noexcept { return true; }
bool return_false_00a47970() noexcept { return false; }
bool return_false_00a47980() noexcept { return false; }
bool return_false_00a479a0() noexcept { return false; }
bool return_false_00a479b0() noexcept { return false; }
bool return_true_00a48f00() noexcept { return true; }
std::uint32_t return_zero_00a48ea0() noexcept { return 0; }
float return_zero_float_00a47990() noexcept { return 0.0f; }
void nop_00a47930() noexcept {}
void nop_00a47940() noexcept {}
void nop_00a47950() noexcept {}
void nop_00a47960() noexcept {}
void nop_00a479c0() noexcept {}
void nop_00a479e0() noexcept {}
void nop_00a47a30() noexcept {}

bool clear_byte4_00a48e90(void* object) noexcept {
    write_field<std::uint8_t>(object, 0x4, 0);
    return return_true_00a48f00();
}

bool has_players_00a47a60(void* system, XLiveSystemHost& host) {
    return read_field<std::uint32_t>(host.player_list_80(system), 0x8) != 0;
}

bool has_players_00a47a80(void* system, XLiveSystemHost& host) {
    return has_players_00a47a60(system, host);
}

bool shutdown_00a46ee0(void* system, XLiveSystemHost& host) {
    if (read_field<std::uint8_t>(system, 0xEC) == 0) {
        write_field<std::uint32_t>(system, 0x2210, 0);
        return false;
    }
    host.leave_session_00a46d60(system);
    const std::int32_t hr = host.xonline_cleanup_00a4d5ea();
    write_field<std::uint32_t>(system, 0x2210, 0);
    if (hr < 0) return false;
    write_field<std::uint8_t>(system, 0xEC, 0);
    return true;
}

bool leave_session_if_players_listed_00a46f30(void* system, XLiveSystemHost& host) {
    if (host.virtual_40(system) || host.virtual_44(system)) host.leave_session_00a46d60(system);
    return true;
}

void* voice_construct_00a48f10(void* v) noexcept {
    for (std::size_t offset : {0x78u, 0x7Cu, 0x80u, 0x104u, 0x70u, 0x74u, 0x10Cu})
        write_field<std::uint32_t>(v, offset, 0);
    write_field<std::int32_t>(v, 0x114, -100);
    for (std::size_t offset : {0x118u, 0x84u, 0xC4u, 0x108u})
        write_field<std::uint32_t>(v, offset, 0);
    return v;
}

void* session_construct_00a499d0(void* s) noexcept {
    for (std::size_t offset : {0x158u, 0x160u, 0x130u, 0x15Cu, 0xC0u})
        write_field<std::uint32_t>(s, offset, 0);
    return s;
}

void* matchmaking_construct_00a49cf0(void* m) noexcept {
    write_field<std::uint32_t>(m, 0x1808, 0);
    write_field<std::uint32_t>(m, 0x193C, 0);
    write_field<std::uint32_t>(m, 0x1940, 1);
    write_field<std::uint32_t>(m, 0x1938, 0);
    write_field<std::uint32_t>(m, 0x1960, 0);
    return m;
}

void* host_qos_construct_00a4a380(void* q) noexcept {
    write_field<std::uint32_t>(q, 0x78, 0);
    write_field<std::uint32_t>(q, 0x8C, 0);
    write_field<std::uint32_t>(q, 0x90, 1);
    write_field<std::uint32_t>(q, 0x88, 0);
    return q;
}

// 00A47EE0: normal constructor order and stores from 00A47F02..00A48310.
// Compiler-only SEH is represented by an explicit completed-member state;
// the host implements its native cleanup graph, never this code's ABI.
void* system_construct_00a47ee0(void* system, XLiveSystemConstructCalls& host,
    online_seg71_cc13::XLiveVoiceCalls& voice_calls,
    online_seg71_cc13::XenonCalls& xenon_calls) {
    auto* const base = static_cast<std::byte*>(system);
    std::int32_t state = -1;
    try {
        engine_seg11_35_66_cc13::construct_xlive_stats_block_00994080(
            *static_cast<engine_seg11_35_66_cc13::XLiveStatsBlockLayout*>(system));
        state = 0;
        write_field<std::uint32_t>(system, 0, 0x00D24878u);
        write_field<std::uint8_t>(system, 0x85, 0);
        // Original image data: D7A260 = BF800000 (-1.0f).
        write_field<std::uint32_t>(system, 0x88, 0xBF800000u);
        write_field<std::uint32_t>(system, 0x8C, 0);
        write_field<std::uint32_t>(system, 0x90, 0);
        write_field(system, 0xE0, host.participant_head_004c3180(base + 0xDC));
        write_field<std::uint32_t>(system, 0xE4, 0);
        state = 1;
        for (std::size_t offset : {0xECu, 0xEDu, 0xEEu, 0xEFu, 0xF0u})
            write_field<std::uint8_t>(system, offset, 0);
        for (std::size_t offset : {0xF4u, 0x100u, 0x10Cu, 0x118u}) {
            player_list_construct_00a47b50(base + offset, host);
            ++state;
        }
        write_field<std::uint8_t>(system, 0x124, 0);
        write_field<std::uint32_t>(system, 0x140, 0xF);
        write_field<std::uint32_t>(system, 0x13C, 0);
        write_field<std::uint8_t>(system, 0x12C, 0);
        write_field<std::uint32_t>(system, 0x154, 0);
        for (std::size_t offset : {0x158u, 0x160u, 0x161u, 0x163u})
            write_field<std::uint8_t>(system, offset, 0);
        write_field<std::uint8_t>(system, 0x162, 1);
        state = 6;
        host.construct_mask_list_00a46b80(base + 0x168, 0);
        for (std::size_t offset : {0x178u, 0x17Cu, 0x180u})
            write_field<std::uint32_t>(system, offset, 0);
        state = 8;
        matchmaking_construct_00a49cf0(base + 0x184);
        state = 9;
        host_qos_construct_00a4a380(base + 0x1AE8);
        online_seg71_cc13::task_stub_pair_construct_00a43070(base + 0x1B9C);
        state = 0xC;
        for (std::size_t group : {0x1BA4u, 0x1BB4u, 0x1BC4u}) {
            for (std::size_t i = 0; i != 4; ++i)
                online_seg71_cc13::task_stub_construct_00a42510(base + group + 4 * i);
            ++state;
        }
        write_field<std::uint8_t>(system, 0x1BD5, 0);
        write_field<std::uint32_t>(system, 0x1BD8, 0);
        write_field<std::uint8_t>(system, 0x1BDC, 0);
        for (std::size_t offset : {0x1BECu, 0x1BF0u, 0x1BF4u})
            write_field<std::uint32_t>(system, offset, 0);
        state = 0x10;
        voice_construct_00a48f10(base + 0x1C24);
        // CE74F8 = 3F4CCCCD (0.8f); preserve the original bit pattern.
        write_field<std::uint32_t>(system, 0x1D40, 0x3F4CCCCDu);
        write_field<std::uint32_t>(system, 0x1D44, 0);
        write_field<std::uint32_t>(system, 0x1D48, 0);
        state = 0x11;
        host.construct_array_00bf7cd1(base + 0x1D5C, 0x1C, 16, 0x004C53C0, 0x004072D0);
        for (std::size_t offset : {0x1F20u, 0x1F24u, 0x1F28u, 0x1F34u})
            write_field<std::uint32_t>(system, offset, 0);
        // CE3800 / CE4E0C / CE3958 = 0.5f / 0.99f / 2.0f.
        write_field<std::uint32_t>(system, 0x1F2C, 0x3F000000u);
        write_field<std::uint32_t>(system, 0x1F30, 0x3F7D70A4u);
        write_field<std::uint32_t>(system, 0x1F38, 0x40000000u);
        state = 0x13;
        host.construct_array_00bf7cd1(base + 0x1F3C, 0xC, 16, 0x00A446A0, 0x004C8BF0);
        state = 0x14;
        // Four 18h-stride records: only the first two vptr words change.
        for (std::size_t i = 0; i != 4; ++i)
            online_seg71_cc13::task_stub_pair_construct_00a43070(base + 0x1FFC + 0x18 * i);
        state = 0x15;
        host.construct_array_00bf7cd1(base + 0x2064, 0xC, 4, 0x00A446E0, 0x00A44340);
        state = 0x16;
        session_construct_00a499d0(base + 0x2098);
        state = 0x17;
        write_field(system, 0x2204, host.stats_head_004c1f50(base + 0x2200));
        write_field<std::uint32_t>(system, 0x2208, 0);
        state = 0x18;
        write_field<std::uint8_t>(system, 0x220C, 0);
        write_field<std::uint8_t>(system, 0x220E, 0);
        write_field<std::uint32_t>(system, 0x2210, 0);
        online_seg71_cc13::clear_friends_list_00a43fa0(system, voice_calls);
        online_seg71_cc13::communication_privilege_00a3eb30(
            host.xenon_system_manager_00f8abe8(), reinterpret_cast<std::uint8_t*>(base + 0x84), xenon_calls);
        // These deliberately unaligned dword stores cover exactly 20h bytes.
        std::memset(base + 0x1C02, 0, 0x20);
        for (std::size_t offset : {0x1D4Cu, 0x1D50u, 0x1D54u, 0x1D58u})
            write_field<std::uint32_t>(system, offset, 0);
        // Image pointer table D24734..D24760, in the original order.
        static constexpr const char* masks[] = {
            "voice_mask_none", "voice_mask_anonymous", "player_sun", "player_jed",
            "player_jenna", "player_sarama", "player_angul", "player_machiko",
            "player_jinx", "player_amran", "player_roosta", "player_vorsh"
        };
        for (const char* name : masks) {
            NativeLegacySboStringStorage temporary;
            temporary.capacity_18 = 0xF;
            temporary.length_14 = 0;
            temporary.buffer_04.inline_bytes[0] = '\0';
            native_legacy_sbo_string_assign_counted_00408720(temporary, name,
                static_cast<std::uint32_t>(std::strlen(name)));
            try {
                host.push_mask_name_00849170(base + 0x1F1C, temporary);
            } catch (...) {
                native_legacy_sbo_string_destroy_004072d0(temporary);
                throw;
            }
            native_legacy_sbo_string_destroy_004072d0(temporary);
        }
        for (std::size_t i = 0; i != 4; ++i)
            native_legacy_sbo_string_assign_counted_00408720(
                *reinterpret_cast<NativeLegacySboStringStorage*>(base + 0x1D5C + 0x1C * i),
                "voice_mask_none", 0xF);
        std::memset(base + 0x9C, 0, 0x40);
        return system;
    } catch (...) {
        host.unwind_failed_construction_00cb49f6(system, state);
        throw;
    }
}

void host_qos_release_lookup_00a4a3a0(void* q, XLiveQosHost& host) {
    if (void* const lookup = read_field<void*>(q, 0x88)) {
        host.xnet_qos_release_00a4d45e(lookup);
        write_field<void*>(q, 0x88, nullptr);
    }
    write_field<std::uint32_t>(q, 0x78, 0); // the JE at 00A4A3AB lands on this store
}

std::int32_t host_qos_finish_if_complete_00a4a3d0(void* q, XLiveQosHost& host) {
    if (read_field<std::uint32_t>(q, 0x8C) == 4) {
        host_qos_release_lookup_00a4a3a0(q, host);
        write_field<std::uint32_t>(q, 0x8C, 0);
    }
    return read_field<std::uint32_t>(q, 0x8C) != 0 ? static_cast<std::int32_t>(0x8000FFFFu) : 0;
}

void heap_pointer_free_and_clear_00a4a750(void** slot, XLiveCrtHost& host) {
    if (*slot != nullptr) {
        host.free_00bf65ac(*slot);
        *slot = nullptr;
    }
}

void pooled_string_release_storage_00a4b470(void* string, XLiveCrtHost& host) {
    if (void* const buffer = read_field<void*>(string, 0x4))
        host.release_string_buffer(buffer, read_field<std::uint32_t>(string, 0x0) + 1);
}

void voice_destroy_00a493a0(void* voice, XLiveVoiceDestroyHost& voice_host, XLiveCrtHost& host) {
    voice_host.shutdown_00a49270(voice);
    host.close_handle(read_field<void*>(voice, 0x10C));
}

std::uint32_t voice_get_local_chat_data_00a49000(void* voice, void* data, std::uint32_t* packets,
    Xhv2EngineCalls& xhv) {
    *packets = 0;
    std::uint32_t bytes = 0xC8;
    const std::int32_t hr = xhv.get_local_chat_data_44(read_field<void*>(voice, 0x78),
        read_field<std::uint32_t>(voice, 0x118), data, &bytes, packets);
    return hr != 0 ? 0 : bytes;
}

std::uint32_t voice_submit_incoming_chat_data_00a49040(void* voice, const void* data, std::uint16_t bytes,
    const std::uint64_t* talker, Xhv2EngineCalls& xhv) {
    std::uint32_t size = bytes;
    xhv.submit_incoming_chat_data_4c(read_field<void*>(voice, 0x78), *talker, data, &size);
    return 0;
}

std::uint32_t voice_is_local_talking_00a49220(void* voice, Xhv2EngineCalls& xhv) {
    return xhv.is_local_talking_38(read_field<void*>(voice, 0x78), read_field<std::uint32_t>(voice, 0x118));
}

void voice_register_active_local_talker_00a492d0(void* voice, Xhv2EngineCalls& xhv, XenonSystemCalls& xenon) {
    static constexpr std::uint32_t kVoiceChatMode[1] = {2}; // 00D24A34
    static constexpr std::uint32_t kLoopbackMode[1] = {1};  // 00D24A30
    write_field<std::uint32_t>(voice, 0x114, read_field<std::uint32_t>(voice, 0x118));
    write_field<std::uint32_t>(voice, 0x118, xenon.active_user_slot());
    void* const engine = read_field<void*>(voice, 0x78);
    if (read_field<std::uint32_t>(voice, 0x108) == 0) xhv.register_local_talker_20(engine, 0);
    xhv.start_local_processing_0c(read_field<void*>(voice, 0x78), 0, kVoiceChatMode, 1);
    xhv.stop_local_processing_10(read_field<void*>(voice, 0x78), 0, kLoopbackMode, 1);
    write_field<std::uint32_t>(voice, 0x108, 1);
    write_field<std::uint16_t>(voice, 0x6C, 0);
}

std::uint32_t voice_update_00a49530(void* voice, Xhv2EngineCalls& xhv, XenonSystemCalls& xenon,
    VoiceUpdateCalls& host) {
    if (read_field<std::uint32_t>(voice, 0x74) == 0x14) voice_register_active_local_talker_00a492d0(voice, xhv, xenon);
    write_field<std::uint32_t>(voice, 0x74, read_field<std::uint32_t>(voice, 0x74) + 1);
    std::uint32_t id = 0, param = 0;
    if (host.xnotify_get_next(read_field<void*>(voice, 0x10C), id, param) && id == 0x11) {
        host.apply_mute_list_00a48f60(voice);
        host.system_virtual_28();
    }
    if (xenon.slot_state_8c(0) == 0) {
        write_field<std::uint32_t>(voice, 0x4, 0);
        write_field<std::uint32_t>(voice, 0x0, 0);
        return 1;
    }
    const std::uint32_t flags = xhv.is_headset_present_34(read_field<void*>(voice, 0x78), 0);
    if (flags != read_field<std::uint32_t>(voice, 0x4)) {
        write_field<std::uint32_t>(voice, 0x4, flags);
        write_field<std::uint32_t>(voice, 0x0, 1);
    }
    return 1;
}

std::int32_t session_arbitration_register_00a49a50(void* s, XSessionCalls& x) {
    for (std::size_t offset = 0x110; offset <= 0x128; offset += 4) write_field<std::uint32_t>(s, offset, 0);
    void* const handle = read_field<void*>(s, 0x12C);
    const auto nonce = read_field<std::uint64_t>(s, 0x108);
    std::uint32_t bytes = 0;
    x.arbitration_register(handle, 0, nonce, &bytes, nullptr);
    void* const results = x.operator_new_00bf55be(bytes);
    std::int32_t hr = 0;
    if (x.arbitration_register(read_field<void*>(s, 0x12C), 0, read_field<std::uint64_t>(s, 0x108), &bytes, results) != 0)
        hr = x.overlapped_extended_error();
    x.free_00bf6989(results);
    return hr;
}

std::int32_t session_join_local_player_00a4ae70(void* s, XSessionCalls& x, XenonSystemCalls& xenon) {
    // 004254B0 (a RET-only trace stub) receives "  --  ONLINE  --  Server Join Local Player".
    const std::uint32_t user = xenon.active_user_slot();
    const std::uint32_t private_slot = 0;
    if (x.join_local(read_field<void*>(s, 0x12C), 1, &user, &private_slot) != 0) return x.overlapped_extended_error();
    return 0;
}

std::int32_t session_leave_local_player_00a4aed0(void* s, XSessionCalls& x, XenonSystemCalls& xenon) {
    const std::uint32_t user = xenon.active_user_slot();
    if (x.leave_local(read_field<void*>(s, 0x12C), 1, &user) != 0) return x.overlapped_extended_error();
    return 0;
}

std::int32_t session_join_remote_player_00a49b90(void* s, const std::uint64_t* xuid, bool invited,
    XSessionCalls& x) {
    const std::uint32_t private_slot = invited ? 1u : 0u;
    if (x.join_remote(read_field<void*>(s, 0x12C), 1, xuid, &private_slot) != 0) return x.overlapped_extended_error();
    return 0;
}

std::int32_t session_remove_remote_player_00a4b180(void* s, const std::uint64_t* xuid,
    XSessionCalls& x, XenonSystemCalls& xenon) {
    if (*xuid == xenon.active_xuid()) return session_leave_local_player_00a4aed0(s, x, xenon);
    if (x.leave_remote(read_field<void*>(s, 0x12C), 1, xuid) != 0) return x.overlapped_extended_error();
    return 0;
}

std::int32_t session_start_qos_listen_00a49c10(void* s, const void* data, std::uint32_t bytes,
    std::uint32_t bits_per_second, XSessionCalls& x) {
    std::uint32_t flags = bits_per_second != 0 ? 0xCu : 0x4u;
    if (read_field<std::uint32_t>(s, 0x130) == 0) flags |= 1;
    const std::int32_t error = x.qos_listen(at(s, 0xC7), data, bytes, bits_per_second, flags);
    if (error != 0) return error; // 004254B0 gets "Failed to start QoS listener, error 0x%08x\n"
    write_field<std::uint32_t>(s, 0x130, 1);
    return 0;
}

namespace {
void remove_record_at(void* owner, std::size_t count_offset, std::size_t stride, std::uint32_t index) noexcept {
    const auto count = read_field<std::uint32_t>(owner, count_offset);
    if (index >= count) return;
    write_field<std::uint32_t>(owner, count_offset, count - 1);
    auto* const record = static_cast<std::byte*>(owner) + index * stride;
    std::memmove(record, record + stride, (count - 1 - index) * stride);
}
} // namespace

void matchmaking_remove_result_at_00a49920(void* m, std::uint32_t index) noexcept {
    remove_record_at(m, 0x1808, 0xF6, index);
}

void host_qos_remove_entry_at_00a49990(void* q, std::uint32_t index) noexcept {
    remove_record_at(q, 0x78, 0x76, index);
}

void matchmaking_cancel_search_00a4af70(void* m, XLiveSearchCalls& x) {
    if (void* const qos = read_field<void*>(m, 0x1938)) {
        x.xnet_qos_release(qos);
        write_field<void*>(m, 0x1938, nullptr);
    }
    if (read_field<std::uint32_t>(m, 0x1944) == 0x3E5) x.x_cancel_overlapped(static_cast<std::byte*>(m) + 0x1944);
    write_field<std::uint32_t>(m, 0x1808, 0);
    if (void* const buffer = read_field<void*>(m, 0x1960)) {
        x.free_00bf65ac(buffer);
        write_field<void*>(m, 0x1960, nullptr);
    }
}

void host_qos_cancel_00a4afd0(void* q, XLiveSearchCalls& x) {
    if (read_field<std::uint32_t>(q, 0x8C) - 1u < 4u) {
        if (void* const lookup = read_field<void*>(q, 0x88)) {
            x.xnet_qos_release(lookup);
            write_field<void*>(q, 0x88, nullptr);
        }
        write_field<std::uint32_t>(q, 0x78, 0);
        write_field<std::uint32_t>(q, 0x8C, 0);
        write_field<std::uint32_t>(q, 0x90, 1);
    }
}

void host_qos_cancel_00a4b410(void* q, XLiveSearchCalls& x) { host_qos_cancel_00a4afd0(q, x); }

void matchmaking_reset_00a4b250(void* m, XLiveSearchCalls& x) {
    switch (read_field<std::uint32_t>(m, 0x193C)) {
    case 1: case 2: case 4: case 0xC:
        matchmaking_cancel_search_00a4af70(m, x);
        write_field<std::uint32_t>(m, 0x193C, 0);
        write_field<std::uint32_t>(m, 0x1940, 1);
        break;
    default:
        break;
    }
}

void matchmaking_reset_00a4b460(void* m, XLiveSearchCalls& x) { matchmaking_reset_00a4b250(m, x); }

void* player_list_construct_00a47b50(void* list, PlayerListCalls& host) {
    write_field<void*>(list, 0x4, host.buy_sentinel_00a43d40());
    write_field<std::uint32_t>(list, 0x8, 0);
    return list;
}

void* player_list_copy_construct_00a474e0(void* list, const void* source, PlayerListCalls& host) {
    void* const head = host.buy_sentinel_00a43d40();
    write_field<void*>(list, 0x4, head);
    write_field<std::uint32_t>(list, 0x8, 0);
    host.insert_range_00a46800(list, read_field<void*>(head, 0x0), source);
    return list;
}

void publish_user_property_10000030_00a4a850(UserPropertyCalls& host, XenonSystemCalls& xenon) {
    if (void* const setup = host.game_setup_5fc())
        host.xuser_set_property(xenon.active_user_slot(), 0x10000030, 4, at(setup, 0x1098));
}
void voice_apply_mute_list_00a48f60(void* voice, Xhv2EngineCalls& xhv, VoiceTalkerCalls& host) {
    std::uint64_t xuids[16]{};
    std::uint32_t count = 0x10;   // [ESP+10h]
    std::uint32_t muted = 0;      // [ESP+14h], uninitialized in the image
    xhv.get_remote_talkers_30(read_field<void*>(voice, 0x78), &count, xuids);
    for (std::uint32_t user = 0; user < 1; ++user) {
        if (read_field<std::uint32_t>(voice, 0x108 + 4 * user) == 0) continue;
        for (std::uint32_t i = 0; i < count; ++i) {
            host.xuser_mute_list_query(user, xuids[i], &muted);
            xhv.set_playback_priority_48(read_field<void*>(voice, 0x78), xuids[i], user,
                muted != 0 ? -1 : 0);
        }
    }
}

std::int32_t voice_add_remote_talker_00a495e0(void* voice, std::uint64_t xuid, Xhv2EngineCalls& xhv) {
    auto* const talkers = read_field<std::byte*>(voice, 0x80);
    const auto count = read_field<std::uint32_t>(voice, 0x7C);
    for (std::uint32_t i = 0; i < count; ++i)
        if (read_field<std::uint64_t>(talkers + i * 0x38, 0) == xuid) return 0;
    const std::int32_t hr = xhv.register_remote_talker_28(read_field<void*>(voice, 0x78), xuid);
    if (hr < 0) return hr;
    static constexpr std::uint32_t kVoiceChatMode[1] = {2}; // 00D24A34
    xhv.start_remote_processing_14(read_field<void*>(voice, 0x78), xuid, kVoiceChatMode, 1);
    std::byte* const record = read_field<std::byte*>(voice, 0x80) + read_field<std::uint32_t>(voice, 0x7C) * 0x38;
    write_field<std::uint64_t>(record, 0x00, xuid);
    write_field<std::int32_t>(record, 0x18, 0);
    write_field<std::int32_t>(record, 0x28, 0);
    write_field<std::int32_t>(record, 0x08, -1);
    write_field<std::uint32_t>(voice, 0x7C, read_field<std::uint32_t>(voice, 0x7C) + 1);
    return 0;
}

std::uint32_t voice_is_remote_talking_00a49840(void* voice, std::uint64_t xuid, std::uint32_t slot,
    Xhv2EngineCalls& xhv, VoiceTalkerCalls& host) {
    if (read_field<std::uint32_t>(voice, 0x84 + 4 * slot) == 0) return 0;
    std::uint32_t talking = xhv.is_remote_talking_3c(read_field<void*>(voice, 0x78), xuid);
    const std::uint32_t headset = xhv.is_headset_present_34(read_field<void*>(voice, 0x78), slot);
    if (talking == 0) return 0;
    std::uint32_t index = 0;
    if (host.find_remote_talker_00a49340(voice, xuid, &index) < 0) return 0;
    const std::byte* const record = read_field<const std::byte*>(voice, 0x80) + index * 0x38;
    if (read_field<std::int32_t>(record, 0x18 + 4 * slot) != 0 || read_field<std::int32_t>(record, 0x28 + 4 * slot) != 0
        || read_field<std::int32_t>(record, 0x08 + 4 * slot) == -1)
        talking = 0;
    if (headset == 0 && read_field<std::uint32_t>(voice, 0xC4 + 4 * slot) == 0) talking = 0;
    return talking;
}
namespace {
struct QosLayout {
    std::size_t state, qos, count, stride, addresses, ids, keys, address_in_record;
};
std::int32_t relink_and_lookup(void* owner, const QosLayout& l, XNetQosLookupCalls& x) {
    if (read_field<std::uint32_t>(owner, l.state) != 4) return static_cast<std::int32_t>(0x8000FFFFu);
    if (void* const qos = read_field<void*>(owner, l.qos)) {
        x.xnet_qos_release(qos);
        write_field<void*>(owner, l.qos, nullptr);
    }
    const auto count = read_field<std::uint32_t>(owner, l.count);
    if (count == 0) return 0;
    auto* const base = static_cast<std::byte*>(owner);
    for (std::uint32_t i = 0; i < count; ++i) {
        std::byte* const address = base + i * l.stride + l.address_in_record;
        write_field<std::uint32_t>(address, 0x34, 0);
        write_field<void*>(owner, l.ids + 4 * i, address - 0x18);
        write_field<void*>(owner, l.addresses + 4 * i, address);
        write_field<void*>(owner, l.keys + 4 * i, address - 0x10);
    }
    void* qos = nullptr;
    const std::int32_t error = x.xnet_qos_lookup(count, reinterpret_cast<const void* const*>(base + l.addresses),
        reinterpret_cast<const void* const*>(base + l.ids), reinterpret_cast<const void* const*>(base + l.keys),
        8, 64000, &qos);
    write_field<void*>(owner, l.qos, qos);
    if (error != 0) return static_cast<std::int32_t>(0x80004005u);
    write_field<std::uint32_t>(owner, l.state, 3);
    return 0;
}
} // namespace

std::int32_t matchmaking_relink_and_lookup_00a49d20(void* m, XNetQosLookupCalls& x) {
    return relink_and_lookup(m, {0x193C, 0x1938, 0x1808, 0xF6, 0x180C, 0x1870, 0x18D4, 0xBE}, x);
}

std::int32_t host_qos_relink_and_lookup_00a4a420(void* q, XNetQosLookupCalls& x) {
    // Order of the three stores per entry follows 00A4A47A..00A4A482: ids, addresses, keys.
    return relink_and_lookup(q, {0x8C, 0x88, 0x78, 0x76, 0x7C, 0x80, 0x84, 0x3E}, x);
}

void* heap_object_scalar_delete_00a4bdc0(void* self, std::uint8_t flags, XLiveCrtHost& host) {
    if (flags & 1) host.free_00bf65ac(self);
    return self;
}

// ===========================================================================
// Packet cc13-online2
// ===========================================================================

static_assert(sizeof(OnlinePlayerRecord) == 0x40);
static_assert(offsetof(OnlinePlayerRecord, id) == 0x20);
static_assert(offsetof(OnlinePlayerRecord, field_28) == 0x28);
static_assert(offsetof(OnlinePlayerRecord, field_2c) == 0x2C);
static_assert(offsetof(OnlinePlayerRecord, user_slot) == 0x30);
static_assert(offsetof(OnlinePlayerRecord, field_34) == 0x34);
static_assert(offsetof(OnlinePlayerRecord, flag_38) == 0x38);
static_assert(offsetof(OnlinePlayerRecord, flag_3b) == 0x3B);
static_assert(offsetof(SessionSearchRequest, use_host_qos) == 0x01);
static_assert(offsetof(SessionSearchRequest, qos_arg_08) == 0x08);
static_assert(offsetof(SessionSearchRequest, search20_arg_10) == 0x10);
static_assert(offsetof(SessionSearchRequest, has_value_1c) == 0x18);
static_assert(offsetof(SessionSearchRequest, value_1c) == 0x1C);
static_assert(offsetof(SessionSearchRequest, has_value_24) == 0x20);
static_assert(offsetof(SessionSearchRequest, value_30) == 0x30);

namespace {

// The defaults every builder stores (00A47130 00A4714F..00A47174, 00A47220
// 00A4724C..00A47284, 00A47AA0 00A47AC3..00A47AE8): an empty string, +2Ch =
// 0, +30h = 0, flags {0, 0, 1, 0}. The other fields keep whatever the
// stack held.
void init_record_defaults(OnlinePlayerRecord& r) noexcept {
    r.name.capacity_18 = 15;
    r.name.length_14 = 0;
    r.name.buffer_04.inline_bytes[0] = '\0';
    r.field_2c = 0;
    r.user_slot = 0;
    r.flag_38 = 0;
    r.flag_39 = 0;
    r.flag_3a = 1;
    r.flag_3b = 0;
}

void assign_c_string(bsp::NativeLegacySboStringStorage& s, const char* text) {
    bsp::native_legacy_sbo_string_assign_counted_00408720(s, text, static_cast<std::uint32_t>(std::strlen(text)));
}

// The A0h-record result vector at system +174h (first +178h, last +17Ch):
// every record's object at +0 is deleted, then the whole range is erased
// (00A46DEC..00A46ED7, 00A46F77..00A47010, 00A48389..00A4841C). The
// checked-index calls to 00BF6713 cannot fire inside this loop.
void clear_search_results(void* system, OnlineSystemCalls& host) {
    const auto first = read_field<std::uintptr_t>(system, 0x178);
    if (first != 0) {
        const auto count = (read_field<std::uintptr_t>(system, 0x17C) - first) / 0xA0;
        for (std::uintptr_t i = 0; i < count; ++i) {
            const auto record = read_field<std::uintptr_t>(system, 0x178) + i * 0xA0;
            if (void* object = *reinterpret_cast<void**>(record)) host.delete_result_object(object);
        }
    }
    host.result_vector_erase_all_00573490(static_cast<std::byte*>(system) + 0x174);
}

} // namespace

void add_local_player_record_00a47130(void* system, OnlineSystemCalls& host) {
    OnlinePlayerRecord record;
    init_record_defaults(record);
    record.user_slot = host.active_user_slot_00a3eac0();
    record.field_34 = 0;
    assign_c_string(record.name, host.selected_username_00a3eae0());
    record.id.xuid = host.active_xuid_00a3eb00();
    host.player_list_push_back(static_cast<std::byte*>(system) + 0x100, record);
    bsp::native_legacy_sbo_string_destroy_004072d0(record.name);
}

void add_lan_local_player_00a47220(void* system, std::uint8_t slot, OnlineSystemCalls& host) {
    if (read_field<std::uint32_t>(system, 0x114) != 0) return;
    OnlinePlayerRecord record;
    init_record_defaults(record);
    record.user_slot = slot;
    const void* string = host.game_setup_string_6a0();
    if (read_field<std::uint32_t>(string, 0) == 0) string = host.game_setup_string_68c();
    const char* text = read_field<const char*>(string, 4);
    assign_c_string(record.name, text != nullptr ? text : "");
    record.flag_3a = 1;
    std::uint8_t xnaddr[36];
    while (host.xnet_get_title_xnaddr(xnaddr) == 0) {
    }
    std::memcpy(record.id.lan.enet, xnaddr + 0x0A, 6);
    record.id.lan.user_slot = slot;
    write_field<std::uint8_t>(system, 0x1D4C + slot, record.flag_3a);
    host.player_list_push_back(static_cast<std::byte*>(system) + 0x10C, record);
    bsp::native_legacy_sbo_string_destroy_004072d0(record.name);
}

bool reset_session_keep_local_player_00a47aa0(void* system, OnlineSystemCalls& host,
    XLiveSearchCalls& search) {
    leave_session_00a46d60(system, host, search);
    OnlinePlayerRecord record;
    init_record_defaults(record);
    host.player_record_assign_006883b0(static_cast<std::byte*>(system) + 0x128, record);
    bsp::native_legacy_sbo_string_destroy_004072d0(record.name);
    add_local_player_record_00a47130(system, host);
    return true;
}

void join_session_record_00a48330(void* system, const void* record, OnlineSystemCalls& host) {
    void* screen = host.session_browser_screen();
    if (screen == nullptr) return;
    if (read_field<std::uint32_t>(host.singleton_player_list_80(), 8) == 0)
        add_local_player_record_00a47130(system, host);
    host.singleton_virtual_1a0(0);
    host.singleton_virtual_d8(1);
    clear_search_results(system, host);
    host.result_vector_push_back_00575150(static_cast<std::byte*>(system) + 0x174, record);
    host.browser_join_session_0057a9c0(host.session_browser_screen(), record, 1);
    write_field<std::uint8_t>(system, 0x4C, 1);
}

void leave_session_00a46d60(void* system, OnlineSystemCalls& host, XLiveSearchCalls& search) {
    auto* const base = static_cast<std::byte*>(system);
    if (read_field<std::uint8_t>(system, 0xED) != 0) {
        host.system_virtual_8c(system);
        const auto matchmaking_state = read_field<std::uint32_t>(system, 0x1AC0);
        if (matchmaking_state == 1 || matchmaking_state == 2) matchmaking_reset_00a4b250(base + 0x184, search);
        if (read_field<std::uint8_t>(system, 0xEF) != 0) {
            const auto qos_state = read_field<std::uint32_t>(system, 0x1B74);
            if (qos_state == 1 || qos_state == 2) host_qos_cancel_00a4afd0(base + 0x1AE8, search);
        }
    }
    const auto session_state = read_field<std::uint32_t>(system, 0x21F0);
    if (session_state == 4 || session_state == 2) host.system_virtual_170(system, 0);
    host.player_list_clear_00688780(base + 0x100);
    host.player_list_clear_00688780(base + 0x10C);
    clear_search_results(system, host);
}

bool begin_session_search_00a46f60(void* system, const SessionSearchRequest& request,
    OnlineSystemCalls& host, XLiveQosHost& qos) {
    if (read_field<std::uint8_t>(system, 0xED) != 0) return false;
    auto* const base = static_cast<std::byte*>(system);
    write_field<std::uint8_t>(system, 0xEE, 0);
    clear_search_results(system, host);
    const bool blocked = host.system_virtual_19c(system);
    const bool allowed = host.system_virtual_40(system);
    if (!allowed || blocked) {
        host.game_1ef0_0076a6f0(system, 0, 0);
        write_field<std::uint8_t>(system, 0xED, 1);
        return true;
    }
    std::int32_t result;
    if (request.use_host_qos != 0) {
        write_field<std::uint8_t>(system, 0xEF, 1);
        write_field<std::uint8_t>(system, 0xF0, 0);
        result = host_qos_finish_if_complete_00a4a3d0(base + 0x1AE8, qos);
    } else {
        write_field<std::uint8_t>(system, 0xEF, 0);
        if (request.search_by_id != 0) {
            write_field<std::uint8_t>(system, 0xF0, 1);
            result = host.matchmaking_begin_search20_00a4b2a0(base + 0x184, request.search20_arg_10,
                request.search20_arg_14);
        } else {
            write_field<std::uint8_t>(system, 0xF0, 0);
            constexpr std::uint64_t kAny = 0x7FFFFFFFFFFFFFFFull;
            const std::uint64_t values[6] = {
                request.value_30, request.value_2c, request.value_28,
                request.has_value_24 != 0 ? request.value_24 : kAny,
                request.has_value_1c != 0 ? request.value_1c : kAny,
                kAny,
            };
            result = host.matchmaking_begin_search_00a4b7f0(base + 0x184, values);
        }
    }
    if (result < 0) {
        write_field<std::int32_t>(system, 0x1BD8, result);
        write_field<std::uint8_t>(system, 0x1BD5, 0);
        return false;
    }
    write_field<std::uint8_t>(system, 0xED, 1);
    return true;
}

namespace {
constexpr std::uint32_t kErrorIoPending = 0x3E5;

// XNetQosListen(key +C7h, 0, 0, 0, XNET_QOS_LISTEN_RELEASE); shared by
// 00A49C80 and the inlined copy in 00A4A790.
std::int32_t release_qos_listen(void* session, XSessionLifetimeCalls& x) {
    const std::int32_t error = x.xnet_qos_listen(static_cast<std::byte*>(session) + 0xC7, nullptr, 0, 0, 0x10);
    if (error != 0) {
        x.log("Warning: Failed to stop QoS listener, error 0x%08x\n", error);
        return error;
    }
    write_field<std::uint32_t>(session, 0x130, 0);
    return 0;
}
} // namespace

std::int32_t session_stop_qos_listen_00a49c80(void* session, XSessionLifetimeCalls& x) {
    if (read_field<std::uint32_t>(session, 0x130) == 0) return 0;
    return release_qos_listen(session, x);
}

void session_terminate_xnet_00a4a790(void* session, XSessionLifetimeCalls& x) {
    write_field<std::uint32_t>(session, 0x158, 0);
    write_field<std::uint32_t>(session, 0x160, 0);
    if (read_field<std::uint32_t>(session, 0x130) != 0) (void)release_qos_listen(session, x);
    if (read_field<std::uint32_t>(session, 0x15C) != 0) {
        std::uint8_t key[8];
        std::memcpy(key, static_cast<std::byte*>(session) + 0xB8, 8);
        x.log_key("%x%x%x%x%x%x%x%x UnregisterKey cNetworkConsole::TerminateXNet() ", key);
        x.xnet_unregister_key(static_cast<std::byte*>(session) + 0xB8);
        write_field<std::uint32_t>(session, 0x15C, 0);
    }
}

void session_terminate_xnet_00a4b020(void* session, XSessionLifetimeCalls& x) {
    session_terminate_xnet_00a4a790(session, x);
}

std::int32_t session_complete_delete_00a4af20(void* session, XSessionLifetimeCalls& x) {
    if (read_field<std::uint32_t>(session, 0x110) == kErrorIoPending) return 0;
    x.x_close_handle(read_field<void*>(session, 0x12C));
    write_field<std::uint32_t>(session, 0x12C, 0xFFFFFFFFu);
    const auto error = static_cast<std::int32_t>(
        x.x_get_overlapped_extended_error(static_cast<std::byte*>(session) + 0x110));
    if (error == 0) {
        write_field<std::uint32_t>(session, 0x158, 0);
        write_field<std::uint32_t>(session, 0x160, 0);
    }
    return error;
}

std::int32_t session_complete_create_00a4b030(void* session, XSessionLifetimeCalls& x,
    XSessionCreateCalls& create) {
    auto* const base = static_cast<std::byte*>(session);
    void* const overlapped = base + 0x110;
    if (read_field<std::uint32_t>(session, 0x110) == kErrorIoPending) return 0;
    if (x.x_get_overlapped_extended_error(overlapped) != 0) {
        std::uint32_t ignored = 0;
        x.x_get_overlapped_result(overlapped, &ignored, 0);
        (void)x.x_get_overlapped_extended_error(overlapped);
        return static_cast<std::int32_t>(0x8000FFFFu);
    }
    std::memcpy(base + 0xB8, base + 0xC7, 8);   // XNKID
    std::memcpy(base + 0xA8, base + 0xF3, 16);  // XNKEY
    void* const setup = x.game_setup();
    write_field<std::uint32_t>(setup, 0x2158, read_field<std::uint32_t>(session, 0x108));
    write_field<std::uint32_t>(setup, 0x215C, read_field<std::uint32_t>(session, 0x10C));
    write_field<std::uint32_t>(session, 0x158, 4);
    const std::uint8_t* const title = x.qos_title_data();
    std::size_t chars = 0;
    while (read_field<std::uint16_t>(title + 2, chars * 2) != 0) ++chars;
    const std::uint32_t flags = read_field<std::uint32_t>(session, 0x130) == 0 ? 5u : 4u;
    const std::int32_t error = x.xnet_qos_listen(base + 0xC7, title + 1,
        static_cast<std::uint32_t>(chars * 2), 0, flags);
    if (error == 0) write_field<std::uint32_t>(session, 0x130, 1);
    else x.log("Failed to start QoS listener, error 0x%08x\n", error);
    create.join_local_player_00a4ae70(session);
    if (read_field<std::uint32_t>(session, 0x160) != 0) return create.flag_restart_00a49a00(session);
    return 0;
}

std::int32_t voice_initialize_00a493c0(void* voice, VoiceInitCalls& host, XenonSystemCalls& xenon,
    Xhv2EngineCalls& xhv) {
    if (!host.has_selected_user_00a3e510()) return 0;
    if (host.signin_state_00a3ead0() == 2) {
        std::uint8_t privilege = 0;
        if (!host.communication_privilege_00a3eb30(&privilege)) return 0;
    }
    static constexpr std::uint32_t kLocalModes[2] = {2, 1};
    static constexpr std::uint32_t kRemoteModes[1] = {2};
    write_field<std::uint32_t>(voice, 0x84, 0);
    write_field<std::uint32_t>(voice, 0xC4, 0);
    write_field<std::uint32_t>(voice, 0x108, 0);
    write_field<std::uint32_t>(voice, 0x4, 0);
    write_field<std::uint32_t>(voice, 0x0, 0);
    const XhvInitParams params{0x10, 1, kLocalModes, 2, kRemoteModes, 1, 0, 0, nullptr,
        host.focus_window_00bec230()};
    void* engine = read_field<void*>(voice, 0x78);
    const std::int32_t created = host.xhv_create_engine(params, &engine);
    write_field<void*>(voice, 0x78, engine);
    if (created < 0) return created;
    write_field<std::uint32_t>(voice, 0x70, host.get_tick_count());
    void* const listener = host.xnotify_create_listener(1);
    write_field<void*>(voice, 0x10C, listener);
    if (listener != nullptr && listener != reinterpret_cast<void*>(static_cast<std::intptr_t>(-1))) {
        void* const talkers = host.operator_new_00bf55be(0x380);
        write_field<void*>(voice, 0x80, talkers);
        if (talkers != nullptr) {
            write_field<std::uint32_t>(voice, 0x74, 0);
            write_field<std::uint32_t>(voice, 0x104, 1);
            if (xenon.slot_state_8c(0) != 0) {
                write_field<std::uint32_t>(voice, 0x4, xhv.is_headset_present_34(engine, 0));
                write_field<std::uint32_t>(voice, 0x0, 1);
            }
            return 0;
        }
    }
    host.shutdown_00a49270(voice);
    return static_cast<std::int32_t>(0x8007000Eu);
}

void session_set_user_properties_00a4a890(void* session, const std::uint32_t settings[14], std::int32_t p3,
    std::int32_t p4, std::uint64_t p5, std::int32_t p7, std::int32_t p8, bool force, SessionPropertyCalls& host) {
    const std::uint32_t user = host.active_user_slot_00a3eac0();
    const std::uint32_t one = 1;
    const std::uint32_t zero = 0;
    auto flag = [&](bool on) -> const void* { return on ? &one : &zero; };
    const auto mode = read_field<std::uint32_t>(session, 0x58);
    if (mode != settings[2] || force) {
        host.xuser_set_property(user, 0x100000CD, 4, flag(mode <= 3));
        host.xuser_set_property(user, 0x100000C9, 4, flag(read_field<std::uint32_t>(session, 0x58) == 4));
        host.xuser_set_property(user, 0x100000CA, 4, flag(read_field<std::uint32_t>(session, 0x58) == 5));
        host.xuser_set_property(user, 0x100000CB, 4, flag(read_field<std::uint32_t>(session, 0x58) == 6));
        host.xuser_set_property(user, 0x100000CC, 4, flag(read_field<std::uint32_t>(session, 0x58) == 7));
    }
    if (p3 != read_field<std::int32_t>(session, 0x44) || force) host.xuser_set_property(user, 0x10000048, 4, &p3);
    if (p4 != read_field<std::int32_t>(session, 0x40) || force) host.xuser_set_property(user, 0x10000047, 4, &p4);
    if (p7 != read_field<std::int32_t>(session, 0x48) || force) host.xuser_set_property(user, 0x100000D9, 4, &p7);
    if (p8 != read_field<std::int32_t>(session, 0x4C) || force) host.xuser_set_property(user, 0x100000DA, 4, &p8);
    auto* const base = static_cast<std::byte*>(session);
    if (read_field<std::uint64_t>(session, 0x98) != read_field<std::uint64_t>(session, 0xA0) || force)
        host.xuser_set_property(user, 0x200000DB, 8, base + 0xA0);
    if (read_field<std::uint64_t>(session, 0x90) != read_field<std::uint64_t>(session, 0xB8) || force)
        host.xuser_set_property(user, 0x200000D8, 8, base + 0xB8);
    if (read_field<std::uint64_t>(session, 0x88) != p5 || force) host.xuser_set_property(user, 0x200000D7, 8, &p5);
    static constexpr std::uint32_t kSettingIds[10] = {
        0x100000D0, 0x100000D6, 0x100000D3, 0x100000D5, 0x100000D5,
        0x100000D1, 0x100000D4, 0x100000CF, 0x100000CE, 0x100000D2,
    };
    for (std::size_t i = 0; i < 10; ++i) {
        if (read_field<std::uint32_t>(session, 0x5C + 4 * i) != settings[3 + i] || force)
            host.xuser_set_property(user, kSettingIds[i], 4, &settings[3 + i]);
    }
    write_field<std::int32_t>(session, 0x44, p3);
    write_field<std::int32_t>(session, 0x40, p4);
    write_field<std::int32_t>(session, 0x4C, p8);
    write_field<std::int32_t>(session, 0x48, p7);
    std::memcpy(base + 0x50, settings, 14 * 4);
    write_field<std::uint64_t>(session, 0x90, read_field<std::uint64_t>(session, 0xB8));
    write_field<std::uint64_t>(session, 0x98, read_field<std::uint64_t>(session, 0xA0));
    write_field<std::uint64_t>(session, 0x88, p5);
}

std::uint32_t session_create_00a4ac40(void* session, SessionCreateCalls& host, SessionPropertyCalls& properties) {
    auto* const base = static_cast<std::byte*>(session);
    if (read_field<std::uint32_t>(session, 0x158) != 0) {
        write_field<std::uint32_t>(session, 0x158, 0);
        write_field<std::uint32_t>(session, 0x160, 0);
    }
    std::memset(base + 0x110, 0, 0x1C);
    host.xuser_set_context(host.active_user_slot_00a3eac0(), 0x800B, 0);
    const bool flag_4d = host.system_byte_4d() != 0;
    const std::uint32_t flags = flag_4d ? 0x43Fu : 0x42Fu;
    host.xuser_set_context(host.active_user_slot_00a3eac0(), 0x800A, flag_4d ? 0u : 1u);
    host.log("CSession::Create()  -- >  XSessionCreate");
    const std::uint32_t result = host.xsession_create(flags, host.active_user_slot_00a3eac0(),
        read_field<std::uint32_t>(session, 0x34), read_field<std::uint32_t>(session, 0x3C), base + 0x108,
        base + 0xC7, base + 0x110, base + 0x12C);
    const void* const setup = host.game_setup();
    if (const auto* block = read_field<const std::byte*>(setup, 0x5FC))
        host.xuser_set_property(host.active_user_slot_00a3eac0(), 0x10000030, 4, block + 0x1098);
    const void* const xenon = host.xenon_system();
    const auto p7 = (std::max)(read_field<std::uint32_t>(xenon, 0x358), read_field<std::uint32_t>(setup, 0x73C));
    const auto p8 = (std::max)(read_field<std::uint32_t>(xenon, 0x35C), read_field<std::uint32_t>(setup, 0x740));
    std::uint32_t settings[14];
    std::memcpy(settings, static_cast<const std::byte*>(setup) + 0x2020, sizeof(settings));
    session_set_user_properties_00a4a890(session, settings, read_field<std::int8_t>(setup, 0x201C),
        read_field<std::int8_t>(setup, 0x201D), read_field<std::uint64_t>(setup, 0x2060),
        static_cast<std::int32_t>(p7), static_cast<std::int32_t>(p8), true, properties);
    if (result == kErrorIoPending) {
        write_field<std::uint8_t>(session, 0xC4, 0);
        write_field<std::uint8_t>(session, 0xC5, 0);
        write_field<std::uint8_t>(session, 0xC6, 0);
        write_field<std::uint32_t>(session, 0x158, 1);
    }
    return 0;
}

void session_info_fill_from_game_setup_00a4bb00(void* session, const void* block, SessionInfoCalls& host) {
    std::u16string name = host.widen_004c5e60(static_cast<const char*>(block));
    if (name.size() > 0x10) name = name.substr(0, 0x10);
    auto* const base = static_cast<std::byte*>(session);
    std::memcpy(base, name.c_str(), (name.size() + 1) * sizeof(char16_t));
    auto c = [&](std::size_t offset) { return static_cast<std::int32_t>(read_field<std::int8_t>(block, offset)); };
    write_field<std::int64_t>(session, 0x28, c(0x27));
    write_field<std::int32_t>(session, 0x30, c(0x28) - c(0x2A));
    write_field<std::int32_t>(session, 0x34, c(0x2A) - c(0x28) - c(0x29) + c(0x27));
    write_field<std::int32_t>(session, 0x38, c(0x2A));
    write_field<std::int32_t>(session, 0x3C, c(0x29) - c(0x2A));
    write_field<std::uint64_t>(session, 0x88, read_field<std::uint64_t>(block, 0x70));
    write_field<std::int32_t>(session, 0x40, c(0x2D));
    write_field<std::int32_t>(session, 0x44, c(0x2C));
    write_field<std::uint64_t>(session, 0x48, read_field<std::uint64_t>(block, 0x78));
    write_field<std::uint64_t>(session, 0xA0, read_field<std::uint64_t>(block, 0x80));
    std::memcpy(base + 0x50, static_cast<const std::byte*>(block) + 0x30, 14 * 4);
}

std::uint32_t matchmaking_select_query_id_00a4b490(std::uint64_t mask, const MatchmakingSearchGlobals& globals) {
    if (mask == 0) return 0;
    const bool c = (mask & 0x80) != 0;
    const bool d = (mask & 0x10) != 0;
    const bool e = (mask & 0x20) != 0;
    const bool s = (mask & 0x40) != 0;
    const bool i = (mask & 0x0F) != 0;
    const int count = int{i} + int{s} + int{e} + int{d} + int{c};
    if (count == 1) {
        if (c) return 3;
        if (d) return 1;
        if (e) return 2;
        if (s) return 4;
        if (i) return 5;
        return 0;
    }
    if (static_cast<unsigned>(count - 2) >= 3) return 0;
    // The pooled string built through BSP_NativeString_Assign/Append; its
    // storage goes back to the pool (00419CC0/00BD1510) on every exit.
    std::string key;
    if (c) key += 'C';
    if (d) key += 'D';
    if (e) key += 'E';
    if (s) key += 'S';
    if (i) key += 'I';
    for (std::size_t row = 5; row < 31; ++row) {
        const auto& r = globals.query_table[row];
        // 0043E9A0(row key, composed, 0): a non-null key that starts with the
        // composed string.
        if (r.key != nullptr && std::strncmp(r.key, key.c_str(), key.size()) == 0) return r.query_id;
    }
    return 0;
}

namespace {
// Shared head of 00A4B2A0 and 00A4B7F0: cancel a running search, zero the
// XOVERLAPPED +1944h, probe the result size and buy the buffer at +1960h.
bool prepare_search(void* m, std::uint32_t procedure, std::uint32_t& bytes, MatchmakingSearchCalls& host,
    XLiveSearchCalls& search) {
    auto* const base = static_cast<std::byte*>(m);
    if (read_field<std::uint32_t>(m, 0x193C) != 0) {
        matchmaking_cancel_search_00a4af70(m, search);
        write_field<std::uint32_t>(m, 0x193C, 0);
    }
    std::memset(base + 0x1944, 0, 0x1C);
    bytes = 0;
    const std::uint32_t probe = host.xsession_search_ex(procedure, host.active_user_slot_00a3eac0(), 0x19, 1, 0, 0,
        nullptr, nullptr, &bytes, nullptr, base + 0x1944);
    if (probe != 0x7A || bytes == 0) return false;
    void* const buffer = host.operator_new_00bf55be(bytes);
    write_field<void*>(m, 0x1960, buffer);
    return buffer != nullptr;
}

void fill_search_contexts(XUserContext (&contexts)[2], MatchmakingSearchCalls& host) {
    contexts[0] = {0x800A, host.system_byte_4d() == 0 ? 1u : 0u};
    contexts[1] = {0x800B, 0};
}

void set_int32_property(XUserProperty& p, std::uint32_t id, std::int32_t value) {
    p.id = id;
    p.data.type = 1;
    // Only the low dword is stored (MOV dword at +10h); the union's high
    // dword keeps its previous contents.
    std::memcpy(&p.data.value, &value, 4);
}
} // namespace

std::int32_t matchmaking_begin_search20_00a4b2a0(void* matchmaking, std::uint64_t session_id,
    MatchmakingSearchGlobals& globals, MatchmakingSearchCalls& host, XLiveSearchCalls& search) {
    const char* name = host.query_name_00f8aefc();
    host.log("MatchMaking Query ID %s", name != nullptr ? name : "");
    std::uint32_t bytes = 0;
    if (!prepare_search(matchmaking, 0x20, bytes, host, search)) return static_cast<std::int32_t>(0x8000FFFFu);
    globals.property20.id = 0x200000D8;
    globals.property20.data.type = 2;
    globals.property20.data.value = static_cast<std::int64_t>(session_id);
    fill_search_contexts(globals.contexts20, host);
    auto* const base = static_cast<std::byte*>(matchmaking);
    const std::uint32_t result = host.xsession_search_ex(0x20, host.active_user_slot_00a3eac0(), 0x19, 1, 1, 2,
        &globals.property20, globals.contexts20, &bytes, read_field<void*>(matchmaking, 0x1960), base + 0x1944);
    if (result != kErrorIoPending) return static_cast<std::int32_t>(0x8000FFFFu);
    write_field<std::uint32_t>(matchmaking, 0x193C, 1);
    return 0;
}

std::int32_t matchmaking_begin_search_00a4b7f0(void* matchmaking, const std::uint64_t q[6],
    MatchmakingSearchGlobals& globals, MatchmakingSearchCalls& host, XLiveSearchCalls& search) {
    const std::uint64_t mask = q[0];
    const std::uint32_t procedure = matchmaking_select_query_id_00a4b490(mask, globals);
    // 00A4B80E..00A4B823: row [procedure] of the query table, key buffer.
    const char* key = procedure < 31 ? globals.query_table[procedure].key : nullptr;
    host.log("MatchMaking Query ID %s", key != nullptr ? key : "");
    std::uint32_t bytes = 0;
    if (!prepare_search(matchmaking, procedure, bytes, host, search)) return static_cast<std::int32_t>(0x8000FFFFu);
    const std::uint64_t single = procedure - 1u <= 4u ? 1u : 0u;
    set_int32_property(globals.properties[0], 0x10000047, static_cast<std::int32_t>(q[2]));
    set_int32_property(globals.properties[1], 0x10000048, static_cast<std::int32_t>(q[1]));
    std::uint32_t count = 2;
    struct ModeProperty { std::uint64_t bits; std::uint32_t id; const char* text; };
    static constexpr ModeProperty kModes[5] = {
        {0x80, 0x100000CC, "MatchMaking Query Set Mode COMPETITIVE"},
        {0x10, 0x100000C9, "MatchMaking Query Set Mode DUEL"},
        {0x20, 0x100000CA, "MatchMaking Query Set Mode ESCORT"},
        {0x40, 0x100000CB, "MatchMaking Query Set Mode SIEGE"},
        {0x0F, 0x100000CD, "MatchMaking Query Set Mode IC"},
    };
    for (const auto& mode : kModes) {
        // C/D/E/S compare the masked bit itself (0 or the bit) with `single`;
        // I compares (any of 0Fh) as 0/1. Either way only "bit clear and not
        // single" matches.
        const std::uint64_t tested = mode.bits == 0x0F ? ((mask & 0x0F) != 0 ? 1u : 0u) : (mask & mode.bits);
        if (tested != single) continue;
        set_int32_property(globals.properties[count], mode.id, static_cast<std::int32_t>(single));
        ++count;
        host.log_text(mode.text);
    }
    fill_search_contexts(globals.contexts, host);
    auto* const base = static_cast<std::byte*>(matchmaking);
    const std::uint32_t result = host.xsession_search_ex(procedure, host.active_user_slot_00a3eac0(), 0x19, 1,
        static_cast<std::uint16_t>(count), 2, globals.properties, globals.contexts, &bytes,
        read_field<void*>(matchmaking, 0x1960), base + 0x1944);
    if (result != kErrorIoPending) return static_cast<std::int32_t>(0x8000FFFFu);
    write_field<std::uint32_t>(matchmaking, 0x193C, 1);
    return 0;
}

namespace {
// The search/QoS state machine layout shared by 00A49DF0 and 00A4A4E0.
struct PollLayout {
    std::size_t state, result, count, qos, overlapped, stride, info_in_record;
    std::uint32_t done_state;
};

std::byte* qos_info(const void* owner, const PollLayout& l, std::uint32_t index) {
    return read_field<std::byte*>(owner, l.qos) + 8 + index * 0x18;
}

// State 3: attach completed XNQOSINFO entries (00A49E2A.., 00A4A51C..).
std::int32_t poll_attach_completed(void* owner, const PollLayout& l) {
    auto* const base = static_cast<std::byte*>(owner);
    for (std::uint32_t i = 0; i < read_field<std::uint32_t>(owner, l.count); ++i) {
        std::byte* const slot = base + i * l.stride + l.info_in_record;
        if (read_field<void*>(slot, 0) == nullptr && (read_field<std::uint8_t>(qos_info(owner, l, i), 0) & 1) != 0)
            write_field<void*>(slot, 0, qos_info(owner, l, i));
    }
    if (read_field<std::uint32_t>(read_field<void*>(owner, l.qos), 4) == 0) {
        write_field<std::uint32_t>(owner, l.state, l.done_state);
        return read_field<std::int32_t>(owner, l.result);
    }
    return static_cast<std::int32_t>(kErrorIoPending);
}

// State 2: drop entries that were not contacted or are disabled.
template <class RemoveAt>
std::int32_t poll_prune(void* owner, const PollLayout& l, RemoveAt remove_at) {
    if (read_field<std::uint32_t>(read_field<void*>(owner, l.qos), 4) != 0)
        return static_cast<std::int32_t>(kErrorIoPending);
    auto* const base = static_cast<std::byte*>(owner);
    std::uint32_t kept = 0;
    const std::uint32_t count = read_field<std::uint32_t>(owner, l.count);
    for (std::uint32_t i = 0; i < count; ++i) {
        const std::uint8_t flags = read_field<std::uint8_t>(qos_info(owner, l, i), 0);
        if ((flags & 2) == 0 || (flags & 4) != 0) {
            remove_at(owner, kept);
        } else {
            write_field<void*>(base + kept * l.stride + l.info_in_record, 0, qos_info(owner, l, i));
            ++kept;
        }
    }
    write_field<std::uint32_t>(owner, l.state, l.done_state);
    return read_field<std::int32_t>(owner, l.result);
}

// The overlapped-failure tail of state 1.
std::int32_t poll_overlapped_failed(void* overlapped, SearchPollCalls& x) {
    std::uint32_t ignored = 0;
    x.x_get_overlapped_result(overlapped, &ignored, 0);
    (void)x.x_get_overlapped_extended_error(overlapped);
    return static_cast<std::int32_t>(0x8000FFFFu);
}

// XNKID, XNKEY, XNADDR and the slot counts of one XSESSION_SEARCHRESULT
// (dwords 0-1, 0Bh-0Eh, 2-0Ah, 0Fh-12h) into a record whose XNKID is at
// `id`; the key follows at id+8h, the address at id+18h, the counts at
// id+3Ch, the QoS info pointer at id+4Ch.
void copy_search_result(std::byte* id, const std::byte* result) {
    std::memcpy(id, result, 8);
    std::memcpy(id + 0x08, result + 0x2C, 16);
    std::memcpy(id + 0x18, result + 0x08, 0x24);
    std::memcpy(id + 0x3C, result + 0x3C, 16);
}
} // namespace

std::int32_t matchmaking_poll_search_00a49df0(void* m, SearchPollCalls& x) {
    static constexpr PollLayout l{0x193C, 0x1940, 0x1808, 0x1938, 0x1944, 0xF6, 0xF2, 0x0C};
    const auto state = read_field<std::uint32_t>(m, l.state);
    if (state == 0) return 0;
    if (state == 4) return read_field<std::int32_t>(m, l.result);
    if (state == 3) return poll_attach_completed(m, l);
    if (state == 2) return poll_prune(m, l, [](void* o, std::uint32_t i) { matchmaking_remove_result_at_00a49920(o, i); });
    auto* const base = static_cast<std::byte*>(m);
    void* const overlapped = base + l.overlapped;
    if (read_field<std::uint32_t>(m, l.overlapped) == kErrorIoPending) return 0;
    if (x.x_get_overlapped_extended_error(overlapped) != 0) return poll_overlapped_failed(overlapped, x);
    write_field<std::uint32_t>(m, l.state, 4);
    const auto* const header = read_field<const std::byte*>(m, 0x1960);
    const std::uint32_t total = read_field<std::uint32_t>(header, 0);
    write_field<std::uint32_t>(m, l.count, total);
    if (total == 0) return 0;
    std::uint32_t kept = 0;
    for (std::uint32_t i = 0; i < total; ++i) {
        const std::byte* const result = read_field<const std::byte*>(read_field<const std::byte*>(m, 0x1960), 4) + i * 0x5C;
        std::byte* const record = base + kept * l.stride;
        std::uint32_t platform = 0;
        x.xnet_get_xnaddr_platform(result + 8, &platform);
        write_field<std::uint32_t>(record, 0xF2, 0);
        if (platform == 1) continue;
        copy_search_result(record + 0xA6, result);
        const auto slot = [&](std::size_t dword) { return read_field<std::uint32_t>(result, dword * 4); };
        write_field<std::uint32_t>(record, 0x10, slot(0x10) + slot(0x0F) + slot(0x12) + slot(0x11));
        write_field<std::uint32_t>(record, 0x14, 0);
        write_field<std::uint32_t>(record, 0x18, slot(0x0F));
        write_field<std::uint32_t>(record, 0x1C, 0);
        std::memset(record, 0, 0x10);
        const std::uint32_t properties = slot(0x13);
        const auto* const property_base = read_field<const std::byte*>(result, 0x54);
        for (std::uint32_t p = 0; p < properties; ++p) {
            const std::byte* const property = property_base + p * 0x18;
            const auto id = read_field<std::uint32_t>(property, 0);
            const auto value = read_field<std::uint32_t>(property, 0x10);
            if (read_field<std::uint8_t>(property, 8) == 4) {
                const auto* text = read_field<const std::byte*>(property, 0x14);
                std::byte* out = record + 0x20;
                std::uint16_t unit;
                do {
                    std::memcpy(&unit, text, 2);
                    std::memcpy(out, &unit, 2);
                    text += 2;
                    out += 2;
                } while (unit != 0);
            }
            switch (id) {
            case 0x10000030: write_field<std::uint32_t>(record, 0x42, value); write_field<std::uint32_t>(record, 0x5A, value); break;
            case 0x10000048: write_field<std::uint32_t>(record, 0x46, value); break;
            case 0x10000047: write_field<std::uint32_t>(record, 0x4A, value); break;
            case 0x200000D7: write_field<std::uint64_t>(record, 0x8E, read_field<std::uint64_t>(property, 0x10)); break;
            case 0x100000D0: write_field<std::uint32_t>(record, 0x62, value); break;
            case 0x100000D6: write_field<std::uint32_t>(record, 0x66, value); break;
            case 0x100000D3: write_field<std::uint32_t>(record, 0x6A, value); break;
            // 00A4A12E and 00A4A143 both test 100000D5h.
            case 0x100000D5: write_field<std::uint32_t>(record, 0x6E, value); write_field<std::uint32_t>(record, 0x72, value); break;
            case 0x100000D1: write_field<std::uint32_t>(record, 0x76, value); break;
            case 0x100000D4: write_field<std::uint32_t>(record, 0x7A, value); break;
            case 0x100000CF: write_field<std::uint32_t>(record, 0x7E, value); break;
            case 0x100000CE: write_field<std::uint32_t>(record, 0x82, value); break;
            case 0x100000D2: write_field<std::uint32_t>(record, 0x86, value); break;
            case 0x100000C9: if (value == 1) write_field<std::uint32_t>(record, 0x5E, 4); break;
            case 0x100000CB: if (value == 1) write_field<std::uint32_t>(record, 0x5E, 6); break;
            case 0x100000CA: if (value == 1) write_field<std::uint32_t>(record, 0x5E, 5); break;
            case 0x100000CC: if (value == 1) write_field<std::uint32_t>(record, 0x5E, 7); break;
            case 0x100000CD: if (value == 1) write_field<std::uint32_t>(record, 0x5E, 0); break;
            case 0x200000DB: write_field<std::uint64_t>(record, 0x9E, read_field<std::uint64_t>(property, 0x10)); break;
            case 0x200000D8: write_field<std::uint64_t>(record, 0x96, read_field<std::uint64_t>(property, 0x10)); break;
            case 0x100000D9: write_field<std::uint32_t>(record, 0x4E, value); break;
            case 0x100000DA: write_field<std::uint32_t>(record, 0x52, value); break;
            default: break;
            }
        }
        if (read_field<std::uint32_t>(record, 0x5E) == 0) write_field<std::uint32_t>(record, 0x5E, read_field<std::uint32_t>(record, 0x62));
        write_field<std::uint32_t>(record, 0xF2, 0);
        write_field<std::byte*>(m, 0x180C + 4 * kept, record + 0xBE);
        write_field<std::byte*>(m, 0x1870 + 4 * kept, record + 0xA6);
        write_field<std::byte*>(m, 0x18D4 + 4 * kept, record + 0xAE);
        ++kept;
    }
    if (kept < total) write_field<std::uint32_t>(m, l.count, kept);
    if (kept != 0) {
        void* qos = read_field<void*>(m, l.qos);
        const std::int32_t error = x.xnet_qos_lookup(kept, base + 0x180C, base + 0x1870, base + 0x18D4, 8, 64000, &qos);
        write_field<void*>(m, l.qos, qos);
        if (error == 0) write_field<std::uint32_t>(m, l.state, 3);
    }
    return 0;
}

std::int32_t host_qos_poll_00a4a4e0(void* q, SearchPollCalls& x) {
    static constexpr PollLayout l{0x8C, 0x90, 0x78, 0x88, 0x94, 0x76, 0x72, 4};
    const auto state = read_field<std::uint32_t>(q, l.state);
    if (state == 0) return 0;
    if (state == 4) return read_field<std::int32_t>(q, l.result);
    if (state == 3) return poll_attach_completed(q, l);
    if (state == 2) return poll_prune(q, l, [](void* o, std::uint32_t i) { host_qos_remove_entry_at_00a49990(o, i); });
    auto* const base = static_cast<std::byte*>(q);
    void* const overlapped = base + l.overlapped;
    if (read_field<std::uint32_t>(q, l.overlapped) == kErrorIoPending) return 0;
    if (x.x_get_overlapped_extended_error(overlapped) != 0) return poll_overlapped_failed(overlapped, x);
    write_field<std::uint32_t>(q, l.state, 4);
    const auto* const header = read_field<const std::byte*>(q, 0xB0);
    const std::uint32_t total = read_field<std::uint32_t>(header, 0);
    write_field<std::uint32_t>(q, l.count, total);
    // Entry i at +0 + i*76h; the stores follow 00A4A660..00A4A6E4 in order, so
    // with two or more results later entries overwrite +78h onwards, as the
    // image does.
    for (std::uint32_t i = 0; i < total; ++i) {
        const std::byte* const result = read_field<const std::byte*>(read_field<const std::byte*>(q, 0xB0), 4) + i * 0x5C;
        std::byte* const entry = base + i * l.stride;
        copy_search_result(entry + 0x26, result);
        write_field<std::uint32_t>(entry, 0x72, 0);
        write_field<std::byte*>(q, 0x7C + 4 * i, entry + 0x3E);
        write_field<std::byte*>(q, 0x80 + 4 * i, entry + 0x26);
        write_field<std::byte*>(q, 0x84 + 4 * i, entry + 0x2E);
    }
    if (total != 0) {
        void* qos = read_field<void*>(q, l.qos);
        const std::int32_t error = x.xnet_qos_lookup(total, base + 0x7C, base + 0x80, base + 0x84, 0, 64000, &qos);
        write_field<void*>(q, l.qos, qos);
        if (error == 0) write_field<std::uint32_t>(q, l.state, 2);
    }
    return 0;
}

namespace {
constexpr std::uint32_t kSessionInfoHolderVtable = 0x00CEED04u;

// The inline wide-string reset before 005707A0 and after each push
// (00A48588..00A48594, 00A488A2..00A488AA): capacity 7, size 0, unit 0.
void reset_record_name(std::byte* record) noexcept {
    write_field<std::uint32_t>(record, 0x20, 7);
    write_field<std::uint32_t>(record, 0x1C, 0);
    write_field<std::uint16_t>(record, 0x0C, 0);
}

// The name's heap buffer goes back when the capacity is 8 units or more.
void release_record_name(std::byte* record, SessionRecordCalls& host) {
    if (read_field<std::uint32_t>(record, 0x20) >= 8) host.free_00bf65ac(read_field<void*>(record, 0x0C));
}

// new(88h) holder: vtable, XNKID +4h, XNKEY +0Ch, XNADDR +1Ch, copied from
// a record whose XNKID is at `id` (key id+8h, address id+18h).
void* make_session_info_holder(const std::byte* id, SessionRecordCalls& host) {
    void* holder = host.operator_new_00bf681b(0x88);
    if (holder != nullptr) write_field<std::uint32_t>(holder, 0, kSessionInfoHolderVtable);
    auto* const h = static_cast<std::byte*>(holder);
    std::memcpy(h + 0x0C, id + 0x08, 16);
    std::memcpy(h + 0x04, id, 8);
    std::memcpy(h + 0x1C, id + 0x18, 0x24);
    return holder;
}

void assign_record_name(std::byte* record, const std::byte* wide, SessionRecordCalls& host) {
    std::uint32_t units = 0;
    while (read_field<std::uint16_t>(wide, units * 2) != 0) ++units;
    host.wstring_assign_0067d5a0(record + 0x08, reinterpret_cast<const char16_t*>(wide), units);
}

void copy_qos_figures(std::byte* record, const std::byte* info) {
    if (info == nullptr) return;
    write_field<std::uint32_t>(record, 0x30, read_field<std::uint16_t>(info, 0x0E));
    write_field<std::uint32_t>(record, 0x38, read_field<std::uint32_t>(info, 0x10));
    write_field<std::uint32_t>(record, 0x3C, read_field<std::uint32_t>(info, 0x14));
}

void delete_result_objects(void* system, SessionRecordCalls& host) {
    const auto first = read_field<std::uintptr_t>(system, 0x178);
    if (first != 0) {
        const auto count = (read_field<std::uintptr_t>(system, 0x17C) - first) / 0xA0;
        for (std::uintptr_t i = 0; i < count; ++i) {
            const auto record = read_field<std::uintptr_t>(system, 0x178) + i * 0xA0;
            if (void* object = *reinterpret_cast<void**>(record)) host.delete_result_object(object);
        }
    }
}
} // namespace

void refresh_session_records_00a48450(void* system, SessionRecordCalls& host, XLiveSearchCalls& search) {
    auto* const base = static_cast<std::byte*>(system);
    delete_result_objects(system, host);
    host.result_vector_erase_all_00573490(base + 0x174);
    alignas(8) std::byte record[0xA0] = {};
    if (read_field<std::uint32_t>(system, 0x1B74) == 4 && read_field<std::uint32_t>(system, 0x1B60) != 0) {
        std::byte* const entry = base + 0x1AE8;
        reset_record_name(record);
        host.session_record_init_005707a0(record);
        write_field<std::uint8_t>(record, 4, 0xFF);
        write_field<void*>(record, 0, make_session_info_holder(entry + 0x26, host));
        assign_record_name(record, entry, host);
        write_field<std::uint32_t>(record, 0x24, read_field<std::uint32_t>(entry, 0x22));
        copy_qos_figures(record, read_field<const std::byte*>(entry, 0x72));
        host.result_vector_push_back_00575150(base + 0x174, record);
        host_qos_cancel_00a4afd0(entry, search);
        release_record_name(record, host);
        write_field<std::uint8_t>(system, 0xED, 0);
        return;
    }
    const std::uint32_t count = read_field<std::uint32_t>(system, 0x198C);
    for (std::uint32_t i = 0; i < count; ++i) {
        // EBP = the result's XNKID (+A6h); the result itself starts at +184h.
        const std::byte* const result = base + 0x184 + i * 0xF6;
        reset_record_name(record); // 00A486C0..00A486CA, every iteration
        host.session_record_init_005707a0(record);
        write_field<std::uint8_t>(record, 4, 0xFF);
        write_field<void*>(record, 0, make_session_info_holder(result + 0xA6, host));
        write_field<std::uint32_t>(record, 0x28,
            read_field<std::uint32_t>(result, 0xEE) + read_field<std::uint32_t>(result, 0xEA));
        write_field<std::uint32_t>(record, 0x2C, read_field<std::uint32_t>(result, 0x10));
        assign_record_name(record, result + 0x20, host);
        write_field<std::uint32_t>(record, 0x24, read_field<std::uint32_t>(result, 0x42));
        write_field<std::uint8_t>(record, 0x49, read_field<std::uint8_t>(result, 0x46));
        write_field<std::uint8_t>(record, 0x48, read_field<std::uint8_t>(result, 0x4A));
        write_field<std::uint64_t>(record, 0x88, read_field<std::uint64_t>(result, 0x8E));
        write_field<std::uint32_t>(record, 0x94, read_field<std::uint32_t>(result, 0x52));
        write_field<std::uint32_t>(record, 0x90, read_field<std::uint32_t>(result, 0x4E));
        write_field<std::uint64_t>(record, 0x98, read_field<std::uint64_t>(result, 0x9E));
        std::memcpy(record + 0x4C, result + 0x56, 14 * 4);
        copy_qos_figures(record, read_field<const std::byte*>(result, 0xF2));
        host.result_vector_push_back_00575150(base + 0x174, record);
        release_record_name(record, host);
        reset_record_name(record);
    }
    matchmaking_reset_00a4b250(base + 0x184, search);
    write_field<std::uint8_t>(system, 0xED, 0);
}

void system_update_00a488d0(void* system, float dt, SystemUpdateCalls& host, SessionRecordCalls& records) {
    auto* const base = static_cast<std::byte*>(system);
    void* const matchmaking = base + 0x184;
    void* const qos = base + 0x1AE8;
    const bool lan = host.system_virtual_19c(system);
    if (read_field<std::uint8_t>(system, 0x220E) != 0 && read_field<std::uint32_t>(system, 0x21F0) != 2) {
        host.session_delete_00a4adc0(base + 0x2098);
        write_field<std::uint8_t>(system, 0x220E, 0);
    }
    const auto host_qos_used = [&] { return read_field<std::uint8_t>(system, 0xEF) != 0; };
    const auto mm_state = [&] { return read_field<std::uint32_t>(system, 0x1AC0); };
    const auto qos_state = [&] { return read_field<std::uint32_t>(system, 0x1B74); };
    if (read_field<std::uint8_t>(system, 0xED) != 0 && !lan) {
        if (host_qos_used() ? qos_state() == 4 : mm_state() == 4) write_field<std::uint8_t>(system, 0xED, 0);
    }
    if (read_field<std::uint8_t>(system, 0xF0) == 0 && read_field<std::uint8_t>(system, 0xED) == 0 && !lan &&
        read_field<std::uint8_t>(system, 0xEE) != 0) {
        if (host_qos_used() ? qos_state() == 4 : mm_state() == 0x0C) {
            write_field<std::uint8_t>(system, 0xEE, 0);
            host.refresh_session_records_00a48450(system);
            void* const menu = host.multi_menu();
            if (menu != nullptr) {
                if (void* screen = read_field<void*>(menu, 0x4C)) host.browser_build_rows_005789c0(screen, 6);
            }
        }
    }
    const auto m = mm_state();
    const auto h = qos_state();
    if (m == 1 || m == 2 || m == 3 || h == 1 || h == 2 || h == 3) {
        const std::int32_t result = host_qos_used() ? host.host_qos_poll_00a4a4e0(qos)
                                                    : host.matchmaking_poll_00a49df0(matchmaking);
        if (result == 0) {
            if (read_field<std::uint8_t>(system, 0xEE) == 0 && read_field<std::uint8_t>(system, 0xED) != 0) {
                if (!host_qos_used()) {
                    if (read_field<std::uint8_t>(system, 0xF0) == 0) host.matchmaking_relink_00a49d20(matchmaking);
                } else {
                    host.host_qos_relink_00a4a420(qos);
                }
                write_field<std::uint8_t>(system, 0xEE, 1);
            }
            host.refresh_session_records_00a48450(system);
        } else if (result < 0) {
            if (host_qos_used()) host.host_qos_cancel_00a4afd0(qos);
            else host.matchmaking_reset_00a4b250(matchmaking);
            write_field<std::uint8_t>(system, 0xED, 0);
        }
    } else if (read_field<std::uint8_t>(system, 0xED) != 0 && lan &&
               read_field<std::uint32_t>(read_field<void*>(system, 0x2210), 0x1FE4) == 2) {
        void* const vector = base + 0x174;
        for (std::uint32_t i = 0; i < host.result_vector_size_00566310(vector); ++i) {
            if (void* object = read_field<void*>(host.result_vector_at_005663b0(vector, i), 0))
                records.delete_result_object(object);
        }
        host.result_vector_clear_00573c90(vector);
        void* const client = read_field<void*>(host.game_setup(), 0x207C);
        const std::uint32_t servers = host.lan_server_count(client);
        for (std::uint32_t index = 0; index < servers; ++index) {
            const auto* const info = static_cast<const char*>(host.lan_server_info(client, index));
            alignas(8) std::byte record[0xA0] = {};
            records.session_record_construct_00571e50(record);
            auto c = [&](std::size_t o) { return static_cast<std::int32_t>(static_cast<signed char>(info[o])); };
            if (c(0x2A) - c(0x29) - c(0x28) + c(0x27) > 0) {
                char16_t name[0x32] = {};
                records.multi_byte_to_wide(info, static_cast<std::int32_t>(std::strlen(info)), name, 0x32);
                records.wstring_assign_z_0067d6b0(record + 0x08, name);
                write_field<std::int32_t>(record, 0x24, c(0x25));
                write_field<std::uint8_t>(record, 0x04, static_cast<std::uint8_t>(index));
                write_field<void*>(record, 0x00, nullptr);
                write_field<std::int32_t>(record, 0x28, c(0x28));
                write_field<std::int32_t>(record, 0x2C, c(0x27));
                write_field<std::uint32_t>(record, 0x30, 0);
                write_field<std::uint8_t>(record, 0x48, static_cast<std::uint8_t>(info[0x2D]));
                write_field<std::uint8_t>(record, 0x49, static_cast<std::uint8_t>(info[0x2C]));
                std::memcpy(record + 0x88, info + 0x70, 0x18);
                std::memcpy(record + 0x4C, info + 0x30, 14 * 4);
                write_field<std::uint32_t>(record, 0x3C, 0x7D0000);
                write_field<std::uint32_t>(record, 0x38, 0x7D0000);
                records.result_vector_push_back_00575150(vector, record);
            }
            release_record_name(record, records);
        }
    }
    if (read_field<std::uint8_t>(system, 0x4C) != 0) {
        void* const setup = host.game_setup();
        auto* const client = read_field<std::byte*>(setup, 0x207C);
        if (read_field<std::uint32_t>(client, 0x9C) == 6 &&
            host.client_poll_create_result_00a39750(client + 0xC8) == 0x217) {
            void* const menu = host.multi_menu();
            host.game_1ef0_0076a7a0(static_cast<std::byte*>(setup) + 0x1EF0,
                read_field<std::uint32_t>(read_field<void*>(menu, 0x4C), 0x24));
            host.multi_menu_push_request_00688ad0(host.multi_menu(), 0x1A);
            write_field<std::uint8_t>(system, 0x4C, 0);
            return;
        }
    }
    if (!host.system_virtual_40(system)) host.system_virtual_44(system);
    host.session_poll_overlapped_00a4b1f0(base + 0x2098);
    const auto session_state = read_field<std::uint32_t>(system, 0x21F0);
    if ((session_state == 4 || session_state == 2) && read_field<std::uint8_t>(system, 0x220D) == 0)
        write_field<std::uint8_t>(system, 0x220D, 1);
    if (host.singleton_virtual_198()) {
        void* const xenon = host.xenon_system();
        void* const setup = host.game_setup();
        const auto upload = read_field<std::uint32_t>(xenon, 0x12C);
        if (upload == 4) {
            const auto a = (std::max)(read_field<std::uint32_t>(xenon, 0x358), read_field<std::uint32_t>(setup, 0x73C));
            const auto b = (std::max)(read_field<std::uint32_t>(xenon, 0x35C), read_field<std::uint32_t>(setup, 0x740));
            write_field<std::uint32_t>(setup, 0x73C, a);
            write_field<std::uint32_t>(setup, 0x740, b);
            write_field<std::uint32_t>(xenon, 0x12C, 1);
            if (read_field<std::uint32_t>(xenon, 0x358) != a || read_field<std::uint32_t>(xenon, 0x35C) != b)
                host.xenon_request_upload_00a3f4a0(xenon, a, b);
        } else if (upload == 0x0A) {
            write_field<std::uint32_t>(setup, 0x73C, 0);
            write_field<std::uint32_t>(setup, 0x740, 0);
            host.xenon_request_upload_00a3f4a0(xenon, 0, 0);
        }
    }
    if (read_field<std::uint32_t>(system, 0x1D28) != 0) host.update_friends_and_voice_00a46480(system, dt);
}

void write_session_stats_00a47b80(void* system, SessionStatsCalls& host) {
    auto* const base = static_cast<std::byte*>(system);
    write_field<float>(system, 0x88, -1.0f);
    if (host.system_virtual_19c(system)) return;
    write_field<float>(system, 0x88, 60.0f);
    void* const setup = host.game_setup();
    const bool send_message = read_field<std::uint32_t>(setup, 0x1FE4) == 1;
    const std::uint8_t ranked = read_field<std::uint8_t>(system, 0x4D);
    const std::uint32_t value_48 = read_field<std::uint32_t>(system, 0x48);
    void* const client = read_field<void*>(setup, 0x207C);
    const std::uint32_t id = client != nullptr ? read_field<std::uint32_t>(client, 0x9CC)
                                               : read_field<std::uint32_t>(system, 0x21C4);
    if (id == 0xFFFFFFFFu || id == 0) return;
    alignas(4) std::byte vector[0x10] = {};
    void* const singleton = host.singleton();
    if (read_field<std::uint8_t>(singleton, 0x74) == 0) host.stats_fill_from_game_00993220(singleton);
    host.stat_vector_assign_00a47350(vector, static_cast<std::byte*>(singleton) + 0x64);
    host.stat_vector_sort_00a47560(vector);
    auto* const first = read_field<std::byte*>(vector, 4);
    // An empty vector reaches the checked at(0) in 00992B30, whose range test
    // calls 00BF6713 (invalid parameter, no return); the C++ stops instead.
    if (first == nullptr) return;
    const std::uint32_t top = read_field<std::uint32_t>(first, 0x10);
    std::byte* const scratch = first;
    for (std::uint32_t i = 0;; ++i) {
        auto* const f = read_field<std::byte*>(vector, 4);
        if (f == nullptr) break;
        const auto count = static_cast<std::uint32_t>((read_field<std::byte*>(vector, 8) - f) / 0x50);
        if (i >= count) break;
        host.stat_record_assign_00992f60(scratch, f + i * 0x50);
        write_field<std::uint32_t>(scratch, 0x18, id);
        write_field<std::uint32_t>(scratch, 0x24, read_field<std::uint32_t>(scratch, 4));
        write_field<std::uint32_t>(scratch, 0x20, read_field<std::uint32_t>(scratch, 0));
        write_field<std::uint32_t>(scratch, 0x28, value_48);
        write_field<std::uint8_t>(scratch, 0x30, ranked);
        write_field<std::uint32_t>(scratch, 0x34, ranked != 0 ? count - i - 1 : 0);
        write_field<std::uint32_t>(scratch, 0x38, (read_field<std::uint32_t>(scratch, 0x10) != top ? 1u : 0u) + 1u);
        host.stats_list_push_back(base + 0x2200, scratch + 0x18);
    }
    if (send_message) host.send_stats_message(static_cast<std::byte*>(setup) + 0x1EF0, base + 0x2200);
    // 00A47E06..00A47E23: head->next must not be the head (00BF6713 check),
    // then its 0Eh-dword value.
    const auto* const head = read_field<const std::byte*>(system, 0x2204);
    const auto* const node = read_field<const std::byte*>(head, 0);
    std::uint32_t c[14];
    std::memcpy(c, node + 8, sizeof(c));
    const std::uint32_t arguments[12] = {id, c[2], c[3], value_48, c[5], c[6], c[10], c[8], c[7], 0, c[11], c[12]};
    host.write_session_stats_004c04c0(setup, arguments);
    if (auto* const f = read_field<std::byte*>(vector, 4)) {
        host.stat_vector_destroy_00993990(f, read_field<std::byte*>(vector, 8));
        host.free_00bf65ac(f);
    }
}
} // namespace bsp::online_cc13
