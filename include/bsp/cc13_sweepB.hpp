#pragma once

// cc13_sweepB: leftover sweep over segments 15-51 (packet cc13-sweepB,
// docs/CC13_SWEEPB.md). Every name is a hypothesis, not a recovered symbol.
// These are new C++ interfaces over the native records, not drop-in
// replacements of the original ABI. Nothing here is wired into the game.
//
// Native objects are addressed as raw byte blocks; only the offsets each
// function reads or writes are touched. Every call the image makes goes
// through a named Cc13SweepBHost method at the same point with the same
// arguments.

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {

// Typed access to a word of a native block.
template <class T>
inline T& cc13_sweepB_at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

// A float from its image bit pattern (exact constants).
inline float cc13_sweepB_f32(std::uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

class Cc13SweepBHost {
public:
    virtual ~Cc13SweepBHost() = default;

    // ---- GUI widgets and manager ----
    // Widget vtable slot +34h with one int argument (set visible; 1 = show, 0 = hide).
    virtual void widget_vcall_34_set_visible(void* widget, std::int32_t show) = 0;
    // 00AA7D00 BSP_GuiWidget_SetLocalXYAndBounds, __thiscall(widget, x, y), RET 8.
    virtual void widget_set_local_xy_00aa7d00(void* widget, float x, float y) = 0;
    // Widget vtable slot +28h, no stack arguments: returns the widget's name
    // (a NativeString*), as 00AA7E00 consumes it.
    virtual void* widget_vcall_28_name(void* widget) = 0;
    // 00AA7E00 BSP_GuiWidget_FindChildByName, __thiscall(widget, name, int), RET 8.
    virtual void* widget_find_child_00aa7e00(void* widget, void* name, std::int32_t unused) = 0;
    // 00AAB4C0 BSP_GuiWidget_CloneSubtree, __thiscall(widget, parent_clone), RET 4.
    virtual void* widget_clone_subtree_00aab4c0(void* widget, void* parent_clone) = 0;
    // 004C12B0 BSP_GuiManager_GetOrCreate, returns the manager.
    virtual void* gui_manager_get_004c12b0() = 0;
    // 00AA1FA0 BSP_GuiManager_SetCursorPosition, __thiscall(manager, x, y), RET 8.
    virtual void gui_manager_set_cursor_00aa1fa0(void* manager, float x, float y) = 0;
    // 00A9C7C0 BSP_GuiListbox_SelectIndex, __thiscall(listbox, index), RET 4.
    virtual void listbox_select_index_00a9c7c0(void* listbox, std::int32_t index) = 0;

    // ---- entities and unit groups ----
    // 00414DB0 BSP_EntityPose_RefreshWorld, ECX = entity.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 0070D070 BSP_UnitGroup_MemberRecordAt, __thiscall(group, index), RET 4.
    virtual void* unit_group_member_record_at_0070d070(void* group, std::int32_t index) = 0;
    // 0070D030 BSP_UnitGroup_IndexOfMember, __thiscall(group, unit), RET 4.
    virtual std::int32_t unit_group_index_of_member_0070d030(void* group, void* unit) = 0;

    // ---- rendering ----
    // Object vtable slot +30h taking a pointer to three floats (set position).
    virtual void object_vcall_30_vec3(void* object, const float* xyz) = 0;
    // 00B63F10 BSP_Matrix_BuildLookAt: ECX dest, EDX eye, stack target pointer
    // then up.x/up.y/up.z; RET 10h; returns dest.
    virtual float* matrix_build_look_at_00b63f10(float* dest, const float* eye, const float* target,
                                                 float up_x, float up_y, float up_z) = 0;
    // 00B71490 BSP_Camera_SetViewMatrix, __thiscall(camera, matrix*), RET 4.
    virtual void camera_set_view_matrix_00b71490(void* camera, const float* matrix) = 0;
    // 00AA2020 BSP_Camera_SetGuiOrthoProjection, __thiscall(camera, l, r, b, t, n, f), RET 18h.
    virtual void camera_set_gui_ortho_00aa2020(void* camera, float left, float right, float bottom,
                                               float top, float near_plane, float far_plane) = 0;
    // 0093D250 BSP_Simple3dScene_SubmitRender, ECX = scene.
    virtual void simple3d_scene_submit_0093d250(void* scene) = 0;

    // ---- weapon-group HUD (screen 2Eh) ----
    // 005464E0 BSP_HudScreen2E_ClearTimedEntries, ECX = screen.
    virtual void screen2e_clear_timed_entries_005464e0(void* screen) = 0;
    // 00547A20 (sight for one weapon kind), __thiscall(screen, kind), RET 4.
    virtual void weapon_group_sight_00547a20(void* screen, std::int32_t kind) = 0;
    // 00549260 BSP_HudWeaponGroupScreen_BindUnit, __thiscall(screen, unit, bound), RET 8.
    virtual void weapon_group_bind_unit_00549260(void* screen, void* unit, void* bound) = 0;
    // 00545600 BSP_HudWeaponGroupEntryB_Destruct, ECX = entry.
    virtual void weapon_group_entry_destruct_00545600(void* entry) = 0;

    // ---- renderer ----
    // Read of the renderer global 00F8D394.
    virtual void* renderer_00f8d394() = 0;
    // Renderer vtable +C0h(const float* p0, const float* p1, argb0, argb1).
    virtual void renderer_vcall_c0_line(void* renderer, const float* p0, const float* p1,
                                        std::uint32_t argb0, std::uint32_t argb1) = 0;
    // 004FB850 BSP_Color_ConvertFloatRgbaToArgb, __thiscall(dest, rgba*), RET 4; returns dest.
    virtual std::uint32_t* color_float_rgba_to_argb_004fb850(std::uint32_t* dest, const float* rgba) = 0;

    // ---- session browser (screen) ----
    // 0056D660 BSP_MultiSessionBrowserScreen_FillSessionSettings, ECX = screen.
    virtual void session_browser_fill_settings_0056d660(void* screen) = 0;
    // 005789C0 BSP_MultiSessionBrowserScreen_BuildRows, __thiscall(screen, key), RET 4.
    virtual void session_browser_build_rows_005789c0(void* screen, std::uint32_t key) = 0;
    // 0056DD80 BSP_MultiSessionBrowserScreen_RefreshFilterRows, ECX = screen.
    virtual void session_browser_refresh_filter_rows_0056dd80(void* screen) = 0;
    // 007FBA30 BSP_ByteListRecord_Assign, __thiscall(rec, a, b, c, bytes*, n), RET 14h.
    virtual void byte_list_record_assign_007fba30(void* record, std::uint32_t a, std::uint32_t b,
                                                  std::uint32_t c, const std::uint8_t* bytes,
                                                  std::int32_t count) = 0;

    // 0056F160 std::wstring::assign(const wstring&, pos, count) instantiation,
    // __thiscall(dest, source, pos, count), RET 0Ch; returns dest.
    virtual void* wstring_assign_0056f160(void* dest, const void* source, std::uint32_t pos,
                                          std::uint32_t count) = 0;

    // ---- GUI movie widgets and native string building ----
    // 00AAC870 BSP_GuiMovie_Stop, ECX = widget.
    virtual void gui_movie_stop_00aac870(void* widget) = 0;
    // 00AAE320 BSP_GuiMovie_SetSource, __thiscall(widget, const NativeString*, byte loop), RET 8.
    virtual void gui_movie_set_source_00aae320(void* widget, const void* source, std::uint8_t loop) = 0;
    // 00AB0EB0 BSP_GuiMovie_Start, ECX = widget.
    virtual void gui_movie_start_00ab0eb0(void* widget) = 0;
    // 004260B0 BSP_NativeString_FromInt, __thiscall(out, int), RET 4; returns out.
    virtual void* native_string_from_int_004260b0(void* out, std::int32_t value) = 0;
    // 0043C130 BSP_NativeString_ConstructPrefixedHeader: ECX out, EDX C prefix,
    // stack const NativeString* suffix; RET 4; returns out.
    virtual void* native_string_prefixed_0043c130(void* out, const char* prefix, const void* suffix) = 0;
    // 004261A0 BSP_NativeString_Concat, __thiscall(lhs, out, rhs), RET 8; returns out.
    virtual void* native_string_concat_004261a0(const void* lhs, void* out, const void* rhs) = 0;

    // Factory vtable slot +8h(int kind, float scale): creates a node.
    virtual void* factory_vcall_08_create(void* factory, std::int32_t kind, float scale) = 0;
    // 00B6D890 BSP_Node_PropagateRootRegistration, __thiscall(node, root), RET 4.
    virtual void node_propagate_root_00b6d890(void* node, void* root) = 0;
    // 00B74310 BSP_GeneratedModel_GetCurrentType (no arguments; ECX unused).
    virtual void* generated_model_current_type_00b74310() = 0;
    // Node vtable slot +0Ch(type): the IsKindOf test, AL.
    virtual bool node_vcall_0c_is_kind_of(void* node, void* type) = 0;
    // 005A07B0 BSP_GeneratedModel_FindMaterialByEffectName: ECX model, EDX const NativeString*.
    virtual void* generated_model_find_material_005a07b0(void* model, const void* name) = 0;
    // 00B18B20 BSP_Material_RegisterFloatParameter, __thiscall(material, const NativeString*, source), RET 8.
    virtual void material_register_float_parameter_00b18b20(void* material, const void* name, void* source) = 0;
    // 0041E870 BSP_NativeString_Assign, __thiscall(string, const char*), RET 4; returns string.
    virtual void* native_string_assign_0041e870(void* string, const char* text) = 0;

    // 00AB64C0 BSP_GuiIcon_AddState, __thiscall(icon, texture, const float uv[4]), RET 8.
    virtual void gui_icon_add_state_00ab64c0(void* icon, void* texture, const void* uv) = 0;
    // Widget vtable slot +88h(int, int, float).
    virtual void widget_vcall_88(void* widget, std::int32_t a, std::int32_t b, float c) = 0;
    // 0043DCA0 std::vector<T*>::push_back instantiation, __thiscall(vector, const T**), RET 4.
    virtual void pointer_vector_push_back_0043dca0(void* vector, void* const* value) = 0;

    // 008ED9C0 BSP_PowerUpManager_LocalPlayerHasUsableItem, ECX = [00F88C30], AL.
    virtual bool powerup_manager_local_has_usable_008ed9c0(void* manager) = 0;
    // 00AA6750 BSP_GuiWidget_GetResolvedPosition, __thiscall(widget, float out[3]), RET 4; returns out.
    virtual float* widget_get_resolved_position_00aa6750(void* widget, float* out) = 0;
    // 00AA8240 BSP_GuiWidget_SetResolvedPosition, __thiscall(widget, const float*), RET 4.
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float* position) = 0;
    // 0066FFA0 BSP_HudSupportManagerScreen_HasAvailableEntries, ECX = screen, AL.
    virtual bool support_manager_has_available_0066ffa0(void* screen) = 0;
    // Reads of the globals 00E198C4 (in-game GUI) and 00F88C30 (power-up manager).
    virtual void* global_00e198c4() = 0;
    virtual void* global_00f88c30() = 0;

    // ---- HUD map ----
    // 005A1220 BSP_HudMapScreen_FindIconCarryingUnit, __thiscall(screen, unit), RET 4.
    virtual void* hud_map_find_icon_carrying_unit_005a1220(void* screen, void* unit) = 0;
    // 005A21E0 BSP_HudMapIcon_StartBlink, ECX = icon.
    virtual void hud_map_icon_start_blink_005a21e0(void* icon) = 0;
    // 005A22D0 BSP_HudMapScreen_FindUnitIcon, __thiscall(screen, unit), RET 4.
    virtual void* hud_map_find_unit_icon_005a22d0(void* screen, void* unit) = 0;

    // ---- scene nodes and native strings ----
    // 00B6D800 BSP_Node_GetNameHeader: ECX node, returns node+54h (borrowed).
    virtual void* node_get_name_header_00b6d800(void* node) = 0;
    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor, __thiscall(node, float, char recurse), RET 8.
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float value, std::int8_t recurse) = 0;
    // 00469840 BSP_NativeString_Substring, __thiscall(source, dest*, start, count), RET 0Ch.
    virtual void* native_string_substring_00469840(void* source, void* dest, std::int32_t start,
                                                   std::int32_t count) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton.
    virtual void* sized_pool_get_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(pool, data, bytes, int), RET 0Ch.
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                  std::int32_t flag) = 0;

    // ---- image globals (stores and loads go through these references) ----
    // 00E188D8: the selected (controlled) unit.
    virtual void* global_00e188d8() = 0;
    // 00F876A4: the game clock (float seconds).
    virtual float global_00f876a4() = 0;
    // 00E188A8: the game/profile root pointer.
    virtual void* global_00e188a8() = 0;
    // 00F8894C: the singleton pointer whose base destructor clears it.
    virtual std::uint32_t& global_00f8894c() = 0;

    // Entity vtable slot +5Ch(kind): the kind test, AL result.
    virtual bool entity_vcall_5c_is_kind(void* entity, std::int32_t kind) = 0;
    // Object vtable slot +0h(flags): a scalar deleting destructor in slot 0.
    virtual void object_vcall_00_delete(void* object, std::uint32_t flags) = 0;
    // Object vtable slot +4h(flags): the scalar deleting destructor.
    virtual void object_vcall_04_delete(void* object, std::uint32_t flags) = 0;

    // 004D92B0 BSP_FrontEnd_ActionEdgeOrRepeat, __thiscall(game, int action), RET 4, AL.
    virtual std::uint8_t frontend_action_edge_or_repeat_004d92b0(void* game, std::int32_t action) = 0;
    // ---- ocean ----
    // Read of the global 00F889F0.
    virtual void* global_00f889f0() = 0;
    // 00B9A330 TRIV_body_00b9a330, ECX = [00F889F0]; result unused.
    virtual void trivial_00b9a330(void* object) = 0;
    // 00BA6FB0 BSP_OceanWaveField_Create, __fastcall(ECX source, DL flag), returns the field.
    virtual void* ocean_wave_field_create_00ba6fb0(void* source, std::uint8_t flag) = 0;
    // 00B97100 BSP_OceanSpectrum_Construct, ECX = 90h block, returns it.
    virtual void* ocean_spectrum_construct_00b97100(void* block) = 0;
    // 00B9DD90 BSP_OceanWaveField_SetSpectrum, __thiscall(field, spectrum), RET 4.
    virtual void ocean_wave_field_set_spectrum_00b9dd90(void* field, void* spectrum) = 0;
    // 00B84E50 BSP_SystemFogOwner_Construct, ECX = 94h block, returns it.
    virtual void* system_fog_owner_construct_00b84e50(void* block) = 0;

    // ---- movie camera ----
    // 007AD070 (path point lookup), __thiscall(path owner, index), RET 4, returns the record.
    virtual void* movcam_path_point_007ad070(void* owner, std::int32_t index) = 0;
    // 00794910 (aim delta to target), __thiscall(camera, out*, target, float), RET 0Ch, EAX out.
    virtual void* movcam_aim_delta_00794910(void* camera, float* out, void* target, float value) = 0;
    // Keyframe modifier vtable slots: +4h(keyframe) attach, +30h() kind.
    virtual void keyframe_modifier_vcall_04_attach(void* modifier, void* keyframe) = 0;
    virtual std::int32_t keyframe_modifier_vcall_30_kind(void* modifier) = 0;
    // Keyframe modifier vtable slot +10h(float).
    virtual void keyframe_modifier_vcall_10(void* modifier, float value) = 0;
    // 00797E40 MovieCameraTrack_ForwardToKeyModifiers, __thiscall(track, float), RET 4.
    virtual void movie_track_forward_modifiers_00797e40(void* track, float value) = 0;

    // 0071ABB0 BSP_ResourceItemAux_Construct, ECX = 54h-byte block, returns it.
    virtual void* resource_item_aux_construct_0071abb0(void* block) = 0;
    // 0071B3E0 BSP_GameResource_ReadPointsItem, __thiscall(item, source), RET 4.
    virtual void resource_read_points_item_0071b3e0(void* item, void* source) = 0;
    // 0041DD40 BSP_NativeString_Resize, __thiscall(string, length, char preserve), RET 8.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint8_t preserve) = 0;
    // 00419440 BSP_Vector3f_Length, ECX = vec3, ST0 result.
    virtual float vector3_length_00419440(const float* xyz) = 0;
    // 00491E70 (unnamed), __thiscall(manager [[00E188A8]+21D0h], entry, int), RET 8.
    virtual void manager21d0_call_00491e70(void* manager, void* entry, std::int32_t arg) = 0;
    // 0048CB30 (unnamed), __thiscall(manager [[00E188A8]+21D0h], entry), RET 4.
    virtual void manager21d0_call_0048cb30(void* manager, void* entry) = 0;
    // 004F60E0 std::vector<T*>::operator= instantiation, __thiscall(dest, source*), RET 4.
    virtual void pointer_vector_assign_004f60e0(void* dest, const void* source) = 0;
    // 00618980 BSP_HudPowerupCarousel_RefreshSelectedLabel, ECX = carousel.
    virtual void powerup_carousel_refresh_label_00618980(void* carousel) = 0;
    // 007BCA50 BSP_Plane_OrdnanceIconKind, ECX = plane, returns EAX.
    virtual std::int32_t plane_ordnance_icon_kind_007bca50(void* plane) = 0;
    // 007B9140 BSP_Unit_CanDropOrdnance, __thiscall(unit, int), RET 4, AL.
    virtual bool unit_can_drop_ordnance_007b9140(void* unit, std::int32_t arg) = 0;
    // 0042E740 BSP_GameTuning_GetSingleton (lazily allocates the 6D0h block).
    virtual void* game_tuning_get_0042e740() = 0;
    // 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x), stdcall RET 14h, ST0.
    virtual float math_interpolate_clamped_00419010(float x0, float y0, float x1, float y1, float x) = 0;

    // 00424C40 BSP_GameSettings_GetSingleton (cdecl, lazy).
    virtual void* game_settings_get_00424c40() = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint, __thiscall(ECX point)(out*, matrix*), RET 8.
    virtual void vector3_transform_affine_point_004142e0(const float* point, float* out, const float* matrix) = 0;
    // 00B63D50 BSP_Matrix_BuildOrthogonalScaledAffineInverse, ECX dest, EDX source.
    virtual void matrix_build_affine_inverse_00b63d50(float* dest, const float* source) = 0;
    // The function-static NativeString of 0081C0A0: guard word 00F87534 (bit 0),
    // the string object 00F8752C {length, data at 00F87530}, the empty literal
    // 00F87528, and the CRT atexit 00BF6FF5 (cdecl) taking the image address of
    // the destructor thunk.
    virtual std::uint32_t& global_00f87534() = 0;
    virtual void* global_object_00f8752c() = 0;
    virtual const char* empty_string_00f87528() = 0;
    virtual std::int32_t crt_atexit_00bf6ff5(std::uint32_t function_address) = 0;
    // IAT 00CE221C InterlockedIncrement (stdcall), returns the new value.
    virtual std::int32_t interlocked_increment_00ce221c(volatile std::int32_t* value) = 0;
    // 00815370 BSP_EffectGroup_SetScalar, __thiscall(group, float), RET 4.
    virtual void effect_group_set_scalar_00815370(void* group, float value) = 0;
    // 0074D780 BSP_LiveEffectReferences_Append, __thiscall(list, const refptr*), RET 4.
    virtual void live_effect_references_append_0074d780(void* list, void* const* reference) = 0;
    // IAT 00CE2220 InterlockedDecrement (stdcall), returns the new value.
    virtual std::int32_t interlocked_decrement_00ce2220(volatile std::int32_t* value) = 0;
    // Ref-counted object vtable slot +0h, no stack arguments (destroy).
    virtual void refcounted_vcall_00_destroy(void* object) = 0;
    // 00818030 BSP_FloatArray_Resize, __thiscall(array, int count), RET 4.
    virtual void float_array_resize_00818030(void* array, std::int32_t count) = 0;

    // ---- CRT ----
    // 00BF681B operator new (cdecl).
    virtual void* crt_operator_new_00bf681b(std::uint32_t size) = 0;
    // 00BF55BE operator new thunk (cdecl), used by the counted arrays.
    virtual void* crt_operator_new_00bf55be(std::uint32_t size) = 0;
    // 00BF7680 memcpy (cdecl).
    virtual void* crt_memcpy_00bf7680(void* dest, const void* source, std::uint32_t size) = 0;
    // 00BF6989 free thunk (cdecl).
    virtual void crt_free_00bf6989(void* block) = 0;
    // 00BF67A7 memmove_s (cdecl), returns errno_t.
    virtual std::int32_t crt_memmove_s_00bf67a7(void* dest, std::uint32_t dest_size, const void* source,
                                                std::uint32_t count) = 0;
    // 00BF9440 strstr (cdecl).
    virtual const char* crt_strstr_00bf9440(const char* text, const char* pattern) = 0;
    // 00BF7FBF _stricmp (cdecl).
    virtual std::int32_t crt_stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 00BF65AC operator delete (cdecl).
    virtual void crt_operator_delete_00bf65ac(void* block) = 0;
    // 00BF7030 _CIsqrt: ST0 in, ST0 out.
    virtual double crt_sqrt_00bf7030(double value) = 0;
};

