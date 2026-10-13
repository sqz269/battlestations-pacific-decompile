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
#include <string_view>

#include "bsp/ingame_gui_cc13.hpp"

namespace bsp::frontend_seg15_22_cc13 {

using bsp::ingame_gui_cc13::Raw;
using bsp::ingame_gui_cc13::TrivialSlot;
using bsp::ingame_gui_cc13::TrivialSlotKind;

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
// Menu prompt screen (55h, vtables 00CED3C4 / 00CED3B0 at +8h / 00CED38C at +Ch).
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

}  // namespace bsp::frontend_seg15_22_cc13

#endif  // BSP_FRONTEND_SEG15_22_CC13_HPP
