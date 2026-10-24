// Front-end and HUD screens of segments 15 and 22, reconstruction pass (packet cc13-fe4).
//
// Segment 15 (00527CB0..00544B60): the gamepad control-layout screen, the HUD
// counters, credits, menu prompt, HUD formation, frame stats, free camera and the
// debug text lists. Segment 22 (005CD070..005DF510): the movie screens, the
// multiplayer chat, error, pause and game-lobby screens. docs/CC13_FE_4.md carries
// the evidence, the status of every routine and the list left to cc9.
//
// Model: every routine works on the original object as raw bytes (Win32 layout,
// offsets as in the image) and reaches the engine through Seg15Host, whose methods
// name the image routine they stand for. No globals are invented: a global the
// image touches is a host accessor that names its address. New C++ interfaces,
// not drop-in binary replacements; nothing here was run in bsp_game.

#ifndef BSP_FRONTEND_SEG15_22_CC13_HPP
#define BSP_FRONTEND_SEG15_22_CC13_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "bsp/ingame_gui_cc13.hpp"
#include "bsp/main_menu_command_bar.hpp"

namespace bsp::segs_13_15_22_44_cc13 {
struct SegBHost;  // 00527D40 tail target host (segs_13_15_22_44_cc13.hpp)
}

namespace bsp::frontend_seg15_22_cc13 {

using bsp::ingame_gui_cc13::Raw;
using bsp::ingame_gui_cc13::TrivialSlot;
using bsp::ingame_gui_cc13::TrivialSlotKind;
using bsp::MainMenuCommandArgument;

// ---------------------------------------------------------------------------
// Image addresses the routines store or compare (vtables, globals, constants).
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGameGlobal = 0x00E188A8;            // *: the game object
inline constexpr std::uint32_t kMainFrontEndGlobal = 0x00E198AC;     // *: main-menu front-end manager
inline constexpr std::uint32_t kOptionsManagerGlobal = 0x00E198B8;   // *: options front-end manager
inline constexpr std::uint32_t kMultiMenuGlobal = 0x00E198B4;        // *: multiplayer menu manager
inline constexpr std::uint32_t kInterfaceGlobal = 0x00E198C4;        // *: in-game interface
inline constexpr std::uint32_t kDisplayModeGlobal = 0x0109CF04;      // *: display mode, +0Dh wide
inline constexpr std::uint32_t kEmptyCString = 0x00E18DB8;           // "" used for a null name

// Control layout screen (0Eh): vtables installed by 00527D00 / 00527DA0.
inline constexpr std::uint32_t kControlLayoutVtable = 0x00CECF48;
inline constexpr std::uint32_t kControlLayoutListboxListenerVtable = 0x00CECF34;  // +8h
inline constexpr std::uint32_t kControlLayoutWidgetListenerVtable = 0x00CECF14;   // +Ch
inline constexpr std::uint32_t kListboxListenerBaseVtable = 0x00CEB0FC;           // 004F8F40 dtor
inline constexpr std::uint32_t kWidgetListenerBaseVtable = 0x00CEB110;
// 00E08674: six C-string pointers (two triples of stick-label widget names);
// 00E18DC0: 18 native strings {count, data} the labels are copied into.
inline constexpr std::uint32_t kStickLabelNames = 0x00E08674;
inline constexpr std::uint32_t kStickLabelNamesEnd = 0x00E0868C;
inline constexpr std::uint32_t kStickLabelTexts = 0x00E18DC0;
inline constexpr std::array<std::string_view, 6> kStickLabelNameValues{
    "LeftStickVert_Text", "LeftStickHorz_Text", "LeftStickClick_Text",
    "RightStickVert_Text", "RightStickHorz_Text", "RightStickClick_Text"};
inline constexpr std::array<std::string_view, 3> kControlLayoutGroups{
    "Plane_Group", "Ship_Group", "Submarine_Group"};  // into +1Ch, +20h, +24h

// HUD counters screen (51h).
inline constexpr std::uint32_t kHudCountersVtable = 0x00CED0AC;
inline constexpr float kCounterBarFloor = 0.1f;          // 00D7A2F0 (compared as double 00D7A3A0)
inline constexpr double kCounterBarFloorCompare = 0.10000000149011612;  // 00D7A3A0
inline constexpr double kCounterHoursThreshold = 3600.0;  // 00CE3D70
inline constexpr double kCounterMinutesThreshold = 60.0;  // 00CE3D68
inline constexpr double kCounterWrapBelow = -900.0;       // 00CED028
inline constexpr double kCounterWrapAdd = 1000.0;         // 00CE47A0
inline constexpr double kCounterScreen58Edge = 0.76;      // 00CED048 (00CED050 0.877f wide)
inline constexpr double kCounterScreen58EdgeWide = 0.8769999742507935;
inline constexpr float kCounterManagerPanelEdge = 0.355f;     // 00CED030
inline constexpr float kCounterManagerPanelEdgeWide = 0.2f;   // 00CE54A0
inline constexpr float kCounterScreen54Edge = 0.71f;          // 00CED058
inline constexpr float kCounterScreen54EdgeWide = 0.89f;      // 00CED05C
inline constexpr double kCounterScreenC0Edge = 0.936;         // 00CED038
inline constexpr double kCounterScreenC0EdgeWide = 1.0520000457763672;  // 00CED040

// ---------------------------------------------------------------------------
// Engine services. Each method names the image routine (ECX noted when it is
// not the first argument's object).
// ---------------------------------------------------------------------------
struct HudMessage35;

struct Seg15Host : bsp::ingame_gui_cc13::HudScreenServices {
    // Static storage of the image at that address (a global block, not a cell).
    virtual Raw global_block(std::uint32_t address) = 0;
    // *(address): the object a global pointer cell holds.
    virtual Raw global_object(std::uint32_t address) = 0;
    // (*obj)[slot](), ECX = obj, no arguments; result in EAX.
    virtual std::uint32_t object_virtual(Raw object, std::uint16_t slot) = 0;
    virtual std::uint32_t object_virtual_arg(Raw object, std::uint16_t slot, std::uint32_t arg) = 0;
    // Widget virtuals: +38h visible?, +5Ch widget type, +48h set scale(vec2*),
    // +80h set highlight flag.
    virtual bool widget_is_visible_38(void* widget) = 0;
    virtual std::int32_t widget_type_5c(void* widget) = 0;
    virtual void widget_set_scale_48(void* widget, const float xy[2]) = 0;
    // 00AA6750, ECX = widget, (out vec3*): the resolved screen position.
    virtual void widget_resolved_position_00aa6750(void* widget, float out[3]) = 0;
    // 00ABBE50, ECX = text widget, (cstring, 1): set localised text.
    virtual void text_set_localised_00abbe50(void* text, std::string_view value) = 0;
    // 0041E870 + 00518D60(0, &string): select front-end frame 0 with that title.
    virtual void front_end_frame_select_00518d60(int frame, std::string_view title) = 0;
    // 0041DD40 then memcpy: copy native string src into dst (count, data).
    virtual void native_string_copy_0041dd40(Raw dst, Raw src) = 0;
    // 004CC460, ECX = a front-end manager: push an interface request (id, arg).
    virtual void front_end_push_interface_004cc460(Raw manager, int id, int arg) = 0;
    // 004C1B90: the sound-request singleton (byte +1Dh is the "accept" cue).
    virtual Raw sound_request_singleton_004c1b90() = 0;
    // 00BF55BE operator new / 00BF6989 _free.
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void free_00bf6989(void* block) = 0;
    // 00523480, ECX = destination: copy-construct an observer handle (18h bytes).
    virtual void observer_handle_copy_00523480(Raw dst, Raw src) = 0;
    // Existing reconstructions in src/band_0050_cc13.cpp, called on the
    // counters screen (ECX = screen): 005292E0, 005290D0, 00529350.
    virtual void counters_restore_hidden_a7_005292e0(Raw screen) = 0;
    virtual void counters_hide_all_a6_005290d0(Raw screen) = 0;
    virtual void counters_restore_hidden_a6_00529350(Raw screen) = 0;
    // 006529E0 (src/ingame_gui_cc13.cpp), ECX = *(interface + BCh): the support
    // manager's mode test.
    virtual bool support_manager_mode_006529e0(Raw manager) = 0;
    // 00529A30, ECX = counters screen: re-lay out the visible counters.
    virtual void counters_relayout_00529a30(Raw screen) = 0;
    // Observer pair on an observer member (ECX = observed, EDX = member).
    virtual void observer_pair_register_00694a60(Raw member, void* observed) = 0;
    virtual void observer_pair_unregister_006952a0(Raw member, void* observed) = 0;
    // 0075B430 + vtable 00CED098 (+4 = 1), fields as in bsp/native_session_message35.hpp,
    // then 00772330 with ECX = *00E188A8 + 1EF0h and 0052AF50 to destroy it.
    virtual void session_send_message35_00772330(const HudMessage35& message) = 0;
    // Counter-entry layouts not reconstructed in this pass (ECX = entry); the
    // strings are native strings passed by value and freed by the callee.
    virtual void counter_layout_timer_00529d70(Raw entry, std::string_view a, std::string_view b) = 0;
    virtual void counter_layout_value_0052a100(Raw entry, std::string_view a, std::string_view b,
                                               std::uint32_t v0, std::uint32_t v1) = 0;
    virtual void counter_layout_countdown_0052a860(Raw entry, std::string_view text, float seconds) = 0;
    virtual void counter_layout_score_0052bb10(Raw entry, std::uint32_t value, std::string_view text) = 0;
    // 0052C1B0 / 0052C640 into a zeroed 0ACh block: construct from the page and
    // the twelve widget names, or clone a template entry.
    virtual Raw counter_entry_construct_0052c1b0(void* page, const std::string_view (&names)[12]) = 0;
    virtual Raw counter_entry_clone_0052c640(Raw source) = 0;
    // 0041DD40 (count, 1) / pool return of a native string's block.
    virtual void native_string_release(Raw string) = 0;
    // 004C53E0, ECX = wide string {count, data}: (count, 1).
    virtual void wide_string_resize_004c53e0(Raw string, std::uint32_t count) = 0;
    // 00BF65AC: the _free the STL vectors use (the arrays above use 00BF6989).
    virtual void free_00bf65ac(void* block) = 0;
    // 00BF7C6E `eh vector destructor iterator`(first, size, count, dtor).
    virtual void eh_vector_destruct_00bf7c6e(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t dtor) = 0;
    // 00432050, ECX = first, EDX = last, two unread stack words, RET 8: destroy a
    // native string range.
    virtual void native_string_range_destroy_00432050(void* first, void* last, void* extra1, void* extra2) = 0;
    // 00530F90, ECX = prompt record: destroy it (strings released).
    virtual void menu_prompt_record_destroy_00530f90(Raw record) = 0;
    // 00532C50, ECX = menu prompt screen: complete the prompt with a result.
    virtual void menu_prompt_complete_00532c50(Raw screen, int result) = 0;
    // In-game interface (ECX = *00E198C4): 0068AB80 collapse overlays (a, b),
    // 00689FC0 / 00689FE0 / 00689FA0 submarine / torpedo-boat / ship input
    // contexts, 0068AB30 leave the unit request level.
    virtual void interface_collapse_overlays_0068ab80(int a, int b) = 0;
    virtual void interface_set_input_contexts(std::uint32_t routine) = 0;
    virtual void interface_leave_unit_request_level_0068ab30() = 0;
    // 004BEC00: the input manager; its action records are 30h bytes at *(+4).
    virtual Raw input_manager_004bec00() = 0;
    // 00A9C920, ECX = listbox: selected index.
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    // 0070D070, ECX = unit group, (index): the member record.
    virtual Raw unit_group_member_record_0070d070(void* group, std::int32_t index) = 0;
    // 004C12B0 then 00AA1FA0(vec2 by value): move the GUI cursor.
    virtual void gui_set_cursor_position_00aa1fa0(float x, float y) = 0;
    // 00538910, ECX = map, (&key): std::map subscript, the value slot.
    virtual void** map_subscript_00538910(void* map, std::uint32_t key) = 0;
    // 00BF681B operator new; 005A6AD0, ECX = block, (member, scene, screen+20h).
    virtual void* operator_new_00bf681b(std::size_t bytes) = 0;
    virtual Raw unit_marker_model_construct_005a6ad0(void* block, std::uint32_t member, void* scene, void* frame) = 0;
    // 00B6DA70, ECX = scene node, (float factor, int).
    virtual void scene_node_set_visibility_00b6da70(void* node, float factor, int flag) = 0;
    // 0093CB10, ECX = screen+0Ch: simple 3D scene sub-object construct.
    virtual void simple_3d_scene_construct_0093cb10(Raw scene) = 0;
    // 005359A0 tree head / 00535720, 005356C0 list heads.
    virtual void* tree_buy_head_node_005359a0() = 0;
    virtual void* list_buy_head_node(std::uint32_t routine) = 0;
    // Profiler object 00E18E80 (ECX): BE32B0 end record, BE3360 elapsed seconds
    // (ST0), BE3300 reset, BE3260 begin (tail jump). 004C1DD0: the profiler
    // instance; *(+20h) is a float array indexed by the counter id held in the
    // named global (0109D014, 0109DB08, 0109DB14, 0109DB34).
    virtual void profiler_record_end_00be32b0() = 0;
    virtual float profiler_elapsed_seconds_00be3360() = 0;
    virtual void profiler_record_reset_00be3300() = 0;
    virtual void profiler_record_begin_00be3260() = 0;
    virtual float profiler_counter_value(std::uint32_t id_global) = 0;
    // 0053BCC0, ECX = frame stats: format the overlay lines (float rate).
    virtual void frame_stats_format_lines_0053bcc0(Raw stats, float rate) = 0;
    // 004134F0, ECX = destination matrix, (source): 4x4 copy.
    virtual void matrix_copy_004134f0(void* dst, const void* src) = 0;
    // 00867B10, ECX = point effect: stop its children.
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;
    // 00871BA0, ECX = out handle, (&name, 1): acquire an effect by name; the
    // returned handle carries one reference owned by the caller.
    virtual void* effect_handle_acquire_00871ba0(std::string_view name) = 0;
    // 0053DA70: free-camera effect list head; 0053F250, ECX = vector: tidy.
    virtual void* list_buy_head_node_0053da70() = 0;
    virtual void effect_vector_tidy_0053f250(Raw vec) = 0;
    // 00695870, ECX = callback-owner sub-object.
    virtual void callback_owner_destroy_00695870(Raw owner) = 0;
    // A routine reached by tail jump with ECX = object and no further arguments.
    virtual void tail_call(std::uint32_t routine, Raw object) = 0;
    // 00ABBE50 with a C string pointer (the row's SBO buffer).
    virtual void text_set_localised_cstr_00abbe50(void* text, const char* value) = 0;
    // Widget +50h set colour (float rgba[4]); +60h set focus (bool).
    // BSP_LegacySboString_AssignSubstring, ECX = dst string, (src, 0, npos).
    virtual void legacy_string_assign_substring(Raw dst, Raw src) = 0;
    // --- segment 22 ---
    // Tree erase-all instances (ECX = tree {alloc, head, size}): 005CE340, 005CD710.
    virtual void tree_erase_all(std::uint32_t routine, Raw tree) = 0;
    virtual void* tree_buy_head_node(std::uint32_t routine) = 0;  // 005CC2E0 / 005CC330
    // BSP_VehicleCameraArray_Resize, ECX = array, (n).
    virtual void vehicle_camera_array_resize(Raw array, std::int32_t n) = 0;
    // 004C8DD0, ECX = dst wide string, (src): copy-construct.
    virtual void wide_string_copy_construct_004c8dd0(Raw dst, Raw src) = 0;
    // 00772170, ECX = *00E188A8 + 1EF0h: (const NativeString* sender, channel,
    // kind, const NativeString* a, const NativeString* b, id0, id1).
    virtual void session_send_chat_00772170(Raw sender, std::int32_t channel, std::int32_t kind, Raw a, Raw b,
                                            std::int32_t id0, std::int32_t id1) = 0;
    // A native string holding "" for the duration of a call (0041E870 from 00CE3A0C).
    virtual Raw native_string_temp(std::string_view value) = 0;
    // 00AA83F0, ECX = widget (clears a chat list widget).
    virtual void widget_clear_00aa83f0(void* widget) = 0;
    // 00A97260 / 00A97140, ECX = screen+8h: text-listener base construct / destroy.
    virtual void text_listener_construct_00a97260(Raw listener) = 0;
    virtual void text_listener_destroy_00a97140(Raw listener) = 0;
    // 005CEE20 / 005CEE40 chat list heads; 005CF910 / 005CF950 list clears
    // (ECX = list {alloc, head, size}).
    virtual void* chat_list_buy_head(std::uint32_t routine) = 0;
    virtual void chat_list_clear(std::uint32_t routine, Raw list) = 0;
    // 004D6480, ECX = *00E188A8, (const vector<int>* screen ids, bool park).
    virtual void gui_park_screens_004d6480(const std::int32_t* ids, std::size_t count, bool park) = 0;
    // 00A966E0, ECX = screen+8h, (bool active, text widget).
    virtual void text_editor_set_active_00a966e0(Raw listener, bool active, void* text) = 0;
    // 006882A0, ECX = *00E198B4: close the pause interface.
    virtual void multi_menu_close_pause_006882a0() = 0;
    // 004261A0, ECX = a, (out, b): native string concatenation.
    virtual void native_string_concat_004261a0(Raw a, Raw out, Raw b) = 0;
    // 0067D940, ECX = text, EDX = prefix, (cursor block, value): fit the input on one line.
    virtual void text_fit_range_one_line_0067d940(void* text, Raw prefix, Raw cursor_block, std::uint32_t value) = 0;
    // 00432650: the global config; colours live at +208h.. (10h apart).
    virtual Raw global_config_00432650() = 0;
    // 00AB95F0, ECX = text, (bool): cursor mode.
    virtual void text_set_cursor_mode_00ab95f0(void* text, bool mode) = 0;
    // Pool return of a wide string's block ((count + 1) * 2 bytes).
    virtual void wide_string_release(Raw string) = 0;
    // 0076FAD0, ECX = *00E188A8 + 1EF0h: session mode.
    virtual void session_set_mode_0076fad0(std::int32_t mode) = 0;
    // *00F8A2FC (the online service) virtual: +14h signed in, +198h session off.
    virtual bool online_service_virtual(std::uint16_t slot) = 0;
    // *00F8ABE8 (the Xenon system manager): 00A3E510 has selected user,
    // 00A3EAD0 sign-in state, 00A3EAC0(0, 0, 0, -1) active user slot.
    virtual bool xenon_has_selected_user_00a3e510() = 0;
    virtual std::int32_t xenon_sign_in_state_00a3ead0() = 0;
    virtual std::int32_t xenon_active_user_slot_00a3eac0() = 0;
    // 00A4D446 XShowMarketplaceUI(user slot, 0, 0, 0) (the import).
    virtual void x_show_marketplace_ui_00a4d446(std::int32_t user) = 0;
    // 004D7920, ECX = *00E188A8: request a game state.
    virtual void game_request_state_004d7920(std::int32_t state) = 0;
    // ECX = *00E198B4: 006881F0 pop the request, 00688B20 pop up to an interface;
    // *(*00E198B4 + 20h) is the current interface.
    virtual void multi_menu_pop_request_006881f0() = 0;
    virtual void multi_menu_pop_up_to_00688b20(std::int32_t id) = 0;
    // Widget +40h update(float seconds).
    virtual void widget_update_40(void* widget, float seconds) = 0;
    // 005D20A0, ECX = error screen: lay the buttons out (not reconstructed: x87).
    virtual void multi_error_layout_buttons_005d20a0(Raw screen) = 0;
    // 00AB6AB0, ECX = text, (const WideString*): submit UTF-16 text.
    virtual void text_submit_utf16_00ab6ab0(void* text, Raw wide) = 0;
    // 004D7970, ECX = *00E188A8: end the scene; 004DB030: toggle the pause menu.
    virtual void game_end_scene_004d7970(std::int32_t how) = 0;
    virtual void game_toggle_pause_menu_004db030() = 0;
    // Widget +84h (bool) / +88h (int, int, float) state virtuals of types 12h / 6.
    virtual void widget_virtual_84(void* widget, std::int32_t value) = 0;
    virtual void widget_virtual_88(void* widget, std::int32_t value, std::int32_t b, float c) = 0;
    // 00425E50, cdecl(object) -> value.
    virtual std::uint32_t call_00425e50(void* object) = 0;
    // 005FD890, ECX = *(interface + D8h), (page, show).
    virtual void pause_screen_show_page_005fd890(std::int32_t page, std::int32_t show) = 0;
    // 004C9680: select the default front-end frame (tail jump).
    virtual void front_end_frame_select_default_004c9680() = 0;
    // 004BCA50, ECX = *00E188A8: the effective game mode; 004BC890 set (mode, 0);
    // 004BB770 (&side a, &side b) participants by side.
    virtual std::int32_t game_effective_mode_004bca50() = 0;
    virtual void game_set_game_mode_004bc890(std::int32_t mode, std::int32_t flag) = 0;
    virtual void game_count_participants_by_side_004bb770(std::int32_t* a, std::int32_t* b) = 0;
    // 005D5500 (src/mission_lobby_settings.cpp): the menu row of a settings slot.
    virtual std::int32_t lobby_settings_menu_row_005d5500(std::int32_t slot) = 0;
    // Session (ECX = *00E188A8 + 1EF0h): 0076C790 kick (slot record), 00771AC0 /
    // 00771CC0 slot swap accept / refuse (a, b).
    virtual void session_kick_player_peer_0076c790(Raw slot) = 0;
    virtual void session_swap_slot_accept_00771ac0(std::int32_t a, std::int32_t b) = 0;
    virtual void session_swap_slot_refuse_00771cc0(std::int32_t a, std::int32_t b) = 0;
    // 00687860, ECX = *00E198B4, (id, 0): interface on the stack.
    virtual bool multi_menu_has_interface_00687860(std::int32_t id) = 0;
    // 00515BC0, ECX = *(*00E198AC + 70h): leave the tactical library.
    virtual void tactical_library_leave_00515bc0() = 0;
    // 006834A0 (float) / 00683790 (0), ECX = scroller block.
    virtual void scroller_set_range_006834a0(Raw scroller, float range) = 0;
    virtual void scroller_attach_content_00683790(Raw scroller, void* content) = 0;
    // Session broadcasts (ECX = *00E188A8 + 1EF0h): 0076C9C0 settings (block),
    // 00772390 scene index, 00772430 sub-scene (00E195C4, value).
    virtual void session_broadcast_multi_settings_0076c9c0(Raw block) = 0;
    virtual void session_broadcast_scene_index_00772390() = 0;
    virtual void session_broadcast_sub_scene_00772430(Raw name, std::uint32_t value) = 0;
    // 004C1E90: the profile-hints owner (+8h: content checks off).
    virtual Raw profile_hints_owner_004c1e90() = 0;
    // 00425D10 then 00532A20(slot), ECX = the menu command screen.
    virtual void menu_prompt_dismiss_slot_00532a20(std::int32_t slot) = 0;
    // 005D8650(map id) -> index; 0050FD10, ECX = *(*00E198AC + 70h), (index): unlocked.
    virtual std::int32_t lobby_map_index_005d8650(std::uint32_t map) = 0;
    virtual bool tactical_library_map_unlocked_0050fd10(std::int32_t index) = 0;
    // Lobby routines not reconstructed here, ECX = the given object.
    virtual void lobby_call(std::uint32_t routine, Raw object) = 0;
    // 007728B0, ECX = *00E188A8 + 1EF0h, (0): broadcast game closed.
    virtual void session_broadcast_game_closed_007728b0(std::int32_t value) = 0;
    // *00F8A2FC virtual with one argument (+170h end hosting).
    virtual void online_service_virtual_arg(std::uint16_t slot, std::int32_t arg) = 0;
    // 00688AD0, ECX = *00E198B4, (id): push an interface request (logged).
    virtual void multi_menu_push_request_00688ad0(std::int32_t id) = 0;
    // 00557BC0, ECX = dst, (src): checked native-string vector copy.
    virtual void checked_string_vector_copy_00557bc0(Raw dst, Raw src) = 0;
    // 0054A0C0, ECX = *00E1930C (command bar), (const NativeString*): help line.
    virtual void command_bar_set_help_line_0054a0c0(Raw text) = 0;
    // 00688C70, ECX = *00E198B4: leave to the main multiplayer interface.
    virtual void multi_menu_leave_to_main_00688c70() = 0;
    // Native string substring (count, data) of the first n characters.
    virtual void native_string_truncate(Raw string, std::uint32_t n) = 0;
    // --- cc13-fe7 additions (docs/CC13_FE_7.md) ---
    // Listbox routines, ECX = listbox: 00A9BEC0 clear rows, 00A9C7C0 select
    // row, 00A9CDC0 clear, 00A9CDF0 (ECX = this, EDX = other, stack = third)
    // clear the other two, 00A9BE00(row) -> row widget.
    virtual void listbox_clear_rows_00a9bec0(void* listbox) = 0;
    virtual void listbox_select_index_00a9c7c0(void* listbox, std::int32_t row) = 0;
    virtual void listbox_clear_00a9cdc0(void* listbox) = 0;
    virtual void listbox_clear_other_two_00a9cdf0(void* listbox, void* edx_listbox, void* stack_listbox) = 0;
    virtual void* listbox_row_00a9be00(void* listbox, std::int32_t row) = 0;
    // 004C12B0 then 00AA0F70, ECX = GUI manager: reset the screens.
    virtual void gui_manager_reset_screens_00aa0f70() = 0;
    // 004BEC00 then 00A92C40(seconds), ECX = input manager.
    virtual void input_manager_update_00a92c40(float seconds) = 0;
    // Tactical library ([*00E198AC+70h]) routines: 004FD500 / 004FD3A0(&handle,
    // uv[4]) fill a texture handle (one reference) and its uv rectangle;
    // 004FBE90(bool) sets the maps-available flag.
    virtual void tactical_library_map_icon_004fd500(Raw library, void** handle, float uv[4]) = 0;
    virtual void tactical_library_available_icon_004fd3a0(Raw library, void** handle, float uv[4]) = 0;
    virtual void tactical_library_set_maps_available_004fbe90(Raw library, bool available) = 0;
    // 00AB2690(state, texture, const uv*), ECX = widget.
    virtual void widget_set_state_texture_00ab2690(void* widget, std::int32_t state, void* texture,
                                                   const float uv[4]) = 0;
    // Drop one reference (InterlockedDecrement of +4h; virtual +0h when it
    // reaches zero).
    virtual void refcounted_release(void* object) = 0;
    // Session routines, ECX = [*00E188A8]+1EF0h: 00771E80(slot, flag) AI
    // enable, 007718F0(a, b) select player, 00771990(a, b) swap slot request.
    virtual void session_slot_ai_enable_00771e80(std::int32_t slot, std::int32_t flag) = 0;
    virtual void session_request_select_player_007718f0(std::int32_t a, std::int32_t b) = 0;
    virtual void session_swap_slot_request_00771990(std::int32_t a, std::int32_t b) = 0;
    // 004BB850(record), ECX = *00E188A8 -> AL (unused by the kick prompt).
    virtual bool game_participant_side_has_room_004bb850(Raw record) = 0;
    // 00531B00 on the 00425D10 singleton (frontend_prompts.hpp's order):
    // slot, message key, kind, callback, unused, title (a native string),
    // timeout, timeout result, countdown key (by value), dismissible.
    virtual void menu_prompt_raise_00531b00(std::int32_t slot, std::string_view message, std::int32_t kind,
                                            std::uint32_t callback, std::int32_t unused, Raw title, float timeout,
                                            std::int32_t timeout_result, std::string_view countdown,
                                            std::int32_t dismissible) = 0;
    // 00425D10 then 00530C20 / 00530670: focus the prompt's last / first
    // navigation row.
    virtual void menu_prompt_focus_last_00530c20() = 0;
    virtual void menu_prompt_focus_first_00530670() = 0;
    // Resize + memcpy of a C string into a native string.
    virtual void native_string_assign_cstr(Raw dst, const char* text) = 0;
    // The characters of a native string (count, data).
    virtual std::string native_string_value(Raw string) = 0;
    // 005D8650 (no arguments) -> the selected map record (name native string
    // at +10h).
    virtual Raw lobby_selected_map_record_005d8650() = 0;
    // 005D5160, ECX = lobby screen -> the map list block (native string vector
    // at +4h: first +8h, last +0Ch, 8-byte elements).
    virtual Raw lobby_map_list_005d5160(Raw screen) = 0;
    // 004C8950(const char*) / 004C6890(int), ECX = *00E188A8.
    virtual void game_set_command_line_text_004c8950(const char* text) = 0;
    virtual void game_select_scene_record_004c6890(std::int32_t index) = 0;
    // Session ([*00E188A8]+1EF0h): 0076F360 collect incompatible clients;
    // 00771DD0(player, flag, 1) request player ready
    // (multiplayer_session_cc13::request_player_ready_00771dd0 models it).
    virtual void session_collect_incompatible_clients_0076f360() = 0;
    virtual void session_request_player_ready_00771dd0(std::int32_t player, bool flag, std::int32_t c) = 0;
};

// MSVC 8 vector layout used by the screens: {allocator dword, first, last, end}.
// Element operations supply the image's copy-construct / assign / destroy.
struct VectorElementOps {
    std::size_t size;
    void (*copy_construct)(Raw dst, Raw src, Seg15Host& host);
    void (*assign)(Raw dst, Raw src, Seg15Host& host);
    void (*destroy)(Raw element, Seg15Host& host);
    std::uint32_t max_count;  // the xlen limit (0FFFFFFFFh / size)
};
// _Insert_n(where, count, value) with 1.5x growth (0052F9E0, 0053FF10,
// 00543E90, 005440E0 are instances).
void vector_insert_n(Raw vec, std::uint8_t* where, std::uint32_t count, Raw value,
                     const VectorElementOps& ops, Seg15Host& host);

// Session message 23h (35) built by the HUD counter senders 0052B400..0052B900.
// The offsets are those of bsp::NativeSessionMessage35.
struct HudMessage35 {
    std::uint32_t value_18 = 0;  // score id (score forms)
    std::uint32_t value_1c = 0;  // owner / player value
    std::uint32_t subtype_20 = 0;  // 0 unit HP, 1 score
    std::uint8_t hide_24 = 0;     // 1 = hide form
    std::uint8_t pair_25 = 0;     // 1 = score pair (values_38 valid)
    std::string_view text_28;
    std::string_view text_30;
    std::uint32_t values_38[2] = {0, 0};
    const std::uint16_t* unit_ids_40 = nullptr;  // word array (each unit's +174h)
    std::int32_t unit_count_40 = 0;
    std::string_view text_4c;
};

// ---------------------------------------------------------------------------
// Trivial vtable slots (constant returns, empty bodies, adjustor thunks).
// ---------------------------------------------------------------------------
extern const TrivialSlot kTrivialSlots[];
extern const std::size_t kTrivialSlotCount;

// ---------------------------------------------------------------------------
// Image containers. A PodArray is {T* data; int32 size; int32 capacity} (0Ch).
// ---------------------------------------------------------------------------
// 00529700 / 00529760 (pointers), word variant BSP_WordArray_Reserve: grow to
// max(n, 1) when n exceeds the capacity (operator new, element copy, _free).
void ptr_array_reserve_00529700(Raw array, std::int32_t n, Seg15Host& host);
// 005297C0 / 005299D0: resize to n, zeroing new slots.
void ptr_array_resize_005297c0(Raw array, std::int32_t n, Seg15Host& host);
// 00529900 / 00529940: push back with doubling growth (minimum 1).
void ptr_array_push_back_00529900(Raw array, std::uint32_t value, Seg15Host& host);
// 005298C0: the 16-bit element variant of push back.
void word_array_push_back_005298c0(Raw array, std::uint16_t value, Seg15Host& host);
// 00529620: erase the element at index (shift down, size - 1).
void ptr_array_erase_at_00529620(Raw array, std::int32_t index);
// 00529650: index of the pointer or -1.
std::int32_t ptr_array_index_of_00529650(Raw array, std::uint32_t value);
// 00529C50 / 0052AE40: resize(0) then _free(data).
void ptr_array_destroy_00529c50(Raw array, Seg15Host& host);
// 00529C70: reserve for 18h-byte observer-handle records.
void rec18_array_reserve_00529c70(Raw array, std::int32_t n, Seg15Host& host);

// ---------------------------------------------------------------------------
// Control layout screen (0Eh, page FE_controls_X360 + FE_controls_X360_listbox).
// ---------------------------------------------------------------------------
// 00527CB0, __thiscall(layout, out a, b, c, d), RET 10h: when +2Ch is set copy
// the four bytes +28h..+2Bh out and clear +2Ch.
bool control_layout_take_button_flags_00527cb0(Raw layout, std::uint8_t out[4]);
Raw control_layout_construct_00527d00(Raw s, Seg15Host& host);
void control_layout_destruct_00527da0(Raw s, Seg15Host& host);
void control_layout_listener_current0c_00527d50(void* widget, Seg15Host& host);
void control_layout_listbox_activate_00527e30(Seg15Host& host);
void control_layout_register_00527e60(Raw s, Seg15Host& host);
void control_layout_bind_widgets_00527f90(Raw s, Seg15Host& host);
void control_layout_exit_00528920(Seg15Host& host);
void control_layout_update_005289b0(Seg15Host& host);
void control_layout_listener_current04_005289e0(void* widget, Seg15Host& host);
void control_layout_collect_children_00528a60(Raw s, void* out, Seg15Host& host);

// ---------------------------------------------------------------------------
// HUD counters screen (51h): timer / value / countdown / score entries.
// ---------------------------------------------------------------------------
// 00528B00, __thiscall(entry, float seconds), RET 4.
void hud_counter_format_time_00528b00(Raw entry, float seconds, Seg15Host& host);
// 00528BE0, __thiscall(counter, float value), RET 4.
void hud_counter_set_value_00528be0(Raw counter, float value);
// 00528E90, __thiscall(counter, float a, float b), RET 8.
void hud_counter_update_bar_00528e90(Raw counter, float a, float b, Seg15Host& host);
void hud_counters_hide_under_manager_panel_00528f70(Raw s, Seg15Host& host);
void hud_counters_hide_under_screen58_00529200(Raw s, Seg15Host& host);
void hud_counters_hide_under_screen54_005293b0(Raw s, Seg15Host& host);
// 00529120 (no Ghidra function; tail of 00529810): hide entries right of the
// C0 edge, flag +A9h.
void hud_counters_hide_right_of_edge_00529120(Raw s, Seg15Host& host);
void hud_counters_update_occlusion_00529810(Raw s, Seg15Host& host);
void hud_counters_remove_counter_a_0052ab40(Raw s, Seg15Host& host);
void hud_counters_cancel_countdown_0052ab90(Raw s, Seg15Host& host);
Raw hud_counters_construct_0052b030(Raw s, Seg15Host& host);
// 0052AD50 / 0052ADD0 / 0052AE60 / 0052C0A0: the 18h-byte observer-handle
// record array (vtable 00CE75CC, observed pointer +14h, "set" byte +10h).
void rec18_array_push_back_0052ad50(Raw array, Raw source, Seg15Host& host);
void rec18_array_remove_at_swap_last_0052add0(Raw array, std::int32_t index, Seg15Host& host);
void rec18_array_resize_0052ae60(Raw array, std::int32_t n, Seg15Host& host);
void rec18_array_destroy_0052c0a0(Raw array, Seg15Host& host);
// Session senders (Lua DisplayUnitHP / HideUnitHP / DisplayScores / HideScoreDisplay).
// 0052B400 __stdcall(owner, const PtrArray* units, NativeString text), RET 10h.
void hud_send_display_unit_hp_0052b400(std::uint32_t owner, Raw units, std::string_view text, Seg15Host& host);
void hud_send_hide_unit_hp_0052b570(std::uint32_t owner, Seg15Host& host);  // RET 4
// 0052B620 (id, owner, NativeString a, NativeString b), RET 18h.
void hud_send_display_score_0052b620(std::uint32_t id, std::uint32_t owner, std::string_view a,
                                     std::string_view b, Seg15Host& host);
// 0052B780 (id, owner, a, b, v0, v1), RET 20h.
void hud_send_display_score_pair_0052b780(std::uint32_t id, std::uint32_t owner, std::string_view a,
                                          std::string_view b, std::uint32_t v0, std::uint32_t v1, Seg15Host& host);
void hud_send_hide_score_0052b900(std::uint32_t id, std::uint32_t owner, Seg15Host& host);  // RET 8
// 0052B9B0, __thiscall(screen, NativeString text, float seconds), RET 0Ch.
void hud_counters_show_countdown_0052b9b0(Raw s, std::string_view text, float seconds, Seg15Host& host);
// 0052BFA0, __thiscall(screen, value, NativeString text), RET 0Ch.
void hud_counters_show_unit_hp_0052bfa0(Raw s, std::uint32_t value, std::string_view text, Seg15Host& host);
// 0052C850, __thiscall(screen, index), RET 4: the score entry, created on demand.
Raw hud_counters_ensure_entry_0052c850(Raw s, std::int32_t index, Seg15Host& host);
// 0052C990, __thiscall(screen, index, a, b), RET 14h.
void hud_counters_show_score_0052c990(Raw s, std::int32_t index, std::string_view a, std::string_view b,
                                      Seg15Host& host);
// 0052CAC0, __thiscall(screen, index, a, b, v0, v1), RET 1Ch.
void hud_counters_show_score_pair_0052cac0(Raw s, std::int32_t index, std::string_view a, std::string_view b,
                                           std::uint32_t v0, std::uint32_t v1, Seg15Host& host);
void hud_counter_entry_destruct_0052c0c0(Raw entry, Seg15Host& host);
void hud_counter_entry_destroy_and_free_0052cdc0(Raw slot, Seg15Host& host);
void hud_counters_register_0052cbf0(Raw s, Seg15Host& host);
void hud_counters_destruct_0052cdf0(Raw s, Seg15Host& host);
// 0052CF00, __thiscall(screen, index), RET 4.
void hud_counters_remove_entry_widget_0052cf00(Raw s, std::int32_t index, Seg15Host& host);
void hud_counters_collect_children_0052d1d0(Raw s, void* out, Seg15Host& host);

// ---------------------------------------------------------------------------
// Widget-listener slots shared by several screens (each vtable has its own copy).
// ---------------------------------------------------------------------------
// 00527D50 / 0052D6B0 (+0Ch), RET 4: type 3 -> alpha 0.5f if byte +D4h else 1.0f;
// any other type re-queries the type (the image's second +5Ch call).
void listener_alpha_from_d4(void* widget, bool requery_other, Seg15Host& host);
// 0052D210 (+18h), __thiscall(listener, widget, bool), RET 8: as above with the flag argument.
void credits_listener_current18_0052d210(void* widget, bool flag, Seg15Host& host);
// 0052D200 (+14h), RET 4: forwards to the widget's +5Ch.
void credits_listener_current14_0052d200(void* widget, Seg15Host& host);
// 0052E5F0 (+4h), RET 4: type 3 -> alpha 1.0; name starting A3h pushes interface 1
// on the main front-end manager (*00E198AC); other types re-query the type.
void credits_listener_current04_0052e5f0(void* widget, Seg15Host& host);

// ---------------------------------------------------------------------------
// Credits screen (5Ch-byte object, vtables 00CED200 / 00CED1E0).
// A credits item is 24h bytes: {int32 type, NativeString text, six dwords}.
// ---------------------------------------------------------------------------
// 0052D1E0, cdecl(buffer, format, ...) = vswprintf (the CRT form without a count;
// this model takes the buffer size).
int credits_format_wide_0052d1e0(wchar_t* buffer, std::size_t count, const wchar_t* format, ...);
void native_string_release_at4_0052dfb0(Raw record, Seg15Host& host);  // also 0052E6D0
// 0052E0F0, __thiscall(wide string, const wchar16*), RET 4 -> EAX this.
Raw wide_string_construct_0052e0f0(Raw dst, const std::uint16_t* source, Seg15Host& host);
void credits_item_copy_construct_0052e2c0(Raw dst, Raw src, Seg15Host& host);
void credits_item_assign_0052e340(Raw dst, Raw src, Seg15Host& host);
// 0052E3B0, __fastcall(ECX slot, EDX unused, src): placement copy when slot != 0.
void credits_item_placement_copy_0052e3b0(Raw slot, Raw src, Seg15Host& host);
// 0052E410, cdecl(first, last, dest_end) -> new dest: copy_backward of items.
std::uint8_t* credits_item_copy_backward_0052e410(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest_end,
                                                  Seg15Host& host);
// 0052F9E0, __thiscall(vector, iterator, where, count, const item*), RET 10h.
void credits_item_vector_insert_n_0052f9e0(Raw vec, std::uint8_t* where, std::uint32_t count, Raw value,
                                           Seg15Host& host);
Raw credits_construct_0052fce0(Raw s, Seg15Host& host);
void credits_collect_children_0052fd70(Raw s, void* out, Seg15Host& host);
void credits_destruct_0052fd80(Raw s, Seg15Host& host);

// ---------------------------------------------------------------------------
// Menu prompt screen (55h, vtables 00CED3C4 / 00CED3B0 at +8h / 00CED3AC at +Ch, the
// 00E18E6C singleton base / 00CED38C at +10h; 005321F0 stores all four).
// A prompt record is 48h bytes: {NativeString +0, dw +8, dw +Ch, NativeString
// +10h, byte +18h, dwords +20h..+34h, NativeString +38h, byte +40h, dw +44h}.
// ---------------------------------------------------------------------------
// 005309F0, widget listener, __thiscall(listener, widget, bool), RET 8: type 3
// -> in session mode 2 (*(game + 5D4h)) the widget's bound record (+70h) gets
// the flag at +D8h; alpha 0.5f when set, else 1.0f.
void menu_prompt_listener_set_toggle_005309f0(void* widget, bool flag, Seg15Host& host);
// 00530CC0 / 00530E00: the 00E18E6C singleton object's destructor body
// (singleton cleared, base vtable 00CE3818), the second with the delete flag.
void singleton_00e18e6c_clear_base_00530cc0(Raw object, Seg15Host& host);
Raw singleton_00e18e6c_deleting_dtor_00530e00(Raw object, std::uint8_t flags, Seg15Host& host);
void menu_prompt_register_005311c0(Raw s, Seg15Host& host);
void menu_prompt_record_copy_construct_00531c90(Raw dst, Raw src, Seg15Host& host);
// 00531F80, __fastcall(list): unlink all nodes, destroy each record (00530F90),
// free the node (00BF65AC).
void menu_prompt_record_list_clear_00531f80(Raw list, Seg15Host& host);
// 005321F0 (Ghidra CG_vector_deleting_dtor_005321f0; it is the class destructor that the
// scalar deleting destructor 00532A00 calls), __thiscall(ECX = screen), RET; SEH 00C6C0EE,
// states 6..2 then -1. The four vtables; page +14h (004C12B0 / 00AA31F0); the record list
// +278h (00531F80, head freed with 00BF65AC, +27Ch = 0); the native string vector +268h
// (004324A0 inlined: 00432050 over [first, last) and free when first is set, then first,
// last, end = 0); the seven 48h records +60h (00BF7C6E with 00530F90); the two native string
// vectors +40h (00BF7C6E, 10h, 2, 004324A0); the singleton base at +0Ch (00530CC0 inlined:
// 00E18E6C = 0, +0Ch = 00CE3818); +8h = 00CEB0FC; 004F71A0.
void menu_prompt_destroy_005321f0(Raw s, Seg15Host& host);
void menu_prompt_collect_children_00532350(Raw s, void* out, Seg15Host& host);
// 005329C0, __thiscall(screen, out), RET 4: +14h, and +18h when that page is visible.
void menu_prompt_collect_visible_pages_005329c0(Raw s, void* out, Seg15Host& host);
// 00532D70, listbox listener (ECX = screen+8h, list, unused), RET 8.
void menu_prompt_listbox_activate_00532d70(Raw listener, void* list, Seg15Host& host);

// ---------------------------------------------------------------------------
// HUD formation screen (2Ah, vtables 00CED6FC, 00CED6E0 at +64h, 00CED6CC at +78h).
// ---------------------------------------------------------------------------
// 005337C0 / 005353A0, __fastcall(ECX out a, EDX out b, stack out c), RET 4.
void read_directional_actions_005337c0(std::uint8_t* a, std::uint8_t* b, std::uint8_t* c, Seg15Host& host);
void hud_formation_read_navigation_actions_005353a0(std::uint8_t* a, std::uint8_t* b, std::uint8_t* c,
                                                    Seg15Host& host);
void hud_formation_show_formation_widgets_00533830(Raw s, Seg15Host& host);
void hud_formation_refresh_order_widgets_00533920(Raw s, Seg15Host& host);
void hud_formation_hide_all_widgets_00533a30(Raw s, Seg15Host& host);
// 00533B20: MOV EAX,[ECX]; JMP [EAX+4] (tail call of the listener's own slot 1).
void hud_formation_listener_call_slot04_00533b20(Raw listener, Seg15Host& host);
// 00533D30, __thiscall(const float* a, const float* b), RET 4: FUCOMIP compares.
bool vec2f_equal_00533d30(const float* a, const float* b);
// 005347C0, __fastcall(message): formation session message destructor body.
void session_message_formation_destruct_005347c0(Raw message, Seg15Host& host);
// 00534BC0, listener at screen+64h, RET 4.
void hud_formation_refresh_if_enabled_00534bc0(Raw listener, Seg15Host& host);
void hud_formation_restore_unit_input_contexts_00534c00(Seg15Host& host);
// 00534E90, __thiscall(input manager, action), RET 4 / 00535EE0 (ECX unused,
// the input manager from 004BEC00), RET 4.
bool input_action_released_positive_00534e90(Raw input, std::int32_t action);
bool input_action_released_this_frame_00535ee0(std::int32_t action, Seg15Host& host);
// 00536100, listener (screen+64h) slot, (unused, list), RET 8.
void hud_formation_listbox_selection_changed_00536100(Raw listener, void* list, Seg15Host& host);
void hud_formation_collect_children_00538d20(Raw s, void* out, Seg15Host& host);
// 00538D60, __thiscall(screen, map, member, float unused), RET 0Ch.
void hud_formation_add_member_marker_00538d60(Raw s, void* map, std::uint32_t member, Seg15Host& host);
Raw hud_formation_construct_00538e30(Raw s, Seg15Host& host);

// ---------------------------------------------------------------------------
// Frame statistics overlay (57h, vtable 00CEDA10) and its statistics object.
// ---------------------------------------------------------------------------
void record_zero_init_0053b9e0(Raw record);  // ten dwords
Raw frame_stats_construct_0053ba10(Raw s, Seg15Host& host);
void frame_stats_enter_0053ba70(Raw s);
// 0053BAB0, cdecl(line, format, ...): _vsprintf_s into a 210h-byte line.
int frame_stats_printf_0053bab0(char* line, const char* format, ...);
void frame_stats_reset_accumulators_0053bb40(Raw stats);
void frame_stats_destruct_0053c440(Raw s, Seg15Host& host);
// 0053C510, __thiscall(stats), RET; ECX = *(game + 19C8h).
void frame_stats_accumulate_0053c510(Raw stats, Seg15Host& host);
void frame_stats_collect_children_0053d220(Raw s, void* out, Seg15Host& host);

// ---------------------------------------------------------------------------
// HUD free camera (2Bh, vtables 00CEDCA8 / observer 00CEDC90 at +8h).
// Effect entry 34h (vtable 00CEDC60, point effect +2Ch); effect record 30h
// (NativeString +1Ch, reference-counted handles +24h, +28h, +2Ch).
// ---------------------------------------------------------------------------
// 0053D2C0, observer slot (ECX = screen+8h, subject), RET 4.
void free_camera_observer_released_0053d2c0(Raw observer, void* subject);
// 0053D700, __fastcall(ECX = source): 00E192B0 = *source, 00E192A9 = 1.
void free_camera_set_matrix_0053d700(const void* source, Seg15Host& host);
void free_camera_effect_entry_construct_0053dc70(Raw entry, Seg15Host& host);
void free_camera_effect_entry_destruct_0053dce0(Raw entry, Seg15Host& host);
void free_camera_effect_record_destroy_0053df40(Raw record, Seg15Host& host);
void free_camera_effect_entry_copy_0053e030(Raw dst, Raw src);
// 0053E0E0, __fastcall(settings block at screen+60h).
void free_camera_acquire_test_effects_0053e0e0(Raw block, Seg15Host& host);
void free_camera_effect_record_copy_0053e300(Raw dst, Raw src, Seg15Host& host);
void free_camera_effect_record_assign_0053e460(Raw dst, Raw src, Seg15Host& host);
std::uint8_t* free_camera_effect_record_copy_backward_0053e6f0(std::uint8_t* first, std::uint8_t* last,
                                                               std::uint8_t* dest_end, Seg15Host& host);
void free_camera_effect_list_clear_0053e850(Raw list, Seg15Host& host);
// 0053FF10, __thiscall(vector, iterator, where, count, const record*), RET 10h.
void free_camera_effect_record_insert_n_0053ff10(Raw vec, std::uint8_t* where, std::uint32_t count, Raw value,
                                                 Seg15Host& host);
void free_camera_construct_effect_lists_005401f0(Raw block, Seg15Host& host);
void free_camera_release_effects_00540260(Raw block, Seg15Host& host);
void free_camera_destruct_005402d0(Raw s, Seg15Host& host);
void free_camera_collect_children_00540380(Raw s, void* out, Seg15Host& host);
Raw free_camera_construct_00540ba0(Raw s, Seg15Host& host);

// ---------------------------------------------------------------------------
// Debug text lists and the command bar (STL instances over small elements).
// IntBool element: {int32, byte} in 8 bytes. ByteSbo element (24h): {byte,
// MSVC 8 std::string at +4 (buffer +8, size +18h, capacity +1Ch), dword +20h}.
// ---------------------------------------------------------------------------
// 00542200 / 00542230, __fastcall(ECX first, EDX last, dest, 3 unused), RET 10h.
std::uint8_t* int_bool_uninit_copy_00542200(const std::uint8_t* first, const std::uint8_t* last, std::uint8_t* dest);
// 00542CB0 / 00542CE0, __fastcall(ECX dest, EDX count, const elem*, 3 unused), RET 10h.
void int_bool_uninit_fill_n_00542cb0(std::uint8_t* dest, std::uint32_t count, const std::uint8_t* value);
// 00542BB0 / 00542BE0, cdecl(first, last, const elem*): fill (8 bytes each).
void pod8_fill_00542bb0(std::uint8_t* first, std::uint8_t* last, const std::uint8_t* value);
// 00543050 / 005430A0, cdecl(first, last, dest_end) -> new dest start.
std::uint8_t* pod8_copy_backward_00543050(const std::uint8_t* first, const std::uint8_t* last, std::uint8_t* dest_end);
// 00543E90 / 005440E0, __thiscall(vector, iterator, where, count, const elem*), RET 10h.
void int_bool_vector_insert_n_00543e90(Raw vec, std::uint8_t* where, std::uint32_t count, Raw value, Seg15Host& host);
// 00542620, __fastcall(list): the rows (24h, vector +14h) get their text and the
// colour by state (+48h inactive, +38h selected, +28h normal); page +8h shown.
void debug_text_list_refresh_rows_00542620(Raw list, Seg15Host& host);
// 00542D70 / 00542EE0 / 005439B0, __fastcall(list): action 46h tail-jumps to
// the first routine, else action 47h to the second.
void debug_text_list_poll_up_down(Raw list, std::uint32_t on_46, std::uint32_t on_47, Seg15Host& host);
// 00543410, __thiscall(bar, index), RET 4: button colours and focus.
void command_bar_set_button_00543410(Raw bar, std::uint32_t index, Seg15Host& host);
// ByteSbo helpers.
void byte_sbo_placement_copy_00543a60(std::uint8_t* slot, const std::uint8_t* src, Seg15Host& host);  // fastcall
void byte_sbo_destroy_range_00543c20(std::uint8_t* first, std::uint8_t* last, Seg15Host& host);  // fastcall, RET 8
std::uint8_t* byte_sbo_copy_00543ca0(const std::uint8_t* first, const std::uint8_t* last, std::uint8_t* dest,
                                     Seg15Host& host);  // cdecl
std::uint8_t* byte_sbo_copy_backward_00543cf0(const std::uint8_t* first, const std::uint8_t* last,
                                              std::uint8_t* dest_end, Seg15Host& host);  // cdecl
std::uint8_t* byte_sbo_uninit_copy_00543d40(const std::uint8_t* first, const std::uint8_t* last, std::uint8_t* dest,
                                            Seg15Host& host);  // fastcall + dest, RET 10h
void byte_sbo_fill_005443b0(std::uint8_t* first, std::uint8_t* last, const std::uint8_t* value,
                            Seg15Host& host);  // cdecl
void byte_sbo_uninit_fill_n_005443f0(std::uint8_t* dest, std::uint32_t count, const std::uint8_t* value,
                                     Seg15Host& host);  // fastcall + value, RET 10h

// ===========================================================================
// Segment 22
// ===========================================================================
// 005CD820, __thiscall(array {data, size}, i), RET 4: grow to i + 1, &data[i] (5Ch each).
std::uint8_t* vehicle_camera_array_at_005cd820(Raw array, std::int32_t index, Seg15Host& host);
// Movie camera debug screen (36h, vtables 00CF1968 / 00CF1950 at +8h).
void movie_camera_debug_destruct_005ce4d0(Raw s, Seg15Host& host);
void movie_camera_debug_collect_children_005ce5d0(Raw s, void* out, Seg15Host& host);
Raw movie_camera_debug_construct_005ce610(Raw s, Seg15Host& host);

// Multiplayer chat screen (15h, vtables 00CF1998 / text listener 00CF19C0 at +8h).
// Overlay line (18h): {dword, WideString +4, NativeString +0Ch, dword +14h};
// history line (14h): {WideString, NativeString +8h, dword +10h}. Lists at
// +68h (overlay), +74h and +80h (history), each {alloc, head, size}.
void wide_string_append_005cf090(Raw dst, Raw src, Seg15Host& host);  // __thiscall, RET 4
void chat_overlay_line_destroy_005cf0f0(Raw line, Seg15Host& host);
void chat_history_line_destroy_005cf180(Raw line, Seg15Host& host);
void multi_chat_register_005cf260(Raw s, Seg15Host& host);
// 005CF320, __fastcall(ECX channel, EDX kind, const NativeString* a, const NativeString* b), RET 8.
void session_post_system_chat_line_005cf320(std::int32_t channel, std::int32_t kind, Raw a, Raw b, Seg15Host& host);
// 005CF3D0, __fastcall(ECX side, EDX unused, a, b), RET 8: side 0 -> (9, 5), side 1 -> (10, 5).
void warning_manager_post_loss_chat_line_005cf3d0(std::int32_t side, Raw a, Raw b, Seg15Host& host);
void chat_overlay_line_copy_005cf460(Raw dst, Raw src, Seg15Host& host);
void chat_history_line_copy_005cf4f0(Raw dst, Raw src, Seg15Host& host);
// 005CF570, __thiscall(a, out, b), RET 8 -> EAX out: out = a + b.
Raw wide_string_concat_005cf570(Raw a, Raw out, Raw b, Seg15Host& host);
// 005CFA40 / 005CFAE0, (next, prev, const line*), RET 0Ch: new node (20h / 1Ch).
std::uint8_t* chat_overlay_list_buy_node_005cfa40(void* next, void* prev, Raw line, Seg15Host& host);
std::uint8_t* chat_history_list_buy_node_005cfae0(void* next, void* prev, Raw line, Seg15Host& host);
void multi_chat_clear_logs_005cfb80(Raw s, Seg15Host& host);
Raw multi_chat_construct_005cfda0(Raw s, Seg15Host& host);
void multi_chat_destroy_005cfe60(Raw s, Seg15Host& host);
void multi_chat_exit_005cff40(Raw s, Seg15Host& host);
void multi_chat_collect_children_005d0020(Raw s, void* out, Seg15Host& host);
void multi_chat_refresh_mode_label_005d07e0(Raw s, Seg15Host& host);
void multi_chat_send_current_line_005d0b10(Raw s, Seg15Host& host);
// 005D1000, __thiscall(screen, bool), RET 4.
void multi_chat_set_team_only_005d1000(Raw s, bool team_only, Seg15Host& host);
// 005D1020 / 005D10F0 / 005D14B0: text-listener slots (ECX = screen+8h).
void multi_chat_clamp_input_length_005d1020(Raw listener, Seg15Host& host);
void multi_chat_submit_input_005d10f0(Raw listener, Seg15Host& host);
void multi_chat_on_open_request_005d14b0(Raw listener, Seg15Host& host);
// 005D11F0, __thiscall(screen, mode), RET 4 (1 lobby, 2 typing, 3 in mission, 4 overlay).
void multi_chat_set_chat_mode_005d11f0(Raw s, std::int32_t mode, Seg15Host& host);

// Multiplayer error screen (1Dh, vtables 00CF1BF4 / 00CF1BE0 at +8h / listener 00CF1BC0 at +Ch).
// +10h page _MultiError, +14h / +18h texts, +20h..+28h buttons, +50h buttons
// mode, +54h error code; 00E08898 is the join-failure detail.
void multi_error_exit_005d1f90(Raw s, Seg15Host& host);
// 005D1FB0 / 005D3440 (and 005D35A0's shape), __thiscall(listener, widget, bool), RET 8.
void listener_focus_text_dim_005d1fb0(void* widget, bool flag, Seg15Host& host);
void multi_error_dismiss_005d2010(Seg15Host& host);
// 005D2240, __thiscall(screen, float), RET 4.
void multi_error_update_005d2240(Raw s, float seconds, Seg15Host& host);
// 005D2310 / 005D23E0: listener slots (ECX = screen+8h).
void multi_error_on_activate_005d2310(Raw listener, void* widget, Seg15Host& host);
void multi_error_on_button_005d23e0(Raw listener, void* button, Seg15Host& host);
void multi_error_register_005d2440(Raw s, Seg15Host& host);
void multi_error_show_message_005d24f0(Raw s, Seg15Host& host);
// 005D2E70, __thiscall(screen, code), RET 4.
void multi_error_set_error_code_005d2e70(Raw s, std::int32_t code, Seg15Host& host);
// 005D2EB0 (Ghidra CG_vector_deleting_dtor_005d2eb0; the class destructor that 005D2F70
// calls), __thiscall(ECX = screen), RET; SEH 00C73DE5, states 2, 1, -1. The three vtables;
// page +10h (004C12B0 / 00AA31F0); the two native string vectors +30h (00BF7C6E, 10h, 2,
// 004324A0); +8h = 00CEB0FC; 004F71A0.
void multi_error_destroy_005d2eb0(Raw s, Seg15Host& host);
void multi_error_collect_children_005d2f60(Raw s, void* out, Seg15Host& host);

// Multiplayer pause screen (19h, vtables 00CF1D70 / 00CF1D5C at +8h / 00CF1D3C at +Ch).
// Prompt callbacks, __fastcall(ECX result): 005D3410 leave (pop the request,
// end the scene), 005D3430 quit flag 00E1AE75.
void prompt_result_leave_multi_menu_005d3410(std::int32_t result, Seg15Host& host);
void prompt_result_set_quit_flag_005d3430(std::int32_t result, Seg15Host& host);
// 005D3440 (RET 8) / 005D35A0 (RET 4, flag from widget byte +D4h): types 3
// (alpha), 6 (+88h(flag, 0, 1.0f)) and 12h (+84h(flag)).
void listener_focus_dim_typed_005d3440(void* widget, bool flag, Seg15Host& host);
void listener_refresh_dim_typed_005d35a0(void* widget, Seg15Host& host);
// 005D34E0, listener slot (unused, listbox), RET 8: 00E19550 = selected index.
void pause_multi_store_selection_005d34e0(void* listbox, Seg15Host& host);
Raw pause_multi_construct_005d3540(Raw s, Seg15Host& host);
void pause_multi_destroy_005d3710(Raw s, Seg15Host& host);
// 005D3790, widget listener at screen+0Ch, RET 4.
void pause_multi_on_activate_005d3790(Raw listener, void* widget, Seg15Host& host);
void pause_multi_register_005d38a0(Raw s, Seg15Host& host);
void pause_multi_exit_005d3960(Raw s, Seg15Host& host);
void pause_multi_collect_children_005d3990(Raw s, void* out, Seg15Host& host);

// Multiplayer game lobby (and its helpers). Player slots: 8 x 118h at game+748h
// (+8h present, +9h AI, +0Ch ready, +58h name); slot records game+18CCh[i];
// local slot index game+18ECh, local player game+18C8h.
bool lobby_slot_swap_has_pair_005d4910(Raw lobby);
// 005D49E0, __fastcall(ECX src): the fourteen stored options 00E1955C..00E19590.
void lobby_settings_copy_stored_options_005d49e0(const std::uint32_t* src, Seg15Host& host);
bool game_is_standard_game_mode_005d4a10(Seg15Host& host);
bool session_record_flag_274_005d50e0(Raw record);
// 005D5130, __fastcall(game): the local slot record or null.
void* game_local_slot_record_005d5130(Raw game);
bool multi_lobby_local_slot_valid_005d51f0(Seg15Host& host);
bool multi_lobby_local_player_ready_005d5220(Seg15Host& host);
// 005D5260: the ledger calls it CountUnreadyPlayers; the body answers whether
// the lobby may start (see the doc).
bool lobby_ready_to_start_005d5260(Seg15Host& host);
bool multi_game_lobby_settings_open_005d5490(Raw s, Seg15Host& host);
// 005D5630, __fastcall(ECX row): the settings slot whose menu row (005D5500) is row, or -1.
std::int32_t lobby_settings_slot_for_menu_row_005d5630(std::int32_t row, Seg15Host& host);
// 005D5660, __fastcall(ECX mode): 00E19564 = mode; modes 0..3 apply 00E19568.
void multi_mode_selector_set_mode_005d5660(std::int32_t mode, Seg15Host& host);
// Prompt results, __fastcall(ECX result).
void prompt_result_kick_player_005d5690(std::int32_t result, Seg15Host& host);
void prompt_result_accept_slot_swap_005d57f0(std::int32_t result, Seg15Host& host);
void multi_game_lobby_refresh_local_ready_005d5790(Raw s, Seg15Host& host);
// 005D5870, __thiscall(screen, bool), RET 4.
void multi_game_lobby_set_editable_005d5870(Raw s, bool editable, Seg15Host& host);
// 005D5930, __thiscall(screen, id), RET 4: the element count (8-byte elements)
// of the setting row with that id (rows 24h at +2E4h..+2E8h, id +0, vector +4h).
std::int32_t multi_game_lobby_find_setting_row_005d5930(Raw s, std::int32_t id);
// 005D62B0: apply the mode and the fourteen settings to the game and broadcast them.
void multi_game_lobby_apply_settings_005d62b0(Seg15Host& host);
// 005D6350, __thiscall(screen, value), RET 4.
void multi_game_lobby_set_setting_for_row1_005d6350(Raw s, std::uint32_t value, Seg15Host& host);
// 005D63A0, __fastcall(screen): the n-th present player slot for the player listbox (+A8h) selection.
void* multi_game_lobby_selected_player_slot_005d63a0(Raw s, Seg15Host& host);
bool multi_game_lobby_slots_meet_content_masks_005d6ba0(Seg15Host& host);
void session_dismiss_load_prompt_005d6cd0(Seg15Host& host);
// 005D8B30: a loaded scene (game+5FCh, +914h) and the local slot record's map
// (+28h) unlocked in the tactical library (005D8650 index, 0050FD10).
bool multi_game_lobby_local_map_available_005d8b30(Seg15Host& host);
// Prompt callbacks (ECX result) that tail-jump with ECX = *(*00E198B4 + 48h).
void prompt_result_leave_lobby_005d98c0(std::int32_t result, Seg15Host& host);
void prompt_result_close_game_005d9fe0(std::int32_t result, Seg15Host& host);
// 005D9D60, __fastcall(screen).
void multi_game_lobby_on_local_slot_changed_005d9d60(Raw s, Seg15Host& host);
// 005D9EF0, __fastcall(screen).
void multi_game_lobby_close_game_005d9ef0(Raw s, Seg15Host& host);
// 005D9F50, __fastcall(screen).
void multi_game_lobby_request_selected_slot_005d9f50(Raw s, Seg15Host& host);
void multi_game_lobby_apply_local_map_005d9f90(Seg15Host& host);
void two_vector_record_construct_005dac60(Raw record);
// 005DAF10, __thiscall(dst, src), RET 4: id +0 and two checked string vectors (+4h, +14h).
void lobby_map_entry_copy_005daf10(Raw dst, Raw src, Seg15Host& host);
// 005D8410, __fastcall(screen).
void multi_game_lobby_leave_to_menu_005d8410(Raw s, Seg15Host& host);
// 005D8A70, __fastcall(screen): delete every row object (+234h vector), then erase all.
void multi_game_lobby_delete_row_objects_005d8a70(Raw s, Seg15Host& host);
// 005DACA0, __fastcall(entry): both string vectors (+14h, +4h) destroyed and freed.
void lobby_map_entry_destruct_005daca0(Raw entry, Seg15Host& host);
// 005D8700, __fastcall(screen): 00E08958 = the player listbox (+A8h) selection;
// the selected row widget's (+250h[i]) player_name_Text text goes to 00E195D4.
void multi_game_lobby_on_player_row_activate_005d8700(Raw s, Seg15Host& host);

// --- cc13-fe7: lobby routines (docs/CC13_FE_7.md) ---
// 005D5340 (no ECX use), plain RET: when the online service's +198h answers
// and its byte +4Dh is set, enable AI on all eight session slots (00771E80(i,
// 0)); else, for game modes 0..3 (004BCA50), mark the sides (dword +20h) of the
// eight participant records (game+750h + k*118h, active +0, not AI +1) and
// enable AI (flag 1) on the first slot record (game+18CCh[k], side +28h) of
// each side without one, 0 on the rest.
void multi_game_lobby_reset_slots_005d5340(Seg15Host& host);
// 005D67E0 __thiscall(screen, widget), RET 4: move the focus (+1Ch) with the
// widgets' +84h(0/1), remember the player listbox row when leaving +A4h,
// update the input manager with 0, set the focus bytes +1B1h / +125h / +89h
// and [+A4h]+78h, and (when [+228h]+85h is clear) select the remembered row
// and add 100 to the scroll offset +2D0h, or clear both listboxes.
void multi_game_lobby_set_focus_005d67e0(Raw s, void* widget, Seg15Host& host);
// 005D6C10 __fastcall(screen): map icon texture (004FD500) on widget +2Ch,
// state 0.
void multi_game_lobby_refresh_map_icon_005d6c10(Raw s, Seg15Host& host);
// 005D8530 __fastcall(slot record): copy the record's name (+58h) to 00E195CC,
// 004BB850(record), raise "FE.multi_lobby_kickplayerconfirm" (kind 1, result
// 005D5690, title 00E195CC) and focus its last row; nothing for null.
void multi_game_lobby_confirm_kick_005d8530(Raw record, Seg15Host& host);
// 005D8800 (ECX = requesting slot, EDX = target slot), plain RET: with no
// pending swap (00E0895C == -1) and EDX the local slot (game+18ECh), store the
// pair in 00E0895C / 00E08960 and raise "FE.multi_lobby_wantswapslot" (kind 1,
// result 005D57F0, title = the requester's name, game+7A0h + slot*118h), focus
// the first row; else refuse (00771CC0(ECX slot, EDX slot)).
void multi_game_lobby_prompt_swap_slot_005d8800(std::int32_t requester, std::int32_t target, Seg15Host& host);
// 005D8950 (EDX = slot), plain RET: when a swap is pending (00E08958 != -1),
// raise "FE.multi_lobby_swapslotrefused" (kind 2, no result), clear 00E08958
// and set the slot's retry time 00E1959C[slot] = game+64Ch + 30.
inline constexpr double kLobbySwapRetrySeconds = 30.0;  // 00CE7630
void multi_game_lobby_show_swap_refused_005d8950(std::int32_t slot, Seg15Host& host);
// 005D98E0 __fastcall(screen): +2DCh = 005D8B30; tell the tactical library;
// when available draw 004FD3A0's icon on widget +24h (state 2, +88h(2) when
// a texture came back, else +88h(0)), else +88h(1).
void multi_game_lobby_refresh_map_availability_005d98e0(Raw s, Seg15Host& host);
// 005D9A10 __fastcall(screen): clear the listboxes +A8h and +94h, dismiss
// prompt slot 1, byte [*00E19698]+4 = 0, delete the row objects (005D8A70),
// reset the GUI screens, scroller +30h range 0 and no content, +ACh = 0.
void multi_game_lobby_exit_005d9a10(Raw s, Seg15Host& host);
// 005D9C50 (ECX passed through, one unused stack argument), RET 4 -> EDX:EAX:
// the selected map's (005D8650) number suffix after the last '_' as a mask,
// 1 << (atol(suffix) - 1) through __allshl (0 for an empty name or a shift of
// 64 or more).
std::uint64_t multi_game_lobby_map_mask_005d9c50(Seg15Host& host);
// 005D9D90 __thiscall(ECX = screen+8h listener, widget, flag), RET 8: player
// listbox activation (select a player, or ask to swap slots no sooner than
// the slot's retry time; a beep through the sound request otherwise).
void multi_game_lobby_on_listbox_activate_005d9d90(Raw s, void* widget, std::uint32_t flag, Seg15Host& host);
// 005DA000 __thiscall(ECX = screen+0Ch listener, widget), RET 4: list
// selection routing by widget type (2: a player row; 12h: the buttons +A4h,
// +20h, +8Ch, +28h).
void multi_game_lobby_on_list_select_005da000(Raw s, void* widget, Seg15Host& host);
// 005DA210 __fastcall(screen): leave the lobby, through a confirmation prompt
// when hosting with more than one player or when a client.
void multi_game_lobby_confirm_leave_005da210(Raw s, Seg15Host& host);
// 005DA530 __fastcall(screen): apply the map list entry +2F4h (when +2F0h and
// +2F4h are set) as the command-line text and into 00E195C4, select scene
// record *00E19594, store the map mask (+2C0h/+2C4h, game+2060h/+2064h with
// +2C8h/+2CCh), apply the settings, +10h = 1 and tail-call 005DA210.
void multi_game_lobby_apply_selected_map_005da530(Raw s, Seg15Host& host);
// 005D71B0 __fastcall(screen) -> AL: with a local player (game+18C8h >= 0,
// slot game+18ECh in 0..7) and a loaded scene (game+5FCh, byte +914h): as host
// (game+1FE4h == 1), when 005D6BA0 fails raise "globals.some_clients_incompatible"
// (prompt slot 2, kind 2) and return 0; when 005D5260 says ready, dismiss the
// load prompt (005D6CD0), byte +271h = 1, byte [*00F8A2FC]+75h = 1, collect the
// incompatible clients (0076F360) and return 1. Otherwise (a session that is not
// closing, and either host or a client whose [game+207Ch]+C4h is set) refresh
// the local ready state (005D5790) and request ready for game+18C8h with flag
// (local slot record +0Ch == 0); return 0.
bool multi_game_lobby_check_client_compatibility_005d71b0(Raw s, Seg15Host& host);
// 005D6060 __thiscall(ECX = screen+0Ch listener, widget, bool focus), RET 8:
// widget type 3 gets alpha 0.5 / 1.0 (00CE3800 / 00D7A24C); the five buttons
// (+A4h, +20h, +8Ch, +28h, +150h) are left alone; +90h and +154h only when
// bytes +78h / +1A0h are clear; with the settings page open (005D5490) only
// +220h and +224h; the three side widgets +144h / +148h / +14Ch switch
// frames together; any other widget shows the focus through +88h(focus) for
// type 6 or +84h(focus) for type 12h.
inline constexpr float kLobbyFocusDimAlpha = 0.5f;  // 00CE3800
void multi_game_lobby_on_focus_005d6060(Raw s, void* widget, bool focus, Seg15Host& host);

// ---------------------------------------------------------------------------
// cc13-fe10 additions (docs/CC13_FE_10.md).
// ---------------------------------------------------------------------------
struct Fe10Host : Seg15Host {
    // Scroller block routines (ECX = the block): 00683320 end a thumb drag,
    // 00683300(int, float, int) set the auto-scroll state.
    virtual void scroller_end_thumb_drag_00683320(Raw scroller) = 0;
    virtual void scroller_set_auto_scroll_00683300(Raw scroller, std::int32_t a, float speed, std::int32_t c) = 0;
    // 00AA8240, ECX = widget, (const vec3*): place the widget at a resolved position.
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float xyz[3]) = 0;
    // 0076CC40, ECX = [*00E188A8]+1EF0h, (slot, level): the slot's AI skill level.
    virtual void session_set_slot_ai_skill_level_0076cc40(std::int32_t slot, std::int32_t level) = 0;
    // 00AAB4C0(0), ECX = template widget: clone it (no parent).
    virtual void* widget_clone_00aab4c0(void* template_widget) = 0;
    // 00A9D750(row, 0, 0), ECX = listbox: append a row.
    virtual void listbox_insert_row_00a9d750(void* listbox, void* row) = 0;
    // 00A9AC50, ECX = listbox: the row block (+8h is the row count).
    virtual Raw listbox_row_block_00a9ac50(void* listbox) = 0;
    // 00A9CF30(a, b), ECX = listbox: reset the highlight.
    virtual void listbox_reset_highlight_00a9cf30(void* listbox, std::int32_t a, std::int32_t b) = 0;
    // Widget virtuals +7Ch (int) and +80h (int).
    virtual void widget_virtual_7c(void* widget, std::int32_t value) = 0;
    virtual void widget_virtual_80(void* widget, std::int32_t value) = 0;
    // 004E1CA0: the award tracker singleton (std::map at +7Ch, head +80h,
    // size +84h; +9Ch the pending-award count).
    virtual Raw award_tracker_004e1ca0() = 0;
    // 004B7DE0, ECX = &iterator: std::_Tree iterator increment (the node).
    virtual Raw tree_iterator_next_004b7de0(Raw node) = 0;
    // 004D48A0 copy-construct of the pooled string list at list, then its +8h
    // count, then 004D05E0 + _free(00BF65AC) on the copy.
    virtual std::uint32_t pooled_string_list_copy_count_004d48a0(Raw list) = 0;
    // Pause screen [*00E198C4+D8h] (ECX): 00683820(1) on its scroller +9Ch,
    // 006037D0 hints update.
    virtual void pause_screen_scroller_update_00683820() = 0;
    virtual void pause_screen_update_hints_006037d0() = 0;
    // 004C43C0(action), ECX = *00E188A8: action pressed this frame.
    virtual bool action_pressed_004c43c0(std::int32_t action) = 0;
    // 00425D10: the menu command (prompt) screen singleton (+4h busy byte).
    virtual Raw menu_command_screen_00425d10() = 0;
    // 0054B530, ECX = *00E1930C: rebuild the command bar from five triples.
    virtual void command_bar_rebuild_0054b530(const MainMenuCommandArgument (&arguments)[5]) = 0;
    // 004C1E90(id) then 00427190, ECX = that owner.
    virtual void auto_test_step_00427190(std::int32_t id) = 0;
    // 004C5E60, ECX = dst wide string, (const char*): widen a C string.
    virtual void wide_string_from_cstring_004c5e60(Raw dst, const char* source) = 0;
    // Widget geometry, ECX = widget: 00AA67F0 local position (vec3*), 00AA6740
    // size (vec2*), 00AA7DC0(const vec3*) set local position and bounds,
    // 00AB6BD0 normalized text height (ST0), vtable +58h set size (const vec2*).
    virtual const float* widget_local_position_00aa67f0(void* widget) = 0;
    virtual const float* widget_size_00aa6740(void* widget) = 0;
    virtual void widget_set_local_position_00aa7dc0(void* widget, const float xyz[3]) = 0;
    virtual double text_normalized_height_00ab6bd0(void* text) = 0;
    virtual void widget_set_size_58(void* widget, const float xy[2]) = 0;
    // 00AAB4C0(parent), ECX = template widget: clone under a parent.
    virtual void* widget_clone_under_00aab4c0(void* template_widget, void* parent) = 0;
    // Singleton lifetime: 00415350 the manager (critical section at +10h,
    // users +18h; EnterCriticalSection 00CE2218 / LeaveCriticalSection
    // 00CE2210), 00BD0C30(object), ECX = manager: register for teardown.
    virtual Raw singleton_lifetime_manager_00415350() = 0;
    virtual void critical_section_enter(void* section) = 0;
    virtual void critical_section_leave(void* section) = 0;
    virtual void singleton_lifetime_register_00bd0c30(Raw manager, void* object) = 0;
    // 00BE9760, ECX = block: the fingerprint payload constructor (204h bytes).
    virtual void* fingerprint_payload_construct_00be9760(void* block) = 0;
    // 00424C40: the game settings singleton (free-camera effect records:
    // vector +740h..+744h, 30h bytes each).
    virtual Raw game_settings_00424c40() = 0;
    // Store a refcounted handle into slot: InterlockedIncrement (00CE221C) the
    // new one, InterlockedDecrement (00CE2220) the old one and its vtable +0 at
    // zero.
    virtual void handle_assign(Raw slot, void* handle) = 0;
    // Free-camera effect records: 00540CA0(const record*), ECX = vector:
    // push_back; 0053E0E0, ECX = record: the built-in test effects; 0053DF40,
    // ECX = record: destroy.
    virtual void free_camera_effect_vector_push_back_00540ca0(Raw vector, Raw record) = 0;
    virtual void free_camera_acquire_test_effects_0053e0e0(Raw record) = 0;
    virtual void free_camera_effect_record_destroy_0053df40(Raw record) = 0;
    // 0093CC80, ECX = screen+8h: the simple 3D scene sub-object destructor.
    virtual void simple_3d_scene_destroy_0093cc80(Raw scene) = 0;
};

