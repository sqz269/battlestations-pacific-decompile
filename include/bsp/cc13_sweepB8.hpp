#pragma once

// cc13_sweepB8: eighth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB8, docs/CC13_SWEEPB8.md). Every name is a hypothesis, not a
// recovered symbol. These are new C++ interfaces over the native records, not
// drop-in replacements of the original ABI. Nothing here is wired into the
// game.
//
// Same conventions as cc13_sweepB..B7 and cc13_pkt642: native objects are raw
// byte blocks touched only at the offsets the image reads or writes, and every
// call the image makes goes through a named host method at the same point with
// the same arguments. Cc13SweepB8Host extends Cc13SweepB7Host, so the earlier
// hooks are reused as they are.

#include "bsp/cc13_sweepB7.hpp"

#include <cstdint>

namespace bsp {

class Cc13SweepB8Host : public Cc13SweepB7Host {
public:
    // ---- HUD map order overlays (segment 20, 005A9DB0) ----
    // Vtable slot +4h of the sub-object unit+1E4h, __thiscall(ECX = unit+1E4h),
    // no arguments; EAX = the row index into the owner's 60h-byte records.
    virtual std::int32_t unit_sub_1e4_vcall_4_index(void* sub_object) = 0;
    // Unit vtable +C8h, __thiscall(ECX = unit), no arguments; float in ST0
    // (the heading handed to the cone model).
    virtual float unit_vcall_c8_heading(void* unit) = 0;
    // Unit vtable +30h, __thiscall(ECX = unit), no arguments; EAX.
    virtual void* unit_vcall_30(void* unit) = 0;
    // Command director vtable +1Ch, no arguments; EAX (the leader unit or null).
    virtual void* director_vcall_1c(void* director) = 0;
    // Command director vtable +20h(int slot), RET 4; AL (slot in use).
    virtual bool director_vcall_20_slot_active(void* director, std::int32_t slot) = 0;
    // Command director vtable +2Ch, no arguments; EAX.
    virtual void* director_vcall_2c(void* director) = 0;
    // 0071BE60 BSP_WeaponDirector_FilledSlotCount, __fastcall(ECX = director); EAX.
    virtual std::int32_t director_filled_slot_count_0071be60(void* director) = 0;
    // 0071BDE0 BSP_WeaponDirector_SlotHasMoveOnPathPath, __thiscall(ECX =
    // director, int slot), RET 4; EAX (the path holder or null).
    virtual void* director_slot_path_0071bde0(void* director, std::int32_t slot) = 0;
    // Path holder vtable +14h, no arguments; EAX (signed first point index).
    virtual std::int32_t path_holder_vcall_14_index(void* holder) = 0;
    // Path holder vtable +8h, no arguments; EAX (the 12-byte point vector).
    virtual void* path_holder_vcall_8_points(void* holder) = 0;
    // 0059CAE0 STL_inst_0059cae0 (vector<float[3]> checked copy-out),
    // __thiscall(ECX = vector, float* out, unsigned index), RET 8; EAX = out.
    // The bound failure calls 00BF6713 inside it.
    virtual const float* vector12_copy_at_0059cae0(void* vector, float* out, std::int32_t index) = 0;
    // 005A4A00 BSP_HudMapScreen_AcquireArrowWidget, __fastcall(ECX = screen); EAX.
    virtual void* hud_map_acquire_arrow_widget_005a4a00(void* screen) = 0;
    // 0059DD20 BSP_HudMapArrowWidget_SetLine, __thiscall(ECX = arrow; screen,
    // const float* from, const float* to, float width), RET 10h.
    virtual void hud_map_arrow_set_line_0059dd20(void* arrow, void* screen, const float* from, const float* to,
                                                 float width) = 0;
    // 005A1800 / 005A16F0 STL_Tree_LowerBoundPair (std::map lower_bound
    // instantiations), __thiscall(ECX = tree; void* out_iterator[2], const
    // void* key), RET 8; EAX = out_iterator ({container, node}).
    virtual void** tree_lower_bound_005a1800(void* tree, void** out_iterator, const void* key) = 0;
    virtual void** tree_lower_bound_005a16f0(void* tree, void** out_iterator, const void* key) = 0;
    // 00521EA0 BSP_CommandTarget_ResolveObject, __fastcall(ECX = 18h-byte
    // command target); EAX = the object.
    virtual void* command_target_resolve_00521ea0(void* target) = 0;
    // 00B63B30 BSP_Matrix_InvertOrthogonalScaledAffine, __fastcall(ECX =
    // destination, EDX = source); EAX = destination.
    virtual float* matrix_invert_affine_00b63b30(float* destination, const float* source) = 0;
    // 00B62D10 BSP_Vector4f_Transform, __thiscall(ECX = input vector4; float*
    // out, const float* matrix), RET 8; EAX = out.
    virtual float* vector4_transform_00b62d10(const float* input, float* out, const float* matrix) = 0;

