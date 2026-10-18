#pragma once

// cc13_segmix: undone game bodies from segments 51, 58, 30, 41, 42, 50, 61, 54
// and 46 that have no cc9 host (packet cc13-seg40F segmix, docs/CC13_SEGMIX.md).
// Names are hypotheses, not recovered symbols. These are new C++ interfaces over
// the native records, not drop-in replacements. Nothing is wired. Same
// conventions as cc13_seg40: raw byte blocks touched only at the offsets the image
// uses, and one named host hook per native call, at the same point with the same
// arguments.

#include <cstdint>

namespace bsp {

class Cc13SegmixHost {
public:
    virtual ~Cc13SegmixHost() = default;

    // ---- CRT ----
    // 00BF65AC _free (cdecl, block).
    virtual void crt_free_00bf65ac(void* block) = 0;

    // ---- SHIP_CREATE base message (native) ----
    // 00813A20: __thiscall(ECX = message; word, ship, type), RET 0Ch.
    virtual void ship_create_construct_00813a20(void* message, std::uint32_t word, void* ship,
                                                std::uint32_t type) = 0;
    // 0080E2C0 BSP_SessionMessage_SHIP_CREATE_ConstructForRead: __fastcall(ECX = message).
    virtual void ship_create_construct_for_read_0080e2c0(void* message) = 0;
    // 0080E380, the SHIP_CREATE destructor: __thiscall(ECX = message).
    virtual void ship_create_destruct_0080e380(void* message) = 0;
    // 00813B10: __thiscall(ECX = message; writer), RET 4.
    virtual void ship_create_write_00813b10(void* message, void* writer) = 0;
    // 006CC800 BSP_AirOpsStockSnapshot_Capture: __thiscall(ECX = snapshot; block or null), RET 4.
    virtual void air_ops_stock_snapshot_capture_006cc800(void* snapshot, void* block) = 0;
    // 006D44F0 BSP_AirOpsStockSnapshot_Write: __thiscall(ECX = snapshot; writer), RET 4.
    virtual void air_ops_stock_snapshot_write_006d44f0(void* snapshot, void* writer) = 0;
    // 00779FC0 BSP_Entity_SendMessageToPeer: __thiscall(ECX = entity; peer, message), RET 8.
    virtual void entity_send_message_to_peer_00779fc0(void* entity, std::uint32_t peer, void* message) = 0;

    // ---- 007593D0 mother-ship init ----
    // 00822C20 BSP_UnitInstance_SEntityInit: __thiscall(ECX = unit).
    virtual void unit_instance_sentity_init_00822c20(void* unit) = 0;
    // 0071AD50 BSP_Model_FindNoteNode: __thiscall(ECX = model; const char* key), RET 4; EAX = node.
    virtual void* model_find_note_node_0071ad50(void* model, const char* key) = 0;
    // 00B6DB60 BSP_Node_GetLocalMatrix: __thiscall(ECX = node); EAX = the 4x4.
    virtual const void* node_get_local_matrix_00b6db60(void* node) = 0;
    // 004134F0 BSP_Matrix_Copy4x4X87: __thiscall(ECX = destination; source), RET 4.
    virtual void matrix_copy_4x4_004134f0(void* destination, const void* source) = 0;
    // 006C0D20 BSP_AirOpsBlock_CreateHolder: __thiscall(ECX = block; const float* point,
    // float a, float b), RET 0Ch.
    virtual void air_ops_block_create_holder_006c0d20(void* block, const float* point, float a, float b) = 0;
    // 006CADD0 BSP_AirOps_LoadFromScene: __thiscall(ECX = block; properties), RET 4.
    virtual void air_ops_load_from_scene_006cadd0(void* block, void* properties) = 0;
    // 006D0A10 BSP_MotherShipSite_Create: __thiscall(ECX = ship); EAX = the site.
    virtual void* mothership_site_create_006d0a10(void* ship) = 0;
    // 009238A0 SEntity_GetSavedEntityLuaData: __thiscall(ECX = entity; out 14h LuaObject), RET 4.
    virtual void saved_entity_lua_data_009238a0(void* entity, void* out_lua_object) = 0;
    // 004425C0 BSP_LuaReader_Construct: __thiscall(ECX = reader; 14h LuaObject by value), RET 14h.
    virtual void lua_reader_construct_004425c0(void* reader, void* lua_object_by_value) = 0;
    // 00BD8E20 BSP_LuaReader_EnterKey: __thiscall(ECX = reader; int kind, key), RET 8.
    virtual void lua_reader_enter_key_00bd8e20(void* reader, std::int32_t kind, const char* key) = 0;
    // 00BD7A20 BSP_LuaReader_LeaveKeyThunk: __thiscall(ECX = reader).
    virtual void lua_reader_leave_key_00bd7a20(void* reader) = 0;
    // 006BEF30 BSP_AirOpsBlock_ReadTrafficTable: __thiscall(ECX = block; reader), RET 4.
    virtual void air_ops_block_read_traffic_table_006bef30(void* block, void* reader) = 0;
    // 00441A20 BSP_LuaReader_DestroyStackInstance: __thiscall(ECX = reader).
    virtual void lua_reader_destroy_00441a20(void* reader) = 0;
    // 006BA670 BSP_AirOpsBlock_ForwardToTraffic08: __thiscall(ECX = block; float dt), RET 4.
    virtual void air_ops_block_tick_006ba670(void* block, float dt) = 0;

