#include "bsp/mp_seg43_cc13.hpp"

namespace bsp::cc13_seg43 {
void supply_state_message_write_0084ce80(Raw message, Raw stream, NbHighMessageCalls& calls) {
    auto* p = static_cast<std::uint8_t*>(message);
    calls.write_extended_header_0075b480(message, stream);
    calls.write_signed_00429090(stream, *reinterpret_cast<std::int32_t*>(p + 0x1c), 32);
    calls.write_bool_004290b0(stream, p[0x20]);
}
void supply_state_message_read_0084ceb0(Raw message, Raw stream, NbHighMessageCalls& calls) {
    auto* p = static_cast<std::uint8_t*>(message);
    calls.read_extended_header_0075b4c0(message, stream);
    auto* cursor = static_cast<std::uint8_t*>(stream) + 4;
    calls.read_signed_00428d30(cursor, reinterpret_cast<std::int32_t*>(p + 0x1c), 32);
    calls.read_bool_00428d70(cursor, p + 0x20);
}
bool supply_state_message_apply_0084d520(Raw receiver, const void* message, NbHighMessageCalls& calls) {
    const auto* p = static_cast<const std::uint8_t*>(message);
    if (p[0x10] != 0xbf) return false;
    const auto value = p[0x20];
    const auto key = *reinterpret_cast<const std::int32_t*>(p + 0x1c);
    calls.map_subscript_005c49c0(static_cast<std::uint8_t*>(receiver) + 0x310, &key) = value;
    return true;
}
void root_message_write_type_008dade0(Raw message, Raw stream, NbHighMessageCalls& calls) {
    calls.write_byte_00428ff0(stream, static_cast<std::uint8_t*>(message)[0x10], 8);
}
void root_message_read_type_008dae00(Raw message, Raw stream, NbHighMessageCalls& calls) {
    calls.read_byte_00428c70(static_cast<std::uint8_t*>(stream) + 4, static_cast<std::uint8_t*>(message) + 0x10, 8);
}
}

#include "bsp/native_string.hpp"

#include <cstring>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// Segment 43, packet cc13-seg43. Evidence, ABI and simplifications per function:
// docs/CC13_SEG43.md.
namespace bsp::cc13_seg43 {
namespace {

template <class T> T get(const void* p, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(p) + offset, sizeof value);
    return value;
}
template <class T> void put(void* p, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + offset, &value, sizeof value);
}
unsigned char* at(void* p, std::size_t offset) noexcept { return static_cast<unsigned char*>(p) + offset; }
const unsigned char* at(const void* p, std::size_t offset) noexcept {
    return static_cast<const unsigned char*>(p) + offset;
}

// Session layout (MMultiPlayer).
constexpr std::size_t kMode = 0xf4;          // 1 host, 2 client
constexpr std::size_t kServer = 0x188;       // server transport
constexpr std::size_t kClient = 0x18c;       // client transport
constexpr std::size_t kQueueData = 0x24c;    // receive queue {data, count}
constexpr std::size_t kQueueCount = 0x250;

// Session-message vtables (scalar deleting destructor in slot 0).
constexpr std::uint32_t kVtableMessage23 = 0x00D030E8; // MT_CHANGEMULTISETTINGS 17h
constexpr std::uint32_t kVtableMessage11 = 0x00D0314C; // MT_SCENELOAD 0Bh
constexpr std::uint32_t kVtableMessage77 = 0x00D02CB8; // 4Dh pending selection
constexpr std::uint32_t kVtableMessage32 = 0x00D03124; // MT_PLAYERSURRENDER 20h
constexpr std::uint32_t kVtableMessage25 = 0x00D03070; // 19h select player
constexpr std::uint32_t kVtableMessage30 = 0x00D030FC; // 1Eh slot AI enable
constexpr std::uint32_t kVtableMessage13 = 0x00D03034; // 0Dh incompatible clients
constexpr std::uint32_t kVtableSubScene = 0x00D036E4;  // MT_SUBSCENE 18h
constexpr std::uint32_t kVtableScoring = 0x00D035B8;   // MT_SCENESCORING 15h
constexpr std::uint32_t kVtableRoleOwner = 0x00D02C90; // MT_ROLEOWNER 4Bh
constexpr std::uint32_t kVtableEntityRelease = 0x00D02CCC; // 4Fh
constexpr std::uint32_t kVtableReconLevel = 0x00D02D1C;    // MT_RECONLEVEL 61h
constexpr std::uint32_t kVtableOptionalBuffer = 0x00D0385C;

constexpr float kFltMax = 3.40282346638528859812e+38f; // 00D7A248
constexpr float kOne = 1.0f;                            // 00D7A24C
constexpr double kReconHoldSeconds = 30.0;              // 00CE7630
constexpr const char* kReconLevel = "reconlevel";       // 00D03DA0
constexpr const char* kEmpty = "";                       // 00F1B03C

void set_message_header(void* message, std::uint32_t vtable) noexcept {
    put<std::uint32_t>(message, 4, 1);
    put<std::uint32_t>(message, 0, vtable);
}

// Inlined NativeString copy construction at `out` (see the seg29 note): zero the header, then
// unless aliasing, 0041DD40(source length, 1) and a _memcpy of the destination length.
void copy_construct_string(void* out, const void* source, NativeStringStorage& strings) {
    put<std::uint32_t>(out, 0, 0);
    put<std::uint32_t>(out, 4, 0);
    if (out == source) return;
    resize_native_string_header_0041dd40(out, strings, get<std::uint32_t>(source, 0), true);
    if (get<std::uint32_t>(source, 0) == 0) return;
    std::memcpy(get<char*>(out, 4), get<const char*>(source, 4), get<std::uint32_t>(out, 0));
}

// Walk the std::list whose head pointer is stored at `head_slot` (re-read every step, as the
// image does), calling f(node).
template <class F> void for_each_list_node(const void* head_slot, F f) {
    for (auto* node = get<std::uint8_t*>(get<std::uint8_t*>(head_slot, 0), 0);
         node != get<std::uint8_t*>(head_slot, 0); node = get<std::uint8_t*>(node, 0))
        f(node);
}

} // namespace

void* resolve_net_entity(std::uint32_t id, const EntityRegistryGlobals& r) noexcept {
    if (static_cast<std::int32_t>(id) < r.split_00f89a10)
        return get<void*>(r.low_table_00f89a54, (id - r.low_first_00f89a0c) * 0x10u + 0xc);
    return get<void*>(r.high_table_00f89aa8, (id - r.high_first_00f89a60) * 0x10u + 0xc);
}

void* session_message_resolve_entity(const void* message, const EntityRegistryGlobals& registry) noexcept {
    return resolve_net_entity(get<std::uint16_t>(message, 0x18), registry);
}

// ---------------------------------------------------------------------------------------------
// Session
// ---------------------------------------------------------------------------------------------
void session_update_transport_0076aa00(Raw session, float dt, MpSeg43Hooks& hooks) {
    if (void* const server = get<void*>(session, kServer)) {
        hooks.transport_console_update_virtual_18(at(server, 0xa4), dt);
        return;
    }
    if (void* const client = get<void*>(session, kClient))
        hooks.transport_console_update_virtual_18(at(client, 0xc8), dt);
}