// Widget focus byte (+D4h) the lobby listeners read.
inline constexpr std::size_t kWidgetFocusedByte = 0xD4;

// 005D5D50 __thiscall(ECX = screen+0Ch listener, widget), RET 4: the lobby's
// activation hook. Type 2 widgets are ignored; type 3 gets alpha 0.5 / 1.0 by
// its focus byte. The buttons +20h, +8Ch, +28h, +150h are left alone. The
// scroll arrows +9Ch/+98h, +13Ch/+138h, +BCh/+B8h stop their pane's auto
// scroll (00683300(0, 0.0, 0) on +30h, +158h, +CCh) and the thumbs +90h,
// +154h, +C8h end a thumb drag (00683320 on the same blocks); with the
// settings page open only +220h / +224h reach the common tail; the three side
// widgets +144h / +148h / +14Ch switch frames together (the multiplayer
// side flag [*00E198B4+54h]+8Ch picks which pair is lit). The tail shows the
// focus through +88h(focus, 0, 1.0) for type 6 or +84h(focus) for type 12h.
void multi_game_lobby_on_activate_005d5d50(Raw s, void* widget, Fe10Host& host);

// 005D6E30 __fastcall(screen): the map preview icons. "settings_Group" is
// found in the page +14h and its "map_001_Icon" gives a resolved position that
// every widget of the pointer array +1F8h..+1FCh takes (00AA8240); the page
// +18h's own "map_001_Icon" does the same for +244h..+248h.
void multi_game_lobby_build_map_preview_005d6e30(Raw s, Fe10Host& host);

