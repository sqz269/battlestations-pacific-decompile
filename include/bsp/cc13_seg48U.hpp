#pragma once

// cc13_seg48U: undone segment 40 and 48 bodies with no cc9 host (packet
// cc13-seg48U, docs/CC13_SEG48U.md): gun and gun-bot helpers, the mission
// narrative object at game+21E8h, and plane/squadron helpers. Names are
// hypotheses, not recovered symbols. These are new C++ interfaces over the
// native records, not drop-in replacements. Nothing is wired. Same conventions
// as cc13_seg40: raw byte blocks touched only at the offsets the image uses, and
// one named host hook per native call, at the same point with the same arguments.

#include "bsp/cc13_seg40.hpp"

#include <cstdint>

namespace bsp {

class Cc13Seg48UHost : public Cc13Seg40Host {
public:
    // 00440F50 BSP_DeviceEntity_OnKilledLua, __thiscall(ECX = device), RET.
    virtual void device_entity_on_killed_lua_00440f50(void* device) = 0;
    // 004426C0 BSP_DeviceEntity_PlaceInWorldLua, __thiscall(ECX = device), RET.
    virtual void device_entity_place_in_world_lua_004426c0(void* device) = 0;
    // 00956BA0 (unit_remove_gun_00956ba0, cc13_seg62), __thiscall(ECX = unit, gun), RET 4.
    virtual void unit_remove_gun_00956ba0(void* unit, void* gun) = 0;
    // 0072D5B0 BSP_Gun_SelectFireMode, __thiscall(ECX = gun; mode, flag), RET 8.
    virtual void gun_select_fire_mode_0072d5b0(void* gun, std::int32_t mode, std::int32_t flag) = 0;
    // 0072BBD0 BSP_GunBot_ConstructBase (cc9 hosts it), __thiscall(ECX = bot, arg), RET 4.
    virtual void gun_bot_construct_base_0072bbd0(void* bot, std::uint32_t arg) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __thiscall(ECX = entity), RET.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // The shot decision's line-of-fire query: __thiscall vtable slot 1 (+4h) of the
    // object at decision+8h, (const float* position, owner [gun+3F0h], arg, float range),
    // RET 10h; AL = blocked.
    virtual std::uint8_t line_of_fire_query_vslot1(void* query, const float* position, void* owner,
                                                   std::uint32_t arg, float range) = 0;
    // 00728340 BSP_SceneNode_ClearFlagsRecursive, __thiscall(ECX = node, flags), RET 4.
    virtual void scene_node_clear_flags_recursive_00728340(void* node, std::uint32_t flags) = 0;
    // 00475E60 BSP_Node_OrFlagsRecursive, __thiscall(ECX = node, flags), RET 4.
    virtual void node_or_flags_recursive_00475e60(void* node, std::uint32_t flags) = 0;
    // 00733430 BSP_MissionNarrativeList_BuyHeadNode, __thiscall(ECX = list), EAX = node.
    virtual void* narrative_list_buy_head_node_00733430(void* list) = 0;
    // 00733B10 BSP_SessionMessage_MISSIONINFO_Construct (cc13_seg40), __thiscall(ECX =
    // message, const native string* text), RET 4; EAX = message.
    virtual void* session_message_missioninfo_construct_00733b10(void* message, const void* text) = 0;
    // 007336A0 BSP_SessionMessage53_Destroy, __thiscall(ECX = message), RET.
    virtual void session_message53_destroy_007336a0(void* message) = 0;
    // 00770BD0 BSP_Session_HostRelayToTeam, __thiscall(ECX = game+1EF0h; team, message,
    // except peer), RET 0Ch.
    virtual void session_host_relay_to_team_00770bd0(void* session, std::int32_t team, void* message,
                                                     std::uint32_t except_peer) = 0;
    // 00770B50 BSP_Session_SendMessageToNonlocalPeer, __thiscall(ECX = game+1EF0h; peer,
    // message), RET 8.
    virtual void session_send_to_nonlocal_peer_00770b50(void* session, void* peer, void* message) = 0;
    // 00734870 (unnamed narrative-list append), __thiscall(ECX = narrative; entry,
    // NativeString*, 0), RET 0Ch.
    virtual void narrative_list_append_00734870(void* narrative, const void* entry, void* text,
                                                std::uint32_t zero) = 0;
    // The save reader's field read is the chain's archive_vcall_10_get (cc13_seg48).
    // 00BF6713 _invalid_parameter_noinfo (the checked-iterator failure), no arguments.
    virtual void crt_invalid_parameter_noinfo_00bf6713() = 0;
    // 00729600 BSP_GunShotDecision_Destruct (the shot-decision object at gun+424h; cc9
    // has no lifetime model for it), __thiscall(ECX = gun+424h), RET.
    virtual void gun_effect_set_destruct_00729600(void* set) = 0;
    // vtable slot 1 (+4h) with (1): the scalar deleting destructor of the gun's
    // +3E0h vector elements, __thiscall(ECX = element, flags), RET 4.
    virtual void vcall_04_delete(void* object, std::uint32_t flags) = 0;
    // 00868420 BSP_PointEffect_CreateFromMatrix: ECX = out handle, EDX = group; stack
    // (definition handle by value, const float* matrix, 0, 0), RET 10h; EAX = out. The
    // caller retains the by-value handle (InterlockedIncrement); the callee consumes it.
    virtual void** point_effect_create_from_matrix_00868420(void** out, void* group, void* definition,
                                                            const float* matrix, std::uint32_t a,
                                                            std::uint32_t b) = 0;
    // 007CF280 STL_inst_007cf280 (erase range of the plane's 10h-byte effect records),
    // __thiscall(ECX = vector; out iterator, first {vector, ptr}, last {vector, ptr}), RET 14h.
    virtual void effect_record_vector_erase_007cf280(void* vector, void* out_iterator, void* first_vector,
                                                     void* first, void* last_vector, void* last) = 0;
    // 007D1DC0 STL_Vector_PushBack_007d1dc0, __thiscall(ECX = vector, const record*), RET 4.
    virtual void effect_record_vector_push_back_007d1dc0(void* vector, const void* record) = 0;
    // Archive reader slot +14h(name word 0, key word 1), the section-present test; AL.
    // Like archive_vcall_04 (cc13_seg48), key word 0 = 1 makes key word 1 an index.
    virtual bool archive_vcall_14_has(void* reader, std::uint32_t name_word0, const char* key) = 0;
    // 007342B0 BSP_NarrativeList_BuyNode, __thiscall(ECX = list; next, prev, const entry*),
    // RET 0Ch; EAX = node (entry copied into node+8h).
    virtual void* narrative_list_buy_node_007342b0(void* list, void* next, void* prev, const void* entry) = 0;
    // 007341B0 STL_NarrativeList_Incsize_007341b0 (MSVC _Incsize): __thiscall(ECX = list, count), RET 4, the
    // list's size increment with its length check.
    virtual void narrative_list_incsize_007341b0(void* list, std::uint32_t count) = 0;
    // 00733DC0 BSP_NarrativeEntry_Destruct (cc13_seg40), __thiscall(ECX = entry), RET.
    virtual void narrative_entry_destruct_00733dc0(void* entry) = 0;
    // 00BF6FF5 _atexit(function), cdecl.
    virtual void crt_atexit_00bf6ff5(std::uint32_t function) = 0;
    // 004263B0 BSP_NativeString_ConcatInt, __thiscall(ECX = lhs; out, int), RET 8; EAX = out.
    virtual void* native_string_concat_int_004263b0(const void* lhs, void* out, std::int32_t value) = 0;
    // 004261A0 BSP_NativeString_Concat, __thiscall(ECX = lhs; out, const rhs*), RET 8; EAX = out.
    virtual void* native_string_concat_004261a0(const void* lhs, void* out, const void* rhs) = 0;
    // 0087A410 BSP_UnitTickableEntity_Destruct, __thiscall(ECX = entity), RET.
    virtual void unit_tickable_entity_destruct_0087a410(void* entity) = 0;
    // 00425850 BSP_NativeString_EqualsCStringInsensitive, __thiscall(ECX = string, const char*), RET 4; AL.
    virtual bool native_string_equals_ci_00425850(const void* string, const char* text) = 0;
    // 007311B0 BSP_TorpedoShotCommand_Construct (cc9 hosts it), __thiscall(ECX = command;
    // float heading, float swimdepth, unit), RET 0Ch.
    virtual void torpedo_shot_command_construct_007311b0(void* command, float heading, float swimdepth,
                                                         void* unit) = 0;
    // A shot command's vtable slot 1 (+4h): __thiscall(ECX = command, reader), RET 4 (the read).
    virtual void shot_command_read_vslot1(void* command, void* reader) = 0;
    // 00441E50 STL_Tree_EraseRange_00441e50, __thiscall(ECX = container; out iterator,
    // first {container, node}, last {container, node}), RET 14h.
    virtual void tree_erase_range_00441e50(void* container, void* out_iterator, void* first_container,
                                           void* first_node, void* last_container, void* last_node) = 0;

