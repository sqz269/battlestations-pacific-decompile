// Segment 71 online reconstructions, packet cc13-online3 (docs/CC13_ONLINE_3.md).
#include "bsp/online_seg71_cc13.hpp"

#include <cstddef>
#include <cstring>

namespace bsp::online_seg71_cc13 {
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

std::byte* at(void* object, std::size_t offset) noexcept { return static_cast<std::byte*>(object) + offset; }

bool voice_active(const void* system) noexcept { return read_field<std::uint32_t>(system, 0x1D28) != 0; }

} // namespace

bool is_ethernet_link_up_00a42690(XLiveSlotCalls& host) { return host.xnet_get_ethernet_link_status() != 0; }
void* player_list_00a426a0(void* s, XLiveSlotCalls& host) {
    return host.system_virtual_19c(s) ? at(s, 0x10C) : at(s, 0x100);
}
void* lan_player_list_00a426c0(void* s) noexcept { return at(s, 0x10C); }
void* result_vector_00a426d0(void* s) noexcept { return at(s, 0x174); }
void* block_1f1c_00a428a0(void* s) noexcept { return at(s, 0x1F1C); }
void* list_168_00a429a0(void* s) noexcept { return at(s, 0x168); }
void nop_00a426e0() noexcept {}
void nop_00a426f0() noexcept {}
void nop_ret10_00a42980() noexcept {}
bool return_false_00a42700() noexcept { return false; }
bool return_false_00a42930() noexcept { return false; }
bool return_false_00a42860() noexcept { return false; }
bool return_false_00a42990() noexcept { return false; }

bool set_qos_listen_00a42710(void* s, std::uint8_t stop, XLiveSlotCalls& host) {
    if (!host.system_virtual_180(s)) return false;
    const bool listening_disabled = read_field<std::uint32_t>(s, 0x21C8) == 0;
    if (stop == 0) {
        if (listening_disabled) return false;
        host.system_virtual_54(s);
        const std::uint8_t* const title = host.qos_title_data();
        std::size_t units = 0;
        while (read_field<std::uint16_t>(title + 2, units * 2) != 0) ++units;
        host.session_start_qos_listen_00a49c10(at(s, 0x2098), title + 1, static_cast<std::uint32_t>(units * 2), 0);
        return true;
    }
    if (!listening_disabled) return false;
    host.session_stop_qos_listen_00a49c80(at(s, 0x2098));
    return true;
}

bool delete_session_00a427a0(void* s, XLiveSlotCalls& host) {
    if (read_field<std::uint8_t>(s, 0x85) != 0) host.system_virtual_58(s);
    host.session_delete_00a4adc0(at(s, 0x2098));
    write_field<std::uint8_t>(s, 0x220E, 0);
    return true;
}

void remove_remote_talker_00a427d0(void* s, std::uint32_t lo, std::uint32_t hi, XLiveSlotCalls& host) {
    if (voice_active(s)) host.voice_remove_remote_talker_00a49680(at(s, 0x1C24), lo, hi);
}
void set_talker_mute_list_flag_00a42800(void* s, std::uint32_t a, std::uint32_t b, std::uint8_t c,
    std::uint8_t d, XLiveSlotCalls& host) {
    if (voice_active(s)) host.voice_set_talker_mute_list_flag_00a49780(at(s, 0x1C24), a, b, c, d);
}
bool is_remote_talking_00a42830(void* s, std::uint32_t lo, std::uint32_t hi, std::uint8_t slot,
    XLiveSlotCalls& host) {
    if (!voice_active(s)) return false;
    return host.voice_is_remote_talking_00a49840(at(s, 0x1C24), lo, hi, slot) != 0;
}

std::uint8_t slot_flag_1c02_00a42870(const void* s, std::uint8_t slot) noexcept {
    if (slot >= 4 || read_field<std::uint8_t>(s, 0x1D4C + slot) == 0) return 0;
    return read_field<std::uint8_t>(s, 0x1C02 + slot);
}
void* voice_mask_record_00a428b0(void* s, std::uint8_t index) noexcept { return at(s, 0x1D5C + index * 0x1C); }
void players_dispatch_00a428d0(void* s, XLiveSlotCalls& host) {
    if (!host.system_virtual_40(s)) host.system_virtual_44(s);
}
std::uint8_t flag_2060_00a428f0(const void* s, std::uint8_t i) noexcept { return read_field<std::uint8_t>(s, 0x2060 + i); }
std::uint8_t flag_205c_00a42900(const void* s, std::uint8_t i) noexcept { return read_field<std::uint8_t>(s, 0x205C + i); }
void clear_flag_2060_00a42910(void* s, std::uint8_t i) noexcept { write_field<std::uint8_t>(s, 0x2060 + i, 0); }
void clear_flag_205c_00a42920(void* s, std::uint8_t i) noexcept { write_field<std::uint8_t>(s, 0x205C + i, 0); }
std::uint8_t slot_flag_1c12_00a42940(const void* s, std::uint8_t i) noexcept {
    return i < 4 ? read_field<std::uint8_t>(s, 0x1C12 + i) : 0;
}
void set_slot_flag_1c12_00a42960(void* s, std::uint32_t i, std::uint8_t value) noexcept {
    if (i < 4) write_field<std::uint8_t>(s, 0x1C12 + i, value);
}
void publish_user_property_00a429d0(XLiveSlotCalls& host) { host.publish_user_property_00a4a850(); }

