// XLive online system reconstructions, packet cc13-gui2 (docs/CC13_GUI_2.md, Part B).
#include "bsp/online_cc13.hpp"

#include <cstddef>
#include <cstring>
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
    static constexpr std::uint32_t kLocalModes[1] = {0};  // 00D24A34 (contents not read here)
    static constexpr std::uint32_t kRemoteModes[1] = {0}; // 00D24A30 (contents not read here)
    write_field<std::uint32_t>(voice, 0x114, read_field<std::uint32_t>(voice, 0x118));
    write_field<std::uint32_t>(voice, 0x118, xenon.active_user_slot());
    void* const engine = read_field<void*>(voice, 0x78);
    if (read_field<std::uint32_t>(voice, 0x108) == 0) xhv.register_local_talker_20(engine, 0);
    xhv.start_local_processing_0c(read_field<void*>(voice, 0x78), 0, kLocalModes, 1);
    xhv.start_remote_processing_10(read_field<void*>(voice, 0x78), 0, kRemoteModes, 1);
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
    const std::uint32_t flags = xhv.data_ready_flags_34(read_field<void*>(voice, 0x78), 0);
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
} // namespace bsp::online_cc13
