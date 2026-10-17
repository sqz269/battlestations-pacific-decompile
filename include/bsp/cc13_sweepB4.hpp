#pragma once

// cc13_sweepB4: fourth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB4, docs/CC13_SWEEPB4.md). Every name is a hypothesis, not a
// recovered symbol. These are new C++ interfaces over the native records, not
// drop-in replacements of the original ABI. Nothing here is wired into the game.
//
// Same conventions as cc13_sweepB / B2 / B3: native objects are raw byte
// blocks touched only at the offsets the image reads or writes, and every call
// the image makes goes through a named host method at the same point with the
// same arguments. Cc13SweepB4Host extends Cc13SweepB3Host, so the earlier
// hooks (image_global, the CRT hooks, the sized pool, ...) are reused.

#include "bsp/cc13_sweepB3.hpp"

#include <cstdint>

namespace bsp {

class Cc13SweepB4Host : public Cc13SweepB3Host {
public:
    // ---- path cursor (segment 46) ----
    // Path cursor vtable +4h: __thiscall(ECX = cursor, float* out, int index),
    // EAX = the point (x at +0, z at +8).
    virtual const float* path_cursor_vcall_04_point(void* cursor, float* out, std::int32_t index) = 0;
    // Path cursor vtable +8h: __thiscall(ECX = cursor, int index), sets the index.
    virtual void path_cursor_vcall_08_set_index(void* cursor, std::int32_t index) = 0;
    // Path source vtable +0Ch: __thiscall(ECX = path), no arguments, EAX = point count.
    virtual std::int32_t path_vcall_0c_count(void* path) = 0;
    // 007ADCC0 BSP_EntityCommand_PathCursorNextIndex: __thiscall(ECX = cursor), EAX
    // (flips the direction byte +10h in place for PATH_FM_PINGPONG).
    virtual std::int32_t path_cursor_next_index_007adcc0(void* cursor) = 0;
    // 00438B10 BSP_Math_SubtractWrappedAngle: two stack floats, RET 8, ST0 result.
    virtual double math_subtract_wrapped_angle_00438b10(float a, float b) = 0;

    // ---- STL instantiations (called, not reconstructed) ----
    // 005A0B80 std::list tidy: ECX = list {proxy, head, size}.
    virtual void list_tidy_005a0b80(void* list) = 0;
    // 00658B50: ECX = the checked map iterator {container, node} at screen+350h;
    // EAX = the pointer to the node's value (node+8h).
    virtual void* support_map_iterator_value_00658b50(void* iterator) = 0;
    // 00658A60: ECX = the checked iterator at screen+300h; EAX = the address of
    // the element slot (the unit pointer).
    virtual void** support_unit_iterator_value_00658a60(void* iterator) = 0;
    // 0059FB60: ECX = a checked list iterator {list, node}; EAX = &node->value.
    virtual void** list_iterator_value_0059fb60(void* iterator) = 0;

    // ---- objects ----
    // Vtable +14h, __thiscall(ECX = object), no stack arguments; EAX = a C
    // string (the display name). 00ABBE50 is RET 8, so the PUSH 1 in front of
    // these calls is 00ABBE50's second argument.
    virtual const char* object_vcall_14_name(void* object) = 0;
    // Vtable +5Ch, __thiscall(ECX = object, int kind), RET 4; AL.
    virtual bool object_vcall_5c_is_kind(void* object, std::int32_t kind) = 0;

    // ---- HUD support manager (segment 27) ----
    // 00670020 (the support manager's full rebuild, not reconstructed here):
    // __fastcall(ECX = screen).
    virtual void hud_support_rebuild_00670020(void* screen) = 0;
    // 0065BBC0 BSP_HudSupportManager_LayoutNameLabel: __fastcall(ECX = screen).
    virtual void hud_support_layout_name_label_0065bbc0(void* screen) = 0;
    // 008434D0 BSP_Shipyard_CountIdleSlots: __fastcall(ECX = shipyard), EAX.
    virtual std::int32_t shipyard_count_idle_slots_008434d0(void* shipyard) = 0;
    // 006BDC30 BSP_AirOps_CountIdleSlots: __fastcall(ECX = slot block), EAX.
    virtual std::int32_t air_ops_count_idle_slots_006bdc30(void* block) = 0;
    // 00946F00: __fastcall(ECX = player), ST0 (the free support points).
    virtual double player_support_points_00946f00(void* player) = 0;
    // 004BCA50 BSP_Game_GetEffectiveGameMode: __fastcall(ECX = game), EAX.
    virtual std::int32_t game_effective_mode_004bca50(void* game) = 0;
    // 004263B0 BSP_NativeString_ConcatInt: __thiscall(ECX = left, out, int),
    // RET 8; constructs out and returns it.
    virtual void* native_string_concat_int_004263b0(const void* left, void* out, std::int32_t value) = 0;