// 005D9660 __thiscall(ECX = screen, bool step_a, bool step_b), RET 8: step the
// AI of the selected "Players_Listbox" slot (the listbox is re-found into
// +A8h from page +14h). Nothing happens while the online service (vslot
// +198h) is busy with byte +4Dh set, or with neither flag. An empty or
// non-AI slot (record byte +0Ah clear) outside the standard game mode enables
// AI and sets level 3 (5 with step_b); otherwise the slot's kind +2Ch moves:
// 1/3 -> 4 on step_a, 4 -> 5 on step_a, 4 -> 1 (standard mode) / 3 on step_b,
// 5 -> 4 on step_b, 1/3 -> 5 on step_b in the standard mode unless byte
// 00F88A30 is set; outside the standard mode the remaining cases disable AI.
// Ledger name RefreshPlayersListbox is wrong (renamed, docs/CC13_FE_10.md).
void multi_game_lobby_step_selected_slot_ai_005d9660(Raw s, bool step_a, bool step_b, Fe10Host& host);

// Multiplayer pause menu (19h). Nine rows, shown by the flag bytes
// 00E19544..00E1954C; the keys are the char* cells 00E0889C..00E088BC
// (FE.multi_ingame_resume first); 00E19550 remembers the selected row.
inline constexpr std::uint32_t kPauseRowFlags = 0x00E19544;
inline constexpr std::uint32_t kPauseRowKeys = 0x00E0889C;
inline constexpr std::uint32_t kPauseRememberedRow = 0x00E19550;
inline constexpr int kPauseRowCount = 9;
inline constexpr double kPauseIconScreenWidth = 960.0;  // 00CEC380
inline constexpr double kPauseIconHalf = 0.5;           // 00D7A280
inline constexpr double kPauseIconGap = 0.014999999664723873;   // 00CEE6C8
inline constexpr double kPauseIconLift = 0.0010000000474974513;  // 00D7A318
inline constexpr float kPauseInputDelay = 0.1f;          // 00D7A2F0