namespace {
void* game_of(void* system) noexcept { return read_field<void*>(system, 0x2210); }
bool session_live(const void* system) noexcept {
    const auto state = read_field<std::uint32_t>(system, 0x21F0);
    return state == 4 || state == 2;
}
} // namespace

void shutdown_voice_00a42a30(void* s, XLiveSessionControlCalls& host) {
    if (voice_active(s)) host.voice_shutdown_00a49270(at(s, 0x1C24));
}

void step_countdown_00a42a50(void* s, float dt, XLiveSessionControlCalls& host) {
    if (read_field<std::uint8_t>(s, 0x85) == 0) return;
    const float current = read_field<float>(s, 0x88);
    if (!(current >= 0.0f)) return;
    const float next = current - dt;
    write_field<float>(s, 0x88, next);
    if (0.0f > next) host.system_virtual_58(s);
}

bool return_true_00a42aa0() noexcept { return true; }

void* stats_block_ensure_loaded_00a42f90(void* owner, XLiveSessionControlCalls& host) {
    if (read_field<std::uint8_t>(owner, 0x74) == 0) host.stats_fill_from_game_00993220(owner);
    return at(owner, 0x64);
}

void* voice_change_message_construct_00a42fb0(void* m, std::uint32_t a, std::uint32_t b, std::uint8_t c,
    XLiveSessionControlCalls& host) {
    host.session_message_construct_base_0075b430(m, 0xE8);
    write_field<std::uint8_t>(m, 0x18, c);
    write_field<std::uint32_t>(m, 0x04, 1);
    write_field<std::uint32_t>(m, 0x00, 0x00D03278u);
    write_field<std::uint32_t>(m, 0x20, a);
    write_field<std::uint32_t>(m, 0x24, b);
    return m;
}

void* task_stub_pair_construct_00a43070(void* pair) noexcept {
    write_field<std::uint32_t>(pair, 0, 0x00D24764u);
    write_field<std::uint32_t>(pair, 4, 0x00D24764u);
    return pair;
}
void task_stub_pair_destruct_00a43080(void* pair) noexcept {
    // 00A48F00 has no effect besides AL = 1.
    write_field<std::uint32_t>(pair, 4, 0x00D24764u);
    write_field<std::uint32_t>(pair, 0, 0x00D24764u);
}

void refresh_mute_for_player_00a43130(void* s, std::uint32_t lo, std::uint32_t hi, XLiveSessionControlCalls& host) {
    (void)s;
    std::uint32_t muted = lo; // the argument slot the image reuses as the output
    host.xuser_mute_list_query(host.xenon_active_user_slot_00a3eac0(), lo, hi, &muted);
    host.multiplayer_set_muted_00771630(at(host.game_setup(), 0x1EF0), lo, hi, muted != 0);
}

std::int32_t arbitration_register_00a43220(void* s, XLiveSessionControlCalls& host) {
    if (host.system_virtual_19c(s) || read_field<std::uint8_t>(s, 0x4D) == 0) return 0;
    void* const setup = host.game_setup();
    if (void* client = read_field<void*>(setup, 0x207C)) return host.client_arbitration_register_00a395e0(at(client, 0xC8));
    if (read_field<std::uint32_t>(setup, 0x2078) != 0) return host.session_arbitration_register_00a49a50(at(s, 0x2098));
    return static_cast<std::int32_t>(0x8000FFFFu);
}

bool cancel_session_search_00a43280(void* s, XLiveSessionControlCalls& host) {
    if (read_field<std::uint8_t>(s, 0xED) == 0) return false;
    if (host.system_virtual_19c(s)) {
        host.game_1ef0_0076a700(at(game_of(s), 0x1EF0));
    } else {
        const auto q = read_field<std::uint32_t>(s, 0x1B74);
        if (q == 1 || q == 2) host.host_qos_cancel_00a4afd0(at(s, 0x1AE8));
        const auto m = read_field<std::uint32_t>(s, 0x1AC0);
        if (m == 1 || m == 2) host.matchmaking_reset_00a4b250(at(s, 0x184));
    }
    write_field<std::uint8_t>(s, 0xED, 0);
    return true;
}

void nop_00a43300() noexcept {}

void clear_last_error_00a43310(void* s, std::uint8_t keep) noexcept {
    if (keep != 0) return;
    write_field<std::uint8_t>(s, 0x1BD5, 0);
    write_field<std::uint32_t>(s, 0x1BD8, 0);
}

bool host_create_session_00a43330(void* s, const void* block, XLiveSessionControlCalls& host) {
    void* const session = at(s, 0x2098);
    if (session_live(s) && !host.system_virtual_170(s, 0)) {
        while (read_field<std::uint32_t>(s, 0x21F0) == 2) host.session_poll_overlapped_00a4b1f0(session);
        host.system_virtual_170(s, 0);
    }
    write_field<std::uint32_t>(s, 0x2150, 0);
    write_field<std::uint32_t>(s, 0x2154, 0);
    host.session_info_fill_00a4bb00(session, block);
    const auto* const b = static_cast<const signed char*>(block);
    write_field<std::uint8_t>(s, 0x220C, b[0x27] - b[0x29] == 0 ? 1 : 0);
    const std::int32_t result = host.session_create_00a4ac40(session);
    write_field<std::uint8_t>(s, 0x220D, 0);
    if (result < 0) {
        host.session_terminate_xnet_00a4a790(session);
        write_field<std::int32_t>(s, 0x1BD8, result);
        write_field<std::uint8_t>(s, 0x1BD5, 0);
    }
    write_field<std::uint32_t>(s, 0x1C24, 1);
    return result >= 0;
}

