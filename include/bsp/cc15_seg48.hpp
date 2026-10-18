#pragma once

// cc15_seg48: the live, unbuilt game bodies of segment 48 (007D1E50-007F8390) left after cc13's
// seg48/seg48U passes and cc9's hosts (packet cc15-r48a, docs/CC15_SEG48.md): the plane-neighbours
// line-of-sight test, the plane squadron's member predicates, its create/attack-run session
// messages, command-record initialisers, the destructor, home-base and stock bookkeeping, the
// archive writers, the creation-record pass, position/heading/matrix setters and the kill-reason
// counter. Reconstructed and build-tested only: these are new C++ interfaces over the native
// records, not drop-in replacements; nothing is wired into a game host. Names are hypotheses, not
// recovered symbols. Same conventions as cc13_seg48U: raw byte blocks touched only at the offsets
// the image uses, and one named host hook per native call, at the same point with the same
// arguments. Exception unwinding (the EH frames the image sets up) is not modelled: the hooks are
// assumed not to throw.
//
// x87: the bodies run on the main thread, whose precision control is 24-bit
// (docs/CC13_FPU_PRECISION.md). Each x87 FADD/FSUB/FMUL/FDIV therefore rounds to a float, which
// is what the float arithmetic below does; at 53-bit precision control the unspilled
// intermediates would differ (named per function). FLD float / FSTP float pairs quiet a
// signalling NaN (x87_float_round_trip); MOVSS copies do not.

#include "bsp/cc13_seg48U.hpp"

#include <cstdint>

namespace bsp {

class Cc15Seg48Host : public Cc13Seg48UHost {
public:
    // ---- geometry and world ----
    // 0042B2F0 BSP_Vector3_LengthFloatThreshold: __thiscall(ECX = const float[3]), ST0 (a float).
    virtual float vector3_length_float_threshold_0042b2f0(const float* v) = 0;
    // 0071C4F0 BSP_Game_PointOutsideMapBounds: __thiscall(ECX = [00E188A8] game, const float[3]),
    // RET 4, AL.
    virtual bool game_point_outside_map_bounds_0071c4f0(void* game, const float* point) = 0;
    // 00489760 (the base position set): __thiscall(ECX = entity, const float[3]), RET 4.
    virtual void entity_set_world_position_00489760(void* entity, const float* position) = 0;
    // 0047F200 (the base heading set): __thiscall(ECX = entity, float heading), RET 4.
    virtual void entity_set_heading_0047f200(void* entity, float heading) = 0;
    // 0042ED50 (the child-node refresh the setters run on every child): __thiscall(ECX = child).
    virtual void scene_child_invalidate_0042ed50(void* child) = 0;
    // 004C17D0 (the avoid-zone layer manager getter): no input, EAX.
    virtual void* avoid_zone_layer_manager_004c17d0() = 0;
    // 0041DF40 (the layer selector): __thiscall(ECX = manager; float slope, int flag), RET 8, EAX.
    virtual std::uint32_t avoid_zone_layer_select_0041df40(void* manager, float slope, std::int32_t flag) = 0;

