#pragma once
// Packet cc13-sweepA2: continues the segment 0..14 leftover sweep of cc13-sweepA
// (the bodies sweepA listed as "skipped (context budget)").
// docs/CC13_SWEEPA2.md lists every function with its outcome, ABI and
// uncertainty, and the ones skipped with the reason.
//
// Objects are reached through cc13_sweepA::Raw (byte offsets into the Win32
// layout). Every call the image makes goes through a SweepA2Host method named
// for its target; SweepA2Host extends SweepAHost so the sweepA bindings and
// functions are reused directly. Descriptive names are hypotheses, not
// recovered symbols. Nothing here is wired.

#include "bsp/cc13_sweepA.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_sweepA2 {

using cc13_sweepA::LuaObjectStorage;
using cc13_sweepA::Raw;

struct SweepA2Host : cc13_sweepA::SweepAHost {
    // --- player / command manager [00E188D8] ------------------------------
    // [[object]+5Ch](object, kind), RET 4, the whole EAX (00525250 passes it
    // on as the 0077D600 flags; the AL tests use SweepAHost::vcall_entity_is_kind_5c).
    virtual std::uint32_t vcall_entity_is_kind_5c_eax(void* entity, std::uint32_t kind) = 0;
    // [[command]+4](command), RET, EAX = the command's name string (slot 1 of
    // vtable 00CFB378 is 006F7F30: MOV EAX,00CFB38Ch).
    virtual const char* vcall_command_name_4(void* command) = 0;
    // 0077D600 BSP_Entity_IssueCommand, __thiscall(ECX = entity, command,
    // const descriptor* (18h bytes), flags), RET 0Ch.
    virtual void entity_issue_command_0077d600(void* entity, void* command, const void* descriptor,
                                               std::uint32_t flags) = 0;

    // --- unit pick screen (screen 29h) callees ----------------------------
    // 00804350 BSP_Unit_SensorsDetectUnit, __fastcall(ECX = owner, EDX = unit,
    // int mask), RET 4, AL.
    virtual bool unit_sensors_detect_unit_00804350(void* owner, void* unit, std::int32_t mask) = 0;
    // 00522050, __thiscall(ECX = screen, const float3*), RET 4, ST0.
    virtual double unit_pick_centre_distance_sq_00522050(Raw screen, const float* point) = 0;
    // 005220C0, __thiscall(ECX = screen, unit), RET 4, AL (not reconstructed).
    virtual bool unit_pick_accepts_unit_005220c0(Raw screen, void* unit) = 0;
    // 00525D90 STL vector<{unit, float}>::push_back, __thiscall(ECX = vector,
    // const record*), RET 4 (vendor body).
    virtual void distance_vector_push_back_00525d90(Raw vector, Raw record) = 0;
    // 00524E90 TMPL_PtrFloat_IntroSortLoop, __fastcall(ECX = first, EDX =
    // last, int count), RET 4 (sort template, not reconstructed).
    virtual void ptr_float_intro_sort_00524e90(void* first, void* last, std::int32_t count) = 0;
    // 00525ED0 STL vector<observer handle 18h>::push_back, __thiscall(ECX =
    // vector, const handle*), RET 4 (vendor body).
    virtual void handle_vector_push_back_00525ed0(Raw vector, Raw handle) = 0;
    // 00695870 BSP_CallbackOwner_Destroy, __thiscall(ECX = handle).
    virtual void callback_owner_destroy_00695870(Raw handle) = 0;
    // 00927880 BSP_ControlledTarget, __fastcall(ECX = entity), EAX.
    virtual void* controlled_target_00927880(void* entity) = 0;
    // 00525170 STL instantiation, __thiscall(ECX = screen+C0h) (vendor body).
    virtual void stl_inst_00525170(Raw container) = 0;

    // 008DDF90 BSP_SzurkeNyil_ContainsUnit, __thiscall(ECX = set, unit), RET 4, AL.
    virtual bool unit_set_contains_008ddf90(void* set, void* unit) = 0;

