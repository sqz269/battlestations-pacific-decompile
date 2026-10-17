#pragma once

// cc13_sweepB6: sixth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB6, docs/CC13_SWEEPB6.md). Every name is a hypothesis, not a
// recovered symbol. These are new C++ interfaces over the native records, not
// drop-in replacements of the original ABI. Nothing here is wired into the game.
//
// Same conventions as cc13_sweepB..B5: native objects are raw byte blocks
// touched only at the offsets the image reads or writes, and every call the
// image makes goes through a named host method at the same point with the same
// arguments. Cc13SweepB6Host extends Cc13SweepB5Host, so the earlier hooks are
// reused as they are.

#include "bsp/cc13_sweepB5.hpp"

#include <cstdint>

namespace bsp {

class Cc13SweepB6Host : public Cc13SweepB5Host {
public:
    // ---- power-ups (segment 58, called) ----
    // 008E9AF0 BSP_PowerUpManager_CollectActive: __thiscall(ECX = manager
    // [00F88C30], five vector addresses, char, char), RET 1Ch.
    virtual void powerup_manager_collect_active_008e9af0(void* manager, void* out_190, void* out_150,
                                                         void* out_160, void* out_170, void* out_180,
                                                         std::int8_t a, std::int8_t b) = 0;
    // 008E62A0 BSP_PowerUp_AppliesToUnit: __thiscall(ECX = entry, unit), RET 4, AL.
    virtual bool powerup_applies_to_unit_008e62a0(void* entry, void* unit) = 0;

    // ---- checked dword vectors {proxy, first, last, end} (STL, called) ----
    // 005A6580 insert one: ECX = vector, (out*, container, where, const dword*), RET 10h.
    virtual void dword_vector_insert_one_005a6580(void* vector, void* out, void* container, void* where,
                                                  const void* value) = 0;
    // 0043DB60 insert one: ECX = vector, (out*, container, where, const dword*), RET 10h.
    virtual void dword_vector_insert_one_0043db60(void* vector, void* out, void* container, void* where,
                                                  const void* value) = 0;

    // ---- options screens (segment 23) ----
    // 005F0220 BSP_OptionsScreen_RowLabelAt: __fastcall(ECX = out native
    // string, EDX = label table, stack index), RET 4, EAX = the label string.
    virtual void* options_row_label_at_005f0220(void* out, const void* labels, std::int32_t index) = 0;
    // 005F1620 BSP_OptionsScreen_ShowValueArrows: __thiscall(ECX = screen, row), RET 4.
    virtual void options_screen_show_value_arrows_005f1620(void* screen, void* row) = 0;
    // 00A9CDF0 BSP_GuiListbox_ClearOtherTwo: __fastcall(ECX = list box, EDX =
    // other, stack third), RET 4.
    virtual void gui_listbox_clear_other_two_00a9cdf0(void* listbox, void* other, void* third) = 0;

    // ---- movie camera (segments 1, 6, 21, 29, 45, 46, 53) ----
    // 0042B2F0 BSP_Vector3_LengthFloatThreshold: __thiscall(ECX = const float[3]), ST0.
    virtual double vector3_length_0042b2f0(const float* v) = 0;
    // 00793730 BSP_Vector3f_ScaleToLength: __fastcall(ECX = out, EDX = v, float length), RET 4, EAX = out.
    virtual float* vector3_scale_to_length_00793730(float* out, const float* v, float length) = 0;
    // 00793F10 BSP_CameraRef_TransformToWorld: __thiscall(ECX = ref, float* out, int frame), RET 8, EAX.
    virtual const float* camera_ref_transform_to_world_00793f10(void* ref, float* out, std::int32_t frame) = 0;
    // 0085DAD0 BSP_Matrix_OrthonormalizeFromRowYThenZ: __fastcall(ECX = float[16]).
    virtual void matrix_orthonormalize_from_row_y_then_z_0085dad0(float* matrix) = 0;
    // 007AD330 BSP_PathPoints_AddPointLocal: __thiscall(ECX = path, const float[3]), RET 4.
    virtual void path_points_add_point_local_007ad330(void* path, const float* point) = 0;
    // 007AF150 BSP_CameraPath_DeriveKnots: __fastcall(ECX = path).
    virtual void camera_path_derive_knots_007af150(void* path) = 0;
    // 007B05E0 (path parameter at a point index): __thiscall(ECX = path, int), RET 4, ST0.
    virtual double path_parameter_at_007b05e0(void* path, std::int32_t index) = 0;
    // 005CB9F0 BSP_MovCam_SetLinearSpeedLimit: __thiscall(ECX = mover, float), RET 4.
    virtual void movcam_set_linear_speed_limit_005cb9f0(void* mover, float value) = 0;
    // 007900B0 BSP_MovCamInterpolator_Init: __thiscall(ECX = interpolator,
    // const native string* name, float, float, float, float), RET 14h, EAX.
    virtual void* movcam_interpolator_init_007900b0(void* interpolator, const void* name, float a, float b, float c,
                                                    float d) = 0;
    // 00796930 STL_List_BuyNode: __thiscall(ECX = list, next, prev, const value*), RET 0Ch, EAX = node.
    virtual void* list_buy_node_00796930(void* list, void* next, void* prev, const void* value) = 0;
    // 0079DE40 (list _Incsize, throws on overflow): __thiscall(ECX = list, count), RET 4.
    virtual void list_incsize_0079de40(void* list, std::uint32_t count) = 0;
    // 0047B660 BSP_Path_Construct: __thiscall(ECX = block, int id), RET 4, EAX.
    virtual void* path_construct_0047b660(void* block, std::int32_t id) = 0;
    // 006B88E0 BSP_LuaVariantVector_Resize: __thiscall(ECX = vector, int size), RET 4.
    virtual void lua_variant_vector_resize_006b88e0(void* vector, std::int32_t size) = 0;
    // 00794360 BSP_MovCam_SeedFromMover: __fastcall(ECX = camera).
    virtual void movcam_seed_from_mover_00794360(void* camera) = 0;