    // ---- session messages ----
    // 0075B430, the session message base constructor: __thiscall(ECX = message; type), RET 4.
    virtual void session_message_construct_0075b430(void* message, std::uint32_t type) = 0;

    // ---- 0093B920 repair-task destructor (cc13-segmixD) ----
    // 00BF6713 _invalid_parameter_noinfo (the checked-iterator failure).
    virtual void crt_invalid_parameter_noinfo_00bf6713() = 0;
    // [00CE2220] InterlockedDecrement; EAX = the new count.
    virtual std::int32_t interlocked_decrement(volatile std::int32_t* value) = 0;
    // Reference-counted object slot 0, no arguments (the release at a zero count).
    virtual void vcall_00_release(void* object) = 0;
    // 00867B10 BSP_PointEffect_StopChildren: __thiscall(ECX = effect).
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;
    // 0093B300 STL_DestroyRange_0093b300 and 0081C940 STL_DestroyRangeRefPtrPair-shaped
    // range destroys: __fastcall(ECX = first, EDX = last; vector, owner). Template
    // bodies, bind-only.
    virtual void stl_destroy_range_0093b300(void* first, void* last, void* vector, void* owner) = 0;
    virtual void stl_destroy_range_0081c940(void* first, void* last, void* vector, void* owner) = 0;

    // ---- 0092F930 ship controller pose (cc13-segmixD) ----
    // 00414DB0 BSP_EntityPose_RefreshWorld: __thiscall(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00C43EA0: __thiscall(ECX = dyn body; out 3x4 transform, float t), RET 8; EAX = out.
    virtual void* dyn_body_transform_at_00c43ea0(void* body, void* out, float t) = 0;
    // 00C33650 Dyn_Transform_Expand3x4To4x4: __thiscall(ECX = transform; float out[16]), RET 4.
    virtual void dyn_transform_expand_00c33650(void* transform, float* out_matrix) = 0;
    // 0042D2E0 BSP_Matrix_ExtractNormalizedBasisAngles: __fastcall(ECX = matrix, EDX =
    // float* c; float* b, float* a), RET 8.
    virtual void matrix_extract_basis_angles_0042d2e0(const float* matrix, float* c, float* b, float* a) = 0;
    // 00467050 BSP_Matrix_SetRotationFromEuler: __thiscall(ECX = matrix; float c, float b,
    // float a), RET 0Ch.
    virtual void matrix_set_rotation_from_euler_00467050(float* matrix, float c, float b, float a) = 0;
    // 0042ED50 BSP_SceneNode_InvalidateSubtreePose: __thiscall(ECX = node).
    virtual void scene_node_invalidate_subtree_pose_0042ed50(void* node) = 0;