namespace {
bool host_session_gate(void* s, XLiveSessionControlCalls& host) {
    return read_field<std::uint32_t>(game_of(s), 0x1FE4) == 1 && host.system_virtual_198(s);
}
bool host_refresh(void* s, XLiveSessionControlCalls& host) {
    if (!host_session_gate(s, host) || !session_live(s)) return false;
    host.session_info_fill_00a4bb00(at(s, 0x2098), at(game_of(s), 0x1FF0));
    return host.session_flag_restart_00a49a00(at(s, 0x2098)) >= 0;
}
} // namespace

bool host_refresh_session_info_00a433f0(void* s, XLiveSessionControlCalls& host) { return host_refresh(s, host); }
bool host_refresh_session_info_00a43460(void* s, XLiveSessionControlCalls& host) { return host_refresh(s, host); }
bool host_flag_session_00a434d0(void* s, XLiveSessionControlCalls& host) {
    if (!host_session_gate(s, host) || !session_live(s)) return false;
    return host.session_flag_restart_00a49a00(at(s, 0x2098)) >= 0;
}

std::int32_t restart_voice_engine_00a43520(void* s, XLiveSessionControlCalls& host) {
    (void)restart_voice_engine_00a435b0(s, host);
    return 0;
}
std::int32_t restart_voice_engine_00a435b0(void* s, XLiveSessionControlCalls& host) {
    if (voice_active(s)) host.voice_shutdown_00a49270(at(s, 0x1C24));
    (void)host.xenon_has_selected_user_00a3e510();
    return host.voice_initialize_00a493c0(at(s, 0x1C24));
}

std::uintptr_t current_session_info_00a43560(void* s, XLiveSessionControlCalls& host) {
    if (session_live(s)) return reinterpret_cast<std::uintptr_t>(at(s, 0x2150));
    void* const game = game_of(s);
    if (read_field<std::uint32_t>(game, 0x1FE4) == 2) {
        void* const client = read_field<void*>(game, 0x207C);
        if (read_field<std::int32_t>(client, 0x9C) >= 7)
            return static_cast<std::uintptr_t>(host.client_virtual_38(client)) + 4;
    }
    return reinterpret_cast<std::uintptr_t>(at(s, 0x8C));
}

void on_countdown_expired_00a435e0(void* s, XLiveSessionControlCalls& host) {
    if (read_field<std::uint8_t>(s, 0x85) == 0) return;
    write_field<std::uint8_t>(s, 0x85, 0);
    void* const setup = host.game_setup();
    const auto mode = read_field<std::uint32_t>(setup, 0x1FE4);
    if (mode == 0 || host.system_virtual_19c(s)) return;
    if (read_field<std::uint32_t>(host.game_setup(), 0x1FE4) == 1) {
        host.session_end_00a49b30(at(s, 0x2098));
        return;
    }
    void* const client = read_field<void*>(host.game_setup(), 0x207C);
    if (client != nullptr && read_field<std::int32_t>(client, 0x9C) >= 7)
        host.client_end_session_00a396b0(at(client, 0xC8));
}

void arm_countdown_00a43650(void* s, XLiveSessionControlCalls& host) {
    if (read_field<std::uint8_t>(s, 0x85) != 0) host.system_virtual_58(s);
    if (read_field<std::uint32_t>(host.game_setup(), 0x1FE4) == 0 || host.system_virtual_19c(s)) return;
    if (read_field<std::uint32_t>(host.game_setup(), 0x1FE4) == 1)
        host.session_start_00a49b00(at(s, 0x2098));
    else
        host.client_start_session_00a39690(at(read_field<void*>(host.game_setup(), 0x207C), 0xC8));
    write_field<std::uint8_t>(s, 0x85, 1);
    write_field<float>(s, 0x88, -1.0f);
}