void session_alloc_host_info_blob_0076c2c0(Raw session, Raw* out_blob, std::uint32_t* out_size,
                                            const MpGlobals& globals, MpSeg43Hooks& hooks) {
    *out_blob = hooks.operator_new(0x88);
    std::memcpy(*out_blob, at(session, 0x100), 0x22 * 4);
    put<std::uint8_t>(*out_blob, 0x26, get<std::uint8_t>(get<void*>(session, kServer), 0x9c));
    for (std::size_t offset = 0x30; offset < 0x68; offset += 8) {
        put<std::uint32_t>(*out_blob, offset, get<std::uint32_t>(globals.host_info_block_00e1955c, offset - 0x30));
        put<std::uint32_t>(*out_blob, offset + 4,
                           get<std::uint32_t>(globals.host_info_block_00e1955c, offset - 0x2c));
    }
    *out_size = 0x88; // EAX is left holding the loop counter
}

std::uint32_t session_classify_join_request_0076c340(Raw session, const void* info) noexcept {
    auto s8 = [&](std::size_t offset) { return static_cast<int>(get<std::int8_t>(session, offset)); };
    if (get<std::int32_t>(get<void*>(session, kServer), 0x9c) != 3) return 1;
    if (s8(0x128) + s8(0x12b) >= s8(0x127)) return 2;
    if (get<std::uint8_t>(info, 0x58) == 0 && s8(0x12a) - s8(0x129) - s8(0x128) + s8(0x127) <= 0)
        return 3;
    const auto mask_lo = get<std::uint32_t>(session, 0x170);
    const auto mask_hi = get<std::uint32_t>(session, 0x174);
    if ((get<std::uint32_t>(info, 0x48) & mask_lo) != mask_lo ||
        (get<std::uint32_t>(info, 0x4c) & mask_hi) != mask_hi)
        return 4;
    const int version = get<std::uint8_t>(info, 0x31);
    if (s8(0x12c) < version || s8(0x12d) > version) return 5;
    put<std::int8_t>(session, 0x12b, static_cast<std::int8_t>(get<std::int8_t>(session, 0x12b) + 1));
    return 0;
}

void session_clear_receive_queue_0076c5a0(Raw session, MpSeg43Hooks& hooks) {
    for (auto* slot = get<void**>(session, kQueueData);
         slot != get<void**>(session, kQueueData) + get<std::int32_t>(session, kQueueCount); ++slot)
        if (*slot != nullptr) hooks.message_delete_virtual_00(*slot);
    put<std::int32_t>(session, kQueueCount, 0);
}

void session_broadcast_multi_settings_0076c9c0(Raw session, const void* settings, MpSeg43Hooks& hooks) {
    alignas(4) unsigned char message[0x50];
    message23_construct_from_settings_0076bb60(message, settings, hooks);
    hooks.transport_broadcast_except_00784790(get<void*>(session, kServer), message, nullptr);
}

void send_pending_selection_0076cf30(Raw owner, std::uint32_t slot, const void* unit, MpSeg43Hooks& hooks) {
    void* const message = hooks.operator_new(0x20);
    if (message != nullptr) {
        hooks.session_message_construct_base_0075b430(message, 0x4d);
        put<std::uint32_t>(message, 0x18, slot);
        set_message_header(message, kVtableMessage77);
        put<std::uint16_t>(message, 0x1c, unit != nullptr ? get<std::uint16_t>(unit, 0x174) : 0);
    }
    hooks.owner_send_virtual_28(owner, message);
}

void session_purge_net_entity_messages_0076da60(Raw session, MpSeg43Hooks& hooks) {
    void* transport = get<void*>(session, kServer);
    if (transport == nullptr) transport = get<void*>(session, kClient);
    if (transport != nullptr) hooks.peer_flush_matching_records_007849c0(transport);
    auto end = [&] { return get<void**>(session, kQueueData) + get<std::int32_t>(session, kQueueCount); };
    for (void** slot = get<void**>(session, kQueueData); slot != end();) {
        void* const message = *slot;
        if (!hooks.message_is_type_virtual_0c(message, 0x46)) {
            ++slot;
            continue;
        }
        hooks.message_delete_virtual_00(message);
        for (void** p = slot; p != end() - 1; ++p) p[0] = p[1];
        put<std::int32_t>(session, kQueueCount, get<std::int32_t>(session, kQueueCount) - 1);
    }
}

void host_info_append_blob_0076e3c0(Raw owner, std::uint32_t blob, MpSeg43Hooks& hooks) {
    if (get<std::uint32_t>(owner, 0x250) == get<std::uint32_t>(owner, 0x254)) {
        const std::uint32_t capacity = get<std::uint32_t>(owner, 0x254) * 2 + 2;
        if (capacity > get<std::uint32_t>(owner, 0x254)) {
            put<std::uint32_t>(owner, 0x254, capacity);
            const std::uint64_t bytes = std::uint64_t{capacity} * 4;
            auto* const data = static_cast<std::uint32_t*>(
                hooks.operator_new(bytes > 0xffffffffu ? 0xffffffffu : static_cast<std::size_t>(bytes)));
            if (get<void*>(owner, 0x24c) != nullptr) {
                for (std::uint32_t i = 0; i < get<std::uint32_t>(owner, 0x250); ++i)
                    data[i] = get<std::uint32_t*>(owner, 0x24c)[i];
                hooks.free_block(get<void*>(owner, 0x24c));
            }
            put<void*>(owner, 0x24c, data);
        }
    }
    get<std::uint32_t*>(owner, 0x24c)[get<std::uint32_t>(owner, 0x250)] = blob;
    put<std::uint32_t>(owner, 0x250, get<std::uint32_t>(owner, 0x250) + 1);
}

bool lan_host_key_less_0076eaa0(const void* left, const void* right, MpSeg43Hooks& hooks) {
    bool compare_tail = true;
    int order = 0;
    if (get<std::uint32_t>(left, 0) == 0) {
        order = get<std::uint32_t>(right, 0) != 0 ? -1 : 0;
    } else if (get<std::uint32_t>(right, 0) != 0) {
        order = _stricmp(get<const char*>(left, 4), get<const char*>(right, 4)); // __stricmp 00BF7FBF
    } else {
        compare_tail = false; // straight to the 00443CB0 test
    }
    if (compare_tail && order < 0) return true;
    if (hooks.native_string_less_ci_00443cb0(right, left)) return false;
    const auto left_hi = get<std::uint32_t>(left, 0xc), right_hi = get<std::uint32_t>(right, 0xc);
    if (left_hi < right_hi) return true;
    if (left_hi > right_hi) return false;
    return get<std::uint32_t>(left, 8) < get<std::uint32_t>(right, 8);
}

void session_step_entity_replication_0076f210(Raw session, std::uint32_t, bool flag,
                                              const MpGlobals& globals, MpSeg43Hooks& hooks) {
    if (globals.game_00e188a8 == nullptr) return;
    if (get<void*>(session, kServer) != nullptr) hooks.replication_drain_child_queues_0077f5e0(session);
    if (!flag) return;
    if (void* const server = get<void*>(session, kServer)) {
        put<std::uint32_t>(server, 0x98, 0);
        hooks.game_tuning_get_singleton_0042e740(); // result unused
        hooks.replication_flush_queued_rows_0077c540(get<void*>(session, kServer));
    }
    void* const client = get<void*>(session, kClient);
    if (client == nullptr || get<std::int32_t>(client, 0x9c) < 8) return;
    hooks.replication_step_local_player_record_0077d7f0();
    hooks.plane_send_sync_ack_batch_007cca50();
}