    // ---- network packet statistics CSV log (segment 44, 00789560) ----
    // IAT 00CE226C GetSystemTime (stdcall), the 16-byte SYSTEMTIME out.
    virtual void get_system_time_00ce226c(std::uint16_t* system_time) = 0;
    // File system [0109CEEC] vtable +4h(const NativeString* name, int mode),
    // RET 8; EAX = the opened (reference-counted) file object.
    virtual void* file_system_vcall_4_open(void* file_system, const void* name, std::int32_t mode) = 0;
    // Reference-counted object vtable slot +0h, __thiscall(ECX = object), no
    // stack arguments (called when InterlockedDecrement(object+4h) reaches 0).
    virtual void refcounted_vcall_0_release(void* object) = 0;

    // ---- pause-menu hint browser (segment 24, 006003A0) ----
    // 005FD220 STL_inst_005fd220 (std::map<int, NativeString> find),
    // __thiscall(ECX = tree; void* out_iterator[2], const int* key), RET 8;
    // EAX = out_iterator ({container, node}).
    virtual void** int_string_map_find_005fd220(void* tree, void** out_iterator, const std::int32_t* key) = 0;
    // 005FCE00 STL_TreeIter_Inc_005fce00, __thiscall(ECX = iterator {container,
    // node}); advances in place.
    virtual void tree_iterator_increment_005fce00(void** iterator) = 0;
    // 004E1CA0 BSP_AwardTracker_GetSingleton; EAX.
    virtual void* award_tracker_get_004e1ca0() = 0;
    // 005FF5D0 (map lookup helper), __thiscall(ECX = tree, const int* key),
    // RET 4; EAX (the key text handed to 006002B0).
    virtual void* int_string_map_lookup_005ff5d0(void* tree, const std::int32_t* key) = 0;
    // 006002B0 STL_StringMapCI_Subscript_006002B0, __thiscall(ECX = map, key),
    // RET 4; EAX = the value slot (a pooled string list).
    virtual void* string_map_ci_subscript_006002b0(void* map, void* key) = 0;
    // 004D48A0 BSP_PooledStringList_CopyConstruct, __thiscall(ECX = dest,
    // const void* source), RET 4.
    virtual void pooled_string_list_copy_construct_004d48a0(void* dest, const void* source) = 0;
    // 004D05E0 BSP_PooledStringList_ClearStorage, __thiscall(ECX = list).
    virtual void pooled_string_list_clear_004d05e0(void* list) = 0;
    // 005FD650 CG_array_ctor_helper_005fd650: the hint definition's
    // constructor, __thiscall(ECX = record).
    virtual void hint_definition_construct_005fd650(void* record) = 0;
    // 004C8F00 CG_vector_deleting_dtor_004c8f00: the hint definition's
    // destructor, __thiscall(ECX = record).
    virtual void hint_definition_destruct_004c8f00(void* record) = 0;
    // 005FFB20 (hint table lookup), __thiscall(ECX = tracker+30h, const void*
    // name), RET 4; EAX = the stored hint definition.
    virtual void* hint_table_lookup_005ffb20(void* table, const void* name) = 0;
    // 005FDD90 BSP_HintDefinition_AssignFull, __thiscall(ECX = dest, const
    // void* source), RET 4.
    virtual void hint_definition_assign_005fdd90(void* dest, const void* source) = 0;
    // 00425F40 BSP_NativeString_AssignHeader, __thiscall(ECX = dest, const
    // NativeString* source), RET 4.
    virtual void native_string_assign_header_00425f40(void* dest, const void* source) = 0;
    // 00A9FAD0 BSP_Localization_ResolveKeyList, __thiscall(ECX = [00F8BC4C];
    // WideString* out, const NativeString* key), RET 8.
    virtual void localization_resolve_key_list_00a9fad0(void* localization, void* out_wide, const void* key) = 0;
    // 00436630 BSP_NativeString_ConstructFromWideLowBytes, __thiscall(ECX =
    // dest, const wchar_t* text), RET 4; EAX = dest.
    virtual void* native_string_from_wide_00436630(void* dest, const std::uint16_t* text) = 0;
    // 00449AF0 BSP_NativeString_NotEqualCaseInsensitive, __thiscall(ECX =
    // left, const NativeString* right), RET 4; AL.
    virtual bool native_string_not_equal_ci_00449af0(const void* left, const void* right) = 0;
    // 005FD360 BSP_HudPauseScreen_RunLuaCommand, __thiscall(ECX = screen,
    // NativeString command by value (8 bytes; the callee releases it)), RET 8.
    virtual void pause_screen_run_lua_command_005fd360(void* screen, std::uint32_t length, char* data) = 0;
    // 00AB6BD0 BSP_TextContext_GetNormalizedTextHeight, __thiscall(ECX =
    // text widget); float in ST0.
    virtual float text_normalized_height_00ab6bd0(void* text) = 0;
    // Widget vtable +58h(const float* xy), RET 4.
    virtual void widget_vcall_58_set_position(void* widget, const float* xy) = 0;

