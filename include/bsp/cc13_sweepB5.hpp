#pragma once

// cc13_sweepB5: fifth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB5, docs/CC13_SWEEPB5.md). Every name is a hypothesis, not a
// recovered symbol. These are new C++ interfaces over the native records, not
// drop-in replacements of the original ABI. Nothing here is wired into the game.
//
// Same conventions as cc13_sweepB..B4: native objects are raw byte blocks
// touched only at the offsets the image reads or writes, and every call the
// image makes goes through a named host method at the same point with the same
// arguments. Cc13SweepB5Host extends Cc13SweepB4Host, so the earlier hooks are
// reused as they are.

#include "bsp/cc13_sweepB4.hpp"

#include <cstdint>

namespace bsp {

class Cc13SweepB5Host : public Cc13SweepB4Host {
public:
    // ---- guns and aiming (segment 16 weapon-group sight) ----
    // 00729F10 BSP_Gun_IsOperational: __fastcall(ECX = gun), AL.
    virtual bool gun_is_operational_00729f10(void* gun) = 0;
    // 00729A40 BSP_Gun_AnyBarrelTimerBelowThreshold: __fastcall(ECX = gun), AL.
    virtual bool gun_any_barrel_timer_below_threshold_00729a40(void* gun) = 0;
    // Unit vtable +50h: __thiscall(ECX = unit), no stack arguments, ST0 (a heading).
    virtual double unit_vcall_50_heading(void* unit) = 0;
    // Unit vtable +20h: __thiscall(ECX = unit), no stack arguments, EAX (the
    // entity the segment query excludes; 006D1E30 returns [unit+360h]).
    virtual void* unit_vcall_20_query_exclude(void* unit) = 0;
    // 0085AB50 BSP_TurningGun_FilterHeadingToTraverse: __thiscall(ECX = gun,
    // float heading, float limit), RET 8, ST0.
    virtual double turning_gun_filter_heading_0085ab50(void* gun, float heading, float limit) = 0;
    // 00521370 BSP_Direction_ToPitchYaw: __fastcall(ECX = row, EDX = out_pitch,
    // stack out_yaw), RET 4.
    virtual void direction_to_pitch_yaw_00521370(const float* row, float* out_pitch, float* out_yaw) = 0;
    // 00957BD0 (cc9 lane, not reconstructed): __fastcall(ECX = unit, EDX = gun,
    // stack: float x, y, z (by value), float yaw, float pitch, void* target,
    // float* out_point), RET 1Ch. The return value is not read by 00547480.
    virtual void unit_gun_aim_point_00957bd0(void* unit, void* gun, float x, float y, float z, float yaw,
                                             float pitch, void* target, float* out_point) = 0;
    // 00955830 BSP_Device_AimDirectionToLocalPair: __fastcall(ECX = device,
    // EDX = const float[3] target, stack out_a, out_b), RET 8. Return not read.
    virtual void device_aim_direction_to_local_pair_00955830(void* device, const float* target, float* out_a,
                                                             float* out_b) = 0;
    // 004B4D80 BSP_Lighting_DirectionFromAngles: __thiscall(ECX = float out[3],
    // float a, float b), RET 8, EAX = out.
    virtual void direction_from_angles_004b4d80(float* out, float a, float b) = 0;
    // 00957740 BSP_Aim_ResolveRayToWorldPoint: __fastcall(ECX = unit, EDX =
    // const float[3] origin, stack direction, out, device), RET 0Ch. Return not read.
    virtual void aim_resolve_ray_to_world_point_00957740(void* unit, const float* origin, const float* direction,
                                                         float* out, void* device) = 0;
    // 00955630 BSP_Gun_SolveGravityArc: __fastcall(ECX = unit, EDX = const
    // float[3] aim, stack const float[3] muzzle, float speed, float* out_pitch,
    // float* out_yaw), RET 10h, AL.
    virtual bool gun_solve_gravity_arc_00955630(void* unit, const float* aim, const float* muzzle, float speed,
                                                float* out_pitch, float* out_yaw) = 0;
    // 00419510 BSP_Vector3f_Normalize: __fastcall(ECX = out, EDX = in), EAX = out.
    virtual float* vector3_normalize_00419510(float* out, const float* in) = 0;
    // 0042E630 BSP_SpatialIndex_GetSingleton: __cdecl(), EAX.
    virtual void* spatial_index_singleton_0042e630() = 0;
    // 0098ADD0 BSP_SpatialIndex_QuerySegment: __thiscall(ECX = index, from, to,
    // exclude, record, kind), RET 14h, AL.
    virtual bool spatial_index_query_segment_0098add0(void* index, const float* from, const float* to,
                                                      void* exclude, void* record, std::int32_t kind) = 0;
    // 005472D0 TMPL_RefPtrArray_AtGrow: __thiscall(ECX = array, int index),
    // RET 4, EAX = the slot address (grows the array when needed).
    virtual void** ref_ptr_array_at_grow_005472d0(void* array, std::int32_t index) = 0;
    // 007F60A0 BSP_GunPlatform_AnglesInFireWindow: __thiscall(ECX = platform,
    // float horz, float vert), RET 8, AL.
    virtual bool gun_platform_angles_in_fire_window_007f60a0(void* platform, float horz, float vert) = 0;