void session_reassign_peer_units_0076f2b0(std::uint32_t player, const MpGlobals& globals,
                                          MpSeg43Hooks& hooks) {
    const std::int32_t state = hooks.game_state_004bca50(globals.game_00e188a8);
    const bool in_mission = state == 4 || state == 5 || state == 6 || state == 7;
    for (auto* node = get<std::uint8_t*>(globals.unit_list_head_00f87198, 0);
         node != globals.unit_list_head_00f87198; node = get<std::uint8_t*>(node, 0)) {
        void* const unit = get<void*>(node, 8);
        if (in_mission) {
            if (unit != nullptr && get<std::uint32_t>(unit, 0x180) == player)
                hooks.unit_release_role_virtual_70(unit, 1);
        } else if (get<std::uint32_t>(unit, 0x1ac) == player) {
            for (std::uint32_t slot = 0; slot < 9; ++slot) hooks.unit_set_role_slot_virtual_154(unit, slot, 8);
        }
    }
    hooks.game_refresh_after_reassign_004d56e0(globals.game_00e188a8);
}

void session_collect_incompatible_clients_0076f360(Raw session, MpSeg43Hooks& hooks) {
    if (get<std::int32_t>(session, kMode) != 1) return;
    for_each_list_node(at(get<void*>(session, kServer), 0xc), [&](std::uint8_t* node) {
        hooks.peer_mark_incompatible_007f7450(get<void*>(get<void*>(node, 8), 0xd50));
    });
    alignas(4) unsigned char message[kSessionMessageSmall];
    hooks.session_message_construct_base_0075b430(message, 0xd);
    set_message_header(message, kVtableMessage13);
    hooks.transport_broadcast_except_00784790(get<void*>(session, kServer), message, nullptr);
}

std::uint32_t session_all_peers_scene_checked_0076f400(Raw session, const MpGlobals& globals,
                                                       MpSeg43Hooks& hooks) {
    if (get<std::int32_t>(session, kMode) != 1) return 1;
    void* const server = get<void*>(session, kServer);
    std::uint32_t checked = 0;
    for_each_list_node(at(server, 0xc), [&](std::uint8_t* node) {
        void* const peer = get<void*>(get<void*>(node, 8), 0xd50);
        if (hooks.peer_scene_checked_007f7470(peer)) {
            ++checked;
        } else if (hooks.peer_should_kick_007f7480(get<void*>(get<void*>(node, 8), 0xd50))) {
            hooks.transport_kick_peer_00784b10(get<void*>(session, kServer),
                get<std::uint32_t>(get<void*>(get<void*>(node, 8), 0xd50), 0x50), 1);
        }
    });
    if (checked < get<std::uint32_t>(server, 0x10) - 1) return 2; // list size at transport+10h
    return hooks.scene_driver_virtual_168(globals.scene_driver_00f8a2fc) == 0 ? 1 : 0;
}

void session_send_hint_message_00770870(Raw session, Raw name, MpSeg43Hooks& hooks) {
    if (get<std::int32_t>(session, kMode) != 1) return;
    alignas(4) unsigned char message[kSessionMessageSmall];
    Raw const built = hooks.hint_message_construct_0076f9e0(message, name);
    hooks.transport_broadcast_except_00784790(get<void*>(session, kServer), built, nullptr);
    hooks.hint_message_destruct_00765d90(message);
}

void session_show_chat_text_007708e0(Raw session, Raw message, std::uint32_t arg,
                                     const MpGlobals& globals, MpSeg43Hooks& hooks) {
    if (get<std::int32_t>(session, kMode) == 1) return;
    const char* text = get<const char*>(message, 4);
    if (text == nullptr) text = kEmpty;
    hooks.game_show_chat_text_004c8950(globals.game_00e188a8, text);
    void* const root = globals.gui_root_00e198b4;
    if (root == nullptr || get<void*>(root, 0x48) == nullptr) return;
    hooks.lobby_append_chat_005d92b0(get<void*>(root, 0x48), message, arg);
    hooks.lobby_refresh_005e5ae0(get<void*>(globals.gui_root_00e198b4, 0x48));
}

void session_set_mode_host_00770e40(Raw session, MpSeg43Hooks& hooks) {
    hooks.session_set_mode_0076fad0(session, 1);
}

void session_request_select_player_007718f0(Raw session, std::uint32_t a, std::uint32_t b,
                                            MpSeg43Hooks& hooks) {
    if (get<std::int32_t>(session, kMode) == 1) {
        hooks.session_host_select_player_0076c840(session, a, b);
        return;
    }
    alignas(4) unsigned char message[kSessionMessageSmall];
    hooks.session_message_construct_base_0075b430(message, 0x19);
    set_message_header(message, kVtableMessage25);
    put<std::uint32_t>(message, 0x18, a);
    put<std::uint32_t>(message, 0x1c, b);
    hooks.session_send_to_host_00770af0(session, message);
}

void session_request_slot_ai_enable_00771e80(Raw session, std::uint32_t slot, std::uint8_t flag,
                                             MpSeg43Hooks& hooks) {
    const auto mode = get<std::int32_t>(session, kMode);
    if (mode == 1) {
        hooks.session_host_slot_ai_enable_0076cb90(session, slot, flag);
        return;
    }
    if (mode != 2) return;
    alignas(4) unsigned char message[kSessionMessageSmall];
    hooks.session_message_construct_base_0075b430(message, 0x1e);
    set_message_header(message, kVtableMessage30);
    put<std::uint32_t>(message, 0x18, slot);
    put<std::uint8_t>(message, 0x1c, flag);
    hooks.session_send_to_host_00770af0(session, message);
}

void session_request_player_surrender_00771f20(Raw session, std::uint32_t player, std::uint8_t flag,
                                               MpSeg43Hooks& hooks) {
    const auto mode = get<std::int32_t>(session, kMode);
    if (mode == 1) {
        hooks.session_set_player_surrender_00770750(session, player, flag);
        return;
    }
    if (mode != 2) return;
    alignas(4) unsigned char message[kSessionMessageSmall];
    hooks.session_message_construct_base_0075b430(message, 0x20);
    set_message_header(message, kVtableMessage32);
    put<std::uint32_t>(message, 0x18, player);
    put<std::uint8_t>(message, 0x1c, flag);
    hooks.session_send_to_host_00770af0(session, message);
}

void session_broadcast_scene_index_00772390(Raw session, const MpGlobals& globals, MpSeg43Hooks& hooks) {
    std::uint8_t* const game = globals.game_00e188a8;
    alignas(4) unsigned char message[0x30];
    Raw const built = hooks.scene_index_message_construct_00771190(message,
        get<std::uint32_t>(game, 0x60c), get<std::uint32_t>(game, 0x5fc) + 0x90c,
        get<std::uint32_t>(game, 0x2060), get<std::uint32_t>(game, 0x2064));
    hooks.transport_broadcast_except_00784790(get<void*>(session, kServer), built, nullptr);
    hooks.scene_index_message_destruct_007662a0(message);
    void* const root = globals.gui_root_00e198b4;
    if (root != nullptr && get<void*>(root, 0x48) != nullptr)
        hooks.lobby_refresh_scene_005d9d60(get<void*>(root, 0x48));
}