    // ---- 0072C170 (cc13-seg48V) ----
    // 004142E0 BSP_Vector3f_TransformAffinePoint, ECX = point; (float out[3], const float
    // matrix[16]), RET 8.
    virtual void vector3f_transform_affine_point_004142e0(const float* point, float* out,
                                                          const float* matrix) = 0;
    // 00419510 BSP_Vector3f_Normalize, __fastcall(ECX = out, EDX = v), RET; EAX = out.
    virtual const float* vector3f_normalize_00419510(float* out, const float* v) = 0;
    // The target entity's vtable slot +34h: __thiscall(ECX = entity, float out[3]), RET 4;
    // EAX = the vector it answers (read as the target's velocity, per metre per second).
    virtual const float* entity_vcall_34_vector(void* entity, float* out) = 0;
    // The bullet class's vtable slot +20h (the spawn): __thiscall(ECX = bullet class; 0,
    // side, const float from[3], float dir[3], flag dword, 0, 0); EAX = projectile or null.
    virtual void* bullet_class_vcall_20_spawn(void* bullet_class, std::uint32_t zero_a, std::uint32_t side,
                                              const float* from, float* direction, std::uint32_t flag,
                                              std::uint32_t zero_b, std::uint32_t zero_c) = 0;
    // 006E6BB0 BSP_Projectile_BindOwner, __thiscall(ECX = projectile, owner), RET 4.
    virtual void projectile_bind_owner_006e6bb0(void* projectile, void* owner) = 0;
    // Slot 0 of the sub-object at unit+170h: __thiscall(ECX = unit+170h); EAX.
    virtual std::uint32_t unit_sub170_vcall_0(void* sub_object) = 0;
    // The projectile's vtable slot +34h: __thiscall(ECX = projectile; const float a[3],
    // const float b[3]), RET 8.
    virtual void projectile_vcall_34(void* projectile, const float* a, const float* b) = 0;