    // ---- GUI widgets ----
    // 00AA6820 BSP_GuiWidget_SetVisibilityRecursive: ECX = widget, (int), RET 4.
    virtual void gui_widget_set_visibility_recursive_00aa6820(void* widget, std::int32_t show) = 0;
    // Widget vtable +58h, __thiscall(ECX = widget, const float* pair), RET 4
    // (the counter layouts pass {0.5, 0.5} and {width, height} pairs).
    virtual void widget_vcall_58_set_pair(void* widget, const float* pair) = 0;
    // Widget vtable +48h, __thiscall(ECX = widget, const float* pair), RET 4.
    virtual void widget_vcall_48_set_pair(void* widget, const float* pair) = 0;

    // ---- HUD counter rows (segment 15) ----
    // 0052AE60 TMPL_Rec18Array_Resize: ECX = the 18h-record array (entry+70h), (int), RET 4.
    virtual void counter_units_resize_0052ae60(void* array, std::int32_t count) = 0;
    // 0052AD50 TMPL_Rec18Array_PushBack: ECX = array, (const record*), RET 4.
    virtual void counter_units_push_back_0052ad50(void* array, const void* record) = 0;
    // 0052ADD0 TMPL_Rec18Array_RemoveAtSwapLast: ECX = array, (int index), RET 4.
    virtual void counter_units_remove_at_0052add0(void* array, std::int32_t index) = 0;
    // 006952A0 BSP_Observer_UnregisterPair: ECX = endpoint, EDX = callback owner.
    virtual void observer_unregister_pair_006952a0(void* endpoint, void* owner) = 0;
    // 00695870 BSP_CallbackOwner_Destroy: ECX = callback owner.
    virtual void callback_owner_destroy_00695870(void* owner) = 0;
    // 00426060 BSP_NativeString_CopyConstruct: ECX = destination, (const source*), RET 4.
    virtual void native_string_copy_construct_00426060(void* dest, const void* source) = 0;
    // 00528B00 BSP_HudCounter_FormatTime: ECX = entry, (float seconds), RET 4.
    virtual void hud_counter_format_time_00528b00(void* entry, float seconds) = 0;
    // 00528E90 BSP_HudCounter_UpdateBar: ECX = entry, (float, float), RET 8.
    virtual void hud_counter_update_bar_00528e90(void* entry, float a, float b) = 0;
    // 004B6260 BSP_NetworkSession_CountsMissionStart: ECX = [00E188A8] + 1EF0h; AL.
    virtual bool session_counts_mission_start_004b6260(void* session) = 0;
    // 00939F70 / 00939F80 / 00939FC0 / 00939FB0: ECX = object, ST0 = the float
    // at +38h / +34h / +30h / +2Ch (one-instruction getters, called).
    virtual double getter_38_00939f70(void* object) = 0;
    virtual double getter_34_00939f80(void* object) = 0;
    virtual double getter_30_00939fc0(void* object) = 0;
    virtual double getter_2c_00939fb0(void* object) = 0;

    // ---- HUD formation screen (segment 15) ----
    // 00A9C920 BSP_GuiListbox_GetSelectedIndex: ECX = list box, EAX.
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    // Unit vtable +114h, __thiscall(ECX = unit), no arguments; EAX = the
    // command director (or null).
    virtual void* unit_vcall_114_director(void* unit) = 0;
    // 0071BE40 BSP_WeaponDirector_CurrentCommand: __fastcall(ECX = director);
    // EAX = the current command's class singleton (an image address
    // 00E08F10..00E08FA0, compared through image_global) or null.
    virtual void* director_current_command_0071be40(void* director) = 0;
    // 00923BE0 BSP_UnitInstance_GetHealth: __thiscall(ECX = unit), ST0.
    virtual double unit_health_00923be0(void* unit) = 0;
    // 00653200 BSP_Hud_PayloadIconFrame: ECX = [00E198C4]+0BCh, (payload), RET 4; EAX.
    virtual std::int32_t hud_payload_icon_frame_00653200(void* gui, void* payload) = 0;