// 005D39A0 __fastcall(ECX = listbox, EDX = wanted row): rebuild the rows.
// The listener is dropped, the rows cleared, "MainListbox_Text" is the row
// template; each shown row is a clone with its key (row 2, the awards row, is
// greyed through +7Ch(0/1) by the tracker's +9Ch). The selection is the
// wanted row's position, or for wanted row 0 the remembered row clamped to
// the row count (and stored back). Then +80h(1), the pause listener
// ([*00E198C4+E4h]+8h or null) and 00A9CF30(1, 1).
void pause_multi_build_menu_rows_005d39a0(void* listbox, std::int32_t wanted, Fe10Host& host);

// 005D3B40 __thiscall(ECX = screen): enter. Frame 0 titled "ingame.paused"
// when game+218Ch is set, else "ingame.pause_multi_title"; the listbox +20h
// ("Main_Listbox" in page +10h) with the arrow icons "felki_bal_Icon" (+14h)
// and "felki_jobb_Icon" (+18h), both hidden; listener +8h, focus; byte +1Ch
// cleared; the row flags (row 1 off, row 5 for a live session whose local
// slot record +1Bh is clear, row 6 the opposite); rows rebuilt at 0; the arrow
// icons are placed beside the awards row ((width/960)*0.5 + 0.015 to either
// side, 0.001 lower); +24h = 0.1.
void pause_multi_enter_005d3b40(Raw s, Fe10Host& host);