void session_broadcast_sub_scene_00772430(Raw session, const void* name, std::uint32_t value,
                                          MpSeg43Hooks& hooks, NativeStringStorage& strings) {
    alignas(4) unsigned char message[0x24];
    Raw const built = message18_build_subscene_007710e0(message, name, value, hooks, strings);
    hooks.transport_broadcast_except_00784790(get<void*>(session, kServer), built, nullptr);
    hooks.subscene_message_destruct_00766150(message);
}

void session_client_send_local_score_00772a40(Raw session, const MpGlobals& globals, MpSeg43Hooks& hooks) {
    if (get<std::int32_t>(globals.game_00e188a8, 0x1fe4) != 2) return;
    alignas(4) unsigned char message[0x190]; // only the fields below are written
    hooks.session_message_construct_base_0075b430(message, 0x15);
    put<std::uint32_t>(message, 4, 1);
    put<std::uint32_t>(message, 0, kVtableScoring);
    put<std::int32_t>(message, 0x1c, -1);
    std::uint8_t* const game = globals.game_00e188a8;
    put<std::uint32_t>(message, 0x18, 1);
    put<std::uint32_t>(message, 0x1c, get<std::uint32_t>(game, 0x18ec));
    const auto player = get<std::uint32_t>(game, 0x18ec);
    put<float>(message, 0x2c, get<float>(get<void*>(game, 0x21a0), player * 0x284u + 0x18));
    hooks.session_send_to_host_00770af0(session, message);
}

void session_host_clear_all_selections_007735b0(Raw session, const MpGlobals& globals,
                                                MpSeg43Hooks& hooks) {
    for (std::uint32_t player = 0; player < 8; ++player) {
        if (get<std::uint8_t>(globals.game_00e188a8, 0x750 + player * 0x118u) == 0) continue;
        hooks.session_request_player_ready_00771dd0(session, player, 0, 1);
        hooks.session_host_select_player_0076c840(session, player, 0xffffffffu);
    }
}

// ---------------------------------------------------------------------------------------------
// Records and messages
// ---------------------------------------------------------------------------------------------
void optional_buffer_destruct_0076b200(Raw buffer, MpSeg43Hooks& hooks) {
    put<std::uint32_t>(buffer, 0, kVtableOptionalBuffer);
    if (get<std::uint8_t>(buffer, 0x14) != 0) hooks.free_block(get<void*>(buffer, 4));
}

Raw message23_construct_from_settings_0076bb60(Raw message, const void* settings, MpSeg43Hooks& hooks) {
    hooks.session_message_construct_base_0075b430(message, 0x17);
    set_message_header(message, kVtableMessage23);
    std::memcpy(at(message, 0x18), settings, 0xe * 4);
    return message;
}

Raw message11_construct_with_value_0076bc40(Raw message, std::uint32_t value, MpSeg43Hooks& hooks) {
    hooks.session_message_construct_base_0075b430(message, 0xb);
    put<std::uint32_t>(message, 0x18, value);
    set_message_header(message, kVtableMessage11);
    return message;
}

Raw lan_host_record_copy_construct_00770fa0(Raw record, const void* source, NativeStringStorage& strings) {
    copy_construct_string(record, source, strings);
    put<std::uint32_t>(record, 8, get<std::uint32_t>(source, 8));
    put<std::uint32_t>(record, 0xc, get<std::uint32_t>(source, 0xc));
    return record;
}

Raw message18_build_subscene_007710e0(Raw message, const void* name, std::uint32_t value,
                                      MpSeg43Hooks& hooks, NativeStringStorage& strings) {
    hooks.session_message_construct_base_0075b430(message, 0x18);
    put<std::uint32_t>(message, 4, 1);
    put<std::uint32_t>(message, 0, kVtableSubScene);
    copy_construct_string(at(message, 0x18), name, strings);
    put<std::uint32_t>(message, 0x20, value);
    return message;
}

Raw message_roleowner_construct_007796f0(Raw message, std::uint32_t a, std::uint32_t b,
                                         std::uint32_t c, std::uint8_t d, MpSeg43Hooks& hooks) {
    hooks.session_message_construct_base_0075b430(message, 0x4b);
    put<std::uint16_t>(message, 0x18, 0);
    put<std::uint8_t>(message, 0x1a, 0);
    put<std::uint8_t>(message, 0x1c, 0);
    put<std::uint8_t>(message, 0x20, 0);
    put<std::uint32_t>(message, 0x24, a);
    put<std::uint8_t>(message, 0x30, d);
    set_message_header(message, kVtableRoleOwner);
    put<std::uint32_t>(message, 0x28, b);
    put<std::uint32_t>(message, 0x2c, c);
    return message;
}

Raw message_entity_release_construct_00779780(Raw message, std::uint32_t payload, MpSeg43Hooks& hooks) {
    hooks.session_message_construct_base_0075b430(message, 0x4f);
    put<std::uint16_t>(message, 0x18, 0);
    put<std::uint8_t>(message, 0x1a, 0);
    put<std::uint8_t>(message, 0x1c, 0);
    put<std::uint32_t>(message, 0x20, payload);
    set_message_header(message, kVtableEntityRelease);
    return message;
}

Raw message_reconlevel_construct_00779850(Raw message, const void* unit, std::int32_t party,
                                          MpSeg43Hooks& hooks) {
    hooks.session_message_construct_base_0075b430(message, 0x61);
    const unsigned char* const record = at(unit, static_cast<std::size_t>(party * 0x34 + 0x1e8));
    put<std::uint16_t>(message, 0x18, 0);
    put<std::uint8_t>(message, 0x1a, 0);
    put<std::uint32_t>(message, 4, 1);
    put<std::uint8_t>(message, 0x1c, 0);
    put<std::uint32_t>(message, 0, kVtableReconLevel);
    put<std::uint32_t>(message, 0x20, get<std::uint32_t>(record, get<std::uint8_t>(record, 0x10) != 0 ? 8 : 4));
    put<std::int32_t>(message, 0x24, party);
    return message;
}

void message_formation_set_write_0077a1a0(Raw message, Raw stream, MpSeg43Hooks& hooks) {
    hooks.message_write_extended_header_0075b480(message, stream);
    hooks.bit_write_bool_004290b0(stream, get<std::uint8_t>(message, 0x1c));
    hooks.bit_write_dword_00429070(stream, get<std::uint32_t>(message, 0x20), 3);
    hooks.bit_write_dword_00429070(stream, get<std::uint32_t>(message, 0x24), 8);
    hooks.bit_write_float_array_00429790(stream, get<void*>(message, 0x28),
        get<std::uint32_t>(message, 0x24) * 4, 0, 1, kFltMax, 0x20);
    hooks.bit_write_float_array_00429790(stream, get<void*>(message, 0x2c),
        get<std::uint32_t>(message, 0x24) * 4, 0, 1, kFltMax, 0x20);
}