    // ---- 007E29A0 (cc13-seg48V) ----
    // The entity's vtable slot +4h: __thiscall(ECX = entity); EAX = the observed object.
    virtual void* entity_vcall_4_observed(void* entity) = 0;
    // 007E1D30 STL_inst_007e1d30, std::remove over a pointer array: __fastcall(ECX =
    // first, EDX = last; const value*), RET 4; EAX = the new end. Template body, bind-only.
    virtual void** std_remove_pointer_007e1d30(void** first, void** last, void* const* value) = 0;

    // ---- 007EC5A0 (cc13-seg48V) ----
    // 0042E740 BSP_GameTuning_GetSingleton: no input; EAX = the 6D0h tuning object.
    virtual void* game_tuning_get_singleton_0042e740() = 0;
    // 00871BA0 BSP_EffectHandle_AcquireByName: __fastcall(ECX = out handle, EDX = native
    // string; flag), RET 4; EAX = out. The handle is refcounted (+4h count).
    virtual void** effect_handle_acquire_by_name_00871ba0(void** out, const void* name, std::int32_t flag) = 0;
    // 00440580 STL_inst_00440580, the refcounted handle assign: __thiscall(ECX = slot;
    // const handle*), RET 4. Template body, bind-only.
    virtual void ref_handle_assign_00440580(void* slot, void* const* source) = 0;
    // 004A8F10 STL_CheckedFloatVector_Resize_004a8f10: __thiscall(ECX = vector; count,
    // float fill), RET 8. Template body, bind-only.
    virtual void float_vector_resize_004a8f10(void* vector, std::uint32_t count, float fill) = 0;
    // 00BD2FC0 BSP_RandomThreads_NextU32: ECX = stream; EAX.
    virtual std::uint32_t random_threads_next_u32_00bd2fc0(std::uint32_t stream) = 0;
    // Plane engine vtable slot 0 (00D08640 -> 007E1BA0, 00D0865C -> 007EBF10 rotor):
    // __thiscall(ECX = engine; visual, node, index, float spread), RET 10h.
    virtual void plane_engine_vcall_00_init(void* engine, void* visual, std::uint32_t node, std::int32_t index,
                                            float spread) = 0;
};

// ---- gun and gun bot ----
void cc13_seg48U_gun_bot_set_skill_index_0072bc80(void* bot, std::int32_t index);
bool cc13_seg48U_gun_bot_has_fire_target_0072bd10(const void* bot);
void* cc13_seg48U_gun_current_mode_record_00731050(const void* gun);
void* cc13_seg48U_gun_current_bullet_00731040(const void* gun);
void cc13_seg48U_gun_on_activated_00729fa0(void* gun, Cc13Seg48UHost& host);
void cc13_seg48U_gun_base_init_device_0072d7e0(void* gun, Cc13Seg48UHost& host);
void* cc13_seg48U_gun_bot_construct_muzzle_class_0072be40(void* bot, Cc13Seg48UHost& host);
bool cc13_seg48U_shot_decision_line_of_fire_clear_00729670(void* decision, std::uint32_t arg, Cc13Seg48UHost& host);
void cc13_seg48U_gun_apply_descriptor_node_flags_00729700(void* gun, Cc13Seg48UHost& host);
void* cc13_seg48U_gun_shot_command_read_and_create_00731ae0(void* reader, Cc13Seg48UHost& host);
void cc13_seg48U_gun_class_effect_record_destruct_007323e0(void* record, Cc13Seg48UHost& host);
void cc13_seg48U_gun_rebuild_barrel_buffers_00729d30(void* gun, Cc13Seg48UHost& host);
void cc13_seg48U_gun_destruct_0072d950(void* gun, Cc13Seg48UHost& host);
// 0072C170, reached only from the Lua native HitTarget (00892F70 at 008931DE).
void cc13_seg48U_gun_compute_aim_for_target_0072c170(void* shooter, std::int32_t device_id, void* target,
                                                     const float* point_a, const float* point_b, float speed,
                                                     std::uint32_t flag_dword, Cc13Seg48UHost& host);

// ---- planes and squadrons ----
void cc13_seg48U_plane_copy_class_record_lists_007d5ac0(void* plane, Cc13Seg48UHost& host);
const char* cc13_seg48U_plane_squadron_display_name_007f35c0(void* squadron, Cc13Seg48UHost& host);
void cc13_seg48U_observer_set_remove_007e29a0(void* set, void* entity, Cc13Seg48UHost& host);
void cc13_seg48U_plane_fx_component_load_from_class_007ec5a0(void* component, void* plane, std::uint8_t active,
                                                             Cc13Seg48UHost& host);

// ---- mission narrative (game+21E8h) ----
void* cc13_seg48U_mission_narrative_construct_00735030(void* narrative, Cc13Seg48UHost& host);
void cc13_seg48U_mission_narrative_reset_00733030(void* narrative);
void* cc13_seg48U_narrative_entry_copy_construct_00733c80(void* entry, const void* source, Cc13Seg48UHost& host);
void cc13_seg48U_mission_narrative_write_007337a0(void* narrative, void* writer, Cc13Seg48UHost& host);
void cc13_seg48U_mission_narrative_read_00734460(void* narrative, void* reader, Cc13Seg48UHost& host);
void cc13_seg48U_mission_narrative_send_to_party_00734b10(void* narrative, std::int32_t party, const void* text,
                                                          Cc13Seg48UHost& host);
void cc13_seg48U_mission_narrative_send_to_player_00734bf0(void* narrative, std::int32_t player, const void* text,
                                                           Cc13Seg48UHost& host);

}  // namespace bsp