// 005D3F20 __thiscall(ECX = screen, float seconds), RET 4: update. With the
// pause hint page open, its scroller and hints update; otherwise the arrow
// icons show while any award-tracker entry has a non-empty string list. While
// +24h is positive in a live session it counts down; else action 4Bh or 1
// with no prompt raised closes the hint page (005FD890(0, 1)) or toggles the
// pause menu and forgets the row. Then the command bar (select A2h, navigate
// B3h, back A3h), an empty help line and auto-test step 10h.
void pause_multi_update_005d3f20(Raw s, float seconds, Fe10Host& host);

// 005D92B0 __thiscall(ECX = lobby screen, const NativeString* name, int kind),
// RET 8: find name (case-insensitive, equal lengths) in the string vector
// (+4h: first +8h, last +0Ch, 8-byte elements) of the 24h-byte map records
// +2E4h..+2E8h whose dword +0 equals kind; on a hit copy it into the native
// string 00E195BC and store record / string index in +2F0h / +2F4h. An empty
// name clears +2F0h / +2F4h (-1) and the strings 00E195BC / 00E195C4. No hit
// recurses with "" and kind -1, then empties game+600h.
inline constexpr std::uint32_t kLobbyMapNameString = 0x00E195BC;
inline constexpr std::uint32_t kLobbySubSceneString = 0x00E195C4;
void multi_game_lobby_select_map_by_name_005d92b0(Raw s, Raw name, std::int32_t kind, Fe10Host& host);

