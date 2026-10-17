#pragma once

// cc13_bigB1: three large leftovers of the segment 15-51 sweep, each as a
// whole body (packet cc13-bigB1, docs/CC13_BIGB1.md). Every name is a
// hypothesis, not a recovered symbol. These are new C++ interfaces over the
// native records, not drop-in replacements of the original ABI. Nothing here is
// wired into the game.
//
// Same conventions as cc13_sweepB..B9: native objects are raw byte blocks
// touched only at the offsets the image reads or writes, and every call the
// image makes goes through a named host method (or a reconstructed function
// called directly) at the same point with the same arguments.
// Cc13BigB1Host extends Cc13SweepB9Host, so the earlier hooks are reused as
// they are.

#include "bsp/cc13_sweepB9.hpp"

#include <cstdint>

namespace bsp {

class Cc13BigB1Host : public Cc13SweepB9Host {
public:
    // ---- movie camera (segments 26, 45, 46), callees of 007A91A0 ----
    // 007A84D0 (ledger BSP_Recon_PublishedListCursor, no record): __thiscall(ECX = camera, int), RET 4.
    virtual void movcam_007a84d0(void* camera, std::int32_t value) = 0;
    // 007A87B0 BSP_MovCam_UpdateHandMode: __thiscall(ECX = camera), RET.
    virtual void movcam_update_hand_mode_007a87b0(void* camera) = 0;
    // 0079F9E0 BSP_MovCam_EditVehicleCameraWindow: __thiscall(ECX = camera), RET.
    virtual void movcam_edit_vehicle_camera_window_0079f9e0(void* camera) = 0;
    // 007A01F0 BSP_MovCam_DebugDrawVehicleCameras: __thiscall(ECX = camera), RET.
    virtual void movcam_debug_draw_vehicle_cameras_007a01f0(void* camera) = 0;
    // 007A0DD0 BSP_MovCam_HasVehicleCameraPose: __thiscall(ECX = camera, entity, int index), RET 8, AL.
    virtual bool movcam_has_vehicle_camera_pose_007a0dd0(void* camera, void* entity, std::int32_t index) = 0;
    // 0078E2B0 BSP_MovCam_AngleWindowContains: __thiscall(ECX = four floats, float a, float b), RET 8, AL.
    virtual bool movcam_angle_windows_contain_0078e2b0(const void* windows, float a, float b) = 0;
    // 007901D0 BSP_MovCam_ApplyInterpolators: __thiscall(ECX = interpolator record, camera), RET 4.
    virtual void movcam_apply_interpolator_007901d0(void* record, void* camera) = 0;
    // 00790540 BSP_MovCam_KeepAboveGround: __thiscall(ECX = camera), RET.
    virtual void movcam_keep_above_ground_00790540(void* camera) = 0;
    // 00793CD0 BSP_CameraRef_TransformPoint: __thiscall(ECX = ref, const float* p, char level), RET 8.
    virtual void camera_ref_set_point_00793cd0(void* ref, const float* p, std::int8_t level) = 0;
    // 007943D0 BSP_MovCam_IntegratePosition: __thiscall(ECX = camera, float dt), RET 4.
    virtual void movcam_integrate_007943d0(void* camera, float dt) = 0;
    // 00794B80 BSP_MovCam_SteerPosition: __thiscall(ECX = camera, float x, y, z, max, byte square), RET 14h.
    virtual void movcam_steer_position_00794b80(void* camera, float x, float y, float z, float max,
                                                std::uint8_t square) = 0;
    // 00794D60 BSP_MovCam_SteerAngles: __thiscall(ECX = camera, float x, y, max, byte square, target), RET 14h.
    virtual void movcam_steer_angles_00794d60(void* camera, float x, float y, float max, std::uint8_t square,
                                              void* target) = 0;
    // 00794F70 BSP_MovCam_SteerAnglesBraking: __thiscall(ECX = camera, float x, y, target), RET 0Ch.
    virtual void movcam_steer_angles_braking_00794f70(void* camera, float x, float y, void* target) = 0;
    // 00795300 BSP_MovCam_LookBasis: __thiscall(ECX = camera, float out[16], target), RET 8, EAX = out.
    virtual float* movcam_look_basis_00795300(void* camera, float* out, void* target) = 0;
    // 0079D960 BSP_MovCam_CheckArrival: __thiscall(ECX = camera, float goal x, y, z, threshold), RET 10h.
    virtual void movcam_check_arrival_0079d960(void* camera, float x, float y, float z, float threshold) = 0;
    // 0079DA90 BSP_MovCam_CheckSecondaryArrival: __thiscall(ECX = camera, target, float threshold), RET 8.
    virtual void movcam_check_secondary_arrival_0079da90(void* camera, void* target, float threshold) = 0;
    // 0064A740 BSP_Math_SignFloat_0064A740: __stdcall(float), RET 4, EAX = -1/0/1
    // (reconstructed as sign_float_0064a740 in band_0060_cc13, a bsp_game file).
    virtual std::int32_t math_sign_float_0064a740(float value) = 0;