    // ---- STL instantiations (called, not reconstructed) ----
    // 005A7A10 std::vector<void*>::push_back: __thiscall(ECX = vector {+0
    // proxy, +4 first, +8 last, +Ch end}, const value*), RET 4.
    virtual void pointer_vector_push_back_005a7a10(void* vector, void* const* value) = 0;
    // 005A6650 std::vector<void*>::insert: __thiscall(ECX = vector, iterator*
    // out, iterator {container, where} by value, const value*), RET 10h.
    virtual void pointer_vector_insert_005a6650(void* vector, void* out_iterator, void* container, void** where,
                                                void* const* value) = 0;

    // ---- front end (segment 23 options screens) ----
    // 005189B0 BSP_FrontEndFrame_SetTitle: ECX = const native string*, no
    // stack arguments.
    virtual void front_end_frame_set_title_005189b0(const void* title) = 0;
    // 0054A0C0 BSP_MainMenuCommandBar_SetHelpLine: __thiscall(ECX = command
    // owner, const native string*), RET 4.
    virtual void main_menu_command_bar_set_help_line_0054a0c0(void* owner, const void* text) = 0;
    // 00A9CDC0 BSP_GuiListbox_Clear: __fastcall(ECX = listbox).
    virtual void gui_listbox_clear_00a9cdc0(void* listbox) = 0;
    // 005F0CD0 BSP_OptionsScreen_RefreshHelpLine: __fastcall(ECX = screen).
    virtual void options_screen_refresh_help_line_005f0cd0(void* screen) = 0;
    // 005F2D20 BSP_OptionsScreen_RefreshContentRows: __fastcall(ECX = screen).
    virtual void options_screen_refresh_content_rows_005f2d20(void* screen) = 0;
    // Widget vtable +84h: __thiscall(ECX = widget, int), RET 4.
    virtual void widget_vcall_84(void* widget, std::int32_t value) = 0;
    // Listbox listener vtable +8h: __thiscall(ECX = listener, row value,
    // listbox), RET 8 (the selection-changed notification).
    virtual void listbox_listener_vcall_08_selected(void* listener, void* value, void* listbox) = 0;