void message_formation_set_read_0077a220(Raw message, Raw stream, MpSeg43Hooks& hooks) {
    hooks.message_read_extended_header_0075b4c0(message, stream);
    void* const cursor = at(stream, 4);
    hooks.bit_read_bool_00428d70(cursor, at(message, 0x1c));
    hooks.bit_read_dword_00428d10(cursor, at(message, 0x20), 3);
    hooks.bit_read_dword_00428d10(cursor, at(message, 0x24), 8);
    auto bytes = [&] {
        // count * 4 wraps in 32 bits (ADD EAX,EAX twice); the * 4 that follows is the checked MUL.
        const std::uint32_t elements = get<std::uint32_t>(message, 0x24) * 4u;
        const std::uint64_t n = static_cast<std::uint64_t>(elements) * 4u;
        return (n >> 32) != 0 ? std::size_t{0xffffffffu} : static_cast<std::size_t>(n);
    };
    // The previous +28h / +2Ch buffers are overwritten without being freed.
    put<void*>(message, 0x28, hooks.operator_new(bytes()));
    void* const second = hooks.operator_new(bytes());
    put<void*>(message, 0x2c, second);
    hooks.bit_read_float_array_004294f0(cursor, get<void*>(message, 0x28),
        get<std::uint32_t>(message, 0x24) * 4, 0, 1, kFltMax, 0x20);
    hooks.bit_read_float_array_004294f0(cursor, get<void*>(message, 0x2c),
        get<std::uint32_t>(message, 0x24) * 4, 0, 1, kFltMax, 0x20);
}

// ---------------------------------------------------------------------------------------------
// Units and replication
// ---------------------------------------------------------------------------------------------
void unit_set_recon_level_0077b0c0(Raw recon, std::uint32_t party, std::uint32_t arg, std::int32_t level,
    std::uint8_t* const& recon_manager_00f8a0c4, const MpGlobals& globals, MpSeg43Hooks& hooks) {
    unsigned char* const unit = static_cast<unsigned char*>(recon) - 0x1e4;
    hooks.recon_party_touch_00803ba0(party);
    if (get<std::int32_t>(globals.game_00e188a8, 0x1fe4) != 2) {
        alignas(4) unsigned char this_table[0x0c];
        alignas(4) unsigned char field[0x0c];
        Raw const table = hooks.entity_lua_this_table_00927b40(unit, this_table);
        Raw const levels = hooks.lua_object_get_field_00b67800(table, field, kReconLevel);
        hooks.lua_object_set_integer_00b665d0(levels, party, static_cast<std::uint32_t>(level));
        hooks.lua_object_destruct_00b67700(field);
        hooks.lua_object_destruct_00b67700(this_table);
    }
    hooks.recon_manager_notify_00980e50(recon_manager_00f8a0c4, party, unit, arg, level);
    if (std::uint8_t* const front = globals.front_end_00e198c4)
        hooks.player_record_recon_changed_005a31a0(get<void*>(front, 0x54), unit);
    if (get<std::int32_t>(globals.game_00e188a8, 0x1fe4) == 1) {
        alignas(4) unsigned char message[0x28]; // no destructor runs on it
        message_reconlevel_construct_00779850(message, unit, static_cast<std::int32_t>(party), hooks);
        for (std::int32_t i = 0; i < get<std::int32_t>(get<void*>(globals.game_00e188a8, 0x5fc), 0x988); ++i) {
            void* const record = get<void*>(globals.game_00e188a8, 0x18cc + static_cast<std::size_t>(i) * 4);
            if (get<std::uint32_t>(record, 0x28) != party || get<std::uint8_t>(record, 9) != 0 ||
                get<std::uint8_t>(record, 0xb) == 0)
                continue;
            put<std::uint16_t>(message, 0x18, get<std::uint16_t>(unit, 0x174));
            hooks.session_send_to_peer_00770b50(globals.game_00e188a8 + 0x1ef0, get<std::uint32_t>(record, 0x50), message);
        }
    }
    std::uint8_t* const game = globals.game_00e188a8;
    void* const local = get<void*>(game, 0x18cc + get<std::uint32_t>(game, 0x18ec) * 4u);
    if (party == get<std::uint32_t>(local, 0x28)) put<float>(unit, 0x2f8, level >= 2 ? kOne : 0.0f);
    const auto side = get<std::int32_t>(unit, 0x54);
    if (side >= 2 || party != (side == 0 ? 1u : 0u)) return;
    put<float>(unit, 0x300, globals.now_00f876a4);
    if (level > 0) put<std::int32_t>(unit, 0x2fc, level);
}

bool unit_is_local_players_role_slot_00779b60(const void* unit, const MpGlobals& globals) noexcept {
    const auto slot = get<std::int32_t>(unit, 0x1ac);
    if (slot >= 8) return false;
    std::uint8_t* const game = globals.game_00e188a8;
    if (game == nullptr) return true;
    if (get<std::int32_t>(game, 0x1fe4) != 0) {
        void* const player = get<void*>(game, 0x18cc + static_cast<std::size_t>(slot) * 4);
        if (player == nullptr) return true;
        return get<std::uint8_t>(player, 0xc8) == 0;
    }
    std::uint8_t* const front = globals.front_end_00e198c4;
    if (front == nullptr || get<void*>(front, 0x54) == nullptr) return true;
    return get<std::uint8_t>(get<void*>(front, 0x54), 5) == 0;
}

void unit_get_property_recon_level_00779bb0(Raw unit, Raw lua, Raw name, MpSeg43Hooks& hooks) {
    hooks.entity_get_property_unit_command_00927ad0(unit, lua, name);
    const char* const text = get<const char*>(name, 4);
    if (text == nullptr || _stricmp(text, kReconLevel) != 0) return;
    alignas(4) unsigned char table[0x0c]; // LuaObject temporary at ESP+0Ch
    hooks.lua_object_new_table_00b67930(lua, table);
    for (std::uint32_t party = 0; party < 3; ++party) {
        const unsigned char* const record = at(unit, 0x1e8 + party * 0x34u);
        const auto value = get<std::uint32_t>(record, get<std::uint8_t>(record, 0x10) != 0 ? 8 : 4);
        hooks.lua_object_set_integer_00b665d0(table, party, value);
    }
    hooks.lua_object_destruct_00b67700(table);
}

void unit_update_owner_hud_state_00779c60(Raw unit, float value, const MpGlobals& globals,
                                          MpSeg43Hooks& hooks) {
    if (hooks.unit_owner_00923810(unit, 1) == nullptr ||
        !hooks.owner_role_check_virtual_5c(hooks.unit_owner_00923810(unit, 1), 2)) {
        put<float>(unit, 0x2f4, value);
        put<float>(unit, 0x2f8, value);
        return;
    }
    void* const owner = hooks.unit_owner_00923810(unit, 1);
    const bool full = globals.replay_00f87152 != 0 || get<std::uint8_t>(owner, 0x2f0) != 0;
    put<float>(unit, 0x2f4, full ? kOne : get<float>(owner, 0x2f4));
    put<float>(unit, 0x2f8, full ? kOne : get<float>(owner, 0x2f8));
}