    // ---- entities, units and squadrons ----
    // 00927F30 BSP_UnitInstance_IsLocalPlayerRole: __thiscall(ECX = unit, int role), RET 4, AL.
    virtual bool unit_instance_is_local_player_role_00927f30(void* unit, std::int32_t role) = 0;
    // Squadron vtable +174h (the current command class): __thiscall(ECX = squadron), EAX = the
    // class singleton's image address (00E08F18..00E08F90).
    virtual std::uint32_t squadron_vcall_174_command_class(void* squadron) = 0;
    // Squadron vtable +178h (the current command target): __thiscall(ECX = squadron, out record),
    // RET 4, EAX = a record whose byte +1h says a target is present and whose +8h is its point.
    virtual const void* squadron_vcall_178_command_target(void* squadron, void* out) = 0;
    // 0071BFF0 BSP_WeaponDirector_CommandPathCursor: __thiscall(ECX = director, int), RET 4, EAX.
    virtual void* weapon_director_command_path_cursor_0071bff0(void* director, std::int32_t index) = 0;
    // 007B1350 BSP_PathCursor_FollowModeLastPoint: __thiscall(ECX = cursor, out {float[3], byte
    // valid at +0Ch}), RET 4.
    virtual void path_cursor_follow_mode_last_point_007b1350(void* cursor, void* out) = 0;
    // A task's vtable +1Ch / +20h (break-off / dive-or-torpedo answers): __thiscall(ECX = task), AL.
    virtual bool task_vcall_1c(void* task) = 0;
    virtual bool task_vcall_20(void* task) = 0;
    // 007EEE50 BSP_PlaneSquadron_RequestLeaderPromotionMessage: __thiscall(ECX = squadron,
    // member), RET 4.
    virtual void squadron_request_leader_promotion_007eee50(void* squadron, void* member) = 0;
    // 007EE7F0 BSP_PilotControl_RefreshArmedFraction: __thiscall(ECX = squadron, int), RET 4.
    virtual void pilot_control_refresh_armed_fraction_007ee7f0(void* squadron, std::int32_t unit) = 0;
    // 007ED610 BSP_PlaneSquadron_PromoteFlightLeader: __thiscall(ECX = squadron, dword), RET 4.
    virtual void squadron_promote_flight_leader_007ed610(void* squadron, std::uint32_t member) = 0;
    // 00438E10 BSP_CString_CompareInsensitive: __fastcall(ECX = a, EDX = b), EAX.
    virtual std::int32_t cstring_compare_insensitive_00438e10(const char* a, const char* b) = 0;
    // 0071EB60 BSP_EntityCommand_ActiveTargetDescriptor: __fastcall(ECX = director), EAX.
    virtual void* entity_command_active_target_descriptor_0071eb60(void* director) = 0;
    // 007B9230 BSP_WeaponController_FirstSlotDescriptor: __thiscall(ECX = unit), EAX.
    virtual void* weapon_controller_first_slot_descriptor_007b9230(void* unit) = 0;
    // 00828EC0 BSP_VehicleClass_IsTorpedoProofSmallCraft / 00827F70
    // BSP_VehicleClass_IsTorpedoBoatOrSmallLandingShip: __thiscall(ECX = class), AL.
    virtual bool vehicle_class_is_torpedo_proof_small_craft_00828ec0(void* vehicle_class) = 0;
    virtual bool vehicle_class_is_torpedo_boat_or_small_landing_ship_00827f70(void* vehicle_class) = 0;
    // 00922B10 BSP_Entity_IsAirborneTarget: __thiscall(ECX = entity), AL.
    virtual bool entity_is_airborne_target_00922b10(void* entity) = 0;
    // 0047B850 and 00604A50 (unnamed unit predicates): __thiscall(ECX = unit), AL.
    virtual bool fun_0047b850(void* unit) = 0;
    virtual bool fun_00604a50(void* unit) = 0;
    // 00780090 BSP_Session_DispatchEntityMessageToMember: __thiscall(ECX = member, message), RET 4.
    virtual void session_dispatch_entity_message_to_member_00780090(void* member, void* message) = 0;
    // Entity vtable +2Ch: __thiscall(ECX = entity; a, b, int* out), RET 0Ch.
    virtual void entity_vcall_2c(void* entity, std::uint32_t a, std::uint32_t b, std::int32_t* out) = 0;
    // Squadron vtable +114h (the command controller getter): __thiscall(ECX = squadron), EAX.
    virtual void* squadron_vcall_114_controller(void* squadron) = 0;
    // Squadron vtable +128h: __thiscall(ECX = squadron, dword), RET 4.
    virtual void squadron_vcall_128(void* squadron, std::uint32_t value) = 0;
    // 0095B9C0 BSP_VehicleClass_GetRegistryId: __thiscall(ECX = class), EAX.
    virtual std::uint32_t vehicle_class_get_registry_id_0095b9c0(void* vehicle_class) = 0;
    // 00928570 (the base world-list unregister): __thiscall(ECX = entity).
    virtual void game_entity_unregister_base_00928570(void* entity) = 0;
    // 006E0860 BSP_ObserverMember9348_Destruct, 00875490 BSP_UnitTickElement_Destruct, 0077E380
    // (the entity base destructor body): __thiscall(ECX = object).
    virtual void observer_member_destruct_006e0860(void* member) = 0;
    virtual void unit_tick_element_destruct_00875490(void* element) = 0;
    virtual void entity_base_destruct_0077e380(void* entity) = 0;
    // 007F0F80 (the observer member constructor): __thiscall(ECX = member, target), RET 4.
    virtual void observer_member_construct_007f0f80(void* member, void* target) = 0;
    // 00922F80: __thiscall(ECX = entity, int), RET 4.
    virtual void fun_00922f80(void* entity, std::int32_t value) = 0;
    // 00696350: __fastcall(ECX = entity, EDX = int; base), RET 4.
    virtual void fun_00696350(void* entity, std::int32_t edx, void* base) = 0;
    // 00926D90 (Kill): __thiscall(ECX = unit, int reason), RET 4.
    virtual void unit_kill_00926d90(void* unit, std::int32_t reason) = 0;
    // 0077F0E0 BSP_UnitOwnerEntity_ActivateAndRegister: __thiscall(ECX = entity).
    virtual void unit_owner_entity_activate_and_register_0077f0e0(void* entity) = 0;
    // 0084D810 BSP_PlaneSquadronCommandBlock_Construct: __thiscall(ECX = block, squadron), RET 4, EAX.
    virtual void* plane_squadron_command_block_construct_0084d810(void* block, void* squadron) = 0;
    // 00927A80 BSP_PropertyBag_GetSkill: __thiscall(ECX = entity), EAX.
    virtual std::uint32_t property_bag_get_skill_00927a80(void* entity) = 0;
    // 009292B0 BSP_Unit_BindLuaClass: __thiscall(ECX = entity; class id, const char* name), RET 8.
    virtual void unit_bind_lua_class_009292b0(void* entity, std::uint32_t class_id, const char* name) = 0;
    // 008F2260 BSP_ScenePropertyBag_Find: __thiscall(ECX = bag, const char* key), RET 4, EAX.
    virtual void* scene_property_bag_find_008f2260(void* bag, const char* key) = 0;
    // 00521E30 BSP_EntityHandleTable_Resolve: __fastcall(ECX = zero-extended word id), EAX.
    virtual void* entity_handle_table_resolve_00521e30(std::uint32_t id) = 0;
    // The command controller's vtable +24h / +28h (byte), AL; +3Ch (state record*), RET 4.
    virtual std::uint8_t controller_vcall_24(void* controller, std::uint8_t value) = 0;
    virtual std::uint8_t controller_vcall_28(void* controller, std::uint8_t value) = 0;
    virtual void controller_vcall_3c(void* controller, const void* state) = 0;
    // 00721280 BSP_CommandControllerStateMessage_Build: __thiscall(ECX = record, controller), RET 4.
    virtual void command_controller_state_build_00721280(void* record, void* controller) = 0;
    // 007213B0 / 007213C0 (the state record's write / read): __thiscall(ECX = record, cursor or
    // stream), RET 4.
    virtual void command_controller_state_write_007213b0(void* record, void* cursor) = 0;
    virtual void command_controller_state_read_007213c0(void* record, void* stream) = 0;
    // 0077C7B0 (the entity message route): __thiscall(ECX = entity; message*, int), RET 8.
    virtual void entity_route_message_0077c7b0(void* entity, void* message, std::int32_t flag) = 0;
    // 007F2920 (the member placement after a squadron move): __thiscall(ECX = squadron).
    virtual void squadron_place_members_007f2920(void* squadron) = 0;
    // 0077C160 BSP_NetEntity_WriteReplicationState: __thiscall(ECX = entity, writer), RET 4.
    virtual void net_entity_write_replication_state_0077c160(void* entity, void* writer) = 0;
    // 0071BDD0 (the director's archive writer): __thiscall(ECX = director, writer), RET 4.
    virtual void director_write_archive_0071bdd0(void* director, void* writer) = 0;