    // ---- main menu (segment 20) ----
    // 0076FAD0 BSP_Session_SetMode: ECX = [00E188A8] + 1EF0h, (int), RET 4.
    virtual void session_set_mode_0076fad0(void* session, std::int32_t mode) = 0;
    // 00518D60 BSP_FrontEndFrame_Select: ECX = backdrop, EDX = panel, stack
    // (title, const native string*), RET 8.
    virtual void frontend_frame_select_00518d60(std::int32_t backdrop, std::int32_t panel, std::int32_t title,
                                                const void* text) = 0;
    // 0058D9E0 BSP_MainMenuScreen_BuildMissionMapIcons: ECX = screen, (int), RET 4.
    virtual void main_menu_build_mission_map_icons_0058d9e0(void* screen, std::int32_t arg) = 0;
    // CRT: 00BF9886 _time64 (cdecl, EAX:EDX; only EAX is used), 00BF98C2
    // srand (cdecl), 00BF98CF rand (EAX). The CRT generator, not 00BD2F10.
    virtual std::int64_t crt_time64_00bf9886(void* timer) = 0;
    virtual void crt_srand_00bf98c2(std::uint32_t seed) = 0;
    virtual std::int32_t crt_rand_00bf98cf() = 0;
    // Main-menu page builders, ECX = screen: 00584AE0 (int, RET 4), 00584F50,
    // 005853C0, 00597870 (int page, RET 4), 0058C010, 005886F0, 00594BF0.
    virtual void main_menu_build_top_level_00584ae0(void* screen, std::int32_t arg) = 0;
    virtual void main_menu_build_single_player_00584f50(void* screen) = 0;
    virtual void main_menu_build_multiplayer_005853c0(void* screen) = 0;
    virtual void main_menu_build_mission_list_00597870(void* screen, std::int32_t page) = 0;
    virtual void main_menu_build_mission_detail_0058c010(void* screen) = 0;
    virtual void main_menu_build_tactical_library_005886f0(void* screen) = 0;
    virtual void main_menu_build_objective_rows_00594bf0(void* screen) = 0;
    // 00549620 BSP_MainMenuCommandOwner_SetHelpVariant: ECX = [00E1930C], (int), RET 4.
    virtual void main_menu_command_owner_set_help_variant_00549620(void* owner, std::int32_t variant) = 0;
    // 005F6EF0 BSP_OptionsScreen_SyncProfileValues: ECX = options screen.
    virtual void options_screen_sync_profile_values_005f6ef0(void* screen) = 0;

    // ---- multiplayer lobby (segment 23) ----
    // 005E1990 BSP_LobbySettings_GetOptionTable: no register input, EAX.
    virtual void* lobby_option_table_005e1990() = 0;
    // 005D5630 BSP_LobbySettings_SlotForMenuRow: __fastcall(ECX = row), EAX.
    virtual std::int32_t lobby_slot_for_menu_row_005d5630(std::int32_t row) = 0;
    // 005E1BF0 BSP_MultiGameLobby_StepMapIndex: ECX = screen, (int step), RET 4.
    virtual void lobby_step_map_index_005e1bf0(void* screen, std::int32_t step) = 0;
    // 0041E350 BSP_NativeString_AssignCString: ECX = string, (const char*), RET 4.
    virtual void native_string_assign_cstring_0041e350(void* string, const char* text) = 0;
    // 005D4AF0 (STL instantiation): ECX = screen+2E0h, (int index), RET 4; EAX = &element.
    virtual void* lobby_slot_record_005d4af0(void* container, std::int32_t index) = 0;
    // 005D4910 BSP_LobbySlotSwap_HasPair: ECX = screen; AL.
    virtual bool lobby_slot_swap_has_pair_005d4910(void* screen) = 0;
    // 005D5160 (STL instantiation): ECX = screen; EAX = a record whose +4h is a
    // native-string vector.
    virtual void* lobby_slot_names_005d5160(void* screen) = 0;
    // 00427AC0 BSP_NativeStringVector_Size: __fastcall(ECX = vector), EAX.
    virtual std::int32_t native_string_vector_size_00427ac0(void* vector) = 0;

    // ---- checked dword vectors {proxy, first, last, end} (STL, called) ----
    // 004685A0 erase(first, last): ECX = vector, (out*, {container, ptr},
    // {container, ptr}) by value, RET 14h.
    virtual void dword_vector_erase_range_004685a0(void* vector, void* out, void* first_container, void* first,
                                                   void* last_container, void* last) = 0;
    // 00442190 push_back: ECX = vector, (const dword*), RET 4.
    virtual void dword_vector_push_back_00442190(void* vector, const void* value) = 0;
    // 00441F30 insert one: ECX = vector, (out*, {container, ptr}, const dword*), RET 10h.
    virtual void dword_vector_insert_one_00441f30(void* vector, void* out, void* container, void* where,
                                                  const void* value) = 0;