// 005D09A0 __thiscall(ECX unused, text, const NativeString* sender, colour,
// const WideString* message, unused), RET 14h: a chat row. With an empty
// sender the message is submitted as is, otherwise sender (or the empty C
// string 00E19530) + L": " + message; the colour is config +208h + 10h*index
// through vtable +50h.
inline constexpr std::uint32_t kChatEmptyName = 0x00E19530;
inline constexpr std::uint16_t kChatSeparator[3] = {u':', u' ', 0};  // 00CF1A84
void multi_chat_format_message_row_005d09a0(void* text, Raw sender, std::int32_t colour, Raw message,
                                            Fe10Host& host);

// 005D0C60 __thiscall(ECX = chat screen, template text, list widget), RET 8 ->
// AL: when +8Dh is set, rebuild the rows from the list chosen by +40h (1/2:
// std::list +80h, rows downward; 3: +68h with fields +0Ch/+14h/+1Ch; 4: +74h;
// rows upward), each a clone of the template under the list widget placed at
// the running position and shown by +8Dh; the list widget is resized to its
// width by the covered height. Clears +8Dh and returns 1 (0 when +8Dh was
// clear; another mode only clears it).
bool multi_chat_rebuild_message_rows_005d0c60(Raw s, void* row_template, void* list, Fe10Host& host);