    // ---- air operations ----
    // 006BCD20 BSP_AirOps_GetBlock: __fastcall(ECX = entity, DL = require active), EAX.
    virtual void* air_ops_get_block_006bcd20(void* entity, std::uint8_t require_active) = 0;
    // 006CC760 / 006CC7B0 BSP_AirOps_PushReadyPlane(Alternate): __thiscall(ECX = block, entity), RET 4.
    virtual void air_ops_push_ready_plane_006cc760(void* block, void* entity) = 0;
    virtual void air_ops_push_ready_plane_alternate_006cc7b0(void* block, void* entity) = 0;
    // 006CA770 (the stock add): __thiscall(ECX = block; class, int count), RET 8.
    virtual void air_ops_add_stock_006ca770(void* block, void* vehicle_class, std::int32_t count) = 0;
    // 006C5950 (the slot return): __thiscall(ECX = block, squadron), RET 4.
    virtual void air_ops_return_slot_006c5950(void* block, void* squadron) = 0;

    // ---- net messages and bit cursors ----
    // 00429120 BSP_BitCursor_WriteWordBits: __thiscall(ECX = cursor; word, bits), RET 8.
    virtual void bit_cursor_write_word_bits_00429120(void* cursor, std::uint32_t value, std::int32_t bits) = 0;
    // 00428E30 BSP_BitCursor_ReadWordBits: __thiscall(ECX = cursor; word* out, bits), RET 8.
    virtual void bit_cursor_read_word_bits_00428e30(void* cursor, std::uint16_t* out, std::int32_t bits) = 0;