    // ---- front-end screens ----
    // 004F83B0 BSP_FrontEndScreen_CommitVisibility: ECX = screen.
    virtual void frontend_screen_commit_visibility_004f83b0(void* screen) = 0;
    // Screen vtable +18h (enter), __thiscall(ECX = screen), no arguments.
    virtual void screen_vcall_18_enter(void* screen) = 0;

    // ---- movie camera (segment 46) ----
    // 00797070 BSP_MovCam_SetLookTarget: ECX = camera, (unit, int), RET 8.
    virtual void movcam_set_look_target_00797070(void* camera, void* unit, std::int32_t flag) = 0;
    // 00793380 BSP_ReconList_PayloadAt: ECX = list, (int index), RET 4; EAX = node+8h value.
    virtual void* recon_list_payload_at_00793380(void* list, std::int32_t index) = 0;
    // 004BF8E0 BSP_ParentList_ClearRawHeaderStorage: ECX = list (the stack list's destructor).
    virtual void parent_list_clear_004bf8e0(void* list) = 0;

    // ---- HUD map (segment 20) ----
    // 0059CAC0 STL_Vector_Size: ECX = 12-byte-element vector {proxy, first, last}, EAX.
    virtual std::int32_t vector12_size_0059cac0(void* vector) = 0;
    // 0059C9B0 BSP_World_PointBeyondMapMargin: ECX = [00E188A8], (const float* point, float margin), RET 8; AL.
    virtual bool world_point_beyond_map_margin_0059c9b0(void* game, const float* point, float margin) = 0;
    // 004215D0 MSVC_Vector12_PushBack: ECX = vector {first, last, end}, (const float* xyz), RET 4.
    virtual void vector12_push_back_004215d0(void* vector, const float* xyz) = 0;

    // ---- free camera effects (segment 15) ----
    // 004C43C0 BSP_InputAction_WasPressedThisFrame: ECX = [00E188A8], (int action), RET 4; AL.
    virtual bool input_action_was_pressed_004c43c0(void* game, std::int32_t action) = 0;
    // 004BEC00 BSP_InputManager_GetSingleton: EAX.
    virtual void* input_manager_singleton_004bec00() = 0;
    // 0053D300 (checked vector element): ECX = effect+4h, (int index), RET 4; EAX = &record (30h bytes).
    virtual void* free_camera_effect_record_at_0053d300(void* vector, std::int32_t index) = 0;
    // 0053DC70 / 0053DCE0 BSP_FreeCameraEffectEntry_Construct / _Destruct: ECX = entry.
    virtual void free_camera_effect_entry_construct_0053dc70(void* entry) = 0;
    virtual void free_camera_effect_entry_destruct_0053dce0(void* entry) = 0;
    // 0085DC80 BSP_Matrix_OrthonormalizeBasisRows: __fastcall(ECX = float m[16]).
    virtual void matrix_orthonormalize_rows_0085dc80(float* matrix) = 0;
    // (InterlockedIncrement / InterlockedDecrement through the IAT and the
    // refcounted vtable +0h are the cc13_sweepB hooks.)
    // 00868420 BSP_PointEffect_CreateFromMatrix: ECX = out smart pointer, EDX =
    // effect system, stack (definition smart pointer BY VALUE -- the caller adds
    // the reference, the callee owns it --, const float m[16], int, int), RET
    // 10h; EAX = out.
    virtual void* point_effect_create_from_matrix_00868420(void* out, void* system, void* definition,
                                                           const float* matrix, std::int32_t a, std::int32_t b) = 0;
    // 004845D0 BSP_RetainedReference_Assign: ECX = slot, (const slot* source), RET 4.
    virtual void retained_reference_assign_004845d0(void* slot, const void* source) = 0;
    // 00867790 BSP_Effect_UpdateChildren: ECX = effect, (float delta, void* node), RET 8.
    virtual void effect_update_children_00867790(void* effect, float delta, void* node) = 0;
    // 0053E890 STL list buy node: ECX = list, (next, prev, const entry*), RET 0Ch; EAX = node.
    virtual void* free_camera_effect_list_buy_node_0053e890(void* list, void* next, void* prev,
                                                            const void* entry) = 0;
    // 0053E930 STL list size grow (xlen check): ECX = list, (int count), RET 4.
    virtual void free_camera_effect_list_grow_0053e930(void* list, std::int32_t count) = 0;

