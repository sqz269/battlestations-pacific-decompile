// cc13 small-row packet 2 (packet cc13-small2): the game bodies left without a record in
// segments 8 7 43 26 13 19 5 27 9 12 20 1 23 66 3 15 that no host or earlier cc13 file
// already carries. docs/CC13_SMALL2.md has the triage, the evidence and the adoption notes
// against cc9's hosts.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in
// the image) and reaches the engine through Small2Host, whose methods name the image routine or
// virtual slot they stand for and give the register inputs. Constants carry their image address
// and width. New C++ interfaces, not drop-in binary replacements; nothing here is bound into
// bsp_game. Names are hypotheses, not recovered symbols.

#ifndef BSP_CC13_SMALL2_HPP
#define BSP_CC13_SMALL2_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_small2 {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGame = 0x00E188A8;            // the game object pointer
inline constexpr std::uint32_t kControlledUnit = 0x00E188D8;  // the player's unit pointer
inline constexpr std::uint32_t kFrontEnd = 0x00E198C4;        // the front-end manager pointer
inline constexpr std::uint32_t kClock = 0x00F876A4;           // float fixed-step clock
inline constexpr std::uint32_t kTick = 0x00F876B0;            // int32 tick counter
inline constexpr std::uint32_t kNetEntityList = 0x00F87194;   // std::list, head at 00F87198
inline constexpr std::uint32_t kNetEntityListHead = 0x00F87198;
inline constexpr std::uint32_t kNetChildQueuePending = 0x00F87190;  // byte, cleared by 0077F5E0
inline constexpr std::uint32_t kNetTraceEnabled = 0x00E0AF20; // byte read by 0077F5E0
inline constexpr std::uint32_t kEmptyText = 0x00F87158;       // "" used for a null name
inline constexpr std::uint32_t kDefaultCommandPosition = 0x00F87574;  // three floats (.bss)
inline constexpr std::uint32_t kRotatingSlot = 0x00E188C4;    // int32 slot index, 004C3C2E
inline constexpr std::int32_t kGameModeSlotScan = 0x0D;       // game+5D4h, 004C3A86
inline constexpr std::uint32_t kMsgFormationSet = 0x78;        // 0075B430 type, 0077A09B
inline constexpr std::uint32_t kMsgFormationSetVtable = 0x00CED548;  // 0077A0BD
inline constexpr std::uint32_t kLastBantoName = 0x00D03E2C;     // "LastBanto", 0077D32B
inline constexpr std::uint32_t kWarningManager = 0x00F8A0C4;  // pointer read by 0064A270

// 0077C540: 64 buckets x 768 queued rows; counts at 00F1B050, bytes at 00F1B150 (300h a
// bucket), receivers at 00F57150 and arguments at 00F27150 (C00h a bucket).
inline constexpr std::uint32_t kFlushCounts = 0x00F1B050;
inline constexpr std::uint32_t kFlushBytes = 0x00F1B150;
inline constexpr std::uint32_t kFlushArgs = 0x00F27150;
inline constexpr std::uint32_t kFlushReceivers = 0x00F57150;
inline constexpr int kFlushBuckets = 64;                      // REP STOSD ECX = 40h
inline constexpr std::uint32_t kFlushPhase = 0x00F87150;      // byte = tick % 12

