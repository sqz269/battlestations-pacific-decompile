#pragma once
// Packet cc13-seg47b (cc13_seg47b marker): segment 47 (007B2DD0-007D1DC0,
// flight), pass two. docs/CC13_SEG47B.md lists every function with its
// outcome (reconstructed here / existing-and-recorded / skipped with the
// reason), its original ABI and its uncertainty.
//
// Same shape as cc13_seg47 (whose Raw, ArchiveKey and Seg47Host this reuses):
// objects are reached through byte offsets into the Win32 layout and every
// call the image makes goes through a host method named for its target,
// invoked at the binary's point with the binary's arguments. Descriptive names
// are hypotheses, not recovered symbols. Nothing here is wired; nothing is
// ABI-compatible.

#include "bsp/cc13_seg47.hpp"

#include <cstdint>

namespace bsp::cc13_seg47b {

using cc13_seg47::ArchiveKey;
using cc13_seg47::Raw;
using cc13_seg47::raw;

struct Seg47bHost : cc13_seg47::Seg47Host {
    // 007C1430 BSP_Plane_SetFlightState, __thiscall(ECX = plane; state), RET 4, EAX.
    virtual std::int32_t plane_set_flight_state_007c1430(Raw plane, std::int32_t state) = 0;
    // 0042D2E0 BSP_Matrix_ExtractNormalizedBasisAngles, (ECX = matrix, EDX = x; y, z), RET 8.
    virtual void matrix_extract_normalized_basis_angles_0042d2e0(void* matrix, float* x, float* y,
                                                                 float* z) = 0;
    // 007D8330 BSP_PlaneFlightController_WriteStateRecord, __thiscall(ECX = controller; out 3Ch), RET 4, EAX.
    virtual void* flight_controller_write_state_record_007d8330(Raw controller, void* out) = 0;
    // 007BAA80 BSP_PlaneStateRecord_Copy, __thiscall(ECX = dst; src), RET 4.
    virtual void plane_state_record_copy_007baa80(Raw dst, void* src) = 0;
    // 007C2810 BSP_PlaneStateRecord_GetOrCreate, __thiscall(ECX = plane (unused); owner), RET 4, EAX = owner+0Ch's record.
    virtual void* plane_state_record_get_or_create_007c2810(Raw plane, void* owner) = 0;
    // 00865FE0 BSP_PointEffect_IsFinished, __thiscall(ECX = point), AL.
    virtual bool point_effect_is_finished_00865fe0(Raw point) = 0;
    // 004842C0 BSP_PointEffect_SetWorldPoint, __thiscall(ECX = point; const float* p), RET 4.
    virtual void point_effect_set_world_point_004842c0(Raw point, const float* p) = 0;
    // 00866B70 BSP_EffectHandle_RequestStop, __thiscall(ECX = handle).
    virtual void effect_handle_request_stop_00866b70(Raw handle) = 0;
    // 00867B10 BSP_PointEffect_StopChildren, __thiscall(ECX = point).
    virtual void point_effect_stop_children_00867b10(Raw point) = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint, __thiscall(ECX = point; out, matrix), RET 8.
    virtual void vector3_transform_affine_point_004142e0(const float* point, float* out, const void* matrix) = 0;
    // 007C2610 BSP_Unit_MinKind21ComponentSpeed, __fastcall(ECX = unit), ST0.
    virtual float unit_min_kind21_component_speed_007c2610(Raw unit) = 0;
    // Unit vtable[48h], __thiscall(ECX = unit; out float[3], seconds), RET 8: predicted position.
    virtual void unit_predict_position_vcall_48(Raw unit, float* out, float seconds) = 0;
    // 00954580 (no ledger name), __thiscall(ECX = plane), RET 0.
    virtual void unit_00954580(Raw plane) = 0;
    // 007C5AC0 BSP_Plane_LandingSiteProbe, __thiscall(ECX = plane; float), RET 4.
    virtual void plane_landing_site_probe_007c5ac0(Raw plane, float value) = 0;
    // 007D9CE0 BSP_PlaneFlightController_ReparentVelocity, __thiscall(ECX = plane+AB0h; parent), RET 4.
    virtual void flight_controller_reparent_velocity_007d9ce0(Raw controller, void* parent) = 0;
    // 00414E10 BSP_EntityPose_GetDerivedAffineInverse, __thiscall(ECX = entity), EAX = matrix*.
    virtual void* entity_pose_derived_affine_inverse_00414e10(Raw entity) = 0;
    // 007BA020 BSP_Matrix_MultiplyInPlace, __thiscall(ECX = matrix; const matrix*), RET 4.
    virtual void matrix_multiply_in_place_007ba020(Raw matrix, const void* other) = 0;
    // 0042ED50 BSP_SceneNode_InvalidateSubtreePose, __thiscall(ECX = node), RET 0.
    virtual void scene_node_invalidate_subtree_pose_0042ed50(Raw node) = 0;
    // (plane+310h)->vtable[0Ch], __thiscall(ECX = plane+310h), RET 0.
    virtual void plane_body_vcall_0c(Raw body) = 0;
    // plane->vtable[ACh](parent), __thiscall, RET 4: set the scene parent.
    virtual void plane_vcall_set_parent_ac(Raw plane, void* parent) = 0;
    // 00923810 BSP_EntityAncestor, __thiscall(ECX = entity; levels), RET 4, EAX.
    virtual void* entity_ancestor_00923810(Raw entity, std::int32_t levels) = 0;
    // 0042D7E0 BSP_EntityPose_GetWorldMatrix, __thiscall(ECX = entity), EAX = entity+CCh.
    virtual void* entity_pose_world_matrix_0042d7e0(Raw entity) = 0;
    // 007D9C10 BSP_PlaneFlightController_RefreshBodyFrame, __thiscall(ECX = plane+AB0h), RET 0.
    virtual void flight_controller_refresh_body_frame_007d9c10(Raw controller) = 0;
    // 007F9F70 BSP_NoseArt_GetTexturePathRet, __thiscall(ECX = nose-art table; out NativeString, index), RET 8, EAX = out.
    virtual Raw nose_art_texture_path_007f9f70(Raw table, Raw out, std::int32_t index) = 0;
    // [00F8D394]->vtable[64h](const NativeString*, 0), __thiscall, RET 8, EAX = refcounted texture.
    virtual void* renderer_vcall_load_texture_64(Raw renderer, Raw name, std::int32_t flag) = 0;
    // 0041E870 BSP_NativeString_Assign, __thiscall(ECX = string; const char*), RET 4, EAX = string.
    virtual void native_string_assign_0041e870(Raw string, const char* text) = 0;
    // 00B85310 BSP_Model_SetTextureSlotByEffect_Provisional, (ECX = model, EDX = effect name; texture, int), RET 8.
    virtual void model_set_texture_slot_by_effect_00b85310(Raw model, Raw effect_name, void* texture,
                                                           std::int32_t flag) = 0;
    // 0095E5B0 BSP_Unit_ReadCommonLuaProperties, __thiscall(ECX = unit), RET 0.
    virtual void unit_read_common_lua_properties_0095e5b0(Raw unit) = 0;
    // 00951F80 (no ledger name), __thiscall(ECX = unit), RET 0.
    virtual void unit_00951f80(Raw unit) = 0;
    // 007BC550 BSP_Plane_SyncElevatorAttachmentForState, __thiscall(ECX = plane), RET 0.
    virtual void plane_sync_elevator_attachment_007bc550(Raw plane) = 0;
    // 00927B40 BSP_MissionEntity_LuaSelfObject, __thiscall(ECX = entity; out LuaObject), RET 4, EAX.
    virtual Raw mission_entity_lua_self_object_00927b40(Raw entity, void* out) = 0;
    // 00B67460 BSP_LuaObject_SetNativeStringInteger, __thiscall(ECX = object; const NativeString*, int), RET 8.
    virtual void lua_object_set_string_integer_00b67460(Raw object, Raw key, std::int32_t value) = 0;
    // 00B67700 BSP_LuaObject_Destruct, __thiscall(ECX = object), RET 0.
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    // 00956EE0 BSP_Plane_SetGunFireEffect (base vtable[1FCh]), __thiscall(ECX = plane; bool), RET 4.
    virtual void plane_set_gun_fire_effect_00956ee0(Raw plane, bool firing) = 0;
    // 008687C0 BSP_PointEffect_CreateWithParentMatrix, (ECX = out ref, EDX = node; definition
    // (a by-value reference the callee releases), const matrix*, a, b, c), RET 14h, EAX = out.
    virtual void* point_effect_create_with_parent_matrix_008687c0(void* out, Raw node, void* definition,
                                                                  const void* matrix, std::int32_t a,
                                                                  std::int32_t b, std::int32_t c) = 0;
    // 0074D780 BSP_LiveEffectReferences_Append, __thiscall(ECX = vector; const ref*), RET 4.
    virtual void live_effect_references_append_0074d780(Raw vector, void* ref) = 0;
    // entity->vtable[10h](), __thiscall(ECX = entity), EAX = const char* name.
    virtual const char* entity_vcall_name_10(Raw entity) = 0;
    // 004261A0 BSP_NativeString_Concat, __thiscall(ECX = lhs; out, rhs), RET 8, EAX = out.
    virtual Raw native_string_concat_004261a0(Raw lhs, Raw out, Raw rhs) = 0;
    // 0095D400 (no ledger name; the unit base kill, reached by a tail jump), __thiscall(ECX = unit).
    virtual void unit_on_killed_0095d400(Raw unit) = 0;
    // 00489370 (no ledger name; an MSVC checked vector<4-byte>::erase(first, last) instantiation),
    // __thiscall(ECX = vector; out iterator, first {container, ptr}, last {container, ptr}), RET 14h.
    virtual void ptr_vector_erase_range_00489370(void* vector, void* out_iterator, void* first_container,
                                                 void* first, void* last_container, void* last) = 0;
    // --- 007CCFA0 dispatch targets --------------------------------------------
    // message->vtable[0Ch](kind), __thiscall, RET 4, AL: the message is of `kind`.
    virtual bool message_vcall_is_kind_0c(Raw message, std::int32_t kind) = 0;
    // plane->vtable[208h](message), __thiscall, RET 4.
    virtual void plane_vcall_handle_kind62_208(Raw plane, Raw message) = 0;
    // 007CC820 BSP_Plane_EnterFlightStateOne / 007CC7A0 ..Two / 007C7110 BSP_Plane_BeginFlying /
    // 007CB5F0 BSP_Plane_OnTouchdownFromFlight / 007BBBA0 BSP_Unit_RequestOrdnanceRelease,
    // each __thiscall(ECX = plane), RET 0.
    virtual void plane_enter_flight_state_one_007cc820(Raw plane) = 0;
    virtual void plane_enter_flight_state_two_007cc7a0(Raw plane) = 0;
    virtual void plane_begin_flying_007c7110(Raw plane) = 0;
    virtual void plane_on_touchdown_from_flight_007cb5f0(Raw plane) = 0;
    virtual void unit_request_ordnance_release_007bbba0(Raw plane) = 0;
    // 0042A7E0 BSP_Plane_IsOnGround, __thiscall(ECX = plane), AL.
    virtual bool plane_is_on_ground_0042a7e0(Raw plane) = 0;
    // 007CA3F0 BSP_Plane_HandleTouchdownOrCrash, __thiscall(ECX = plane; int), RET 4.
    virtual void plane_handle_touchdown_or_crash_007ca3f0(Raw plane, std::int32_t flag) = 0;
    // 007C6F50 BSP_Plane_DetachFromParentKeepWorld, __thiscall(ECX = plane; byte), RET 4.
    virtual void plane_detach_from_parent_keep_world_007c6f50(Raw plane, std::uint8_t value) = 0;
    // 007CC8B0 BSP_Plane_ReturnToStockOnDisappear, __thiscall(ECX = plane; entity), RET 4.
    virtual void plane_return_to_stock_on_disappear_007cc8b0(Raw plane, void* entity) = 0;
    // 007B83F0 BSP_Plane_SetLaunchDelayAA0, __thiscall(ECX = plane; byte), RET 4.
    virtual void plane_set_launch_delay_aa0_007b83f0(Raw plane, std::uint8_t value) = 0;
    // 0095ABE0 BSP_Unit_HandleMessage, __thiscall(ECX = unit; message), RET 4, AL.
    virtual bool unit_handle_message_0095abe0(Raw unit, Raw message) = 0;
    // 0077E830 BSP_MissionEntity_AttachLuaSelfWithResourceUsage, __thiscall(ECX = entity), RET 0.
    virtual void mission_entity_attach_lua_self_0077e830(Raw entity) = 0;
    // 0048E9F0 BSP_ScenePropertyBag_Contains, __thiscall(ECX = bag; const char*), RET 4, AL.
    virtual bool scene_property_bag_contains_0048e9f0(void* bag, const char* key) = 0;
    // 00521E30 BSP_EntityHandleTable_Resolve, (ECX = zero-extended 16-bit id), EAX.
    virtual void* entity_handle_table_resolve_00521e30(std::uint32_t id) = 0;
    // 007ED0D0 BSP_PlaneSquadron_InsertPlaneSorted, __thiscall(ECX = squadron; plane, int), RET 8.
    virtual void plane_squadron_insert_plane_sorted_007ed0d0(Raw squadron, Raw plane, std::int32_t order) = 0;
    // 007CDC70 BSP_Plane_PublishStepRecord, __thiscall(ECX = plane), RET 0.
    virtual void plane_publish_step_record_007cdc70(Raw plane) = 0;
    // 007CD3F0 STL_DestroyRange (10h-byte {ref, ...} elements) / 004CC760
    // BSP_ReferencePointerRange_Release, (ECX = first, EDX = last; vector, allocator byte), RET 8.
    virtual void stl_destroy_range_007cd3f0(void* first, void* last, void* vector, std::uint32_t allocator) = 0;
    virtual void reference_pointer_range_release_004cc760(void* first, void* last, void* vector,
                                                          std::uint32_t allocator) = 0;
    // 007CCD30 BSP_PlaneEngineEffectVector_DestroyRecords, __thiscall(ECX = vector), RET 0.
    virtual void plane_engine_effect_vector_destroy_records_007ccd30(Raw vector) = 0;
    // 007C9290 BSP_PlaneEngineEffectVector_Resize, __thiscall(ECX = array; size), RET 4.
    virtual void plane_engine_effect_vector_resize_007c9290(Raw array, std::int32_t size) = 0;
    // 007BD600 BSP_PodArray64_Reserve, __thiscall(ECX = array; capacity), RET 4.
    virtual void pod_array64_reserve_007bd600(Raw array, std::int32_t capacity) = 0;
    // 0095F1C0 BSP_VehicleClass_DestructBase, __thiscall(ECX = class), RET 0.
    virtual void vehicle_class_destruct_base_0095f1c0(Raw cls) = 0;
    // 004F9B30 BSP_Vector3f_Cross, __fastcall(ECX = out, EDX = a; const float* b), RET 4, EAX = out.
    virtual float* vector3_cross_004f9b30(float* out, const float* a, const float* b) = 0;
    // 0085E4D0 BSP_Matrix_RotateByAngularVelocity, __fastcall(ECX = out matrix, EDX = in matrix;
    // wx, wy, wz, dt), RET 10h.
    virtual void matrix_rotate_by_angular_velocity_0085e4d0(void* out, const void* in, float wx, float wy,
                                                            float wz, float dt) = 0;
    // --- MP senders ------------------------------------------------------------
    // 007ED160 BSP_PlaneSquadron_MemberIndexOf, __thiscall(ECX = squadron; plane), RET 4, EAX.
    virtual std::int32_t plane_squadron_member_index_of_007ed160(Raw squadron, Raw plane) = 0;
    // plane->vtable[34h](out float[3]), __thiscall, RET 4, EAX = the vector it answers: the world
    // velocity (007BBB70).
    virtual const float* plane_vcall_world_velocity_34(Raw plane, float* out) = 0;
    // 007BD3E0 BSP_SessionMessage_PLANE_CREATE_Construct, __thiscall(ECX = message; class+70h,
    // plane, squadron, member index, equipment +C54h, velocity float[3] by value, flag byte,
    // +6B8h), RET 28h.
    virtual void plane_create_message_construct_007bd3e0(Raw message, std::uint32_t class_70, Raw plane,
                                                         void* squadron, std::int32_t index,
                                                         std::int32_t equipment, const float* velocity,
                                                         std::uint8_t flag, std::int32_t value_6b8) = 0;
    // 007BDD30 BSP_SessionMessage_PLANE_SYNC_Construct, __thiscall(ECX = message; plane, record,
    // a, b, const float* plane+674h, const float* plane+9E4h, the 3Ch control block by value,
    // state, byte, byte, byte), RET 64h, EAX = message. The byte arguments are pushed as
    // dwords whose upper bytes are stale; the callee reads bytes only.
    virtual void* plane_sync_message_construct_007bdd30(Raw message, Raw plane, void* record, std::int32_t a,
                                                        std::uint8_t b, const float* matrix_674,
                                                        const float* state_9e4, const void* block_3c,
                                                        std::int32_t state, std::uint8_t c, std::uint8_t flag_904,
                                                        std::uint8_t ctl_94) = 0;
    // 00779F80 BSP_NetEntityHolder_StampSyncTick, __thiscall(ECX = entity (unused); holder), RET 4.
    virtual void net_entity_holder_stamp_sync_tick_00779f80(Raw entity, void* holder) = 0;
    // 00779F90 BSP_Session_SendMessageToHost, __thiscall(ECX = entity; message), RET 4.
    virtual void session_send_message_to_host_00779f90(Raw entity, void* message) = 0;
    // --- impact predictors -------------------------------------------------------
    // 007BA340 BSP_Plane_LandscapeHeightAt, __thiscall(ECX = [00E188A8]; x, z), RET 8, ST0.
    virtual float plane_landscape_height_at_007ba340(Raw game, float x, float z) = 0;
    // 007BD120 BSP_WeaponController_RocketIgnitionLift, __thiscall(ECX = plane), ST0.
    virtual float rocket_ignition_lift_007bd120(Raw plane) = 0;
    // 00470470 BSP_HitRecord_Reset / 004704B0 BSP_HitRecord_Destroy, __thiscall(ECX = record).
    virtual void hit_record_reset_00470470(void* record) = 0;
    virtual void hit_record_destroy_004704b0(void* record) = 0;
    // 00904400 (no ledger name; a world segment query), __thiscall(ECX = [game+19CCh]; kind,
    // const float* from, const float* to, record*, int), RET 14h, AL = hit.
    virtual bool world_segment_query_00904400(Raw world, std::int32_t kind, const float* from, const float* to,
                                              void* record, std::int32_t flag) = 0;
    // --- 007C4850 class derivation --------------------------------------------------
    // 004254B0 (a trivial body: the error-log stub), __cdecl(const char*).
    virtual void error_log_004254b0(const char* text) = 0;
    // 00412E20 BSP_Math_TangentX87Float, (float), RET 4, ST0.
    virtual float tangent_00412e20(float x) = 0;
    // 007D98F0 BSP_PlaneClass_SolveMaxClimbAngle, __thiscall(ECX = class; speed), RET 4, ST0.
    virtual float plane_class_solve_max_climb_angle_007d98f0(Raw cls, float speed) = 0;
    // 007DB4D0 BSP_PlaneClass_LeaderTurnRadius, __thiscall(ECX = class), ST0.
    virtual float plane_class_leader_turn_radius_007db4d0(Raw cls) = 0;
    // 00415550 BSP_Math_MaxFloatByRef / 00415510 BSP_Math_MinFloatByRef, (ECX = const float* a,
    // EDX = const float* b), ST0.
    virtual float max_float_by_ref_00415550(const float* a, const float* b) = 0;
    virtual float min_float_by_ref_00415510(const float* a, const float* b) = 0;
    // 007C47F0 BSP_PlaneClass_LevelFlightSpeed, __thiscall(ECX = class), ST0.
    virtual float plane_class_level_flight_speed_007c47f0(Raw cls) = 0;
    // 00951F20 BSP_VehicleClass_DeriveConstantsDefault, __thiscall(ECX = class), AL (reached by a tail jump).
    virtual bool vehicle_class_derive_constants_default_00951f20(Raw cls) = 0;
    // --- 007C6820 tick ---------------------------------------------------------------
    // 00956600 BSP_UnitInstance_UpdateTimers, __thiscall(ECX = unit; dt), RET 4.
    virtual void unit_instance_update_timers_00956600(Raw unit, float dt) = 0;
    // 007E27B0 BSP_Plane_UpdateDamageEffects, __thiscall(ECX = [plane+E80h]; dt), RET 4.
    virtual void plane_update_damage_effects_007e27b0(Raw object, float dt) = 0;
    // 00B70490 BSP_Camera_GetViewProjectionMatrix, __thiscall(ECX = camera), EAX = matrix*.
    virtual void* camera_view_projection_matrix_00b70490(Raw camera) = 0;
    // model->vtable[48h](), __thiscall(ECX = model), EAX = its bounding sphere.
    virtual void* model_vcall_bounding_sphere_48(Raw model) = 0;
    // 007C1180 BSP_Sphere_TransformAffine, __thiscall(ECX = sphere; out sphere, matrix), RET 8.
    virtual void sphere_transform_affine_007c1180(void* sphere, float* out, const void* matrix) = 0;
    // 007DE9A0 BSP_ActuatorBlock_PushPartAnimFrame, __thiscall(ECX = [plane+DECh]), RET 0.
    virtual void actuator_block_push_part_anim_frame_007de9a0(Raw block) = 0;
    // 007B91C0 BSP_WeaponController_HasOrdnanceOfKind, __thiscall(ECX = plane; kind, flag), RET 8, AL.
    virtual bool weapon_controller_has_ordnance_of_kind_007b91c0(Raw plane, std::int32_t kind,
                                                                std::int32_t flag) = 0;
    // 007C4DD0 BSP_Plane_UpdateEngineEffects, __thiscall(ECX = plane; dt), RET 4.
    virtual void plane_update_engine_effects_007c4dd0(Raw plane, float dt) = 0;
    // --- 007D0820 ------------------------------------------------------------------
    // 0071AD50 BSP_Model_FindNoteNode, __thiscall(ECX = node; const char* name), RET 4, EAX.
    virtual void* model_find_node_0071ad50(Raw node, const char* name) = 0;
    // 00506DD0 STL checked vector<void*>::push_back, __thiscall(ECX = vector; const value*), RET 4.
    virtual void ptr_vector_push_back_00506dd0(void* vector, void* const* value) = 0;
    // 007D05E0 BSP_Node_AppendChildrenByNamePrefix / 007D06A0 (the numbered-name variant,
    // deferred by cc13_seg47), (ECX = node, EDX = const char* name; vector*), RET 4.
    virtual void node_append_children_by_name_007d05e0(Raw node, const char* name, void* vector) = 0;
    virtual void node_append_numbered_children_007d06a0(Raw node, const char* prefix, void* vector) = 0;
    // 00B6D800 BSP_Node_GetNameHeader, __thiscall(ECX = node), EAX = const NativeString*.
    virtual Raw node_name_00b6d800(Raw node) = 0;
    // --- 007CD5E0: MSVC std::string {+4 buffer/pointer, +14h size, +18h capacity} ---------
    // 00711C30 BSP_PartNamedRecord_CopyLegacyName, __thiscall(ECX = record; out string), RET 4, EAX = out.
    virtual Raw part_named_record_copy_name_00711c30(void* record, Raw out) = 0;
    // 0070F900 string::find_first_of(const char*, pos, count), __thiscall, RET 0Ch, EAX.
    virtual std::uint32_t legacy_string_find_first_of_0070f900(Raw string, const char* set, std::uint32_t pos,
                                                               std::uint32_t count) = 0;
    // 004CDBE0 string::substr into out, __thiscall(ECX = string; out, pos, count), RET 0Ch, EAX = out.
    virtual Raw legacy_string_substring_004cdbe0(Raw string, Raw out, std::uint32_t pos, std::uint32_t count) = 0;
    // 00408720 BSP_CharacterString_AssignCounted, __thiscall(ECX = string; const char*, count), RET 8.
    virtual void character_string_assign_counted_00408720(Raw string, const char* text, std::uint32_t count) = 0;
    // 007E1D80 BSP_PlaneControlSurface_IndexFromName, __thiscall(ECX = string), EAX.
    virtual std::int32_t plane_control_surface_index_from_name_007e1d80(Raw string) = 0;
    // 00BFA66C _atof, __cdecl(const char*), ST0 (vendor CRT).
    virtual double atof_00bfa66c(const char* text) = 0;
    // 007DDD60 BSP_PartRecordRef_Construct, __thiscall(ECX = memory; kind, node, angle), RET 0Ch, EAX.
    virtual void* part_record_ref_construct_007ddd60(void* memory, std::int32_t kind, void* node, float angle) = 0;
    // 0085F990 BSP_DebugDraw_LineRgbaFloats, __fastcall(ECX = p0, EDX = p1; rgba0 16 bytes, rgba1 16 bytes by value).
    virtual void debug_draw_line_rgba_0085f990(const float* p0, const float* p1, const float* rgba0,
                                               const float* rgba1) = 0;
    // 00860BE0 BSP_DebugDraw_WireSphereMarker, __fastcall(ECX = centre, EDX = rgba*; radius, rgba*, detailed).
    virtual void debug_draw_wire_sphere_00860be0(const float* centre, const float* rgba, float radius,
                                                 const float* rgba2, std::int32_t detailed) = 0;
};

// 007C1570, __thiscall(ECX = plane; requested, force), RET 8.
void plane_request_flight_state_007c1570(Seg47bHost& host, Raw plane, std::int32_t requested, bool force);
// 007C17D0, __thiscall(ECX = plane), RET 0.
void plane_notify_takeoff_start_007c17d0(Seg47bHost& host, Raw plane);
// 007C18B0, __thiscall(ECX = plane; float dt), RET 4.
void plane_update_pose_derived_state_007c18b0(Seg47bHost& host, Raw plane, float dt);
// 007C2090 / 007C2130, __thiscall(ECX = plane), RET 0.
void plane_request_flight_state_two_007c2090(Seg47bHost& host, Raw plane);
void plane_request_flight_state_one_007c2130(Seg47bHost& host, Raw plane);
// 007C2AF0, __thiscall(ECX = record; plane, owner), RET 8.
void plane_state_record_serialize_007c2af0(Seg47bHost& host, Raw record, Raw plane, void* owner);
// 007C21D0 / 007C2450, __thiscall(ECX = plane; out float[3]), RET 4, EAX = out (HUD bomb sight).
float* plane_predict_bomb_impact_point_007c21d0(Seg47bHost& host, Raw plane, float* out);
float* plane_predict_rocket_impact_point_007c2450(Seg47bHost& host, Raw plane, float* out);
// 007C4850, __thiscall(ECX = plane class), AL: the class flight-constant derivation.
bool plane_class_derive_flight_constants_007c4850(Seg47bHost& host, Raw cls);
// 007C2680, __thiscall(ECX = plane; peer), RET 4 (vtable +17Ch).
void plane_send_create_message_007c2680(Seg47bHost& host, Raw plane, void* peer);
// 007C2880, __thiscall(ECX = plane; holder, force), RET 8.
void plane_send_control_state_message_007c2880(Seg47bHost& host, Raw plane, void* holder, std::int32_t force);
// 007C29F0, __thiscall(ECX = plane; owner), RET 4 (vtable +184h).
void plane_send_sync_message_007c29f0(Seg47bHost& host, Raw plane, void* owner);
// 007C3700, __thiscall(ECX = plane), RET 0.
void plane_update_bow_wave_effect_007c3700(Seg47bHost& host, Raw plane);
// 007C39B0, (ECX = plane, EDX = target), RET 0. Debug draw; unreferenced in the image.
void plane_debug_draw_gun_lead_007c39b0(Seg47bHost& host, Raw plane, Raw target);
// 007C6820, __thiscall(ECX = plane; float dt), RET 4: the per-step actuator/effect drive.
void plane_drive_actuator_channels_007c6820(Seg47bHost& host, Raw plane, float dt);
// 007CD5E0, __thiscall(ECX = plane), RET 0.
void plane_scan_model_part_records_007cd5e0(Seg47bHost& host, Raw plane);
// 007D0820, __thiscall(ECX = plane), RET 0.
void plane_hide_other_mesh_holders_007d0820(Seg47bHost& host, Raw plane);
// 007C6E10, __thiscall(ECX = plane), RET 0, EAX = 0 / 1 / 2.
std::int32_t plane_stall_warning_level_007c6e10(Seg47bHost& host, Raw plane);
// 007C74A0 / 007C75A0, __thiscall(ECX = plane), RET 0.
void plane_register_firing_guns_007c74a0(Seg47bHost& host, Raw plane);
void plane_unregister_firing_guns_007c75a0(Seg47bHost& host, Raw plane);
// 007C95A0, __thiscall(ECX = plane), RET 0.
void plane_apply_nose_art_decal_007c95a0(Seg47bHost& host, Raw plane);
// 007C9770, __thiscall(ECX = plane), RET 0.
void plane_sentity_init_pass_c_007c9770(Seg47bHost& host, Raw plane);
// 007CA5F0, __thiscall(ECX = plane; bool firing), RET 4.
void plane_set_gun_fire_effects_007ca5f0(Seg47bHost& host, Raw plane, bool firing);
// 007CB440, __thiscall(ECX = plane; out NativeString), RET 4, EAX = out.
Raw plane_format_debug_name_007cb440(Seg47bHost& host, Raw plane, Raw out);
// 007CC580, __thiscall(ECX = plane), RET 0 (tail jump to 0095D400).
void plane_on_killed_007cc580(Seg47bHost& host, Raw plane);
// 007CCA50, __cdecl(), RET 0.
void plane_send_sync_ack_batch_007cca50(Seg47bHost& host);
// 007CCFA0, __thiscall(ECX = plane; message), RET 4, AL.
bool plane_handle_message_007ccfa0(Seg47bHost& host, Raw plane, Raw message);
// 007CD930, __thiscall(ECX = plane), RET 0.
void plane_compute_pilot_fires_007cd930(Seg47bHost& host, Raw plane);
// 007CDF20, __thiscall(ECX = plane), RET 0 (vtable slot +9Ch, the attach override).
void plane_attach_lua_and_read_type_007cdf20(Seg47bHost& host, Raw plane);
// 007CFA30, __thiscall(ECX = plane class), RET 0: the plane class destructor body.
void plane_class_base_destruct_007cfa30(Seg47bHost& host, Raw cls);
// 007C78A0, __thiscall(ECX = plane; state, flag byte), RET 8.
void plane_set_flight_state_unguarded_007c78a0(Seg47bHost& host, Raw plane, std::int32_t state, std::uint8_t flag);

}  // namespace bsp::cc13_seg47b