    // ---- multiplayer main menu / awards (segment 23) ----
    // 00BF85B0 floor (CRT, cdecl double, ST0).
    virtual double crt_floor_00bf85b0(double value) = 0;
    // 00AA7910 BSP_GuiWidget_SetLocalZ: ECX = widget, (float), RET 4.
    virtual void widget_set_local_z_00aa7910(void* widget, float z) = 0;
    // 0050FC30 BSP_AwardTable_FindRecord: ECX = [00E19900]+10h, (name*), RET 4; EAX = record.
    virtual void* award_table_find_record_0050fc30(void* table, void* name) = 0;
    // 0050A210 BSP_AwardRecord_CopyConstruct: ECX = out (54h bytes), (const record*), RET 4.
    virtual void award_record_copy_construct_0050a210(void* out, const void* source) = 0;
    // 00501FA0 BSP_AwardRecord_DestroyActualStorage: ECX = record.
    virtual void award_record_destroy_00501fa0(void* record) = 0;

    // ---- GUI list boxes ----
    // 00A9D750 BSP_GuiListbox_InsertRow: ECX = list box, (row, iterator*, after byte), RET 0Ch.
    virtual void gui_listbox_insert_row_00a9d750(void* listbox, void* row, void* where, std::int32_t after) = 0;
    // 00A9CEB0 BSP_GuiListbox_SetField124: ECX = list box, (float, char relayout), RET 8.
    virtual void gui_listbox_set_field_124_00a9ceb0(void* listbox, float value, std::int32_t relayout) = 0;
    // 00A9AC40 BSP_GuiListbox_SetListener: ECX = list box, (listener), RET 4.
    virtual void gui_listbox_set_listener_00a9ac40(void* listbox, void* listener) = 0;

    // ---- multiplayer mode selector (segment 23) ----
    // 00578320 BSP_SessionBrowserRow_SetConnectionIcons: __fastcall(ECX = group, EDX = value).
    virtual void session_row_set_connection_icons_00578320(void* group, std::int32_t value) = 0;
    // 00ABB000 BSP_GuiText_SetEllipsizedSource: ECX = text, (native string*, float width, int), RET 0Ch.
    virtual void gui_text_set_ellipsized_source_00abb000(void* widget, const void* text, float width,
                                                         std::int32_t localise) = 0;
    // Text widget vtable +7Ch, (int), RET 4; vtable +50h, (const float rgba[4]), RET 4.
    virtual void widget_vcall_7c(void* widget, std::int32_t value) = 0;
    virtual void widget_vcall_50_colour(void* widget, const float* rgba) = 0;

    // ---- in-game HUD markers (segment 26) ----
    // 00852860 BSP_Entity_IsBelowAltitudeThird: ECX = entity; AL.
    virtual bool entity_is_below_altitude_third_00852860(void* entity) = 0;
    // 00638E50 BSP_HudMarker_ProjectAndClip: __fastcall(ECX = const float3*, EDX = float3* out); AL.
    virtual bool hud_marker_project_and_clip_00638e50(const float* world, float* screen) = 0;
    // 0063A6C0 BSP_HudMarker_ProjectUnitScreenBounds: ECX = screen, (float3 by
    // value, unit, char* collapsed, float* min_x, float* min_y, float* max_x,
    // float* max_y), RET 24h.
    virtual void hud_marker_project_unit_bounds_0063a6c0(void* screen, float x, float y, float z, void* unit,
                                                         std::uint8_t* collapsed, float* min_x, float* min_y,
                                                         float* max_x, float* max_y) = 0;
    // 00432650 BSP_GlobalConfig_GetSingleton: EAX.
    virtual void* global_config_00432650() = 0;
    // 00640940 BSP_InGameHudMarkers_AcquireCommandUnitMarker: ECX = screen; EAX = marker.
    virtual void* hud_markers_acquire_command_unit_marker_00640940(void* screen) = 0;

    // 00432050 STL_NativeStringRange_Destroy: ECX = begin, EDX = end, two
    // unused stack arguments (the vector and the owner block), RET 8.
    virtual void native_string_range_destroy_00432050(void* begin, void* end, void* vector, void* owner) = 0;

    // ---- HUD objectives (segment 23) ----
    // 005EDD70 (checked pointer-vector erase range): ECX = vector, (out*,
    // {container, ptr}, {container, ptr}), RET 14h.
    virtual void objective_vector_erase_range_005edd70(void* vector, void* out, void* first_container,
                                                       void* first, void* last_container, void* last) = 0;
    // 005EE3B0 / 005DABA0 (pointer-vector insert one): ECX = vector, (out*,
    // {container, ptr}, const value*), RET 10h.
    virtual void objective_vector_insert_one_005ee3b0(void* vector, void* out, void* container, void* where,
                                                      const void* value) = 0;
    virtual void widget_vector_insert_one_005daba0(void* vector, void* out, void* container, void* where,
                                                   const void* value) = 0;
    // 008DA180 (one-instruction getter, called): ECX = game+21A4h[player]; EAX = the objective manager.
    virtual void* objective_manager_008da180(void* owner) = 0;
    // 008DA140 BSP_Objective_IsShownInHud: __fastcall(ECX = objective); AL.
    virtual bool objective_is_shown_in_hud_008da140(void* objective) = 0;
    // 00A9BEC0 BSP_GuiListbox_ClearRows: ECX = list box.
    virtual void gui_listbox_clear_rows_00a9bec0(void* listbox) = 0;
    // List box vtable +80h, (int), RET 4.
    virtual void widget_vcall_80(void* widget, std::int32_t value) = 0;