    // ---- plane HUD screen frame (segment 24), callees of 0060ABD0 ----
    // Globals read or written: 00E188D8 (the local party object), 00E198B8,
    // byte 00F88A32, byte 00E19694 (written).
    virtual void* global_00e188d8() = 0;
    virtual void* global_00e198b8() = 0;
    virtual std::uint8_t global_00f88a32() = 0;
    virtual std::uint8_t* global_00e19694() = 0;
    // 006485A0 BSP_InGameHudRoot_ForceSelectUnit: __thiscall(ECX = [00E198C4]+40h), RET.
    virtual void ingame_hud_force_select_unit_006485a0(void* hud_root) = 0;
    // 00647300 BSP_InGameHudRoot_SetSpectatedUnit: __thiscall(ECX = hud root, unit), RET 4.
    virtual void ingame_hud_set_spectated_unit_00647300(void* hud_root, void* unit) = 0;
    // 004CC460 BSP_FrontEndManager_PushInterfaceRequest: __thiscall(ECX = [00E198C4], int id, arg), RET 8.
    virtual void frontend_push_interface_request_004cc460(void* gui, std::int32_t id, void* arg) = 0;
    // 007C3CB0 BSP_Plane_StepCountdown964: __thiscall(ECX = unit, float dt), RET 4.
    virtual void plane_step_countdown_007c3cb0(void* unit, float dt) = 0;
    // Unit vtable +1C0h: __thiscall(ECX = unit), EAX.
    virtual std::int32_t unit_vcall_1c0(void* unit) = 0;
    // 007C7800 BSP_Plane_SyncCockpitCameraGunShake: __thiscall(ECX = unit, camera), RET 4.
    virtual void plane_sync_cockpit_camera_gun_shake_007c7800(void* unit, void* camera) = 0;
    // 00927F30 BSP_UnitInstance_IsLocalPlayerRole: __thiscall(ECX = unit, int), RET 4, AL.
    virtual bool unit_is_local_player_role_00927f30(void* unit, std::int32_t which) = 0;
    // 0077C470 (unnamed; cc9's role transfer): __thiscall(ECX = unit, int a, int b), RET 8.
    virtual void unit_0077c470(void* unit, std::int32_t a, std::int32_t b) = 0;
    // 007BB9A0 BSP_Plane_IsLocalInputEnabled: __thiscall(ECX = unit), AL.
    virtual bool plane_is_local_input_enabled_007bb9a0(void* unit) = 0;
    // 00645160 BSP_Unit_IsSelectableForCurrentPlayer: __fastcall(ECX = unit, DL), AL.
    virtual bool unit_is_selectable_for_current_player_00645160(void* unit, std::uint8_t dl) = 0;
    // 0047B850 (unnamed): __thiscall(ECX = unit), AL.
    virtual bool unit_0047b850(void* unit) = 0;
    // 00604F30 BSP_PlaneHudScreen_GetCockpitViewFlag: __thiscall(ECX = screen), EAX.
    virtual std::int32_t plane_hud_cockpit_view_flag_00604f30(void* screen) = 0;
    // 00608280 BSP_PlaneHudScreen_ApplyViewMode / 006068A0 BSP_PlaneHudScreen_UpdateViewFov:
    // __thiscall(ECX = screen), RET.
    virtual void plane_hud_apply_view_mode_00608280(void* screen) = 0;
    virtual void plane_hud_update_view_fov_006068a0(void* screen) = 0;
    // 00604A50 (unnamed): __thiscall(ECX = unit), AL (unit vtable +5Ch kind 17h and +C24h).
    virtual bool unit_00604a50(void* unit) = 0;
    // 00440490 (retained reference copy construct): __thiscall(ECX = destination slot, const slot*), RET 4.
    virtual void retained_reference_copy_construct_00440490(void* destination, const void* source) = 0;
    // 008687C0 BSP_PointEffect_CreateWithParentMatrix: __fastcall(ECX = out slot, EDX =
    // parent node; definition slot by value (the callee releases it), const float
    // matrix[16], int, int, int), RET 14h, EAX = out.
    virtual void* point_effect_create_with_parent_matrix_008687c0(void* out, void* parent, void* definition,
                                                                   const float* matrix, std::int32_t a,
                                                                   std::int32_t b, std::int32_t c) = 0;
    // 00440A30 BSP_RefPtrSlot_Release: __thiscall(ECX = slot), RET.
    virtual void ref_ptr_slot_release_00440a30(void* slot) = 0;
    // 00484620 BSP_EffectReference_AdoptWithoutRetain: __thiscall(ECX = slot, value), RET 4.
    virtual void effect_reference_adopt_without_retain_00484620(void* slot, void* value) = 0;
    // 0042B570 BSP_ContactPointList_Append: __thiscall(ECX = list (8 x 18h + count +C0h), a*, b*), RET 8.
    virtual void contact_point_list_append_0042b570(void* list, const float* a, const float* b) = 0;
    // 0098B370 BSP_SpatialIndex_SweepSegments: __thiscall(ECX = index; list, int, hit record), RET 0Ch, AL tested.
    virtual bool spatial_index_sweep_segments_0098b370(void* index, void* list, std::int32_t flag,
                                                       void* hit) = 0;
    // Unit vtable +38h: __thiscall(ECX = unit), ST0 (popped unused here).
    virtual double unit_vcall_38(void* unit) = 0;
    // 0047C240 BSP_Transform_InitTranslation_0047c240: __fastcall(ECX = float out[16], EDX = const float[3]), EAX.
    virtual float* transform_init_translation_0047c240(float* out, const float* translation) = 0;
    // 004C5090 BSP_InputManager_IsActionHeld / 004C50D0 BSP_InputManager_WasActionPressed:
    // __thiscall(ECX = [00E188A8], int action), RET 4, AL.
    virtual bool input_action_is_held_004c5090(void* game, std::int32_t action) = 0;
    virtual bool input_action_was_pressed_004c50d0(void* game, std::int32_t action) = 0;
    // 004C5070 BSP_InputManager_GetActionHoldTime: __thiscall(ECX = [00E188A8], int action), RET 4, ST0.
    virtual float input_action_hold_time_004c5070(void* game, std::int32_t action) = 0;
    // 00606250 BSP_InputManager_IsActionEnabled: __thiscall(ECX = [00E188A8], int action), RET 4, AL.
    virtual bool input_action_is_enabled_00606250(void* game, std::int32_t action) = 0;
    // 0042AA00 (unnamed): __thiscall(ECX = screen +98h object, float, float, int), RET 0Ch.
    virtual void cockpit_camera_0042aa00(void* camera, float a, float b, std::int32_t c) = 0;
    // Command object ([unit+9D4h]) vtable +114h: __thiscall(ECX), EAX.
    virtual void* command_vcall_114(void* command) = 0;
    // 0071EB60 BSP_EntityCommand_ActiveTargetDescriptor: __thiscall(ECX), EAX.
    virtual void* entity_command_active_target_descriptor_0071eb60(void* command) = 0;
    // 0042D0D0 BSP_Vector3f_TransformDirectionOptionalNormalize: __fastcall(ECX = out, EDX =
    // in; const float matrix[16], int normalize), RET 8, EAX = out.
    virtual float* vector3_transform_direction_0042d0d0(float* out, const float* in, const float* matrix,
                                                        std::int32_t normalize) = 0;
    // 00415690 BSP_Math_ClampInPlace: __fastcall(ECX = float* value, EDX = const float* low;
    // const float* high), RET 4.
    virtual void math_clamp_in_place_00415690(float* value, const float* low, const float* high) = 0;
    // 00923810 BSP_EntityAncestor: __thiscall(ECX = entity, int), RET 4, EAX.
    virtual void* entity_ancestor_00923810(void* entity, std::int32_t level) = 0;
    // 00927880 (unnamed): __thiscall(ECX = [00E188D8]), EAX.
    virtual void* party_00927880(void* party) = 0;
    // 006062B0 (unnamed axis filter): __thiscall(ECX = filter record, float, float, float dt), RET 0Ch, ST0.
    virtual float plane_hud_axis_filter_006062b0(void* filter, float a, float b, float dt) = 0;
    // 004F7740 BSP_PriorityLists_GetTopPopulatedLevel: no ECX, EAX.
    virtual std::int32_t priority_lists_top_level_004f7740() = 0;
    // 00A92050 BSP_InputAction_AnyBindingForcesUnitScale / 00A92090
    // BSP_InputAction_HasDeviceQuery1C: __thiscall(ECX = [input+4]+1E90h), AL.
    virtual bool input_action_any_binding_forces_unit_scale_00a92050(void* action) = 0;
    virtual bool input_action_has_device_query_00a92090(void* action) = 0;
    // 006529E0 TRIV_body_006529E0: __thiscall(ECX = [00E198C4]+BCh), AL.
    virtual bool hud_006529e0(void* object) = 0;
    // 00608C00 BSP_BitFlagSet_TestFlag: __thiscall(ECX, int), RET 4, AL.
    virtual bool bit_flag_set_test_00608c00(void* set, std::int32_t flag) = 0;
    // 006082D0 BSP_PlaneHudScreen_UpdateBombFire: __thiscall(ECX = screen, block*), RET 4.
    virtual void plane_hud_update_bomb_fire_006082d0(void* screen, void* block) = 0;
    // 007B8C90 BSP_Plane_SetPilotCommandBlock: __thiscall(ECX = unit, const block*), RET 4.
    virtual void plane_set_pilot_command_block_007b8c90(void* unit, const void* block) = 0;
    // 007B9510 BSP_WeaponController_FirstBombClass: __thiscall(ECX = unit), EAX.
    virtual void* weapon_first_bomb_class_007b9510(void* unit) = 0;
    // 007BC8C0 BSP_WeaponController_TotalBombRounds: __thiscall(ECX = unit), ST0.
    virtual float weapon_total_bomb_rounds_007bc8c0(void* unit) = 0;
    // 0042ECA0 BSP_ObserverRef_Set_0042ECA0: __thiscall(ECX = slot, object), RET 4.
    virtual void observer_ref_set_0042eca0(void* slot, void* object) = 0;
    // Screen +E4h object vtable +108h: __thiscall(ECX), EAX; that result's vtable +38h: AL.
    virtual void* object_vcall_108(void* object) = 0;
    virtual bool object_vcall_38_bool(void* object) = 0;
    // 00609BD0 BSP_HudPlaneScreen_UpdateInstruments: __thiscall(ECX = [00E198C4]+68h,
    // float dt, byte, byte), RET 0Ch.
    virtual void hud_plane_update_instruments_00609bd0(void* instruments, float dt, std::uint8_t a,
                                                       std::uint8_t b) = 0;
    // Unit vtable +204h: __thiscall(ECX = unit), ST0.
    virtual float unit_vcall_204(void* unit) = 0;
    // 004155B0 BSP_Float_ClampByPointer: __fastcall(ECX = const float* value, EDX = const
    // float* low; const float* high), RET 4, ST0.
    virtual float float_clamp_by_pointer_004155b0(const float* value, const float* low, const float* high) = 0;
    // 0042AC60 BSP_Math_StepTowards: __thiscall(ECX = float* value; float target, float rate), RET 8.
    virtual void math_step_towards_0042ac60(float* value, float target, float rate) = 0;
    // 00BFA020 _CItan (its other caller is math_tan 00A61730): ST0 in, ST0 out.
    virtual double crt_citan_00bfa020(double value) = 0;