// ---------------------------------------------------------------------------
// Segment 15
// ---------------------------------------------------------------------------

// Widget-holder helpers 005411A0..00541210 (no callers, no vtable slot).
void cc13_sweepB_holder18_show_005411a0(void* holder, Cc13SweepBHost& host);
void cc13_sweepB_holder08_show_005411b0(void* holder, Cc13SweepBHost& host);
void cc13_sweepB_holder18_set_pixel_xy_005411c0(void* holder, std::int32_t x, std::int32_t y,
                                                Cc13SweepBHost& host);
void cc13_sweepB_holder_set_field04_00541200(void* holder, std::uint32_t value);
void cc13_sweepB_holder08_set_pixel_xy_00541210(void* holder, std::int32_t x, std::int32_t y,
                                                Cc13SweepBHost& host);

// 00535F50: formation screen drag press; picks the nearest group member's map
// icon to the cursor within 0.03 and selects it. Returns 1 when one was picked.
bool cc13_sweepB_formation_drag_pick_00535f50(void* screen, Cc13SweepBHost& host);

// 0052C640 BSP_HudCounterEntry_ConstructClone: a 0ACh-byte counter entry built
// by cloning the template entry's widget subtree. Returns entry.
void* cc13_sweepB_counter_entry_construct_clone_0052c640(void* entry, const void* source,
                                                        Cc13SweepBHost& host);