    // ---- HUD support manager (segment 27) ----
    // Widget vtable +54h: __thiscall(ECX = widget, float out[4]), RET 4, EAX =
    // out (the colour, returned through the hidden pointer; alpha at +0Ch).
    virtual const float* widget_vcall_54_colour(void* widget, float* out) = 0;
    // 0065A2B0 BSP_HudSupportManager_SyncCursorIterators: __fastcall(ECX = screen).
    virtual void hud_support_sync_cursor_iterators_0065a2b0(void* screen) = 0;
    // 00658580 BSP_HudSupportManagerScreen_WorldToMap: __thiscall(ECX =
    // screen, float out[2], float x, float y, float z), RET 10h.
    virtual void hud_support_world_to_map_00658580(void* screen, float* out, float x, float y, float z) = 0;
    // The unit -> widget std::map at screen+3FCh (node +0Ch key, +10h value)
    // and the unit std::list at screen+1ECh (STL instantiations, called):
    // 00654650 tree iterator ++: __thiscall(ECX = iterator {container, node}).
    virtual void tree_iterator_increment_00654650(void* iterator) = 0;
    // 00659810 tree _Erase(subtree): __thiscall(ECX = tree, node), RET 4.
    virtual void tree_erase_subtree_00659810(void* tree, void* node) = 0;
    // 00662750 map erase(where): __thiscall(ECX = map, iterator* out,
    // iterator {container, node} by value), RET 0Ch.
    virtual void tree_erase_at_00662750(void* tree, void* out_iterator, void* container, void* node) = 0;
    // 00667E30 map operator[]: __thiscall(ECX = map, const key*), RET 4, EAX = &value.
    virtual void** unit_widget_map_subscript_00667e30(void* map, void* const* key) = 0;
    // 00659C30 list _Buynode: __thiscall(ECX = list, next, prev, const value*), RET 0Ch, EAX = node.
    virtual void* unit_list_buy_node_00659c30(void* list, void* next, void* prev, void* const* value) = 0;
    // 00663050 list _Incsize: __thiscall(ECX = list, count), RET 4 (throws
    // length_error when the size would pass 3FFFFFFFh).
    virtual void unit_list_incsize_00663050(void* list, std::uint32_t count) = 0;

    // ---- mission score record members (segment 20; STL copy constructors,
    // called, not reconstructed): each __thiscall(ECX = dst, const src*), RET 4.
    virtual void tree_copy_construct_00590080(void* dst, const void* src) = 0;
    virtual void tree_copy_construct_00593310(void* dst, const void* src) = 0;
    virtual void tree_copy_construct_00593480(void* dst, const void* src) = 0;
    virtual void tree_copy_construct_00591d90(void* dst, const void* src) = 0;
    virtual void tree_copy_construct_0058d4b0(void* dst, const void* src) = 0;
    virtual void tree_copy_construct_0058d540(void* dst, const void* src) = 0;
    virtual void tree_copy_construct_0058d5d0(void* dst, const void* src) = 0;
    virtual void tree_copy_construct_0058d660(void* dst, const void* src) = 0;
    virtual void tree_copy_construct_0058d6f0(void* dst, const void* src) = 0;
    virtual void tree_copy_construct_0058f370(void* dst, const void* src) = 0;
    // 0058AAC0: a std::vector of 2-byte elements, copy constructor.
    virtual void vector_copy_construct_0058aac0(void* dst, const void* src) = 0;
    // 0058B870: the 10h-byte member at +14Ch, copy constructor.
    virtual void member_copy_construct_0058b870(void* dst, const void* src) = 0;