    // ---- segment 46 camera (cc13-segmixE) ----
    // [00E188A8], the game object.
    virtual void* game_global_00e188a8() = 0;
    // The unit's vtable slot +5Ch(kind): __thiscall(ECX = unit; kind), RET 4; AL.
    virtual bool unit_vcall_5c_is_kind(void* unit, std::int32_t kind) = 0;
    // 00793610 / 00793670 / 007936D0: __thiscall(ECX = player lists; unit), RET 4;
    // EAX = the unit's index in the first / second / third list, or -1.
    virtual std::int32_t player_lists_index_a_00793610(void* lists, void* unit) = 0;
    virtual std::int32_t player_lists_index_b_00793670(void* lists, void* unit) = 0;
    virtual std::int32_t player_lists_index_c_007936d0(void* lists, void* unit) = 0;
    // 007A6910: __thiscall(ECX = camera; unit or null), RET 4 (the camera's target set).
    virtual void movcam_set_target_007a6910(void* camera, void* unit) = 0;
    // 005CD640 BSP_VehicleCameraArray_Resize: __thiscall(ECX = array; count), RET 4.
    virtual void vehicle_camera_array_resize_005cd640(void* array, std::int32_t count) = 0;
    // 005CD820 BSP_VehicleCameraArray_At: __thiscall(ECX = array; index), RET 4; EAX = entry (5Ch).
    virtual void* vehicle_camera_array_at_005cd820(void* array, std::int32_t index) = 0;
    // 007A6710 BSP_MovCam_EnterVehicleCamera: __thiscall(ECX = camera; index), RET 4.
    virtual void movcam_enter_vehicle_camera_007a6710(void* camera, std::int32_t index) = 0;
    // 0079F710 BSP_MovCam_GetVehicleCameraPose: __thiscall(ECX = camera; out pose,
    // object, index, bool* failed), RET 10h.
    virtual void movcam_get_vehicle_camera_pose_0079f710(void* camera, void* out_pose, void* object,
                                                         std::int32_t index, std::uint8_t* flag) = 0;
    // 00797070 BSP_MovCam_SetLookTarget: __thiscall(ECX = camera; a, b), RET 8.
    virtual void movcam_set_look_target_00797070(void* camera, std::uint32_t a, std::uint32_t b) = 0;