void adjustor_00a410f0(void* self, MNetworkCalls& host) {
    host.mnetwork_queue_incoming_peer_00785790(static_cast<std::byte*>(self) - 0xA4);
}
void adjustor_00a41210(void* self, MNetworkCalls& host) {
    host.mnetwork_receive_packet_007858c0(static_cast<std::byte*>(self) - 0xC8);
}
void adjustor_00a41250(void* self, MNetworkCalls& host) {
    host.mnetwork_receive_packet_007858c0(static_cast<std::byte*>(self) - 0xA4);
}
void begin_lan_search_00a41100(void* c) noexcept {
    write_field<std::uint8_t>(c, 0x941, 1);
    write_field<std::uint32_t>(c, 0x944, 0);
}
void end_lan_search_00a41120(void* c) noexcept { write_field<std::uint8_t>(c, 0x941, 0); }
void client_write_local_address_00a41140(const void* c, void* out) noexcept {
    const auto* const src = static_cast<const std::byte*>(c);
    auto* const dst = static_cast<std::byte*>(out);
    std::memcpy(dst + 0x5A, src + 0x81C, 0x24);
    std::memcpy(dst + 0x7E, src + 0x840, 8);
    std::memcpy(dst + 0x86, src + 0x848, 16);
}
bool xnaddr_equals_00a411a0(const void* a, const void* b) noexcept {
    return std::memcmp(static_cast<const std::byte*>(a) + 0x24, static_cast<const std::byte*>(b) + 0x24, 8) == 0;
}
void client_on_disconnected_00a41220(void* sub) noexcept {
    write_field<std::uint32_t>(static_cast<std::byte*>(sub) - 0x2C, 0, 4);
    write_field<std::uint8_t>(sub, 0x81A, 1);
    write_field<std::uint8_t>(sub, 0x880, 0);
}
void server_update_00a41260(void* self, float dt, MNetworkCalls& host) { host.server_update_00a3d6f0(self, dt); }
void client_update_00a41270(void* self, float dt, MNetworkCalls& host) { host.client_update_00a39d10(self, dt); }
void client_stop_receive_00a41280(void* self, MNetworkCalls& host) { host.object_virtual_arg(at(self, 0xC8), 0x0C, 0); }
void start_hosting_and_set_state2_00a41300(void* self, MNetworkCalls& host) {
    host.object_virtual(self, 0x28);
    write_field<std::uint32_t>(self, 0x9C, 2);
}
bool return_true_00a41320() noexcept { return true; }
void server_xlive_destruct_00a41360(void* self, MNetworkCalls& host) {
    write_field<std::uint32_t>(self, 0, 0x00D243ECu);
    write_field<std::uint32_t>(self, 0xA4, 0x00D243C0u);
    host.server_destruct_00a3dcf0(at(self, 0xA4));
    write_field<std::uint32_t>(self, 0, 0x00D24390u);
    host.mnetwork_destruct_007844e0(self);
}
void forward_slot10_00a413d0(void* sub, MNetworkCalls& host) { host.object_virtual(read_field<void*>(static_cast<std::byte*>(sub) - 0xA0, 0), 0x10); }
void forward_slot14_00a413e0(void* sub, MNetworkCalls& host) { host.object_virtual(read_field<void*>(static_cast<std::byte*>(sub) - 0xA0, 0), 0x14); }
void forward_slot08_00a413f0(void* sub, MNetworkCalls& host) { host.object_virtual(read_field<void*>(static_cast<std::byte*>(sub) - 0xA0, 0), 0x08); }
void server_disconnect_peer_00a41400(void* self, void* peer, MNetworkCalls& host) {
    host.request_disconnect_00a3bbb0(at(peer, 0xD74), 2.0f);
    host.object_virtual_arg(self, 0x08, reinterpret_cast<std::uintptr_t>(peer));
}
float server_peer_latency_00a41440(void* peer, MNetworkCalls& host) {
    if (peer == nullptr) return 1.0f;
    void* slot = peer;
    return host.estimate_latency_00a3bda0(at(peer, 0xD74), &slot);
}
float server_peer_profile_value_00a41470(void* peer, MNetworkCalls& host) {
    return peer == nullptr ? 1.0f : host.profile_value_00a3be60(at(peer, 0xD74));
}
std::uint32_t server_peer_pending_count_00a414a0(void* peer, MNetworkCalls& host) {
    return peer == nullptr ? 100u : host.pending_count_00a3bee0(at(peer, 0xD74));
}
void* client_xlive_construct_00a414d0(void* self, std::uint32_t argument, MNetworkCalls& host) {
    host.mnetwork_client_construct_00785b80(self, argument);
    host.client_construct_00a39570(at(self, 0xC8));
    write_field<std::uint32_t>(self, 0, 0x00D24448u);
    write_field<std::uint32_t>(self, 0xC8, 0x00D2441Cu);
    write_field<std::uint32_t>(self, 0x9D8, 0x00CEED04u);
    void* target = host.operator_new_00bf681b(0xDC0);
    if (target != nullptr) {
        host.session_target_construct_007839d0(target, 0, 0);
        write_field<std::uint32_t>(target, 0, 0x00D03860u);
    }
    write_field<void*>(self, 0x93C, target);
    host.mnetwork_append_peer_00785c10(self, target);
    return self;
}
void* client_host_info_00a41590(void* self) noexcept { return at(self, 0x9D8); }
void client_set_state2_00a415a0(void* sub) noexcept { write_field<std::uint32_t>(static_cast<std::byte*>(sub) - 0xB4, 0, 2); }
void client_xlive_destruct_00a415b0(void* self, MNetworkCalls& host) {
    write_field<std::uint32_t>(self, 0, 0x00D24448u);
    write_field<std::uint32_t>(self, 0xC8, 0x00D2441Cu);
    host.mnetwork_client_clear_host_lists_00785170(self);
    write_field<std::uint8_t>(self, 0x948, 0);
    if (read_field<std::uint8_t>(self, 0x961) != 0) {
        host.xnet_unregister_key(at(self, 0x959));
        write_field<std::uint8_t>(self, 0x961, 0);
    }
    write_field<std::uint32_t>(self, 0x9D8, 0x00CEED00u);
    host.client_destruct_00a39e30(at(self, 0xC8));
    host.mnetwork_client_destruct_00784d60(self);
}
void client_xlive_open_00a41660(void* self, MNetworkCalls& host) {
    host.client_open_00a39f70(at(self, 0xC8), 1000);
    write_field<std::uint32_t>(self, 0x14, 1);
    void* const singleton = host.xlive_singleton();
    if (singleton != nullptr && host.object_virtual_al(singleton, 0x198) != 0) {
        const std::uint8_t value = host.object_virtual_al(host.xlive_singleton(), 0x30);
        write_field<std::uint8_t>(host.game_setup(), 0x6A9, value);
    }
}