    // ---- HUD map (segment 20) ----
    // 00B646E0 BSP_Matrix_BuildRotationY: __fastcall(ECX = float out[16], EDX
    // = const float* angle), EAX = out.
    virtual float* matrix_build_rotation_y_00b646e0(float* out, const float* angle) = 0;
    // 008DA130 BSP_Objective_IsActive: __fastcall(ECX = objective), AL.
    virtual bool objective_is_active_008da130(void* objective) = 0;
    // 008DACD0 BSP_ObjectiveEntry_IsRevealedToLocalSide (provisional):
    // __fastcall(ECX = entry), AL.
    virtual bool objective_entry_is_revealed_008dacd0(void* entry) = 0;
    // Unit vtable +140h: __thiscall(ECX = unit), EAX (the value 005A22D0 looks up).
    virtual void* unit_vcall_140(void* unit) = 0;
    // Unit vtable +0C4h: __thiscall(ECX = unit, float out[3]), RET 4, EAX = the position.
    virtual const float* unit_vcall_c4_position(void* unit, float* out) = 0;
    // 005A7A80 std::vector<void*>::push_back: __thiscall(ECX = vector, const value*), RET 4.
    virtual void pointer_vector_push_back_005a7a80(void* vector, void* const* value) = 0;
    // Marker sub-node vtable +34h: __thiscall(ECX = node, const float frame[16]), RET 4.
    virtual void marker_node_vcall_34_set_frame(void* node, const float* frame) = 0;
    // 00A7D120 BSP_SoundManager_PushPopSpeechDuck: __thiscall(ECX = manager
    // [00F8BBD8], char push), RET 4.
    virtual void sound_manager_push_pop_speech_duck_00a7d120(void* manager, std::uint8_t push) = 0;
    // 00B172D0 BSP_Effect_GetName: __fastcall(ECX = effect), EAX = const native string*.
    virtual const Cc13SweepBNativeString* effect_get_name_00b172d0(void* effect) = 0;
    // 0059E5E0 BSP_PointerArray_Reserve: __thiscall(ECX = array {+0 data,
    // +4 count, +8 capacity}, int capacity), RET 4.
    virtual void pointer_array_reserve_0059e5e0(void* array, std::int32_t capacity) = 0;
    // 0059FCE0 BSP_PointerArray_Resize: __thiscall(ECX = array, int count), RET 4.
    virtual void pointer_array_resize_0059fce0(void* array, std::int32_t count) = 0;
    // 005BCA30 BSP_HudPanel_SetVisible3: __thiscall(ECX = panel, int flag), RET 4.
    virtual void hud_panel_set_visible3_005bca30(void* panel, std::int32_t flag) = 0;
    // 00B6DFA0 BSP_Node_UnlinkAndRelease: __fastcall(ECX = node).
    virtual void node_unlink_and_release_00b6dfa0(void* node) = 0;
    // 00529490 / 00529350 BSP_HudCounters_RestoreHiddenA8 / A6: __fastcall(ECX = counters).
    virtual void hud_counters_restore_hidden_a8_00529490(void* counters) = 0;
    virtual void hud_counters_restore_hidden_a6_00529350(void* counters) = 0;
    // 006952A0 BSP_Observer_UnregisterPair: __fastcall(ECX = first endpoint,
    // EDX = callback owner).
    virtual void observer_unregister_pair_006952a0(void* first, void* owner) = 0;
    // 00535110 tree iterator ++ (ECX = iterator {container, node}) and
    // 005356E0 tree _Erase(subtree) (ECX = tree, node, RET 4): STL, called.
    virtual void tree_iterator_increment_00535110(void* iterator) = 0;
    virtual void tree_erase_subtree_005356e0(void* tree, void* node) = 0;
    // 005A3AA0 BSP_HudMapScreen_DeleteMarkerObjects: __fastcall(ECX = screen).
    virtual void hud_map_delete_marker_objects_005a3aa0(void* screen) = 0;
    // 005A09A0 BSP_HudMapScreen_ReleaseIconArray: __fastcall(ECX = screen+544h).
    virtual void hud_map_release_icon_array_005a09a0(void* array) = 0;
    // 00772A40 BSP_Session_ClientSendLocalScore: __fastcall(ECX = game+1EF0h).
    virtual void session_client_send_local_score_00772a40(void* session) = 0;
    // 0054E610 BSP_HudHintsScreen_Dismiss: __fastcall(ECX = hints screen).
    virtual void hud_hints_dismiss_0054e610(void* screen) = 0;