    // ---- unit SEntityInit (segment 51), callees of 00822C20 ----
    // Byte 00E0C978 (gameplay modifiers enabled).
    virtual std::uint8_t global_00e0c978() = 0;
    // 00822B70 BSP_UnitInstance_ResetNavigatorParams: __thiscall(ECX = unit, int), RET 4.
    virtual void unit_reset_navigator_params_00822b70(void* unit, std::int32_t flag) = 0;
    // Unit vtable +10h / +230h: __thiscall(ECX = unit), no stack arguments.
    virtual void unit_vcall_10(void* unit) = 0;
    virtual void unit_vcall_230(void* unit) = 0;
    // Unit vtable +128h: __thiscall(ECX = unit, int), RET 4 (the skill / crew setter).
    virtual void unit_vcall_128(void* unit, std::int32_t value) = 0;
    // 00955420 BSP_UnitInstance_InitializeSceneBindings: __thiscall(ECX = unit), RET.
    virtual void unit_initialize_scene_bindings_00955420(void* unit) = 0;
    // 00866CD0 BSP_PointEffect_FindOrCreateNamedChild: __thiscall(ECX = effect; out slot*,
    // const NativeString* name), RET 8, EAX = out.
    virtual void* point_effect_find_or_create_named_child_00866cd0(void* effect, void* out, const void* name) = 0;
    // 006FBEB0 BSP_RetainedReference_AssignFromSlot: __thiscall(ECX = slot, const slot*), RET 4.
    virtual void retained_reference_assign_from_slot_006fbeb0(void* slot, const void* source) = 0;
    // 00822460 (vector resize): __thiscall(ECX = vector; count, dword value by value), RET 8.
    virtual void vector_resize_00822460(void* vector, std::uint32_t count, std::uint32_t value) = 0;
    // 00818EA0 BSP_UnitInstance_ResetPoseDependentState: __thiscall(ECX = unit), RET.
    virtual void unit_reset_pose_dependent_state_00818ea0(void* unit) = 0;
    // 008F2260 BSP_ScenePropertyBag_Find: __thiscall(ECX = bag, const char* key), RET 4, EAX = record or 0
    // (record +4h type, 0 = int; +0Ch payload).
    virtual void* scene_property_bag_find_008f2260(void* bag, const char* key) = 0;
    // 0080FC30 BSP_UnitInstance_GetReferenceSpeed: __thiscall(ECX = unit), ST0.
    virtual double unit_reference_speed_0080fc30(void* unit) = 0;
    // 0080D9B0 BSP_UnitOrderRing_SetThrottleImmediate: __thiscall(ECX = unit+838h, float), RET 4.
    virtual void unit_order_ring_set_throttle_immediate_0080d9b0(void* ring, float throttle) = 0;
    // 0092D770 BSP_UnitController_SetAxialSpeed: __thiscall(ECX = [unit+1018h], float), RET 4.
    virtual void unit_controller_set_axial_speed_0092d770(void* controller, float speed) = 0;
    // 0092D730 BSP_UnitController_GetBodyAxisSpeed: __thiscall(ECX = [unit+1018h]), ST0.
    virtual double unit_controller_body_axis_speed_0092d730(void* controller) = 0;
    // 0081E350 (unnamed): __thiscall(ECX = unit+10B8h, const class+664h*), RET 4.
    virtual void unit_0081e350(void* object, const void* source) = 0;
    // 0074F490 BSP_LeakManager_Init: __thiscall(ECX = unit+10D4h; unit, int), RET 8.
    virtual void leak_manager_init_0074f490(void* manager, void* unit, std::int32_t value) = 0;
    // 00711C30 BSP_PartNamedRecord_CopyLegacyName: __thiscall(ECX = record, std::string* out), RET 4.
    virtual void part_named_record_copy_legacy_name_00711c30(void* record, void* out_string) = 0;
    // 00814870 (std::string find): __thiscall(ECX = string; const char*, pos, count), RET 0Ch, EAX.
    virtual std::uint32_t stl_string_find_00814870(const void* string, const char* text, std::uint32_t pos,
                                                   std::uint32_t count) = 0;
    // 00927A80 BSP_PropertyBag_GetSkill: __thiscall(ECX = unit), EAX.
    virtual std::int32_t property_bag_get_skill_00927a80(void* unit) = 0;
    // Weapon director ([unit+738h]) vtable +24h / +28h: __thiscall(ECX, int), RET 4, AL;
    // vtable +3Ch: __thiscall(ECX, one dword argument), RET 4.
    virtual bool director_vcall_24(void* director, std::int32_t stance) = 0;
    virtual bool director_vcall_28(void* director, std::int32_t stance) = 0;
    virtual void director_vcall_3c(void* director, const void* argument) = 0;
    // 007214C0 BSP_WeaponDirectorStateMessage_BuildEnables: __thiscall(ECX = message, director), RET 4.
    virtual void weapon_director_message_build_enables_007214c0(void* message, void* director) = 0;
    // 008346E0 BSP_ShipUnit_BindScrewAndRudderNodes / 00834690 TRIV_body_00834690:
    // __thiscall(ECX = unit), RET.
    virtual void ship_unit_bind_screw_and_rudder_nodes_008346e0(void* unit) = 0;
    virtual void unit_00834690(void* unit) = 0;
    // Unit +310h sub-object vtable +0Ch: __thiscall(ECX = unit+310h).
    virtual void unit_part_310_vcall_0c(void* part) = 0;
    // 0080D690 BSP_EntityPath_Construct: __thiscall(ECX = 19Ch block), EAX.
    virtual void* entity_path_construct_0080d690(void* block) = 0;
    // Path vtable +98h: __thiscall(ECX = path; unit, [unit+30h], const float matrix[16]), RET 0Ch;
    // vtable +A0h: __thiscall(ECX = path).
    virtual void path_vcall_98(void* path, void* unit, void* node, const float* matrix) = 0;
    virtual void path_vcall_a0(void* path) = 0;
    // Path +170h point list vtable +8h: __thiscall(ECX = path+170h, const point (0Ch bytes)*), RET 4.
    virtual void path_points_vcall_08(void* points, const void* point) = 0;
    // 0081DA30 (pointer vector insert one): __thiscall(ECX = vector; out*, container, where,
    // const value*), RET 10h.
    virtual void pointer_vector_insert_one_0081da30(void* vector, void* out, void* container, void* where,
                                                    const void* value) = 0;
    // 004A6830 (unnamed): __thiscall(ECX = [[00E188A8]+21D0h], unit), RET 4.
    virtual void unit_manager_004a6830(void* manager, void* unit) = 0;
    // 004842C0 BSP_PointEffect_SetWorldPoint: __thiscall(ECX = effect, const float[3]), RET 4.
    virtual void point_effect_set_world_point_004842c0(void* effect, const float* point) = 0;
    // 008E6430 BSP_GameplayModifiers_ProductForUnit: __thiscall(ECX = [00F88C30], int kind, unit), RET 8, ST0.
    virtual double gameplay_modifiers_product_for_unit_008e6430(void* manager, std::int32_t kind, void* unit) = 0;
};

// 007A91A0 BSP_MovCam_Step: __thiscall(ECX = camera+170h, float dt), RET 4.
// Slot +8h of the +170h sub-object vtable 00D04B1C. `camera` is the MovCam
// object (ECX - 170h); every offset in the body is camera-relative.
void cc13_bigB1_movcam_step_007a91a0(void* camera, float dt, Cc13BigB1Host& host);

// 0060ABD0 BSP_HudPlaneEffectsScreen_Update: __thiscall(ECX = screen, float dt),
// RET 4; slot +20h of vtable 00CF43AC. The player plane's per-frame HUD screen
// step: effects, the forward sweep probe, free look, stick/throttle into the
// pilot command block, bomb timer, instruments and the zoom FOV. Two draws of
// the shared stream 00BD2F10 (ECX = 1) at 0060B0CF / 0060B101.
void cc13_bigB1_hud_plane_effects_update_0060abd0(void* screen, float dt, Cc13BigB1Host& host);

// 00822C20 BSP_UnitInstance_SEntityInit: __thiscall(ECX = unit), RET; slot A0h
// of the game-unit vtables. Class (+538h) speed and colour defaults, the
// four bow/stern effects with their "Sound" children, the scene record
// (+C0h) arms (kind 1: property bag keys; kind 2: the authored record), the
// weapon director enables, screw/rudder binding, the entity paths, the wake
// effects and the burst effects. No 00BD2F10 draws.
void cc13_bigB1_unit_sentity_init_00822c20(void* unit, Cc13BigB1Host& host);

}  // namespace bsp