std::int32_t unit_recon_record_value_00779d00(const void* unit, const MpGlobals& globals) noexcept {
    const std::size_t party = get<std::int32_t>(unit, 0x54) == 0 ? 1 : 0;
    const unsigned char* const record = at(unit, party * 0x34 + 0x1e8);
    const auto value = get<std::int32_t>(record, get<std::uint8_t>(record, 0x10) != 0 ? 8 : 4);
    const auto held = get<std::int32_t>(unit, 0x2fc);
    // FLD float +300h, FADD double 30.0, FCOMIP against FLD float [00F876A4]: x87 extended compare.
    if (value < held &&
        static_cast<long double>(get<float>(unit, 0x300)) + kReconHoldSeconds >
            static_cast<long double>(globals.now_00f876a4))
        return held;
    return value;
}

void replication_resolve_pending_create_00779e20(Raw unit, const MpGlobals& globals, MpSeg43Hooks& hooks) {
    void* const holder = get<void*>(unit, 0xc0);
    const auto id = get<std::uint16_t>(get<void*>(holder, 8), 0x20);
    void* const entity = id != 0 ? resolve_net_entity(id, globals.registry) : nullptr;
    alignas(4) unsigned char transform[0x40];
    Raw const result = hooks.transform_set_identity_0075b500(get<void*>(holder, 8), transform);
    hooks.unit_apply_create_virtual_98(unit, entity, get<std::uint32_t>(globals.game_00e188a8, 0x19cc), result);
    hooks.unit_notify_entity_virtual_ac(unit, entity);
}

void net_entity_holder_stamp_sync_tick_00779f80(Raw holder, const MpGlobals& globals) noexcept {
    put<std::int32_t>(holder, 4, globals.tick_00f876b0);
}

void entity_send_message_to_host_00779f90(const void* entity, Raw message, const MpGlobals& globals,
                                          MpSeg43Hooks& hooks) {
    put<std::uint16_t>(message, 0x18, get<std::uint16_t>(entity, 0x174));
    hooks.session_send_to_host_00770af0(globals.game_00e188a8 + 0x1ef0, message);
}

void entity_send_message_to_peer_00779fc0(const void* entity, const void* peer, Raw message,
                                          const MpGlobals& globals, MpSeg43Hooks& hooks) {
    put<std::uint16_t>(message, 0x18, get<std::uint16_t>(entity, 0x174));
    hooks.session_send_to_peer_00770b50(globals.game_00e188a8 + 0x1ef0, get<std::uint32_t>(peer, 0x50), message);
}

bool message_is_privileged_kind_00779ff0(Raw message, MpSeg43Hooks& hooks) {
    switch (get<std::uint8_t>(message, 0x10)) {
    case 0x82: case 0x81: case 0xa7: case 0xa8: case 0xa9: return true;
    default: return hooks.message_is_type_virtual_0c(message, 0x4a);
    }
}

Raw command_registry_find_by_id_0077a050(const void* message, const MpGlobals& globals) noexcept {
    const std::uint32_t id = get<std::uint8_t>(message, 0x20);
    for (std::uint8_t* node = globals.command_registry_00e19a70; node != nullptr;
         node = get<std::uint8_t*>(node, 4)) {
        void* const command = get<void*>(node, 8);
        if (id == get<std::uint32_t>(command, 4)) return command;
    }
    return nullptr;
}

std::uint32_t command_registry_dispatch_by_id_0077a020(const void* message, const MpGlobals& globals,
                                                       MpSeg43Hooks& hooks) {
    // No null test: an unknown id makes the image call through a null object.
    return hooks.command_virtual_04(command_registry_find_by_id_0077a050(message, globals));
}

// ---------------------------------------------------------------------------------------------
// Role slots
// ---------------------------------------------------------------------------------------------
void unit_apply_role_owner_slots_0077a5d0(Raw unit, const MpGlobals& globals, RoleHooks& hooks) {
    std::uint8_t* const game = globals.game_00e188a8;
    const auto owner = get<std::int32_t>(unit, 0x188);
    if (owner == 9 || owner == get<std::int32_t>(game, 0x18ec))
        hooks.unit_set_role_owner_virtual_148(unit, 2, get<std::uint32_t>(game, 0x18ec));
    for (std::uint32_t slot = 0; slot < 9; ++slot) {
        const auto a = get<std::int32_t>(unit, 0x188 + slot * 4);
        if (a >= 8) continue;
        const auto b = get<std::int32_t>(unit, 0x1ac + slot * 4);
        if (b >= 8 || a == b) continue;
        hooks.unit_set_role_slot_virtual_154(unit, slot, 8);
    }
}

namespace {
void mark_role_suppressed(Raw unit, std::uint8_t* const& role_manager, const MpGlobals& globals,
                          RoleHooks& hooks) {
    if (!hooks.unit_is_kind_virtual_5c(unit, 5)) return;
    if (hooks.unit_is_kind_virtual_5c(unit, 0x1c)) return;
    if (hooks.unit_is_kind_virtual_5c(unit, 0x1b)) return;
    const auto owner = get<std::int32_t>(unit, 0x188);
    if (owner == 9) return;
    std::uint8_t* const game = globals.game_00e188a8;
    const auto local = get<std::int32_t>(game, 0x18ec);
    if (owner == local) return;
    void* const record = get<void*>(game, 0x18cc + static_cast<std::size_t>(local) * 4);
    if (get<std::uint32_t>(unit, 0x54) != get<std::uint32_t>(record, 0x28)) return;
    void* key = unit; // the image reuses its first argument slot for the key
    *hooks.suppress_slot_0077edf0(role_manager + 0xd4, &key) = 1;
}
} // namespace

void unit_assign_single_device_role_0077f2d0(Raw unit, std::uint32_t a, std::uint8_t* const& role_manager_00f8a0c4,
                                             const MpGlobals& globals, RoleHooks& hooks) {
    hooks.unit_role_009278a0(unit, a);
    mark_role_suppressed(unit, role_manager_00f8a0c4, globals, hooks);
}
void unit_set_role_available_0077f360(Raw unit, std::uint32_t a, std::uint32_t b,
                                      std::uint8_t* const& role_manager_00f8a0c4, const MpGlobals& globals,
                                      RoleHooks& hooks) {
    hooks.unit_role_00927d20(unit, a, b);
    mark_role_suppressed(unit, role_manager_00f8a0c4, globals, hooks);
}
void unit_set_role_owner_and_refresh_0077f3f0(Raw unit, std::uint32_t a, std::uint32_t b,
                                              std::uint8_t* const& role_manager_00f8a0c4,
                                              const MpGlobals& globals, RoleHooks& hooks) {
    hooks.unit_role_00927810(unit, a, b);
    mark_role_suppressed(unit, role_manager_00f8a0c4, globals, hooks);
}
void unit_set_role_slot_and_refresh_0077f480(Raw unit, std::uint32_t a, std::uint32_t b,
                                             std::uint8_t* const& role_manager_00f8a0c4,
                                             const MpGlobals& globals, RoleHooks& hooks) {
    hooks.unit_role_009281c0(unit, a, b);
    mark_role_suppressed(unit, role_manager_00f8a0c4, globals, hooks);
}
void unit_send_role_owner_and_refresh_0077f510(Raw unit, std::uint32_t a, std::uint32_t b,
                                               std::uint32_t c, std::uint8_t* const& role_manager_00f8a0c4,
                                               const MpGlobals& globals, RoleHooks& hooks) {
    hooks.unit_role_00927e50(unit, a, b, c);
    mark_role_suppressed(unit, role_manager_00f8a0c4, globals, hooks);
}