    // ---- HUD map refresh (segment 20, 005B2D70) ----
    // 00B6E750 BSP_Node_FindChildByName: __thiscall(ECX = node, const native
    // string*, char recursive), RET 8, EAX.
    virtual void* node_find_child_by_name_00b6e750(void* node, const void* name, std::uint8_t recursive) = 0;
    // 005B2CB0 BSP_HudMapScreen_GetOrCreateLayer: __thiscall(ECX = screen, unit), RET 4.
    virtual void hud_map_get_or_create_layer_005b2cb0(void* screen, void* unit) = 0;
    // Unit vtable +0B4h: __thiscall(ECX = unit), AL.
    virtual bool unit_vcall_b4(void* unit) = 0;
    // 008DDF90 BSP_SzurkeNyil_ContainsUnit: __thiscall(ECX = list owner, unit), RET 4, AL.
    virtual bool objective_owner_contains_unit_008ddf90(void* owner, void* unit) = 0;
    // 0059D940 BSP_HudMapScreen_IsUnitShownAtZoom: __thiscall(ECX = screen, unit), RET 4, AL.
    virtual bool hud_map_is_unit_shown_at_zoom_0059d940(void* screen, void* unit) = 0;
    // 005ADDA0 BSP_HudMapScreen_UpdateUnitIcon (not reconstructed here):
    // __thiscall(ECX = screen, icon), RET 4.
    virtual void hud_map_update_unit_icon_005adda0(void* screen, void* icon) = 0;
    // 005A9DB0 BSP_HudMapScreen_BuildUnitOrderOverlays (not reconstructed
    // here): __thiscall(ECX = screen, unit, int), RET 8.
    virtual void hud_map_build_unit_order_overlays_005a9db0(void* screen, void* unit, std::int32_t flag) = 0;
    // 0059F9B0 BSP_HudMapScreen_HideUnusedArrowWidgets: __fastcall(ECX = screen).
    virtual void hud_map_hide_unused_arrow_widgets_0059f9b0(void* screen) = 0;
    // 005A2650 BSP_HudMapScreen_SetSpeakingUnit: __thiscall(ECX = screen, unit, int, int), RET 0Ch.
    virtual void hud_map_set_speaking_unit_005a2650(void* screen, void* unit, std::int32_t a, std::int32_t b) = 0;
    // STL (called): 00537F20 map erase(where) __thiscall(ECX = map, iterator*
    // out, iterator by value), RET 0Ch; 0059F160 tree iterator ++ (ECX =
    // iterator); 0059FDB0 tree _Erase(subtree) (ECX = tree, node), RET 4.
    virtual void tree_erase_at_00537f20(void* tree, void* out_iterator, void* container, void* node) = 0;
    virtual void tree_iterator_increment_0059f160(void* iterator) = 0;
    virtual void tree_erase_subtree_0059fdb0(void* tree, void* node) = 0;

    // ---- multiplayer error screen (segment 22, 005D26D0) ----
    // 00AA0F70 BSP_GuiManager_ResetScreens: __fastcall(ECX = manager).
    virtual void gui_manager_reset_screens_00aa0f70(void* manager) = 0;
    // 00531030 BSP_NativeString_ConstructByte: __thiscall(ECX = out, char), RET 4, EAX = out.
    virtual void* native_string_construct_byte_00531030(void* out, std::int8_t value) = 0;
    // 004C5E60 BSP_WideString_ConstructFromCString: __thiscall(ECX = out, const char*), RET 4.
    virtual void wide_string_construct_004c5e60(void* out, const char* text) = 0;
    // 00531130 BSP_TextContext_SetGlyphReference: __thiscall(ECX = text,
    // wide wrapper*, listener, reference text), RET 0Ch.
    virtual void text_context_set_glyph_reference_00531130(void* text, const void* wide, void* listener,
                                                           void* reference) = 0;
    // 00AB80C0 BSP_TextContext_ClearGlyphChildren: __fastcall(ECX = text).
    virtual void text_context_clear_glyph_children_00ab80c0(void* text) = 0;
    // 00A9BF90 BSP_GuiListbox_ClearItems: __fastcall(ECX = listbox).
    virtual void gui_listbox_clear_items_00a9bf90(void* listbox) = 0;
    // 00A9AC70 BSP_GuiListbox_SetInputFilter: __thiscall(ECX = listbox, filter code address), RET 4.
    virtual void gui_listbox_set_input_filter_00a9ac70(void* listbox, std::uint32_t filter) = 0;
    // Listbox vtable +40h: __thiscall(ECX = listbox, float), RET 4.
    virtual void widget_vcall_40_float(void* widget, float value) = 0;
    // 005D24F0 BSP_MultiErrorScreen_ShowMessage: __fastcall(ECX = screen).
    virtual void multi_error_show_message_005d24f0(void* screen) = 0;
    // Command owner vtable +1Ch: __thiscall(ECX = owner), no arguments.
    virtual void command_owner_vcall_1c(void* owner) = 0;