void* host_info_assign_00a41880(void* holder, const void* source) noexcept {
    std::memcpy(at(holder, 4), static_cast<const std::byte*>(source) + 4, 0x84);
    return holder;
}

void client_xlive_disconnect_00a41900(void* self, MNetworkLifetimeCalls& host) {
    host.log("MNetworkClientXLive::DisconnectFromServerInner()");
    host.log("Client send MquiteMsg and Flush queue, Disconnecet low level");
    host.client_disconnect_from_server_00a39dc0(at(self, 0xC8));
    write_field<std::uint32_t>(self, 0x14, 1);
    if (read_field<std::uint8_t>(self, 0x948) != 0 && read_field<std::uint8_t>(self, 0x961) != 0) {
        std::uint8_t key[8];
        std::memcpy(key, at(self, 0x959), 8);
        host.log_key("%x%x%x%x%x%x%x%x Unregister Key by MNetworkClientXLive::DisconnectFromServerInner()", key);
        host.xnet_unregister_key(at(self, 0x959));
        write_field<std::uint8_t>(self, 0x961, 0);
    }
}

void client_xlive_create_00a41c70(std::uint32_t argument, void** out, MNetworkLifetimeCalls& host) {
    void* object = host.operator_new_00bf681b(0xA60);
    if (object != nullptr) host.client_xlive_construct_00a414d0(object, argument);
    *out = object;
    host.object_virtual(object, 0x04);
}
void server_xlive_create_00a42170(std::uint32_t argument, void** out, MNetworkLifetimeCalls& host) {
    void* object = host.operator_new_00bf681b(0x920);
    if (object != nullptr) host.server_xlive_construct_00a41ce0(object, argument);
    *out = object;
    host.object_virtual(object, 0x04);
}

void* server_xlive_construct_00a41ce0(void* self, std::uint32_t argument, MNetworkLifetimeCalls& host) {
    host.mnetwork_construct_00784f90(self, argument);
    write_field<std::uint32_t>(self, 0, 0x00D24390u);
    if (read_field<std::uint32_t>(self, 0x9C) != 1) write_field<std::uint32_t>(self, 0x9C, 1);
    write_field<std::uint8_t>(self, 0xA0, 0);
    host.server_construct_00a3dc80(at(self, 0xA4));
    write_field<std::uint32_t>(self, 0xA4, 0x00D243C0u);
    write_field<std::uint32_t>(self, 0, 0x00D243ECu);
    return self;
}

void server_xlive_remove_peer_00a422c0(void* self, std::uint32_t peer, MNetworkLifetimeCalls& host) {
    host.int_list_remove_value_00a421e0(at(self, 0x90C), &peer);
    host.mnetwork_queue_peer_removal_007857f0(self, peer);
}

void client_post_peer_address_message_00a42480(void* console, std::uint32_t a, std::uint32_t b,
    const void* xnkid, const void* xnkey, const void* xnaddr, MNetworkLifetimeCalls& host) {
    void* message = host.operator_new_00bf681b(0x64);
    if (message != nullptr) {
        write_field<std::uint32_t>(message, 0x18, 0);
        write_field<std::uint32_t>(message, 0x1C, 0x0F);
        write_field<std::uint8_t>(message, 0x08, 0);
        write_field<std::uint32_t>(message, 0x20, 0);
        write_field<std::uint32_t>(message, 0x00, 0x00D24674u);
    }
    std::memcpy(at(message, 0x24), xnkid, 8);
    std::memcpy(at(message, 0x2C), xnkey, 16);
    std::memcpy(at(message, 0x3C), xnaddr, 0x24);
    host.mnetwork_client_push_pending_host_00785ee0(static_cast<std::byte*>(console) - 0xC8, message);
    host.lan_host_record_set_blob_00783540(message, a, b);
}

void* task_stub_construct_00a42510(void* stub) noexcept {
    write_field<std::uint32_t>(stub, 0, 0x00D24764u);
    return stub;
}

namespace {
struct SockaddrIn {
    std::uint16_t family;
    std::uint16_t port;
    std::uint32_t address;
    std::uint8_t zero[8];
};

} // namespace

bool client_connect_to_server_inner_00a416b0(void* self, const void* message, MNetworkConnectCalls& host) {
    if (read_field<std::uint8_t>(self, 0x948) != 0) {
        if (read_field<std::uint8_t>(self, 0x961) != 0) {
            host.log_pointer("%x%x%x%x%x%x%x%x Unreg key MNetworkClientXLive::ConnectToServerInner", at(self, 0x959));
            host.xnet_unregister_key(at(self, 0x959));
            write_field<std::uint8_t>(self, 0x961, 0);
        }
        write_field<std::uint8_t>(self, 0x948, 0);
    }
    const auto* const m = static_cast<const std::uint8_t*>(message);
    (void)host.xnet_register_key(m + 0x24, m + 0x2C);
    std::uint32_t ina = 0;
    if (host.xnet_xnaddr_to_inaddr(m + 0x3C, m + 0x24, &ina) != 0) {
        host.log_address("NETWORK ConnectToServerInner To %d.%d.%d.%d   XnAddr: %02X:%02X:%02X:%02X:%02X:%02X",
            m + 0x3C, m + 0x46);
        return false;
    }
    write_field<std::uint8_t>(self, 0x948, 1);
    std::memcpy(at(self, 0x959), m + 0x24, 8);
    write_field<std::uint8_t>(self, 0x961, 1);
    std::uint8_t key[8];
    std::memcpy(key, at(self, 0x959), 8);
    host.log_key("%x%x%x%x%x%x%x%x RegiseterKey MNetworkClientXLive::ConnectToServerInner", key);
    void* buffer = nullptr;
    {
        SockaddrIn address{2, host.xsocket_htons(1000), ina, {}};
        host.client_set_server_address_00a39d40(at(self, 0xC8), &address);
        std::uint32_t length = 0;
        host.build_connect_message(read_field<void*>(self, 4), &buffer, &length);
        host.client_send_connect_00a39870(at(self, 0xC8), buffer, length);
    }
    host.object_virtual(self, 0x30);
    host.free_00bf6989(buffer);
    void* const client = read_field<void*>(host.game_setup(), 0x207C);
    if (read_field<std::uint32_t>(client, 0x9C) != 6) write_field<std::uint32_t>(client, 0x9C, 6);
    return true;
}