    // ---- materials ----
    // 00B179F0 BSP_Material_GetDiffuseColor: ECX = material, (int), RET 4; EAX = float[4] (live).
    virtual float* material_diffuse_color_00b179f0(void* material, std::int32_t slot) = 0;
    // 00B18B00 BSP_Material_RegisterFloat2Parameter: ECX = material, (name*, source*), RET 8.
    virtual void material_register_float2_parameter_00b18b00(void* material, const void* name, void* source) = 0;
    // Renderer vtable +60h, (int count, int, int), RET 0Ch; EAX = a ref-counted index stream.
    virtual void* renderer_vcall_60_create_index_stream(void* renderer, std::int32_t count, std::int32_t a,
                                                        std::int32_t b) = 0;
    // 00B73B70 BSP_Geometry_SetIndexStream: ECX = geometry, (stream), RET 4.
    virtual void geometry_set_index_stream_00b73b70(void* geometry, void* stream) = 0;
    // Index stream vtable +0Ch (0, 0, 0): EAX = std::uint16_t* (locked); +10h: unlock.
    virtual std::uint16_t* index_stream_vcall_0c_lock(void* stream, std::int32_t a, std::int32_t b,
                                                      std::int32_t c) = 0;
    virtual void index_stream_vcall_10_unlock(void* stream) = 0;

    // ---- in-game pause menu (segment 22) ----
    // 00425850 BSP_NativeString_EqualsCStringInsensitive: ECX = string, (const char*), RET 4; AL.
    virtual bool native_string_equals_cstring_ci_00425850(const void* string, const char* text) = 0;
    // 00688AD0 BSP_MultiMenu_PushRequestInterfaceLogged: ECX = [00E198B4], (int), RET 4.
    virtual void multi_menu_push_request_interface_00688ad0(void* menu, std::int32_t interface_id) = 0;
    // 004DB030 BSP_Game_TogglePauseMenu: ECX = game.
    virtual void game_toggle_pause_menu_004db030(void* game) = 0;
    // 00601200 BSP_HudPauseScreen_BuildHintList: __fastcall(ECX = pause screen).
    virtual void hud_pause_build_hint_list_00601200(void* pause) = 0;
    // 005FD890 BSP_HudPauseScreen_ShowPage: ECX = pause screen, (bool, bool), RET 8.
    virtual void hud_pause_show_page_005fd890(void* pause, std::int32_t second_page, std::int32_t multiplayer) = 0;
    // 00425D10 BSP_MenuCommandScreen_GetSingleton: EAX.
    virtual void* menu_command_screen_00425d10() = 0;
    // 00531B00 BSP_MenuPromptScreen_Raise: ECX = menu, ten logical arguments
    // (int type, const string* message, int, callback address, int, const
    // string* title, float, int, string BY VALUE (callee-owned), int), RET 2Ch.
    virtual void menu_prompt_raise_00531b00(void* menu, std::int32_t type, const void* message, std::int32_t a,
                                            std::uint32_t callback, std::int32_t b, const void* title, float c,
                                            std::int32_t d, Cc13SweepBNativeString text, std::int32_t e) = 0;
    // 0041DD20 BSP_NativeString_DestroyStorage: ECX = string.
    virtual void native_string_destroy_storage_0041dd20(void* string) = 0;
    // 00530C20 BSP_FrontEndPrompt_FocusLastNavigation: ECX = menu.
    virtual void frontend_prompt_focus_last_navigation_00530c20(void* menu) = 0;
    // 00771F20 BSP_Session_RequestPlayerSurrender (cc9's): ECX = game+1EF0h, (int, int), RET 8.
    virtual void session_request_player_surrender_00771f20(void* session, std::int32_t player, std::int32_t flag) = 0;
    // 005D39A0 BSP_HudPauseMultiScreen_BuildMenuRows: __fastcall(ECX, EDX = int).
    virtual void hud_pause_multi_build_menu_rows_005d39a0(void* ecx_value, std::int32_t edx_value) = 0;
    // 004BB8A0 BSP_Game_SessionLocalSideCheck: __fastcall(ECX = game); AL.
    virtual bool game_session_local_side_check_004bb8a0(void* game) = 0;
    // 00425E10 BSP_NativeString_Append: ECX = destination, (const string*), RET 4.
    virtual void native_string_append_00425e10(void* dest, const void* source) = 0;
    // Screen vtable +8h, no arguments.
    virtual void screen_vcall_08(void* screen) = 0;