inline constexpr std::int32_t kSessionModeServer = 1;         // game+1FE4h
inline constexpr std::int32_t kSessionModeClient = 2;         // game+1FE4h
inline constexpr std::int32_t kChildKindQueued = 8;           // [[elem+50h]+0Ch], 0077C5A0
inline constexpr double kMinDistanceSquared = 1e-10;          // double 00CE3820, 00779F3D
inline constexpr float kTickSeconds = 0.05f;                  // float 00D0DE84, 0077F7D2
inline constexpr std::int32_t kGameModeDelayAlt = 8;          // 004BCA50 result, 0077F7E1
inline constexpr std::uint32_t kRoleMaskAll = 0x1FF;          // 0077F80F / 0077F82F
inline constexpr std::uint32_t kOrderClearOrders = 0x00E08F08;  // `clearorders`, 0077F8BD
inline constexpr std::uint32_t kOrderStop = 0x00E08F88;         // `stop`, 0077F915
inline constexpr std::uint32_t kInterfaceAfterExpiry = 0x34;    // 004CC460 id, 0077F86A
inline constexpr std::int32_t kLimboStateExpired = 3;           // 00565FB0 arg, 0077F84C
inline constexpr std::int32_t kKillReasonExpired = 3;           // 00926D90 arg, 0077A67F
inline constexpr std::int32_t kUnitKind05 = 5;                   // vtable[5Ch] arg, 004C3B2C
inline constexpr std::int32_t kUnitKind06 = 6;                  // vtable[5Ch] arg, 005F98FB
inline constexpr std::int32_t kUnitKind09 = 9;                   // vtable[5Ch] arg, 005F990C
inline constexpr std::int32_t kUnitKind0F = 0x0F;               // vtable[5Ch] arg, 0077D884
inline constexpr std::int32_t kUnitKind18 = 0x18;               // vtable[5Ch] arg, 0077D89F
inline constexpr std::uint32_t kThisTableName = 0x00CE7494;     // "thisTable"
inline constexpr std::uint32_t kSetGuiNameFormat = 0x00D03E38;  // "--- SETGUINAME: %s -> %s"

// The engine's 8-byte string {uint32 length; char* data} (0041DD40 et al.).
struct NativeString {
    std::uint32_t length;
    char* data;
};

// The 14h-byte Lua object handle (00B67980 / 00B67800 / 00B68100 / 00B67700).
struct LuaObject {
    std::uint8_t bytes[0x14];
};

// The 18h-byte command target descriptor built by 0077F7B0 (00720450 / 006F7DD0 layout):
// +0 kind byte, +1 has-entity byte, +2 word, +4 entity, +8 point, +14h float.
struct TargetDescriptor {
    std::uint8_t kind;
    std::uint8_t has_entity;
    std::uint16_t pad2;
    std::uint32_t entity;
    float point[3];
    float extra;
};
static_assert(sizeof(TargetDescriptor) == 0x18, "TargetDescriptor");

// 0077B050's out record: a point and a valid byte at +0Ch.
struct TargetPoint {
    float point[3];
    std::uint8_t valid;
};

struct Small2Host {
    virtual ~Small2Host() = default;

    // The dword / float / byte at image global `address`, and a pointer to it.
    virtual void* global_ptr(std::uint32_t address) = 0;
    virtual void* global_address(std::uint32_t address) = 0;
    // 00BF6713, the checked-iterator invalid-parameter report.
    virtual void invalid_parameter_00bf6713() = 0;

    // Entity virtual slots (ECX = entity).
    virtual void* entity_weapon_director_114(void* entity) = 0;
    virtual void* entity_command_target_178(void* entity, void* scratch) = 0;
    virtual bool entity_is_kind_5c(void* entity, std::int32_t kind) = 0;
    virtual void entity_queue_child_180(void* entity, void* element) = 0;
    virtual void entity_send_local_record_184(void* entity, void* element_or_null) = 0;
    virtual void entity_trace_child_17c(void* entity, void* child) = 0;
    virtual void entity_clear_roles_148(void* entity, std::uint32_t mask, std::int32_t arg) = 0;
    virtual const char* entity_name_10(void* entity) = 0;
    // receiver->vtable[188h](arg, byte), 0077C62A.
    virtual void net_flush_row_188(void* receiver, std::uint32_t arg, std::uint8_t byte) = 0;