bool client_connect_to_server_00a42010(void* self, const void* host_info, MNetworkConnectCalls& host) {
    host.log("MNetworkClientXLive::ConnectToServerInner");
    host_info_assign_00a41880(at(self, 0x9D8), host_info);
    write_field<std::uint8_t>(self, 0x961, 0);
    std::uint32_t ina = 0;
    const std::int32_t failed = host.xnet_xnaddr_to_inaddr(at(self, 0x9F4), at(self, 0x9DC), &ina);
    std::memcpy(at(self, 0x908), at(self, 0x9DC), 8);
    if (failed != 0) return false;
    write_field<std::uint8_t>(self, 0x948, 1);
    SockaddrIn address{2, host.xsocket_htons(1000), ina, {}};
    host.client_set_server_address_00a39d40(at(self, 0xC8), &address);
    void* buffer = nullptr;
    std::uint32_t length = 0;
    host.build_connect_message(read_field<void*>(self, 4), &buffer, &length);
    host.client_send_connect_00a39870(at(self, 0xC8), buffer, length);
    host.free_00bf6989(buffer);
    return true;
}

std::uint32_t task_stub_return_zero_00a42520() noexcept { return 0; }

namespace {
// _strncpy semantics: copy up to n bytes, zero-padding after a short source.
void strncpy_bytes(std::byte* out, const char* text, std::size_t n) noexcept {
    std::size_t i = 0;
    for (; i < n && text[i] != '\0'; ++i) out[i] = static_cast<std::byte>(text[i]);
    for (; i < n; ++i) out[i] = std::byte{0};
}
} // namespace

void server_xlive_start_hosting_00a41d80(void* self, MNetworkHostingCalls& host) {
    host.server_open_00a3d480(at(self, 0xA4), 8, 1000);
    auto* const info = static_cast<std::byte*>(host.operator_new_00bf681b(0x98));
    void* const setup = host.game_setup();
    const void* string = at(setup, 0x6A0);
    if (read_field<std::uint32_t>(string, 0) == 0) string = at(setup, 0x68C);
    const char* name = read_field<const char*>(string, 4);
    strncpy_bytes(info, name != nullptr ? name : "", 0x20);
    std::uint32_t version_length = 0;
    const char* version = host.version_text_00f88a38(&version_length);
    const std::uint32_t count = version_length > 4 ? 5 : version_length;
    strncpy_bytes(info + 0x20, version != nullptr ? version : "", count);
    info[0x20 + count] = std::byte{0};
    info[0x1F] = std::byte{0};
    write_field<std::uint64_t>(info, 0x28, read_field<std::uint64_t>(setup, 0x698));
    write_field<std::uint8_t>(info, 0x59, read_field<std::uint8_t>(setup, 0x6A9));
    void* const singleton = host.xlive_singleton();
    if (singleton != nullptr && host.object_virtual_al(singleton, 0x198) != 0) {
        write_field<std::uint8_t>(info, 0x59, host.object_virtual_al(host.xlive_singleton(), 0x30));
        write_field<std::uint8_t>(host.game_setup(), 0x6A9, host.object_virtual_al(host.xlive_singleton(), 0x30));
    }
    write_field<std::uint8_t>(info, 0x58, 0);
    void* const xenon = host.xenon_system();
    const auto a = read_field<std::uint32_t>(setup, 0x73C);
    const auto b = read_field<std::uint32_t>(xenon, 0x358);
    write_field<std::uint32_t>(info, 0x50, a <= b ? b : a);
    const auto c = read_field<std::uint32_t>(setup, 0x740);
    const auto d = read_field<std::uint32_t>(xenon, 0x35C);
    write_field<std::uint32_t>(info, 0x54, c <= d ? d : c);
    strncpy_bytes(info + 0x32, host.language_name_008d4870(), 0x10);
    write_field<std::uint64_t>(info, 0x48, read_field<std::uint64_t>(setup, 0x728));
    void* target = host.operator_new_00bf681b(0xDC0);
    if (target != nullptr) {
        host.session_target_construct_007839d0(target, info, 0x98);
        write_field<std::uint32_t>(target, 0, 0x00D03860u);
    }
    std::uint32_t ina = 0;
    host.xnet_xnaddr_to_inaddr(at(self, 0x8C0), at(self, 0x8E4), &ina);
    const std::uint16_t port = host.xsocket_htons(1000);
    write_field<std::uint16_t>(target, 0xD74, 2);
    write_field<std::uint16_t>(target, 0xD76, port);
    write_field<std::uint32_t>(target, 0xD78, ina);
    write_field<std::uint64_t>(target, 0xD7C, 0);
    host.mnetwork_queue_incoming_peer_00785790(self, target);
}