    // ---- unit classes ----
    // Class vtable +18h, (int kind), RET 4; AL.
    virtual bool class_vcall_18_is_kind(void* class_object, std::int32_t kind) = 0;
    // Unit vtable +174h, no arguments; EAX = the current command class
    // singleton (an image address such as 00E08FA0).
    virtual void* unit_vcall_174_command_class(void* unit) = 0;
};

// ---- segment 46 ----

// 007ADD70 BSP_EntityCommand_PathCursorFollowStep(cursor, pos, radius, count,
// heading, factor, force): AL.
bool cc13_sweepB4_path_cursor_follow_step_007add70(void* cursor, const float* pos, float radius,
                                                   std::int32_t count, float heading, float factor,
                                                   bool force, Cc13SweepB4Host& host);

// ---- segment 15: HUD counter rows ----
// Counter entry: +0h..+28h widgets, +2Ch widget, +30h position (x first),
// +3Ch, +54h, +60h floats, +88h/+90h native strings, +98h, +9Ch, +0A0h.

// 00534870 BSP_Hud_UnitOrderIcon(screen): the formation screen's selected-unit
// panel (order icon, name, health bars, payload icon).
void cc13_sweepB4_hud_unit_order_icon_00534870(void* screen, Cc13SweepB4Host& host);

// 00536310 BSP_HudFormationScreen_BuildQuadModel(screen, size).
void cc13_sweepB4_hud_formation_build_quad_model_00536310(void* screen, float size, Cc13SweepB4Host& host);

// 0053EE00 BSP_FreeCameraEffect_Update(effect, dt, camera block).
void cc13_sweepB4_free_camera_effect_update_0053ee00(void* effect, float dt, const void* camera,
                                                     Cc13SweepB4Host& host);

// 0052C1B0 BSP_HudCounterEntry_Construct(entry, root, names[12]): returns
// entry. caller_edi is the caller's EDI, which the image passes to the final
// +0h->+34h call without pushing an argument (see the .cpp).
void* cc13_sweepB4_hud_counter_entry_construct_0052c1b0(void* entry, void* root, const char* const* names,
                                                        std::int32_t caller_edi, Cc13SweepB4Host& host);

// 00529D70 BSP_HudCounter_LayoutTimer(entry, a, b).
void cc13_sweepB4_hud_counter_layout_timer_00529d70(void* entry, Cc13SweepBNativeString a,
                                                    Cc13SweepBNativeString b, Cc13SweepB4Host& host);

// 0052B080 BSP_HudCounter_Tick(entry, seconds).
void cc13_sweepB4_hud_counter_tick_0052b080(void* entry, float seconds, Cc13SweepB4Host& host);

// 0052BB10 BSP_HudCounter_LayoutScore(entry, units {data, count}, text).
void cc13_sweepB4_hud_counter_layout_score_0052bb10(void* entry, const void* units,
                                                    Cc13SweepBNativeString text, Cc13SweepB4Host& host);

// 0052A100 BSP_HudCounter_LayoutValue(entry, a, b, v0, v1); a and b are
// by-value native strings the callee releases.
void cc13_sweepB4_hud_counter_layout_value_0052a100(void* entry, Cc13SweepBNativeString a,
                                                    Cc13SweepBNativeString b, std::int32_t v0,
                                                    std::int32_t v1, Cc13SweepB4Host& host);

// 0052A470 BSP_HudCounter_LayoutTextPair(entry, a, b) (hypothesis).
void cc13_sweepB4_hud_counter_layout_text_pair_0052a470(void* entry, Cc13SweepBNativeString a,
                                                        Cc13SweepBNativeString b, Cc13SweepB4Host& host);

// 0052A860 BSP_HudCounter_LayoutCountdown(entry, text, seconds).
void cc13_sweepB4_hud_counter_layout_countdown_0052a860(void* entry, Cc13SweepBNativeString text,
                                                        float seconds, Cc13SweepB4Host& host);

// ---- segment 19 ----

// 00570300 BSP_MultiSessionBrowserScreen_PrepareEnter(screen).
void cc13_sweepB4_session_browser_prepare_enter_00570300(void* screen, Cc13SweepB4Host& host);

// ---- segment 20 ----

// 005A2730 BSP_HudMap_CreateConeModel(radius, angle): returns the model.
void* cc13_sweepB4_hud_map_create_cone_model_005a2730(float radius, float angle, Cc13SweepB4Host& host);

// 005A2C30 BSP_HudMap_CreateSimpleColorModel(radius): returns the model.
void* cc13_sweepB4_hud_map_create_circle_model_005a2c30(float radius, Cc13SweepB4Host& host);

// 005A1C60 BSP_HudMapIcon_ReleaseWidgets(icon, keep_first).
void cc13_sweepB4_hud_map_icon_release_widgets_005a1c60(void* icon, std::int32_t keep_first, Cc13SweepB4Host& host);

// 005A7550 BSP_HudMapArrowWidget_SetWaypointPath(arrow, map, start, path,
// first_index, curve_arg, texture_index).
void cc13_sweepB4_hud_map_arrow_set_waypoint_path_005a7550(void* arrow, void* map, const float* start,
                                                           void* path, std::int32_t first_index,
                                                           std::int32_t curve_arg, std::int32_t texture_index,
                                                           Cc13SweepB4Host& host);

// 005C4D40 BSP_MissionSideBlock_Destruct(block) (154h-byte side block).
void cc13_sweepB4_mission_side_block_destruct_005c4d40(void* block, Cc13SweepB4Host& host);

// 005987F0 BSP_MainMenuScreen_Enter(screen) (screen vtable +18h).
void cc13_sweepB4_main_menu_enter_005987f0(void* screen, Cc13SweepB4Host& host);

// ---- segment 22 ----

// 005D43C0 BSP_HudPauseMultiScreen_OnConfirm(screen, widget, arg2) (listener slot).
void cc13_sweepB4_hud_pause_multi_on_confirm_005d43c0(void* screen, void* widget, void* arg2,
                                                      Cc13SweepB4Host& host);

// ---- segment 23 ----

// 005EE520 BSP_HudObjectivesScreen_RefreshRows(screen).
void cc13_sweepB4_hud_objectives_refresh_rows_005ee520(void* screen, Cc13SweepB4Host& host);

// 005EBDD0 BSP_MultiModeSelectorScreen_RefreshServerInfo(screen).
void cc13_sweepB4_mode_selector_refresh_server_info_005ebdd0(void* screen, Cc13SweepB4Host& host);

// 005E9BE0 BSP_MultiMainMenuScreen_UpdateSessionList(screen).
void cc13_sweepB4_multi_main_update_session_list_005e9be0(void* screen, Cc13SweepB4Host& host);

// 005E1CF0 BSP_MultiGameLobby_StepSetting(screen, step).
void cc13_sweepB4_lobby_step_setting_005e1cf0(void* screen, std::int32_t step, Cc13SweepB4Host& host);

// ---- segment 25 ----

// 00616C70 BSP_HudPowerupManagerScreen_FillSecondRows(screen).
void cc13_sweepB4_powerup_fill_second_rows_00616c70(void* screen, Cc13SweepB4Host& host);

// ---- segment 26 ----

// 0063C110 BSP_HudUnitMarkerWidgets_Construct(set, root): returns set.
void* cc13_sweepB4_hud_unit_marker_widgets_construct_0063c110(void* set, void* root, Cc13SweepB4Host& host);

// 00642C20 BSP_InGameHudMarkers_AddCommandUnitMarker(screen, unit).
void cc13_sweepB4_hud_markers_add_command_unit_marker_00642c20(void* screen, void* unit, Cc13SweepB4Host& host);

// 00652C80 BSP_Hud_UnitIconFrame_00652C80(screen, class, unused, unused,
// use_unit, unit): the icon frame index.
std::int32_t cc13_sweepB4_hud_unit_icon_frame_00652c80(void* screen, void* class_object, std::int32_t unused2,
                                                       std::int32_t unused3, std::int32_t use_unit, void* unit,
                                                       Cc13SweepB4Host& host);

// 00637080 BSP_HudSceneInitScreen_Enter(screen) (vtable 00CF569C slot +18h).
void cc13_sweepB4_hud_scene_init_enter_00637080(void* screen, Cc13SweepB4Host& host);

// 007A8DA0 BSP_MovCam_StepDeckWalk(camera).
void cc13_sweepB4_movcam_step_deck_walk_007a8da0(void* camera, Cc13SweepB4Host& host);

// ---- segment 27 ----

// 00671D60 BSP_HudSupportManager_ShowSelectionInfo(screen).
void cc13_sweepB4_hud_support_show_selection_info_00671d60(void* screen, Cc13SweepB4Host& host);

}  // namespace bsp