    // ---- Lua and the mission-entity lock ----
    // 004C1570 BSP_MissionEntityLock_Get: no input, EAX (the lock holder; +4h = CRITICAL_SECTION*).
    virtual void* mission_entity_lock_get_004c1570() = 0;
    // EnterCriticalSection / LeaveCriticalSection (IAT [00CE2218] / [00CE2210]).
    virtual void win32_enter_critical_section(void* section) = 0;
    virtual void win32_leave_critical_section(void* section) = 0;
    // 00927B40 BSP_MissionEntity_LuaSelfObject: __thiscall(ECX = entity, out), RET 4.
    virtual void* mission_entity_lua_self_object_00927b40(void* entity, void* out) = 0;
    // 00B67800 BSP_LuaObject_GetByName: __thiscall(ECX = table; out, const char*), RET 8, EAX = out.
    virtual void* lua_object_get_by_name_00b67800(void* table, void* out, const char* name) = 0;
    // 00B65FB0 BSP_LuaObject_IsNil / 00B66290 BSP_LuaObject_GetInteger: __thiscall(ECX = object).
    virtual bool lua_object_is_nil_00b65fb0(void* object) = 0;
    virtual std::int32_t lua_object_get_integer_00b66290(void* object) = 0;
    // 00B67460 BSP_LuaObject_SetNativeStringInteger: __thiscall(ECX = table; key {size, data}*,
    // int), RET 8.
    virtual void lua_object_set_native_string_integer_00b67460(void* table, void* key, std::int32_t value) = 0;
    // 00B67580 BSP_LuaObject_SetNewTable: __thiscall(ECX = table, key {size, data}*), RET 4.
    virtual void lua_object_set_new_table_00b67580(void* table, void* key) = 0;
    // 00B67690 BSP_LuaObject_Assign: __thiscall(ECX = object, source), RET 4.
    virtual void lua_object_assign_00b67690(void* object, void* source) = 0;
    // 00B67700 BSP_LuaObject_Destruct: __thiscall(ECX = object).
    virtual void lua_object_destruct_00b67700(void* object) = 0;
};

// ---- plane neighbours (the one row in cc9's 007D8000-007E3000 band; the host stubs it) ----
bool cc15_seg48_plane_neighbours_line_of_sight_clear_007df360(void* neighbours, const void* target,
                                                              const float* point, Cc15Seg48Host& host);

// ---- squadron predicates and counters ----
float cc15_seg48_plane_squadron_max_member_908_007ee150(const void* squadron);
bool cc15_seg48_plane_squadron_is_leaving_on_purpose_007ee2e0(void* squadron, std::uint8_t check_leader,
                                                              Cc15Seg48Host& host);
void cc15_seg48_plane_squadron_detach_pilot_007ee4e0(void* squadron, Cc15Seg48Host& host);
bool cc15_seg48_plane_squadron_first_member_df4_entry_007ee720(void* squadron, Cc15Seg48Host& host);
void cc15_seg48_plane_squadron_recount_active_wingmen_007ee790(void* squadron, Cc15Seg48Host& host);
bool cc15_seg48_unit_attack_command_applies_007ee8f0(void* squadron, std::uint32_t command_class, void* target,
                                                     std::uint8_t allow, std::uint32_t loadout,
                                                     Cc15Seg48Host& host);
bool cc15_seg48_plane_squadron_target_is_home_007eee20(void* squadron, Cc15Seg48Host& host);
bool cc15_seg48_plane_squadron_slot168_command_target_check_007efeb0(void* squadron, const char* command,
                                                                     Cc15Seg48Host& host);
bool cc15_seg48_plane_squadron_promote_armed_leader_007effb0(void* squadron, Cc15Seg48Host& host);
void cc15_seg48_pilot_control_raise_force_release_flag_007efb30(void* squadron, Cc15Seg48Host& host);
bool cc15_seg48_pilot_control_handle_message_007f0030(void* squadron, void* message, Cc15Seg48Host& host);

// ---- session messages ----
void* cc15_seg48_session_message_planesquadron_create_construct_007ef490(void* message, std::uint32_t class_word,
                                                                         void* squadron, const void* home_base,
                                                                         std::uint8_t inside_home_base,
                                                                         Cc15Seg48Host& host);
void cc15_seg48_session_message_planesquadron_create_write_007ef590(void* message, void* cursor,
                                                                   Cc15Seg48Host& host);
void cc15_seg48_session_message_planesquadron_create_read_007ef610(void* message, void* stream,
                                                                  Cc15Seg48Host& host);
void cc15_seg48_session_message_planesquadron_attackrun_write_007ef690(void* message, void* cursor,
                                                                      Cc15Seg48Host& host);
void cc15_seg48_session_message_planesquadron_attackrun_read_007ef6c0(void* message, void* stream,
                                                                     Cc15Seg48Host& host);
void cc15_seg48_plane_squadron_send_create_message_007f0b30(void* squadron, std::uint32_t peer,
                                                           Cc15Seg48Host& host);

// ---- command records (the 1Ch-byte {class, target descriptor} pair) ----
void* cc15_seg48_command_record_init_with_target_007ef8b0(void* record, std::uint32_t command_class,
                                                         const void* target);
void* cc15_seg48_command_record_init_with_entity_007f1000(void* record, std::uint32_t command_class,
                                                         const void* entity, Cc15Seg48Host& host);

// ---- lifetime, accessors, world lists ----
void cc15_seg48_plane_squadron_destruct_007ef9d0(void* squadron, Cc15Seg48Host& host);
void* cc15_seg48_plane_squadron_get_subobject_310_007efa80(void* squadron);
std::uint32_t cc15_seg48_plane_squadron_get_field_1e4_007efa90(const void* object);
void cc15_seg48_plane_squadron_slot18c_noop_007efac0(void* squadron, std::uint32_t unused);
void cc15_seg48_plane_squadron_unregister_from_world_lists_007f10e0(void* squadron, Cc15Seg48Host& host);

// ---- archives ----
void cc15_seg48_pilot_bot_settings_save_007f0bf0(void* settings, void* writer, Cc15Seg48Host& host);
void cc15_seg48_pilot_bot_settings_load_007f0dc0(void* settings, void* reader, Cc15Seg48Host& host);
void cc15_seg48_plane_squadron_save_to_archive_007f1160(void* squadron, void* writer, Cc15Seg48Host& host);
void* cc15_seg48_pilot_bot_settings_init_007f2bd0(void* settings, Cc15Seg48Host& host);

// ---- home base, stock, layers, creation ----
void cc15_seg48_plane_squadron_set_home_air_base_007f1c00(void* squadron, void* base, std::uint8_t inside,
                                                         Cc15Seg48Host& host);
void cc15_seg48_plane_squadron_return_plane_to_stock_007f1ca0(void* squadron, void* base, void* plane,
                                                             Cc15Seg48Host& host);
void cc15_seg48_plane_squadron_select_avoid_zone_layers_007f1d90(void* squadron, float slope, std::int32_t flag,
                                                                Cc15Seg48Host& host);
void cc15_seg48_plane_squadron_begin_from_creation_record_007f1fe0(void* squadron, Cc15Seg48Host& host);

// ---- placement ----
void cc15_seg48_plane_squadron_set_world_position_007f2e20(void* squadron, const float* position,
                                                          Cc15Seg48Host& host);
void cc15_seg48_plane_squadron_set_heading_007f2eb0(void* squadron, float heading, Cc15Seg48Host& host);
void cc15_seg48_plane_squadron_set_world_matrix_007f2f90(void* squadron, const float* matrix,
                                                        Cc15Seg48Host& host);

// ---- Lua ----
void cc15_seg48_entity_add_plane_kill_reason_007f2fd0(void* entity, std::int32_t reason, std::int32_t count,
                                                      Cc15Seg48Host& host);

}  // namespace bsp