void server_xlive_resolve_packet_source_00a422f0(void* server, std::uintptr_t peer, MNetworkHostingCalls& host) {
    void* const outer = static_cast<std::byte*>(server) - 0xA4;
    if (peer != 0) {
        host.outer_virtual_08(outer, host.server_virtual_04(server, peer));
        return;
    }
    auto* const head = read_field<std::byte*>(server, 0x86C);
    write_field<void*>(server, 0x874, at(server, 0x868));
    write_field<std::byte*>(server, 0x878, read_field<std::byte*>(head, 0));
    while (read_field<std::byte*>(server, 0x878) != read_field<std::byte*>(server, 0x86C)) {
        auto* const node = read_field<std::byte*>(server, 0x878);
        host.outer_virtual_08(outer, read_field<std::uintptr_t>(node, 8));
        write_field<std::byte*>(server, 0x878, read_field<std::byte*>(read_field<std::byte*>(server, 0x878), 0));
    }
    auto* const sentinel = read_field<std::byte*>(server, 0x86C);
    auto* node = read_field<std::byte*>(sentinel, 0);
    write_field<std::byte*>(sentinel, 0, sentinel);
    write_field<std::byte*>(sentinel, 4, sentinel);
    write_field<std::uint32_t>(server, 0x870, 0);
    while (node != sentinel) {
        auto* const next = read_field<std::byte*>(node, 0);
        host.free_00bf65ac(node);
        node = next;
    }
    write_field<std::uint8_t>(server, 0x81A, 1);
}

void console_base_handle_packet_00a39060(void* console, std::uint32_t a, std::int32_t type, std::uint32_t c,
    std::uint32_t d, NetworkConsoleCalls& host) {
    if (type >= 5 && type <= 6) host.console_virtual_08(console, a, c, d);
}

void* console_base_construct_00a390b0(void* c) noexcept {
    write_field<std::uint32_t>(c, 0, 0x00D23B88u);
    write_field<std::uint8_t>(c, 0x804, 0);
    write_field<std::uint8_t>(c, 0x818, 0);
    write_field<std::uint8_t>(c, 0x819, 0);
    write_field<std::uint8_t>(c, 0x81A, 0);
    return c;
}

void* console_construct_00a390e0(void* c) noexcept {
    console_base_construct_00a390b0(c);
    write_field<std::uint8_t>(c, 0x860, 0);
    write_field<std::uint8_t>(c, 0x861, 0);
    write_field<std::int32_t>(c, 0x858, -1);
    write_field<std::int32_t>(c, 0x85C, -1);
    write_field<std::uint32_t>(c, 0, 0x00D23BA4u);
    return c;
}

std::uint32_t console_is_ethernet_link_up_00a39110(NetworkConsoleCalls& host) {
    return host.xnet_get_ethernet_link_status() != 0 ? 1u : 0u;
}

void console_pump_receive_00a391a0(void* c, NetworkConsoleCalls& host) {
    if (read_field<std::uint8_t>(c, 0x818) == 0 || read_field<std::uint8_t>(c, 0x861) != 0) return;
    for (std::uint32_t channel = 0; channel < 2; ++channel) {
        for (;;) {
            std::uint32_t from = 0;
            std::uint8_t sockaddr[16] = {};
            const std::int32_t bytes = host.receive_packet_00a3c3c0(read_field<std::uint32_t>(c, 0x858), &from,
                sockaddr, at(c, 4), 0x800, channel);
            if (bytes < 0) break;
            if (channel == 0) write_field<std::uint8_t>(c, 0x819, 0);
            const std::uint32_t peer = host.console_resolve_04(c, sockaddr);
            if (peer == 0) std::memcpy(at(c, 0x808), sockaddr, 16);
            host.console_deliver_00(c, peer, from, at(c, 4), bytes);
        }
    }
}

void console_register_key_00a39480(void* c, NetworkConsoleCalls& host) {
    if (read_field<std::uint8_t>(c, 0x861) != 0 || read_field<std::uint8_t>(c, 0x860) == 0) return;
    if (host.xnet_create_key(at(c, 0x840), at(c, 0x848)) == 0) host.xnet_register_key(at(c, 0x840), at(c, 0x848));
}

void console_unregister_key_00a394c0(void* c, NetworkConsoleCalls& host) {
    if (read_field<std::uint8_t>(c, 0x861) == 0 && read_field<std::uint8_t>(c, 0x860) != 0)
        host.xnet_unregister_key(at(c, 0x840));
    write_field<std::uint8_t>(c, 0x860, 0);
    write_field<std::uint8_t>(c, 0x861, 0);
}

void console_close_sockets_00a39500(void* c, NetworkConsoleCalls& host) {
    const auto first = read_field<std::int32_t>(c, 0x858);
    if (first != -1) {
        host.reset_for_socket_00a3b870(static_cast<std::uint32_t>(first), 0);
        host.xsocket_close(static_cast<std::uint32_t>(read_field<std::int32_t>(c, 0x858)));
        write_field<std::int32_t>(c, 0x858, -1);
    }
    const auto second = read_field<std::int32_t>(c, 0x85C);
    if (second != -1) {
        host.xsocket_close(static_cast<std::uint32_t>(second));
        write_field<std::int32_t>(c, 0x85C, -1);
    }
}