    // --- scene contents ----------------------------------------------------
    // 004D05E0 BSP_PooledStringList_ClearStorage, __thiscall(ECX = list).
    virtual void pooled_string_list_clear_004d05e0(Raw list) = 0;
    // 0095C1C0, __fastcall(ECX = result pooled string, EDX = class id), EAX =
    // the result: the "Class_" preload alias of a Lua vehicle class.
    virtual Raw scene_class_preload_alias_0095c1c0(Raw result, std::int32_t class_id) = 0;
    // 004CE6F0, __thiscall(ECX = list, next, previous, const PooledString*),
    // RET 0Ch, EAX = a new 10h node holding a copy of the string.
    virtual void* alias_node_allocate_copy_004ce6f0(Raw list, void* next, void* previous, Raw source) = 0;
    // 004CE780, __thiscall(ECX = list, count), RET 4: the size increment with
    // the STL length check (throws past max_size).
    virtual void alias_list_grow_count_004ce780(Raw list, std::uint32_t count) = 0;

    // 004263B0 BSP_NativeString_ConcatInt, __thiscall(ECX = string, result,
    // int value), RET 8, EAX = result: result = string + decimal value.
    virtual Raw native_string_concat_int_004263b0(Raw string, Raw result, std::int32_t value) = 0;
    // 004E5B20, __thiscall(ECX = destination 10h record, const record*), RET 4:
    // vptr 00CE89D4 and +4..+0Ch copied.
    virtual void scene_property_value_copy_construct_004e5b20(Raw destination, void* source) = 0;
    // 008F0DE0, __thiscall(ECX = record): vptr 00CE89D4, then jumps to 008F0640.
    virtual void scene_property_value_destroy_008f0de0(Raw record) = 0;
    // 00964790 BSP_VehicleClass_GetOrCreate, __fastcall(ECX = type id, DL =
    // read race), EAX = the class.
    virtual void* vehicle_class_get_or_create_00964790(std::int32_t type_id, std::uint8_t read_race) = 0;

    // --- Lua class readers -------------------------------------------------
    // 0087CA80 BSP_DamageableClass_ReadLuaFields, __thiscall(ECX = class,
    // LuaObject* table), RET 4.
    virtual void damageable_class_read_lua_fields_0087ca80(Raw cls, LuaObjectStorage* table) = 0;
    // 00B65FB0 BSP_LuaObject_IsNil, __thiscall(ECX = object), AL.
    virtual bool lua_object_is_nil_00b65fb0(LuaObjectStorage* object) = 0;
    // 00B66290 BSP_LuaObject_GetInteger, __thiscall(ECX = object), EAX
    // (lua_tonumber truncated by __ftol).
    virtual std::int32_t lua_object_get_integer_00b66290(LuaObjectStorage* object) = 0;
    // LuaPlus LuaObject type tests and reads, each __thiscall(ECX = object):
    // 00B661B0 IsTable, 00B66A60 IsInteger, 00B66000 IsBoolean, 00B66050
    // IsNumber, 00B660A0 IsString (AL); 00B66250 GetBoolean (EAX), 00B66270
    // GetNumber (ST0), 00B662B0 GetString (EAX).
    virtual bool lua_object_is_table_00b661b0(LuaObjectStorage* object) = 0;
    virtual bool lua_object_is_integer_00b66a60(LuaObjectStorage* object) = 0;
    virtual bool lua_object_is_boolean_00b66000(LuaObjectStorage* object) = 0;
    virtual bool lua_object_is_number_00b66050(LuaObjectStorage* object) = 0;
    virtual bool lua_object_is_string_00b660a0(LuaObjectStorage* object) = 0;
    virtual std::uint32_t lua_object_get_boolean_00b66250(LuaObjectStorage* object) = 0;
    virtual double lua_object_get_number_00b66270(LuaObjectStorage* object) = 0;
    virtual const char* lua_object_get_string_00b662b0(LuaObjectStorage* object) = 0;
    // Scene property bag writers, each __thiscall(ECX = bag, const char* name,
    // value), RET 8: 008F3710 int, 008F3940 bool (the whole EAX of
    // GetBoolean), 008F3770 float, 008F38A0 string, 008F3B60 sub-bag.
    virtual void scene_property_bag_add_int_008f3710(void* bag, const char* name, std::int32_t value) = 0;
    virtual void scene_property_bag_add_bool_008f3940(void* bag, const char* name, std::uint32_t value) = 0;
    virtual void scene_property_bag_add_float_008f3770(void* bag, const char* name, float value) = 0;
    virtual void scene_property_bag_add_string_008f38a0(void* bag, const char* name, const char* value) = 0;
    virtual void scene_property_bag_add_sub_bag_008f3b60(void* bag, const char* name, void* sub_bag) = 0;
    // 00BF681B operator new(size), __cdecl.
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    // 008F41A0 BSP_ScenePropertyBag_Construct, __thiscall(ECX = 114h block,
    // int zero), RET 4, EAX = the bag.
    virtual void* scene_property_bag_construct_008f41a0(void* block, std::int32_t arg) = 0;
    // 00870CD0 BSP_EffectHandle_Acquire, __fastcall(ECX = handle out, EDX =
    // effect id, int arg), RET 4, EAX = the handle slot (a retained pointer
    // whose +4 is the reference count).
    virtual void** effect_handle_acquire_00870cd0(void** handle_out, std::int32_t id, std::int32_t arg) = 0;

