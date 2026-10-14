// Segments 13, 15, 22 and 44, reconstruction pass (packet cc13-segB).
//
// Segment 13 (0051A0E0..0051E730): the briefing screen (00E198AC+64h, INTF_BRIEFING) and two
// binocular helpers. Segment 44 (007870D0..0078D850): the network packet statistics singleton
// and the ocean environment's owned-pointer helpers. Segments 15 and 22 continue the front-end
// work of docs/CC13_FE_4.md and docs/CC13_FE_10.md. docs/CC13_SEGS_13_15_22_44.md carries the
// evidence and the status of every routine.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in
// the image) and reaches the engine through SegBHost, whose methods name the image routine they
// stand for. Win32 critical sections and InterlockedDecrement are called directly, as the image
// does. New C++ interfaces, not drop-in binary replacements; nothing here was run in bsp_game.

#ifndef BSP_SEGS_13_15_22_44_CC13_HPP
#define BSP_SEGS_13_15_22_44_CC13_HPP

#include <cstdint>
#include <string_view>

#include "bsp/engine_seg11_35_66_cc13.hpp"
#include "bsp/ingame_gui_cc13.hpp"

namespace bsp::segs_13_15_22_44_cc13 {

using bsp::ingame_gui_cc13::Raw;
using bsp::engine_seg11_35_66_cc13::SingletonLifetimeCalls;

// The engine's 8-byte string {length, data}; storage comes from the sized pool and is returned
// as (data, length + 1) through 00419CC0 / 00BD1510.
struct NativeStringRaw {
    std::uint32_t length;
    char* data;
};

// ---------------------------------------------------------------------------
// Image addresses the routines store or read.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGameGlobal = 0x00E188A8;           // *: game object, +1FE4h session
inline constexpr std::uint32_t kMainFrontEndGlobal = 0x00E198AC;    // *: main-menu front-end manager
inline constexpr std::uint32_t kMultiMenuGlobal = 0x00E198B4;       // *: multiplayer menu manager
inline constexpr std::uint32_t kCommandBarGlobal = 0x00E1930C;      // *: command-bar screen (0054B530)

// Briefing screen (138h bytes). Vtables installed by 0051E4D0 / 0051E050.
inline constexpr std::uint32_t kBriefingVtable = 0x00CEC948;
inline constexpr std::uint32_t kBriefingListboxListenerVtable = 0x00CEC934;  // +8h
inline constexpr std::uint32_t kBriefingWidgetListenerVtable = 0x00CEC914;   // +Ch
inline constexpr std::uint32_t kListboxListenerBaseVtable = 0x00CEB0FC;
inline constexpr std::uint32_t kWidgetListenerBaseVtable = 0x00CEB110;
inline constexpr int kBriefingScreenId = 3;                 // 0051E250: INTF_BRIEFING
inline constexpr float kBriefingDimmedFactor = 0.5f;        // 00CE3800
inline constexpr float kBriefingLitFactor = 1.0f;           // FLD1
inline constexpr std::string_view kBriefingTextChild = "Text";                  // 00CEC778
inline constexpr std::string_view kBriefingListboxPage = "FE_briefing_listbox";  // 00CEC984
inline constexpr std::string_view kBriefingGridPage = "FE_briefing_grid";        // 00CEC970
inline constexpr std::string_view kBriefingMainListbox = "Main_Listbox";         // 00CEB334
inline constexpr std::string_view kBriefingListboxText = "MainListbox_Text";     // 00CEC040
// 0051B450 help line: control codes as the byte arguments of 0054B530.
inline constexpr std::uint8_t kHelpPlayCode = 0xA2;
inline constexpr std::uint8_t kHelpHistoryCode = 0xA7;
inline constexpr std::uint8_t kHelpNavigateCode = 0xAF;  // PUSH -51h
inline constexpr std::uint8_t kHelpBackCode = 0xA3;      // PUSH -5Dh
inline constexpr std::string_view kHelpPlayKey = "FE.briefing_play";            // 00CEC7AC
inline constexpr std::string_view kHelpHistoryKey = "FE.briefing_history";      // 00CEC798
inline constexpr std::string_view kHelpNavigateKey = "FE_xbox.bhelp_navigate";   // 00CEC780
inline constexpr std::string_view kHelpBackKey = "globals.back";                // 00CEB3A8
inline constexpr std::string_view kHelpEmptyKey = "";                           // 00CE3A0C

// Network packet statistics singleton (7649B4h bytes, 0078C400).
inline constexpr std::uint32_t kPacketStatsVtable = 0x00D04388;
inline constexpr std::uint32_t kPacketStatsBaseVtable = 0x00D04318;
inline constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818;
inline constexpr std::uint32_t kPacketStatsPublished = 0x00F871B4;
inline constexpr std::size_t kPacketStatsEnabledOffset = 0x762900;  // byte
inline constexpr std::size_t kPacketStatsLockOffset = 0x7628FC;     // {CRITICAL_SECTION, depth +18h}*
inline constexpr std::size_t kPacketStatsTreeOffset = 0x762904;     // std::_Tree, head +4h, size +8h
inline constexpr std::size_t kPacketStatsVectorAOffset = 0x762910;  // vector, begin +4h
inline constexpr std::size_t kPacketStatsVectorBOffset = 0x762920;  // vector, begin +4h
inline constexpr std::size_t kPacketStatsStringOffset = 0x7649AC;   // NativeString

// Ocean environment (0078D5D0).
inline constexpr std::uint32_t kRefCountedVtable_d04390 = 0x00D04390;

// ---------------------------------------------------------------------------
// Engine services. Each method names the image routine or virtual slot it stands for.
// ---------------------------------------------------------------------------
struct SegBHost {
    virtual ~SegBHost() = default;
    // Globals the routines dereference (named by address).
    virtual Raw game_e188a8() = 0;
    virtual void* main_front_end_e198ac() = 0;
    virtual void* multi_menu_e198b4() = 0;
    virtual void* command_bar_e1930c() = 0;
    virtual const char* packet_type_name_e0ab68(std::int32_t index) = 0;  // char* table 00E0AB68