    // ---- weapon-group sight (segments 16, 39, 78) ----
    // 00727D30 BSP_Gun_AnyBarrelReadyWithin: __thiscall(ECX = gun, float), RET 4, AL.
    virtual bool gun_any_barrel_ready_within_00727d30(void* gun, float limit) = 0;
    // 00727C30 BSP_Gun_IndexOfReadiestBarrel: __thiscall(ECX = gun), EAX.
    virtual std::int32_t gun_index_of_readiest_barrel_00727c30(void* gun) = 0;
    // 005459B0 BSP_Gun_IsArtilleryKind: __thiscall(ECX = gun), AL.
    virtual bool gun_is_artillery_kind_005459b0(void* gun) = 0;
    // 005459E0 BSP_Gun_IsTorpedoClassLauncher: __thiscall(ECX = gun), AL.
    virtual bool gun_is_torpedo_class_launcher_005459e0(void* gun) = 0;
    // 00545CB0 / 00545D10 TMPL_PtrArray_Reserve: __thiscall(ECX = {data, size, cap}, int), RET 4.
    virtual void ptr_array_reserve_00545cb0(void* array, std::int32_t capacity) = 0;
    virtual void ptr_array_reserve_00545d10(void* array, std::int32_t capacity) = 0;
    // 005471B0 BSP_RefCountedPointerArray_Resize: __thiscall(ECX = {data, size}, int), RET 4.
    virtual void ref_pointer_array_resize_005471b0(void* array, std::int32_t count) = 0;
    // 00ABE6E0 BSP_GuiWidget_SetQuadFloats: __thiscall(ECX = widget, four floats), RET 10h.
    virtual void gui_widget_set_quad_floats_00abe6e0(void* widget, float a, float b, float c, float d) = 0;
    // 00412E20 BSP_Math_TangentX87Float: __stdcall(float), RET 4, ST0.
    virtual double math_tangent_00412e20(float angle) = 0;

    // ---- HUD map screen destructor (segment 20) ----
    // 005A6350 / 00538360 / 005A5E40 STL tree erase(first, last): ECX = tree,
    // (out*, {container, first}, {container, last}) by value, RET 14h.
    virtual void tree_erase_range_005a6350(void* tree, void* out, void* first_container, void* first,
                                           void* last_container, void* last) = 0;
    virtual void tree_erase_range_00538360(void* tree, void* out, void* first_container, void* first,
                                           void* last_container, void* last) = 0;
    virtual void tree_erase_range_005a5e40(void* tree, void* out, void* first_container, void* first,
                                           void* last_container, void* last) = 0;
    // 0093CC80 CG_adjustor_thunk (the simple 3D scene sub-object destructor): ECX = screen+8.
    virtual void simple_3d_scene_destruct_0093cc80(void* scene) = 0;