    // ---- 007A74F0 path-walk point properties (cc13-segmixF) ----
    // 00BF681B operator new (cdecl, size).
    virtual void* crt_operator_new_00bf681b(std::uint32_t size) = 0;
    // 0079C830 BSP_MovCam_ClearPathRecords: __thiscall(ECX = camera).
    virtual void movcam_clear_path_records_0079c830(void* camera) = 0;
    // 0079EB10, the pointer vector resize: __thiscall(ECX = vector; count, value), RET 8.
    virtual void pointer_vector_resize_0079eb10(void* vector, std::uint32_t count, void* value) = 0;
    // 007A5A00 BSP_MovCam_BeginPathWalk: __thiscall(ECX = camera; float, byte, float,
    // name, variants), RET 14h.
    virtual void movcam_begin_path_walk_007a5a00(void* camera, float a, std::uint8_t b, float c, void* name,
                                                 std::uint32_t variants) = 0;
    // LuaObject blocks are 14h bytes, handled only through these.
    // 00B65F50 BSP_LuaObject_Construct and 00B67700, the destructor: __thiscall(ECX = object).
    virtual void lua_object_construct_00b65f50(void* object) = 0;
    virtual void lua_object_destroy_00b67700(void* object) = 0;
    // 00B67080 BSP_LuaObject_IterateFirst / 00B67190 BSP_LuaObject_IterateNext:
    // __thiscall(ECX = table; key, value), RET 8.
    virtual void lua_object_iterate_first_00b67080(void* table, void* key, void* value) = 0;
    virtual void lua_object_iterate_next_00b67190(void* table, void* key, void* value) = 0;
    // 00B66420 BSP_LuaObject_IsUnbound: __thiscall(ECX ignored; object), RET 4; AL. The end
    // of an iteration (the image passes the table in ECX).
    virtual bool lua_object_is_unbound_00b66420(void* ignored, void* object) = 0;
    // 00B65FB0 BSP_LuaObject_IsNil: __thiscall(ECX = object); AL.
    virtual bool lua_object_is_nil_00b65fb0(void* object) = 0;
    // 00B66290 BSP_LuaObject_GetInteger: __thiscall(ECX = object); EAX.
    virtual std::int32_t lua_object_get_integer_00b66290(void* object) = 0;
    // 00B66270 BSP_LuaObject_GetNumber: __thiscall(ECX = object); ST0, which every caller here
    // stores as a float.
    virtual double lua_object_get_number_00b66270(void* object) = 0;
    // 00B67800 BSP_LuaObject_GetByName: __thiscall(ECX = table; out object, const char* key),
    // RET 8; EAX = out.
    virtual void* lua_object_get_by_name_00b67800(void* table, void* out, const char* key) = 0;
    // 00B67690, the LuaObject assignment: __thiscall(ECX = object; source), RET 4.
    virtual void lua_object_assign_00b67690(void* object, void* source) = 0;
    // 00888AA0 BSP_ObjectHandle_FromLuaTable: __thiscall(ECX = object); EAX = the entity.
    virtual void* object_handle_from_lua_table_00888aa0(void* object) = 0;
    // 00888760 BSP_LuaObject_ReadVector3: __fastcall(ECX = out float[3], EDX = object); EAX = out.
    virtual float* lua_object_read_vector3_00888760(float* out, void* object) = 0;
    // 00694A60 BSP_Observer_RegisterPair / 006952A0 BSP_Observer_UnregisterPair:
    // __fastcall(ECX = subject, EDX = observer).
    virtual void observer_register_pair_00694a60(void* subject, void* observer) = 0;
    virtual void observer_unregister_pair_006952a0(void* subject, void* observer) = 0;
};

// ---- segment 42: the mother ship and its MOTHERSHIP_CREATE message ----
// The message is SHIP_CREATE (12Ch bytes) plus an air-ops stock snapshot at +12Ch
// (vtable 00CF8EB8, vectors +138h..+140h and +148h..+150h); 154h bytes in all.
void cc13_segmix_mothership_create_destruct_00759020(void* message, Cc13SegmixHost& host);
void cc13_segmix_mothership_create_write_007590e0(void* message, void* writer, Cc13SegmixHost& host);
void* cc13_segmix_mothership_create_construct_007592a0(void* message, std::uint32_t word, void* ship,
                                                       Cc13SegmixHost& host);
void cc13_segmix_mothership_send_create_message_00759330(void* ship, std::uint32_t peer, Cc13SegmixHost& host);
void cc13_segmix_mothership_init_all_007593d0(void* ship, Cc13SegmixHost& host);
void* cc13_segmix_landingship_create_construct_for_read_00759920(void* message, Cc13SegmixHost& host);
void* cc13_segmix_mothership_create_construct_for_read_00766ef0(void* message, Cc13SegmixHost& host);

// ---- segment 61: ship systems and failures ----
void* cc13_segmix_failure_message9b_construct_0093a2c0(void* message, std::uint32_t value_20, std::uint32_t value_24,
                                                       std::uint8_t flag_28, Cc13SegmixHost& host);
void cc13_segmix_repair_task_destruct_0093b920(void* task, Cc13SegmixHost& host);
void cc13_segmix_unit_controller_update_pose_0092f930(void* controller, float seconds, Cc13SegmixHost& host);

// ---- segment 46: the mission camera ----
void cc13_segmix_movcam_cycle_target_007a84d0(void* camera, std::uint8_t backward, Cc13SegmixHost& host);
void cc13_segmix_movcam_update_hand_mode_007a87b0(void* camera, Cc13SegmixHost& host);
// `table` is the 14h-byte LuaObject passed by value; this function destroys it, as the
// image does.
void cc13_segmix_movcam_set_path_walk_point_property_007a74f0(void* camera, void* table, float a,
                                                              std::uint8_t reverse, float c, void* name,
                                                              std::uint32_t variants, Cc13SegmixHost& host);

}  // namespace bsp