    // CRT and checked-iterator support.
    virtual void invalid_iterator_00bf6713() = 0;
    virtual void crt_free_00bf65ac(void* block) = 0;
    // 00419CC0 then 00BD1510(data, size, 1): return a pooled block.
    virtual void pool_return_00bd1510(void* data, std::uint32_t size) = 0;
    virtual void native_string_resize_0041dd40(NativeStringRaw& s, std::uint32_t length) = 0;  // (n, 1)
    virtual void native_string_assign_0041e870(NativeStringRaw& s, const char* text) = 0;
    virtual void native_string_assign_cstring_0041e350(NativeStringRaw& s, const char* text) = 0;
    virtual void native_string_from_unsigned_005f1750(NativeStringRaw& out, std::uint32_t value) = 0;
    // 00432050, ECX = first, EDX = last, (vector, owner): destroy a NativeString range.
    virtual void native_string_range_destroy_00432050(void* first, void* last, Raw vector) = 0;

    // Front-end screen base (src/frontend_state_machine.cpp).
    virtual void front_end_screen_construct_004f7180(Raw screen) = 0;
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;
    virtual void front_end_screen_register_004f71d0(Raw screen) = 0;
    // 004CC460, ECX = front-end manager, (request, 0), RET 8.
    virtual void front_end_push_interface_request_004cc460(void* manager, std::int32_t request,
                                                           std::int32_t arg) = 0;
    // 006881F0, ECX = multiplayer menu.
    virtual void multi_menu_pop_request_interface_006881f0(void* menu) = 0;
    // 0054B530, ECX = command bar, five (byte code, NativeString*, placement) triples, RET 3Ch.
    virtual void command_bar_rebuild_0054b530(void* bar, std::uint8_t code1, NativeStringRaw* s1,
                                              std::uint32_t place1, std::uint8_t code2,
                                              NativeStringRaw* s2, std::uint32_t place2,
                                              std::uint8_t code3, NativeStringRaw* s3,
                                              std::uint32_t place3, std::uint8_t code4,
                                              NativeStringRaw* s4, std::uint32_t place4,
                                              std::uint8_t code5, NativeStringRaw* s5,
                                              std::uint32_t place5) = 0;