    // ---- multiplayer mode selector (segment 23, 005EADC0) ----
    // 00683380 BSP_GuiScroller_AttachScrollBar, __thiscall(ECX = scroller;
    // frame box, float track length, top arrow, bottom arrow), RET 10h.
    virtual void gui_scroller_attach_scroll_bar_00683380(void* scroller, void* frame, float length, void* top_arrow,
                                                         void* bottom_arrow) = 0;

    // ---- multiplayer lobby settings rows (segment 23, 005E3580) ----
    // 005D8A70 BSP_MultiGameLobbyScreen_DeleteRowObjects, ECX = screen.
    virtual void lobby_delete_row_objects_005d8a70(void* screen) = 0;
    // 005D5930 BSP_MultiGameLobbyScreen_FindSettingRow, __thiscall(screen,
    // int key), RET 4; EAX.
    virtual std::int32_t lobby_find_setting_row_005d5930(void* screen, std::int32_t key) = 0;
    // 005D8B80 (no export), __thiscall(ECX = screen), no stack arguments; EAX
    // = a pointer whose first dword this caller tests.
    virtual std::int32_t* lobby_005d8b80(void* screen) = 0;
    // 005D8FC0 (no export), __thiscall(ECX = screen), no stack arguments.
    virtual void lobby_005d8fc0(void* screen) = 0;
    // 005E2320 BSP_LobbySettings_FormatOptionLabel, __fastcall(ECX =
    // NativeString* out, EDX = slot); EAX = out.
    virtual void* lobby_format_option_label_005e2320(void* out, std::int32_t slot) = 0;
    // 005D5500 BSP_LobbySettings_MenuRowForSlot, __fastcall(ECX = slot); EAX.
    virtual std::int32_t lobby_menu_row_for_slot_005d5500(std::int32_t slot) = 0;
    // 005DAD40 STL_Vector_PushBack_005dad40 (vector<void*>), __thiscall(ECX =
    // vector, void* const* value), RET 4.
    virtual void pointer_vector_push_back_005dad40(void* vector, void* const* value) = 0;
    // 005D86B0 STL_inst_005d86b0, __thiscall(ECX = screen); EAX = the current
    // scene record (the float pairs at +364h..+3E8h are read).
    virtual std::uint8_t* lobby_scene_record_005d86b0(void* screen) = 0;
    // 005D98E0 BSP_MultiGameLobby_RefreshMapAvailability, ECX = screen.
    virtual void lobby_refresh_map_availability_005d98e0(void* screen) = 0;
    // Widget vtable +8Ch, no arguments.
    virtual void widget_vcall_8c(void* widget) = 0;
    // Widget vtable +70h, no arguments.
    virtual void widget_vcall_70(void* widget) = 0;
    // Listener vtable +8h(row, listbox), RET 8 (the listbox listener's
    // selection callback, 006003A0's slot).
    virtual void listener_vcall_8_current(void* listener, void* row, void* listbox) = 0;