    // ---- movie camera mode entry (segments 0..3, 7, 21, 22, 45, 46, 88) ----
    // 00B6FBB0 BSP_Camera_SetFieldOfView / 00B6FBF0 BSP_Camera_SetNearPlane:
    // __thiscall(ECX = camera node, float), RET 4.
    virtual void camera_set_field_of_view_00b6fbb0(void* camera, float value) = 0;
    virtual void camera_set_near_plane_00b6fbf0(void* camera, float value) = 0;
    // 0042B450 / 0042B490 BSP_Matrix_SetRotationX/YNegated: __thiscall(ECX = float[16], float angle), RET 4, EAX = ECX.
    virtual float* matrix_set_rotation_x_negated_0042b450(float* matrix, float angle) = 0;
    virtual float* matrix_set_rotation_y_negated_0042b490(float* matrix, float angle) = 0;
    // 00414260 BSP_Vector3f_ScaleTo: __thiscall(ECX = this, out, float), RET 8, EAX = out.
    virtual float* vector3_scale_to_00414260(const float* self, float* out, float s) = 0;
    // 004142A0 BSP_Vector3f_AddTo: __thiscall(ECX = this, out, const v), RET 8, EAX = out (out = v + this).
    virtual float* vector3_add_to_004142a0(const float* self, float* out, const float* v) = 0;
    // 0042AE20 BSP_Vector3f_DivideScalarTo: __thiscall(ECX = this, out, float), RET 8, EAX = out.
    virtual float* vector3_divide_scalar_to_0042ae20(const float* self, float* out, float s) = 0;
    // 00794130 BSP_CameraPosition_AimAtTargetOffset: __thiscall(ECX = camera, target, float), RET 8.
    virtual void camera_position_aim_at_target_offset_00794130(void* camera, void* target, float scale) = 0;
    // 007904D0 / 00799750 / 00794000 / 007972F0: __fastcall(ECX = camera).
    virtual void movcam_wrap_and_clamp_angles_007904d0(void* camera) = 0;
    virtual void movcam_apply_position_and_angles_00799750(void* camera) = 0;
    virtual void movcam_reset_motion_00794000(void* camera) = 0;
    virtual void camera_position_update_path_007972f0(void* camera) = 0;
    // 00794070 BSP_CameraPosition_AimAtPoint: __thiscall(ECX = camera, float3 by value), RET 0Ch.
    virtual void camera_position_aim_at_point_00794070(void* camera, float x, float y, float z) = 0;
    // 00445790 BSP_WeatherConfig_ApplyCameraShake: __thiscall(ECX = [[00E188A8]+21C4h], camera), RET 4.
    virtual void weather_config_apply_camera_shake_00445790(void* weather, void* camera) = 0;
    // 00793920 BSP_CameraRef_ConstructPoint: __thiscall(ECX = ref, entity, const float* point), RET 8, EAX = ref.
    virtual void* camera_ref_construct_point_00793920(void* ref, void* entity, const float* point) = 0;
    // 00790320 BSP_CameraRef_Assign: __thiscall(ECX = dst, const src), RET 4.
    virtual void* camera_ref_assign_00790320(void* dst, const void* src) = 0;
    // 00793AC0 BSP_CameraRef_Resolve: __thiscall(ECX = ref, float* out, char level), RET 8, EAX.
    virtual const float* camera_ref_resolve_00793ac0(void* ref, float* out, std::int8_t level) = 0;
    // 007B04C0 BSP_CameraPath_SampleWorld: __thiscall(ECX = path, float, const float*, float* out, int), RET 10h.
    virtual void camera_path_sample_world_007b04c0(void* path, float t, const float* origin, float* out,
                                                   std::int32_t flag) = 0;
    // 005CD820 BSP_VehicleCameraArray_At: __thiscall(ECX = array, int), RET 4, EAX = element.
    virtual void* vehicle_camera_array_at_005cd820(void* array, std::int32_t index) = 0;
    // 0047E540 BSP_Transform_InitTranslation: __fastcall(ECX = float[16] out, EDX = const float[3]), EAX = out.
    virtual float* transform_init_translation_0047e540(float* out, const float* translation) = 0;
    // 005CC5E0 BSP_MovCam_ShowModeText: __thiscall(ECX = mover, native string
    // by value (8 bytes, owned and released by the callee)), RET 8.
    virtual void movcam_show_mode_text_005cc5e0(void* mover, Cc13SweepBNativeString text) = 0;
    // 007A0AC0 / 007A4E10 STL map subscripts: __thiscall(ECX = map, const key*), RET 4, EAX = value address.
    virtual std::int32_t* map_subscript_007a0ac0(void* map, const void* key) = 0;
    virtual void* map_subscript_007a4e10(void* map, const void* key) = 0;
    // 00418170 (checked 8-byte vector element): __thiscall(ECX = vector, index), RET 4, EAX.
    virtual float* vector8_at_00418170(void* vector, std::uint32_t index) = 0;
    // 0042D7E0 BSP_EntityPose_GetWorldMatrix: __fastcall(ECX = entity), EAX = const float[16].
    virtual const float* entity_get_world_matrix_0042d7e0(void* entity) = 0;
    // Camera +170h sub-object vtable +0Ch, no stack arguments.
    virtual void movcam_controller_vcall_0c(void* controller) = 0;
};

// ---------------------------------------------------------------------------
// Segment 16
// ---------------------------------------------------------------------------

// 00547A20 (weapon-group sight for one weapon kind): __thiscall(ECX = screen,
// int kind), RET 4. Lays out one sight marker pair (+0E4h / +0F0h arrays) per
// gun of the kind across the screen's groups, sets each pair's state from the
// gun's barrels or 00547480, and arranges the pairs on two arcs.
void cc13_sweepB6_weapon_group_sight_00547a20(void* screen, std::int32_t kind, Cc13SweepB6Host& host);

// ---------------------------------------------------------------------------
// Segment 20
// ---------------------------------------------------------------------------

// 005A7C20 BSP_HudMapIcon_RefreshPowerUpIcons: __thiscall(ECX = icon,
// template widget, char second), RET 8, EAX = the count. Rate-gated to once a
// second per list; releases the icon's power-up widget list (+88h when
// `second`, else +0C8h), collects the active power-ups that apply to the icon's
// unit (+4h) and, with a template, clones one widget per power-up into the list.
std::int32_t cc13_sweepB6_hud_map_icon_refresh_power_up_icons_005a7c20(void* icon, void* template_widget,
                                                                     bool second, Cc13SweepB6Host& host);

// 005A8600 BSP_HudMapScreen_Destruct: __thiscall(ECX = screen), RET. The
// GUI_map destructor body (called by 005AB140): restores the terrain bush /
// tree groups, removes the two GUI pages, releases the ref-pointer members,
// tears down the member containers and runs the base destructors.
void cc13_sweepB6_hud_map_screen_destruct_005a8600(void* screen, Cc13SweepB6Host& host);

// ---------------------------------------------------------------------------
// Segment 23
// ---------------------------------------------------------------------------

// 005F24D0 BSP_OptionsScreen_BuildSettingsPage: __thiscall(ECX = screen,
// const native string* title, const char** labels, int label_count, int
// value_rows, const int* row_kinds, char list_pages), RET 18h. Rebuilds the
// value rows (list box +88h) and, with `list_pages`, the page rows (+84h) and
// the command bar.
void cc13_sweepB6_options_screen_build_settings_page_005f24d0(void* screen, const void* title, const void* labels,
                                                              std::int32_t label_count, std::int32_t value_rows,
                                                              const std::int32_t* row_kinds, bool list_pages,
                                                              Cc13SweepB6Host& host);

// ---------------------------------------------------------------------------
// Segment 27
// ---------------------------------------------------------------------------

// 0065D220 BSP_HudSupportManager_RebindSlots: __thiscall(ECX = screen), RET.
// Clones the four slot pairs from the template +138h, binds the slot widget
// arrays (+7Ch, +8Ch, +9Ch, +0ACh, +0BCh, +0CCh, +0DCh, +108h, +118h) to
// their named children, hides the parts and offsets the slots by [+2A8h].
void cc13_sweepB6_hud_support_rebind_slots_0065d220(void* screen, Cc13SweepB6Host& host);

// ---------------------------------------------------------------------------
// Segment 46
// ---------------------------------------------------------------------------

// 007A5110 BSP_MovCam_EnterMode: __thiscall(ECX = camera, char snap), RET 4.
// Resets the motion state, then sets the camera up for mode [+384h] (0 free,
// 1 follow, 2 vehicle camera, 3/4/6 orbit variants, 5 path walk).
void cc13_sweepB6_movcam_enter_mode_007a5110(void* camera, std::uint8_t snap, Cc13SweepB6Host& host);

// 007A6200 (not a Ghidra function; inside 007A5A00's Ghidra body):
// __thiscall(ECX = camera, int index), RET 4. index -1 selects mode 1, any
// other index mode 2 with that vehicle camera; 007A5110(camera, 0) only on a
// change; then +170h vtable +0Ch.
void cc13_sweepB6_movcam_select_deck_camera_007a6200(void* camera, std::int32_t index, Cc13SweepB6Host& host);

// 007A5A00 BSP_MovCam_BeginPathWalk: __thiscall(ECX = camera, float start,
// char keep_points, float duration, const native string* path_name, void*
// variants), RET 14h. Takes the path name and Lua variant vector, sets mode 5,
// (unless keep_points) seeds the first two path points from the camera, builds
// a path object from the point list, and sets the walk speed (or queues a
// "moveSpeed" interpolator) before 007A5110.
void cc13_sweepB6_movcam_begin_path_walk_007a5a00(void* camera, float start, std::uint8_t keep_points,
                                                  float duration, const void* path_name, void* variants,
                                                  Cc13SweepB6Host& host);

}  // namespace bsp