    // GUI. 004C12B0 (manager get-or-create) is folded into the page calls.
    virtual void* gui_load_page_00aa5840(const NativeStringRaw& name) = 0;  // (name, 1, 0)
    virtual void gui_remove_and_destroy_page_00aa31f0(void* page) = 0;
    virtual void* gui_find_child_00aa7e00(void* parent, const NativeStringRaw& name) = 0;  // (name, 1)
    virtual void* gui_find_child_by_suffix_00aaa2b0(void* widget, const NativeStringRaw& name) = 0;
    virtual std::int32_t list_box_selected_index_00a9c920(void* list_box) = 0;
    virtual void list_box_set_listener_00a9ac40(void* list_box, void* listener) = 0;
    virtual void widget_set_visible_34(void* widget, bool visible) = 0;  // vtable +34h(byte)
    // 00B6DA70, ECX = scene node, (float factor, 1), RET 8.
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float factor,
                                                           std::int32_t flag) = 0;
    // 004D6790, ECX = pointer vector, (const void*& element): push_back.
    virtual void pointer_vector_push_back_004d6790(void* vector, void* const& element) = 0;
    // Briefing-specific engine routines.
    virtual void briefing_load_objective_pages_0051dda0(Raw screen) = 0;
    virtual void* tree_buy_head_node_0051acb0(Raw tree) = 0;
    virtual void tree_iter_inc_0051a850(void*& node) = 0;
    // 0051CA90, ECX = tree, (result*, first, last): erase [first, last).
    virtual void tree_erase_range_0051ca90(Raw tree, void* first, void* last) = 0;

    // Binoculars.
    virtual void game_set_view_fov_004dc940(Raw game, float fov, std::int32_t flag) = 0;  // (fov, 1)
    virtual void unit_virtual_dc(void* unit, float value) = 0;  // vtable +DCh(float)

    // Packet statistics.
    // 00787850, ECX = stats, (data, length, 0, direction), RET 10h. Only a fragment of it is
    // reconstructed (src/native_bit_cursor_read.cpp).
    virtual void packet_stats_accumulate_00787850(Raw stats, void* data, std::int32_t length,
                                                  std::int32_t zero, std::int32_t direction) = 0;
    virtual void stream_virtual_5c(void* stream, NativeStringRaw* text, std::int32_t arg) = 0;
    // 0078BA70, ECX = first, EDX = last, (owner): destroy an element range.
    virtual void range_destroy_0078ba70(void* first, void* last, Raw owner) = 0;
    // 0078BDB0, ECX = tree, (result*, first, last).
    virtual void tree_erase_range_0078bdb0(Raw tree, void* first, void* last) = 0;

    // Owned / refcounted pointer virtuals.
    virtual void object_destroy_slot0(void* object) = 0;                // vtable +0h(), ECX = object
    virtual void object_deleting_dtor_04(void* object, std::int32_t flags) = 0;  // vtable +4h(1)
    virtual void refcounted_destroy_base_00bd30f0(Raw object) = 0;