    // Direct calls.
    virtual void* weapon_director_slot_path_object_0071bfc0(void* director, std::int32_t i) = 0;
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    virtual const float* target_descriptor_resolve_position_006f7dd0(void* descriptor,
                                                                     void* out) = 0;
    virtual void session_owner_broadcast_0077c7b0(void* owner, void* msg, std::int32_t arg) = 0;
    virtual void session_send_secondary_first_peer_00770af0(void* session, void* msg) = 0;
    virtual void native_string_from_int_004260b0(NativeString* out, std::int32_t value) = 0;
    virtual void native_string_assign_0041dd40(NativeString* dst, const NativeString& src) = 0;
    virtual void native_string_free_storage(NativeString* s) = 0;   // 00419CC0 / 00BD1510
    virtual void* lua_globals_00b67980(void* state_owner, LuaObject* out) = 0;
    virtual void* lua_get_by_name_00b67800(void* table, LuaObject* out, const char* name) = 0;
    virtual void* lua_get_by_native_string_00b68100(void* table, LuaObject* out,
                                                    const NativeString* key) = 0;
    virtual void lua_destroy_00b67700(LuaObject* obj) = 0;
    // The SETGUINAME message (0077CCC0 construct, 00765B70 destroy) and the CAMERA message
    // (00764960 construct, 0075B420 destroy); `msg` is caller storage.
    virtual void* msg_setguiname_construct_0077ccc0(void* msg, const NativeString& name,
                                                    std::uint32_t arg) = 0;
    virtual void msg_setguiname_destroy_00765b70(void* msg) = 0;
    virtual void* msg_camera_construct_00764960(void* msg, std::uint32_t target,
                                                bool hud_flag) = 0;
    virtual void msg_camera_destroy_0075b420(void* msg) = 0;
    virtual void log_noop_004254b0(const char* format, const char* a, const char* b) = 0;
    virtual bool unit_is_local_player_role_00927f30(void* unit, std::int32_t role) = 0;
    virtual std::int32_t game_effective_mode_004bca50(void* game) = 0;
    virtual void* global_config_00432650() = 0;
    virtual void unit_send_role_transfer_0077c470(void* unit, std::uint32_t mask,
                                                  std::int32_t arg) = 0;
    virtual void limbo_screen_set_state_00565fb0(void* screen, std::int32_t state) = 0;
    virtual void front_end_push_interface_004cc460(void* front_end, std::int32_t id,
                                                   std::uint32_t payload) = 0;
    virtual void entity_issue_command_0077d600(void* entity, std::uint32_t order,
                                               const TargetDescriptor* target,
                                               std::int32_t flags) = 0;
    virtual void expiry_notice_0090ebf0(void* registry, void* unit) = 0;
    virtual void mission_entity_kill_00926d90(void* unit, std::int32_t reason) = 0;
    virtual void crt_free_00bf65ac(void* block) = 0;
    // 00645160 __fastcall(ECX = unit, DL = flag).
    virtual bool unit_is_selectable_for_current_player_00645160(void* unit, bool flag) = 0;
    virtual void hud_root_set_spectated_unit_00647300(void* hud_root, void* unit) = 0;
    virtual void* hud_get_controlled_unit_00644230(void* hud_root) = 0;
    // widget->vtable[34h](byte), 004C3BE9.
    virtual void widget_set_flag_34(void* widget, bool flag) = 0;
    virtual void session_message_construct_base_0075b430(void* msg, std::uint32_t type) = 0;
    virtual void* operator_new_array_00bf55be(std::uint32_t bytes) = 0;
    virtual void* group_member_record_at_0070d070(void* group, std::uint32_t index) = 0;
    // 00803BA0 __fastcall(ECX = index).
    virtual void recon_mark_slot_dirty_00803ba0(std::int32_t index) = 0;
    virtual void* mission_entity_lock_get_004c1570() = 0;
    // Imports through [00CE2218] / [00CE2210] (EnterCriticalSection / LeaveCriticalSection).
    virtual void enter_critical_section_iat_00ce2218(void* section) = 0;
    virtual void leave_critical_section_iat_00ce2210(void* section) = 0;
    virtual void native_string_resize_0041dd40(NativeString* s, std::uint32_t length,
                                               std::int32_t keep) = 0;
    virtual void* mission_entity_lua_self_object_00927b40(void* entity, LuaObject* out) = 0;
    virtual void lua_set_object_00b675d0(void* table, const NativeString* name, void* value) = 0;
    virtual void mission_entity_on_killed_lua_00929b60(void* entity) = 0;
    // 0077FE80's arms.
    virtual bool message_is_ready_10(void* msg) = 0;                        // msg->vtable[10h]
    virtual void entity_claim_player_144(void* entity, std::uint32_t player) = 0;
    virtual void warning_manager_report_new_unit_00977cd0(void* manager, std::uint32_t player) = 0;
    virtual void unit_group_release_member_0077c980(void* unit, void* other) = 0;
    virtual void unit_apply_role_owner_slots_0077a5d0(void* unit) = 0;
    virtual void entity_apply_message_payload_0c(void* entity, void* payload,
                                                 std::uint8_t flag) = 0;  // vtable[0Ch]
    // 00521E30 __fastcall(CX = id).
    virtual void* entity_handle_resolve_00521e30(std::uint16_t id) = 0;
    virtual void unit_group_join_or_merge_0077f940(void* unit, void* target) = 0;
    virtual void unit_group_hand_over_leadership_0077bd70(void* unit, void* successor) = 0;
    virtual void unit_group_apply_formation_shape_0070efd0(void* group, std::uint32_t shape) = 0;
    virtual bool entity_generic_message_164(void* entity, void* msg) = 0;   // vtable[164h]
    virtual bool unit_is_selectable_00645060(void* unit, std::int32_t team,
                                             bool allow_spectate) = 0;
    virtual void hud_root_set_controlled_unit_00645600(void* hud_root, void* unit) = 0;
    virtual void hud_root_force_select_unit_006485a0(void* hud_root) = 0;
    virtual void warning_manager_repair_channel_00983c10(void* manager, std::uint32_t entity,
                                                         std::int32_t setting) = 0;
};