    // ---- main-menu mission list page (segment 20, 00597870) ----
    // 00518B90 BSP_FrontEndFrame_SetFlag, __fastcall(ECX = flag).
    virtual void frontend_frame_set_flag_00518b90(std::int32_t flag) = 0;
    // 00583E50 BSP_MainMenu_ClearObjectiveRows, ECX = screen.
    virtual void main_menu_clear_objective_rows_00583e50(void* screen) = 0;
    // 00AAC8E0 BSP_GuiMovie_IsPlaying, ECX = widget; AL.
    virtual bool gui_movie_is_playing_00aac8e0(void* widget) = 0;
    // 007FC820 STL_inst_007fc820 (BSP_Profile_TestUnlockRequirements),
    // __thiscall(ECX = [00E188A8]+650h, const void* requirements), RET 4; AL.
    virtual bool profile_test_unlock_007fc820(void* profile, const void* requirements) = 0;
    // 005C2F70 BSP_MissionRecord_IsCompleted, ECX = record; AL.
    virtual bool mission_record_is_completed_005c2f70(void* record) = 0;
    // 00581CE0 BSP_MainMenuScreen_LayoutMissionRow, __thiscall(screen, int
    // list), RET 4.
    virtual void main_menu_layout_mission_row_00581ce0(void* screen, std::int32_t list) = 0;
    // 00594B60 BSP_MainMenu_UpdateMissionMedal, ECX = screen.
    virtual void main_menu_update_mission_medal_00594b60(void* screen) = 0;
    // 005806A0 BSP_MainMenu_GetSelectedMission, ECX = screen; EAX.
    virtual void* main_menu_selected_mission_005806a0(void* screen) = 0;
    // 005C35D0 BSP_MissionTree_SelectById, __thiscall(ECX = [[00E198AC]+5Ch],
    // id), RET 4.
    virtual void mission_tree_select_by_id_005c35d0(void* tree, void* id) = 0;
    // 007F8D60 STL_inst_007f8d60, __thiscall(ECX = [00E188A8]+650h, mission),
    // RET 4; EAX (handed to a widget's vtable +34h).
    virtual std::int32_t profile_lookup_007f8d60(void* profile, void* mission) = 0;
    // 007FC370 BSP_PlayerProfile_CheckpointVehicleClassValid, __thiscall(ECX
    // = [00E188A8]+650h, mission), RET 4; AL.
    virtual bool profile_checkpoint_vehicle_class_valid_007fc370(void* profile, void* mission) = 0;
    // 005830A0 BSP_MainMenuScreen_ReleaseMissionPreview, ECX = screen.
    virtual void main_menu_release_mission_preview_005830a0(void* screen) = 0;
    // Widget vtable +80h(int), RET 4.
    virtual void widget_vcall_80(void* widget, std::int32_t value) = 0;