    // Segment 15.
    virtual void matrix_copy_004134f0(void* destination, const float (&source)[16]) = 0;  // ECX = dst
    virtual const float* position_f87574() = 0;  // three floats 00F87574 / 00F87578 / 00F8757C
    virtual Raw interface_e198c4() = 0;          // *: in-game interface
    // 0077D600, ECX = entity, (descriptor, message*, 1).
    virtual void entity_issue_command_0077d600(void* entity, std::uint32_t descriptor,
                                               const void* message, std::int32_t flag) = 0;
    virtual void interface_collapse_overlays_0068ab80(Raw interface, std::int32_t a,
                                                      std::int32_t b) = 0;  // (1, 1)
    // Widget vtable +88h(int state, int, float).
    virtual void widget_virtual_88(void* widget, std::int32_t state, std::int32_t arg,
                                   float scale) = 0;
    virtual void debug_text_list_refresh_rows_00542620(Raw list) = 0;
    virtual void control_layout_destruct_00527da0(Raw screen) = 0;
    // 00543CA0 (first, last, destination, ...) / 00543C20 (ECX = first, EDX = last, vector):
    // the copy and destroy of 24h-byte debug-text rows.
    virtual void text_row_copy_00543ca0(void* first, void* last, void* destination) = 0;
    virtual void text_row_destroy_range_00543c20(void* first, void* last, Raw vector) = 0;

    // Segment 44 water helpers (ECX = game world). 0078CF20 returns its height in ST0; the
    // double carries the value the caller compares without a float store.
    virtual double water_height_0078cf20(Raw world, float a, float b) = 0;  // RET 8
    // Packet statistics constructor.
    // 00403560: eh vector constructor iterator (first, element size, count, constructor).
    virtual void vector_constructor_iterator_00403560(void* first, std::uint32_t size,
                                                      std::uint32_t count,
                                                      std::uint32_t constructor) = 0;
    virtual void* tree_buy_head_node_00787da0(Raw tree) = 0;
    virtual void* critical_section_create_00bd1860() = 0;  // the {section, depth} pair
    // 0078C0E0, ECX = map<int, vector<int>>, (const int& key), RET 4: operator[]; 0078B560,
    // ECX = vector<int>, (const int& value), RET 4: push_back.
    virtual void* type_map_subscript_0078c0e0(Raw tree, const std::int32_t& key) = 0;
    virtual void int_vector_push_back_0078b560(void* vector, const std::int32_t& value) = 0;
    // 0078D1B0, ECX = world, (out*, Vec3 above by value, Vec3 below by value), returns out.
    virtual const float* water_bisect_crossing_0078d1b0(Raw world, float (&out)[3],
                                                        const float (&above)[3],
                                                        const float (&below)[3]) = 0;
};

// ---------------------------------------------------------------------------
// Segment 13: briefing screen. `screen` is the 138h object; `listener` is screen+8h, the
// list-box listener subobject (vtable 00CEC934) whose slots 1 and 2 are 0051B8C0 / 0051B250.
// ---------------------------------------------------------------------------
void briefing_wrap_and_show_page_0051a590(Raw screen, SegBHost& host);
void native_string_release_0051b0e0(NativeStringRaw& s, SegBHost& host);
void briefing_exit_0051b1b0(Raw screen, SegBHost& host);
void briefing_selection_changed_0051b250(Raw listener, SegBHost& host);
void briefing_build_help_line_0051b450(Raw screen, bool suppress_all, bool suppress_history,
                                       SegBHost& host);
// 0051B6C0 / 0051B710: {NativeString key, dword value} pair constructors.
Raw string_pair_construct_0051b6c0(Raw self, const NativeStringRaw& key, const std::uint32_t& value,
                                   SegBHost& host);
Raw string_pair_copy_0051b710(Raw self, Raw source, SegBHost& host);
void briefing_list_activated_0051b8c0(Raw listener, SegBHost& host);
Raw briefing_mission_data_construct_0051cb60(Raw data);
void briefing_mission_data_destroy_0051cbb0(Raw data, SegBHost& host);
void briefing_destruct_0051e050(Raw screen, SegBHost& host);
int briefing_screen_id_0051e250();
void briefing_collect_children_0051e270(Raw screen, void* out_vector, SegBHost& host);
void briefing_register_0051e280(Raw screen, SegBHost& host);
Raw briefing_construct_0051e4d0(Raw screen, SegBHost& host);
void binoculars_add_clamped_pitch_0051e650(Raw mover, float delta);
void binoculars_apply_fov_0051e6e0(Raw screen, SegBHost& host);