    // ---- rewards screen (segment 26) ----
    // 00A3EB00 BSP_XenonSystemManager_GetActiveXuid: __fastcall(ECX =
    // [00F8ABE8]), EDX:EAX.
    virtual std::uint64_t xenon_active_xuid_00a3eb00(void* manager) = 0;
    // 0061D710: __fastcall(ECX = rewards screen), EDX:EAX (the selected row's
    // 64-bit id).
    virtual std::uint64_t rewards_selected_xuid_0061d710(void* screen) = 0;
    // 0061D8B0 BSP_RewardsScreen_IsSelectedEntryFlagged: __fastcall(ECX = screen), AL.
    virtual bool rewards_selected_entry_flagged_0061d8b0(void* screen) = 0;

    // ---- multiplayer chat screen (segment 22) ----
    // 005B66D0 BSP_InGameInterfaceManager_TransitionPending: __fastcall(ECX = manager), AL.
    virtual bool interface_transition_pending_005b66d0(void* manager) = 0;
    // 004BA6D0 BSP_InputBackend_GetDeviceByClassAndIndex (provisional):
    // __thiscall(ECX = [00F8BBF4], int, int), RET 8, EAX = device.
    virtual void* input_backend_device_004ba6d0(void* backend, std::int32_t device_class, std::int32_t index) = 0;
    // Input device vtable +20h: __thiscall(ECX = device, int key), RET 4, AL (key down).
    virtual bool input_device_vcall_20_key_down(void* device, std::int32_t key) = 0;
    // 005D11F0 BSP_MultiChatScreen_SetChatMode: __thiscall(ECX = screen, int mode), RET 4.
    virtual void multi_chat_set_mode_005d11f0(void* screen, std::int32_t mode) = 0;
    // 004D6480 BSP_GuiScreens_ParkWidgets: (std::vector<int>* kept, char park),
    // RET 8; the caller loads ECX with [00E188A8], which is not read.
    virtual void gui_screens_park_widgets_004d6480(void* kept, std::uint8_t park) = 0;
    // 00A966E0 BSP_TextEditor_SetActive: __thiscall(ECX = editor, char active, int row), RET 8.
    virtual void text_editor_set_active_00a966e0(void* editor, std::uint8_t active, std::int32_t row) = 0;
    // 00A96F40 BSP_TextOwner_DispatchQueuedEvents: __thiscall(ECX = owner,
    // float (unused by the callee)), RET 4.
    virtual void text_owner_dispatch_queued_events_00a96f40(void* owner, float dt) = 0;
    // 005D0C60 BSP_MultiChatScreen_RebuildMessageRows: __thiscall(ECX =
    // screen, log, text widget), RET 8.
    virtual void multi_chat_rebuild_message_rows_005d0c60(void* screen, void* log, void* widget) = 0;
    // 00AA83A0 BSP_GuiWidget_DetachChild: __thiscall(ECX = parent, child), RET 4.
    virtual void gui_widget_detach_child_00aa83a0(void* parent, void* child) = 0;
    // std::list iterator helpers (STL, called): 00519E00 ++ (ECX = iterator
    // {container, node}, EAX = iterator); 00425AC0 / 005CF010 operator* (EAX =
    // &node->value); 005CF8A0 erase: __thiscall(ECX = list, iterator* out,
    // iterator {container, node} by value), RET 0Ch, EAX = out.
    virtual void list_iterator_increment_00519e00(void* iterator) = 0;
    virtual void** list_iterator_value_00425ac0(void* iterator) = 0;
    virtual float* list_iterator_value_005cf010(void* iterator) = 0;
    virtual void** chat_list_erase_005cf8a0(void* list, void** out_iterator, void* container, void* node) = 0;