void unit_send_role_transfer_0077c470(Raw unit, std::uint32_t a, std::uint32_t b, const MpGlobals& globals,
                                      MpSeg43Hooks& hooks, RoleHooks& role_hooks) {
    std::uint8_t* const game = globals.game_00e188a8;
    const auto state = get<std::int32_t>(game, 0x5d4);
    if (state < 0xd && get<std::uint8_t>(game, 0x216c) == 0) return;
    if (get<std::int32_t>(game, 0x1fe4) != 0) {
        void* const local = get<void*>(game, 0x18cc + get<std::uint32_t>(game, 0x18ec) * 4u);
        if (get<std::uint8_t>(local, 0x19) != 0 || state != 0xd) return;
    }
    // The image builds MT_ROLEOWNER inline with the same stores as 007796F0.
    alignas(4) unsigned char message[0x34];
    message_roleowner_construct_007796f0(message, a, get<std::uint32_t>(game, 0x18ec), b, 0, hooks);
    role_hooks.route_message_0077c2a0(unit, message, 0, 0);
}

void entity_send_activate_0077d580(Raw entity, std::uint8_t flag, MpSeg43Hooks& hooks, RoleHooks& role_hooks) {
    if (flag != 0) return;
    alignas(4) unsigned char message[0x24];
    hooks.session_message_construct_base_0075b430(message, 0x52);
    put<std::uint16_t>(message, 0x18, 0);
    put<std::uint8_t>(message, 0x1a, 0);
    put<std::uint32_t>(message, 4, 1);
    put<std::uint8_t>(message, 0x1c, 0);
    put<std::uint32_t>(message, 0, 0x00D02D58u);
    put<std::uint8_t>(message, 0x20, 0);
    role_hooks.entity_send_0077c7b0(entity, message, 0);
}

Raw entity_as_kind_1b_0077bcf0(Raw entity, RoleHooks& hooks) {
    if (entity == nullptr || !hooks.unit_is_kind_virtual_5c(entity, 0x1b)) return nullptr;
    return entity;
}

// ---------------------------------------------------------------------------------------------
// Records, lists, MNetwork
// ---------------------------------------------------------------------------------------------
namespace {
void link_new_tail(void* head, void* node) noexcept {
    put<void*>(head, 4, node);
    put<void*>(get<void*>(node, 4), 0, node);
}
CRITICAL_SECTION* section(void* lock) noexcept { return static_cast<CRITICAL_SECTION*>(lock); }
void lock_enter(void* lock) {
    EnterCriticalSection(section(lock)); // [00CE2218]
    put<std::int32_t>(lock, 0x18, get<std::int32_t>(lock, 0x18) + 1);
}
void lock_leave(void* lock) {
    put<std::int32_t>(lock, 0x18, get<std::int32_t>(lock, 0x18) - 1);
    LeaveCriticalSection(section(lock)); // [00CE2210]
}
} // namespace

Raw net_entity_peer_holder_buy_node_0077ba60(Raw next, Raw prev, const void* value, MpSeg43Hooks& hooks) {
    void* const node = hooks.operator_new(0x18);
    if (node != nullptr) put<void*>(node, 0, next);
    // The image tests node+4 and node+8 for null, which is never true; a null node faults here.
    put<void*>(node, 4, prev);
    std::memcpy(at(node, 8), value, 0x10);
    return node;
}

void net_entity_add_peer_holder_0077eb50(Raw entity, std::uint32_t peer, std::uint8_t& dirty_00f87190,
                                         MpSeg43Hooks& hooks, ListHooks& lists) {
    alignas(4) unsigned char holder[0x10] = {}; // the image leaves bytes 0Ah..0Bh as stack contents
    put<std::uint32_t>(holder, 0, peer);
    put<std::int32_t>(holder, 4, -999999); // 0FFF0BDC1h
    put<std::uint16_t>(holder, 8, 0);
    put<std::uint32_t>(holder, 0xc, 0);
    void* const head = get<void*>(entity, 0x2a8);
    void* const node = net_entity_peer_holder_buy_node_0077ba60(head, get<void*>(head, 4), holder, hooks);
    lists.list_inc_size_0077dd00(at(entity, 0x2a4), 1);
    link_new_tail(head, node);
    void* const head2 = get<void*>(entity, 0x29c);
    void* const node2 = lists.list_buy_node_0077bb00(at(entity, 0x298), head2, get<void*>(head2, 4), &peer);
    lists.list_inc_size_0077de40(at(entity, 0x298), 1);
    link_new_tail(head2, node2);
    dirty_00f87190 = 1;
}

void replication_flag_remote_players_0077f090(Raw entity, const MpGlobals& globals,
                                              std::uint8_t& dirty_00f87190, MpSeg43Hooks& hooks,
                                              ListHooks& lists) {
    if (globals.game_00e188a8 == nullptr || get<std::int32_t>(globals.game_00e188a8, 0x1fe4) != 1) return;
    for (std::size_t offset = 0x18cc; offset < 0x18ec; offset += 4) {
        void* const player = get<void*>(globals.game_00e188a8, offset);
        if (player == nullptr || get<std::uint8_t>(player, 0xb) == 0) continue;
        net_entity_add_peer_holder_0077eb50(entity, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player)),
                                            dirty_00f87190, hooks, lists);
    }
}

Raw attribution_ref_assign_0077cc80(Raw reference, const void* source, ListHooks& lists) {
    void* const current = get<void*>(reference, 0x14);
    void* const target = get<void*>(source, 0x14);
    if (current != target) {
        if (current != nullptr) lists.observer_unregister_pair_006952a0(current, reference);
        put<void*>(reference, 0x14, target);
        if (target != nullptr) lists.observer_register_pair_00694a60(target, reference);
    }
    put<std::uint16_t>(reference, 0x18, get<std::uint16_t>(source, 0x18));
    return reference;
}

Raw message_setguiname_construct_0077ccc0(Raw message, std::uint32_t length, char* chars, std::uint8_t flag,
                                          MpSeg43Hooks& hooks, NativeStringStorage& strings) {
    hooks.session_message_construct_base_0075b430(message, 0x54);
    put<std::uint16_t>(message, 0x18, 0);
    put<std::uint8_t>(message, 0x1a, 0);
    put<std::uint32_t>(message, 4, 1);
    put<std::uint8_t>(message, 0x1c, 0);
    put<std::uint32_t>(message, 0, 0x00D03694u);
    put<std::uint32_t>(message, 0x20, 0);
    put<std::uint32_t>(message, 0x24, 0);
    put<std::uint8_t>(message, 0x28, flag);
    // The alias test against the by-value argument never holds.
    resize_native_string_header_0041dd40(at(message, 0x20), strings, length, true);
    if (length != 0) std::memcpy(get<char*>(message, 0x24), chars, get<std::uint32_t>(message, 0x20));
    if (chars != nullptr) strings.release(chars, length + 1); // the by-value argument's destructor
    return message;
}

void multi_score_holder_destruct_00780c50(Raw holder, ListHooks& lists, NativeStringStorage& strings) {
    lists.score_holder_member_destruct_00884af0(at(holder, 0x10));
    if (char* const chars = get<char*>(holder, 8)) strings.release(chars, get<std::uint32_t>(holder, 4) + 1);
}