// 00535430 BSP_InterfaceView_DrawTargetOrtho: the level-3 top-down view.
void cc13_sweepB_interface_view_draw_target_ortho_00535430(void* view, Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 16
// ---------------------------------------------------------------------------

// 005457C0: 1 when party is neither the entity's own (+54h) nor neutral (2).
bool cc13_sweepB_entity_is_hostile_to_party_005457c0(const void* entity, std::int32_t party);

// 00548300: the weapon-group sight dispatch on the selected kind (+44h).
void cc13_sweepB_weapon_group_update_sight_00548300(void* screen, Cc13SweepBHost& host);

// 00549430: rebind the screen to the unit of its first group entry.
void cc13_sweepB_weapon_group_rebind_first_unit_00549430(void* screen, Cc13SweepBHost& host);

// 00554600 BSP_GuiRect_DrawOutline: one renderer +C0h line from a to b, both
// offset by -0.5, in the colour converted from rgba.
void cc13_sweepB_gui_rect_draw_outline_00554600(const float* a, const float* b, const float* rgba,
                                                Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 18
// ---------------------------------------------------------------------------

// 00568CE0 BSP_SessionSearchResult_CopyConstruct: the 88h-byte search result
// (vtable 00CEED04), every word after the vtable copied. Returns object.
void* cc13_sweepB_session_search_result_copy_00568ce0(void* object, const void* source);

// 00568EB0 BSP_Singleton00F8894C_ClearBase: clears 00F8894C, base vtable.
void cc13_sweepB_singleton_f8894c_clear_base_00568eb0(void* object, Cc13SweepBHost& host);

// 005692E0: the scalar deleting destructor over 00568EB0's body.
void* cc13_sweepB_singleton_f8894c_scalar_delete_005692e0(void* object, std::uint32_t flags,
                                                         Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 19
// ---------------------------------------------------------------------------

// 0056E360: copy the applied session filter (+244h..+24Ch, bytes +10h..+18h)
// into the edit copy (+250h..+258h, +19h..+21h), refill, select row 0.
void cc13_sweepB_session_browser_begin_filter_edit_0056e360(void* screen, Cc13SweepBHost& host);

// 0057A910: the reverse copy, rebuild rows, refresh, and store the filter in
// the profile record [00E188A8]+650h.
void cc13_sweepB_session_browser_apply_filter_edit_0057a910(void* screen, Cc13SweepBHost& host);

// 00571EA0: copy-construct a 0A0h-byte session record into raw storage dest
// from source, with a freshly allocated 88h-byte search result at +0h.
void cc13_sweepB_session_record_copy_construct_00571ea0(void* dest, const void* source,
                                                       Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 20
// ---------------------------------------------------------------------------

// 005845D0 BSP_MainMenuScreen_AdvanceBackgroundMovie(screen): next of three
// background movies "movies\FE_bg_0<n>.bik" on the movie widget +340h.
void cc13_sweepB_main_menu_advance_background_movie_005845d0(void* screen, Cc13SweepBHost& host);

// 005A1B40 BSP_HudMap_CreateAlphaBlendModel(factory, parent, out node, out
// sub, out model, out material, alpha source).
void cc13_sweepB_hud_map_create_alpha_blend_model_005a1b40(void* factory, void* parent, void** out_node,
                                                           void** out_sub, void** out_model, void** out_material,
                                                           void* alpha_source, Cc13SweepBHost& host);

// 005A48C0 BSP_HudMapScreen_ResolveIconUnit(screen, unit_slot, prev_slot,
// flag): maps the unit to the one shown on the map, highlights its icon and
// blinks the previous one; *prev_slot takes the resolved unit.
void cc13_sweepB_hud_map_resolve_icon_unit_005a48c0(void* screen, void** unit_slot, void** prev_slot, bool flag,
                                                     Cc13SweepBHost& host);

// 005A8420 BSP_HudMapIcon_RefreshUnitBadge(icon, template, unused): one badge
// for a kind 1Ch unit with a party table (+760h); returns the badge count +38h.
std::uint32_t cc13_sweepB_hud_map_icon_refresh_unit_badge_005a8420(void* icon, void* badge_template,
                                                                   std::uint32_t unused, Cc13SweepBHost& host);

// 0059DA90 BSP_HudMapLayer_Destruct: deletes the owned widget +4h and the
// eight widgets +8h..+24h through their vtable +4h(1), clearing each slot.
void cc13_sweepB_hud_map_layer_destruct_0059da90(void* layer, Cc13SweepBHost& host);

// 0059E120 BSP_Game_ClampPointToWorldBoundsXZ: clamps point[0] and point[1]
// to the world rectangle of [00E188A8] widened by 500. The two further stack
// arguments are not read.
void cc13_sweepB_clamp_point_to_world_bounds_0059e120(float* point, std::uint32_t unused1,
                                                      std::uint32_t unused2, Cc13SweepBHost& host);

// The counted pointer array 0059E690/0059FAD0 manage: the slot holds a
// header block {data (16-aligned inside the block at +18h), size, capacity};
// the pointers follow from data.
struct Cc13SweepBCountedArrayHeader {
    void** data;           // +00h
    std::int32_t size;     // +04h
    std::int32_t capacity; // +08h
};

// 0059E690 BSP_HudMapIconArray_Reserve(slot, n).
void cc13_sweepB_counted_array_reserve_0059e690(Cc13SweepBCountedArrayHeader** slot, std::int32_t count,
                                                Cc13SweepBHost& host);

// 0059FAD0 TMPL_PointerArray_PushBack_0059fad0(slot, value*).
void cc13_sweepB_counted_array_push_back_0059fad0(Cc13SweepBCountedArrayHeader** slot, void* const* value,
                                                  Cc13SweepBHost& host);

// 0059F460: the entity when its vtable +5Ch(2) says so, else null.
void* cc13_sweepB_entity_cast_if_kind02_0059f460(void* entity, Cc13SweepBHost& host);

// The 8-byte native string passed and returned by value: length, then data
// (released to the sized pool with length + 1 bytes).
struct Cc13SweepBNativeString {
    std::uint32_t length;  // +0h
    char* data;            // +4h
};
static_assert(sizeof(void*) != 4 || sizeof(Cc13SweepBNativeString) == 8);

// 005A3EF0: for each direct child of node whose name, after the first ':'
// (whole name when there is none), equals name case-insensitively, set its
// visibility factor to value (no recursion). Consumes name.
void cc13_sweepB_node_set_child_factor_by_name_005a3ef0(void* node, Cc13SweepBNativeString name, float value,
                                                        Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 23
// ---------------------------------------------------------------------------

// 00604A20: 0 when the screen mode +900h is 7, 6 or 3, or 4 with byte +904h
// clear; otherwise 1.
bool cc13_sweepB_interface_allows_input_00604a20(const void* screen);

// 00604A50: 1 when the entity is kind 17h (vtable +5Ch) and byte +C24h is clear.
bool cc13_sweepB_entity_kind17_ready_00604a50(void* entity, Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 24
// ---------------------------------------------------------------------------

// 006057E0 BSP_ResponseCurve_Evaluate(curve[3], x): sign(x) * t blended with
// t*t by curve[2], t = 00419010(curve[0], 0, curve[1], 1, |x|).
float cc13_sweepB_response_curve_evaluate_006057e0(const float* curve, float x, Cc13SweepBHost& host);

// 006062B0: the 14h-byte analog-axis filter {byte active +0h, phase +4h,
// value +8h, last input +0Ch, output +10h} stepped with (rate, input, dt);
// returns the output.
float cc13_sweepB_axis_filter_step_006062b0(void* filter, float rate, float input, float dt,
                                            Cc13SweepBHost& host);

// 0060C5B0: 1 when the float at +198h equals 0.0f (UCOMISS, NaN gives 0).
bool cc13_sweepB_field198_is_zero_0060c5b0(const void* object);

// 0060CB40 BSP_Unit_AsKind9OrNull: the unit when vtable +5Ch(9), else null.
void* cc13_sweepB_unit_as_kind9_0060cb40(void* unit, Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 25
// ---------------------------------------------------------------------------

// The 10h-byte MSVC vector passed by value: allocator proxy word, then
// first/last/end.
struct Cc13SweepBVectorByValue {
    std::uint32_t proxy;  // +0h
    void* first;          // +4h
    void* last;           // +8h
    void* end;            // +0Ch
};
static_assert(sizeof(void*) != 4 || sizeof(Cc13SweepBVectorByValue) == 0x10);

// 00612DA0 BSP_HudPowerupManagerScreen_RefreshPromptWidgets(screen).
void cc13_sweepB_powerup_refresh_prompt_widgets_00612da0(void* screen, Cc13SweepBHost& host);

// 0061A400 BSP_HudPowerupManagerScreen_AssignLists(carousel, a, b): assigns
// the two by-value vectors to +18h and +28h, refreshes the label, frees both.
void cc13_sweepB_powerup_assign_lists_0061a400(void* carousel, Cc13SweepBVectorByValue a,
                                               Cc13SweepBVectorByValue b, Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 26
// ---------------------------------------------------------------------------

// 006441B0: for a kind 18h or 0Fh unit (18h maps to its plane at +3D0h): 1
// when the ordnance icon kind is 9, 0Fh, 0Ah or 0Bh and 007B9140(plane, 0)
// says no.
bool cc13_sweepB_unit_ordnance_pending_006441b0(void* unit, Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 28
// ---------------------------------------------------------------------------

// 00696450 BSP_FrontEnd_ReadAction55: *out = 004D92B0([00E188A8], 55h).
void cc13_sweepB_frontend_read_action55_00696450(std::uint8_t* out, Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 29
// ---------------------------------------------------------------------------

// 006AC260: out = the entity's world +20h/+28h direction flattened to XZ and
// normalised (zero vector when its length is not positive).
void cc13_sweepB_entity_forward_xz_006ac260(void* entity, float* out, Cc13SweepBHost& host);

// 006BE730 BSP_NativeVector20_PushBack_006be730: engine array {data, count,
// capacity} of 14h-byte elements; grows to capacity*2 + 2 when full.
void cc13_sweepB_native_vector20_push_back_006be730(void* array, const void* element, Cc13SweepBHost& host);

// 006AC4D0 / 006AC550: for each entry of the std::list at +1FCh, hand the
// entry's object (+1E4h of [value] or of [value+8h]) to the manager at
// [00E188A8]+21D0h through 00491E70(.., 0) or 0048CB30; nothing when the
// manager is null.
void cc13_sweepB_list_register_entries_006ac4d0(void* owner, Cc13SweepBHost& host);
void cc13_sweepB_list_unregister_entries_006ac550(void* owner, Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 35
// ---------------------------------------------------------------------------

// 0070BBA0 BSP_FlagParams_InitDefaults: {2.5f, 1.5f, "Zaszlok.tga", 0}.
void* cc13_sweepB_flag_params_init_0070bba0(void* params, Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 37
// ---------------------------------------------------------------------------

// 0071B5C0 BSP_AuxResourceParser_CreateItem(parser, source): a new 54h-byte
// Aux item read from source. ECX (the parser) is not read.
void* cc13_sweepB_aux_parser_create_item_0071b5c0(void* parser, void* source, Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 43
// ---------------------------------------------------------------------------

// 00786C40 BSP_PacketStatRecord_Reset: 16 zero bytes, then the word at +0Ch
// = 9999. Returns record.
void* cc13_sweepB_packet_stat_record_reset_00786c40(void* record);

// ---------------------------------------------------------------------------
// Segments 44 and 51: the engine ref-pointer release (three identical bodies)
// ---------------------------------------------------------------------------

// 0078D850 / 0081B5B0 / 0081B5E0 TMPL_RefPtr_Release: when the slot holds an
// object, InterlockedDecrement(object+4h) and, at zero, its vtable +0h; the
// slot is cleared either way.
void cc13_sweepB_ref_ptr_release_0078d850(void** slot, Cc13SweepBHost& host);
void cc13_sweepB_ref_ptr_release_0081b5b0(void** slot, Cc13SweepBHost& host);
void cc13_sweepB_ref_ptr_release_0081b5e0(void** slot, Cc13SweepBHost& host);

// 0081B1F0 TMPL_RefPtrArray_Destroy_0081B1F0: array {data, count}: release
// every slot, then free data (nothing when data is null).
void cc13_sweepB_ref_ptr_array_destroy_0081b1f0(void* array, Cc13SweepBHost& host);

// 0081C940 BSP_RefPtrRange_Release: release each slot of [first, last).
void cc13_sweepB_ref_ptr_range_release_0081c940(void** first, void** last, std::uint32_t unused1,
                                                std::uint32_t unused2, Cc13SweepBHost& host);

// 00827DD0: record {entity +0h, byte +4h, float +8h, vec3 +0Ch}: set active,
// +8h from the game settings, and +0Ch = (x, 0, z) in the entity's local
// frame (inverse world). The y argument is not used.
void cc13_sweepB_local_point_record_set_00827dd0(void* record, float x, float y, float z, Cc13SweepBHost& host);

// 0081C0A0: the unit's display name as a C string copied into a function-
// static NativeString: +160h when the count +15Ch is non-zero (empty literal
// when null), else [+538h]+54h.
const char* cc13_sweepB_unit_display_name_0081c0a0(void* unit, Cc13SweepBHost& host);

// 0081C830 BSP_UnitInstance_ApplyControlledEffectScalar(unit, other, effect).
void cc13_sweepB_unit_apply_controlled_effect_scalar_0081c830(void* unit, void* other, void* effect,
                                                             Cc13SweepBHost& host);

// 0081AF80: for each entry of the std::list at +2A4h, ensure its 1Ch-byte
// side record (+0Ch of the element) and set that record's +14h to 999.
void cc13_sweepB_list_entries_mark_999_0081af80(void* owner, Cc13SweepBHost& host);

// 00827820: 1 when the unit's +114Ch exceeds the game settings' +464h.
bool cc13_sweepB_unit_field114c_exceeds_setting_00827820(const void* unit, Cc13SweepBHost& host);

// 0081B3C0 TMPL_RefPtrArray_Grow_0081B3C0: array {data, count, capacity}:
// capacity = capacity*2 + 2, every reference copied into a new block and the
// old slot released, old block freed.
void cc13_sweepB_ref_ptr_array_grow_0081b3c0(void* array, Cc13SweepBHost& host);

// 0081B610 BSP_FloatArray_Tidy_0081b610: resize to 0, then free the block.
void cc13_sweepB_float_array_tidy_0081b610(void* array, Cc13SweepBHost& host);

// ---------------------------------------------------------------------------
// Segment 45
// ---------------------------------------------------------------------------

// 0078DAA0 BSP_OceanEnvironment_Construct(env, source, flag). Returns env.
void* cc13_sweepB_ocean_environment_construct_0078daa0(void* env, void* source, std::uint8_t flag,
                                                       Cc13SweepBHost& host);

// 0078E370 BSP_MovCamInterpolator_Init(this, word, a, b, c, d): +0h = word,
// +4h..+10h = a..d, +14h = 0. Returns this.
void* cc13_sweepB_movcam_interpolator_init_0078e370(void* interpolator, std::uint32_t word, float a, float b,
                                                    float c, float d);

// 0078FC60 BSP_MovCam_GetPathPointPair(owner, out, index): out = record +34h/+38h.
float* cc13_sweepB_movcam_path_point_pair_0078fc60(void* owner, float* out, std::int32_t index,
                                                   Cc13SweepBHost& host);

// 0078FCC0 MovieCameraKeyframe_GetPosition(keyframe, out): out = +810h..+818h.
float* cc13_sweepB_movie_keyframe_get_position_0078fcc0(const void* keyframe, float* out);

// 00794B50 BSP_MovCam_AimDeltaToTargetDefault(camera, out, target).
float* cc13_sweepB_movcam_aim_delta_default_00794b50(void* camera, float* out, void* target,
                                                     Cc13SweepBHost& host);

// 00797E40 MovieCameraTrack_ForwardToKeyModifiers(track, value): each key in
// the std::list at +68h whose kind +F8h is neither 0 nor 3 and whose modifier
// +E4h is set gets modifier->vtable+10h(value).
void cc13_sweepB_movie_track_forward_modifiers_00797e40(void* track, float value, Cc13SweepBHost& host);

// 00793670 BSP_Recon_EnemyContactIndexOf(slot, unit): the index of the first
// contact (linked list +DE8h, count +DE4h) whose record +4h is unit, or -1.
std::int32_t cc13_sweepB_recon_enemy_contact_index_of_00793670(const void* slot, const void* unit);

// 00799DE0 BSP_MovieKeyframe_SetModifier(keyframe, modifier).
void cc13_sweepB_movie_keyframe_set_modifier_00799de0(void* keyframe, void* modifier, Cc13SweepBHost& host);

// 0079C830 BSP_MovCam_ClearPathRecords(camera): +474h = 0; every record in
// the pointer vector +478h deleted and the vector emptied.
void cc13_sweepB_movcam_clear_path_records_0079c830(void* camera, Cc13SweepBHost& host);

// 00798C50 BSP_HudMovieCamera_ForwardToTrackModifiers(camera, value): both
// tracks (+2A4h, +328h) through 00797E40.
void cc13_sweepB_movie_camera_forward_modifiers_00798c50(void* camera, float value, Cc13SweepBHost& host);

// 005A2650 BSP_HudMapScreen_SetSpeakingUnit(screen, unit, fallback, flag).
void cc13_sweepB_hud_map_set_speaking_unit_005a2650(void* screen, void* unit, void* fallback, bool flag,
                                                    Cc13SweepBHost& host);

}  // namespace bsp