    // ---- frame statistics overlay (segment 15, 0053BCC0) ----
    // 0053BAB0 BSP_FrameStats_Printf and 004F9A80 BSP_FormatString128: cdecl
    // varargs (buffer, format, ...), the caller cleans the stack.
    virtual void frame_stats_printf_0053bab0(char* buffer, const char* format, ...) = 0;
    virtual void format_string128_004f9a80(char* buffer, const char* format, ...) = 0;
    // 00BF97CB strcat_s (CRT, called): cdecl(dest, size, source), EAX.
    virtual std::int32_t crt_strcat_s_00bf97cb(char* dest, std::uint32_t size, const char* source) = 0;
    // 0053BB40 BSP_FrameStats_ResetAccumulators: __fastcall(ECX = stats record).
    virtual void frame_stats_reset_accumulators_0053bb40(void* stats) = 0;
    // 00B1FEF0 BSP_D3D9Renderer_GetDevice: __fastcall(ECX = [00F8D394]), EAX.
    virtual void* renderer_get_device_00b1fef0(void* renderer) = 0;
    // Device COM vtable +10h (stdcall, this on the stack): EAX = available
    // texture memory in bytes.
    virtual std::uint32_t device_vcall_10_available_texture_mem(void* device) = 0;
    // [0109CEFC] vtable +4h: __thiscall(ECX = object), EAX (the memory in use, bytes).
    virtual std::int32_t memory_tracker_vcall_04_in_use(void* tracker) = 0;