// Exposed for the briefing routines: the checked vector<T*>::erase 0051AEB0 inlined by 0051B1B0.
void pointer_vector_erase_0051aeb0(Raw vector, std::uint8_t* first, std::uint8_t* last,
                                   SegBHost& host);

// ---------------------------------------------------------------------------
// Segment 15: control layout destructor, HUD helpers and the debug text list.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kIssueCommandDescriptor = 0x00E08FB8;
inline constexpr float kMatrixUnitW = 1.0f;  // 00D7A24C
Raw control_layout_scalar_delete_00527e40(Raw screen, std::uint32_t flags, SegBHost& host);
void scale_matrix_00534690(void* out, float sx, float sy, float sz, SegBHost& host);
void* uniform_scale_matrix_00534720(void* out, float s, SegBHost& host);
void issue_move_to_marker_00535b20(Raw screen, SegBHost& host);
void on_confirm_issue_move_00535f30(std::int32_t code, SegBHost& host);
void command_bar_paint_button_00541a10(Raw bar, std::uint32_t index, SegBHost& host);
void debug_text_list_next_enabled_005427e0(Raw list, SegBHost& host);
void debug_text_list_previous_enabled_00542890(Raw list, SegBHost& host);
// 005431A0 and 00543210 are byte-identical.
void button_vector_show_all_005431a0(Raw bar, bool visible, SegBHost& host);
void debug_text_list_clear_00544a70(Raw list, SegBHost& host);

// ---------------------------------------------------------------------------
// Segment 44: network packet statistics and owned-pointer helpers.
// ---------------------------------------------------------------------------
void packet_stats_record_received_00787aa0(Raw stats, void* peer, std::uint8_t* packet,
                                           std::int32_t length, SegBHost& host);
void packet_stats_record_sent_00787b00(Raw stats, void* peer, std::uint8_t* packet,
                                       std::int32_t length, SegBHost& host);
void packet_stats_construct_base_00787b60(Raw stats, void*& published_f871b4,
                                          SingletonLifetimeCalls& lifetime);
void packet_stats_destruct_base_00787c00(Raw stats, void*& published_f871b4,
                                         SingletonLifetimeCalls& lifetime);
void packet_stats_format_type_column_00788250(Raw counts, std::int32_t type, void* stream,
                                              SegBHost& host);
NativeStringRaw& packet_stats_format_field_00788380(const NativeStringRaw& prefix,
                                                    NativeStringRaw& out, std::uint32_t value,
                                                    SegBHost& host);
Raw packet_stats_construct_0078c400(Raw stats, void*& published_f871b4,
                                    SingletonLifetimeCalls& lifetime, SegBHost& host);
void packet_stats_destruct_0078c280(Raw stats, void*& published_f871b4,
                                    SingletonLifetimeCalls& lifetime, SegBHost& host);
void owned_ptr_delete_sub4_0078c790(void*& slot, SegBHost& host);
// 0078C7B0 and 0078C7D0 are byte-identical.
void owned_ptr_delete_0078c7b0(void*& slot, SegBHost& host);
// 0078D540, 0078D570, 0078D5A0 are byte-identical (return the slot); 0078D820 is the same
// release without the return value.
void** refptr_release_0078d540(void*& slot, SegBHost& host);
void ocean_environment_destruct_0078d5d0(Raw ocean, SegBHost& host);
bool water_segment_crossing_0078d6c0(Raw world, const float (&p)[3], const float (&q)[3],
                                     float (&out)[3], SegBHost& host);

}  // namespace bsp::segs_13_15_22_44_cc13

#endif