// Segment 43.
void* entity_weapon_director_ref_construct_00778820(void* holder, void* entity,
                                                   Small2Host& host);
void* unit_director_path_object0_00778860(void* unit, Small2Host& host);
void* session_message_formation_set_construct_0077a080(void* msg, void* group,
                                                       Small2Host& host);
void unit_instance_on_killed_lua_0077d270(void* unit, Small2Host& host);
float unit_distance_to_record_00779eb0(void* entity, void* const* record, Small2Host& host);
void unit_expire_at_scheduled_time_0077a650(void* unit, Small2Host& host);
void entity_update_command_target_position_0077b050(void* entity, TargetPoint* out,
                                                    Small2Host& host);
void replication_flush_queued_rows_0077c540(Small2Host& host);
void unit_group_broadcast_formation_0077c880(void* owner, void* msg, Small2Host& host);
LuaObject* unit_lua_this_table_entry_0077cd80(void* unit, LuaObject* out, Small2Host& host);
void entity_set_gui_name_0077d3f0(void* entity, const NativeString* name, std::uint32_t arg,
                                  Small2Host& host);
void replication_step_local_player_record_0077d7f0(Small2Host& host);
void replication_drain_child_queues_0077f5e0(Small2Host& host);
bool session_dispatch_entity_kind_message_0077fe80(void* entity, void* msg, bool* out,
                                                   Small2Host& host);
void unit_apply_expiry_message_0077f7b0(void* unit, const void* msg, Small2Host& host);

// Segment 26.
void hud_root_listener_current_0c_00645bb0(void* listener, void* unit, std::int32_t event,
                                           void* candidate, Small2Host& host);
void hud_root_ensure_selectable_controlled_unit_00648930(void* hud_root, Small2Host& host);
void repair_task_set_repair_priority_0064a270(void* task, std::int32_t priority,
                                              Small2Host& host);

// Segment 9.
void game_update_player_slot_unit_flags_004c3a80(void* game, Small2Host& host);

// Segment 23.
bool unit_within_friendly_capture_radius_005f98f0(void* unit, Small2Host& host);

}  // namespace bsp::cc13_small2

#endif  // BSP_CC13_SMALL2_HPP