    // ---- multiplayer main menu layout (segment 23, 005E7B10) ----
    // 006B8CC0 STL_List_Begin_006b8cc0, __thiscall(ECX = achievement registry
    // [00E19900], void* out_iterator[2]), RET 4.
    virtual void achievement_list_begin_006b8cc0(void* registry, void** out_iterator) = 0;
    // 006B8B10 TRIV_body_006b8b10 (the registry's entry count), ECX =
    // registry; EAX.
    virtual std::int32_t achievement_count_006b8b10(void* registry) = 0;
    // 004FBE20 STL_TreeIter_Inc_004fbe20, __thiscall(ECX = iterator).
    virtual void tree_iterator_increment_004fbe20(void** iterator) = 0;
    // 00AB44B0 BSP_GUI_SetStateTexture, __thiscall(ECX = widget; int state,
    // const NativeString* name, const float uv[4]), RET 0Ch.
    virtual void gui_set_state_texture_00ab44b0(void* widget, std::int32_t state, const void* name,
                                                const float* uv) = 0;
    // Widget vtable +2Ch(const void*), RET 4 (here the achievement key string).
    virtual void widget_vcall_2c(void* widget, const void* value) = 0;
};

// 005A9DB0 BSP_HudMapScreen_BuildUnitOrderOverlays, __thiscall(ECX = HUD map
// screen; unit, int orders_only_flag), RET 8; caller 005B2D70 (reconstructed
// in cc13_sweepB5 as hud_map_build_unit_order_overlays_005a9db0, which passes
// 0 for the first list and 1 for the others). For the unit's order record it
// builds the capture-radius model (kind 1Ch units), the line or cone models of
// each order entry (flag 0 only), then (flag 0 only) the AI and waypoint
// arrows of the unit's command director and its target arrow.
void cc13_sweepB8_hud_map_build_unit_order_overlays_005a9db0(void* screen, void* unit, std::int32_t flag,
                                                            Cc13SweepB8Host& host);

// 00789560 BSP_NetworkPacketStatistics_StartCsvLog (toggle), __fastcall(ECX =
// statistics block), RET; caller 0088D6E0 (the Lua binding NetworkLog,
// reconstructed in cc13_left55_seg56 through packet_statistics_start_csv_00789560).
// Samples the clock; when logging is off opens "ns_<name><_server|_client>
// _send_M-D-H-Min-S.csv" and the matching "_recv_" file, clears the 1001
// statistics rows and counters; when on writes the closing row (007884D0)
// and releases both files. Byte +762900h toggles; AL is the new value (the
// caller's "AL result").
bool cc13_sweepB8_packet_stats_toggle_csv_log_00789560(void* stats, Cc13SweepB8Host& host);

// 006003A0 BSP_HudPauseListboxListener_Current08 (vtable 00CF3EE0 slot +8h),
// __thiscall(ECX = listener = pause screen + 8h; two stack words, unread),
// RET 8. When active (+8h): for the selected row key (listbox+164h +
// selection) either shows the row's entry text from the map +114h (stripping
// a leading "AAAAAA" marker) and hides the hint text, or (when +11Ch) finds
// the nearest lower key, looks up its award-tracker hint, fills the title,
// body and Lua command for the local party (Allied / Japanese / default
// fields, "_pc" localisation variant unless [00F88A30]), runs the command and
// lays out the text; then positions the rows without entries and the scroll
// bar thumb.
void cc13_sweepB8_pause_listbox_current08_006003a0(void* listener, std::uint32_t unused_row, std::uint32_t unused_list,
                                                   Cc13SweepB8Host& host);

// 005EADC0 BSP_MultiModeSelectorScreen_BindLayout (vtable 00CF2B44 slot
// +14h), __fastcall(ECX = screen), RET. Loads the pages FE_multimode_listbox
// (+14h), FE_multimode (+18h) and FE_createserver (+1Ch), binds 25 named
// children into the screen fields, wires the description scroller +3Ch,
// fills the create-server pop-up list box +A8h with four rows and sets
// their column offsets.
void cc13_sweepB8_multi_mode_selector_bind_layout_005eadc0(void* screen, Cc13SweepB8Host& host);

// 005E3580 BSP_MultiGameLobbyScreen_RefreshSettingRows, __fastcall(ECX =
// lobby screen), RET; callers 005E50D0, 005E51D0, 005E5AE0, 005E5B30. Clears
// and rebuilds the settings list boxes (+94h host column when +10h bit 0,
// +228h names), re-selects +274h, lays out the scroller, then updates the
// map preview widgets +1F8h/+244h for the selected scene (texture kind and
// UV rectangle from the scene's map bounds) and clears +10h.
void cc13_sweepB8_lobby_refresh_setting_rows_005e3580(void* screen, Cc13SweepB8Host& host);

// 00597870 BSP_MainMenuScreen_BuildMissionListPage, __thiscall(ECX = main
// menu screen, int page), RET 4; callers 005987F0, 00598B60, 00599340.
// Stores the page in 00E08874; pages 4..8 pick the title, the published
// group 00E194D8, the group widget +110h and list index (4 IJN, 5 USN, 8
// training, 6 IJN DLC, 7 USN DLC); then rebuilds the mission list box +1B8h
// from the group's 434h-byte mission records, the map flags and points, the
// selection 00E194DC/00E194E0 and the checkpoint widgets.
void cc13_sweepB8_main_menu_build_mission_list_00597870(void* screen, std::int32_t page, Cc13SweepB8Host& host);

// 005E7B10 BSP_MultiMainMenuScreen_BindLayout (vtable 00CF2708 slot +14h),
// __fastcall(ECX = screen), RET. Loads FE_multimenu_listbox (+18h) and
// FE_multimenu (+1Ch), binds 26 named children, wires the achievement
// scroller +6Ch, and clones achievement_0_Icon once per registry entry
// whose award record flag +24h is set (texture "" + record+14h, key node+0Ch).
void cc13_sweepB8_multi_main_menu_bind_layout_005e7b10(void* screen, Cc13SweepB8Host& host);

}  // namespace bsp