void* client_construct_00a39570(void* c, ClientSessionCalls& host) {
    host.console_construct_00a390e0(c);
    write_field<std::uint8_t>(c, 0x878, 0);
    write_field<std::uint8_t>(c, 0x879, 0);
    write_field<std::uint32_t>(c, 0x87C, 0);
    write_field<std::uint8_t>(c, 0x819, 0);
    write_field<std::uint8_t>(c, 0x880, 0);
    write_field<std::uint8_t>(c, 0x90C, 0);
    write_field<std::uint32_t>(c, 0x908, 0);
    write_field<std::uint32_t>(c, 0x8A0, 0);
    write_field<std::uint32_t>(c, 0x904, 0);
    write_field<std::uint8_t>(c, 0x899, 0);
    write_field<std::uint32_t>(c, 0, 0x00D23BC0u);
    write_field<std::uint32_t>(c, 0x89C, 7);
    return c;
}

std::int32_t client_arbitration_register_00a395e0(void* c, ClientSessionCalls& host) {
    std::memset(at(c, 0x8E8), 0, 0x1C);
    void* const session = read_field<void*>(c, 0x904);
    const auto nonce = read_field<std::uint64_t>(c, 0x8E0);
    std::uint32_t bytes = 0;
    host.xsession_arbitration_register(session, 0, nonce, &bytes, nullptr, nullptr);
    void* const buffer = host.operator_new_00bf55be(bytes);
    std::int32_t result = 0;
    if (host.xsession_arbitration_register(read_field<void*>(c, 0x904), 0, read_field<std::uint64_t>(c, 0x8E0),
            &bytes, buffer, nullptr) != 0)
        result = static_cast<std::int32_t>(host.x_get_overlapped_extended_error(nullptr));
    host.free_00bf6989(buffer);
    return result;
}

std::uint32_t client_start_session_00a39690(void* c, ClientSessionCalls& host) {
    write_field<std::uint8_t>(c, 0x90C, 1);
    return host.xsession_start(read_field<void*>(c, 0x904), 0, nullptr);
}

std::uint32_t client_end_session_00a396b0(void* c, ClientSessionCalls& host) {
    void* const session = read_field<void*>(c, 0x904);
    if (session == nullptr || read_field<std::uint8_t>(c, 0x90D) != 0 || read_field<std::uint8_t>(c, 0x90C) == 0)
        return 0;
    write_field<std::uint8_t>(c, 0x90D, 1);
    return host.xsession_end(session, nullptr);
}

std::int32_t client_join_remote_player_00a396e0(void* c, const std::uint64_t* xuid, std::uint8_t invited,
    ClientSessionCalls& host) {
    host.log_join("  --  ONLINE  --  Client Join Remote Player %llx   %s", *xuid, invited != 0 ? "Invite" : " ");
    const std::uint32_t private_slot = invited != 0 ? 1u : 0u;
    if (host.xsession_join_remote(read_field<void*>(c, 0x904), 1, xuid, &private_slot, nullptr) != 0)
        return static_cast<std::int32_t>(host.x_get_overlapped_extended_error(nullptr));
    return 0;
}

std::int32_t client_poll_create_result_00a39750(void* c, ClientSessionCalls& host) {
    if (read_field<std::uint32_t>(c, 0x908) != 1 || read_field<std::uint32_t>(c, 0x8E8) == 0x3E5) return 0;
    void* const overlapped = at(c, 0x8E8);
    if (host.x_get_overlapped_extended_error(overlapped) != 0) {
        std::uint32_t transferred = 0;
        const std::uint32_t result = host.x_get_overlapped_result(overlapped, &transferred, 0);
        const std::uint32_t error = host.x_get_overlapped_extended_error(overlapped);
        host.log_hex3("__ ONLINE __ Client Session hiba %x %x %x", transferred, result, error);
        return static_cast<std::int32_t>(0x8000FFFFu);
    }
    write_field<std::uint32_t>(c, 0x908, 0);
    return 0x217;
}

void client_delete_session_00a397d0(void* c, ClientSessionCalls& host) {
    host.log("NETWORK: CLIENT --- SessionDelete");
    void* const session = read_field<void*>(c, 0x904);
    if (session == nullptr) return;
    if (read_field<std::uint8_t>(c, 0x90D) == 0 && read_field<std::uint8_t>(c, 0x90C) != 0) {
        host.log("  --  ONLINE  --  cNetworkClient  SessionEnd");
        write_field<std::uint8_t>(c, 0x90D, 1);
        host.xsession_end(session, nullptr);
    }
    if (read_field<std::uint8_t>(c, 0x90E) == 0) {
        host.xsession_delete(read_field<void*>(c, 0x904), nullptr);
        host.log("NETWORK: CLIENT --- SessionDeleted");
        write_field<std::uint8_t>(c, 0x90E, 1);
        host.x_close_handle(read_field<void*>(c, 0x904));
        write_field<void*>(c, 0x904, nullptr);
    }
    write_field<void*>(c, 0x904, nullptr);
}

float client_peer_latency_00a41ba0(void* peer, MNetworkCalls& host) { return server_peer_latency_00a41440(peer, host); }
float client_peer_profile_value_00a41bd0(void* peer, MNetworkCalls& host) { return server_peer_profile_value_00a41470(peer, host); }
std::uint32_t client_peer_pending_count_00a41c00(void* peer, MNetworkCalls& host) { return server_peer_pending_count_00a414a0(peer, host); }

} // namespace bsp::online_seg71_cc13