// 005D14D0 __thiscall(ECX = chat screen): bind the layout of page +3Ch:
// "ChatModeOn_Group" +44h (its "SendMessage_Text" +5Ch and hidden
// "MessageTemplate_Text" +54h), "ChatModeOff_Group" +48h (hidden template
// +50h), "Lobby_Group" +4Ch ("SendMessage_Text" +60h); input string +20h
// emptied, mode +40h = 0, the groups hidden, byte +64h = 0, the editor
// (screen+8h) inactive on +5Ch, +60h hidden, mode label refreshed, and the
// sound request byte +1Ah set.
void multi_chat_load_layout_005d14d0(Raw s, Fe10Host& host);

// 0053BC00 __cdecl() -> the fingerprint payload singleton (*0109DB78),
// created under the lifetime manager's lock (double-checked: 204h bytes,
// 00BE9760) and registered for teardown (00BD0C30).
inline constexpr std::uint32_t kFingerprintPayloadGlobal = 0x0109DB78;
void* fingerprint_payload_get_singleton_0053bc00(Fe10Host& host);

// 00540D50 __thiscall(ECX = free-camera settings block, screen+64h): one
// effect record per game-settings record (+740h, 30h bytes): floats from
// +8h, +0Ch, +14h, +18h, +1Ch, +20h, the byte +10h, the name +0h and the
// three refcounted handles +24h, +28h, +2Ch, pushed into the vector +4h; with
// no settings records the built-in test effects (0053E0E0) are pushed.
void free_camera_bind_settings_00540d50(Raw block, Fe10Host& host);