    // ---- control layout screen (segment 15) ----
    // 004C9680 BSP_FrontEndFrame_SelectDefault: no register or stack inputs.
    virtual void frontend_frame_select_default_004c9680() = 0;
    // 00A9CF30 BSP_GuiListbox_ResetHighlight: __thiscall(ECX = listbox, char
    // apply, int argument), RET 8.
    virtual void gui_listbox_reset_highlight_00a9cf30(void* listbox, std::uint8_t apply, std::int32_t argument) = 0;
};

// ---------------------------------------------------------------------------
// Segment 15
// ---------------------------------------------------------------------------

// 00528240 BSP_ControlLayoutScreen_Enter: the control-layout options page
// (three layout rows, two 3x3 text grids, the command bar).
void cc13_sweepB5_control_layout_screen_enter_00528240(void* screen, Cc13SweepB5Host& host);

// 0053BCC0 BSP_FrameStats_FormatLines: formats the frame-statistics overlay
// (full mode +8 == 1, or the compact single line with the network counters).
void cc13_sweepB5_frame_stats_format_lines_0053bcc0(void* stats, float rate, Cc13SweepB5Host& host);

// ---------------------------------------------------------------------------
// Segment 16
// ---------------------------------------------------------------------------

// 00547480: the weapon-group sight status of one gun (0 disabled flag, 1 not
// operational, 2 no solution / blocked, 3 off bearing, 4 on bearing, 5 barrels
// not ready).
std::int32_t cc13_sweepB5_weapon_group_gun_sight_status_00547480(void* screen, void* unit, void* gun,
                                                                 std::int32_t mode, const float* origin,
                                                                 float aim_a, float aim_b,
                                                                 Cc13SweepB5Host& host);

// ---------------------------------------------------------------------------
// Segment 19
// ---------------------------------------------------------------------------

// 0056DD80 BSP_MultiSessionBrowserScreen_RefreshFilterRows: the nine filter
// rows (screen+54h + 4*i): tick icons from the flags screen+10h+i, the three
// numeric settings +244h/+248h/+24Ch or "globals.no".
void cc13_sweepB5_session_browser_refresh_filter_rows_0056dd80(void* screen, Cc13SweepB5Host& host);

// ---------------------------------------------------------------------------
// Segment 20
// ---------------------------------------------------------------------------

// 00593CA0 BSP_MissionScoreRecord_Assign (a member-wise copy constructor):
// returns dst.
void* cc13_sweepB5_mission_score_record_copy_00593ca0(void* dst, const void* src, Cc13SweepB5Host& host);

// 005A4200 BSP_HudMapScreen_Exit: restores the terrain bush/tree visibility,
// releases the map's marker models and pools, and resets the map state.
void cc13_sweepB5_hud_map_screen_exit_005a4200(void* screen, Cc13SweepB5Host& host);

// 005B2D70 BSP_HudMapScreen_LoadBackground (ledger name; the body refreshes
// the whole map: background layer, unit layers and icons, order overlays,
// model visibility, the speaking unit).
void cc13_sweepB5_hud_map_load_background_005b2d70(void* screen, Cc13SweepB5Host& host);

// 005B25D0 BSP_HudMapScreen_UpdateMarkerModels: rebuilds the objective
// marker models (+57Ch vector) from the local side's revealed objective
// entries, advances the spin angles by dt (unless paused) and places every
// marker.
void cc13_sweepB5_hud_map_update_marker_models_005b25d0(void* screen, float dt, std::uint8_t paused,
                                                        Cc13SweepB5Host& host);

// 005A91E0 BSP_HudMap_BuildCaptureRadiusModel: takes the next pooled radius
// model (screen+4B4h vector, index screen+4C4h), sets its four radius
// parameters and circle position, places it and advances the index.
void cc13_sweepB5_hud_map_build_capture_radius_model_005a91e0(void* screen, float scale, const float* xz,
                                                              float height, float land_radius,
                                                              float capture_radius, float band,
                                                              Cc13SweepB5Host& host);

// 005A9770 BSP_HudMapScreen_AcquireConeModel: takes the next pooled cone
// model (screen+4DCh vector, index screen+4ECh), places it and advances the
// index. Layer 0Bh stretches the cone along z around +/-range*250000.
void cc13_sweepB5_hud_map_acquire_cone_model_005a9770(void* screen, const float* frame, float heading,
                                                      float scale, float stretch, float range,
                                                      float cone_angle, std::uint32_t layer,
                                                      Cc13SweepB5Host& host);

// ---------------------------------------------------------------------------
// Segment 22
// ---------------------------------------------------------------------------

// 005D18A0 BSP_MultiChatScreen_Update: the chat screen's per-frame update
// (open/close by input, the text editor, the message rows and their fade).
void cc13_sweepB5_multi_chat_screen_update_005d18a0(void* screen, float unused, Cc13SweepB5Host& host);

// 005D26D0 BSP_MultiErrorScreen_Enter: binds the dialog widgets, sets the
// glyph references and option texts and shows the current message.
void cc13_sweepB5_multi_error_screen_enter_005d26d0(void* screen, Cc13SweepB5Host& host);

// ---------------------------------------------------------------------------
// Segment 23
// ---------------------------------------------------------------------------

// 005F3B50 BSP_OptionsScreen_BuildContentPage: the downloaded-content page
// (one cloned row per 00F8A304 entry).
void cc13_sweepB5_options_screen_build_content_page_005f3b50(void* screen, Cc13SweepB5Host& host);

// ---------------------------------------------------------------------------
// Segment 26
// ---------------------------------------------------------------------------

// 00621C60 BSP_RewardsScreen_RefreshPage: shows page `page` (title, the six
// page widgets) and rebuilds the command bar for it.
void cc13_sweepB5_rewards_screen_refresh_page_00621c60(void* screen, std::uint32_t page, Cc13SweepB5Host& host);

// ---------------------------------------------------------------------------
// Segment 27
// ---------------------------------------------------------------------------

// 0065FBF0 BSP_HudSupportManagerScreen_UpdateSlotHighlights: fades the slot
// highlight widgets of the list at screen+15Ch in by dt * 4 and clears
// screen+2B0h when the walk ends on a fully lit slot (or the list is empty).
void cc13_sweepB5_hud_support_update_slot_highlights_0065fbf0(void* screen, float dt, Cc13SweepB5Host& host);

// 00660B10 BSP_HudSupportManagerScreen_BeginScrollForward: shifts the four
// slot widget arrays by one (down, or up when +154h is set) and clones a new
// slot from the template +138h into the freed end.
void cc13_sweepB5_hud_support_begin_scroll_forward_00660b10(void* screen, Cc13SweepB5Host& host);

// 00661240 BSP_HudSupportManagerScreen_BeginScrollBack: the mirror of
// 00660B10 over the second slot group (+0ACh/+0BCh/+0DCh).
void cc13_sweepB5_hud_support_begin_scroll_back_00661240(void* screen, Cc13SweepB5Host& host);

// 00668770: rebuilds the support-manager unit list (+1ECh) and the unit ->
// marker widget map (+3FCh) from the game's unit list, then places every
// marker on the map.
void cc13_sweepB5_hud_support_rebuild_unit_markers_00668770(void* screen, Cc13SweepB5Host& host);

}  // namespace bsp