    // 00415450 STL vector size, __thiscall(ECX = vector), EAX (vendor body).
    virtual std::uint32_t vector_size_00415450(Raw vector) = 0;
    // 00442190 STL vector<int>::push_back, __thiscall(ECX = vector, const
    // int*), RET 4 (vendor body).
    virtual void int_vector_push_back_00442190(Raw vector, const std::int32_t* value) = 0;
    // 0043EE60 BSP_String_FormatBuffer2A, __cdecl(buffer, format, ...):
    // _vsprintf_s(buffer, 2Ah, format, args); called here with one int.
    virtual void string_format_buffer2a_0043ee60(char* buffer, const char* format, std::int32_t value) = 0;
    // 006B8AD0 BSP_LuaMachine_RunString, __thiscall(ECX = machine, source,
    // result sink, error out, mode), RET 10h.
    virtual void lua_machine_run_string_006b8ad0(void* machine, const char* source, void* sink, void* error_out,
                                                 std::int32_t mode) = 0;

    // --- device entity kill (segment 3) ----------------------------------
    // [[entity]+194h](entity, const NativeString* event, 0), RET 8.
    virtual void vcall_entity_lua_event_194(void* entity, Raw event, std::int32_t arg) = 0;
    // [[unit]+1A8h](unit).
    virtual void vcall_unit_part_destroyed_1a8(void* unit) = 0;
    // 0043F8E0 STL vector<T*>::operator[], __thiscall(ECX = vector, index),
    // RET 4, EAX = &element (vendor body, checked).
    virtual void** pointer_vector_at_0043f8e0(Raw vector, std::uint32_t index) = 0;
    // 00439820 BSP_Vector3f_TransformProjectPoint, __thiscall(ECX = float3*
    // out, EDX = const float3* local, const 4x4*, float* inverse_w), RET 8.
    virtual void vector3_transform_project_point_00439820(float* out, const float* local, const float* matrix,
                                                         float* inverse_w) = 0;
    // 00B6E0D0 BSP_Node_GetInverseWorldMatrix, __thiscall(ECX = node), EAX.
    virtual const float* node_get_inverse_world_matrix_00b6e0d0(void* node) = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint, __thiscall(ECX = const
    // float3* in, float3* out, const 4x4*), RET 8, EAX = out.
    virtual float* vector3_transform_affine_point_004142e0(const float* in, float* out, const float* matrix) = 0;
    // 008689C0 BSP_PointEffect_Create, __thiscall(ECX = handle out, EDX =
    // parent node, effect handle by value (released by the callee), const
    // float3*, transform byte, option byte, tail), RET 14h, EAX = out.
    virtual void** point_effect_create_008689c0(void** out, void* parent, void* handle_by_value, const float* point,
                                                std::int32_t transform, std::int32_t option,
                                                std::int32_t tail) = 0;
    // 00878990 BSP_UnitTickableEntity_OnKilled, __thiscall(ECX = entity).
    virtual void unit_tickable_entity_on_killed_00878990(void* entity) = 0;

    // --- movie action lists (segment 5) ----------------------------------
    // 00455680 STL list buy-node, __thiscall(ECX = list, next, previous,
    // const value*), RET 0Ch, EAX = node (vendor body).
    virtual void* list_buy_node_00455680(Raw list, void* next, void* previous, void* const* value) = 0;
    // 004599A0 STL list size increment with the length check, __thiscall(ECX =
    // list, count), RET 4 (vendor body).
    virtual void list_grow_count_004599a0(Raw list, std::uint32_t count) = 0;
    // 00455E30 BSP_EntityTrack_FindActionBefore (reconstructed in
    // src/actions_seg5_cc13.cpp), __thiscall(ECX = track, iterator out[2],
    // float time), RET 8.
    virtual void entity_track_find_action_before_00455e30(Raw track, void** iterator_out, float time) = 0;
    // 00459FE0 STL list insert, __thiscall(ECX = list, iterator* result,
    // container, node, const value*), RET 10h (vendor body).
    virtual void list_insert_00459fe0(Raw list, void** result, void* container, void* node,
                                      void* const* value) = 0;
    // [[track]+0](track), AL: the track-kind test used by 0045D930.
    virtual bool vcall_track_is_camera_track_0(void* track) = 0;
    // 0045BE40 BSP_MovieCameraTrack_Destroy / 0045B220
    // BSP_MovieEntityTrack_Destroy, __thiscall(ECX = track).
    virtual void movie_camera_track_destroy_0045be40(void* track) = 0;
    virtual void movie_entity_track_destroy_0045b220(void* track) = 0;
    // 005B9760 BSP_VoiceManager_BeginSoundFade, __thiscall(ECX = manager,
    // float target, float duration, const NativeString*), RET 0Ch.
    virtual void voice_manager_begin_sound_fade_005b9760(void* manager, float target, float duration, Raw name) = 0;
    // [[track]+8](track), RET.
    virtual void vcall_track_changed_8(void* track) = 0;

    // --- matrices and projection ------------------------------------------
    // 00413920, __thiscall(ECX = 4x4 a, float out[16], const 4x4 b), RET 8,
    // EAX = out: out = a * b.
    virtual float* matrix_multiply_00413920(const float* a, float* out, const float* b) = 0;
    // 004134F0, __thiscall(ECX = destination 4x4, const source 4x4), RET 4.
    virtual void matrix_copy_004134f0(float* destination, const float* source) = 0;
    // 00638E50 BSP_HudMarker_ProjectAndClip, __fastcall(ECX = const float3*
    // world, EDX = float3* screen), AL.
    virtual bool hud_marker_project_and_clip_00638e50(const float* world, float* screen) = 0;

    // --- GUI manager and front-end pages -----------------------------------
    // 004C12B0 BSP_GuiManager_GetOrCreate, no arguments, EAX = [00F8BC5C].
    virtual void* gui_manager_get_or_create_004c12b0() = 0;
    // 00AA4F80 BSP_GuiManager_Update, __thiscall(ECX = manager, float dt,
    // int zero), RET 8.
    virtual void gui_manager_update_00aa4f80(void* manager, float dt, std::int32_t arg) = 0;
    // 005329C0, __thiscall(ECX = menu command screen, vector<page*>*), RET 4:
    // appends its visible pages.
    virtual void front_end_collect_visible_pages_005329c0(void* menu_screen, Raw pages) = 0;
    // 004D6790 STL vector<page*>::push_back, __thiscall(ECX = vector, const
    // value*), RET 4 (vendor body).
    virtual void page_vector_push_back_004d6790(Raw pages, void* const* value) = 0;
    // 00AA5840 BSP_GuiManager_LoadPage, __thiscall(ECX = manager, const
    // NativeString* name, int screen flag, char add reference), RET 0Ch, EAX.
    virtual void* gui_manager_load_page_00aa5840(void* manager, Raw name, std::int32_t screen_flag,
                                                 std::uint8_t add_reference) = 0;
    // 004D35D0 STL vector<page*> copy constructor, __thiscall(ECX =
    // destination, const source*), RET 4 (vendor body).
    virtual void page_vector_copy_construct_004d35d0(Raw destination, Raw source) = 0;
    // 00AA3F70 BSP_GuiManager_UpdatePageList, __thiscall(ECX = manager,
    // vector<page*> by value (10h bytes, destroyed by the callee), float dt),
    // RET 14h.
    virtual void gui_manager_update_page_list_00aa3f70(void* manager, Raw pages_by_value, float dt) = 0;

    // --- multiplayer session prompts --------------------------------------
    // [[object]+14h](object), AL (on [00F8A2FC], the network session).
    virtual bool vcall_session_query_14(void* session) = 0;
    // 006881F0 BSP_MultiMenu_PopRequestInterface, no register or stack input.
    virtual void multi_menu_pop_request_interface_006881f0() = 0;
    // 004CD0F0 BSP_Game_SetCinematicMode, __thiscall(ECX = game, hide,
    // allow simulation, p4), RET 0Ch.
    virtual void game_set_cinematic_mode_004cd0f0(Raw game, std::uint8_t hide, std::uint8_t allow_simulation,
                                                  std::uint8_t p4) = 0;
    // 00531B00 BSP_MenuPromptScreen_Raise, __thiscall(ECX = menu command
    // screen, a1, const NativeString* text, a3, callback, a5, const
    // NativeString* a6, float a7, a8, NativeString by value (8 bytes, released
    // by the callee), a10), RET 2Ch.
    virtual void menu_prompt_screen_raise_00531b00(void* menu, std::int32_t a1, Raw text, std::int32_t a3,
                                                   std::uint32_t callback, std::int32_t a5, Raw a6, float a7,
                                                   std::int32_t a8, Raw string_by_value, std::int32_t a10) = 0;

    // --- tactical library screen -------------------------------------------
    // 00505DE0 STL vector erase(first, last), __thiscall(ECX = vector,
    // iterator* result, container, first, container, last), RET 14h (vendor).
    virtual void entry_vector_erase_00505de0(Raw vector, void** result, void* first_container, void* first,
                                             void* last_container, void* last) = 0;
    // 00A9CA00 STL instantiation, __thiscall(ECX = list widget), EAX = the
    // selected item or null (vendor body).
    virtual void* list_widget_selected_item_00a9ca00(void* widget) = 0;
    // [[item]+28h](item), EAX = a NativeString* (its data at +4).
    virtual Raw vcall_item_text_28(void* item) = 0;
    // 004F9360 STL vector<entry 2Ch>::operator[], __thiscall(ECX = vector,
    // index), RET 4, EAX = &entry (vendor body, checked).
    virtual Raw library_entry_at_004f9360(Raw vector, std::uint32_t index) = 0;
    // 004FB090 BSP_TacticalLibrary_FindEntryById, __thiscall(ECX = screen,
    // id), RET 4, EAX.
    virtual void* tactical_library_find_entry_by_id_004fb090(Raw screen, std::int32_t id) = 0;
    // 0050AD00 STL vector push_back(const value&), __thiscall(ECX = vector,
    // const value*), RET 4 (vendor body; element type not established).
    virtual void entry_vector_push_back_0050ad00(Raw vector, void* entry) = 0;

    // --- HUD digit icons -----------------------------------------------------
    // 00BF9010 _itoa(value, buffer, radix), __cdecl.
    virtual void crt_itoa_00bf9010(std::int32_t value, char* buffer, std::int32_t radix) = 0;
    // [[widget]+A0h](widget), EAX = the widget's vertex stream.
    virtual void* vcall_widget_vertex_stream_a0(void* widget) = 0;
    // [[widget]+9Ch](widget, 0), EAX = the icon record (u0 +0Ch, v0 +10h,
    // u1 +14h, v1 +18h).
    virtual void* vcall_widget_icon_record_9c(void* widget, std::int32_t index) = 0;
    // [[stream]+10h](stream, 0, 0, 0), RET 0Ch: lock.
    virtual void vcall_vertex_stream_lock_10(void* stream, std::int32_t a, std::int32_t b, std::int32_t c) = 0;
    // [[stream]+14h](stream): unlock.
    virtual void vcall_vertex_stream_unlock_14(void* stream) = 0;

    // --- layered scene entities (segment 7) --------------------------------
    // 00489C40, __thiscall(ECX = game [00E188A8], const float3*), RET 4, EAX =
    // a record whose +4 is a side (tagged STL_inst; the body is not
    // reconstructed here).
    virtual void* game_lookup_at_point_00489c40(void* game, const float* point) = 0;
    // 00803BE0 BSP_Party_RelativeToLocalPlayer, __fastcall(ECX = side), EAX.
    virtual std::int32_t party_relative_to_local_player_00803be0(std::int32_t side) = 0;
    // [[factory]+8](factory), EAX = the new member object.
    virtual void* vcall_factory_create_8(void* factory) = 0;
    // [[object]+98h](object, 0, scene, const float matrix[16]), RET 0Ch.
    virtual void vcall_object_attach_98(void* object, std::int32_t zero, void* scene, const float* matrix) = 0;
    // [[object]+114h](object, owner), RET 4.
    virtual void vcall_object_set_owner_114(void* object, void* owner) = 0;
    // [[object]+A0h](object).
    virtual void vcall_object_activate_a0(void* object) = 0;

    // --- command-building view camera (segment 14) -------------------------
    // 005470C0 BSP_HudWeaponGroupScreen_Release, __thiscall(ECX = screen).
    virtual void hud_weapon_group_screen_release_005470c0(void* screen) = 0;
    // 0077C470, __thiscall(ECX = entity, int message, int arg), RET 8.
    virtual void entity_send_message_0077c470(void* entity, std::int32_t message, std::int32_t arg) = 0;
    // 00694AF0 BSP_Observer_IsPairRegistered, __fastcall(ECX = observed, EDX =
    // observer), AL.
    virtual bool observer_is_pair_registered_00694af0(void* observed, void* observer) = 0;
    // 0051E8E0 BSP_HudBinoculars_Reset, __thiscall(ECX = binoculars).
    virtual void hud_binoculars_reset_0051e8e0(void* binoculars) = 0;
    // 00BF79F0 memset is bound directly; 00519290 BSP_ShipCamera_ConstructBase,
    // __thiscall(ECX = 3F4h block), EAX = the camera.
    virtual void* ship_camera_construct_base_00519290(void* block) = 0;
    // 00923870, __thiscall(ECX = camera, 0, scene, const float matrix[16]),
    // RET 0Ch.
    virtual void camera_attach_00923870(void* camera, std::int32_t zero, void* scene, const float* matrix) = 0;
    // 0042F650 BSP_CameraMover_SetTarget, __thiscall(ECX = mover, unit), RET 4.
    virtual void camera_mover_set_target_0042f650(void* mover, void* unit) = 0;
    // 004BC410 BSP_Game_SetCameraMover, __thiscall(ECX = game, mover), RET 4.
    virtual void game_set_camera_mover_004bc410(void* game, void* mover) = 0;
    // 0051E730 BSP_HudBinoculars_BindUnit, __thiscall(ECX = binoculars,
    // camera), RET 4.
    virtual void hud_binoculars_bind_0051e730(void* binoculars, void* camera) = 0;
    // 00549260 BSP_HudWeaponGroupScreen_BindUnit, __thiscall(ECX = screen,
    // unit, camera), RET 8.
    virtual void hud_weapon_group_screen_bind_00549260(void* screen, void* unit, void* camera) = 0;

    // --- named class base reader (segment 7) -------------------------------
    // 007188A0 BSP_Resource_LoadWithGameFactory, __thiscall(ECX = const
    // NativeString* name), EAX = the resource.
    virtual void* resource_load_with_game_factory_007188a0(Raw name) = 0;
    // 00B66380 BSP_LuaReference_GetIntegerOrDefault, __thiscall(ECX = object,
    // int default), RET 4, EAX.
    virtual std::int32_t lua_object_get_integer_or_default_00b66380(LuaObjectStorage* object,
                                                                   std::int32_t fallback) = 0;
    // 00408720 BSP_CharacterString_AssignCounted (MSVC std::string assign of
    // (const char*, count)), __thiscall(ECX = string, text, count), RET 8.
    virtual void std_string_assign_counted_00408720(Raw string, const char* text, std::uint32_t count) = 0;
    // 00718870 BSP_GameResource_LookupNamedPointGroup, __thiscall(ECX =
    // resource, const std::string* name, int index), RET 8, EAX.
    virtual void* resource_lookup_named_point_group_00718870(void* resource, Raw name, std::int32_t index) = 0;
    // 00484270 STL instantiation, __thiscall(ECX = point group, float3* out,
    // int index), RET 8, EAX = the point (vendor body).
    virtual const float* point_group_point_00484270(void* group, float* out, std::int32_t index) = 0;

    // --- unit pick command issue (segment 14) ------------------------------
    // 0075B430 BSP_SessionMessage_ConstructBase, __thiscall(ECX = message,
    // int type), RET 4.
    virtual void session_message_construct_base_0075b430(Raw message, std::int32_t type) = 0;
    // [[player]+13Ch](player), EAX = the session object messages route through.
    virtual void* vcall_player_session_13c(void* player) = 0;
    // 0077C2A0 BSP_Session_RouteMessage, __thiscall(ECX = session, message,
    // int a, int b), RET 0Ch.
    virtual void session_route_message_0077c2a0(void* session, Raw message, std::int32_t a, std::int32_t b) = 0;
    // 00465080 BSP_CommandTarget_FromEntity, __thiscall(ECX = descriptor out,
    // entity, float), RET 8, EAX = descriptor.
    virtual Raw command_target_from_entity_00465080(Raw out, void* entity, float value) = 0;
    // 004B4B00 BSP_Game_GetControlledVehicle, no input, EAX.
    virtual void* game_get_controlled_vehicle_004b4b00() = 0;
    // 007BB170 BSP_Plane_SetPilotCommandLastTime, __thiscall(ECX = plane, int
    // slot, float time), RET 8.
    virtual void plane_set_pilot_command_last_time_007bb170(void* plane, std::int32_t slot, float time) = 0;
    // 0042E740 BSP_GameTuning_GetSingleton, no input, EAX.
    virtual void* game_tuning_get_singleton_0042e740() = 0;

    // --- scene header lists (segment 11) ------------------------------------
    // 0094EC70 (split_command_line_0094ec70 in src/app_bootstrap.cpp),
    // __fastcall(ECX = vector<std::string> out, EDX = const std::string*,
    // const char* delimiters), RET 4.
    virtual void split_string_0094ec70(Raw out, Raw text, const char* delimiters) = 0;
    // 004CDC20 BSP_NativeStringVector_Append, __thiscall(ECX = vector, const
    // NativeString*), RET 4.
    virtual void native_string_vector_append_004cdc20(void* vector, Raw string) = 0;
    // 004CE520 STL_StringRange_Destroy_1C, __fastcall(ECX = first, EDX =
    // last, allocator*, unused), RET 8 (vendor body).
    virtual void string_range_destroy_004ce520(void* first, void* last, void* allocator, void* unused) = 0;

    // --- award tracker teardown (segment 10) --------------------------------
    // STL tree erase(first, last) instantiations, each __thiscall(ECX = tree,
    // iterator* result, container, first node, container, last node), RET 14h
    // (vendor bodies): 004DE3E0, 00444B10, 004DC4E0.
    virtual void tree_erase_range_004de3e0(Raw tree, void** result, void* c1, void* first, void* c2, void* last) = 0;
    virtual void tree_erase_range_00444b10(Raw tree, void** result, void* c1, void* first, void* c2, void* last) = 0;
    virtual void tree_erase_range_004dc4e0(Raw tree, void** result, void* c1, void* first, void* c2, void* last) = 0;
    // 004D0140 BSP_AwardTracker_DrainHintDeque, __thiscall(ECX = deque).
    virtual void award_tracker_drain_hint_deque_004d0140(Raw deque) = 0;

    // --- sound -------------------------------------------------------------
    // 00A7E490, __thiscall(ECX = sound manager [00F8BBD8], hidden result
    // slot, sample, class index, type index, uint8 persistent), RET 14h:
    // writes a retained tracked sound (or null) to *result.
    virtual void sound_create_tracked_00a7e490(void* manager, void** result, void* sample,
                                               std::int32_t class_index, std::int32_t type_index,
                                               std::uint8_t persistent) = 0;
};

// ============================ segment 11 ===================================
void land_convoy_preload_type_classes_004ee300(void* bag, SweepA2Host& host);
void scene_header_append_unquoted_names_004f1b60(const char* list, void* owner, SweepA2Host& host);
bool scene_property_bag_remove_record_004f01f0(Raw bag, void* record, SweepA2Host& host);

// ============================ segment 3 ====================================
void hud_digit_icon_roll_uv_0043aba0(std::int32_t digit_index, void* widget, float magnitude, SweepA2Host& host);
void lua_table_copy_into_property_bag_0043d8f0(LuaObjectStorage* table, void* bag, SweepA2Host& host);
void device_entity_on_killed_lua_00440f50(Raw entity, SweepA2Host& host);
void device_class_registry_clear_lua_class_tables_004423a0(SweepA2Host& host);
void gun_class_read_lua_explosion_efx_00440ac0(Raw cls, LuaObjectStorage* table, SweepA2Host& host);

// ============================ segment 5 ====================================
void cutscene_player_destruct_0045d930(Raw player, SweepA2Host& host);
Raw movie_record_copy_construct_0045f110(Raw destination, Raw source, SweepA2Host& host);
Raw movie_record_assign_0045d230(Raw destination, Raw source, SweepA2Host& host);
void movie_track_insert_action_sorted_0045a210(Raw track, void* action, SweepA2Host& host);

// ============================ segment 7 ====================================
void named_class_read_lua_0048f670(Raw cls, LuaObjectStorage* table, SweepA2Host& host);
void* scene_entity_layer_member_0048c6a0(Raw entity, std::int32_t layer, std::int32_t index, SweepA2Host& host);

// ============================ segment 10 ===================================
void scene_build_class_preload_aliases_004d4720(Raw list, SweepA2Host& host);
void award_tracker_destroy_members_004e1a50(Raw tracker, SweepA2Host& host);
void game_show_session_lost_message_004d7ba0(Raw game, std::uint8_t kicked, SweepA2Host& host);
void frontend_update_active_screens_004d8620(Raw game, SweepA2Host& host);

// ============================ segment 13 ===================================
void tactical_library_show_selected_group_0050b730(Raw screen, SweepA2Host& host);

// ============================ segment 14 ===================================
void command_building_view_bind_unit_camera_005213d0(Raw view, void* unit, SweepA2Host& host);
std::int32_t squadron_member_nearest_screen_centre_005224c0(Raw squadron, SweepA2Host& host);
bool unit_pick_is_command_candidate_005226d0(void* unit, SweepA2Host& host);
void unit_pick_consider_unit_00522f10(Raw screen, void* unit, void** command_out, float* best_distance,
                                      std::int32_t mask, void* owner, void** best, SweepA2Host& host);
void* unit_pick_find_member_target_00523020(Raw screen, void** command_out, SweepA2Host& host);
void* unit_pick_find_command_target_00523130(Raw screen, void** command_out, SweepA2Host& host);
bool unit_pick_issue_command_at_cursor_00525250(Raw screen, SweepA2Host& host);
void unit_pick_collect_candidates_00525f80(Raw screen, SweepA2Host& host);
bool unit_pick_issue_command_to_target_005241d0(Raw screen, void* command, SweepA2Host& host);

}  // namespace bsp::cc13_sweepA2