// 00540FE0 __thiscall(ECX = free-camera screen): register. Base register;
// block +64h: +88h = 0.0, +84h = 0, +8Ch = 0.0, then 00540D50; page
// "GUI_freecam" into +1Ch, its hidden "weapon_Text" into +20h.
void free_camera_register_00540fe0(Raw s, Fe10Host& host);

// 00538A80 __thiscall(ECX = HUD formation screen): destructor body. Vtables
// 00CED6FC / 00CED6F8 (+8h) / 00CED6E0 (+64h) / 00CED6CC (+78h); the pages
// +98h, +A4h, +9Ch, +A0h, +A8h removed; the handles of the list +1A0h (node
// +8h) released and the list freed with its head; the list +198h freed; the
// vector +12Ch..+134h freed; the trees +11Ch and +110h erased (00538360 over
// [begin, end)) and their heads freed; +78h back to the listener base
// 00CEB0FC; the callback owner +64h, the scene +8h and the base destroyed.
void hud_formation_destruct_00538a80(Raw s, Fe10Host& host);

// 0052CF60 __thiscall(ECX = HUD counters screen, const record* r), RET 4:
// apply a counter record by its kind +20h. Kind 1: remove (+24h) the entry
// +18h, or show it as a score pair (+25h: strings +28h / +30h, values +38h /
// +3Ch) or a score. Kind 0: remove counter A (+24h), or gather the units of
// the id words +40h[0..+44h) from the two object registries (id below
// *00F89A10: *00F89A54 at (id - *00F89A0C) * 10h + 0Ch, else *00F89AA8 at
// (id - *00F89A60) * 10h + 0Ch) into a pointer array and show their hit
// points with the text +4Ch (0052BFA0), then free the array.
void hud_counters_apply_counter_record_0052cf60(Raw s, Raw r, Fe10Host& host);

// 00529A30 __thiscall(ECX = HUD counters screen): re-lay the counters out.
// Entries (pointer array +20h) whose widget +0 is hidden (vtable +38h) and
// whose bytes +A5h..+A9h are all clear are erased; the row starts at x =
// 0.07 (-0.07 wide) or, with [*00E198C4+C0h]+4 set or the support manager in
// its mode (006529E0), 0.385 (0.23 wide); each kept entry's widget goes to x
// and x += size(+2Ch).w + 0.0305; when the first entry sat left of -900 the
// entries are parked 1000 further left again; tail jump to 00529810.
inline constexpr double kCounterRowGap = 0.030500000342726707;  // 00CED060
inline constexpr float kCounterRowStart = 0.07000000029802322f;         // 00CED070 (00CED074 wide: -)
inline constexpr float kCounterRowStartManager = 0.38499999046325684f;  // 00CED068
inline constexpr float kCounterRowStartManagerWide = 0.23000000417232513f;  // 00CED06C
void hud_counters_relayout_00529a30(Raw s, Fe10Host& host);

// ---------------------------------------------------------------------------
// Packet cc13-trivialslots: one ordinary function per kTrivialSlots row, re-read from
// the disk bytes (local generator, rows checked against the image). `Raw` is the
// ECX object; stack arguments the body never reads are noted in the ABI and not
// passed. Adjustor and tail-jump rows appear only where the target is an existing
// declaration; the descriptive table above stays as documentation.
// ---------------------------------------------------------------------------
// 00527D30 MOV EAX,0Eh; RET (B8 0E 00 00 00 C3; next byte CC).
// BSP_ControlLayoutScreen_ScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 14, RET.
std::int32_t control_layout_screen_screen_id_00527d30(Raw) noexcept;
// 00527D40 SUB ECX,8h; JMP 00527E40 (83 E9 08 E9 F8 00 00 00; next byte CC).
// BSP_ControlLayoutScreen_Destroy_Adjustor8. Original ABI: __thiscall(ECX = this, flags), the
// target's RET; ECX adjusted, tail call to 00527E40
// (segs_13_15_22_44_cc13::control_layout_scalar_delete_00527e40).
Raw control_layout_screen_destroy_adjustor8_00527d40(Raw self, std::uint32_t flags, segs_13_15_22_44_cc13::SegBHost& host);
// 00528C60 RET (C3; next byte CC). BSP_HudCountersScreen_BindLayout. Original ABI:
// __thiscall(ECX = this), RET.
void hud_counters_screen_bind_layout_00528c60(Raw) noexcept;
// 0052B060 MOV EAX,51h; RET (B8 51 00 00 00 C3; next byte CC).
// BSP_HudCountersScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 81, RET.
std::int32_t hud_counters_screen_get_screen_id_0052b060(Raw) noexcept;
// 0052B070 MOV AL,1; RET (B0 01 C3; next byte CC). BSP_HudCountersScreen_IsSelfManaged.
// Original ABI: __thiscall(ECX = this) -> AL = 1, RET.
bool hud_counters_screen_is_self_managed_0052b070(Raw) noexcept;
// 0052FD60 MOV EAX,8h; RET (B8 08 00 00 00 C3; next byte CC). BSP_CreditsScreen_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 8, RET.
std::int32_t credits_screen_get_screen_id_0052fd60(Raw) noexcept;
// 00530640 RET 8 (C2 08 00; next byte CC). BSP_MenuPromptScreen_OnListboxSelectionChangedNoOp.
// Original ABI: __thiscall(ECX = this, 2 stack dwords unread), RET 8.
void menu_prompt_screen_on_listbox_selection_changed_no_op_00530640(Raw) noexcept;
// 00532310 MOV EAX,55h; RET (B8 55 00 00 00 C3; next byte CC).
// BSP_MenuPromptScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 85, RET.
std::int32_t menu_prompt_screen_get_screen_id_00532310(Raw) noexcept;
// 00532320 MOV AL,1; RET (B0 01 C3; next byte CC). BSP_MenuPromptScreen_IsSelfManaged. Original
// ABI: __thiscall(ECX = this) -> AL = 1, RET.
bool menu_prompt_screen_is_self_managed_00532320(Raw) noexcept;
// 00533B30 RET 8 (C2 08 00; next byte CC). BSP_HudFormationScreen_OnListboxActivateNoOp.
// Original ABI: __thiscall(ECX = this, 2 stack dwords unread), RET 8.
void hud_formation_screen_on_listbox_activate_no_op_00533b30(Raw) noexcept;
// 00538CD0 MOV EAX,2Ah; RET (B8 2A 00 00 00 C3; next byte CC).
// BSP_HudFormationScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 42, RET.
std::int32_t hud_formation_screen_get_screen_id_00538cd0(Raw) noexcept;
// 00538CE0 MOV AL,1; RET (B0 01 C3; next byte CC). BSP_HudFormationScreen_OccludesLowerLevels.
// Original ABI: __thiscall(ECX = this) -> AL = 1, RET.
bool hud_formation_screen_occludes_lower_levels_00538ce0(Raw) noexcept;
// 0053BA50 MOV EAX,57h; RET (B8 57 00 00 00 C3; next byte CC).
// BSP_FrameStatsScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 87, RET.
std::int32_t frame_stats_screen_get_screen_id_0053ba50(Raw) noexcept;
// 0053BA60 MOV AL,1; RET (B0 01 C3; next byte CC). BSP_FrameStatsScreen_IsSelfManaged. Original
// ABI: __thiscall(ECX = this) -> AL = 1, RET.
bool frame_stats_screen_is_self_managed_0053ba60(Raw) noexcept;
// 0053BA90 RET (C3; next byte CC). BSP_FrameStatsScreen_Exit. Original ABI: __thiscall(ECX =
// this), RET.
void frame_stats_screen_exit_0053ba90(Raw) noexcept;
// 0053D280 RET (C3; next byte CC). BSP_HudFreeCameraScreen_Exit. Original ABI: __thiscall(ECX =
// this), RET.
void hud_free_camera_screen_exit_0053d280(Raw) noexcept;
// 0053FF00 JMP 0053F250 (E9 4B F3 FF FF; next byte CC). TMPL_Rec30Vector_Tidy_0053ff00.
// Original ABI: __thiscall(ECX = this), the target's RET; tail call to 0053F250
// (Seg15Host::effect_vector_tidy_0053f250).
void rec30vector_tidy_0053ff00(Raw self, Seg15Host& host);
// 00540360 MOV EAX,2Bh; RET (B8 2B 00 00 00 C3; next byte CC).
// BSP_HudFreeCameraScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 43, RET.
std::int32_t hud_free_camera_screen_get_screen_id_00540360(Raw) noexcept;
// 005CE5B0 MOV EAX,36h; RET (B8 36 00 00 00 C3; next byte CC).
// BSP_HudMovieCameraDebugScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 54,
// RET.
std::int32_t hud_movie_camera_debug_screen_get_screen_id_005ce5b0(Raw) noexcept;
// 005CE740 RET (C3; next byte CC). BSP_MultiChatScreen_EnterNoOp. Original ABI: __thiscall(ECX
// = this), RET.
void multi_chat_screen_enter_no_op_005ce740(Raw) noexcept;
// 005CFE40 MOV EAX,15h; RET (B8 15 00 00 00 C3; next byte CC). BSP_MultiChatScreen_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 21, RET.
std::int32_t multi_chat_screen_get_screen_id_005cfe40(Raw) noexcept;
// 005CFE50 MOV AL,1; RET (B0 01 C3; next byte CC). BSP_MultiChatScreen_IsSelfManaged. Original
// ABI: __thiscall(ECX = this) -> AL = 1, RET.
bool multi_chat_screen_is_self_managed_005cfe50(Raw) noexcept;
// 005D2F40 MOV EAX,1Dh; RET (B8 1D 00 00 00 C3; next byte CC).
// BSP_MultiErrorScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 29, RET.
std::int32_t multi_error_screen_get_screen_id_005d2f40(Raw) noexcept;
// 005D3580 MOV EAX,19h; RET (B8 19 00 00 00 C3; next byte CC).
// BSP_HudPauseMultiScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 25, RET.
std::int32_t hud_pause_multi_screen_get_screen_id_005d3580(Raw) noexcept;

}  // namespace bsp::frontend_seg15_22_cc13

#endif  // BSP_FRONTEND_SEG15_22_CC13_HPP
