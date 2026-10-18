#pragma once
// Segment 43 (0076AA00..00786C40), packet cc13-seg43: the multiplayer session (MMultiPlayer at
// game+1EF0h), its session messages, the replication helpers, recon-level messages and the
// MNetwork client records. Evidence and per-function ABI: docs/CC13_SEG43.md. Names are
// hypotheses, not recovered symbols.
//
// Objects are the image's own storage, addressed by byte offset. Every call into a body that is
// not reconstructed here goes through MpSeg43Hooks, invoked at the same point with the same
// arguments. Session-message vtables are written as the image's constants (00D0xxxx).

#include <cstddef>
#include <cstdint>

namespace bsp {
class NativeStringStorage;
} // namespace bsp

namespace bsp::cc13_seg43 {

using Raw = void*;

// The two net-id handle tables: id < split ? low table : high table; entry = table base +
// (id - first) * 10h, object pointer at +0Ch.
struct EntityRegistryGlobals {
    const std::int32_t& split_00f89a10;
    const std::int32_t& low_first_00f89a0c;
    std::uint8_t* const& low_table_00f89a54;
    const std::int32_t& high_first_00f89a60;
    std::uint8_t* const& high_table_00f89aa8;
};
// 0076BD70 / 0076BE10 / 0076BEA0 and the inline copy in 00779E20.
void* resolve_net_entity(std::uint32_t id, const EntityRegistryGlobals& registry) noexcept;
// 0076BD70 / 0076BE10 / 0076BEA0 (one copy per message class), __fastcall(ECX msg) RET: the word at +18h.
void* session_message_resolve_entity(const void* message, const EntityRegistryGlobals& registry) noexcept;

// Process-wide words the functions read. The host binds them to its copies of the image globals.
struct MpGlobals {
    std::uint8_t* const& game_00e188a8;          // the game object
    std::uint8_t* const& gui_root_00e198b4;      // +48h: the lobby screen
    std::uint8_t* const& front_end_00e198c4;     // +54h: the local player record
    const std::int32_t& tick_00f876b0;           // the simulation tick
    const float& now_00f876a4;                   // the simulation time
    const std::uint8_t& replay_00f87152;         // a mode byte, read as bool
    std::uint8_t* const& unit_list_head_00f87198; // std::list head node of all units
    std::uint8_t* const& command_registry_00e19a70; // singly linked {?, next +4h, object +8h}
    std::uint8_t* const& scene_driver_00f8a2fc;  // object with vtable slot 168h
    const std::uint8_t* host_info_block_00e1955c; // 38h bytes copied into the host-info blob
    EntityRegistryGlobals registry;
};

struct MpSeg43Hooks {
    virtual ~MpSeg43Hooks() = default;
    // Allocation: operator new 00BF55BE / 00BF681B, _free 00BF6989 / 00BF65AC.
    virtual void* operator_new(std::size_t bytes) = 0;
    virtual void free_block(void* block) = 0;
    // 0075B430 BSP_SessionMessage_ConstructBase, __thiscall(msg, kind) RET 4.
    virtual void session_message_construct_base_0075b430(Raw message, std::uint32_t kind) = 0;
    // CALL [[obj]+slot] variants named by slot.
    virtual void message_delete_virtual_00(Raw message) = 0;               // slot 0, flags 1
    virtual bool message_is_type_virtual_0c(Raw message, std::uint32_t kind) = 0;
    virtual void transport_console_update_virtual_18(Raw console, float dt) = 0;
    virtual void owner_send_virtual_28(Raw owner, Raw message) = 0;
    virtual void unit_release_role_virtual_70(Raw unit, std::uint32_t flag) = 0;
    virtual void unit_set_role_slot_virtual_154(Raw unit, std::uint32_t slot, std::uint32_t value) = 0;
    virtual void unit_apply_create_virtual_98(Raw unit, Raw entity, std::uint32_t game_19cc,
                                              Raw transform) = 0;
    virtual void unit_notify_entity_virtual_ac(Raw unit, Raw entity) = 0;
    virtual bool owner_role_check_virtual_5c(Raw owner, std::uint32_t role) = 0;
    virtual std::uint32_t scene_driver_virtual_168(Raw driver) = 0;
    virtual std::uint32_t command_virtual_04(Raw command) = 0;            // tail call, EAX result
    // Session transport and session routines (ECX = transport or session).
    virtual void transport_broadcast_except_00784790(Raw transport, Raw message, Raw except) = 0;
    virtual void peer_flush_matching_records_007849c0(Raw transport) = 0;
    virtual void transport_kick_peer_00784b10(Raw transport, std::uint32_t peer, std::uint32_t flag) = 0;
    virtual void session_send_to_host_00770af0(Raw session, Raw message) = 0;
    virtual void session_send_to_peer_00770b50(Raw session, std::uint32_t peer, Raw message) = 0;
    virtual void session_set_player_surrender_00770750(Raw session, std::uint32_t player,
                                                       std::uint32_t flag) = 0;
    virtual void session_host_select_player_0076c840(Raw session, std::uint32_t player,
                                                     std::uint32_t value) = 0;
    virtual void session_host_slot_ai_enable_0076cb90(Raw session, std::uint32_t slot,
                                                      std::uint32_t flag) = 0;
    virtual void session_request_player_ready_00771dd0(Raw session, std::uint32_t player,
                                                       std::uint32_t a, std::uint32_t b) = 0;
    virtual void session_set_mode_0076fad0(Raw session, std::uint32_t mode) = 0;
    // 0077F5E0 / 0077C540 are __cdecl and do not read the ECX value passed here (the call
    // site's leftover register); bodies in src/cc13_small2.cpp (docs/CC13_SMALL2.md).
    virtual void replication_drain_child_queues_0077f5e0(Raw session) = 0;
    virtual void game_tuning_get_singleton_0042e740() = 0;
    virtual void replication_flush_queued_rows_0077c540(Raw transport) = 0;
    virtual void replication_step_local_player_record_0077d7f0() = 0;
    virtual void plane_send_sync_ack_batch_007cca50() = 0;
    // Message bodies built elsewhere (ECX = the stack message).
    virtual Raw hint_message_construct_0076f9e0(Raw message, Raw name) = 0;
    virtual void hint_message_destruct_00765d90(Raw message) = 0;
    virtual Raw scene_index_message_construct_00771190(Raw message, std::uint32_t a, std::uint32_t b,
                                                       std::uint32_t c, std::uint32_t d) = 0;
    virtual void scene_index_message_destruct_007662a0(Raw message) = 0;
    virtual void subscene_message_destruct_00766150(Raw message) = 0;
    // Peers, units, lobby.
    virtual std::int32_t game_state_004bca50(Raw game) = 0;
    virtual void game_refresh_after_reassign_004d56e0(Raw game) = 0;
    virtual void peer_mark_incompatible_007f7450(Raw peer_state) = 0;
    virtual bool peer_scene_checked_007f7470(Raw peer_state) = 0;
    virtual bool peer_should_kick_007f7480(Raw peer_state) = 0;
    virtual void game_show_chat_text_004c8950(Raw game, const char* text) = 0;
    virtual void lobby_append_chat_005d92b0(Raw lobby, Raw message, std::uint32_t arg) = 0;
    virtual void lobby_refresh_005e5ae0(Raw lobby) = 0;
    virtual void lobby_refresh_scene_005d9d60(Raw lobby) = 0;
    virtual Raw unit_owner_00923810(Raw unit, std::uint32_t which) = 0;
    virtual Raw transform_set_identity_0075b500(Raw source, Raw out) = 0;
    // Lua property plumbing (00779BB0).
    virtual void entity_get_property_unit_command_00927ad0(Raw unit, Raw lua, Raw name) = 0;
    virtual void lua_object_new_table_00b67930(Raw lua, Raw object) = 0;
    virtual void lua_object_set_integer_00b665d0(Raw object, std::uint32_t index, std::uint32_t value) = 0;
    virtual void lua_object_destruct_00b67700(Raw object) = 0;
    // Bit cursor (message write/read).
    virtual void message_write_extended_header_0075b480(Raw message, Raw stream) = 0;
    virtual void message_read_extended_header_0075b4c0(Raw message, Raw stream) = 0;
    virtual void bit_write_bool_004290b0(Raw stream, std::uint32_t value) = 0;
    virtual void bit_write_dword_00429070(Raw stream, std::uint32_t value, std::uint32_t bits) = 0;
    virtual void bit_write_float_array_00429790(Raw stream, Raw values, std::uint32_t count,
        std::uint32_t a, std::uint32_t b, float limit, std::uint32_t bits) = 0;
    virtual void bit_read_bool_00428d70(Raw cursor, Raw out) = 0;
    virtual void bit_read_dword_00428d10(Raw cursor, Raw out, std::uint32_t bits) = 0;
    virtual void bit_read_float_array_004294f0(Raw cursor, Raw values, std::uint32_t count,
        std::uint32_t a, std::uint32_t b, float limit, std::uint32_t bits) = 0;
    // 00443CB0 BSP_NativeString_LessCaseInsensitive, __thiscall(ECX left, right) RET 4.
    virtual bool native_string_less_ci_00443cb0(const void* left, const void* right) = 0;
    // 0077B0C0 callees.
    virtual void recon_party_touch_00803ba0(std::uint32_t party) = 0;                  // ECX = party
    virtual Raw entity_lua_this_table_00927b40(Raw unit, Raw out) = 0;                  // returns LuaObject*
    virtual Raw lua_object_get_field_00b67800(Raw object, Raw out, const char* name) = 0; // returns LuaObject*
    virtual void recon_manager_notify_00980e50(Raw manager, std::uint32_t party, Raw unit,
                                               std::uint32_t arg, std::int32_t level) = 0;
    virtual void player_record_recon_changed_005a31a0(Raw player_record, Raw unit) = 0;    // 0043BDF0, __thiscall(NativeString* out, float value) RET 4: the value formatted with "%f".
    virtual void native_string_from_float_0043bdf0(Raw out, float value) = 0;
};

// ---- session (ECX = MMultiPlayer, game+1EF0h) ----------------------------------------------
void session_update_transport_0076aa00(Raw session, float dt, MpSeg43Hooks& hooks);
void session_alloc_host_info_blob_0076c2c0(Raw session, Raw* out_blob, std::uint32_t* out_size,
                                            const MpGlobals& globals, MpSeg43Hooks& hooks);
std::uint32_t session_classify_join_request_0076c340(Raw session, const void* info) noexcept;
void session_clear_receive_queue_0076c5a0(Raw session, MpSeg43Hooks& hooks);
void session_broadcast_multi_settings_0076c9c0(Raw session, const void* settings, MpSeg43Hooks& hooks);
void send_pending_selection_0076cf30(Raw owner, std::uint32_t slot, const void* unit, MpSeg43Hooks& hooks);
void session_purge_net_entity_messages_0076da60(Raw session, MpSeg43Hooks& hooks);
void host_info_append_blob_0076e3c0(Raw owner, std::uint32_t blob, MpSeg43Hooks& hooks);
bool lan_host_key_less_0076eaa0(const void* left, const void* right, MpSeg43Hooks& hooks);
void session_step_entity_replication_0076f210(Raw session, std::uint32_t unused, bool flag,
                                              const MpGlobals& globals, MpSeg43Hooks& hooks);
void session_reassign_peer_units_0076f2b0(std::uint32_t player, const MpGlobals& globals,
                                          MpSeg43Hooks& hooks);
void session_collect_incompatible_clients_0076f360(Raw session, MpSeg43Hooks& hooks);
std::uint32_t session_all_peers_scene_checked_0076f400(Raw session, const MpGlobals& globals,
                                                       MpSeg43Hooks& hooks);
void session_send_hint_message_00770870(Raw session, Raw name, MpSeg43Hooks& hooks);
void session_show_chat_text_007708e0(Raw session, Raw message, std::uint32_t arg,
                                     const MpGlobals& globals, MpSeg43Hooks& hooks);
void session_set_mode_host_00770e40(Raw session, MpSeg43Hooks& hooks);
void session_request_select_player_007718f0(Raw session, std::uint32_t a, std::uint32_t b,
                                            MpSeg43Hooks& hooks);
void session_request_slot_ai_enable_00771e80(Raw session, std::uint32_t slot, std::uint8_t flag,
                                             MpSeg43Hooks& hooks);
void session_request_player_surrender_00771f20(Raw session, std::uint32_t player, std::uint8_t flag,
                                               MpSeg43Hooks& hooks);
void session_broadcast_scene_index_00772390(Raw session, const MpGlobals& globals, MpSeg43Hooks& hooks);
void session_broadcast_sub_scene_00772430(Raw session, const void* name, std::uint32_t value,
                                          MpSeg43Hooks& hooks, NativeStringStorage& strings);
void session_client_send_local_score_00772a40(Raw session, const MpGlobals& globals, MpSeg43Hooks& hooks);
void session_host_clear_all_selections_007735b0(Raw session, const MpGlobals& globals,
                                                MpSeg43Hooks& hooks);

// ---- records and messages ---------------------------------------------------------------------
void optional_buffer_destruct_0076b200(Raw buffer, MpSeg43Hooks& hooks);
Raw message23_construct_from_settings_0076bb60(Raw message, const void* settings, MpSeg43Hooks& hooks);
Raw message11_construct_with_value_0076bc40(Raw message, std::uint32_t value, MpSeg43Hooks& hooks);
Raw lan_host_record_copy_construct_00770fa0(Raw record, const void* source, NativeStringStorage& strings);
Raw message18_build_subscene_007710e0(Raw message, const void* name, std::uint32_t value,
                                      MpSeg43Hooks& hooks, NativeStringStorage& strings);
Raw message_roleowner_construct_007796f0(Raw message, std::uint32_t a, std::uint32_t b,
                                         std::uint32_t c, std::uint8_t d, MpSeg43Hooks& hooks);
Raw message_entity_release_construct_00779780(Raw message, std::uint32_t payload, MpSeg43Hooks& hooks);
Raw message_reconlevel_construct_00779850(Raw message, const void* unit, std::int32_t party,
                                          MpSeg43Hooks& hooks);
void message_formation_set_write_0077a1a0(Raw message, Raw stream, MpSeg43Hooks& hooks);
void message_formation_set_read_0077a220(Raw message, Raw stream, MpSeg43Hooks& hooks);

// ---- units, replication ---------------------------------------------------------------------
// 0077B0C0, __thiscall(ECX = unit+1E4h, party, arg, level) RET 0Ch.
void unit_set_recon_level_0077b0c0(Raw recon, std::uint32_t party, std::uint32_t arg, std::int32_t level,
    std::uint8_t* const& recon_manager_00f8a0c4, const MpGlobals& globals, MpSeg43Hooks& hooks);
bool unit_is_local_players_role_slot_00779b60(const void* unit, const MpGlobals& globals) noexcept;
void unit_get_property_recon_level_00779bb0(Raw unit, Raw lua, Raw name, MpSeg43Hooks& hooks);
void unit_update_owner_hud_state_00779c60(Raw unit, float value, const MpGlobals& globals,
                                          MpSeg43Hooks& hooks);
std::int32_t unit_recon_record_value_00779d00(const void* unit, const MpGlobals& globals) noexcept;
void replication_resolve_pending_create_00779e20(Raw unit, const MpGlobals& globals, MpSeg43Hooks& hooks);
void net_entity_holder_stamp_sync_tick_00779f80(Raw holder, const MpGlobals& globals) noexcept;
void entity_send_message_to_host_00779f90(const void* entity, Raw message, const MpGlobals& globals,
                                          MpSeg43Hooks& hooks);
void entity_send_message_to_peer_00779fc0(const void* entity, const void* peer, Raw message,
                                          const MpGlobals& globals, MpSeg43Hooks& hooks);
bool message_is_privileged_kind_00779ff0(Raw message, MpSeg43Hooks& hooks);
std::uint32_t command_registry_dispatch_by_id_0077a020(const void* message, const MpGlobals& globals,
                                                       MpSeg43Hooks& hooks);
Raw command_registry_find_by_id_0077a050(const void* message, const MpGlobals& globals) noexcept;

// ---- role slots (units) ----------------------------------------------------------------------
struct RoleHooks {
    virtual ~RoleHooks() = default;
    virtual bool unit_is_kind_virtual_5c(Raw unit, std::uint32_t kind) = 0;          // [[unit]+5Ch]
    virtual void unit_set_role_owner_virtual_148(Raw unit, std::uint32_t a, std::uint32_t b) = 0;
    virtual void unit_set_role_slot_virtual_154(Raw unit, std::uint32_t slot, std::uint32_t value) = 0;
    // 0077EDF0 BSP_MessageSystem_EntitySuppressSlot, __thiscall(map, const key*) RET 4: byte slot.
    virtual std::uint8_t* suppress_slot_0077edf0(Raw map, const void* key) = 0;
    // The base-class bodies the five role wrappers forward to, __thiscall on the unit.
    virtual void unit_role_009278a0(Raw unit, std::uint32_t a) = 0;
    virtual void unit_role_00927d20(Raw unit, std::uint32_t a, std::uint32_t b) = 0;
    virtual void unit_role_00927810(Raw unit, std::uint32_t a, std::uint32_t b) = 0;
    virtual void unit_role_009281c0(Raw unit, std::uint32_t a, std::uint32_t b) = 0;
    virtual void unit_role_00927e50(Raw unit, std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    // 0077C2A0 BSP_Session_RouteMessage, __thiscall(unit, msg, a, b) RET 0Ch.
    virtual void route_message_0077c2a0(Raw unit, Raw message, std::uint32_t a, std::uint32_t b) = 0;
    // 0077C7B0, __thiscall(entity, msg, flag) RET 8: the entity's message send.
    virtual void entity_send_0077c7b0(Raw entity, Raw message, std::uint32_t flag) = 0;
};
void unit_apply_role_owner_slots_0077a5d0(Raw unit, const MpGlobals& globals, RoleHooks& hooks);
// The shared tail: mark the unit in the suppress map [00F8A0C4]+D4h when it is a kind-5 unit that
// is neither kind 1Ch nor 1Bh, whose role owner +188h is not 9 or the local player, and whose
// party +54h equals the local player record's +28h.
void unit_assign_single_device_role_0077f2d0(Raw unit, std::uint32_t a, std::uint8_t* const& role_manager_00f8a0c4,
                                             const MpGlobals& globals, RoleHooks& hooks);
void unit_set_role_available_0077f360(Raw unit, std::uint32_t a, std::uint32_t b,
                                      std::uint8_t* const& role_manager_00f8a0c4, const MpGlobals& globals,
                                      RoleHooks& hooks);
void unit_set_role_owner_and_refresh_0077f3f0(Raw unit, std::uint32_t a, std::uint32_t b,
                                              std::uint8_t* const& role_manager_00f8a0c4,
                                              const MpGlobals& globals, RoleHooks& hooks);
void unit_set_role_slot_and_refresh_0077f480(Raw unit, std::uint32_t a, std::uint32_t b,
                                             std::uint8_t* const& role_manager_00f8a0c4,
                                             const MpGlobals& globals, RoleHooks& hooks);
void unit_send_role_owner_and_refresh_0077f510(Raw unit, std::uint32_t a, std::uint32_t b,
                                               std::uint32_t c, std::uint8_t* const& role_manager_00f8a0c4,
                                               const MpGlobals& globals, RoleHooks& hooks);
// 0077C470, __thiscall(unit, a, b) RET 8: MT_ROLEOWNER (4Bh) routed through 0077C2A0.
void unit_send_role_transfer_0077c470(Raw unit, std::uint32_t a, std::uint32_t b, const MpGlobals& globals,
                                      MpSeg43Hooks& hooks, RoleHooks& role_hooks);
// 0077D580, __thiscall(entity, flag) RET 4: when flag is 0, 52h message through 0077C7B0.
void entity_send_activate_0077d580(Raw entity, std::uint8_t flag, MpSeg43Hooks& hooks, RoleHooks& role_hooks);
Raw entity_as_kind_1b_0077bcf0(Raw entity, RoleHooks& hooks);

// ---- records, lists, MNetwork ------------------------------------------------------------------
struct ListHooks {
    virtual ~ListHooks() = default;
    // STL list buy-node / size-increment helpers, __thiscall(list, ...).
    virtual Raw list_buy_node_0077bb00(Raw list, Raw next, Raw prev, const void* value) = 0;
    virtual Raw list_buy_node_0076ea10(Raw list, Raw next, Raw prev, const void* value) = 0;
    virtual void list_inc_size_0077dd00(Raw list, std::uint32_t n) = 0;
    virtual void list_inc_size_0077de40(Raw list, std::uint32_t n) = 0;
    virtual void list_inc_size_00773600(Raw list, std::uint32_t n) = 0;
    virtual Raw list_buy_head_node_00784040(Raw list) = 0;
    virtual Raw list_buy_head_node_007840b0(Raw list) = 0;
    virtual void pointer_vector_push_back_00785c50(Raw vector, const void* value) = 0;
    virtual Raw critical_section_create_00bd1860() = 0;
    virtual void mnetwork_destruct_007844e0(Raw network) = 0;
    virtual void score_holder_member_destruct_00884af0(Raw member) = 0;
    virtual void entity_register_in_parent_00928560(Raw entity) = 0;
    virtual void entity_unregister_from_parent_00928570(Raw entity) = 0;
    virtual void unit_list_push_back_00484540(Raw list, Raw entity) = 0;
    virtual void unit_list_erase_node_004837d0(Raw list, Raw node) = 0;
    virtual void observer_unregister_pair_006952a0(Raw first, Raw owner) = 0;
    virtual void observer_register_pair_00694a60(Raw first, Raw owner) = 0;
};
// 0077BA60, __stdcall(next, prev, const 10h-byte value*) RET 0Ch: 18h-byte list node.
Raw net_entity_peer_holder_buy_node_0077ba60(Raw next, Raw prev, const void* value, MpSeg43Hooks& hooks);
// 0077EB50, __thiscall(entity, peer) RET 4.
void net_entity_add_peer_holder_0077eb50(Raw entity, std::uint32_t peer, std::uint8_t& dirty_00f87190,
                                         MpSeg43Hooks& hooks, ListHooks& lists);
// 0077F090, __thiscall(entity) RET.
void replication_flag_remote_players_0077f090(Raw entity, const MpGlobals& globals,
                                              std::uint8_t& dirty_00f87190, MpSeg43Hooks& hooks,
                                              ListHooks& lists);
Raw attribution_ref_assign_0077cc80(Raw reference, const void* source, ListHooks& lists);
Raw message_setguiname_construct_0077ccc0(Raw message, std::uint32_t length, char* chars, std::uint8_t flag,
                                          MpSeg43Hooks& hooks, NativeStringStorage& strings);
void multi_score_holder_destruct_00780c50(Raw holder, ListHooks& lists, NativeStringStorage& strings);
void entity_register_in_world_lists_007811d0(Raw entity, ListHooks& lists);
void entity_detach_from_world_lists_00781200(Raw entity, ListHooks& lists);
// 00781740, __thiscall(NativeString*, float) RET 4: append the 0043BDF0 text.
void native_string_append_float_00781740(Raw string, float value, MpSeg43Hooks& hooks, NativeStringStorage& strings);
void lan_host_record_set_blob88_00783540(Raw record, const void* blob, std::uint32_t size, MpSeg43Hooks& hooks);
void session_target_history_destruct_00783950(Raw history, MpSeg43Hooks& hooks);
void lan_host_record_destruct_00784e00(Raw record, MpSeg43Hooks& hooks);
void mnetwork_client_destruct_00784d60(Raw client, MpSeg43Hooks& hooks, ListHooks& lists);
Raw mnetwork_construct_00784f90(Raw network, std::uint32_t argument, ListHooks& lists);
void mnetwork_queue_incoming_peer_00785790(Raw network, std::uint32_t peer, ListHooks& lists);
Raw mnetwork_client_construct_00785b80(Raw client, std::uint32_t argument, ListHooks& lists);
void mnetwork_append_peer_00785c10(Raw network, std::uint32_t peer, ListHooks& lists);
void mnetwork_client_push_pending_host_00785ee0(Raw client, std::uint32_t message, ListHooks& lists);

inline constexpr std::size_t kSessionMessageSmall = 0x20;

} // namespace bsp::cc13_seg43