void entity_register_in_world_lists_007811d0(Raw entity, ListHooks& lists) {
    lists.entity_register_in_parent_00928560(entity);
    lists.unit_list_push_back_00484540(at(get<void*>(entity, 0x30), 0x30), entity);
    lists.unit_list_push_back_00484540(at(get<void*>(entity, 0x30), 0x498), entity);
}

void entity_detach_from_world_lists_00781200(Raw entity, ListHooks& lists) {
    lists.entity_unregister_from_parent_00928570(entity);
    for (std::size_t offset : {std::size_t{0x30}, std::size_t{0x498}}) {
        void* const list = at(get<void*>(entity, 0x30), offset);
        for (void* node = get<void*>(list, 4); node != nullptr; node = get<void*>(node, 4)) {
            if (get<void*>(node, 8) != entity) continue;
            lists.unit_list_erase_node_004837d0(list, node);
            break;
        }
    }
}

void native_string_append_float_00781740(Raw string, float value, MpSeg43Hooks& hooks, NativeStringStorage& strings) {
    alignas(4) unsigned char text[8]; // the formatted temporary {length, chars}
    hooks.native_string_from_float_0043bdf0(text, value);
    const auto length = get<std::uint32_t>(text, 0);
    char* const chars = get<char*>(text, 4);
    if (length != 0) {
        const auto old = get<std::uint32_t>(string, 0);
        resize_native_string_header_0041dd40(string, strings, length + old, true);
        std::memcpy(get<char*>(string, 4) + old, chars, length);
    }
    if (chars != nullptr) strings.release(chars, length + 1);
}

void lan_host_record_set_blob88_00783540(Raw record, const void* blob, std::uint32_t size, MpSeg43Hooks& hooks) {
    if (void* const old = get<void*>(record, 0x20)) {
        hooks.free_block(old);
        put<void*>(record, 0x20, nullptr);
    }
    if (size != 0x88) {
        put<void*>(record, 0x20, nullptr);
        return;
    }
    void* const copy = hooks.operator_new(0x88);
    put<void*>(record, 0x20, copy);
    std::memcpy(copy, blob, 0x22 * 4);
}

void session_target_history_destruct_00783950(Raw history, MpSeg43Hooks& hooks) {
    put<std::uint32_t>(history, 0, 0x00D04268u);
    hooks.free_block(get<void*>(history, 0xc)); // _free 00BF6989, no null test
}

void lan_host_record_destruct_00784e00(Raw record, MpSeg43Hooks& hooks) {
    put<std::uint32_t>(record, 0, 0x00D042ECu);
    if (void* const blob = get<void*>(record, 0x20)) {
        hooks.free_block(blob);
        put<void*>(record, 0x20, nullptr);
    }
    // std::string tidy at +4h: heap buffer when capacity +1Ch >= 10h.
    if (get<std::uint32_t>(record, 0x1c) >= 0x10) hooks.free_block(get<void*>(record, 8));
    put<std::uint32_t>(record, 0x1c, 0xf);
    put<std::uint32_t>(record, 0x18, 0);
    put<std::uint8_t>(record, 8, 0);
}

void mnetwork_client_destruct_00784d60(Raw client, MpSeg43Hooks& hooks, ListHooks& lists) {
    put<std::uint32_t>(client, 0, 0x00D042A0u);
    if (void* const lock = get<void*>(client, 0xc0)) {
        while (get<std::int32_t>(lock, 0x18) > 0) lock_leave(lock);
        DeleteCriticalSection(section(lock)); // [00CE2214]
        hooks.free_block(lock);
    }
    if (void* const p = get<void*>(client, 0xb4)) hooks.free_block(p);
    put<std::uint32_t>(client, 0xb4, 0);
    put<std::uint32_t>(client, 0xb8, 0);
    put<std::uint32_t>(client, 0xbc, 0);
    if (void* const p = get<void*>(client, 0xa4)) hooks.free_block(p);
    put<std::uint32_t>(client, 0xa4, 0);
    put<std::uint32_t>(client, 0xa8, 0);
    put<std::uint32_t>(client, 0xac, 0);
    lists.mnetwork_destruct_007844e0(client); // tail JMP
}

Raw mnetwork_construct_00784f90(Raw network, std::uint32_t argument, ListHooks& lists) {
    put<std::uint32_t>(network, 0, 0x00D04274u);
    put<std::uint32_t>(network, 4, argument);
    for (std::size_t list : {std::size_t{0x8}, std::size_t{0x18}, std::size_t{0x28}}) {
        put<void*>(network, list + 4, lists.list_buy_head_node_00784040(at(network, list)));
        put<std::uint32_t>(network, list + 8, 0);
    }
    put<void*>(network, 0x3c, lists.list_buy_head_node_007840b0(at(network, 0x38)));
    put<std::uint32_t>(network, 0x40, 0);
    put<std::uint32_t>(network, 0x14, 0);
    put<std::uint8_t>(network, 0x94, 1);
    put<void*>(network, 0x34, lists.critical_section_create_00bd1860());
    put<void*>(network, 0x24, lists.critical_section_create_00bd1860());
    put<void*>(network, 0x44, lists.critical_section_create_00bd1860());
    put<std::uint32_t>(network, 0x90, 0);
    put<std::uint32_t>(network, 0x8c, 0);
    put<float>(network, 0x48, 0.0f);
    for (std::size_t offset = 0x4c; offset <= 0x88; offset += 4) put<std::uint32_t>(network, offset, 0);
    return network;
}

void mnetwork_queue_incoming_peer_00785790(Raw network, std::uint32_t peer, ListHooks& lists) {
    lock_enter(get<void*>(network, 0x34));
    void* const head = get<void*>(network, 0x2c);
    void* const node = lists.list_buy_node_0076ea10(at(network, 0x28), head, get<void*>(head, 4), &peer);
    lists.list_inc_size_00773600(at(network, 0x28), 1);
    link_new_tail(head, node);
    lock_leave(get<void*>(network, 0x34));
}

Raw mnetwork_client_construct_00785b80(Raw client, std::uint32_t argument, ListHooks& lists) {
    mnetwork_construct_00784f90(client, argument, lists);
    put<std::uint32_t>(client, 0, 0x00D042A0u);
    for (std::size_t offset : {0xa4, 0xa8, 0xac, 0xb4, 0xb8, 0xbc}) put<std::uint32_t>(client, offset, 0);
    put<void*>(client, 0xc0, lists.critical_section_create_00bd1860());
    put<std::uint8_t>(client, 0xc4, 0);
    put<std::uint32_t>(client, 0x9c, 1);
    return client;
}

void mnetwork_append_peer_00785c10(Raw network, std::uint32_t peer, ListHooks& lists) {
    void* const head = get<void*>(network, 0xc);
    void* const node = lists.list_buy_node_0076ea10(at(network, 8), head, get<void*>(head, 4), &peer);
    lists.list_inc_size_00773600(at(network, 8), 1);
    link_new_tail(head, node);
}

void mnetwork_client_push_pending_host_00785ee0(Raw client, std::uint32_t message, ListHooks& lists) {
    lock_enter(get<void*>(client, 0xc0));
    lists.pointer_vector_push_back_00785c50(at(client, 0xb0), &message);
    lock_leave(get<void*>(client, 0xc0));
}

} // namespace bsp::cc13_seg43
