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
} // namespace bsp::online_cc13
