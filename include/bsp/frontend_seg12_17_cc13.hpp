#pragma once
// Front-end and menu routines of segment 12 (004F9D80-00519FE0: tactical library, frame, in-game
// screen 23, HUD pilot/bomber) and segment 17 (00554680-00568930: control binding screen and its
// input-settings trees, leaderboards, HUD limbo), packet cc13-fe3 (docs/CC13_FE_3.md).
//
// Objects are raw image storage (Raw::at at the image's offsets); engine routines that are not
// reconstructed are host methods named with their address. Bodies were read from the live Ghidra
// decompile and from bsp.py disasm-raw wherever ECX/EDX, stack cleanup or x87 mattered.
// Build-tested only: not ABI-compatible, not game-validated.
#include "bsp/ingame_gui_cc13.hpp"
#include "bsp/main_menu_command_bar.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bsp::cc13_fe3 {

using ingame_gui_cc13::CollectChildrenSlot;
using ingame_gui_cc13::Raw;
using ingame_gui_cc13::TrivialSlot;
using ingame_gui_cc13::TrivialSlotKind;

// ---------------------------------------------------------------------------
// Trivial slots. Adjustor value is the SUB ECX amount (negative for ADD ECX).
// ---------------------------------------------------------------------------
inline constexpr std::array<TrivialSlot, 29> kTrivialSlots{{
    {0x004FA170, TrivialSlotKind::Empty, 0, 0, 0, "widget listener slot +1Ch (RET)"},
    {0x00509D60, TrivialSlotKind::ReturnInt, 0x0B, 0, 0, "tactical library screen id"},
    {0x00509D70, TrivialSlotKind::Adjustor, -0x64, 0, 0x006833F0, "tactical library listener: scroller recapture at +64h"},
    {0x00509D80, TrivialSlotKind::Adjustor, 0x64, 0, 0x0050D8E0, "tactical library deleting destructor from +64h"},
    {0x00509D90, TrivialSlotKind::Adjustor, 8, 0, 0x0050D8E0, "tactical library deleting destructor from +8h"},
    {0x00517680, TrivialSlotKind::ReturnInt, 0x23, 0, 0, "in-game screen 23 id"},
    {0x00517690, TrivialSlotKind::Empty, 0, 4, 0, "in-game screen 23 collect children (none)"},
    {0x00517790, TrivialSlotKind::Adjustor, 0x10, 0, 0x00517880, "airfield camera deleting destructor from +10h"},
    {0x005177A0, TrivialSlotKind::Adjustor, 0x170, 0, 0x00517880, "airfield camera deleting destructor from +170h"},
    {0x005177B0, TrivialSlotKind::Adjustor, 0x24, 0, 0x00517880, "airfield camera deleting destructor from +24h"},
    {0x005177C0, TrivialSlotKind::Adjustor, 8, 0, 0x00517800, "in-game screen 23 deleting destructor from +8h"},
    {0x005178A0, TrivialSlotKind::Empty, 0, 4, 0, "in-game screen 23 update(float)"},
    {0x00517D30, TrivialSlotKind::ReturnInt, 0x0C, 0, 0, "front-end frame screen id"},
    {0x00517D40, TrivialSlotKind::ReturnByte, 1, 0, 0, "front-end frame self-managed"},
    {0x00517D50, TrivialSlotKind::Adjustor, 8, 0, 0x00517EB0, "front-end frame deleting destructor from +8h"},
    {0x00519350, TrivialSlotKind::Adjustor, 0x170, 0, 0x005193B0, "ship camera deleting destructor from +170h"},
    {0x00519360, TrivialSlotKind::Adjustor, 0x24, 0, 0x005193B0, "ship camera deleting destructor from +24h"},
    {0x00519370, TrivialSlotKind::Adjustor, 0x10, 0, 0x005193B0, "ship camera deleting destructor from +10h"},
    {0x00519450, TrivialSlotKind::ReturnInt, 0x25, 0, 0, "HUD bomber screen id"},
    {0x00519460, TrivialSlotKind::Adjustor, 8, 0, 0x00519470, "HUD bomber deleting destructor from +8h"},
    {0x005194C0, TrivialSlotKind::Adjustor, 0x170, 0, 0x00519500, "bomber view camera deleting destructor from +170h"},
    {0x005194D0, TrivialSlotKind::Adjustor, 0x24, 0, 0x00519500, "bomber view camera deleting destructor from +24h"},
    {0x005194E0, TrivialSlotKind::Adjustor, 0x10, 0, 0x00519500, "bomber view camera deleting destructor from +10h"},
    {0x00555800, TrivialSlotKind::ReturnInt, 0x0F, 0, 0, "control binding screen id"},
    {0x00555810, TrivialSlotKind::Adjustor, 8, 0, 0x00555C40, "control binding deleting destructor from +8h"},
    {0x005610D0, TrivialSlotKind::Empty, 0, 8, 0, "leaderboards listbox activate (no-op)"},
    {0x005633F0, TrivialSlotKind::ReturnInt, 0x0A, 0, 0, "leaderboards screen id"},
    {0x00563420, TrivialSlotKind::Adjustor, 8, 0, 0x005635D0, "leaderboards deleting destructor from +8h"},
    {0x00565D60, TrivialSlotKind::ReturnInt, 0x32, 0, 0, "HUD limbo screen id"},
}};
const TrivialSlot* find_trivial_slot(std::uint32_t address) noexcept;

// Collect-children slots: __thiscall(screen, vector* out), RET 4; each pushes the ADDRESS of the
// page slot at the offset through 004D6790 (ECX = out).
inline constexpr std::array<CollectChildrenSlot, 5> kCollectChildrenSlots{{
    {0x00509DA0, {0x4DC, 0x4E0, 0x4E4}, 3},  // tactical library
    {0x00519D40, {0x2C}, 1},        // HUD bomber
    {0x00558680, {0x14, 0x18}, 2},  // control binding
    {0x00563430, {0x30}, 1},        // leaderboards
    {0x00565FA0, {0x08}, 1},        // HUD limbo
}};

// ---------------------------------------------------------------------------
// Engine routines the packet's code calls. ECX-taking routines get the object first.
// ---------------------------------------------------------------------------
struct Fe3Host {
    virtual ~Fe3Host() = default;
    // Allocation.
    virtual void free_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;  // pool 00419CC0
    // Front-end screen base and widgets.
    virtual void front_end_screen_construct_004f7180(Raw screen) = 0;
    virtual void front_end_screen_register_004f71d0(Raw screen) = 0;
    virtual void object_delete(void* object) = 0;                                       // vtable +0(1)
    virtual void widget_virtual_48(void* widget, const float xy[2]) = 0;                // vtable +48h
    virtual void gui_set_state_texture_uv_00ab2690(void* widget, std::uint32_t state, std::uint32_t texture,
        const void* uv) = 0;
    virtual void gui_text_set_localised_00abbe50(void* text, const char* key, std::uint32_t flag) = 0;
    virtual void scroller_recapture_006833f0(void* scroller) = 0;
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    // Globals and singletons.
    virtual void movie_player_stop_004f8ac0() = 0;                                      // ECX = [00E18D48]
    virtual void game_cancel_leaderboard_read_004bc7f0() = 0;                           // ECX = [00E188A8]
    virtual void clear_frame_singleton_00e18d80() = 0;                                  // [00E18D80] = 0
    virtual const char* global_config_c8_name() = 0;                                    // [00432650()+0CCh]
    virtual const char* empty_string_00e19476() = 0;
    virtual void input_settings_reload_006aa2d0() = 0;                                  // ECX = 005547D0()
    virtual void settings_save_to_profile_008d6c00() = 0;                               // ECX = 00F88980, (00BD53C0)
    virtual void* control_binding_screen_00e198b8() = 0;                                // [[00E198B8]+48h]
    // Library routines with ECX = the object.
    virtual void* library_find_entry_by_name_004fbf70(void* atlas, const char* name) = 0;
    virtual void loading_job_pointers_resize_004fb4b0(void* vector, std::uint32_t size) = 0;
    virtual void award_index_list_destroy_00487110(void* list) = 0;
    virtual void library_entry_destruct_004fd1d0(void* entry) = 0;
    virtual void ref_ptr_array_resize_00504960(void* vector, std::uint32_t size) = 0;
    virtual void loading_queue_destroy_00509fd0(void* queue) = 0;
    virtual void iterator_next(std::uint32_t routine, void* iterator) = 0;              // 00552770, 005540C0, 00552D40, 00552E20
    virtual void* tree_allocate_head(std::uint32_t routine, void* tree) = 0;            // 00554CD0, 00554EC0, 004C3020
    virtual void tree_destroy_subtree(std::uint32_t routine, void* tree, void* node) = 0;
    // 0055A3A0, ECX = first, EDX = last, two stack words (allocator references).
    virtual void string_vector_range_destroy_0055a3a0(void* first, void* last, void* extra1, void* extra2) = 0;
    virtual void control_binding_refresh_page_0055e3a0(Raw screen, std::uint32_t argument) = 0;
    virtual void control_binding_load_selected_preset_0055f710(void* screen) = 0;
};

// 004FA2F0 BSP_SessionMessage221_IsType, slot (ignored ECX, type), RET 4: AL = type == 0DDh.
bool session_message_221_is_type_004fa2f0(std::int32_t type) noexcept;
// 004FAA90, __fastcall(ECX = holder): the object at +8h is deleted (vtable +0(1)) and +8h = 0.
void owned_object_delete_004faa90(Raw holder, Fe3Host& host);
// 004FAAB0: tail 004F8AC0 with ECX = [00E18D48] (stop the tutorial movie).
void stop_tutorial_movie_004faab0(Fe3Host& host);
// 004FB9E0, __thiscall(widget, float s), RET 4: vtable +48h(&{s, s}).
void widget_set_uniform_scale_004fb9e0(void* widget, float scale, Fe3Host& host);
// 004FC070 BSP_IconAtlas_ApplyByName, __thiscall(atlas, name, widget), RET 8: entry =
// 004FBF70(atlas, name); found -> 00AB2690(ECX = widget; 0, entry +0, &entry +4).
void icon_atlas_apply_by_name_004fc070(void* atlas, const char* name, void* widget, Fe3Host& host);
// 004FCBC0 / 00505730, __fastcall: resize(0) (004FB4B0 / 00504960), then free(+0) (00BF6989).
void loading_job_pointers_tidy_004fcbc0(Raw vector, Fe3Host& host);
void ref_ptr_array_destroy_00505730(Raw vector, Fe3Host& host);
// 004FE630 BSP_AwardIndexRecord_Destruct, __fastcall: 00487110(+8h); a string {length +0, data
// +4h} with data returns length + 1 bytes to the sized pool (00419CC0 / 00BD1510, count 1).
void award_index_record_destruct_004fe630(Raw record, Fe3Host& host);
// 00501CE0, __fastcall(ECX = holder): *holder -> 004FD1D0, free (00BF65AC), *holder = 0.
void owned_library_entry_delete_00501ce0(Raw holder, Fe3Host& host);
// 0050ACE0 / 00517D60 scalar deleting destructors, __thiscall(this, flags), RET 4, EAX = this.
// 0050ACE0: 00509FD0. 00517D60: [00E18D80] = 0, vtable 00CE3818. Bit 0 frees (00BF65AC).
void* loading_queue_deleting_destructor_0050ace0(Raw object, std::uint8_t flags, Fe3Host& host);
void* frame_deleting_destructor_00517d60(Raw object, std::uint8_t flags, Fe3Host& host);
// 005176A0 BSP_InGameScreen23_Register: 004F71D0, then +20h = 0.
void in_game_screen_23_register_005176a0(Raw screen, Fe3Host& host);
// 005176C0, __thiscall(observer, released), RET 4: clears whichever of +14h / +18h holds it
// (the first match only).
void in_game_screen_23_on_released_005176c0(Raw observer, void* released) noexcept;
// 00517CF0 BSP_FrontEndFrame_ClearSingleton: [00E18D80] = 0, vtable 00CE3818.
void frame_clear_singleton_00517cf0(Raw frame, Fe3Host& host);
// 00517D10 BSP_FrontEndFrame_Construct, EAX = frame: 004F7180; +8h = 00CEC388; +0h = 00CEC39C;
// +8h = 00CEC398 (the second store wins).
void* frame_construct_00517d10(Raw frame, Fe3Host& host);
// 00554A20, 00554BC0, 00554E20, 00554E40, 00555220: iterator ++, EAX = iterator, through the
// routine named in kIteratorPreIncrement.
struct IteratorPreIncrement {
    std::uint32_t address;
    std::uint32_t next;
};
inline constexpr std::array<IteratorPreIncrement, 5> kIteratorPreIncrement{{
    {0x00554A20, 0x00552770}, {0x00554BC0, 0x005540C0}, {0x00554E20, 0x00552D40},
    {0x00554E40, 0x00552E20}, {0x00555220, 0x005540C0},
}};
void* iterator_pre_increment(const IteratorPreIncrement& row, void* iterator, Fe3Host& host);
// 00555A40, __thiscall(tree, two ignored), RET 8, EAX = tree: head = 00554CD0(tree); +4h =
// head; head +1Dh (is-nil) = 1; head left/parent/right = head; +8h = 0.
void* int_scalar_tree_map_construct_00555a40(Raw tree, Fe3Host& host);
// 00557B30, EAX = list: +4h = 004C3020(list), +8h = 0.
void* pooled_string_list_construct_00557b30(Raw list, Fe3Host& host);
// Tree clear, __fastcall(ECX = tree): DestroySubtree(tree, head->parent); head parent = head,
// size 0, head left = head, head right = head. Rows: clear routine and its DestroySubtree.
struct TreeClear {
    std::uint32_t address;
    std::uint32_t destroy_subtree;
};
inline constexpr std::array<TreeClear, 6> kTreeClear{{
    {0x00558740, 0x00558310}, {0x00559400, 0x00558E60}, {0x00559430, 0x00558ED0}, {0x0055B030, 0x0055AF60},
    {0x00558D50, 0x00558420}, {0x00555660, 0x00554F10},
}};
void tree_clear(const TreeClear& row, Raw tree, Fe3Host& host);
// 0055AC60, __thiscall(this, a, b), RET 8: 0055A3A0(ECX = a, EDX = b; this, b).
void string_vector_range_destroy_wrap_0055ac60(void* self, void* a, void* b, Fe3Host& host);
// 0055F980 ReloadInputConfig / 0055F9B0 SaveAndRefresh, __fastcall(screen): 006AA2D0 on the
// input settings singleton / 008D6C00(00BD53C0) on 00F88980; +16Ch += 1; 0055E3A0(screen, 0).
void control_binding_reload_input_config_0055f980(Raw screen, Fe3Host& host);
void control_binding_save_and_refresh_0055f9b0(Raw screen, Fe3Host& host);
// 0055F9E0, prompt callback __fastcall(ECX = result): result 1 -> 0055F710 on [[00E198B8]+48h]
// and 008D6C00 save.
void control_binding_on_load_preset_prompt_0055f9e0(std::int32_t result, Fe3Host& host);
// 00560FE0 BSP_LeaderboardsScreen_Exit: tail 004BC7F0 with ECX = [00E188A8].
void leaderboards_exit_00560fe0(Fe3Host& host);
// 00561300, EAX: list box [+94h] with +85h set -> tail 00A9C920(listbox); else +0B8h.
std::int32_t leaderboards_selected_row_00561300(Raw screen, Fe3Host& host);
// 00563400: 006833F0(+28h), then tail 006833F0(+0B0h).
void leaderboards_listener_recapture_00563400(Raw listener, Fe3Host& host);
// 00565D40 BSP_HudLimboScreen_Construct, EAX = screen: 004F7180, vtable 00CEEBAC.
void* hud_limbo_construct_00565d40(Raw screen, Fe3Host& host);
// 00565D70 BSP_HudLimboScreen_Exit: +14h = 0.
void hud_limbo_exit_00565d70(Raw screen) noexcept;
// 00566050, __fastcall(screen): active (+5h) -> 00ABBE50(ECX = +0Ch; name, 1), name = the global
// config's +0CCh string or "" (00E19476) when null.
void limbo_show_config_message_00566050(Raw screen, Fe3Host& host);

// ---------------------------------------------------------------------------
// Batch B: session message 221, library records and their templates.
// A native string is {length +0h, data +4h}; 0041DD40 resizes it (ECX = string; length, 1).
// ---------------------------------------------------------------------------
struct Fe3RecordHost {
    virtual ~Fe3RecordHost() = default;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
    virtual void bit_write_byte_00428ff0(void* cursor, std::uint8_t value, std::uint32_t bits) = 0;
    virtual void bit_write_dword_00429070(void* cursor, std::uint32_t value, std::uint32_t bits) = 0;
    virtual void bit_read_byte_00428c70(void* cursor, void* out, std::uint32_t bits) = 0;
    virtual void bit_read_dword_00428d10(void* cursor, void* out, std::uint32_t bits) = 0;
    virtual void* library_find_entry_by_id_004fb090(void* atlas, std::int32_t id) = 0;
    virtual void gui_set_state_texture_uv_00ab2690(void* widget, std::uint32_t state, std::uint32_t texture,
        const void* uv) = 0;
    virtual void clear_atlas_singleton_00e18d4c() = 0;                                  // [00E18D4C] = 0
    virtual void loading_job_pointers_reserve_004fb310(void* vector, std::uint32_t capacity) = 0;
    virtual void ref_ptr_array_reserve_005017a0(void* vector, std::uint32_t capacity) = 0;
    virtual std::int32_t interlocked_decrement(void* count) = 0;                        // [00CE2220]
    virtual void interlocked_increment(void* count) = 0;                                // [00CE221C]
    virtual void ref_counted_destroy(void* object) = 0;                                 // vtable +0, ECX only
    virtual Raw library_screen() = 0;                                                   // [[00E198AC]+70h]
    virtual void library_reset_order_state_004f8fa0(Raw screen) = 0;
    virtual void* listbox_selected_item_00a9ca00(void* listbox) = 0;
    virtual void* item_virtual_28(void* item) = 0;                                      // vtable +28h
    virtual const char* empty_string_00e18d78() = 0;
    virtual std::int32_t atol_00bf8417(const char* text) = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void library_entry_assign_004fe310(void* to, const void* from) = 0;          // 6Ch entries
    virtual void award_record_destroy_storage_00501fa0(void* storage) = 0;
    virtual void* list_buy_head_node_004870f0(void* list) = 0;
    virtual void loading_job_destroy_storage_005051a0(void* job) = 0;
    virtual void pointer_vector_assign_004f60e0(void* to, const void* from) = 0;
    virtual void* create_thread(void* attributes, std::uint32_t stack, std::uint32_t start_00501510, void* parameter,
        std::uint32_t flags, void* id) = 0;                                              // [00CE2238]
    virtual void worker_virtual_04(void* worker) = 0;                                   // [+0Ch] vtable +4h
    // "invalid vector<T> subscript" (00CEB66C, 1Bh): 00408720 string, 00411700 logic error with
    // vtable 00D6926C, thrown with type info 00D863A8. Does not return.
    [[noreturn]] virtual void throw_vector_subscript() = 0;
};

// 004FA300 / 004FA340 Write / Read, __thiscall(message, cursor), RET 4: byte +10h (8 bits),
// +18h (6 bits), +1Ch (32 bits). Read uses the cursor's +4h sub-object.
void session_message_221_write_004fa300(Raw message, void* cursor, Fe3RecordHost& host);
void session_message_221_read_004fa340(Raw message, void* cursor, Fe3RecordHost& host);
// 004FA380, 004FB190 scalar deleting destructors, RET 4, EAX = this: vtable 00CE4974 /
// ([00E18D4C] = 0 and vtable 00CE3818); bit 0 frees (00BF65AC).
void* session_message_221_scalar_delete_004fa380(Raw message, std::uint8_t flags, Fe3RecordHost& host);
void* atlas_scalar_delete_004fb190(Raw atlas, std::uint8_t flags, Fe3RecordHost& host);
// 004FB150 BSP_IconAtlas_ApplyById: as 004FC070, through 004FB090(atlas, id).
void icon_atlas_apply_by_id_004fb150(void* atlas, std::int32_t id, void* widget, Fe3RecordHost& host);
// 004FB1F0, __fastcall(ECX = native string): ASCII a..z -> A..Z in place (signed char compare).
void native_string_to_upper_ascii_004fb1f0(Raw string) noexcept;
// 004FC0A0 / 00504870 push_back, __thiscall(vector, const T*), RET 4: full -> reserve(max(2 *
// capacity, 1)) (004FB310 / 005017A0); store; size += 1. 00504870 stores a ref-counted pointer
// (0, then the pointer, InterlockedIncrement(+4h) when non-null).
void loading_job_pointers_push_back_004fc0a0(Raw vector, void* const* value, Fe3RecordHost& host);
void ref_ptr_array_push_back_00504870(Raw vector, void* const* value, Fe3RecordHost& host);
// 004FC450, __thiscall(holder, pointer), RET 4, EAX = holder: the old object's count (+4h) is
// InterlockedDecremented and the object destroyed (vtable +0, ECX only) at zero; then store.
void* ref_counted_ptr_assign_004fc450(Raw holder, void* object, Fe3RecordHost& host);
// 004FC720 BSP_TacticalLibraryCarousel_StepBack, __fastcall(ECX = carousel index): index 0 ->
// nothing. Else index -= 1; 004F8FA0 on the library screen; with a selected list item (00A9CA00
// on screen +0B8h), id = atol(item vtable +28h()->+4h or ""), screen +138h[id] = screen +434h.
void library_carousel_step_back_004fc720(Raw carousel, Fe3RecordHost& host);
// 004FC840, __stdcall(screen, side, id), RET 0Ch, AL: the side's order-of-battle vector at
// screen +154h * side + 14Ch (8-byte records, string at +4h) contains atol(string) == id.
bool library_order_of_battle_contains_004fc840(Raw screen, std::int32_t side, std::int32_t id,
    Fe3RecordHost& host);
// String + vector record teardown: free the vector begin (00BF65AC) and zero begin/end/capacity,
// then return the native string to the sized pool. Some rows first zero a dword (pre_zero).
// Rows marked stdcall take the record on the stack (RET 4); the rest take ECX.
struct StringVectorDestroy {
    std::uint32_t address;
    std::uint8_t vector_offset;
    std::uint8_t string_offset;
    std::int8_t pre_zero;  // -1: none
    bool stdcall;
};
inline constexpr std::array<StringVectorDestroy, 19> kStringVectorDestroy{{
    {0x004FD350, 0x1C, 0x10, -1, false}, {0x00501F30, 0x1C, 0x10, -1, false}, {0x00504AF0, 0x1C, 0x10, -1, true},
    {0x005569A0, 0x0C, 0x00, -1, false}, {0x005569F0, 0x10, 0x00, 0x08, false}, {0x00556A70, 0x18, 0x0C, -1, false},
    {0x00556AC0, 0x1C, 0x0C, 0x14, false}, {0x00556BD0, 0x10, 0x00, -1, false}, {0x00556D00, 0x0C, 0x00, -1, false},
    {0x00557130, 0x18, 0x0C, -1, false}, {0x00557180, 0x1C, 0x0C, -1, false}, {0x005573F0, 0x18, 0x0C, -1, false},
    {0x00557440, 0x1C, 0x0C, 0x14, false}, {0x00557700, 0x18, 0x0C, -1, true}, {0x00557750, 0x1C, 0x0C, 0x14, true},
    {0x005579F0, 0x18, 0x0C, -1, false}, {0x00557A40, 0x1C, 0x0C, -1, false}, {0x00558380, 0x18, 0x0C, -1, true},
    {0x005583D0, 0x1C, 0x0C, -1, true},
}};
void string_vector_destroy(const StringVectorDestroy& row, Raw record, Fe3RecordHost& host);
// 004FE660 / 004FE6B0, __thiscall(value, const NativeString* / const value*, [const int*]),
// RET 8 / 4, EAX = value: copy the string (+0h/+4h; self-copy skipped), then +8h = *arg2 /
// source +8h.
void* named_value_construct_004fe660(Raw value, const void* string, const std::int32_t* number,
    Fe3RecordHost& host);
void* named_value_copy_construct_004fe6b0(Raw value, const void* source, Fe3RecordHost& host);
// 00501620 BSP_LoadingQueue_StartWork, __thiscall(queue, job), RET 4: no thread (+1Ch) ->
// CreateThread(0, 0, 00501510, queue +4h, 0, 0) into +1Ch. Job +0Ch set -> +8h = job, worker
// [+0Ch] vtable +4h.
void loading_queue_start_work_00501620(Raw queue, void* job, Fe3RecordHost& host);
// 00501EA0 copy_backward of 6Ch library entries (cdecl(first, last, dest_end), EAX = dest
// begin) through 004FE310.
void* library_entry_copy_backward_00501ea0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest_end,
    Fe3RecordHost& host);
// 005020A0 BSP_AwardRecord_Destroy, __fastcall: 00501FA0(+8h), then the string {+0h, +4h} to the
// pool. 00502110, EAX = record: string = {0, 0}, list +8h: head 004870F0, size 0.
void award_record_destroy_005020a0(Raw record, Fe3RecordHost& host);
void* award_record_construct_list_00502110(Raw record, Fe3RecordHost& host);
// 00504E30 / 00504EA0: throw "invalid vector<T> subscript".
[[noreturn]] void vector_throw_subscript_00504e30(Fe3RecordHost& host);
// 005060E0, __fastcall(holder): *holder -> 005051A0, free, *holder = 0.
void loading_job_delete_005060e0(Raw holder, Fe3RecordHost& host);
// 00506140 2Ch record assign, __thiscall(to, from), RET 4, EAX = to: +0h, +4h (float, through
// x87), +8h, +0Ch, +0Dh; the string at +10h (self skipped); 004F60E0 for +18h; +28h.
void* record_2c_assign_00506140(Raw to, Raw from, Fe3RecordHost& host);
// 005061B0 copy / 00506230 copy_backward of 2Ch records, cdecl(first, last, dest), EAX = dest end /
// dest begin.
void* record_2c_copy_005061b0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest, Fe3RecordHost& host);
void* record_2c_copy_backward_00506230(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest_end,
    Fe3RecordHost& host);
// 00506270 BSP_AwardRecord_Construct, EAX = record: +0h, +4h, +0Ch..+20h = 0; 00502110(+28h);
// +48h..+50h = 0.
void* award_record_construct_00506270(Raw record, Fe3RecordHost& host);

// ---------------------------------------------------------------------------
// Batch B (continued): library queue and screens, input-settings tree templates.
// ---------------------------------------------------------------------------
struct Fe3TemplateHost {
    virtual ~Fe3TemplateHost() = default;
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
    virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;
    virtual void* tree_allocate_head(std::uint32_t routine, void* tree) = 0;            // 00554CD0, 00554EC0, 004C2700
    virtual void tree_copy(std::uint32_t routine, void* tree, const void* source) = 0;  // 00555AA0, 00555B20
    virtual void tree_destroy_subtree(std::uint32_t routine, void* tree, void* node) = 0;
    virtual void value_copy(std::uint32_t routine, void* to, const void* from) = 0;     // node value / tail copy constructors
    virtual void vector_length_error_00556c20() = 0;
    virtual void* allocate_words_00553260(std::uint32_t count) = 0;                     // ECX = count, EDX = 0
    virtual void pooled_string_list_clear_storage_004d05e0(void* list) = 0;
    virtual void native_string_vector_copy_00557bc0(void* to, const void* from) = 0;
    virtual void native_string_range_destroy_00432050(void* first, void* last, void* extra1, void* extra2) = 0;
    virtual void string_vector_range_destroy_0055a3a0(void* first, void* last, void* extra1, void* extra2) = 0;
    virtual void int_scalar_tree_erase_range_005572f0(void* tree, void* result, void* first_node, void* last_node) = 0;
    virtual void* int_scalar_map_copy_subtree_0055b060(void* tree, void* root, void* head) = 0;
    // Library queue and screens.
    virtual void log_004254b0(const char* text) = 0;
    virtual void loader_virtual_18() = 0;                                               // [00F8D394] vtable +18h
    virtual void loader_virtual_1c() = 0;                                               // [00F8D394] vtable +1Ch
    virtual void sleep(std::uint32_t milliseconds) = 0;                                 // [00CE2230]
    virtual void vfs_tick_providers_00bdb0b0() = 0;                                     // ECX = [0109CEEC]
    virtual void loading_queue_update_00509190(Raw queue) = 0;
    virtual void* item_virtual_28(void* item) = 0;
    virtual const char* empty_string_00e18d78() = 0;
    virtual std::int32_t atol_00bf8417(const char* text) = 0;
    virtual void library_select_order_of_battle_side_00511bf0(Raw screen, std::int32_t id) = 0;
    virtual void library_refresh_entry_detail_00510c20(Raw screen) = 0;
    virtual void callback_owner_destroy_00695870(void* owner) = 0;
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;
    virtual void front_end_screen_construct_004f7180(Raw screen) = 0;
    virtual bool observer_is_registered_00694af0(void* observed, void* member) = 0;     // ECX, EDX
    virtual void observer_register_00694a60(void* observed, void* member) = 0;          // ECX, EDX
    virtual void widget_virtual_88(void* widget, std::int32_t frame, std::int32_t zero, float alpha) = 0;
    virtual void set_frame_selection_00e18d84(void* selection) = 0;                     // [00E18D84] = selection
    virtual void frame_apply_selection_00518c80() = 0;                                  // ECX = 004C1AC0()
    virtual void gui_remove_page_00aa31f0(void* page) = 0;                              // ECX = 004C12B0()
};

// 005092E0 BSP_LoadingQueue_StopAndWait, __fastcall: logs "GIAchievements::StopLoadingThread()";
// busy (+14h): [00F8D394] vtable +18h, [[+10h]]+4 = 1, then while busy: Sleep(100), 00BDB0B0,
// 00509190; finally tail [00F8D394] vtable +1Ch.
void loading_queue_stop_and_wait_005092e0(Raw queue, Fe3TemplateHost& host);
// String + tail copy constructors, __thiscall(to, const from*), RET 4, EAX = to: the native
// string {+0h, +4h} (self skipped), then the row's routine on (+8h, from +8h).
struct StringTailCopy {
    std::uint32_t address;
    std::uint32_t tail_copy;
};
inline constexpr std::array<StringTailCopy, 5> kStringTailCopy{{
    {0x00509830, 0x005071B0}, {0x0050A380, 0x0050A210}, {0x005577A0, 0x00557590}, {0x0055BA00, 0x0055B920}, {0x005584D0, 0x005578C0},
}};
void* string_tail_copy(const StringTailCopy& row, Raw to, Raw from, Fe3TemplateHost& host);
// Tree node allocation, __thiscall(tree (unused), left, parent, right, const value*, BYTE colour),
// RET 14h, EAX = node or 0: operator new(size); links +0h/+4h/+8h; value copied at +0Ch by the
// row's routine; +colour = colour, +colour+1 (is-nil) = 0.
struct TreeBuyNode {
    std::uint32_t address;
    std::uint8_t size;
    std::uint32_t value_copy;
    std::uint8_t colour_offset;
};
inline constexpr std::array<TreeBuyNode, 6> kTreeBuyNode{{
    {0x0050D840, 0x6C, 0x0050A380, 0x68}, {0x00558DC0, 0x28, 0x005577A0, 0x24}, {0x005591E0, 0x2C, 0x00558280, 0x28},
    {0x00559510, 0x28, 0x005584D0, 0x24}, {0x00559C90, 0x2C, 0x00558F80, 0x28}, {0x0055BB40, 0x9C, 0x0055BA00, 0x98},
}};
void* tree_buy_node(const TreeBuyNode& row, void* left, void* parent, void* right, const void* value,
    std::uint8_t colour, Fe3TemplateHost& host);
// 00554F50: the same with an 18h node and the 8-byte value stored inline at +0Ch (colour +14h).
void* int_scalar_tree_buy_node_00554f50(void* left, void* parent, void* right, const std::uint32_t value[2],
    std::uint8_t colour, Fe3TemplateHost& host);
// 00515B40, listener (ECX = screen +64h) (item, listbox), RET 8: item, screen +94h != 7 and the
// library list box (+0B8h): id = atol(item vtable +28h()->+4h or ""), id != 99999 -> 00511BF0(id),
// and with +508h set 00510C20.
void library_on_listbox_selection_changed_00515b40(Raw listener, void* item, void* listbox,
    Fe3TemplateHost& host);
// 00517620 / 005193D0 destructors: vtables 00CEC194, +8h 00CEC17C / 00CEC590, +8h 00CEC578
// (005193D0 also removes the page +2Ch); 00695870(+8h); 004F71A0.
void in_game_screen_23_destruct_00517620(Raw screen, Fe3TemplateHost& host);
void hud_bomber_destruct_005193d0(Raw screen, Fe3TemplateHost& host);
// 00517730: observed [+1Ch] not registered with member +8h -> 00694A60 (ECX = observed, EDX =
// screen +8h).
void in_game_screen_23_ensure_observer_00517730(Raw screen, Fe3TemplateHost& host);
// 005178B0 BSP_InGameScreen23_Construct, EAX = screen: 004F7180, +8h = 00CE3CD4, +0Ch..+14h = 0,
// +18h = 0, +1Ch = 0, vtables 00CEC194 / +8h 00CEC17C.
void* in_game_screen_23_construct_005178b0(Raw screen, Fe3TemplateHost& host);
// 00517CB0, __fastcall(ECX = widget, EDX = rank): vtable +88h(clamp(rank - 1, 0, 13h), 0, 1.0f)
// (a negative index uses 0).
void set_player_rank_icon_frame_00517cb0(void* widget, std::int32_t rank, Fe3TemplateHost& host);
// 00518DA0, __fastcall(ECX = selection): [00E18D84] = selection; tail 00518C80 on 004C1AC0().
void frame_set_selection_00518da0(void* selection, Fe3TemplateHost& host);
// 005190A0, __thiscall(observer, released), RET 4: clears +14h or else +20h.
void hud_bomber_on_released_005190a0(Raw observer, void* released) noexcept;
// Tree heads: 00554CD0 (20h, colour +1Ch) and 00554EC0 (18h, colour +14h), EAX = head or 0:
// links 0, colour 1, is-nil 0.
void* tree_allocate_head(std::uint8_t size, std::uint8_t colour_offset, Fe3TemplateHost& host);
// Tree constructors, ECX = tree: head from the row's allocator, is-nil = 1, links to itself,
// size 0; copy rows then copy the source (00555AA0 / 00555B20, size stored first).
struct TreeConstruct {
    std::uint32_t address;
    std::uint32_t allocate_head;
    std::uint8_t is_nil_offset;
    std::uint32_t copy;    // 0: none
    std::uint8_t ret_bytes;
    bool returns_tree;
};
inline constexpr std::array<TreeConstruct, 5> kTreeConstruct{{
    {0x00555360, 0x00554CD0, 0x1D, 0, 0, false}, {0x00555540, 0x00554EC0, 0x15, 0, 0, false},
    {0x0055B4D0, 0x00554CD0, 0x1D, 0, 0, true}, {0x00557670, 0x004C2700, 0x11, 0x00555AA0, 4, true},
    {0x00558570, 0x00554EC0, 0x15, 0x00555B20, 4, true},
}};
void* tree_construct(const TreeConstruct& row, Raw tree, const void* source, Fe3TemplateHost& host);
// Subtree destruction, __thiscall(tree, node), RET 4: recurse right, step left, destroy the value
// (string +0Ch / vector +18h for the N28 rows) and free the node, until an is-nil node.
struct TreeDestroySubtree {
    std::uint32_t address;
    std::uint8_t is_nil_offset;
    bool string_vector_value;
    std::uint8_t vector_offset;  // value teardown: vector freed and zeroed,
    std::uint8_t string_offset;  // string returned to the pool,
    std::int8_t pre_zero;        // and this dword zeroed first (-1: none)
};
inline constexpr std::array<TreeDestroySubtree, 5> kTreeDestroySubtree{{
    {0x00554F10, 0x15, false, 0, 0, -1}, {0x00558310, 0x25, true, 0x18, 0x0C, -1},
    {0x00558E60, 0x25, true, 0x18, 0x0C, -1}, {0x00558420, 0x29, true, 0x1C, 0x0C, 0x14},
    {0x00558ED0, 0x29, true, 0x1C, 0x0C, -1},
}};
void tree_destroy_subtree(const TreeDestroySubtree& row, void* node, Fe3TemplateHost& host);
// Case-insensitive lower bound, __thiscall(tree, const NativeString* key), RET 4, EAX = node:
// an empty node key is below any non-empty key, an empty search key is below everything else,
// otherwise _stricmp(node key, key) < 0 goes right. Node key at +0Ch/+10h.
struct TreeLowerBound {
    std::uint32_t address;
    std::uint8_t is_nil_offset;
};
inline constexpr std::array<TreeLowerBound, 3> kTreeLowerBound{{{0x005548C0, 0x25}, {0x00554920, 0x29}, {0x00554980, 0x99}}};
void* tree_lower_bound(const TreeLowerBound& row, Raw tree, Raw key, Fe3TemplateHost& host);
// 005558F0, __thiscall(string, pos, count), RET 8: pos < length -> count clipped to the tail,
// memmove the tail down, resize(length - count, 1).
void native_string_erase_005558f0(Raw string, std::uint32_t pos, std::uint32_t count, Fe3TemplateHost& host);
// 00557200, __thiscall(vector, n), RET 4, AL: begin/end/capacity (+4h..+0Ch) = 0; n == 0 -> 0;
// n > 3FFFFFFFh -> 00556C20; storage = 00553260(n); AL = 1.
bool word_vector_construct_n_00557200(Raw vector, std::uint32_t n, Fe3TemplateHost& host);
// 00557B50: 004D05E0, free +4h, +4h = 0.
void pooled_string_list_destroy_00557b50(Raw list, Fe3TemplateHost& host);
// 00558640 (__fastcall(ECX = to, EDX = from)) / 005590C0 (__stdcall(to, from), RET 8): placement
// copy of a native string vector through 00557BC0 when `to` is non-null.
void native_string_vector_placement_copy(void* to, const void* from, Fe3TemplateHost& host);
// Vector tidy: 00559DC0 (__stdcall(vector), RET 4, elements through 00432050) and 0055AFF0 /
// 0055B110 (__fastcall, elements through 0055A3A0): with storage, destroy [begin, end), free,
// zero +4h..+0Ch.
void native_string_vector_tidy_00559dc0(Raw vector, Fe3TemplateHost& host);
void string_vector_vector_tidy(Raw vector, void* stack_word, Fe3TemplateHost& host);
// 0055A3A0, ECX = first, EDX = last, two stack words, RET 8: each 10h element with storage is
// destroyed through 00432050, freed and zeroed.
void string_vector_range_destroy_0055a3a0(std::uint8_t* first, std::uint8_t* last, void* extra1, void* extra2,
    Fe3TemplateHost& host);
// 0055A330 (__fastcall(owner)) / 0055A910 (__stdcall(owner), RET 4): the tree at owner +10h
// erases [head->left, head) (005572F0), frees its head, head and size = 0.
void int_scalar_tree_clear_owner(Raw owner, Fe3TemplateHost& host);
// 0055B300, __thiscall(map, const map&), RET 4: root = 0055B060(source root, head); size copied;
// head left/right = leftmost/rightmost of the root, or the head when the copy is empty.
void int_scalar_map_copy_body_0055b300(Raw map, Raw source, Fe3TemplateHost& host);

// ---------------------------------------------------------------------------
// Batch B (end): control binding prompts, leaderboards, HUD limbo, colours.
// ---------------------------------------------------------------------------
struct Fe3ScreenHost {
    virtual ~Fe3ScreenHost() = default;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual void input_device_record_destroy_0055b690(void* record) = 0;
    virtual void settings_save_to_profile_008d6c00() = 0;                               // ECX = 00F88980, (00BD53C0)
    virtual void input_settings_reload_006aa2d0() = 0;                                  // ECX = 005547D0()
    virtual Raw control_binding_screen_00e198b8() = 0;                                  // [[00E198B8]+48h]
    virtual void control_binding_refresh_page_0055e3a0(Raw screen, std::uint32_t argument) = 0;
    virtual float revert_value_00ce3800() = 0;
    virtual void input_configuration_load_00698a10() = 0;                               // ECX = [00E188A8]+3Ch
    virtual void leaderboards_move_selection_00561860(Raw screen, void* scroller, std::uint32_t a, std::uint32_t b,
        std::uint32_t zero) = 0;
    virtual void leaderboards_show_board_name_00561660(Raw screen) = 0;
    virtual void leaderboard_row_assign_005625a0(void* to, const void* from) = 0;
    virtual void front_end_screen_construct_004f7180(Raw screen) = 0;
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;
    virtual void gui_scroller_construct_00683610(void* scroller) = 0;
    virtual void gui_remove_page_00aa31f0(void* page) = 0;                              // ECX = 004C12B0()
    virtual void hud_limbo_poll_respawn_input_00565d80(Raw screen) = 0;                 // cc9 (see the doc)
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;        // vtable +34h
    // 00E194A8 guard bit 0 and the colour at 00E19498.
    virtual std::uint32_t& static_red_guard_00e194a8() = 0;
    virtual float* static_red_00e19498() = 0;
    virtual float one_00d7a24c() = 0;
};

// 0055B8B0, __fastcall(pair): 0055B690(+8h), then the string {+0h, +4h} to the sized pool.
void input_device_pair_destroy_0055b8b0(Raw pair, Fe3ScreenHost& host);
// 0055FA10 BSP_ControlBindingScreen_OnRevertPromptResult, __fastcall(ECX = result): 1 -> save
// (008D6C00), 3 -> reload (006AA2D0); either -> +16Ch += 1 and 0055E3A0(screen, 0). Always:
// screen +10h = [00CE3800], then tail 00698A10 (ECX = [00E188A8]+3Ch).
void control_binding_on_revert_prompt_0055fa10(std::int32_t result, Fe3ScreenHost& host);
// 00561E50, listener (ECX = screen +8h) (item, listbox), RET 8. The board list box (+94h):
// 00561860(scroller +34h, listbox, +90h, 0) and 00561660. Otherwise 00561860(scroller +0BCh,
// +11Ch, +118h, 0). (Offsets here are screen-relative.)
void leaderboards_on_listbox_selection_changed_00561e50(Raw listener, void* listbox, Fe3ScreenHost& host);
// 00562E60 copy_backward of 40h leaderboard rows (cdecl(first, last, dest_end), EAX = dest begin)
// through 005625A0.
void* leaderboard_row_copy_backward_00562e60(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest_end,
    Fe3ScreenHost& host);
// 00563370 BSP_LeaderboardsScreen_Construct, EAX = screen: 004F7180; +8h = 00CEB0FC, +0Ch =
// 00CEB110; vtables 00CEE9CC / +8h 00CEE9B8 / +0Ch 00CEE998; +14h..+1Ch = 0; scrollers +34h, +0BCh.
void* leaderboards_construct_00563370(Raw screen, Fe3ScreenHost& host);
// 00565E00 BSP_HudLimboScreen_Update, __thiscall(screen, float dt), RET 4: 00565D80; timer +18h >
// 0 (COMISS) -> t -= dt, stored; t <= 0 (FCOMI, NaN keeps waiting) -> +18h = 0 and tail [+10h]
// vtable +34h(1).
void hud_limbo_update_00565e00(Raw screen, float dt, Fe3ScreenHost& host);
// 00565E60 BSP_HudLimboScreen_Destruct: vtable 00CEEBAC, page +8h removed, 004F71A0.
void hud_limbo_destruct_00565e60(Raw screen, Fe3ScreenHost& host);
// 00568220 BSP_GuiColour_StaticRed, EAX = 00E19498: first call (guard bit 0) stores {1.0
// ([00D7A24C]), 0, 0, 1.0}.
const float* gui_colour_static_red_00568220(Fe3ScreenHost& host);
// 00568330 / 00568380 scalar deleting destructors: vtable 00CEED00, bit 0 frees.
void* ceed00_scalar_delete(Raw object, std::uint8_t flags, Fe3ScreenHost& host);

// ---------------------------------------------------------------------------
// Batch C: tactical library screen helpers. Library entry vectors hold 6Ch-byte entries; the
// screen's carousel/order state lives on the library screen [[00E198AC]+70h]. Checked-index
// calls to 00BF6713 that cannot fire on an in-range index are not modelled.
// ---------------------------------------------------------------------------
struct Fe3LibraryHost {
    virtual ~Fe3LibraryHost() = default;
    // Lua (00B65F50 construct, 00B67700 destruct, 00B67980 globals of [00E188A8]+1A0Ch, 00B67800
    // by name, 00B67720 by index, 00B66000 is-boolean, 00B66250 boolean). The handle model keeps
    // the call order; handles are released in reverse.
    virtual void* lua_globals_00b67980() = 0;
    virtual void* lua_get_by_name_00b67800(void* table, const char* name) = 0;
    virtual void* lua_get_by_index_00b67720(void* table, std::int32_t index) = 0;
    virtual bool lua_is_boolean_00b66000(void* object) = 0;
    virtual bool lua_get_boolean_00b66250(void* object) = 0;
    virtual void lua_release_00b67700(void* object) = 0;
    // Session messages: 0075B430 base constructor (type), then sent through game +1EF0h.
    virtual void session_message_base_0075b430(void* message, std::uint32_t type) = 0;
    virtual void session_send_first_peer_00770af0(void* message) = 0;
    virtual void session_receive_unlock_0076cb30(void* message) = 0;
    virtual Raw game() = 0;                                                             // [00E188A8]
    virtual bool profile_unlock_satisfied_007fc4c0(const void* string) = 0;             // ECX = game +650h
    virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual std::uint32_t widget_type_5c(void* widget) = 0;                             // vtable +5Ch
    virtual std::int16_t widget_state_94(void* widget) = 0;                             // vtable +94h
    virtual void widget_set_float_4c(void* widget, float value) = 0;                    // vtable +4Ch
    virtual void widget_virtual_84(void* widget, std::uint32_t focus) = 0;              // vtable +84h
    virtual void widget_virtual_88(void* widget, std::uint32_t a, std::uint32_t b, float alpha) = 0;
    virtual void scroller_end_thumb_drag_00683320(void* scroller) = 0;
    virtual void scroller_set_auto_scroll_00683300(void* scroller, std::uint32_t a, float speed, std::uint32_t b) = 0;
    // Library screen and its list box.
    virtual Raw library_screen() = 0;                                                   // [[00E198AC]+70h]
    virtual void library_reset_order_state_004f8fa0(Raw screen) = 0;
    virtual void* listbox_selected_item_00a9ca00(void* listbox) = 0;
    virtual void* listbox_item_00a9be00(void* listbox, std::uint32_t index) = 0;
    virtual void* item_virtual_28(void* item) = 0;
    virtual const char* empty_string_00e18d78() = 0;
    virtual std::int32_t atol_00bf8417(const char* text) = 0;
};

// 004FA230, __thiscall(out, b) with EDX = a, RET 4, EAX = out: out = 004F9B30 cross(a, b) (x87:
// products in extended precision; the C++ uses double, exact for the products).
void* vector3_assign_cross_004fa230(float* out, const float* a, const float* b) noexcept;
// 004FAB40 BSP_VehicleClassLua_IsHeavyCruiser, slot(index), RET 4, AL:
// globals.VehicleClass[index].HeavyCruiser is a boolean equal to 1.
bool vehicle_class_is_heavy_cruiser_004fab40(std::int32_t index, Fe3LibraryHost& host);
// 004FADC0 BSP_Session_SendPlayerUnlockUnit, __thiscall(screen, unit, extra), RET 8: a type-0DDh
// (221) message on the stack: +18h = game +18C8h, +1Ch = unit, +4h = 1, vtable 00CEB138; mode 2
// (game +1FE4h) sends it to the first secondary peer (00770AF0), otherwise it is received
// locally (0076CB30). Then screen +454h = unit, +458h = extra.
void library_send_player_unlock_unit_004fadc0(Raw screen, std::int32_t unit, std::int32_t extra,
    Fe3LibraryHost& host);
// 004FAE70, __thiscall(screen, id), RET 4, AL: an academy entry (+408h vector) with +40h == id
// and +44h set.
bool library_academy_entry_ready_004fae70(Raw screen, std::int32_t id) noexcept;
// 004FAC50, __fastcall(screen): each +3C8h entry's +45h ("locked") = its unlock string (+30h)
// is non-empty (data non-null and not "") and 007FC4C0 does not report it satisfied.
void library_build_entry_locks_004fac50(Raw screen, Fe3LibraryHost& host);
// 004FAF40 BSP_TacticalLibraryScreen_OnWidgetRefresh, listener (ECX = screen +68h;
// offsets below are listener-relative: scroller +64h = screen +0CCh, arrays = screen +1D8h/+278h,
// matching 0050B2D0) (widget), RET 4.
// Type 3: alpha 0.5 when focused (+0D4h) else 1.0 through vtable +4Ch. Otherwise:
//  - widgets +554h/+550h: 00683300(screen +64h; 0, 0.0f, 0), forced; +558h: 00683320, forced;
//  - +50Ch/+510h: styled when vtable +94h != 3, else only when forced;
//  - the 40-entry arrays +170h/+210h: a type-6 widget with vtable +94h < 3 is styled, any other
//    member only when forced; a widget in none of these is styled.
// Styling: type 6 -> vtable +88h(focused, 0, 1.0f); type 12h -> vtable +84h(focused).
void library_on_widget_refresh_004faf40(Raw screen, void* widget, Fe3LibraryHost& host);
// 004FB090 / 004FBF70, __thiscall(library, key), RET 4, EAX = entry or 0: by id (+3F8h entries,
// +48h) / by name (+3E8h entries, string +28h: equal length and, when non-empty, _stricmp == 0).
void* library_find_entry_by_id_004fb090(Raw library, std::int32_t id) noexcept;
void* library_find_entry_by_name_004fbf70(Raw library, Raw name, Fe3LibraryHost& host);
// 004FBE90 BSP_TacticalLibraryScreen_SetMapsAvailable, __thiscall(screen, BYTE check), RET 4:
// unless `check` is set and +454h appears in the first +470h record's dword list (+1Ch..+20h),
// 004FADC0(screen, -1, 0).
void library_set_maps_available_004fbe90(Raw screen, std::uint8_t check, Fe3LibraryHost& host);
// 004FC7A0 StepForward (__fastcall(ECX = carousel)): index (+0h) below count - 1 of its +14h..
// +18h entries -> index += 1, then the StepBack tail (004FC720).
void library_carousel_step_forward_004fc7a0(Raw carousel, Fe3LibraryHost& host);
// 004FC8C0 SelectInitialEntry (ECX unused): wanted = list item id != 0 ? game +730h : +734h; the
// first library entry (+448h) with +48h == wanted becomes +434h and +438h = float(i) + 0.004
// ([00CEB160], a double); none -> nothing.
void library_select_initial_entry_004fc8c0(Fe3LibraryHost& host);
// 004FC9D0 IsEntryUnlocked, __fastcall(screen), AL: entry = +448h[+434h]; +46h clear -> 0.
// Else +4B0h = entry +48h, the list item id is read (only bounds-checked against +470h), and
// 004FADC0(screen, first +470h record's list[+434h], +138h); AL = 1. The id never selects the
// record.
bool library_is_entry_unlocked_004fc9d0(Raw screen, Fe3LibraryHost& host);
// 004FCB30 FindRowById, __thiscall(screen, id), RET 4, EAX: the first row i < count(+460h, 2Ch
// records) whose list item (00A9BE00(+0B8h, i)) id equals `id`; else -1.
std::int32_t library_find_row_by_id_004fcb30(Raw screen, std::int32_t id, Fe3LibraryHost& host);
// 004FCEE0, __fastcall(string): data -> length + 1 bytes back to the sized pool.
void native_string_release_004fcee0(Raw string, Fe3LibraryHost& host);

// ---------------------------------------------------------------------------
// Batch C (continued): library entries and the preview model.
// ---------------------------------------------------------------------------
struct Fe3EntryHost {
    virtual ~Fe3EntryHost() = default;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual std::int32_t interlocked_decrement(void* count) = 0;                        // [00CE2220]
    virtual void interlocked_increment(void* count) = 0;                                // [00CE221C]
    virtual void ref_counted_destroy(void* object) = 0;                                 // vtable +0, ECX only
    virtual void object_delete_04(void* object) = 0;                                    // vtable +4h(1)
    virtual void matrix_copy_004134f0(void* to, const float from[16]) = 0;
    virtual void ref_counted_destroy_base_00bd30f0(void* object) = 0;
};

// 004FCFC0 BSP_TacticalLibraryPreviewModel_Construct, EAX = model: vtable 00CEB130 then
// 00CEB168; +4h = 1; +8h..+14h = 0; +18h = 2; identity matrix into +20h (004134F0); +1Ch = 0.
void* library_preview_model_construct_004fcfc0(Raw model, Fe3EntryHost& host);
// 004FD080 BSP_TacticalLibraryPreviewModel_Destruct: vtable 00CEB168; release the ref-counted
// +10h; delete +14h (vtable +4h(1)); string {+8h, +0Ch} to the pool; 00BD30F0.
void library_preview_model_destruct_004fd080(Raw model, Fe3EntryHost& host);
// 004FD1D0 BSP_TacticalLibraryEntry_Destruct: the strings at +64h, +5Ch, +54h, +4Ch, +38h, +30h,
// +28h (in that order) to the pool; release +14h and +0h.
void library_entry_destruct_004fd1d0(Raw entry, Fe3EntryHost& host);
// 004FD3A0 (named CountAvailableMaps; it is a lookup), __thiscall(library, ref* out_object,
// out[16]), RET 8, AL: the +3F8h entry whose +48h equals +454h -> *out_object = entry +0h
// (ref-counted assign: the new one AddRef'd, the old Released) and out = entry +4h..+13h.
bool library_find_current_entry_004fd3a0(Raw library, void** out_object, std::uint8_t out[16], Fe3EntryHost& host);

// ---------------------------------------------------------------------------
// Batch C (continued): VehicleClass Lua readers. A native string output is resized with
// preserve 0 (0041DD40) and the bytes copied without the terminator.
// ---------------------------------------------------------------------------
struct Fe3LuaHost {
    virtual ~Fe3LuaHost() = default;
    virtual void* lua_globals_00b67980() = 0;                                           // of [00E188A8]+1A0Ch
    virtual void* lua_get_by_name_00b67800(void* table, const char* name) = 0;
    virtual void* lua_get_by_index_00b67720(void* table, std::int32_t index) = 0;
    virtual const char* lua_get_string_00b662b0(void* object) = 0;
    // 00B685C0: a native string from the object, or the default when it is not a string.
    virtual void lua_string_or_default_00b685c0(void* object, const char* fallback, void* out_string) = 0;
    virtual void lua_release_00b67700(void* object) = 0;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
};

// 004FD7C0 GetType / 004FDA50 GetNameOrNoUnlock, slot(index, NativeString* out), RET 8:
// out = globals.VehicleClass[index].Type / .Name. 004FDA50 with index -1 answers
// "FE.unitlib_nounlock" (00CEB184) without Lua.
void vehicle_class_get_type_004fd7c0(std::int32_t index, Raw out, Fe3LuaHost& host);
void vehicle_class_get_name_or_no_unlock_004fda50(std::int32_t index, Raw out, Fe3LuaHost& host);
// 004FD8D0 GetUnlockDlcMinimum, slot(index, out), RET 8: -1 -> "0" (00CE43BC); else
// .UnlockDLCMinimum through 00B685C0 with default "0" into a temporary string, copied to out
// (self-copy skipped) and the temporary returned to the pool.
void vehicle_class_get_unlock_dlc_minimum_004fd8d0(std::int32_t index, Raw out, Fe3LuaHost& host);
// 004FDF20, 004FDFD0, 004FE080, 004FE130 (byte-identical but for addresses), __fastcall(ECX =
// field name, EDX = NativeString* out) with a LuaObject (14h bytes) by value, RET 14h: out =
// object[name] as a string; the by-value object is destroyed by the callee. 004FE1E0: the same
// with ECX = const int* index (object[*index]).
void lua_object_read_string_field(const char* name, Raw out, void* object_by_value, Fe3LuaHost& host);
void lua_object_read_string_by_index_004fe1e0(const std::int32_t* index, Raw out, void* object_by_value,
    Fe3LuaHost& host);

// ---------------------------------------------------------------------------
// Batch C (continued): library movie, loading worker, ref-pointer arrays.
// ---------------------------------------------------------------------------
struct Fe3WorkerHost {
    virtual ~Fe3WorkerHost() = default;
    virtual void movie_player_stop_004f8ac0() = 0;                                      // ECX = [00E18D48]
    virtual const char* global_string_00e18d78() = 0;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
    virtual void native_string_assign_0041e870(void* string, const char* text) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual void sound_stream_start_00a867b0(const void* name) = 0;                     // ECX = [[00E198AC]+50h]
    virtual void sound_stream_fade_in_00a85c20() = 0;                                   // ECX = [[00E198AC]+50h]
    virtual void event_virtual_08(void* event) = 0;                                     // vtable +8h (wait)
    virtual void event_virtual_0c(void* event) = 0;                                     // vtable +0Ch (signal)
    virtual void* resource_load_007188a0(const void* name) = 0;                         // ECX = name string
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void interlocked_increment(void* count) = 0;
    virtual std::int32_t interlocked_decrement(void* count) = 0;
    virtual void ref_counted_destroy(void* object) = 0;                                 // vtable +0, ECX only
    virtual void library_entry_assign_004fe310(void* to, const void* from) = 0;
    virtual void* lua_get_by_index_00b67720(void* table, std::int32_t index) = 0;
    virtual bool lua_is_nil_00b65fb0(void* object) = 0;
    virtual const char* lua_get_string_00b662b0(void* object) = 0;
    virtual void lua_release_00b67700(void* object) = 0;
};

// 004FE7A0 BSP_TacticalLibrary_StopMovie: 004F8AC0; a temporary copy of the string at 00E18D78
// (resize(strlen, 1), copy with the terminator) starts the menu sound stream (00A867B0), the
// temporary goes back to the pool, then 00A85C20 fades it in.
void library_stop_movie_004fe7a0(Fe3WorkerHost& host);
// 00501510 BSP_LoadingQueue_WorkerMain, thread procedure __stdcall(queue +4h), RET 4, EAX = 1.
// event = [+8h] (queue +0Ch): wait (vtable +8h); until the stop flag ([+0h]) is set: the request
// [[+4h]+8h] gets +0Ch = 007188A0(copy of its +0h/+4h name, or ""), then signal (vtable +0Ch)
// and wait again.
std::uint32_t loading_queue_worker_main_00501510(Raw state, Fe3WorkerHost& host);
// 005017A0 TMPL_RefPtrArray_Reserve, __thiscall(array {data, size, capacity}, n), RET 4: n < 1
// -> 1; capacity >= n -> nothing; else a new block (00BF55BE, n * 4) gets AddRef'd copies, the old
// elements are Released and zeroed, the old block freed (00BF6989), data = new, capacity = n.
void ref_ptr_array_reserve_005017a0(Raw array, std::int32_t n, Fe3WorkerHost& host);
// 00501D10, __fastcall(ECX = const int* index, EDX = NativeString* out) with a LuaObject (14h)
// by value and a const NativeString* fallback, RET 18h: field = object[*index]; nil -> out =
// fallback, else out = a temporary built from the string (0041E870), then pooled.
void library_read_lua_string_field_00501d10(const std::int32_t* index, Raw out, void* object_by_value,
    Raw fallback, Fe3WorkerHost& host);
// 00501E10 copy of 6Ch library entries (cdecl(first, last, dest), EAX = dest + count) via 004FE310.
void* library_entry_copy_00501e10(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest, Fe3WorkerHost& host);

// ---------------------------------------------------------------------------
// Batch C (continued): ref-pointer resize, 2Ch records, dword vectors.
// ---------------------------------------------------------------------------
struct Fe3VectorHost {
    virtual ~Fe3VectorHost() = default;
    virtual void ref_ptr_array_reserve_005017a0(void* array, std::int32_t n) = 0;
    virtual std::int32_t interlocked_decrement(void* count) = 0;
    virtual void ref_counted_destroy(void* object) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual void vector_length_error_00504f10() = 0;
    virtual void* allocate_dwords_004f9ad0(std::uint32_t count) = 0;                    // ECX = count, EDX = 0
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
    virtual void dword_vector_copy_construct_004f6210(void* to, const void* from) = 0;
    virtual void* listbox_selected_item_00a9ca00(void* listbox) = 0;
    virtual void* item_virtual_28(void* item) = 0;
    virtual const char* empty_string_00e18d78() = 0;
    virtual std::int32_t atol_00bf8417(const char* text) = 0;
    virtual void library_refresh_unlock_rows_00503250(Raw screen, std::uint32_t entry_id) = 0;
};

// 00504960 TMPL_RefPtrArray_Resize, __thiscall(array, n), RET 4: n > capacity -> 005017A0; new
// slots from size to n are nulled; while n < size the last element is Released and zeroed; size
// = n.
void ref_ptr_array_resize_00504960(Raw array, std::int32_t n, Fe3VectorHost& host);
// 00504C60 ShowCurrentEntry, __fastcall(screen): +4B0h = (+448h entry [+434h]) +48h; the list
// item id is read (only bounds-checked against +470h); 00503250(screen, +4B0h); +134h = +4B0h.
void library_show_current_entry_00504c60(Raw screen, Fe3VectorHost& host);
// 005052B0, ECX = first, EDX = last, two unused stack words, RET 8: per 2Ch record free the
// vector +1Ch (zeroing +1Ch..+24h) and return the string +10h to the pool.
void record_2c_destroy_range_005052b0(std::uint8_t* first, std::uint8_t* last, Fe3VectorHost& host);
// 00505750, as 00557200 with 00504F10 and 004F9AD0.
bool dword_vector_construct_n_00505750(Raw vector, std::uint32_t n, Fe3VectorHost& host);
// 00505CC0, __fastcall(record): +10h, +14h, +1Ch..+28h = 0; +0Dh = 1 (+0h..+0Ch untouched).
void record_2c_clear_storage_00505cc0(Raw record) noexcept;
// 00505F10 copy construct, __thiscall(to, from), RET 4, EAX = to: as 00506140 but the string is
// zeroed first and the vector +18h is copy-constructed (004F6210).
void* record_2c_copy_construct_00505f10(Raw to, Raw from, Fe3VectorHost& host);

// ---------------------------------------------------------------------------
// Batch C (continued): the GIAchievements loading queue. Queue: +0h vtable, +0Ch worker/event,
// +10h job pointer vector {data, size +14h}, +1Ch thread. Job: +0h state (0 request file, 1
// waiting for the file store, 2 file ready, 3 loading), +4h cancelled, +8h work item list, +0Ch
// work count, +14h file name, +1Ch completion callback.
// ---------------------------------------------------------------------------
struct Fe3QueueHost {
    virtual ~Fe3QueueHost() = default;
    virtual void loading_queue_retire_front_00506bf0(Raw queue) = 0;
    virtual bool vfs_flag_78() = 0;                                                     // byte [0109CEEC]+78h
    // 004FC150 factory, 00BE80B0 GetOrCreate(name, 00504850), 00BE7CD0 RequestFile.
    virtual bool file_store_request_file(const void* name) = 0;
    virtual void loading_job_ensure_file_block_00504790(Raw job) = 0;
    virtual void loading_queue_start_work_00501620(Raw queue, Raw job) = 0;
    virtual bool worker_virtual_10(void* worker) = 0;                                   // vtable +10h, busy
    virtual void release_native_resource_slot_00483850(void* slot) = 0;
    virtual void call_completion(void* callback, std::uint32_t a, std::uint32_t b) = 0;
    virtual void loading_work_items_erase_00501670(void* list, std::uint32_t argument) = 0;
    virtual void loading_queue_stop_and_wait_005092e0(Raw queue) = 0;
    virtual void loading_job_pointers_resize_004fb4b0(void* vector, std::uint32_t size) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void object_delete(void* object) = 0;                                       // vtable +0(1)
    virtual void clear_queue_singleton_00e18d4c() = 0;
};

// 00509190 BSP_LoadingQueue_Update, __fastcall(queue). With jobs: a cancelled front job in state 0
// or 2 is retired first. Then by the front job's state:
//  0: request its file through the file store; with [0109CEEC]+78h set, success -> state 1 and
//     failure marks it cancelled; otherwise state = success ? 1 : 2.
//  2: 00504790, state 3, 00501620.
//  3: worker busy (vtable +10h) -> nothing. Cancelled -> release [job +8h]+0Ch and retire every
//     job. Else the callback (+1Ch) runs with (item +0Ch, item +8h) of the first work item,
//     00501670 erases it, and the job is retired when no work is left or restarted (00501620).
void loading_queue_update_00509190(Raw queue, Fe3QueueHost& host);
// 00509FD0 BSP_LoadingQueue_Destroy: vtable 00CEB198; 005092E0; jobs resize(0) and free; the
// worker +0Ch deleted (vtable +0(1)); [00E18D4C] = 0; vtable 00CE3818.
void loading_queue_destroy_00509fd0(Raw queue, Fe3QueueHost& host);

// ---------------------------------------------------------------------------
// Batch C (continued): award records, DLC list, order-of-battle view.
// ---------------------------------------------------------------------------
struct Fe3AwardHost {
    virtual ~Fe3AwardHost() = default;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual void string_vector_pair_copy_00509830(void* to, const void* from) = 0;
    virtual void dword_vector_copy_construct_004f6210(void* to, const void* from) = 0;
    virtual void award_record_copy_construct_0050a210(void* to, const void* from) = 0;
    virtual void string_vector_clear_004954f0(void* vector) = 0;                        // erase [begin, end)
    virtual void string_vector_append_00450540(void* vector, const void* string) = 0;
    virtual void library_open_entry_as_unlocks_0050e3c0(Raw screen, std::uint32_t a, std::uint32_t b,
        std::uint32_t one) = 0;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;        // vtable +34h
    virtual void library_refresh_entry_detail_00510c20(Raw screen) = 0;
    virtual void library_toggle_view_00511b40(Raw screen) = 0;
    virtual void library_show_order_of_battle_page_004fff20(Raw screen) = 0;
    virtual void library_build_order_of_battle_list_00506660(Raw screen) = 0;
    virtual void library_reset_vehicle_class_order_00506510(Raw screen) = 0;            // cc9 (class registry)
    virtual void loading_queue_stop_and_wait_005092e0() = 0;                            // ECX = 004FDE20()
};

// 0050A210 BSP_AwardRecord_CopyConstruct, __thiscall(to, from), RET 4, EAX = to: strings +0h,
// +0Ch, +14h, +1Ch (each zeroed, then copied unless self), +8h, +24h byte, 00509830(+28h), +3Ch,
// +40h, 004F6210(+44h).
void* award_record_copy_construct_0050a210(Raw to, Raw from, Fe3AwardHost& host);
// 0050AE60, __thiscall(pair, const NativeString*, const record*), RET 8: string +0h, then
// 0050A210(+8h, record).
void* award_table_construct_pair_0050ae60(Raw pair, Raw key, const void* record, Fe3AwardHost& host);
// 0050A820 BSP_TacticalLibrary_ReceiveDlcList, __thiscall(screen, packet), RET 4: the string
// vector +47Ch is emptied; each of the 255 WORDs from packet +18h that is non-zero adds
// sprintf_s("DLC_%2d") (00CEB9F0, 1Eh buffer).
void library_receive_dlc_list_0050a820(Raw screen, const std::uint8_t* packet, Fe3AwardHost& host);
// 0050FD10, __thiscall(screen, a, b), RET 8, AL: 0050E3C0(a, b, 1); then the first +470h record
// holds more than one dword in its +18h list.
bool library_is_map_unlocked_0050fd10(Raw screen, std::uint32_t a, std::uint32_t b, Fe3AwardHost& host);
// 00514FB0 OpenOrderOfBattleView, __fastcall(screen): [+4FCh] and [+0A8h] shown, [+0C4h] hidden;
// [+0C4h]+77h = 1, [+0B8h]+77h = 0, [+0B8h] shown; 00510C20; +94h = 5, +98h = 99; +90h == 1 ->
// 00511B40; 004FFF20; 00506660; 00506510; tail 005092E0 on 004FDE20().
void library_open_order_of_battle_view_00514fb0(Raw screen, Fe3AwardHost& host);

// ---------------------------------------------------------------------------
// Batch C (continued): library exit, front-end frame placement.
// ---------------------------------------------------------------------------
struct Fe3FrameHost {
    virtual ~Fe3FrameHost() = default;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;        // vtable +34h
    virtual void object_delete_04(void* object) = 0;                                    // vtable +4h(1)
    virtual void loading_queue_stop_and_wait_005092e0() = 0;                            // ECX = 004FDE20()
    virtual void library_preview_clear_rotors_0050a950(Raw screen) = 0;
    virtual void scroller_attach_content_00683790(void* scroller, void* content) = 0;
    virtual void renderer_set_optional_sync_00b33aa0(std::uint32_t value) = 0;          // ECX = [00F8D394]
    virtual const float* widget_local_position_00aa67f0(void* widget) = 0;
    virtual const float* widget_size_00aa6740(void* widget) = 0;
    virtual void widget_set_local_position_00aa7dc0(void* widget, const float xyz[3]) = 0;
    virtual void gui_text_set_localised_00abbe50(void* text, const char* key, std::uint32_t flag) = 0;
    virtual const char* rank_name_00e08e00(std::int32_t rank) = 0;                      // [00E08E00 + 4 * rank]
    virtual void widget_virtual_88(void* widget, std::int32_t frame, std::int32_t zero, float alpha) = 0;
};

// 0050B2D0 BSP_TacticalLibraryScreen_Exit, __fastcall(screen): hide [+540h..+554h] (non-null);
// delete +508h, +50Ch, +510h and the two 40-entry arrays +1D8h / +278h (vtable +4h(1), zeroed);
// +51Ch..+524h = 0; +9Ch = 0, +0A0h = 2; 005092E0; 0050A950; +4B0h = +4B4h = -1; 00683790(
// scroller +0CCh, 0); 00B33AA0(0).
void library_exit_0050b2d0(Raw screen, Fe3FrameHost& host);
// 00517B00, __fastcall(ECX = a, EDX = b, BYTE b_is_anchor), RET 4. Anchor b: a goes to b.x -
// float(b +114h / 960) - 0.003 (b's y, z). Otherwise b goes to a.x + float(a +114h / 960) +
// 0.003. 960.0 and 0.003 are the doubles 00CEC380 / 00CEBE08; x87 keeps the sum in extended
// precision until the float store (the C++ uses double).
void frame_offset_widget_x_00517b00(void* a, void* b, std::uint8_t b_is_anchor, Fe3FrameHost& host);
// 00517BA0, __fastcall(ECX = badge, EDX = icon, text, rank), RET 8: r = clamp(rank - 1, 0, 13h);
// text = rank name r (00E08E00); icon vtable +88h(r, 0, 1.0f); with x = text.x - float(text +114h
// / 960): icon.x = x - icon width - 0.003, then badge.x = x - 2 * icon width - 0.006 (00CEC390);
// y and z from the text.
void set_player_rank_widget_00517ba0(void* badge, void* icon, void* text, std::int32_t rank, Fe3FrameHost& host);

// ---------------------------------------------------------------------------
// Batch C (continued): front-end frame pages. Page tables: +0Ch [13][5] (5 entries 34h apart
// per row), +110h and +138h [2][5] (5 entries 8 apart per row). Selection globals 00E18D84,
// 00E18D88, 00E18D8C; the frame's own index is +160h.
// ---------------------------------------------------------------------------
struct Fe3PageHost {
    virtual ~Fe3PageHost() = default;
    virtual void gui_remove_page_00aa31f0(void* page) = 0;                              // ECX = 004C12B0()
    virtual void clear_frame_singleton_00e18d80() = 0;
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;
    virtual Raw frame_get_or_create_004c1ac0() = 0;
    virtual std::int32_t frame_selection(std::uint32_t global) = 0;                     // [00E18D84/88/8C]
    virtual void* gui_find_child_00aa7e00(void* parent, const void* name, std::uint32_t one) = 0;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual const float* widget_local_position_00aa67f0(void* widget) = 0;
    virtual void widget_set_local_position_00aa7dc0(void* widget, const float xyz[3]) = 0;
    virtual void push_page_slot_004d6790(void* vector, void** slot) = 0;
};

// 00517D90 BSP_FrontEndFrame_Destruct: vtables 00CEC39C / +8h 00CEC398; every non-null page of
// the three tables is removed (004C12B0 / 00AA31F0); [00E18D80] = 0; +8h = 00CE3818; 004F71A0.
void frame_destruct_00517d90(Raw frame, Fe3PageHost& host);
// 005186B0 SetTitlePosition, __fastcall(ECX = box position, EDX = title position): on the
// current title page (frame [+138h table] at [00E18D8C] + 2 * frame +160h) the children
// "title_Text" (00CEC3D8) and "szarnyas_FrameBox" (00CEC3C4) are found through temporary
// strings; the box takes ECX's vector, the title EDX's (00AA7DC0). 00518820 GetTitlePosition
// (ECX = box out, EDX = title out) copies them back (00AA67F0).
void frame_set_title_position_005186b0(const float box[3], const float title[3], Fe3PageHost& host);
void frame_get_title_position_00518820(float box[3], float title[3], Fe3PageHost& host);
// 00518E00 BSP_FrontEndFrame_CollectChildren, __thiscall(frame, out), RET 4: pushes the address
// of each non-null current page: +0Ch + 4 * (13 * +160h + [00E18D84]), +4 * ([00E18D88] + 2 *
// +160h + 44h), +4 * ([00E18D8C] + 2 * +160h + 4Eh).
void frame_collect_children_00518e00(Raw frame, void* out, Fe3PageHost& host);

// ---------------------------------------------------------------------------
// Batch D: control binding screen, input-settings trees (continued), HUD pilot/bomber.
// ---------------------------------------------------------------------------
struct Fe3InputHost {
    virtual ~Fe3InputHost() = default;
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
    virtual void* tree_buy_node(std::uint32_t routine, void* tree, void* left, void* parent, void* right,
        const void* value, std::uint8_t colour) = 0;
    virtual void* tree_copy_subtree(std::uint32_t routine, void* tree, void* node, void* parent) = 0;
    virtual void value_copy(std::uint32_t routine, void* to, const void* from) = 0;
    virtual void int_scalar_tree_erase_range_005572f0(void* tree, void* result, void* first_node, void* last_node) = 0;
    virtual void vector_length_error_00556c90() = 0;
    virtual void* allocate_16_00553300(std::uint32_t count) = 0;                        // ECX = count, EDX = 0
    // Screens.
    virtual void front_end_screen_construct_004f7180(Raw screen) = 0;
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;
    virtual void front_end_screen_register_004f71d0(Raw screen) = 0;
    virtual void gui_scroller_construct_00683610(void* scroller) = 0;
    virtual void gui_remove_page_00aa31f0(void* page) = 0;                              // ECX = 004C12B0()
    virtual void* gui_load_page_00aa5840(const void* name, std::uint32_t one, std::uint32_t zero) = 0;  // ECX = 004C12B0()
    virtual void screen_virtual_14(Raw screen) = 0;
    virtual std::uint32_t widget_type_5c(void* widget) = 0;
    virtual void widget_set_float_4c(void* widget, float value) = 0;
    virtual void widget_virtual_84(void* widget, std::uint32_t focus) = 0;
    virtual void widget_virtual_88(void* widget, std::uint32_t a, std::uint32_t b, float alpha) = 0;
    // HUD root [00E198C4]: +4Ch binoculars, +50h screen 2Eh, +68h plane screen.
    virtual Raw hud_root_00e198c4() = 0;
    virtual void binoculars_update_view_0051f330(void* binoculars, float dt) = 0;
    virtual void plane_update_instruments_00609bd0(void* plane, float dt, std::uint32_t a, std::uint32_t b) = 0;
    virtual void screen_2e_set_view_centre_005454b0(void* screen, float x, float y, float z) = 0;
};

// Copy of a subtree, __thiscall(tree, node, parent), RET 8, EAX: a nil node gives the head;
// otherwise a node from the row's buy-node routine (head, parent, head, &value, colour) whose
// left and right are copied recursively.
struct TreeCopySubtree {
    std::uint32_t address;
    std::uint32_t buy_node;
    std::uint8_t is_nil_offset;
    std::uint8_t colour_offset;
};
inline constexpr std::array<TreeCopySubtree, 4> kTreeCopySubtree{{
    {0x00555410, 0x005550D0, 0x11, 0x10}, {0x005555B0, 0x00554F50, 0x15, 0x14},
    {0x00559460, 0x005591E0, 0x29, 0x28}, {0x0055B060, 0x0055A290, 0x1D, 0x1C},
}};
void* tree_copy_subtree(const TreeCopySubtree& row, Raw tree, void* node, void* parent, Fe3InputHost& host);
// 005550D0: 14h node, value dword inline at +0Ch, colour +10h, is-nil +11h (as 00554F50).
void* int_key_tree_buy_node_005550d0(void* left, void* parent, void* right, const std::uint32_t* value,
    std::uint8_t colour, Fe3InputHost& host);
// 0055A290: 20h node; value {dword at +0Ch, a tree copy-constructed at +10h by 00558570 from
// value +4h}; colour +1Ch, is-nil +1Dh. Returns 0 when operator new fails.
void* int_scalar_map_buy_node_0055a290(void* left, void* parent, void* right, const void* value,
    std::uint8_t colour, Fe3InputHost& host);
// 00555B20, __thiscall(tree, const tree&), RET 4: as 0055B300 with 005555B0 and is-nil +15h.
void int_scalar_tree_assign_00555b20(Raw tree, Raw source, Fe3InputHost& host);
// 00559610, __fastcall(owner): the tree at owner +4h erases [head->left, head) through 005572F0,
// frees its head, head and size = 0.
void int_scalar_tree_clear_at_4_00559610(Raw owner, Fe3InputHost& host);
// 0055AF60, __thiscall(tree, node), RET 4: subtree destruction (is-nil +1Dh) whose node value is
// a tree at node +10h, cleared as 00559610 does before the node is freed.
void int_tree_tree_destroy_subtree_0055af60(void* node, Fe3InputHost& host);
// 00557250, __thiscall(vector, n), RET 4, AL: as 00557200 for 10h-byte elements (limit 0FFFFFFFh,
// 00556C90, 00553300).
bool vector16_construct_n_00557250(Raw vector, std::uint32_t n, Fe3InputHost& host);
// String + dword + tail copy, __thiscall(to, from), RET 4: string +0h, dword +8h, the routine on
// (+0Ch, from +0Ch). 005586B0 takes (const NativeString*, const {dword, tail}*), RET 8.
struct StringDwordTailCopy {
    std::uint32_t address;
    std::uint32_t tail_copy;
};
inline constexpr std::array<StringDwordTailCopy, 2> kStringDwordTailCopy{{{0x00558280, 0x00557590}, {0x00558F80, 0x00557820}}};
void* string_dword_tail_copy(const StringDwordTailCopy& row, Raw to, Raw from, Fe3InputHost& host);
void* string_sensitivity_pair_construct_005586b0(Raw to, Raw key, const std::uint8_t* value, Fe3InputHost& host);
// 0055B920, __thiscall(to, from), RET 4, EAX = to: ten members copy-constructed in order.
struct MemberCopy {
    std::uint8_t offset;
    std::uint32_t routine;
};
inline constexpr std::array<MemberCopy, 10> kInputDeviceRecordMembers{{
    {0x00, 0x00559F30}, {0x0C, 0x0055A770}, {0x18, 0x0055A800}, {0x24, 0x0049DEC0}, {0x30, 0x00557BC0},
    {0x40, 0x00557BC0}, {0x50, 0x0055B400}, {0x5C, 0x0055B150}, {0x6C, 0x0055ABD0}, {0x78, 0x00557670},
}};
void* input_device_record_copy_0055b920(Raw to, Raw from, Fe3InputHost& host);
// 0055BA80, __thiscall(pair, const NativeString*, const record*), RET 8: string +0h, then
// 0055B920(+8h, record).
void* input_device_pair_construct_0055ba80(Raw pair, Raw key, const void* record, Fe3InputHost& host);
// 00555770 BSP_ControlBindingScreen_Construct, EAX: 004F7180; +8h/+0Ch = 00CEB0FC/00CEB110, then
// vtables 00CEE524 / +8h 00CEE510 / +0Ch 00CEE4F0; strings +3Ch and +184h = {0, 0}; scroller
// +1A0h; +0E4h = 0; +0E0h = 10.
void* control_binding_construct_00555770(Raw screen, Fe3InputHost& host);
// 00555820 Destruct: vtables as above; pages +14h, +18h removed; strings +184h, +3Ch to the pool;
// +8h = 00CEB0FC; 004F71A0.
void control_binding_destruct_00555820(Raw screen, Fe3InputHost& host);
// 00555C60 Register: 004F71D0; pages "FE_controls_PC_listbox" (00CEE55C) -> +14h and
// "FE_controls_PC" (00CEE54C) -> +18h through 00AA5840(name, 1, 0); +16Ch..+17Ch = 0; +180h = 1;
// vtable +14h.
void control_binding_register_00555c60(Raw screen, Fe3InputHost& host);
// 00560FF0, listener (widget, BYTE focused), RET 8: type 2 -> nothing; type 3 -> vtable +4Ch(
// focused ? 0.5 : 1.0). Otherwise only the widget at +0A0h while +70h is clear is styled (type 6:
// vtable +88h(focused, 0, 1.0f); type 12h: vtable +84h(focused)). A second test of +124h/+0F8h
// branches to the same instruction either way, so it has no effect.
void leaderboards_listener_style_00560ff0(Raw listener, void* widget, std::uint8_t focused, Fe3InputHost& host);
// 005191B0, __thiscall(screen, float dt), RET 4: binoculars 0051F330(dt); plane screen [+68h]
// active (+5h) -> 00609BD0(dt, 0, 0); 005454B0 on [+50h] with (0.5, 0.5, binoculars +38h).
void hud_pilot_update_binocular_view_005191b0(float dt, Fe3InputHost& host);
// 00519830 BSP_HudBomberScreen_Register: 004F71D0; +20h = +1Ch = +28h = 0; page "GUI_bomber"
// (00CEC738) -> +2Ch.
void hud_bomber_register_00519830(Raw screen, Fe3InputHost& host);

// ---------------------------------------------------------------------------
// Batch D (continued): leaderboards screen. Rows are 40h bytes: +0h dword, strings +4h, +0Ch,
// +14h, +1Ch dword, +20h..+3Fh eight dwords. List boxes: boards +94h (fallback index +0B8h),
// entries +11Ch (+148h); scrollers +34h, +0BCh.
// ---------------------------------------------------------------------------
struct Fe3BoardHost {
    virtual ~Fe3BoardHost() = default;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    virtual void listbox_clear_00a9cdc0(void* listbox) = 0;
    virtual void listbox_select_index_00a9c7c0(void* listbox, std::int32_t index) = 0;
    virtual void listbox_clear_rows_00a9bec0(void* listbox) = 0;
    virtual void widget_set_focus_60(void* widget, std::uint32_t focus) = 0;            // vtable +60h
    virtual Raw board_row_00e19468(std::int32_t index) = 0;                             // 40h rows at [00E19468]
    virtual void gui_text_set_localised_source_00abaed0(void* text, const void* source, std::uint32_t flag) = 0;
    virtual void leaderboards_move_selection_00561860(Raw screen, void* scroller, std::uint32_t a, std::uint32_t b,
        std::uint32_t flag) = 0;
    virtual void please_wait_hide_00e19698() = 0;                                       // [[00E19698]]+4 = 0
    virtual void set_menu_return_00e08874(std::uint32_t value) = 0;
    virtual void front_end_push_interface_request_004cc460(std::uint32_t a, std::uint32_t b) = 0;  // ECX = [00E198AC]
    virtual void frame_set_selection_00518da0(std::uint32_t selection) = 0;
    virtual void scroller_attach_content_00683790(void* scroller, void* content) = 0;
    virtual void scroller_end_thumb_drag_00683320(void* scroller) = 0;
    virtual void gui_remove_page_00aa31f0(void* page) = 0;                              // ECX = 004C12B0()
    virtual void wide_string_range_destroy_004d0b80(void* first, void* last, void* a, void* b) = 0;
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;
    virtual bool game_reading_7058() = 0;                                               // [00E188A8]+7058h != 0
    virtual Raw please_wait_screen_00e19698() = 0;
    virtual void front_end_commit_visibility_004f83b0(Raw screen) = 0;
    virtual void screen_virtual_18(Raw screen) = 0;
    virtual void please_wait_render_now_0060d570() = 0;
    virtual void* wide_string_vector_erase_004d6ea0(void* vector) = 0;                  // erase all, returns begin
    virtual std::int32_t leaderboard_mode_00e19430() = 0;
    virtual std::uint64_t xenon_active_xuid_00a3eb00() = 0;
    virtual void game_begin_leaderboard_read_004bc6d0(std::uint32_t zero, std::uint64_t xuid, std::uint32_t first,
        std::uint32_t count, std::uint32_t zero2, std::uint32_t friends_word) = 0;     // ECX = [00E188A8]
};

// 00561660, __fastcall(screen): index = boards list box active (+85h) ? its selection : +0B8h; the
// row's +14h string goes to text [+9Ch] (00ABAED0(source, 1)); +150h = 0.5, +154h = 1.
void leaderboards_show_board_name_00561660(Raw screen, Fe3BoardHost& host);
// 005617C0 row destroy: strings +14h, +0Ch, +4h to the pool. 00562490 / 005625A0 row copy
// construct / assign, __thiscall(to, from), RET 4, EAX = to (the copy zeroes each string first).
void leaderboard_row_destroy_005617c0(Raw row, Fe3BoardHost& host);
void* leaderboard_row_copy_00562490(Raw to, Raw from, bool construct, Fe3BoardHost& host);
// 00561AD0 SyncSelection, __thiscall(screen, BYTE boards), RET 4: nothing while +154h is set or
// when the list box being left is active (+85h). boards: +148h = entries selection, entries
// cleared, boards focused and selected at +0B8h; else the mirror (+0B8h = boards selection, boards
// cleared, entries focused at +148h). Then 00561860(+0BCh, entries, +118h, 1); +8Dh = boards,
// +115h = !boards.
void leaderboards_sync_selection_00561ad0(Raw screen, std::uint8_t boards, Fe3BoardHost& host);
// 005629A0 ReturnToMenu, __fastcall(screen): please-wait +4h = 0; [00E08874] = 0Bh; 004CC460(1,
// 0); 00518DA0(0Ch); rows of both list boxes cleared and both scrollers detached.
void leaderboards_return_to_menu_005629a0(Raw screen, Fe3BoardHost& host);
// 00563440 BSP_LeaderboardsScreen_Destruct: vtables 00CEE9CC / 00CEE9B8 / 00CEE998; page +30h
// removed; the wide string vector +10h destroyed (004D0B80), freed, zeroed; +8h = 00CEB0FC;
// 004F71A0.
void leaderboards_destruct_00563440(Raw screen, Fe3BoardHost& host);
// 005635F0 (named SetLimboState; it starts a leaderboard read), __thiscall(screen, ignored),
// RET 4: +7Ch -> 00683320(+34h). Unless a read is running (game +7058h): show the please-wait
// screen (+4h = +5h = 1, 004F83B0, vtable +18h, 0060D570), empty the wide string vector +10h
// (+20h = the returned begin), +28h = +2Ch, and 004BC6D0(0, XUID (mode 2 only, else 0), +20h,
// +24h, 0, a dword whose low byte is mode == 1 and whose upper bytes are whatever EDX held); +15Ch
// = 0.
void leaderboards_begin_read_005635f0(Raw screen, Fe3BoardHost& host);

// ---------------------------------------------------------------------------
// Batch D (continued): leaderboard reads and the HUD limbo screen.
// ---------------------------------------------------------------------------
struct Fe3ReadHost {
    virtual ~Fe3ReadHost() = default;
    virtual void listbox_set_listener_00a9ac40(void* listbox, void* listener) = 0;
    virtual void listbox_clear_rows_00a9bec0(void* listbox) = 0;
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    virtual Raw board_row_00e19468(std::int32_t index) = 0;
    virtual void gui_text_set_ellipsized_00abb000(void* text, const void* source, float width, std::uint32_t flag) = 0;
    virtual void leaderboards_begin_read_005635f0(Raw screen, std::uint32_t board) = 0;
    virtual void results_assign_00563760(void* vector, const void* results) = 0;
    virtual void leaderboards_fill_rows_00561eb0(Raw screen) = 0;
    virtual std::int32_t leaderboard_mode() = 0;                                        // [00E19430]
    virtual void set_leaderboard_mode(std::int32_t mode) = 0;
    virtual const char* filter_name_00e086b4(std::int32_t mode) = 0;
    virtual std::uint64_t xenon_active_xuid_00a3eb00() = 0;
    virtual Raw please_wait_screen_00e19698() = 0;
    virtual void gui_set_enabled_00aa0e00(std::uint32_t value) = 0;                     // ECX = 004C12B0()
    virtual void gui_set_pointer_visible_00aa0e50(std::uint32_t value) = 0;             // ECX = 004C12B0()
    virtual void native_string_assign_0041e870(void* string, const char* text) = 0;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
    virtual void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) = 0;
    virtual void* gui_load_page_00aa5840(const void* name, std::uint32_t one, std::uint32_t zero) = 0;
    virtual void front_end_screen_register_004f71d0(Raw screen) = 0;
    virtual const char* global_config_string(std::uint32_t offset) = 0;                 // [00432650()+offset+4]
    virtual const char* empty_string_00e19476() = 0;
    virtual void gui_text_set_localised_00abbe50(void* text, const char* key, std::uint32_t flag) = 0;
};

// 00564E70 ResetRows, __fastcall(screen): +148h = 0; the entries list box loses its listener and
// rows; text [+138h] shows the board row's +0Ch string (00ABB000, width -1.0 = [00D7A260], 1);
// +24h = 0; 005635F0(board row +0h); +154h = 2.
void leaderboards_reset_rows_00564e70(Raw screen, Fe3ReadHost& host);
// 00564F60 OnReadComplete, __thiscall(screen, const results*), RET 4: inactive (+5h) -> nothing.
// The current board id == +20h and +28h == +2Ch: results into +10h (00563760), 00561EB0, +154h =
// 0; mode 2 scans the 60h-byte results for the active XUID (+58h) and does nothing with the
// answer; please-wait +4h = 0; +158h -> GUI enabled and pointer shown. Otherwise, while the
// please-wait screen is up, 005635F0(board id) again.
void leaderboards_on_read_complete_00564f60(Raw screen, const void* results, Fe3ReadHost& host);
// 005650C0 CycleFilter, __fastcall(screen): mode = ([00E19430] + 1) % 3; text [+134h] = filter
// name [00E086B4 + 4 * mode] through a temporary string; +2Ch = 1; entries active -> +164h = 1;
// 00564E70.
void leaderboards_cycle_filter_005650c0(Raw screen, Fe3ReadHost& host);
// 00565EE0 BSP_HudLimboScreen_Register: 004F71D0; page "GUI_limbo" (00CEEBD4) -> +8h.
void hud_limbo_register_00565ee0(Raw screen, Fe3ReadHost& host);
// 00565FB0 BSP_HudLimboScreen_SetState, __thiscall(screen, state), RET 4: +14h = state; the
// message is the global config string at +0A8h / +0B8h / +0D0h / +0C0h / +0B0h for states 1..5
// (jump table 00566030), +0D8h otherwise ("" when null); shown on [+0Ch] while active (+5h).
void hud_limbo_set_state_00565fb0(Raw screen, std::int32_t state, Fe3ReadHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-fe6 additions (docs/CC13_FE_6.md)
// ---------------------------------------------------------------------------

// 00566080 BSP_HudLimboScreen_Enter, slot +18h of 00CEEBAC, __fastcall(ECX = screen), RET.
// +0Ch = "Message_Text" (00CED36C) and +10h = "Continue_Text" (00CEEBE0) under page +8h;
// +10h hidden; 00565FB0(+14h); timer +18h = 6.0f (00CE6630); 00A95CB0 clears every gamepad
// force channel.
struct Fe6LimboHost {
    virtual ~Fe6LimboHost() = default;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;  // vtable +34h
    virtual void hud_limbo_set_state_00565fb0(Raw screen, std::int32_t state) = 0;
    virtual void gamepad_force_clear_all_channels_00a95cb0() = 0;
};
inline constexpr float kLimboContinueDelay = 6.0f;  // 00CE6630
void hud_limbo_enter_00566080(Raw screen, Fe6LimboHost& host);

// 00561860 BSP_LeaderboardsScreen_MoveSelection, __thiscall(screen, scroller, list box, clip,
// bool no_scroll), RET 10h. The board list box (+94h) uses first/last/selected +B0h/+B4h/+B8h,
// any other list +140h/+144h/+148h. first = the list's row count (+104h), last = 0. With the
// clip's top-left C = pos - size * anchor (+18h, +1Ch), each row (00A9BE00) is visible when its
// top-left y (rounded to float) is below C.y and row.y + row.h < C.y + clip.h (both sums in
// x87 extended precision); visible rows update first (when smaller) or else last (when
// larger), and every row gets vtable +60h(visible). Then, while the scroller's +48h byte and
// +20h are clear and no_scroll is false, a selection above first scrolls toward the start
// (00683680) and one below last toward the end (00683700), by the current row's (00425E50)
// height. Otherwise a selection outside [first, last] snaps to first / last through 00A9C8F0
// and is stored in the selected slot.
struct Fe6LeaderboardHost {
    virtual ~Fe6LeaderboardHost() = default;
    virtual std::array<float, 3> widget_resolved_position_00aa6750(void* widget) = 0;
    virtual std::array<float, 2> widget_size_00aa6740(void* widget) = 0;
    virtual void* list_box_row_00a9be00(void* list_box, std::int32_t index) = 0;
    virtual void widget_set_enabled_60(void* widget, std::uint32_t enabled) = 0;  // vtable +60h
    virtual std::int32_t list_box_selected_index_00a9c920(void* list_box) = 0;
    virtual void* list_box_current_row_00425e50(void* list_box) = 0;
    virtual void list_box_select_index_silently_00a9c8f0(void* list_box, std::int32_t index) = 0;
    virtual void scroller_toward_start_00683680(void* scroller, float delta) = 0;
    virtual void scroller_toward_end_00683700(void* scroller, float delta) = 0;
};
void leaderboards_move_selection_00561860(Raw screen, void* scroller, void* list_box, void* clip, bool no_scroll,
    Fe6LeaderboardHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-fe8 additions (docs/CC13_FE_8.md): leaderboards.
// Screen layout used here: listener +8h (list boxes), +0Ch (rows and widgets); result rows
// vector +14h..+18h (60h bytes: +4h rank, +0Ch wide name, +10h value); first shown row +2Ch;
// scrollers +34h / +0BCh (embedded); boards list box +94h, its row template +98h, clip +90h;
// entries list box +11Ch, template +120h, clip +118h; total rows +14Ch; pending select +164h.
// ---------------------------------------------------------------------------
struct Fe8LeaderboardHost {
    virtual ~Fe8LeaderboardHost() = default;
    // Widget virtuals (ECX = widget).
    virtual std::int32_t widget_type_5c(void* widget) = 0;                                   // vtable +5Ch
    virtual void widget_set_alpha_4c(void* widget, float alpha) = 0;                         // vtable +4Ch
    virtual void widget_set_highlight_88(void* widget, std::uint32_t state, std::uint32_t zero, float one) = 0;  // +88h
    virtual void widget_set_highlight_84(void* widget, std::uint32_t state) = 0;              // vtable +84h
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;             // vtable +34h
    virtual void widget_set_enabled_60(void* widget, std::uint32_t enabled) = 0;             // vtable +60h
    // Scrollers (ECX = embedded scroller).
    virtual void scroller_set_auto_scroll_00683300(void* scroller, std::uint32_t a, float speed, std::uint32_t b) = 0;
    virtual void scroller_end_thumb_drag_00683320(void* scroller) = 0;
    virtual void scroller_attach_content_00683790(void* scroller, void* content) = 0;
    virtual void scroller_set_scroll_range_006834a0(void* scroller, float range) = 0;
    virtual void scroller_set_widget_alphas_00683330(void* scroller, float a, float up, float down) = 0;
    // Widgets and list boxes.
    virtual void* widget_clone_subtree_00aab4c0(void* source, std::uint32_t zero) = 0;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;     // (name, 1)
    virtual void gui_text_set_ellipsized_00abb000(void* text, std::string_view source, float width,
        std::uint32_t flag) = 0;
    virtual void widget_set_listener_fields_00aa6bc0(void* widget, void* listener, std::uint32_t zero) = 0;
    virtual void widget_set_hit_clip_from_00aa8f30(void* widget, void* clip, std::uint32_t zero) = 0;
    virtual std::array<float, 2> widget_size_00aa6740(void* widget) = 0;
    virtual void list_box_clear_rows_00a9bec0(void* list_box) = 0;
    virtual void list_box_set_listener_00a9ac40(void* list_box, void* listener) = 0;
    virtual void list_box_insert_row_00a9d750(void* list_box, void* row, std::uint32_t a, std::uint32_t b) = 0;
    virtual void list_box_select_index_00a9c7c0(void* list_box, std::int32_t index) = 0;
    virtual std::int32_t list_box_selected_index_00a9c920(void* list_box) = 0;
    virtual void list_box_clear_00a9cdc0(void* list_box) = 0;
    virtual void list_box_set_field124_00a9ceb0(void* list_box, float value, std::uint32_t zero) = 0;
    // Leaderboard data and screen routines.
    virtual Raw board_row_00e19468(std::uint32_t index) = 0;   // 40h rows at [00E19468], bounds-checked
    virtual std::int32_t leaderboard_mode_00e19430() = 0;
    virtual void leaderboards_show_board_name_00561660(Raw screen) = 0;
    virtual void leaderboards_move_selection_00561860(Raw screen, void* scroller, void* list_box, void* clip,
        bool no_scroll) = 0;
};

// 00561320 BSP_LeaderboardsWidgetListener_Current0C, __thiscall(listener = screen+0Ch, widget),
// RET 4. Type 2: nothing. Type 3: vtable +4Ch(+0D4h byte ? 0.5f (00CE3800) : 1.0f (00D7A24C)).
// Otherwise the boards widgets (listener +98h / +94h) stop the +34h scroller's auto-scroll
// (00683300(0, 0.0f, 0)) and the boards thumb (+0A0h) ends its drag (00683320); the entries
// widgets (+11Ch / +118h, thumb +124h) do the same on +0BCh. Then type 6 -> vtable +88h(+0D4h != 0,
// 0, 1.0f) and type 12h -> vtable +84h(+0D4h != 0). Listener offsets are screen offsets - 0Ch.
void leaderboards_listener_focus_00561320(Raw listener, void* widget, Fe8LeaderboardHost& host);

// 00561BE0 (named RefreshFilterLabel; it builds the boards list), __fastcall(ECX = screen), RET.
// With a boards list box (+94h): rows cleared, listener dropped, then 18 rows (i * 40h < 480h)
// cloned from +98h, each "filter_Text" (00CEE938) child showing board row i's +4h string
// (00ABB000, width -1.0f, 1), shown, listener +0Ch, clipped to +90h, +0D8h = 0, appended.
// Scroller +34h: content 0, range = (float)((double)(h98 * 18.0 + 1/72.0) - h90) (both doubles
// 00CEE930 / 00CEBDD0, the sum stored as a double). Row 0 selected, vtable +60h(1), +149h = 0,
// listener +8h, 00A9CEB0(0.3875f, 0), +12Ch = 1/96 (00CEE924), +130h = 1/360 (00CEE600),
// +134h = 0, +138h = 1, +14Ah = 0. Always: 00561660, +150h = 0, +154h = 1, +115h = 0.
void leaderboards_build_board_list_00561be0(Raw screen, Fe8LeaderboardHost& host);

// 00561EB0 BSP_LeaderboardsScreen_FillRows, __fastcall(ECX = screen), RET. Entries list box
// (+11Ch) loses its listener; one row per 60h result (clone of +120h): "name_Text" = the wide
// name at +0Ch narrowed by low bytes (00436630; L"" 00E19474 when null; flag 0), "rank_Text" =
// +4h and "value_Text" = +10h as decimals (004260B0; flag 1), width -1.0f; shown, listener +0Ch,
// clipped to +118h, +0D8h = 1, appended. Scroller +0BCh: range = (float)((double)(h120 * 1.005
// * count) - h118), the product stored as a double (00CEE950, FIMUL by list +104h). Then
// 00A9CDC0, +149h = 0, listener +8h, 00A9CEB0(5/12 (00CEE948), 0), +12Ch = 1/320 (00CEE944),
// +130h = 1/360, +134h = 0, +138h = 1, +14Ah = 0. A pending select (+164h) clears +154h and,
// when the entries list is inactive (+85h), moves focus there (boards selection to +0B8h, boards
// cleared, entries focused and selected at +148h, 00561860(+0BCh, entries, +118h, 1), +8Dh = 0,
// +115h = 1); +0B8h is then restored to its entry value, value 2 selects the last row, and +164h
// = 0. Arrow alphas by mode [00E19430] through 00683330 on +0BCh: mode 0 -> (0, first(+2Ch) - 17
// > 0 ? 1 : 0.25, first + 17 < total(+14Ch) ? 1 : 0.25); mode 1 -> all 1.0 when more than 17 rows,
// else (0, 0.25, 0.25); mode 2 -> (0, 0.25, 0.25); other modes leave them.
void leaderboards_fill_rows_00561eb0(Raw screen, Fe8LeaderboardHost& host);

struct Fe8LeaderboardEnterHost : Fe8LeaderboardHost {
    // 00518D60, ECX = 0, EDX = 0, stack (0, title NativeString), RET 8.
    virtual void frame_select_00518d60(std::uint32_t ecx, std::uint32_t edx, std::uint32_t zero,
        std::string_view title) = 0;
    virtual void frame_set_flag_00518b90(std::uint32_t flag) = 0;                            // ECX = flag
    virtual void command_bar_set_help_line_0054a0c0(std::string_view line) = 0;              // ECX = [00E1930C]
    virtual void gui_text_set_localised_cstring_00abbe50(void* text, const char* key, std::uint32_t flag) = 0;
    virtual const char* filter_name_00e086b4(std::int32_t mode) = 0;                         // [00E086B4 + mode*4]
    virtual void leaderboards_build_board_list_00561be0(Raw screen) = 0;
};

// 00562750 BSP_LeaderboardsScreen_Enter, slot +18h of 00CEE9CC, __fastcall(ECX = screen), RET.
// 00518D60(ECX 0, EDX 0; 0, "FE.main_leaderboards_title" (00CEE97C)); 00518B90(ECX 2); help line
// "" (00CE3A0C) through 0054A0C0 on [00E1930C]. The title text [+13Ch] is re-set to a copy of its
// own string (+0F4h/+0F8h): 00ABBE50("", 1) then 00ABB000(copy, -1.0f, 1). The filter text
// [+134h] shows the [00E086B4 + mode*4] key (FE.lb_topplayers ...) through 00ABB000(-1.0f, 1).
// +20h = 0; 00561BE0; +158h = +160h = +168h (bytes) = 0, +14Ch = +164h = +15Ch = 0, +2Ch = 1.
void leaderboards_enter_00562750(Raw screen, Fe8LeaderboardEnterHost& host);

// Lua access shared by the table loaders. Handles follow the image's LuaObject temporaries;
// a handle is released (00B67700) by the host when the loader drops it.
struct Fe8LuaHost {
    virtual ~Fe8LuaHost() = default;
    // Script owner: 00B66BD0 construct, 00B6A020 Open(4), 00B69D40 RunScriptWithOverrides(path,
    // 0), 00B67980 GetGlobals (ECX = the owner), 00B669A0 Close.
    virtual void* lua_owner_open_run_00b6a020(std::uint32_t libs, std::string_view script) = 0;
    virtual void* lua_owner_globals_00b67980(void* owner) = 0;
    virtual void lua_owner_close_00b669a0(void* owner) = 0;
    virtual void* lua_get_by_name_00b67800(void* table, const char* name) = 0;
    virtual void* lua_get_by_index_00b67720(void* table, std::int32_t index) = 0;
    virtual bool lua_is_nil_00b65fb0(void* object) = 0;
    virtual std::int32_t lua_get_integer_00b66290(void* object) = 0;
    virtual const char* lua_get_string_00b662b0(void* object) = 0;                           // may be null
    virtual void lua_release_00b67700(void* object) = 0;
};

// The leaderboard row loader's view of a 40h row: +0h ID, strings +4h Name, +0Ch ColumnName,
// +14h TableDesc, +1Ch column count, +20h columns (the image stores Columns[j] at +1Ch + j*4).
struct Fe8BoardRow {
    std::int32_t id = 0;
    std::string name;
    std::string column_name;
    std::string table_desc;
    std::int32_t column_count = 0;
    std::array<std::int32_t, 8> columns{};
};
struct Fe8BoardTableHost : Fe8LuaHost {
    // The three dword vectors at 00E19434 + k*10h ({alloc, begin +4h, end +8h, capacity +0Ch}).
    virtual void board_ids_clear_00e19434(std::int32_t k) = 0;
    virtual void board_ids_push_00441f30(std::int32_t k, std::int32_t id) = 0;
    // The 40h row vector at 00E19464: erase all (00562F80), push_back (005636D0).
    virtual void board_rows_clear_00562f80() = 0;
    virtual void board_rows_push_005636d0(const Fe8BoardRow& row) = 0;
};

// 005638F0 BSP_LeaderboardsScreen_ReadTable, no register or stack arguments, RET. Runs
// "Scripts/datatables/Leaderboards.lua" (00CEEA38) in a private Lua owner opened with 4. The
// three id vectors and the row vector are emptied. For k = 1..3, the integers of
// Leaderboards.LB_UniqueID[k][1..] up to the first nil go to id vector k-1. For i = 1..18 a zeroed
// row is pushed for every i; when Leaderboards.LBoards[i] is not nil it is filled from ID (00CE59B4),
// Name (00CE8ED0), ColumnName, TableDesc (null strings -> empty) and Columns[1..] up to the first
// nil (the count lands at +1Ch). The image does not bound the column index: more than eight
// columns would write past the 40h row into the loader's other locals; this model stops at eight.
void leaderboards_read_table_005638f0(Fe8BoardTableHost& host);

struct Fe8LeaderboardRegisterHost {
    virtual ~Fe8LeaderboardRegisterHost() = default;
    virtual void front_end_screen_register_004f71d0(Raw screen) = 0;
    virtual void* gui_load_page_00aa5840(std::string_view name, std::uint32_t one, std::uint32_t zero) = 0;  // ECX = 004C12B0()
    virtual void leaderboards_read_table_005638f0() = 0;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;     // (name, 1)
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;             // vtable +34h
    virtual void widget_set_listener_fields_00aa6bc0(void* widget, void* listener, std::uint32_t zero) = 0;
    virtual std::array<float, 2> widget_size_00aa6740(void* widget) = 0;
    virtual void scroller_attach_content_00683790(void* scroller, void* content) = 0;
    // 00683380, ECX = scroller, (thumb, span, up, down).
    virtual void scroller_attach_scroll_bar_00683380(void* scroller, void* thumb, float span, void* up,
        void* down) = 0;
};

// 00564220 BSP_LeaderboardsScreen_Register, slot +14h of 00CEE9CC, __fastcall(ECX = screen), RET.
// 004F71D0; page "FE_leaderboards" (00CEEB10) -> +30h (00AA5840(name, 1, 0)); 005638F0; then the
// lookups (00AA7E00, parent -> store): +30h "clipbox_Clipbox" -> +90h; +90h "filters_Listbox" ->
// +94h; +94h "listbox_Group" -> +98h (hidden); +30h "info_Group" -> I; I "info_Text" -> +9Ch;
// +30h "slider_Group" -> S; S "scroll_up_Icon" +0A0h, "scroll_down_Icon" +0A4h, "slider_FrameBox"
// +0ACh, "back_Icon" +0A8h. The up/down/thumb get listener +0Ch; scroller +34h: content +94h, bar
// (thumb +0ACh, span = (float)(h(+0A8h) - h(+0ACh)), up +0A0h, down +0A4h). +30h "data_Group" -> D;
// D "filter_Text" -> +134h (listener +0Ch); D "clipbox_2_Clipbox" -> +118h; +118h "data_Listbox" ->
// +11Ch; +11Ch "listbox_Group" -> +120h (hidden); D "data_misc_Group" -> M; M "value_Text" +138h,
// "rank_Text" +13Ch; D "slider2_Group" -> S2; S2 "scroll_up2_Icon" +124h, "scroll_down2_Icon"
// +128h, "slider2_FrameBox" +130h, "back_Icon" +12Ch; listener +0Ch on the three; scroller +0BCh:
// content +11Ch, bar (+130h, h(+12Ch) - h(+130h), +124h, +128h). Finally +117h = 0, +20h = 0.
// The span subtraction runs on x87 with the first height spilled as a double; float - float is
// exact in double, so (float)((double)a - b) is used.
void leaderboards_register_00564220(Raw screen, Fe8LeaderboardRegisterHost& host);

struct Fe8LeaderboardInputHost : Fe8LeaderboardHost {
    virtual void scroller_begin_thumb_drag_00683a40(void* scroller) = 0;
    virtual void list_box_select_row_00a9c740(void* list_box, void* row) = 0;
    virtual void leaderboards_sync_selection_00561ad0(Raw screen, std::uint8_t boards) = 0;
    virtual void leaderboards_return_to_menu_005629a0(Raw screen) = 0;
    virtual void leaderboards_cycle_filter_005650c0(Raw screen) = 0;
    // 004BCD20: the checked operator[] of the result-row vector at screen +10h (60h rows; XUID at
    // +58h/+5Ch).
    virtual Raw result_row_004bcd20(Raw screen, std::int32_t index) = 0;
    virtual std::uint32_t xenon_active_user_slot_00a3eac0() = 0;                             // ECX = [00F8ABE8]
    virtual void xshow_gamer_card_ui_00a4d440(std::uint32_t user, std::uint64_t xuid) = 0;   // stdcall
};

// 00565190 BSP_LeaderboardsWidgetListener_Current04 (activate), __thiscall(listener = screen+0Ch,
// widget), RET 4. Offsets below are screen offsets. +158h = 1. Type 2 (a list row): an entries row
// (+0D8h != 0) first syncs to the entries (00561AD0(0)) while the boards list is active, then is
// selected (00A9C740) in the entries list; a board row, while the entries list is active, +154h
// is clear and the boards list inactive, moves focus back (+148h = entries selection, entries
// cleared, boards vtable +60h(1) and selected at +0B8h, +8Dh = 1, +115h = 0) and is selected in the
// boards list. Type 3: vtable +4Ch(1.0f); the filter text (+134h) and any icon whose label
// (+0F0h, "" 00E19474 when null) starts with A7h cycle the filter (005650C0) unless +154h is set;
// A5h shows the gamer card of the selected entry (while the entries list is active and has a
// selection: XShowGamerCardUI(00A3EAC0(), row +58h)); A3h returns to the menu (005629A0). Other
// types: boards widgets — +0A4h auto-scrolls +34h with (2, 0.004f, 1) and, when +154h is clear and
// the boards list inactive, moves focus back as above; +0A0h (1, 0.004f, 1) then 00561AD0(1); the
// thumb +0ACh begins a drag (00683A40) then 00561AD0(1). Entries widgets — +128h: mode != 0 ->
// auto-scroll +0BCh (2, 0.004f, 1); mode 0 with the entries list active and first + 17 below the
// total (+14Ch) -> first += 17, +154h = 1, +164h = 2, +168h = 1; otherwise +168h = 0. +124h: the
// mirror (1, ...; first - 17 > 0 -> first -= 17, +164h = 1). Thumb +130h: drag. Finally type 6 ->
// vtable +88h(2, 0, 1.0f), type 12h -> vtable +84h(2).
void leaderboards_listener_activate_00565190(Raw listener, void* widget, Fe8LeaderboardInputHost& host);

struct Fe8LeaderboardUpdateHost : Fe8LeaderboardInputHost {
    virtual bool front_end_action_edge_004d92b0(std::uint32_t action) = 0;                   // ECX = [00E188A8]
    virtual void scroller_update_00683820(void* scroller, std::uint32_t one) = 0;
    virtual std::int32_t leaderboards_selected_board_00561300(Raw screen) = 0;
    virtual void leaderboards_reset_rows_00564e70(Raw screen) = 0;
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    virtual bool xenon_duplicate_sign_in_128() = 0;                                          // [00F8ABE8]+128h != 0
    // 00531B00 on 00425D10() with (slot, message, kind, callback 0, 0, title "", 0.0f, 0, "", 1).
    virtual void raise_prompt_00531b00(std::int32_t slot, const char* message, std::int32_t kind) = 0;
    virtual bool main_menu_connection_gate_00585b40() = 0;
    virtual bool main_menu_account_gate_00585810(std::uint8_t cl) = 0;
    virtual std::int32_t game_complete_leaderboard_read_004dd7c0() = 0;                      // ECX = [00E188A8]
    virtual void game_cancel_leaderboard_read_004bc7f0() = 0;                                // ECX = [00E188A8]
    virtual Raw please_wait_screen_00e19698() = 0;
};

// 00565540 BSP_LeaderboardsScreen_Update, slot +20h, __thiscall(screen, float dt), RET 4.
// Action 4Bh (004D92B0 on [00E188A8]) or +160h -> 005629A0 and return. Both scrollers update
// (00683820(1)); a scrolling boards scroller (+7Ch byte or +54h) runs 00561860(+34h, boards, +90h,
// 0) and re-shows the board name (00561660) when +0B8h changed; the entries scroller (+104h /
// +0DCh) runs 00561860(+0BCh, entries, +118h, 0). Then the first of these actions that fires:
// 4Dh: boards active with entry rows -> 00561AD0(0); 4Ch: entries active -> 00561AD0(1);
// 50h: +154h clear -> 005650C0; 4Fh: entries active with a selection -> gamer card of that row;
// 46h: entries active, first - 17 > 0, mode 0 and selection 0 -> first -= 17, +154h = +164h = 1,
// +168h = 1, else +168h = 0; 47h: entries active, first + 17 < total, mode 0 and selection 10h ->
// first += 17, +154h = 1, +164h = 2, +168h = 1, else +168h = 0. A pending page (+154h): a non-zero
// timer +150h counts down by dt (float) and on going negative is zeroed with first = 1; with the
// timer zero and +154h == 1, unless +168h, a first page (+20h) equal to the selected board row's id
// (00561300, 00561510) clears +154h; otherwise 00564E70. +168h = 0. Command bar (0054B530 on
// [00E1930C]): (A7h "FE.toggle_filter" 0, B3h "globals.navigate" 1, A5h "globals.showgamercard" 1
// when the entries list is active with rows and a selection, else (0, "") 1, B2h
// "globals.scroll_menu" 1 when it also has more than 17 rows, else (0, ""), A3h "globals.back" 2).
// A duplicate sign-in ([00F8ABE8]+128h) raises "FE_pc.multi_liveerrorduplicate" (slot 2, kind 2)
// and sets +160h; failing 00585B40 or 00585810(CL 0) also sets +160h. Otherwise with a read
// running (+20h): result 80004005h or 8000FFFFh hides the please-wait screen (+4h = 0) and
// 80004005h raises "FE.error_while_reading_leaderboard", cancels (004BC7F0) and clears +20h; then
// +15Ch += dt (float) and, while the please-wait screen shows and it exceeds 30.0, raises
// "FE.leaderboards_cantaccess", hides the please-wait screen, cancels and clears +20h.
void leaderboards_update_00565540(Raw screen, float dt, Fe8LeaderboardUpdateHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-fe8: control binding screen (segment 17). Screen offsets: list listener +8h, widget
// listener +0Ch, +20h settings list box, +34h conflict text, +168h page, +16Ch + page*4 per-page
// selection (page 0's slot +16Ch is the input device index), +180h binding variant, +184h binding name (NativeString), +18Ch
// "binding unknown", +190h binding text widget, +1A0h scroller (embedded; its +48h byte is
// screen +1E8h and +20h is +1C0h), +218h clip, +21Ch / +220h first / last visible row, +224h row.
// ---------------------------------------------------------------------------
struct Fe8ControlBindingHost {
    virtual ~Fe8ControlBindingHost() = default;
    // 005547D0 input settings singleton; devices are 8-byte keys at +18h..+1Ch (checked index).
    virtual Raw input_settings_005547d0() = 0;
    // 0055C110, ECX = settings +8h: std::map<device key, entry>::operator[]; the entry is a tree
    // whose head (end) is at +4h.
    virtual void* input_device_map_index_0055c110(void* map, const void* key) = 0;
    // 00555940, ECX = entry tree: find(name) -> the node (end() = the head).
    virtual void* binding_tree_find_00555940(void* tree, std::string_view name) = 0;
    // 006AAC50, ECX = settings: (key, name, variant, NativeString* conflict) -> AL.
    virtual bool find_input_binding_conflict_006aac50(Raw settings, const void* key, std::string_view name,
        std::uint32_t variant, std::string& conflict) = 0;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;             // vtable +34h
    virtual void widget_set_enabled_60(void* widget, std::uint32_t enabled) = 0;             // vtable +60h
    virtual void gui_text_set_localised_source_00abaed0(void* text, std::string_view source, std::uint32_t flag) = 0;
    virtual std::int32_t list_box_selected_index_00a9c920(void* list_box) = 0;
    virtual void* list_box_current_row_00425e50(void* list_box) = 0;
    virtual void* list_box_row_00a9be00(void* list_box, std::int32_t index) = 0;
    virtual void list_box_select_index_silently_00a9c8f0(void* list_box, std::int32_t index) = 0;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;     // (name, 1)
    virtual std::array<float, 3> widget_resolved_position_00aa6750(void* widget) = 0;
    virtual std::array<float, 2> widget_size_00aa6740(void* widget) = 0;
    virtual void scroller_toward_start_00683680(void* scroller, float delta) = 0;
    virtual void scroller_toward_end_00683700(void* scroller, float delta) = 0;
    virtual void command_bar_set_help_line_0054a0c0(std::string_view line) = 0;              // ECX = [00E1930C]
    // A NativeString copy: 0041DD40 Resize(length, 1), then memcpy of the characters.
    virtual void native_string_copy_0041dd40(void* string, std::string_view text) = 0;
    // Screen routines reconstructed below, routed through the host so callers can substitute.
    virtual void control_binding_show_conflict_text_0055c200(Raw screen) = 0;
    virtual void control_binding_refresh_selected_binding_0055c3b0(Raw screen) = 0;
    virtual bool control_binding_scroll_to_selection_0055cec0(Raw screen, bool no_scroll) = 0;
};

// 0055C200 BSP_ControlBindingScreen_ShowConflictText, __fastcall(ECX = screen), RET. key = the
// settings' device key [+16Ch]; entry = map[key] (0055C110); a binding name (+184h) found in the
// entry (00555940) hides the conflict text (+34h vtable +34h(0)). Otherwise 006AAC50(key, +184h,
// +180h, &conflict) decides; the text's vtable +34h gets that byte (the image pushes a dword
// whose upper bytes are stale stack contents), and a conflict shows
// "FE_pc.controls_conflict|.: |" (00CEE63C) + conflict through 00ABAED0(text, 1).
void control_binding_show_conflict_text_0055c200(Raw screen, Fe8ControlBindingHost& host);

// 0055C3B0 BSP_ControlBindingScreen_RefreshSelectedBinding, __fastcall(ECX = screen), RET. With a
// selection in +20h: entry = map[key]; +184h = a copy of the current row's (00425E50) "main_Text"
// (00E086A0 -> 00CEE4CC) string (+0F4h); +18Ch = the name is not in the entry. Unknown: +180h = 1
// and +190h = the row's "main_Text"; known: +190h = "var_2_Text" when +180h else "var_1_Text"
// (00E086A8 / 00E086A4). Then 0055C200.
void control_binding_refresh_selected_binding_0055c3b0(Raw screen, Fe8ControlBindingHost& host);

// 0055CEC0 BSP_ControlBindingScreen_ScrollToSelection, __thiscall(screen, BYTE no_scroll), RET 4,
// AL. first (+21Ch) = the row count of +20h, last (+220h) = 0. Clip C = pos - size * anchor of
// +218h (both components rounded to float). Each row is visible when its rounded top y is below
// C.y and C.y + clip.h > row.y + row.h (both sums in x87 extended precision); a visible row
// lowers first or else raises last; every row gets vtable +60h(visible). While the scroller is
// idle (+1E8h byte and +1C0h clear) and no_scroll is false, a selection above first scrolls
// +1A0h toward the start (00683680) and one below last toward the end (00683700), by the current
// row's height, returning 1; otherwise 0. With the scroller moving or no_scroll set, a selection
// above first snaps forward from first past rows whose +77h byte is set, one below last snaps
// back from last the same way; the row is selected silently (00A9C8F0), 0055C3B0 runs, +224h =
// the row, and 1 is returned; 0 when nothing moved or the row index is -1.
bool control_binding_scroll_to_selection_0055cec0(Raw screen, bool no_scroll, Fe8ControlBindingHost& host);

// 0055D180 BSP_ControlBindingScreen_OnListboxSelectionChanged, __thiscall(listener = screen+8h,
// row, list box), RET 8 (screen offsets below; the body rebases with ADD ESI, -8 at 0055D2E3). A
// null row does nothing. +16Ch + page(+168h)*4 = the list's selection (00A9C920), so page 0 writes
// the device index. Page 0: the help line (0054A0C0 on [00E1930C]) is the row label (+0F4h) +
// "_help" (00CEE6BC; 004261A0 concatenation); page 1: "FE_pc.cs_loadcontrollerpreset_help"
// (00CEE698); page 2 with the settings list +20h: 0055C3B0, list vtable +60h(1), then 0055CEC0(0)
// until it returns 0.
void control_binding_on_selection_changed_0055d180(Raw listener, void* row, void* list_box,
    Fe8ControlBindingHost& host);

struct Fe8ControlBindingEnterHost {
    virtual ~Fe8ControlBindingEnterHost() = default;
    virtual bool game_in_mission_61f() = 0;                                                  // [00E188A8]+61Fh
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;     // (name, 1)
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;             // vtable +34h
    virtual void list_box_set_listener_00a9ac40(void* list_box, void* listener) = 0;
    virtual void control_binding_refresh_page_0055e3a0(Raw screen, std::int32_t page) = 0;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
};

// 0055FA80 BSP_ControlBindingScreen_Enter, slot +18h of 00CEE524, __fastcall(ECX = screen), RET.
// In a mission ([00E188A8]+61Fh) +1Ch = "Ingame_Main_Listbox" (00CEE830) and "Main_Listbox"
// (00CEB334) is hidden; otherwise the reverse (both under page +14h). +24h = the list's
// "MainListbox_Text" (00CEC040); listener +8h; the list shown; 0055E3A0(0); +10h = 0.25f
// (00CE3868); +0E4h = 0; +38h = 1; the string +3Ch is erased from the front by min(length,
// 7FFFFFFFh) (memmove of the remainder, then 0041DD40(length - n, 1)), which empties it.
void control_binding_enter_0055fa80(Raw screen, Fe8ControlBindingEnterHost& host);

struct Fe8ControlBindingConfirmHost {
    virtual ~Fe8ControlBindingConfirmHost() = default;
    // 00450540 on 00425D10() +268h: the prompt's button labels.
    virtual void prompt_append_label_00450540(std::string_view label) = 0;
    virtual Raw prompt_screen_00425d10() = 0;
    // 00531B00 on 00425D10(): (slot, message, kind, callback, 0, title "", 0.0f, 0, "", 1).
    virtual void raise_prompt_00531b00(std::int32_t slot, const char* message, std::int32_t kind,
        std::uint32_t callback) = 0;
    virtual void input_settings_save_006aa2d0() = 0;                                         // ECX = 005547D0()
    virtual void control_binding_refresh_page_0055e3a0(Raw screen, std::int32_t page) = 0;
};

// 00560210 BSP_ControlBindingScreen_ConfirmOrNextDevice, __fastcall(ECX = screen), RET. With
// +19Ch set: labels "FE.opt_cancel", "FE.opt_discard", "globals.accept" appended to the prompt
// (+268h), prompt +284h = 0, then 00531B00(slot 0, "FE.opt_acceptchanges", kind 4, callback
// 0055FA10). Otherwise 006AA2D0 on the input settings, +16Ch += 1 and 0055E3A0(0).
void control_binding_confirm_or_next_device_00560210(Raw screen, Fe8ControlBindingConfirmHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-fe8: tactical library category lists (segment 12). Each builder shows the list
// box +0B8h (vtable +34h(1)), clones rows from +0ACh, labels each row's "MenuItem_Text"
// (00CEB474) child through 00ABB000(text, -1.0f, 1), names the row with its index (004260B0 then
// row vtable +2Ch(&string)), clears row +77h, appends it (00A9D750(row, 0, 0)) and finishes with
// 00A9CF30(1, 1). Labels are copied through a 200-byte buffer with strcpy_s / sprintf("%s").
// ---------------------------------------------------------------------------
struct Fe8LibraryListHost {
    virtual ~Fe8LibraryListHost() = default;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;             // vtable +34h
    virtual void widget_set_name_2c(void* widget, std::string_view name) = 0;                // vtable +2Ch
    virtual void* widget_clone_subtree_00aab4c0(void* source, std::uint32_t zero) = 0;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;     // (name, 1)
    virtual void gui_text_set_ellipsized_00abb000(void* text, std::string_view source, float width,
        std::uint32_t flag) = 0;
    virtual void list_box_insert_row_00a9d750(void* list_box, void* row, std::uint32_t a, std::uint32_t b) = 0;
    virtual void list_box_reset_highlight_00a9cf30(void* list_box, std::uint32_t a, std::uint32_t b) = 0;
    // 004FDA50 GetNameOrNoUnlock, slot(index, NativeString* out), RET 8.
    virtual std::string vehicle_class_name_or_no_unlock_004fda50(std::int32_t index) = 0;
    // 004FAE70, __thiscall(screen, id), RET 4, AL.
    virtual bool library_has_academy_entry_004fae70(Raw screen, std::int32_t id) = 0;
    virtual const char* image_string_table(std::uint32_t table, std::int32_t index) = 0;     // [table + index*4]
};

// 005001A0 BSP_TacticalLibraryScreen_BuildUnlocksList, __fastcall(ECX = screen), RET. One row (index
// 0). With the first 2Ch record of +46Ch (checked non-empty) holding more ids (+1Ch..+20h) than
// +458h and ids[+458h] == +454h: +138h = +458h and the label is 004FDA50(record 0 ids[+458h]).
// Otherwise +138h = 0, +454h = -1, +458h = 0 and the label is "FE.unitlib_nounlock" (00CEB184).
void library_build_unlocks_list_005001a0(Raw screen, Fe8LibraryListHost& host);
// 00500450 BSP_TacticalLibraryScreen_BuildMovieList: one row, label [00E08494] ("FE.main_movies").
void library_build_movie_list_00500450(Raw screen, Fe8LibraryListHost& host);
// 00500640 BuildNoseArtList: rows i = 0, 1 labelled [00E084F0 + i*4], each built when +98h >= 5Ah
// or +98h == i.
void library_build_nose_art_list_00500640(Raw screen, Fe8LibraryListHost& host);
// 00500860 BuildUnitsWeaponsList: five rows, labels [+31Ch + i*8]; the text is shown before it is
// labelled.
void library_build_units_weapons_list_00500860(Raw screen, Fe8LibraryListHost& host);
// 00500E90 BuildPowerUpList: three rows, labels [+344h + i*8].
void library_build_power_up_list_00500e90(Raw screen, Fe8LibraryListHost& host);
// 005010C0 BuildAcademyList: thirteen candidates, labels [+35Ch + i*8]; i is skipped when
// 004FAE70(screen, i) is true.
void library_build_academy_list_005010c0(Raw screen, Fe8LibraryListHost& host);
// 005012F0 BuildAchievementsList: six rows labelled [00E084D8 + i*4].
void library_build_achievements_list_005012f0(Raw screen, Fe8LibraryListHost& host);

struct Fe8LibrarySmallHost {
    virtual ~Fe8LibrarySmallHost() = default;
    // 00437F50: the vehicle class registry; two 800h-dword permutation tables at +10h and +2010h.
    virtual Raw vehicle_class_registry_00437f50() = 0;
    virtual void library_reset_order_state_004f8fa0(Raw screen) = 0;                         // tail call
    virtual void library_refresh_unlock_rows_00503250(Raw screen, std::int32_t class_id) = 0;
    virtual void* list_box_current_item_00a9ca00(void* list_box) = 0;
    virtual Raw widget_name_28(void* widget) = 0;                                            // vtable +28h -> NativeString
    virtual std::int32_t atol_00bf8417(const char* text) = 0;
    // The function-static colour at 00E18D64 and its guard dword 00E18D74.
    virtual float* static_grey_00e18d64() = 0;
    virtual std::uint32_t& static_grey_guard_00e18d74() = 0;
    virtual std::int32_t list_box_selected_index_00a9c920(void* list_box) = 0;
    virtual void widget_apply_uv_8c(void* widget) = 0;                                       // vtable +8Ch
};

// 004F9D80 BSP_Color_StaticGrey, EAX = 00E18D64: the first call (guard bit 0) stores {0.25, 0.25,
// 0.25 (00CE3868), 1.0 (00D7A24C)}.
const float* library_colour_static_grey_004f9d80(Fe8LibrarySmallHost& host);
// 00506510 BSP_TacticalLibraryScreen_ResetVehicleClassOrder, __fastcall(ECX = screen): +134h = 0;
// both registry tables get the identity permutation (0..7FFh); tail 004F8FA0(screen).
void library_reset_vehicle_class_order_00506510(Raw screen, Fe8LibrarySmallHost& host);
// 00506550 BSP_TacticalLibraryScreen_ShowCurrentEntryDetail, __fastcall(ECX = screen): +4B0h =
// the class id (+48h) of entry [+434h] of the 6Ch-entry vector +448h; the current row of +0B8h
// (00A9CA00) names a record index (atol of vtable +28h's string, "" 00E18D78 when null); rec = the
// first dword of that 2Ch record (+460h). When +4B0h differs from +134h: 00503250(+4B0h), +134h =
// +4B0h, both tables reset to the identity, then table1[rec] = +4B0h and table2[+4B0h] = rec.
void library_show_current_entry_detail_00506550(Raw screen, Fe8LibrarySmallHost& host);
// 005661C0 (named GetSelectedSessionIndex), __fastcall(ECX = list box), EAX: the list's own
// selection (00A9C920 with the same ECX) plus +164h when the +148h byte is set.
std::int32_t session_list_absolute_selection_005661c0(void* list_box, Fe8LibrarySmallHost& host);
// 00568290 BSP_GuiIcon_SetUvRect, __thiscall(icon, const float uv[4]), RET 4: +118h..+124h = uv,
// then vtable +8Ch().
void gui_icon_set_uv_rect_00568290(Raw icon, const float uv[4], Fe8LibrarySmallHost& host);

struct Fe8LibraryPageHost : Fe8LibrarySmallHost {
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    // 00518D60, ECX = -1, EDX = 0, stack (0, title NativeString), RET 8.
    virtual void frame_select_00518d60(std::uint32_t ecx, std::uint32_t edx, std::uint32_t zero,
        std::string_view title) = 0;
};

// 004FFD00 BSP_TacticalLibraryScreen_ShowUnlocksPage, __fastcall(ECX = screen), RET. Only for view
// kind +94h == 6 with a current item in +0B8h (00A9CA00). The item name is passed to atol and the
// result discarded. Command bar (A2h "globals.select" 0, A7h "globals.3dview" ("globals.2dview"
// when +90h == 1) 1, B2h "globals.scroll_menu" 1, AFh "globals.change" 1, A3h "globals.back" 2);
// then 00518D60(ECX -1, EDX 0; 0, "FE.taclib_unlocks_title").
void library_show_unlocks_page_004ffd00(Raw screen, Fe8LibraryPageHost& host);
// 004FFF20 BSP_TacticalLibraryScreen_ShowOrderOfBattlePage, __fastcall(ECX = screen), RET. Only
// for view kind 5 with a current item: rec = the 2Ch record (+45Ch vector, 004F9360) at atol(item
// name). Command bar (A2h "globals.continue" 0, A7h 3dview/2dview 1, B3h "globals.navigate" 1,
// (AFh "globals.change" when rec has ids (+1Ch..+20h) and its +0Ch byte is set, else (0, "")) 1,
// A3h "globals.back" 2); then 00518D60(-1, 0; 0, "FE.taclib_orderofbattle_title").
void library_show_order_of_battle_page_004fff20(Raw screen, Fe8LibraryPageHost& host);

struct Fe8LibraryMapHost {
    virtual ~Fe8LibraryMapHost() = default;
    virtual Raw game() = 0;                                                                  // [00E188A8]
    // The case-insensitive std::map<NativeString, int> at game +6F0h (head at +6F4h): 004BF370
    // lower_bound and 005070C0 operator[], both ECX = the map; nodes keep the key at +0Ch.
    virtual void* string_int_map_lower_bound_004bf370(void* map, const void* key) = 0;
    virtual std::int32_t& string_int_map_index_005070c0(void* map, const void* key) = 0;
    virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;
};

// 00509DE0 BSP_TacticalLibraryScreen_BuildMapEntries, __fastcall(ECX = screen), RET. For each
// 6Ch entry of +3D8h..+3DCh, its +28h name is looked up in the game's map (+6F0h): found
// (lower_bound plus the !(key < node) test, empty names ordering first) with a positive value
// clears the entry's +45h "locked" byte; otherwise it is set to 1.
void library_build_map_entries_00509de0(Raw screen, Fe8LibraryMapHost& host);

struct Fe8LibraryFilterHost {
    virtual ~Fe8LibraryFilterHost() = default;
    virtual void entry_vector_clear_00505de0(void* vector) = 0;                              // erase(begin, end)
    virtual std::uint32_t entry_vector_size_004f92e0(void* vector) = 0;
    virtual Raw entry_vector_at_005056c0(void* vector, std::uint32_t index) = 0;
    virtual void entry_vector_push_back_0050ad00(void* vector, Raw entry) = 0;
    virtual void gui_movie_stop_00aac870(void* movie) = 0;
    virtual void library_view5_reset_0050b730(Raw screen) = 0;
    virtual void library_view6_reset_0050b440(Raw screen) = 0;
};

// 0050D900 (named ClearList; it filters a list), __thiscall(screen, source vector, out vector),
// RET 8. out is emptied (00505DE0 over [begin, end), both bounds checked), then gets every source
// entry (005056C0) whose +40h equals +3C0h or exceeds 59h and whose +44h byte is set (0050AD00).
// Then by view kind +94h (jump table 0050DA10): 4 stops the movies +51Ch, +520h, +524h (non-null,
// 00AAC870) and sets +558h = +55Ch = -1; 5 -> 0050B730(screen); 6 -> 0050B440(screen).
void library_filter_entries_0050d900(Raw screen, void* source, void* out, Fe8LibraryFilterHost& host);

struct Fe8FrameRegisterHost {
    virtual ~Fe8FrameRegisterHost() = default;
    virtual void front_end_screen_register_004f71d0(Raw screen) = 0;
    virtual void* gui_load_page_00aa5840(std::string_view name, std::uint32_t one, std::uint32_t zero) = 0;  // ECX = 004C12B0()
    virtual const char* image_string_table(std::uint32_t table, std::int32_t index) = 0;     // [table + index*4]
};

// 00517ED0 BSP_FrontEndFrame_Register, slot +14h, __fastcall(ECX = screen), RET. 004F71D0; frame
// index +160h = 4. Three page tables, each slot loaded (00AA5840(name, 1, 0)) only in the row of
// the frame index and when the name is non-null, otherwise 0: +0Ch + (row*13 + j)*4 for j < 13
// from [00E08500 + (index*13 + j)*4]; +110h + (row*2 + j)*4 for j < 2 from [00E08604 + (index*2 +
// j)*4] ("FE_frame..."); +138h likewise from [00E0862C ...] ("FE_frame_title..."). Rows run 0..4.
void front_end_frame_register_00517ed0(Raw screen, Fe8FrameRegisterHost& host);

struct Fe8LibraryEventHost {
    virtual ~Fe8LibraryEventHost() = default;
    virtual std::int32_t widget_type_5c(void* widget) = 0;                                   // vtable +5Ch
    virtual std::int16_t widget_state_94(void* widget) = 0;                                  // vtable +94h
    virtual void widget_set_alpha_4c(void* widget, float alpha) = 0;                         // vtable +4Ch
    virtual void widget_set_highlight_88(void* widget, std::uint32_t state, std::uint32_t zero, float one) = 0;
    virtual void widget_set_highlight_84(void* widget, std::uint32_t state) = 0;
    virtual void* list_box_row_00a9be00(void* list_box, std::int32_t index) = 0;
    virtual void list_box_select_index_00a9c7c0(void* list_box, std::int32_t index) = 0;
    virtual void scroller_set_auto_scroll_00683300(void* scroller, std::uint32_t a, float speed, std::uint32_t b) = 0;
    virtual void scroller_begin_thumb_drag_00683a40(void* scroller) = 0;
    // Library screen routines (ECX = screen).
    virtual void library_play_entry_movie_004ff580(Raw screen) = 0;
    virtual void front_end_show_difficulty_select_004ff6f0(Raw screen) = 0;
    virtual bool library_is_entry_unlocked_004fc9d0(Raw screen) = 0;
    virtual void library_back_00515e00(Raw screen) = 0;
    virtual void library_apply_nose_art_005161c0(Raw screen) = 0;
    virtual void library_switch_view_00514c20(Raw screen) = 0;
    virtual void library_open_order_of_battle_view_00514fb0(Raw screen) = 0;
    virtual std::int32_t library_find_row_by_id_004fcb30(Raw screen, std::int32_t id) = 0;
    virtual void library_step_carousel_00516010(Raw screen, bool forward) = 0;
};

// 00517300 BSP_TacticalLibraryScreen_HandleGuiEvent, slot +4h of the listener at screen+68h,
// __thiscall(listener, widget), RET 4 (screen offsets below). Type 3: a click on rows 0..2 of the
// list +0C4h while the timer +5B4h is exactly -1.0f selects that row and sets the timer to 0.5f;
// otherwise vtable +4Ch(1.0f) and the label's first byte (+0F0h, "" 00E18D7C) decides: A2h by view
// kind +94h (jump table 00517600): 3 -> 004FF580, 5 -> 004FF6F0, 6 -> 00515E00 when 004FC9D0, 7 ->
// the timer rule above, 8 -> 005161C0; A7h with kind 2, 5 or 6 -> 00514C20; A3h -> 00514FB0 for
// kind 7, else 00515E00. Other types: +5BCh / +5B8h auto-scroll +0CCh (2 / 1, 0.004f, 1), +5C0h
// begins a drag; the carousel arrows +574h / +578h step the carousel (00516010(widget == +574h))
// unless their state (+94h) is 3 (then return); a widget of the pair tables +1D8h / +278h (40
// each) selects, for kind 5, the row 004FCB30(i) in +0B8h, and with type 6 and state below 3 steps
// the carousel (forward when it is the +1D8h entry), returning otherwise. Then type 6 ->
// vtable +88h(2, 0, 1.0f), type 12h -> vtable +84h(2). The StepCarousel byte is pushed as a dword
// whose upper bytes are stale register contents.
void library_handle_gui_event_00517300(Raw listener, void* widget, Fe8LibraryEventHost& host);

struct Fe8InGame23Host {
    virtual ~Fe8InGame23Host() = default;
    virtual void unit_send_role_transfer_0077c470(void* unit, std::uint32_t role, std::uint32_t zero) = 0;  // ECX = unit
    // Observer pairs: ECX = observed, EDX = observer member (screen +8h).
    virtual bool observer_is_pair_registered_00694af0(void* observed, void* observer) = 0;
    virtual void observer_unregister_pair_006952a0(void* observed, void* observer) = 0;
    virtual void observer_register_pair_00694a60(void* observed, void* observer) = 0;
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void camera_mover_construct_base_00432750(void* camera) = 0;
    virtual Raw game() = 0;                                                                  // [00E188A8]
    // 00923870, ECX = camera, (0, game +19CCh, const float matrix[16]).
    virtual void camera_set_transform_00923870(void* camera, std::uint32_t zero, std::uint32_t scene,
        const float* matrix) = 0;
    virtual void airfield_camera_setup_0042cb30(void* camera, void* unit_part) = 0;          // ECX = camera
    virtual void game_set_camera_mover_004bc410(void* camera) = 0;                           // ECX = [00E188A8]
};

// 005178F0 BSP_InGameScreen23_BindUnit, __thiscall(screen, unit), RET 4. An old unit (+1Ch) gets
// 0077C470(1FEh, 0) and loses the observer pair (screen +8h) when registered; +1Ch = unit, which
// registers the pair when missing. Without a camera (+20h): new(388h), zeroed, 00432750, vtables
// 00CEC1F8 / +10h 00CEC1DC / +24h 00CEC1D4 / +170h 00CEC1BC (a failed allocation leaves 0); +20h =
// it; 00923870(ECX camera; 0, [game+19CCh], identity matrix) and the pair (camera, screen +8h)
// registered. Then 0042CB30(camera; unit ? unit+72Ch : 0), 004BC410(camera), +24h = 0.5f.
void in_game_screen23_bind_unit_005178f0(Raw screen, void* unit, Fe8InGame23Host& host);

struct Fe8BomberCameraHost : Fe8InGame23Host {
    virtual void ship_camera_construct_base_00519290(void* camera) = 0;
    virtual void camera_mover_set_target_0042f650(void* camera, void* target) = 0;           // ECX = camera
    virtual void* binoculars_screen_00e198c4_4c() = 0;                                       // [[00E198C4]+4Ch]
    virtual void binoculars_bind_unit_0051e730(void* binoculars, void* camera) = 0;
    virtual void binoculars_reset_0051e8e0(void* binoculars) = 0;
};

// 005198F0 BSP_BomberView_EnsureCamera, __fastcall(ECX = screen), RET. A camera (+28h) whose +60h
// or +5Dh byte is set is dropped (+28h = 0, not freed). Without one: new(3F4h), zeroed, 00519290,
// vtables 00CEC5F0 / +10h 00CEC5D8 / +24h 00CEC5D0 / +170h 00CEC5B8; 00923870(camera; 0,
// [game+19CCh], identity); camera +3ECh = -1.553343f (00CEC574), +3F0h = 1.553343f (00CEC400); pair
// (camera, screen +8h) registered. When the game's mover (+1ED4h) is not the camera or the target
// +1Ch differs from +20h: 0042F650(camera; +1Ch); a camera with +3E8h clear gets +384h = +388h = 0
// and +38Ch = +398h = +394h = +390h = 30.0f (00CE38C8); 004BC410(camera); +20h = +1Ch. Always:
// 0051E730(binoculars; camera) and 0051E8E0(binoculars) on [[00E198C4]+4Ch].
void bomber_view_ensure_camera_005198f0(Raw screen, Fe8BomberCameraHost& host);

struct Fe8EntryAssignHost {
    virtual ~Fe8EntryAssignHost() = default;
    virtual long interlocked_increment_00ce221c(long* value) = 0;                            // import
    virtual long interlocked_decrement_00ce2220(long* value) = 0;                            // import
    virtual void object_virtual_0(void* object) = 0;                                         // ECX = object, vtable +0
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) = 0;
};

// 004FE310 BSP_TacticalLibraryEntry_Assign, __thiscall(to, const from), RET 4, EAX = to. A 6Ch
// library entry: intrusive references at +0h and +14h (count at +4h of the target; the old target
// is released through its vtable +0 when the count reaches 0, the new one is retained first);
// dwords +4h..+10h, +18h..+24h, +40h, +48h; bytes +44h, +45h, +46h (+47h is not copied); strings
// +28h, +30h, +38h, +4Ch, +54h, +5Ch, +64h (each, unless aliased: 0041DD40(length, 1), then the
// characters copied when the source is non-empty).
void* library_entry_assign_004fe310(Raw to, Raw from, Fe8EntryAssignHost& host);

struct Fe8UnlockRowsHost : Fe8LibraryListHost {
    virtual void* list_box_row_00a9be00(void* list_box, std::int32_t index) = 0;
    virtual void* list_box_current_item_00a9ca00(void* list_box) = 0;
    virtual Raw widget_name_28(void* widget) = 0;                                            // vtable +28h
    virtual std::int32_t atol_00bf8417(const char* text) = 0;
    virtual void* library_find_entry_by_id_004fb090(Raw screen, std::int32_t id) = 0;       // ECX = screen
};

// 00503250 BSP_TacticalLibraryScreen_RefreshUnlockRows, __thiscall(screen, class id), RET 4. For each
// row of +0B8h: index = atol(row name); -1 and 99999 skip the row. rec = the 2Ch record at index of
// +460h (kind +94h == 5) or +470h (kind 6); other kinds keep the previous row's rec (the local is
// not reset, and is uninitialised for the first row). id = the class id for the current item
// (00A9CA00), else rec's first dword. 004FB090(id) runs with its result unused. Label: kind 6 with
// id -1 -> "FE.unitlib_nounlock", else 004FDA50(id); kind 5 with id != rec's id prefixes "|.* |"
// (00CEB580, 0043C130). The row's "MenuItem_Text" shows it through 00ABB000(-1.0f, 1).
void library_refresh_unlock_rows_00503250(Raw screen, std::int32_t class_id, Fe8UnlockRowsHost& host);

struct Fe8LibraryLookupHost : Fe8EntryAssignHost {
    virtual Raw game() = 0;                                                                  // [00E188A8]
};

// 004FD500 (named CountAvailableMapsForSide; it is a lookup), __thiscall(library, ref* out_object,
// dword out_data[4]), RET 8, AL. wanted = game +734h when the active slot record ([game + 18CCh +
// [game+18ECh]*4]) has +28h == 0, else game +730h. The first 6Ch entry of +418h..+41Ch whose +48h
// equals wanted is copied out: its +0h reference into *out_object (retain new, release old as in
// 004FE310) and its +4h..+10h into out_data; AL = 1. No match: AL = 0 (the upper EAX bytes hold
// the entry count).
bool library_find_entry_for_side_004fd500(Raw library, Raw out_object, std::uint32_t out_data[4],
    Fe8LibraryLookupHost& host);

struct Fe8OrderOfBattleSideHost {
    virtual ~Fe8OrderOfBattleSideHost() = default;
    virtual void library_show_order_of_battle_page_004fff20(Raw screen) = 0;
    virtual bool native_string_equals_insensitive_00425850(const void* string, const char* text) = 0;  // ECX = string
    virtual void command_bar_set_help_line_0054a0c0(std::string_view line) = 0;              // ECX = [00E1930C]
    virtual void library_filter_entries_0050d900(Raw screen, void* source, void* out) = 0;
    virtual void library_refresh_entry_detail_00510c20(Raw screen) = 0;
    virtual void scroller_attach_content_00683790(void* scroller, void* content) = 0;
    virtual long double text_normalized_height_00ab6bd0(void* text) = 0;                     // x87 return
    virtual std::array<float, 2> widget_size_00aa6740(void* widget) = 0;
    virtual void scroller_set_scroll_range_006834a0(void* scroller, float range) = 0;
};

// 00511BF0 BSP_TacticalLibraryScreen_SelectOrderOfBattleSide, __thiscall(screen, side), RET 4.
// +434h = +138h[side]; +438h = (float)(+434h + 0.003) (FILD, FADD qword 00CEBE08); +558h = +55Ch =
// -1. Kind 5: +134h = record[side] (+460h, 2Ch) ids[+434h - 1] (00418140) when that record has ids
// and +434h != 0, else the record's first dword; 004FFF20; the help line is "FE.oob_changeable_help"
// / "FE.oob_fixed_help" when the record's +10h string is "change" / "fixed" (case-insensitive,
// 00425850), else that string itself. extra = (float)(1/72) for kind 4, else 0. A new side (!=
// +3C0h) for kinds 0..6 and 8 stores +3C0h and filters (0050D900) into +444h from +3D4h, +3E4h,
// +3F4h, +3C4h, +404h, +3F4h, +3F4h, +414h (jump table 00511F10). Then 00510C20, scroller +0CCh
// content 0, and range = (float)((double)(text height(+0A8h) + extra) - h(+5ACh)).
void library_select_order_of_battle_side_00511bf0(Raw screen, std::uint32_t side, Fe8OrderOfBattleSideHost& host);

struct Fe8ControlBindWidgetsHost {
    virtual ~Fe8ControlBindWidgetsHost() = default;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;     // (name, 1)
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;             // vtable +34h
    virtual void list_box_set_listener_00a9ac40(void* list_box, void* listener) = 0;
    virtual void list_box_set_field124_00a9ceb0(void* list_box, float value, std::uint32_t zero) = 0;
    virtual void widget_set_listener_fields_00aa6bc0(void* widget, void* listener, std::uint32_t zero) = 0;
    virtual void scroller_attach_content_00683790(void* scroller, void* content) = 0;
    virtual std::array<float, 2> widget_size_00aa6740(void* widget) = 0;
    virtual void scroller_attach_scroll_bar_00683380(void* scroller, void* thumb, float span, void* up,
        void* down) = 0;
    virtual void gui_text_set_localised_cstring_00abbe50(void* text, const char* key, std::uint32_t flag) = 0;
    // 005F0390, ECX = parent, EDX = listener (options_find_auto_control_group_005f0390).
    virtual void options_find_auto_control_group_005f0390(void* parent, void* listener) = 0;
    virtual const char* image_string_table(std::uint32_t table, std::int32_t index) = 0;     // [table + index*4]
};

// 00555DC0 BSP_ControlBindingScreen_BindWidgets, slot +14h of 00CEE524 (after 00555C60),
// __fastcall(ECX = screen), RET. Lookups (parent -> store): +14h "Main_Listbox" -> +1Ch (listener
// +8h); +1Ch "MainListbox_Text" -> +24h; +18h "column_FrameBox" -> +210h and "listbox_FrameBox" ->
// +214h (both hidden); +18h "clipbox_Clipbox" -> +218h; +218h "Settings_Listbox" -> +20h (listener
// +8h; +12Ch = 0, +130h = 1/360, +134h = 0, 00A9CEB0(0.75f, 0), +128h = 1/360); +18h "slider_Group"
// -> +1FCh with "scroll_up_Icon" +200h, "scroll_down_Icon" +204h, "slider_FrameBox" +20Ch,
// "back_Icon" +208h; listener +0Ch on +200h, +204h, +20Ch; scroller +1A0h: content +20h, bar
// (+20Ch, (float)(h(+208h) - h(+20Ch)), +200h, +204h). +20h "setting_Group" -> +28h (hidden); its
// "main_Text", "var_1_Text", "var_2_Text" ([00E086A0..A8]) get 00ABBE50("", 0); "conflict_1_Group"
// and "conflict_2_Group" ([00E086AC], [00E086B0]) are hidden. +20h "slider_Group" -> +2Ch (hidden),
// its arrows (listener +0Ch) and 005F0390(+2Ch, +0Ch); +20h "toggle_text_Group" -> +30h (hidden),
// its arrows (listener +0Ch). +14h "SelectedConflict_Text" -> +34h.
void control_binding_bind_widgets_00555dc0(Raw screen, Fe8ControlBindWidgetsHost& host);

struct Fe8ControlEventHost {
    virtual ~Fe8ControlEventHost() = default;
    virtual std::int32_t widget_type_5c(void* widget) = 0;                                   // vtable +5Ch
    virtual std::int16_t widget_child_count_98(void* widget) = 0;                            // vtable +98h
    virtual void* widget_virtual_28(void* widget, const char* name, std::uint32_t zero) = 0; // vtable +28h
    virtual bool native_string_has_prefix_at_00553c80(void* result) = 0;                     // ECX = result
    virtual void widget_set_alpha_4c(void* widget, float alpha) = 0;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;
    virtual void widget_set_highlight_88(void* widget, std::uint32_t state, std::uint32_t zero, float one) = 0;
    virtual void widget_set_highlight_84(void* widget, std::uint32_t state) = 0;
    virtual float gui_manager_field_5c() = 0;                                                // 004C12B0() +5Ch
    virtual void list_box_select_index_00a9c7c0(void* list_box, std::int32_t index) = 0;
    virtual void* list_box_current_row_00425e50(void* list_box) = 0;
    virtual void list_listener_activate_04(void* listener, void* row, void* list_box) = 0;   // [screen+8] vtable +4
    virtual void scroller_set_auto_scroll_00683300(void* scroller, std::uint32_t a, float speed, std::uint32_t b) = 0;
    virtual void scroller_begin_thumb_drag_00683a40(void* scroller) = 0;
    virtual void set_options_page_00e19650(std::int32_t page) = 0;
    virtual void front_end_push_interface_request_004cc460(std::uint32_t a, std::uint32_t b) = 0;  // ECX = [00E198B8]
    virtual bool input_config_load_suppressed_00f88a30() = 0;                                // byte [00F88A30]
    virtual void input_configuration_load_00698a10() = 0;                                    // ECX = game +3Ch
    // Control binding routines (ECX = screen).
    virtual void control_binding_refresh_selected_binding_0055c3b0(Raw screen) = 0;
    virtual void control_binding_adjust_setting_0055ccc0(Raw screen, std::int32_t delta) = 0;
    virtual void control_binding_refresh_page_0055e3a0(Raw screen, std::int32_t page) = 0;
    virtual void control_binding_confirm_or_next_device_00560210(Raw screen) = 0;
    virtual void control_binding_position_marker_00553b00(Raw screen, void* widget) = 0;
};

// 00560BF0 BSP_ControlBindingScreen_HandleGuiEvent, slot +4h of the widget listener at screen+0Ch,
// __thiscall(listener, widget), RET 4 (screen offsets). Page +168h == 3 or widget type 10h: nothing.
// Type 6 outside the slider group (+1FCh) and type 2 select their row id ([widget+70h]+0D8h /
// widget+0D8h; zero does nothing): +180h = (004C12B0()+5Ch > 0.625f (00CEE8DC)), 0055C3B0, settings
// list +20h selects id & 0FFFFh; a type-6 widget with more than one child (vtable +98h) then
// adjusts the setting by -1 / +1 (0055CCC0) when vtable +28h("arrow_left_Icon" / "arrow_right_Icon",
// 0) passes 00553C80. Type 3: vtable +4Ch(1.0f); label (+0F0h, "" 00E1942C) A7h does nothing; A2h on
// pages 0/1 activates the list listener (screen+8h vtable +4) with (the main list's (+1Ch) current
// row, that list); A3h on page 0 sets [00E19650] = 0, 004CC460(0Ch, 0), reloads the input configuration
// (00698A10 on game +3Ch) unless [00F88A30], clears +16Ch..+17Ch and hides +210h; page 1 ->
// 0055E3A0(0); page 2 -> 00560210. Any other label with a row id: top byte FEh / FFh sets +180h to
// 0 / 1, 0055C3B0, selection as above, 00553B00(+18Ch ? 0 : +190h), 0055E3A0(3). Other types:
// +204h / +200h auto-scroll +1A0h (2 / 1, 0.004f, 1), +20Ch begins a drag; then type 6 -> vtable
// +88h(2, 0, 1.0f), type 12h -> vtable +84h(2).
void control_binding_handle_gui_event_00560bf0(Raw listener, void* widget, Fe8ControlEventHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-fe9 (docs/CC13_FE_9.md): control binding bodies left open by cc13-fe8.
// ---------------------------------------------------------------------------
struct Fe9ControlRowsHost {
    virtual ~Fe9ControlRowsHost() = default;
    virtual Raw input_settings_005547d0() = 0;
    virtual void* input_device_map_index_0055c110(void* map, const void* key) = 0;
    // 005548C0, ECX = entry tree: the case-insensitive lower bound (kTreeLowerBound row 0).
    virtual void* binding_tree_lower_bound_005548c0(void* tree, std::string_view name) = 0;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name, std::uint32_t recurse) = 0;
    // 006A9850, ECX = settings: (NativeString* out, key, name, variant) -> the slot's key text.
    virtual std::string input_binding_slot_text_006a9850(Raw settings, const void* key, std::string_view name,
        std::uint32_t variant) = 0;
    // 006AAC50 with a null conflict output.
    virtual bool input_binding_has_conflict_006aac50(Raw settings, const void* key, std::string_view name,
        std::uint32_t variant) = 0;
    virtual void gui_text_set_localised_source_00abaed0(void* text, std::string_view source, std::uint32_t flag) = 0;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;             // vtable +34h
    virtual void widget_set_local_x_00aa78d0(void* widget, float x) = 0;                   // thiscall, RET 4
    virtual void control_binding_show_conflict_text_0055c200(Raw screen) = 0;
};

// 0055C5A0 BSP_ControlBindingScreen_UpdateBindingRowText, __fastcall(ECX = screen), RET. entry =
// map[device key] (0055C110 on settings +8h, key = settings +18h + [+16Ch]*8, checked index). Walks
// the std::list of rows at settings list +20h (+14Ch, head pointer +150h, row at node +8h). A row
// without a "main_Text" child (00AA7E00 recurse 0) is skipped; otherwise its main_Text (recurse 1)
// NativeString (+0F4h) is the binding name. When the entry holds the name (lower bound 005548C0,
// then !(name < node), with an empty name below any non-empty node key and _stricmp otherwise):
// var_1_Text / var_2_Text get 006A9850(key, name, 0 / 1) through 00ABAED0(text, 1); for slot 0 then
// 1, conflict_N_Group (00E086AC / 00E086B0) gets vtable +34h(006AAC50(key, name, slot, null)) (the
// image pushes a dword whose upper bytes are stale) and, on a conflict, its "Left_Text" (00CEE65C)
// child moves to x = -(float)(var_N_Text +114h / 960.0) (00CEC380; FDIV by the double, FSTP to a
// float, FCHS). Ends with 0055C200. The checked-iterator reports (00BF6713) are not modelled.
void control_binding_update_binding_row_text_0055c5a0(Raw screen, Fe9ControlRowsHost& host);

struct Fe9ControlListHost : Fe9ControlRowsHost {
    virtual void list_box_set_listener_00a9ac40(void* list, void* listener) = 0;
    virtual void list_box_clear_rows_00a9bec0(void* list) = 0;
    virtual void list_box_insert_row_00a9d750(void* list, void* row, std::uint32_t a, std::uint32_t b) = 0;
    virtual void list_box_select_index_00a9c7c0(void* list, std::int32_t index) = 0;
    virtual void* list_box_row_00a9be00(void* list, std::int32_t index) = 0;
    virtual void* widget_clone_subtree_00aab4c0(void* widget, std::uint32_t flag) = 0;
    virtual void widget_set_listener_fields_00aa6bc0(void* widget, void* listener, std::uint32_t flag) = 0;
    virtual void widget_set_hit_clip_rect_00aa8f30(void* widget, void* clip, std::uint32_t flag) = 0;
    virtual void widget_set_enabled_60(void* widget, std::uint32_t enabled) = 0;           // vtable +60h
    virtual void widget_virtual_80(void* widget, std::uint32_t value) = 0;                 // vtable +80h
    virtual void gui_text_set_localised_cstring_00abbe50(void* text, const char* source, std::uint32_t flag) = 0;
    // 0055A9A0, ECX = entry +18h: map<NativeString, setting>::operator[]; the setting's kind is +0h.
    virtual std::int32_t* setting_map_index_0055a9a0(void* map, std::string_view name) = 0;
    // 00444BE0, ECX = entry +24h: map<NativeString, float>::operator[].
    virtual float* setting_value_map_index_00444be0(void* map, std::string_view name) = 0;
    // 005F3750 BSP_ControlBindingScreen_SetSliderRow, __fastcall(ECX = row, EDX = position).
    virtual void control_set_slider_row_005f3750(void* row, std::int32_t position) = 0;
    virtual void control_binding_refresh_selected_binding_0055c3b0(Raw screen) = 0;
    virtual void scroller_attach_content_00683790(void* scroller, void* content) = 0;
    virtual void scroller_set_scroll_range_006834a0(void* scroller, float range) = 0;
    virtual std::array<float, 3> widget_resolved_position_00aa6750(void* widget) = 0;
    virtual std::array<float, 2> widget_size_00aa6740(void* widget) = 0;
};

// 0055D380 BSP_ControlBindingScreen_BuildSettingsList, __fastcall(ECX = screen), RET. The list +20h
// loses its listener (00A9AC40(0)), is disabled (vtable +60h(0)) and cleared (00A9BEC0). entry =
// map[device key]. Bindings: for each name i of the entry's vector +34h (8-byte NativeStrings) a
// clone of template +28h is shown, gets the widget listener (screen+0Ch, 00AA6BC0), row id i |
// FFFF0000h at +0D8h and the clip +218h (00AA8F30). Its main_Text shows the name when the entry's
// tree holds it (the 005548C0 lookup of 0055C5A0) and "^" + name otherwise. Unknown names mark the
// row +77h; known ones clear main_Text +78h, fill var_1_Text / var_2_Text from 006A9850(slot 0 / 1)
// (each also +78h = 1, the widget listener, id i | FE000000h / i | FF000000h) and show
// conflict_N_Group per 006AAC50(slot, null), whose "Right_Text" (00CEE730) moves to x =
// (float)((float)(var_N_Text +114h / 960.0) + 1/96.0) (00CEC380, 00CEE728). Each row is appended
// (00A9D750(row, 0, 0)). Settings (the vector +44h, when not empty): a spacer (template +28h, +77h)
// and the "^FE_pc.cs_invert" (00CEE710) header (+77h, +78h = 0); kind (0055A9A0 on entry +18h) 3
// or 4 gets a template +30h row: main_Text = the name, var_Text = "FE.opt_normal" when the value
// (00444BE0 on entry +24h) is above 0.0f (00D7A218, COMISS) else "FE.opt_inverted". With at least one
// such row a second spacer and header follow; the "^FE_pc.controls_sensitivities" (00CEE6F0) label
// lands on that header, or on the invert header when there were none. Every other kind gets a
// template +2Ch slider row whose "dodgeListboxAutoControl_Group" +0D8h is the row index, slider
// position 005F3750(-1 - (int)(value * -4.0)). Rows built from +30h / +2Ch carry id = the list's row
// count | FFFF0000h. Then selection min(+174h, count - 1) (00A9C7C0), vtable +80h(1), the list
// listener (screen+8h), vtable +60h(1), list +77h = 0, page 2 -> 0055C3B0. The scroller +1A0h is
// detached (00683790(0)) and gets the range ((float)(last.y - first.y) + first.h + 0.015 (00CEE6C8))
// rounded to float, minus the clip's height, rounded to float. The second 0055A9A0 call of each
// settings row (result discarded) and the checked-iterator reports are not modelled.
void control_binding_build_settings_list_0055d380(Raw screen, Fe9ControlListHost& host);

struct Fe9ControlPageHost : Fe9ControlListHost, Fe8LuaHost {
    virtual void input_manager_update_00a92c40(float dt) = 0;                                // ECX = 004BEC00()
    virtual bool input_flag_61f() = 0;                                                       // byte [00E188A8]+61Fh
    virtual bool flag_00e19428() = 0;                                                        // byte [00E19428]
    virtual std::uint8_t flag_00f88a30() = 0;                                                // byte [00F88A30]
    // 004BA6D0 on [00F8BBF4]: (class, index) -> the device or null.
    virtual void* input_backend_device_004ba6d0(std::int32_t device_class, std::int32_t index) = 0;
    // 006A9EA0, ECX = settings: does any binding of the device (key) use slot kind 2.
    virtual bool input_device_has_kind2_binding_006a9ea0(Raw settings, const void* key) = 0;
    virtual void widget_virtual_7c(void* widget, std::uint32_t value) = 0;                  // vtable +7Ch
    virtual void widget_virtual_84(void* widget, std::uint32_t value) = 0;                  // vtable +84h
    virtual void widget_set_size_58(void* widget, std::array<float, 2> size) = 0;           // vtable +58h
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    // 00531B00 on 00425D10(): (slot, message, kind, callback, 0, title "", 0.0f, 0, "", 1).
    virtual void raise_prompt_00531b00(std::int32_t slot, const char* message, std::int32_t kind,
        std::uint32_t callback) = 0;
    virtual void command_bar_set_help_line_0054a0c0(std::string_view line) = 0;              // ECX = [00E1930C]
    // 00518D60: ECX, EDX, one stack word and the title NativeString, RET 8.
    virtual void frame_select_00518d60(std::int32_t ecx, std::int32_t edx, std::uint32_t stack_word,
        std::string_view title) = 0;
    virtual void list_box_reset_highlight_00a9cf30(void* list, std::uint32_t a, std::uint32_t b) = 0;
    virtual void gui_text_set_ellipsized_00abb000(void* text, std::string_view source, float width,
        std::uint32_t flag) = 0;
    // 00BDD990 on [0109CEEC]: (directory, extension, 0, list*) -> the matching resource names; the
    // image pops them front first (00557A90).
    virtual std::vector<std::string> vfs_enumerate_resources_00bdd990(std::string_view directory,
        std::string_view extension, std::uint32_t flag) = 0;
    // [0109CEEC] vtable +4(path, 2) -> stream or null; stream vtable +18h is-open, +24h read(buffer,
    // length, &count), +4h(1) deleting destructor.
    virtual void* vfs_open_04(std::string_view path, std::uint32_t mode) = 0;
    virtual bool stream_is_open_18(void* stream) = 0;
    virtual void stream_read_24(void* stream, void* buffer, std::uint32_t length) = 0;
    virtual void stream_delete_04(void* stream) = 0;
    virtual void control_binding_build_settings_list_0055d380(Raw screen) = 0;
};

// 0055E3A0 BSP_ControlBindingScreen_RefreshPage, __thiscall(screen, page), RET 4. 00A92C40(0.0f),
// +195h = 1, +198h = 0.5f (00CE3800), [+214h] vtable +84h([00E188A8]+61Fh != 0). Command bar per
// page: 2 -> (0,"",1)(0,"",1)(B3h navigate,1)(0,"",1)(A3h back,2); 3 -> five (0,"",1); others ->
// (A2h select,0)(0,"",1)(B3h,1)(0,"",1)(A3h,2). Page 2 with [00E19428] clear, no class-2 device
// (004BA6D0(2, 0)) and a device binding of kind 2 (006A9EA0) raises "FE_pc.cs_touchcontroller"
// (slot 3, kind 3), sets +168h = 4 and returns. +214h is shown on pages 2/3 and hidden otherwise.
// Moving between pages 2 and 3 keeps the settings list; +168h = page; both lists are hidden and
// disabled, the help line cleared, and without the 2/3 move list +20h loses its listener. +34h
// hidden. Page 0: frame (6, 0, 0, "^FE_pc.controls_categories"), the device list (+1Ch) gets the
// "FE_pc.controls_general" header, one row per device key (settings +18h; vtable +7Ch([00F88A30])),
// and "FE_pc.controls_loadpreset" (+0D8h = 1); shown, 00A9CF30(1, 1), +19Ch = 0, and with
// [00E188A8]+61Fh frame (0, 0, 0, the categories title) again. Page 1: frame (6, 0, 0,
// "^FE_pc.controls_loadpreset"), one row per string of the input settings' Lua global
// ControlPresetsOrder[1..] (settings +78h), then up to 10 (7 with +61Fh) list rows from the
// "Presets" / ".preset" resources: the name loses its first 8 characters and its first ".preset",
// must be non-empty and [0-9A-Za-z ] only, and the file (VFS vtable +4(path, 2)) must start with
// "CONTROL_PRESET\x01" (00CEE75C, 15 bytes read into a zeroed 40h buffer); the row is sized
// (0.3645833f (00CEE758), its height) and ellipsized at -1.0f (00D7A260). Pages 0, 1 and any
// other page end with: the device list selects +16Ch + page*4, enabled, +77h = 0, list listener,
// vtable +80h(1), +1FCh hidden. Page 2: map[key] (result unused), frame (6, 0, 0, "^" + key)
// unless +61Fh, 0055D380 unless kept (kept: enabled, +77h = 0, listener), list +20h and +1FCh
// shown. Page 3: frame (6, 0, 0, "^FE_pc.controls_adjust") unless +61Fh, list +20h disabled with
// +77h = 1, then shown and enabled, +1FCh shown. All paths but the prompt end with the device
// list's listener (screen+8h) and list +20h byte +14Ah = 0. A null Lua string (strlen on null in
// the image) is read as "".
void control_binding_refresh_page_0055e3a0(Raw screen, std::int32_t page, Fe9ControlPageHost& host);

struct Fe9ControlPresetHost : Fe8LuaHost {
    virtual Raw input_settings_005547d0() = 0;
    virtual void* list_box_current_row_00425e50(void* list_box) = 0;
    // A fresh 00B66BD0 owner, 00B6A020 Open(libs), 00B69D40 RunScriptWithOverrides(path, flag).
    virtual void* lua_owner_open_run_flagged(std::uint32_t libs, std::string_view script, std::uint32_t flag) = 0;
    virtual void* lua_get_by_native_string_00b68100(void* table, std::string_view key) = 0;
    // 006ABA50 on the input settings with a 004425C0 LuaReader over a 00B66FA0 copy of the
    // object; the reader is destroyed afterwards (00441A20).
    virtual void input_settings_read_keyboard_archive_006aba50(void* lua_object) = 0;
    virtual void control_binding_refresh_page_0055e3a0(Raw screen, std::int32_t page) = 0;
};

// 0055F710 BSP_ControlBindingScreen_LoadSelectedPreset, __fastcall(ECX = screen), RET. With +38h
// set (a built-in preset): the input settings' Lua globals (owner +78h) ControlPresets (00CEE820)
// [the device list's (+1Ch) current row text +0F4h] is read through 006ABA50 (no nil check).
// Otherwise a fresh Lua owner, opened with 1, runs the path +3Ch (flag 1); its global Preset
// (00CEE818) is read the same way unless nil, and the owner is closed. Then 0055E3A0(0).
void control_binding_load_selected_preset_0055f710(Raw screen, Fe9ControlPresetHost& host);

struct Fe9ControlListenerHost {
    virtual ~Fe9ControlListenerHost() = default;
    virtual std::int32_t list_box_selected_index_00a9c920(void* list_box) = 0;
    virtual Raw input_settings_005547d0() = 0;
    virtual void set_options_page_00e19650(std::int32_t page) = 0;
    virtual void front_end_mark_field_40() = 0;                                              // [[00E198B8]+40h]+78h = 1
    virtual void front_end_push_interface_request_004cc460(std::uint32_t a, std::uint32_t b) = 0;  // ECX = [00E198B8]
    virtual bool input_flag_61f() = 0;                                                       // byte [00E188A8]+61Fh
    virtual void frame_select_00518d60(std::int32_t ecx, std::int32_t edx, std::uint32_t stack_word,
        std::string_view title) = 0;
    virtual bool input_config_load_suppressed_00f88a30() = 0;                                // byte [00F88A30]
    virtual void input_configuration_load_00698a10() = 0;                                    // ECX = [00E188A8]+3Ch
    virtual void control_binding_refresh_page_0055e3a0(Raw screen, std::int32_t page) = 0;
    virtual void input_settings_006aa1c0(Raw settings) = 0;                                  // ECX = settings
    virtual void frame_select_default_004c9680() = 0;
    virtual void control_binding_open_virtual_keyboard_00553bf0(Raw screen) = 0;
    virtual void raise_prompt_00531b00(std::int32_t slot, const char* message, std::int32_t kind,
        std::uint32_t callback) = 0;
    virtual void prompt_focus_last_navigation_00530c20() = 0;                                // ECX = 00425D10()
    virtual void native_string_assign_cstring_0041e350(void* string, const char* text) = 0;
    virtual bool vfs_exists_08(std::string_view path) = 0;                                   // [0109CEEC] vtable +8
};

// 0055FCA0 BSP_ControlBindingScreen_ListboxListener04, slot +4h of the list listener at screen+8h,
// __thiscall(listener, row, list box), RET 8 (screen offsets below). +16Ch + page(+168h)*4 = the
// list's selection. Page 0 (n device keys): selection 0 sets [00E19650] = 4, [[00E198B8]+40h]+78h,
// pushes interface request (0Ch, 0), with +61Fh frames (-1, -1, 0, "FE.opt_title_main"), reloads the
// input configuration unless [00F88A30], and clears +16Ch..+17Ch; 1..n chooses device sel - 1 (+16Ch),
// +174h = 0, +180h = 1, 0055E3A0(2), 006AA1C0 on the settings, then 004C9680 when +61Fh; n + 3
// pushes (0Ch, 0) and reloads / clears as above; otherwise the row's +0D8h 1 -> 0055E3A0(1), 2 ->
// 00553BF0. Page 1: a row text (+0F4h) equal to "FE.opt_cancel" (00449AF0, case-insensitive) ->
// 0055E3A0(0); selection 0 sets +38h and raises "FE_pc.cs_loadpreset_confirm" (slot 0, kind 1,
// callback 0055F9E0), then 00530C20; a positive selection clears +38h, sets +3Ch =
// sprintf_s("Presets/%s.preset", text) (buffer 100h) and, when the VFS has it (vtable +8), raises the
// same prompt; a negative one -> 0055E3A0(0). Other pages: only the selection store.
void control_binding_list_listener_activate_0055fca0(Raw listener, void* row, void* list_box,
    Fe9ControlListenerHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-fe9: tactical library bodies.
// ---------------------------------------------------------------------------
struct Fe9LibraryLayoutHost {
    virtual ~Fe9LibraryLayoutHost() = default;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;     // (name, 1)
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;             // vtable +34h
    virtual void list_box_set_listener_00a9ac40(void* list_box, void* listener) = 0;
    // 00A9AC70: the list's input filter callback (004F90D0 here).
    virtual void list_box_set_input_filter_00a9ac70(void* list_box, std::uint32_t callback) = 0;
    virtual void widget_set_listener_fields_00aa6bc0(void* widget, void* listener, std::uint32_t zero) = 0;
    virtual void scroller_attach_content_00683790(void* scroller, void* content) = 0;
    virtual void scroller_attach_scroll_bar_00683380(void* scroller, void* thumb, float span, void* up,
        void* down) = 0;
};

// 004FE870 BSP_TacticalLibraryScreen_BindLayout, slot +14h of 00CEB9B4, __fastcall(ECX = screen), RET.
// Lookups (parent -> store): +4E0h "homaly_Clipbox" -> +0C8h; +4E4h "MainOOB_Listbox" -> +0BCh,
// "Main_Listbox" -> +0C0h, "Difficulty_Listbox" -> +0C4h (each: listener +64h, input filter
// 004F90D0), "Fixed_Text" -> +128h, "Changeable_Text" -> +12Ch, "Bonus_Text" -> +130h; +4DCh
// "hajnal_Group" -> +4E8h; +4E8h "powerups_Group" +4F8h, "achievment_Group" +4F4h, "movies_Group"
// +4F0h, "tutorial_Group" +500h, "units_Group" +4ECh, "noseart_Group" +504h, "arrow_left_Icon" +56Ch,
// "arrow_right_Icon" +570h, "left_big_Icon" +574h, "right_big_Icon" +578h; +4DCh "background_Group"
// -> +518h (shown) and "INFO_box_Group" -> +4FCh; +4FCh "test_Clipbox" -> +5ACh; +5ACh "textbox_Group"
// -> +514h; +514h "content_main_Text" -> +0A8h (hidden); scroller +0CCh content +514h; +4FCh
// "arrow_top_scroll_Icon" +5B8h, "arrow_botton_scroll_Icon" +5BCh, "textx_scroll_Framebox" +5C0h
// (widget listener +68h on +5C0h, +5B8h, +5BCh); scroll bar (+5C0h, 0.0902778f (00CEB19C), +5B8h,
// +5BCh).
void library_bind_layout_004fe870(Raw screen, Fe9LibraryLayoutHost& host);

// The five library loaders called by 0050FDB0. Entries are 6Ch records (layout at 004FE310);
// a fresh one has +0h/+14h null, uv rects +4h and +18h = (0, 0, 1, 1), strings +28h..+3Ch and
// +4Ch..+68h empty, and +40h..+48h uninitialised in the image (zero here).
struct Fe9LibraryLoaderHost : Fe8LuaHost {
    virtual void* game_lua_owner_1a0c() = 0;                                                 // [00E188A8]+1A0Ch
    virtual bool lua_is_table_00b661b0(void* object) = 0;
    // 004FDF20 / 004FDFD0 / 004FE080 / 004FE130 (lua_object_read_string_field): out = object[name].
    virtual void lua_read_string_field(const char* name, void* out_string, void* object) = 0;
    virtual void lua_read_string_by_index_004fe1e0(std::int32_t index, void* out_string, void* object) = 0;
    virtual void lua_read_string_field_00501d10(std::int32_t index, void* out_string, void* object,
        std::string_view fallback) = 0;
    // A NativeString assignment (the inlined pool resize + copy, or 0041DD40 + copy).
    virtual void native_string_assign(void* string, std::string_view text) = 0;
    // 00AA2660 through 004C12B0: (name, uv rect out, size out, 1.0f) -> a new texture reference.
    virtual void* resolve_gui_texture_00aa2660(std::string_view name, float* uv, float* size, float scale) = 0;
    // InterlockedDecrement([00CE2220]) of ref +4h; at zero the ref's vtable +0.
    virtual void texture_ref_release(void* ref) = 0;
    virtual void entry_vector_push_back_0050ad00(void* vector, Raw entry) = 0;
    virtual void library_entry_destruct_004fd1d0(Raw entry) = 0;
};

// 0050B8E0 BSP_TacticalLibraryScreen_LoadPowerUpLib, __fastcall(ECX = screen), RET. A fresh Lua owner
// (Open(4)) runs "Scripts\datatables\PowerUpLib.lua" (00CEBAF0, flag 0); for groups g = 0..2 of the
// global PowerupClasses: +340h + g*8 = group[g+1].GroupName (null -> ""); then while group[j] (j from
// 1) is not nil, a fresh entry is appended to +3E4h and the vector's last entry filled: +28h = name,
// +0h/+4h = texture GUITextureBackGround, +14h/+18h = texture GUITextureForeGround, +40h = g, +44h =
// 1, +5Ch = description.
void library_load_power_up_lib_0050b8e0(Raw screen, Fe9LibraryLoaderHost& host);
// 0050BF00 BSP_TacticalLibraryScreen_LoadNoseArt, __fastcall(ECX = screen), RET. The game's Lua
// globals ([00E188A8]+1A0Ch) NoseArt, when a table: for i = 0..999 (index 0 first; nil or non-table
// items are skipped, not a stop), one reused entry gets +0h/+4h = texture "models/Textures/noseart/"
// + Texture, +40h = IsJapan, +5Ch = "", +48h = i, +44h = 1, and is appended to +414h.
void library_load_nose_art_0050bf00(Raw screen, Fe9LibraryLoaderHost& host);
// 0050C300 BSP_TacticalLibraryScreen_LoadMovieList, __fastcall(ECX = screen), RET. For the four movies
// (00E08498 images, 00E084C8 missions, 00E084A8 titles) a reused entry gets +0h/+4h = texture
// "fe/movies/" + image, +30h = mission, +40h = 0, +44h = 1, +5Ch = title, and is appended to +3C4h.
void library_load_movie_list_0050c300(Raw screen, Fe9LibraryLoaderHost& host);
// 0050C760 BSP_TacticalLibraryScreen_LoadTutorialLib: as 0050B8E0 with "Scripts\datatables\
// TutorialLib.lua" (00CEBB78), global TutorialLib, 13 groups, names at +358h, entries in +404h:
// +28h = Name, +48h = Index, +30h = Movie, +38h = LuaCommand ("" when nil), +40h = g, +44h = 1,
// +5Ch = Description.
void library_load_tutorial_lib_0050c760(Raw screen, Fe9LibraryLoaderHost& host);
// 0050CE70 BSP_TacticalLibraryScreen_LoadUnitLib: as 0050B8E0 with "Scripts\datatables\UnitLib.lua"
// (00CEBBC0), global UnitLib, 5 groups, names at +318h, entries in +3F4h: +28h = Name, +48h =
// VehicleClass, +0h/+4h = texture "fe/units/" + GUITextureBackGround, +14h/+18h = texture
// "common/icons/" + GUITextureForeGround, +40h = g, +44h = 1, +45h = 0, and from Description:
// +4Ch/+54h/+5Ch = [1]/[2]/[3] (004FE1E0), +64h = [4] or "" (00501D10).
void library_load_unit_lib_0050ce70(Raw screen, Fe9LibraryLoaderHost& host);

struct Fe9LibraryRegisterHost : Fe9LibraryLoaderHost {
    virtual void frontend_screen_register_004f71d0(Raw screen) = 0;
    virtual void* gui_load_page_00aa5840(std::string_view name, std::uint32_t a, std::uint32_t b) = 0;  // ECX = 004C12B0()
    virtual void* resource_load_64(std::string_view name, std::uint32_t flag) = 0;          // [00F8D394] vtable +64h
    virtual void screen_virtual_14(Raw screen) = 0;                                          // vtable +14h (004FE870)
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual Raw award_table_00e19900() = 0;                                                  // [00E19900]
    // 0050FC30 BSP_AwardTable_FindRecord, ECX = [00E19900]+10h: (const NativeString* name) -> record.
    virtual Raw award_table_find_record_0050fc30(const void* name) = 0;
    virtual void library_load_unit_lib_0050ce70(Raw screen) = 0;
    virtual void library_load_class_tables_00504130(Raw screen) = 0;
    virtual void library_load_power_up_lib_0050b8e0(Raw screen) = 0;
    virtual void library_load_movie_list_0050c300(Raw screen) = 0;
    virtual void library_load_tutorial_lib_0050c760(Raw screen) = 0;
    virtual void library_load_nose_art_0050bf00(Raw screen) = 0;
    virtual void library_preview_setup_environment_00502480(Raw screen) = 0;
};

// 0050FDB0 BSP_TacticalLibraryScreen_Register, __fastcall(ECX = screen), RET. 004F71D0; pages
// (00AA5840(name, 1, 0)) "FE_tactical" -> +4DCh, "FE_tactical_text" -> +4E0h, "FE_tactical_listbox"
// -> +4E4h; [00F8D394] vtable +64h(name, 0): "colourremap.tga" +6Ch, "oceannormalmap04.tga" +70h,
// "hdr_rcs_usn_04_defend_g_ocean.tga" +74h, "hdr_RCS_usn_04_Defend_G.tga" +78h; vtable +14h; +4C8h = 1,
// +454h = -1, +458h = 0, +94h = 3, +9Ch = 0, +0A0h = 2, +540h..+554h = 0. +5B0h = a new blank entry
// (null when the allocation fails, then dereferenced anyway): +28h "FE.unitlib_nounlock", +48h = -1,
// texture "fe/lobby/unlock_disabled.tga", +40h = -1, +44h = +46h = 1, +4Ch/+54h/+5Ch
// "FE.unitlib_nounit_d1/2/3", +64h " ". Each award name of [00E19900] (vector +4h, 8-byte
// NativeStrings) appends a blank entry to +3D4h and fills it: +14h/+18h = texture of the award
// record's (0050FC30) +14h string, +0h/+4h = texture "common/whiteGui.tga", category +40h = 6, then by
// prefix (case-insensitive): RANK_ 0, CA_ 1, MA_ 2, AO_ 2, RUA_ 3, GA_ 4, SA_ 5, B 7 (and +44h = 0;
// otherwise +44h = 1); +28h = the name, +45h = 1. Then the unit library, 00504130, power-ups,
// movies, tutorials, nose art and 00502480.
void library_register_0050fdb0(Raw screen, Fe9LibraryRegisterHost& host);

// The library view at screen+434h: +0h current index, +4h float carousel position, +14h/+18h the
// filtered 6Ch-entry vector (screen +448h/+44Ch). Accessors, __fastcall(ECX = view), EAX (the
// checked-index reports 00BF6713 are not modelled; truncation is FISTP under RC = chop or CVTTSS2SI):
std::uint8_t* library_view_class_id_004fa890(Raw view);     // &entries[+0h] + 48h
std::uint8_t* library_view_previous_004fa8f0(Raw view);     // entries[(int)pos - 1]
std::uint8_t* library_view_at_position_004fa960(Raw view);  // entries[(int)pos]
std::uint8_t* library_view_current_004fa9c0(Raw view);      // entries[+0h]
std::uint8_t* library_view_next_004faa00(Raw view);         // entries[(int)pos + 1], or the last one

struct Fe9LibraryDetailHost {
    virtual ~Fe9LibraryDetailHost() = default;
    virtual void sleep_00ce2230(std::uint32_t milliseconds) = 0;
    virtual bool widget_virtual_38(void* widget) = 0;                                        // vtable +38h
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;
    virtual void gui_set_state_texture_uv_00ab2690(void* widget, std::uint32_t state, void* texture,
        const float* uv) = 0;
    // 00AA86B0: vtable +48h(scale) on the widget and, recursively, its children.
    virtual void widget_set_scale_recursive_00aa86b0(void* widget, const float scale[2]) = 0;
    virtual void widget_set_colour_50(void* widget, const void* colour) = 0;                 // vtable +50h
    virtual const void* colour_static_grey_004f9d80() = 0;
    virtual const void* colour_static_white_004f9d30() = 0;
    virtual void widget_set_listener_fields_00aa6bc0(void* widget, void* listener, std::uint32_t zero) = 0;
    virtual std::int16_t widget_state_94(void* widget) = 0;                                  // vtable +94h
    virtual void widget_set_state_88(void* widget, std::uint32_t state, std::uint32_t zero, float one) = 0;
    virtual void gui_text_set_localised_source_00abaed0(void* text, std::string_view source, std::uint32_t flag) = 0;
    virtual void gui_text_set_localised_cstring_00abbe50(void* text, const char* source, std::uint32_t flag) = 0;
    virtual Raw award_table_find_record_0050fc30(const void* name) = 0;                     // ECX = [00E19900]+10h
    // 006B8AD0 BSP_LuaMachine_RunString on [[00E188A8]+1A08h]+4h: (text, 0, 0, 2) -> 0 on success.
    virtual std::int32_t lua_machine_run_string_006b8ad0(const char* text, std::uint32_t a, std::uint32_t b,
        std::uint32_t c) = 0;
    virtual void debug_print_004254b0(const char* format, const char* text) = 0;             // empty body
    virtual bool input_config_load_suppressed_00f88a30() = 0;                                // byte [00F88A30]
    // 00A9FAD0 on [00F8BC4C] (resolve the key to its wide text), then 00436630 (low bytes).
    virtual std::string localization_resolve_narrow(std::string_view key) = 0;
    virtual void library_preview_load_vehicle_model_0050aa10(Raw screen) = 0;
};

// 00510C20 BSP_TacticalLibraryScreen_RefreshEntryDetail, __fastcall(ECX = screen), RET. No card
// (+508h) -> nothing; an empty entry vector -> Sleep(1). With +90h == 0 the carousel cards (+508h
// previous, +50Ch current, +510h next) are shown as (0,1,0) for one entry, (0,1,1) at position 0,
// (1,1,0) at the last position, else (1,1,1); every visible card (vtable +38h) takes its entry's
// textures (+0h/+4h on the first widget, +14h/+18h on the second and, when present, the third:
// +580h/+57Ch/+584h, +58Ch/+588h/+590h, +598h/+594h/+59Ch) through 00AB2690(0, ...), each scaled to
// 1.00001f (00CEBDC8; 00AA86B0), and is greyed (004F9D80) on page kind 6 for entries whose +46h is
// clear, white (004F9D30) otherwise. Arrows +574h / +578h: with +560h shown and given the widget
// listener (+68h), state 3 at the first / last index, else state 0 unless already 1 or 2 (vtable
// +94h / +88h(state, 0, 1.0f)); without +560h hidden with a null listener. Detail text +0A8h by kind
// +94h, with sep = "|. |####|" (00CEBDBC) and NAME = the current entry's +28h upper-cased (004FB1F0):
// 0 (awards): the award record's (0050FC30) name upper-cased; with the current entry's +45h set and
// the entry at the position in category 5 "FE.achievement_secret_desc", else that name + sep +
// record +0Ch; 1, 3, 8: NAME + sep + +5Ch; 2, 5, 6:
// +4B0h = class id (+48h), NAME + sep + +5Ch + sep + +64h, then 0050AA10 when +90h == 1; 4 (tutorials,
// non-empty vector): a LuaCommand (+38h) is run (006B8AD0(command, 0, 0, 2)) and logged through
// 004254B0; the description +5Ch takes "_pc" when [00F88A30] is clear and that key localises to
// something else; NAME + sep + description; 7: nothing.
void library_refresh_entry_detail_00510c20(Raw screen, Fe9LibraryDetailHost& host);

// The preview scene's environment block: a 288h-byte stack record built by 004CB420 and destroyed
// by 004C6520; fog colour +80h, directional fog colours +90h..+CCh, fog scalars +0D0h..+0E4h (see
// scene_weather_cc13.hpp for the authored names), NativeStrings +1Ch, +24h, +44h, +208h, +210h,
// +218h, +234h, +23Ch, +244h, +24Ch, +254h and +280h.
inline constexpr std::size_t kPreviewEnvironmentBlockSize = 0x288;
struct PreviewEnvironmentWrite {
    std::uint16_t offset;
    float value;
};

struct Fe9PreviewEnvironmentHost {
    virtual ~Fe9PreviewEnvironmentHost() = default;
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    // 0078DAA0 BSP_OceanEnvironment_Construct, __thiscall(storage, ocean source, 0), EAX = storage.
    virtual void* ocean_environment_construct_0078daa0(void* storage, void* source, std::uint32_t zero) = 0;
    virtual void scene_environment_block_construct_004cb420(void* block) = 0;
    virtual void scene_environment_block_destruct_004c6520(void* block) = 0;
    virtual void native_string_assign(void* string, std::string_view text) = 0;
    // 00B1B830 on [00F8D434]: (const NativeString* name, const NativeString* value).
    virtual void world_parameter_set_string_00b1b830(std::string_view name, const void* value) = 0;
    // Fog owner = [screen+10h]+184h.
    virtual void fog_set_color_00b84c40(void* fog, const float* rgba) = 0;
    virtual void fog_set_directional_color_00b84fa0(void* fog, const float* rgba, std::uint32_t index) = 0;
    virtual void fog_set_scalar_68_00b84d00(void* fog, float value) = 0;
    virtual void fog_set_scalar_6c_00b84d10(void* fog, float value) = 0;
    virtual void fog_set_scalar_70_00b84d20(void* fog, float value) = 0;
    virtual void fog_set_scalar_74_00b84d30(void* fog, float value) = 0;
    virtual void fog_set_scalar_78_00b84d40(void* fog, float value) = 0;
    // 004FB850, __thiscall(out dword, const float rgba[4]): the ARGB word.
    virtual std::uint32_t colour_float_rgba_to_argb_004fb850(const float* rgba) = 0;
    virtual void camera_set_clear_color_00b6fe50(void* camera, std::uint32_t argb) = 0;
    virtual void linearize_rgb_004e5c00(float* rgb) = 0;                                     // scene_weather_cc13
    // 0078C9B0, ECX = the ocean environment: applies the block.
    virtual void ocean_environment_apply_0078c9b0(void* ocean, void* block) = 0;
    virtual bool flag_00f88a0c() = 0;                                                        // byte [00F88A0C]
    // 00BBDFF0 BSP_Atmosphere_Construct, __thiscall(storage, ocean source, const NativeString* sky).
    virtual void* atmosphere_construct_00bbdff0(void* storage, void* source, std::string_view sky) = 0;
    virtual void* atmosphere_get_sky_box_00bbddb0(void* atmosphere) = 0;
    virtual void sky_box_set_rotation_degrees_00bbf1a0(void* sky_box, float degrees) = 0;
    virtual void sky_box_set_luminance_00bbf200(void* sky_box, float luminance) = 0;
    virtual void resource_manager_virtual_120() = 0;                                         // [00F8D394] vtable +120h
};

// 00502480 BSP_TacticalLibraryPreview_SetupEnvironment, __fastcall(ECX = screen), RET. Only when
// +4B8h is null: +4B8h = a new B8h ocean environment (0078DAA0(+0Ch, 0); null when the allocation
// fails). A default block (004CB420) gets the strings "hdr_RCS_usn_04_Defend_G.dds" +208h,
// "hdr_RCS_usn_04_Defend_G_ocean.dds" +210h and "white.tga" at +218h, +234h, +23Ch, +244h, +24Ch,
// +254h (resize with preserve 0, then the characters); world parameters ReflectionCubeMap = +208h and
// StaticShadowTexture = +254h; the first constant table (kPreviewEnvironmentFog); fog colour +80h,
// directional colours +90h + i*10h (i < 4), the camera's (+10h) clear colour from +80h (004FB850),
// fog scalars 68h/6Ch/70h/78h/74h from +0D0h/+0D4h/+0D8h/+0DCh/+0E0h; the second table
// (kPreviewEnvironmentBlock), strings +280h "oceannormalmap04.tga", +1Ch and +24h "white.tga", byte
// +0Ch = 0, +44h emptied; 004E5C00 on the triples +154h, +1A0h, +164h, +174h, +118h, +1E4h (none is
// written again afterwards); 0078C9B0(+4B8h, block). The sky is "sky_069" (00CEB4A8), "HDR_sky_069"
// with [00F88A0C]; +4BCh = a new 40h atmosphere (00BBDFF0(+0Ch, sky); null when the allocation fails)
// whose sky box turns to 70.0f (00CE77B4) and gets luminance 0.8f (00CE74F8); then [00F8D394] vtable
// +120h and the block's destructor. The tables were taken from the listing by a symbolic pass over the
// stack stores (local/fe9_stackfill.py); they keep the image's store order, duplicates included.
void library_preview_setup_environment_00502480(Raw screen, Fe9PreviewEnvironmentHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-fe9: session browser.
// ---------------------------------------------------------------------------
struct Fe9SessionCreateHost {
    virtual ~Fe9SessionCreateHost() = default;
    virtual std::array<float, 3> widget_resolved_position_00aa6750(void* widget) = 0;
    virtual void widget_set_resolved_position_00aa8240(void* widget, const std::array<float, 3>& position) = 0;
    virtual void* widget_clone_subtree_00aab4c0(void* widget, void* parent) = 0;
    virtual void gui_text_set_localised_cstring_00abbe50(void* text, const char* source, std::uint32_t flag) = 0;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;
    virtual std::uint32_t online_system_virtual_198() = 0;                                   // [00F8A2FC] vtable +198h
    virtual void scroller_set_scroll_range_006834a0(void* scroller, float range) = 0;
};

// 005684D0 BSP_MultiSessionBrowserScreen_BuildCreateOptions, __fastcall(ECX = screen), RET. From the
// label template +94h's resolved position (x, y, z) and the row step +260h, clones (parent +124h) go
// to that position with y advancing by the step after each row ((float)(step + y)): +0C0h
// "FE.multi_createrankmin" (shown); +0DCh (value template +0A4h, hidden) and +0C4h "-" (shown);
// +0C8h "FE.multi_createrankmax" (shown); +0E0h (+0A4h, hidden) and +0CCh "-" (shown); +0D4h
// "FE.multi_createdroprate" and, one row on, +0E4h (+0A4h, hidden) and +0D8h "-", both labels shown
// per [00F8A2FC] vtable +198h(). The scroller +188h range is (float)(y - 0.501) (00CEED08) with y the
// row after the last.
void session_browser_build_create_options_005684d0(Raw screen, Fe9SessionCreateHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-fe9c (docs/CC13_FE_9C.md): remaining tactical library bodies.
// ---------------------------------------------------------------------------
struct Fe9cLibraryDlcHost : Fe8LuaHost {
    virtual void* game_lua_owner_1a0c() = 0;                                                 // [00E188A8]+1A0Ch
    // 004FD8D0 BSP_VehicleClassLua_GetUnlockDlcMinimum, __thiscall(screen, class id, out).
    virtual std::string vehicle_class_unlock_dlc_minimum_004fd8d0(Raw screen, std::int32_t class_id) = 0;
    virtual std::uint64_t owned_dlc_mask_2060() = 0;                                         // [00E188A8]+2060h
};

// 005021E0 BSP_TacticalLibraryScreen_LoadDownloadableClasses, __fastcall(ECX = screen), RET. Fetches
// (and only releases) the game Lua global VehicleClass; for each 6Ch entry of +3F8h..+3FCh: the
// class's UnlockDLCMinimum (004FD8D0 with the entry's +48h) loses every "DL_Content_" (00CEB49C;
// 004CAD40 with "" and count 7FFFFFFFh), n = atol of the rest, and +46h = (mask & owned) == mask for
// the 64-bit mask 1 << (n - 1) (__allshl: a count of 64 or more, n = 0 included, gives 0, so 1).
void library_load_downloadable_classes_005021e0(Raw screen, Fe9cLibraryDlcHost& host);

struct Fe9cVehicleRowHost : Fe8LuaHost {
    virtual void* game_lua_owner_1a0c() = 0;
    virtual void lua_iterate_first_00b67080(void* table, void* key, void* value) = 0;
    virtual void lua_iterate_next_00b67190(void* table, void* key, void* value) = 0;
    virtual bool lua_is_unbound_00b66420(void* key) = 0;
    virtual bool lua_is_table_00b661b0(void* object) = 0;
    virtual bool lua_is_boolean_00b66000(void* object) = 0;
    virtual bool lua_get_boolean_00b66250(void* object) = 0;
    virtual bool lua_is_string_00b660a0(void* object) = 0;
    // 007FC4C0 on [00E188A8]+650h: is the unlock with this id satisfied.
    virtual bool profile_is_unlock_satisfied_007fc4c0(std::string_view id) = 0;
    // Key/value iteration handles owned by the host for the walk.
    virtual void* lua_new_object_00b65f50() = 0;
    virtual void int_vector_push_back_00442190(void* vector, std::int32_t value) = 0;
    virtual void string_vector_append_00450540(void* vector, std::string_view text) = 0;
};

// 0050A400 BSP_TacticalLibrary_ReadVehicleClassRow, __thiscall(ECX unused, class ids, types, flags,
// unused), RET 10h. Walks the game Lua global VehicleClass (00B67080 / 00B67190 until the key is
// unbound); a table value whose Unlock is the boolean true and whose UnlockID is nil, "" or a
// satisfied unlock (007FC4C0; a non-string non-nil id fails) and whose Race is not nil with an
// integer above 0 appends: the key's integer to class ids, Type to types (00450540), and (Race 1 or
// 4) to flags.
void library_read_vehicle_class_rows_0050a400(void* class_ids, void* types, void* flags,
    Fe9cVehicleRowHost& host);

struct Fe9cPreviewLoadHost : Fe8LuaHost {
    virtual void* game_lua_owner_1a0c() = 0;
    virtual void library_preview_clear_rotors_0050a950(Raw screen) = 0;
    virtual void debug_print_004254b0(const char* format, std::int32_t value) = 0;           // empty body
    // 00BDF4C0 on [0109CEEC]: does the resource exist.
    virtual bool vfs_resolve_existing_00bdf4c0(std::string_view name) = 0;
    // 005055C0 on 004FDE20(): enqueue a load job for the path; the last job's +1Ch is then set to
    // 00509FB0 when the queue (+10h, count +14h) is not empty.
    virtual void loading_queue_enqueue_005055c0(std::string_view path) = 0;
    virtual void loading_queue_set_last_callback(std::uint32_t callback) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void* library_preview_model_construct_004fcfc0(void* storage) = 0;
    virtual void native_string_assign_cstring_0041e350(void* string, const char* text) = 0;
    // 00504D20 on 004FDE20(): (const NativeString* name, model).
    virtual void loading_queue_submit_00504d20(void* name, void* model) = 0;
};

// 0050AA10 BSP_TacticalLibraryPreview_LoadVehicleModel, __fastcall(ECX = screen), RET. Nothing when
// +4B4h == +4B0h; else 0050A950, +4B4h = +4B0h, and nothing more for -1. The pack is
// "mpak/Class_own_<class>.mpak", else "mpak/Class_<class>.mpak", else (logging "ERROR:MPAK files not
// found for class %d!!!") "giachievement_model_load_temp"; it is enqueued (005055C0) and the last job
// gets the callback 00509FB0. A new 60h preview model (004FCFC0; null when the allocation fails,
// then dereferenced) gets +8h = VehicleClass[class].Mesh, +18h = 0, +1Ch = the class, and is
// submitted (00504D20(model +8h, model)).
void library_preview_load_vehicle_model_0050aa10(Raw screen, Fe9cPreviewLoadHost& host);

struct Fe9cPreviewSceneHost {
    virtual ~Fe9cPreviewSceneHost() = default;
    virtual void simple_3d_scene_init_0093ccc0(void* scene) = 0;                            // ECX = screen+8h
    virtual void camera_set_render_mode_and_mask_00b6fdf0(void* camera, std::uint32_t value) = 0;
    virtual void camera_set_clear_flags_00b6fe10(void* camera, std::uint32_t flags) = 0;
    virtual void* scene_lighting_owner_00b72110(void* scene) = 0;
    // 004B4D80 BSP_Lighting_DirectionFromAngles(out, a, b).
    virtual std::array<float, 3> lighting_direction_from_angles_004b4d80(float a, float b) = 0;
    virtual bool flag_00f88a0c() = 0;                                                        // byte [00F88A0C]
    virtual void linearize_rgb_004e5c00(float* rgb) = 0;
    virtual void light_set_base_diffuse_004b62e0(void* light, const float* rgba) = 0;
    virtual void light_set_diffuse_scale_00b7af90(void* light, float scale) = 0;
    virtual void light_set_specular_scale_00b7b010(void* light, float scale) = 0;
    virtual void ambient_set_00b7af20(void* ambient, const float* rgba) = 0;
    virtual void ambient_set_cube_face_00b7af60(void* ambient, const float* rgba, std::uint32_t face) = 0;
    virtual void render_device_virtual_04(std::uint32_t value) = 0;                          // [00F8BBF0] vtable +4h
    virtual std::uint8_t flag_00f88a04() = 0;
    virtual std::uint8_t flag_00f88a0d() = 0;
    virtual std::uint32_t dword_00f889d8() = 0;
    // 00B107F0 on [00F8D39C]: (a0c, a0d, 9d8).
    virtual void render_resources_initialize_00b107f0(std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    virtual void scene_renderer_set_hdr_params_00b0e9b0(const void* params) = 0;            // ECX = [00F8D39C]
    // 00A8FD30, ECX = light, DL = 0 -> a referenced shadow owner.
    virtual void* directional_light_create_shadow_owner_00a8fd30(void* light) = 0;
    virtual void directional_light_set_shadow_owner_00b7bdf0(void* light, void* owner) = 0;
    virtual void ref_release(void* object) = 0;                                              // [00CE2220] at +4h, vtable +0
    virtual void lua_run_script_00b69d40(std::string_view script, std::uint32_t flag) = 0;   // on [00E188A8]+1A0Ch
};

// 00504130 BSP_TacticalLibraryScreen_LoadClassTables (the name is the ledger's; most of the body sets up
// the preview scene), __fastcall(ECX = screen), RET. 0093CCC0 on +8h; camera +10h render mode 0, clear
// flags 7; the first light of the scene's (+0Ch) lighting owner (+1Ch list): direction from angles
// (0.4363323f, 4.363323f) (00CEB664 / 00CEB668) into +1E0h..+1E8h; base diffuse (1.0, 0.8, 0.6157,
// 1.0) linearised with [00F88A0C]; the same colour (rebuilt) stored raw at +1C4h..+1D0h; diffuse
// scale 4.0f, specular scale 35.0f. The ambient (lighting +10h) gets (0.2392, 0.24315, 0.17645) linearised
// with alpha 1.0f, and six cube faces (kPreviewAmbientFaces): s = (float)(s * 1.5) (00CE3D78), colour
// ((float)(r*s), (float)(g*s), (float)(b*s), 1.0f) linearised, face i. Then [00F8BBF0] vtable
// +4h([00F88A04]); 00B107F0([00F88A0C], [00F88A0D], [00F889D8]); HDR parameters (kPreviewHdrParams,
// byte +24h = 0); a shadow owner created for the light, set and released; +48Ch = 0; the game Lua
// runs "SCRIPTS/datatables/autoload/VehicleClasses.lua" and ".../DeviceClasses.lua" (flag 0);
// +4B0h = 1.
inline constexpr std::array<std::array<float, 4>, 6> kPreviewAmbientFaces{{
    {0.24709999561309814f, 0.29409998655319214f, 0.3215999901294708f, 1.0f},
    {0.7293999791145325f, 0.7215999960899353f, 0.5960999727249146f, 1.0f},
    {0.26669999957084656f, 0.28630000352859497f, 0.3176000118255615f, 1.0f},
    {0.1412000060081482f, 0.18039999902248383f, 0.20389999449253082f, 0.5f},
    {0.40779998898506165f, 0.45489999651908875f, 0.53329998254776f, 1.0f},
    {0.8587999939918518f, 0.8510000109672546f, 0.7922000288963318f, 1.0f},
}};
struct PreviewHdrParams {
    float values[9];   // +00h..+20h
    std::uint8_t flag; // +24h
    float scale;       // +28h
};
inline constexpr PreviewHdrParams kPreviewHdrParams{
    {0.5f, 2.0f, 0.30000001192092896f, 0.05000000074505806f, 0.10000000149011612f, 0.10000000149011612f, 10.0f, 2.0f,
     10.0f},
    0,
    1.0f};
void library_load_class_tables_00504130(Raw screen, Fe9cPreviewSceneHost& host);

struct Fe9cOrderOfBattleRowHost {
    virtual ~Fe9cOrderOfBattleRowHost() = default;
    virtual void* library_find_entry_by_id_004fb090(Raw screen, std::int32_t id) = 0;      // result unused
    virtual std::string vehicle_class_name_or_no_unlock_004fda50(Raw screen, std::int32_t id) = 0;
    virtual void* widget_clone_subtree_00aab4c0(void* widget, void* parent) = 0;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;     // (name, 1)
    virtual void widget_set_name_2c(void* widget, std::string_view name) = 0;               // vtable +2Ch
    virtual void gui_text_set_ellipsized_00abb000(void* text, std::string_view source, float width,
        std::uint32_t flag) = 0;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;
    virtual void gui_text_set_localised_source_00abaed0(void* text, std::string_view source, std::uint32_t flag) = 0;
    virtual void gui_text_set_localised_cstring_00abbe50(void* text, const char* source, std::uint32_t flag) = 0;
    virtual void list_box_insert_row_00a9d750(void* list, void* row, std::uint32_t a, std::uint32_t b) = 0;
};

// 00500A90 BSP_TacticalLibraryScreen_FillOrderOfBattleRow, __thiscall(screen, index), RET 4. Record =
// the 2Ch-byte row index of +460h..+464h (id +0h, count +8h). 004FB090(id) runs with its result unused;
// a template +0ACh clone is named by the decimal index (vtable +2Ch), +77h = 0; "MenuItem_Text" gets
// the class name (004FDA50), ellipsized at -1.0f, shown; "NumUnits_Text" is shown with "x" + count
// for a positive count, else " " (00CE3A90). The row is appended to +0B8h; a count of -1 then marks
// the row +77h, empties NumUnits_Text and sets byte +121h of the list +0B8h.
void library_fill_order_of_battle_row_00500a90(Raw screen, std::int32_t index, Fe9cOrderOfBattleRowHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-fe11 (docs/CC13_FE_11.md): last open tactical library / preview bodies.
// ---------------------------------------------------------------------------
struct Fe11DifficultyHost {
    virtual ~Fe11DifficultyHost() = default;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;            // vtable +34h
    virtual void widget_set_listener_fields_00aa6bc0(void* widget, void* listener, std::uint32_t zero) = 0;
    // 00518D60, ECX = 0, EDX = 0, stack (0, title NativeString), RET 8.
    virtual void frame_select_00518d60(std::uint32_t ecx, std::uint32_t edx, std::uint32_t zero,
        std::string_view title) = 0;
    virtual void command_bar_set_help_line_0054a0c0(std::string_view line) = 0;              // ECX = [00E1930C]
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    virtual void list_box_clear_rows_00a9bec0(void* list_box) = 0;
    virtual std::array<float, 3> widget_resolved_position_00aa6750(void* widget) = 0;
    virtual void widget_set_enabled_60(void* widget, std::uint32_t enabled) = 0;             // vtable +60h
    virtual void* widget_clone_subtree_00aab4c0(void* source, std::uint32_t zero) = 0;
    virtual void widget_virtual_30(void* widget, const float* pair) = 0;                     // vtable +30h
    virtual void gui_text_set_localised_cstring_00abbe50(void* text, const char* key, std::uint32_t flag) = 0;
    virtual void list_box_insert_row_00a9d750(void* list_box, void* row, std::uint32_t a, std::uint32_t b) = 0;
    virtual void list_box_select_index_00a9c7c0(void* list_box, std::int32_t index) = 0;
    virtual void list_box_scroll_by_00a9cc50(void* list_box, std::int32_t step) = 0;
    virtual void list_box_reset_highlight_00a9cf30(void* list_box, std::uint32_t a, std::uint32_t b) = 0;
    virtual std::int32_t profile_difficulty_6ac() = 0;                                       // [[00E188A8]+6ACh]
};

// 004FF6F0 BSP_FrontEnd_ShowDifficultySelect, __fastcall(ECX = screen), RET. +94h = 7, +98h = 99;
// hides +508h, +50Ch, +510h, +12Ch, +130h, +128h, +574h (listener fields cleared), +578h (same),
// +56Ch, +570h, the forty pairs +1D8h[i] / +278h[i], +0A8h and +4FCh, shows +518h. Frame title
// "FE.main_difficulty_title" (00CEB3B8), help line "", command bar (A2h "FE.play" slot 3, B3h
// "globals.navigate", 0 "", A3h "globals.back" slot 4, 0 ""). The list +0C4h is cleared, +0B8h is
// hidden and marked +77h; 00AA6750 runs on +0C4h with its result overwritten unread; +0C4h vtable
// +60h(1). Three rows cloned from +0B0h ("FE.easy" 00CEB384, "FE.normal" 00CEB378, "FE.hard"
// 00CEB370): +78h = 1, listener screen+68h, vtable +30h(&{0.5f, 0.5f}) (00CE3800), the key set on the
// row itself, +77h = 0, +100h = 1 (the second row sets it after the insert), appended with (0, 0).
// The list is shown, +77h = 0, then 00A9C7C0(1), 00A9CC50(1), 00A9CC50(2), 00A9CF30(1, 1) and
// 00A9C7C0 with the profile difficulty [[00E188A8]+6ACh].
void front_end_show_difficulty_select_004ff6f0(Raw screen, Fe11DifficultyHost& host);

struct Fe11PreviewDrawHost {
    using Mat4 = std::array<float, 16>;
    using Vec4 = std::array<float, 4>;
    using Vec3 = std::array<float, 3>;
    virtual ~Fe11PreviewDrawHost() = default;
    virtual float preview_distance_00e084f8() = 0;                                           // float [00E084F8]
    virtual float game_time_64c() = 0;                                                       // float [[00E188A8]+64Ch]
    virtual Mat4 rotation_y_00b646e0(float angle) = 0;                                       // ECX out, EDX &angle
    virtual Mat4 rotation_z_00b64780(float angle) = 0;                                       // ECX out, EDX &angle
    virtual Vec4 vector4_transform_00b62d10(const Vec4& in, const Mat4& matrix) = 0;         // ECX in, (out, matrix)
    virtual Mat4 look_at_00b63f10(const Vec3& eye, const Vec3& target, const Vec3& up) = 0;
    virtual Mat4 matrix_multiply_00413920(const Mat4& left, const Mat4& right) = 0;
    virtual Mat4 matrix_invert_general_00b632d0(const Mat4& source) = 0;
    virtual void camera_set_view_matrix_00b71490(void* camera, const Mat4& view) = 0;
    virtual void camera_set_field_of_view_00b6fbb0(void* camera, float fov) = 0;
    virtual void camera_set_near_plane_00b6fbf0(void* camera, float near_plane) = 0;
    virtual void camera_set_far_plane_00b6fc10(void* camera, float far_plane) = 0;
    virtual void camera_set_aspect_ratio_00b6fbd0(void* camera, float aspect) = 0;
    virtual Mat4 camera_view_projection_00b70490(void* camera) = 0;
    virtual Mat4 camera_projection_00b6fcf0(void* camera) = 0;
    virtual void transform_refresh_world_matrix_00b6db70(void* node) = 0;
    virtual void debug_print_004254b0(const char* format, std::uint32_t index, void* node) = 0; // empty body
    virtual Vec3 node_copy_local_position_00b6e0a0(void* node) = 0;
    virtual void node_set_local_matrix_38(void* node, const Mat4& matrix) = 0;               // vtable +38h
    // 00B6FDE0 (camera viewport) then 00B1F740 (size, two dwords).
    virtual std::array<std::int32_t, 2> camera_viewport_size(void* camera) = 0;
    // 00B6FDE0 then 00B1F940(&{width, height}).
    virtual void camera_viewport_set_dimensions(void* camera, const std::int32_t* size) = 0;
    virtual std::int32_t display_width_24() = 0;                                             // [[0109CF04]+24h]
    virtual std::int32_t display_height_28() = 0;                                            // [[0109CF04]+28h]
    virtual float display_aspect_10() = 0;                                                   // float [[0109CF04]+10h]
    virtual void object_delete_04(void* object, std::uint32_t flags) = 0;                    // vtable +4h
    virtual void library_preview_setup_environment_00502480(Raw screen) = 0;
    virtual void scene_renderer_set_hdr_params_00b0e9b0(const void* params) = 0;            // ECX = [00F8D39C]
    // 00B0FD70 / 00B0FDC0 / 00B0FE10 / 00B0FE60 on [00F8D39C] (slot 0..3, NativeString name).
    virtual void render_resources_set_remap_texture(std::uint32_t slot, std::string_view name) = 0;
    virtual void* scene_lighting_owner_00b72110(void* scene) = 0;
    virtual void crt_invalid_parameter_00bf6713() = 0;
    virtual void* light_shadow_map_owner_00b7aab0(void* light, void* camera) = 0;
    virtual void shadow_map_update_view_from_camera_00a8f3b0(void* shadow) = 0;
    // 0078CFF0, ECX = the environment object +4B8h, (camera, time).
    virtual void environment_update_0078cff0(void* environment, void* camera, float time) = 0;
    virtual void scene_renderer_submit_scene_render_00b101c0(void* scene, void* camera) = 0; // ECX = [00F8D39C]
};

// 00503510 BSP_FrontEndPreview_Draw, __fastcall(ECX = preview screen), RET. +7Ch = [00E084F8] (v); the
// rest runs only when +90h is set. d = 50.0f (00CEB4D4) when the class byte +4CCh is set, else 0;
// h = (float)(v * 0.33333331346511841 (00CEB5B0) + d); a = -(float)([[00E188A8]+64Ch] / 3.0).
// eye = (x, y, z) * (float)(1 / w) of (v, h, 0, 1) * RotY(a); view = LookAt(eye, (0, d, 0), (0, 1, 0)) on
// the camera +10h, field of view 0.785398185f (00CEB5A8), near 1.0, far 10000.0f (00CE3D64).
// The anchors +574h / +578h (node +4Ch, world matrix refreshed when +5Ch bit 1 is clear, position
// +120h..+128h) project through the GUI camera [[[+4DCh]+0F0h]+18h]; their screen centre (mean of the
// x/w and y/w pairs, (float)(sum * 0.5)) at the depth z/w of the target projected through the 3D
// camera is unprojected with the inverse projection. The view is rebuilt as LookAt(eye again) *
// Translation(Qx/w, Qy/w, 0). Each rotor node of +428h..+42Ch gets RotZ(-(float)(t + t)) *
// Translation(its local position) through vtable +38h. When the camera viewport differs from the
// display size [0109CF04]+24h/+28h: the environment +4B8h is deleted (vtable +4h(1)), 00502480 runs,
// HDR parameters (the 00504130 block) and "ColourRemap.tga" in the four remap slots. Then aspect
// [[0109CF04]+10h], viewport dimensions, the shadow map of the scene's first light follows the
// camera, 0078CFF0(camera, t) on +4B8h when set, and the scene (+0Ch) is submitted with the camera.
// x87: the quotients and products are stored to float before reuse; the model computes them in
// double and rounds at each store.
void front_end_preview_draw_00503510(Raw screen, Fe11PreviewDrawHost& host);

struct Fe11OrderOfBattleListHost {
    virtual ~Fe11OrderOfBattleListHost() = default;
    virtual void list_box_clear_rows_00a9bec0(void* list_box) = 0;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;            // vtable +34h
    virtual void library_fill_order_of_battle_row_00500a90(Raw screen, std::int32_t index) = 0;
    virtual void list_box_scroll_to_item_00a9d360(void* list_box, std::int32_t index, float offset,
        std::uint32_t zero) = 0;
    virtual void* list_box_row_00a9be00(void* list_box, std::int32_t index) = 0;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;     // (name, 1)
    virtual std::array<float, 3> widget_resolved_position_00aa6750(void* widget) = 0;
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float* position) = 0;
    virtual void list_box_reset_highlight_00a9cf30(void* list_box, std::uint32_t a, std::uint32_t b) = 0;
};

// 00506660 BSP_TacticalLibraryScreen_BuildOrderOfBattleList, __fastcall(ECX = screen), RET. Clears the
// list +0B8h and shows it. The 2Ch-byte records of +460h..+464h are grouped by their +28h type into
// three local index vectors (00442190): type 2, type 3, type 1 (other types are skipped), and the rows
// are filled (00500A90) in that group order. 00A9D360(index, 0.023f (00CEB698), 0) runs with the type 2
// count when that group is non-empty, then with the type 2 + type 3 count when the type 3 group is
// non-empty. Each non-empty group's header (+128h for type 2, +12Ch for type 3, +130h for type 1) is
// moved to the resolved position of the "MenuItem_Text" child of the group's first row minus (0.03,
// 0.055, 0.0) (doubles 00CEB690 / 00CEB688 / 00D7A258, each difference rounded to float), and shown.
// 00A9CF30(1, 1) ends it. The record reads re-check the index (00BF6713), which the loop bound already
// guarantees.
void library_build_order_of_battle_list_00506660(Raw screen, Fe11OrderOfBattleListHost& host);

// The class table 0050A400 builds (three parallel vectors) and a 2Ch-byte record of +46Ch as
// 0050E3C0 / 0050EF80 build it.
struct Fe11VehicleClassRows {
    std::vector<std::int32_t> ids;
    std::vector<std::string> types;
    std::vector<std::int32_t> flags;   // Race 1 or 4
};
struct Fe11LibraryRecord {
    std::int32_t id = 0;               // +0h class / entry id
    std::int32_t count = 0;            // +8h unit count (-1 disables the row); +4h is left uninitialised
    std::uint8_t byte_0c = 0;          // +0Ch
    std::uint8_t byte_0d = 0;          // +0Dh
    std::string text;                  // +10h NativeString
    std::vector<std::int32_t> ids;     // +18h vector (begin +1Ch)
    std::int32_t type = 0;             // +28h group (00506660)
};

struct Fe11OpenEntryHost : Fe8LuaHost {
    virtual void library_records_clear_0050a110(Raw screen) = 0;                            // +46Ch erase all
    // 0050A400 with the three local vectors (ECX and the fourth argument unused).
    virtual Fe11VehicleClassRows library_read_vehicle_class_rows_0050a400() = 0;
    virtual std::int32_t game_effective_mode_004bca50() = 0;                                 // ECX = [00E188A8]
    // 00506C80: copy of the string vector at `data + offset`.
    virtual std::vector<std::string> string_vector_copy_00506c80(std::uintptr_t data, std::uint32_t offset) = 0;
    virtual std::int32_t selected_class_00e1956c() = 0;                                      // [00E1956C]
    virtual void* game_lua_owner_1a0c() = 0;                                                 // [00E188A8]+1A0Ch
    virtual bool lua_is_boolean_00b66000(void* object) = 0;
    virtual bool lua_get_boolean_00b66250(void* object) = 0;
    virtual std::int32_t crt_atol_00bf8417(const char* text) = 0;
    virtual std::string native_string_from_int_004260b0(std::int32_t value) = 0;
    // 004FD7C0 BSP_VehicleClassLua_GetType, __thiscall(screen, id, out).
    virtual std::string vehicle_class_type_004fd7c0(Raw screen, std::int32_t id) = 0;
    // 004FAB40 BSP_VehicleClassLua_IsHeavyCruiser, __thiscall(screen, id), AL.
    virtual bool vehicle_class_is_heavy_cruiser_004fab40(Raw screen, std::int32_t id) = 0;
    virtual void* library_find_entry_by_id_004fb090(Raw screen, std::int32_t id) = 0;
    virtual void library_records_push_0050e110(Raw screen, const Fe11LibraryRecord& record) = 0; // ECX = +46Ch
};

// 0050E3C0 BSP_TacticalLibraryScreen_OpenEntryAsUnlocks, __thiscall(screen, data, side, unused), RET 0Ch.
// Empties +46Ch, reads the class rows (0050A400) and copies the string vector `data + side*154h + off`
// where off is chosen by the effective game mode (004BCA50): 0 1CCh, 1 1DCh, 2 1ECh, 3 1FCh, 4 18Ch,
// 5 1ACh, 6 19Ch, 7 1BCh (jump table 0050EF40); other modes leave the list empty. In mode 4 the list is
// cut down to the first id whose VehicleClass[atol(id)].Type equals, ignoring case, the type picked
// by [00E1956C] (0 Destroyer, 1 Cruiser (not heavy), 2 Cruiser, 3 BattleShip, 4 TorpedoBoat, 5
// Submarine, 6 Fighter; jump table 0050EF60), with HeavyCruiser (a boolean true) matching for Cruiser;
// the list is then erased and that id appended as a decimal string. The record (kind -10, +8h -1,
// +0Ch / +0Dh 1, text "") gets ids -1 and then, for every listed class n and every class row j whose
// type equals n's type (004FD7C0), whose heavy-cruiser flag (004FAB40) equals n's, whose Race flag
// equals `side` and whose entry exists (004FB090): ids[j]; the row is consumed (id and flag -10, type
// "0_0_0" (00CEBBE0)). The record is appended to +46Ch (0050E110).
void library_open_entry_as_unlocks_0050e3c0(Raw screen, std::uintptr_t data, std::int32_t side,
    Fe11OpenEntryHost& host);

struct Fe11OrderOfBattleOpenHost {
    virtual ~Fe11OrderOfBattleOpenHost() = default;
    virtual Fe11VehicleClassRows library_read_vehicle_class_rows_0050a400() = 0;
    virtual std::int32_t crt_atol_00bf8417(const char* text) = 0;
    virtual std::string vehicle_class_type_004fd7c0(Raw screen, std::int32_t id) = 0;
    virtual void* library_find_entry_by_id_004fb090(Raw screen, std::int32_t id) = 0;
    // 0050E110 on the two local 2Ch-record vectors is modelled with std::vector; this one appends to +45Ch.
    virtual void library_battle_records_clear_0050a110(Raw screen) = 0;                     // +45Ch erase all
    virtual void library_battle_records_push_0050e110(Raw screen, const Fe11LibraryRecord& record) = 0;
    // String vectors of the side block (8-byte NativeStrings, null data read as "").
    virtual std::vector<std::string> string_vector_at(std::uintptr_t address) = 0;
    virtual bool profile_unlock_satisfied_007fc4c0(std::string_view id) = 0;                // ECX = game +650h
    // 004FC840, __stdcall(data, side, id), RET 0Ch, AL.
    virtual bool library_order_of_battle_contains_004fc840(std::uintptr_t data, std::int32_t side,
        std::int32_t id) = 0;
};

// 0050EF80 BSP_TacticalLibraryScreen_OpenEntryAsOrderOfBattle, __thiscall(screen, data, side), RET 8.
// Empties +45Ch and reads the class rows (0050A400). With E = data + side*154h:
// - for k below the count of E+170h: a record (type 1, +0Dh 1) with id atol(E+160h[2k]); when the unlock
//   E+160h[2k+1] is not satisfied (007FC4C0) it gets +0Ch 0, +0Dh 0, count -1 and text E+180h[k] (""
//   when E+180h is empty) and goes to a local list U; otherwise +0Ch = 004FC840(data, side, id), count
//   atol(E+170h[k]), text E+180h[k] + "_reached" (or ""), and, when +0Ch is set, the ids of every class
//   row with the same type (004FD7C0, case-insensitive), Race flag == side, a different id and an
//   existing entry (004FB090); it goes to a local list S. Rows are not consumed here.
// - for k below the count of E+130h: a record (+0Dh 1) with id atol(E+130h[k]), +0Ch = 004FC840, count
//   atol(E+140h[k]); not contained: type 2, text "fixed" (00CEBBF0); contained: the same row ids, type
//   3, text "change". These go straight to +45Ch.
// Then S and U are appended to +45Ch in that order. Index reads past a shorter E+160h / E+180h / E+140h trip
// the CRT bounds check (00BF6713); the model assumes consistent tables.
void library_open_entry_as_order_of_battle_0050ef80(Raw screen, std::uintptr_t data, std::int32_t side,
    Fe11OrderOfBattleOpenHost& host);

struct Fe11CarouselStepHost {
    virtual ~Fe11CarouselStepHost() = default;
    virtual void library_refresh_entry_detail_00510c20(Raw screen) = 0;
    virtual void widget_set_local_x_00aa78d0(void* widget, float x) = 0;                   // thiscall, RET 4
    virtual void widget_virtual_48(void* widget, const float xy[2]) = 0;                    // vtable +48h
    virtual void widget_set_uniform_scale_004fb9e0(void* widget, float scale) = 0;          // vtable +48h(&{s, s})
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;            // vtable +34h
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name, std::uint32_t recurse) = 0;
    virtual std::array<float, 3> widget_resolved_position_00aa6750(void* widget) = 0;
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float* position) = 0;
    virtual void gui_text_set_localised_cstring_00abbe50(void* text, const char* key, std::uint32_t flag) = 0;
    virtual std::string native_string_text(const void* string) = 0;                          // 8-byte NativeString
    virtual bool gui_movie_is_playing_00aac8e0(void* movie) = 0;
    virtual void gui_movie_stop_00aac870(void* movie) = 0;
    virtual void gui_movie_set_source_00aae320(void* movie, std::string_view path, std::uint32_t flag) = 0;
    virtual void gui_movie_start_00ab0eb0(void* movie) = 0;
    virtual void gui_movie_set_callback_7c(void* movie, std::uint32_t callback) = 0;        // vtable +7Ch
    virtual void gui_movie_forward_player_slot10_00aac8c0(void* movie) = 0;
};

// 00515050 BSP_TacticalLibraryScreen_UpdateTutorialMovie (ledger name; the body is the carousel
// animation step, the tutorial movies are its kind-4 tail), __thiscall(screen, float rate), RET 4.
// Nothing when the filtered entries (+448h..+44Ch, 6Ch each) are empty, when T = (float)+434h equals the
// position C = +438h, or when |(float)(T - C)| < 0.002f (00CE779C). Else C' = (float)(C + 2*rate*(T -
// C)) (extended until the store) goes to +438h, f = (float)(C' - trunc(C')), and 00510C20 runs. The cards
// +508h / +50Ch / +510h move to x = m - s, m, m + s (00AA78D0) with s = +43Ch, m = (float)(+440h - s*f).
// Scale: f > 0.5 -> vtable +48h (0.5, 0.5), (0.5, 0.5), (f, f); 0 < f < 0.5 -> 004FB9E0 0.5,
// (float)(1 - f), 0.5; -0.5 < f < 0 -> unchanged; otherwise 0.5, 1.0, 0.5. +0A8h is shown. With all
// three cards present and kind +94h == 8, each card's "keret_Framebox" (recursive) is moved to its
// "icon_big_BG_Icon" (direct child) resolved position (x + 0, y + 0, z + 3.0). Kind 0 / 3: arrows
// +540h..+548h / +54Ch..+554h show the +45h flag of the previous card (only when trunc(C') - 1 > -1),
// the card at the position and the next card (only when trunc(C') + 1 <= count);
// kind 0 then sets "FE.achievement_secret_desc" (00CEBDA0) on +0A8h when the positioned entry's +40h is
// 5 and the current entry's +45h is set. Kind 4: the movie slots +51Ch / +520h / +524h with frames
// +528h / +52Ch / +530h: when trunc(C') == T the centre movie +520h plays "movies/Tutorials/" + entry
// +30h + ".bik" for entry trunc(C') (skipped while +558h is that index and it is playing), +558h =
// index, end callback 004FAAB0; when trunc(C') + 1 == T the right movie +524h plays entry trunc(C') + 1
// (+55Ch); otherwise every movie forwards (00AAC8C0) and only the frames show.
void library_update_tutorial_movie_00515050(Raw screen, float rate, Fe11CarouselStepHost& host);

struct Fe11PreviewModelLoadedHost : Fe8LuaHost {
    using Mat4 = std::array<float, 16>;
    // Empty log body 004254B0 (kept for the call order).
    virtual void debug_print_004254b0(const char* format) = 0;
    // operator new(1ACh) + 007135C0(0, resource vtable +8h(0, 1.0f)); null when the allocation fails.
    virtual void* unit_part_instance_new_007135c0(void* resource) = 0;
    virtual void model_groups_select_visible_00710bb0(void* instance, std::int32_t group) = 0;
    virtual std::string format_string128_004f9a80(const char* format, std::int32_t value) = 0;
    // 00B6F9A0, ECX = node: every descendant whose name matches (an empty name matches all).
    virtual std::vector<void*> node_collect_descendants_00b6f9a0(void* node, std::string_view name) = 0;
    virtual const char* node_name_00b6d800(void* node) = 0;                                  // header +4h, may be null
    virtual void scene_node_set_visibility_00b6da70(void* node, float factor, std::int32_t flag) = 0;
    virtual void node_set_scale_2c(void* node, const float* xyz) = 0;                       // vtable +2Ch
    // 0071BA20, ECX = model (instance +160h): children whose name starts with the prefix.
    virtual std::vector<void*> model_collect_children_by_prefix_0071ba20(void* model, std::string_view prefix) = 0;
    virtual void* model_find_note_node_0071ad50(void* model, std::string_view name) = 0;
    // Push onto the dword vector screen+424h (inline fast path, 00505D50 on growth).
    virtual void rotor_vector_push(Raw screen, void* node) = 0;
    virtual void debug_print_rotor_004254b0(std::int32_t index, void* node) = 0;            // "+Rotor %d=0x%x"
    virtual void node_propagate_root_registration_00b6d890(void* node, void* scene) = 0;
    // [[00E198AC]+70h]+0B8h list: 00A9CA00 item, vtable +28h()->+4h string (atol result unused).
    virtual const char* hud_list_selected_text_00a9ca00() = 0;
    virtual std::int32_t crt_atol_00bf8417(const char* text) = 0;
    virtual void node_set_parent_00b6e680(void* node, void* parent) = 0;
    virtual void model_instance_set_numbering_00711be0(void* instance, std::int32_t value) = 0;
    virtual void* game_lua_owner_1a0c() = 0;
    virtual std::string lua_string_or_default_00b685c0(void* object, const char* fallback) = 0;
    virtual void debug_print_type_004254b0(const char* type, const char* ship) = 0;
    // vtable +44h of a model root -> bounds {x, y, z, radius}.
    virtual std::array<float, 4> node_bounds_44(void* node) = 0;
    virtual bool lua_is_number_00b66050(void* object) = 0;
    virtual double lua_get_number_00b66270(void* object) = 0;
    virtual void set_preview_distance_00e084f8(float value) = 0;
    virtual bool lua_is_table_00b661b0(void* object) = 0;
    virtual bool lua_is_integer_00b66a60(void* object) = 0;
    virtual bool lua_is_string_00b660a0(void* object) = 0;
    virtual void* lua_new_object_00b65f50() = 0;
    virtual void lua_iterate_first_00b67080(void* table, void* key, void* value) = 0;
    virtual void lua_iterate_next_00b67190(void* table, void* key, void* value) = 0;
    virtual bool lua_is_unbound_00b66420(void* table, void* key) = 0;
    virtual std::array<float, 2> lua_get_vector2_00b679f0(void* object) = 0;
    // 00718000, ECX = resource: the last point group named `name` with index `index`.
    virtual void* resource_find_last_named_point_group_00718000(void* resource, std::string_view name,
        std::int32_t index) = 0;
    virtual std::array<float, 3> vector3_cross_004f9b30(const float* edx_operand, const float* stack_operand) = 0;
    virtual void matrix_orthonormalize_basis_rows_0085dc80(float* matrix) = 0;
    // operator new(60h) + 004FCFC0; null when the allocation fails.
    virtual void* preview_job_new_004fcfc0() = 0;
    virtual void native_string_assign(void* string, const char* text) = 0;                  // null text -> ""
    virtual Mat4 rotation_y_00b646e0(float angle) = 0;
    virtual Mat4 matrix_multiply_00413920(const Mat4& left, const Mat4& right) = 0;
    // 00504D20 on 004FDE20(): (job +8h name, job).
    virtual void loading_queue_submit_00504d20(void* name, void* job) = 0;
    // 00B68D70 on a Lua object (dotted path, &found) and 00B66330(default) on the result.
    virtual float lua_resolve_float_or_default(void* object, std::string_view path, float fallback, bool& found) = 0;
    virtual void node_apply_animation_00b6e110(void* node, std::string_view name, float frame) = 0;
    virtual void node_set_local_matrix_38(void* node, const float* matrix) = 0;            // vtable +38h
    virtual void ref_array_push_00504870(Raw array, void* job) = 0;                         // screen +5C4h
    virtual std::int32_t interlocked_decrement(std::int32_t* counter) = 0;                  // [00CE2220]
    virtual void object_destroy_00(void* object) = 0;                                       // vtable +0h()
    // 00B85200 (provisional name GetDiffuseColorByEffect), ECX = node, EDX = &name, (const float* rgba).
    virtual void model_effect_diffuse_00b85200(void* node, std::string_view name, const float* rgba) = 0;
};

// 005072C0 BSP_TacticalLibraryPreview_OnModelLoaded, __thiscall(screen, resource, job), RET 8. The job is
// the 60h preview model of 0050AA10 / this routine (+4h refcount, +8h mesh name, +10h resource, +14h part
// instance, +18h mode, +1Ch class, +20h a 4x4 placement). +10h = resource; a part instance of it goes to
// +14h (group 0); root = [[instance+160h]+0Ch]. Root descendants "fizika_00".."fizika_19" without
// "ep_fizika" in their name and the model's "hajobelso" children get visibility 0 and scale 100000.0
// (00CE4BC8). "rotor", "rotor0".."rotor3" note nodes get visibility 1 and join +424h; their
// "rotor_still*" / "rotor_still_dam*" companions are hidden. The root is registered with the scene
// +0Ch. Mode 0 (the vehicle): +48Ch = the instance, numbering 0; VehicleClass[+1Ch].Type ("unknown")
// sets +4CCh = not one of MotherShip, BattleShip, Cruiser, Cargo, Destroyer, TorpedoBoat,
// LandingShip, Submarine; job +54h = 50.0 for a non-ship, else 0; +84h = (float)(radius * (non-ship
// ? 3.0 : 1.5)) from the root bounds; a non-ship's placement becomes identity with translation
// (-0.0 - x, 50.0, -0.0 - z); a Submarine hides its "water" nodes; "szarny letorik" nodes are hidden;
// +7Ch = UnitlibViewDistance when it is a number, else +84h, copied to [00E084F8]. Each integer-keyed
// Platforms entry whose Gun[1] is an integer device with a string DeviceClass[device].Mesh and whose
// resource point group "slot" with index <key> (00718000) has more than two points queues a mode-1 job for that mesh with the
// placement RotY(-yaw) * basis (row 3 = p0, row 2 = p2 - p0, row 1 = 004F9B30(p2 - p0, p1 - p0),
// orthonormalized; yaw = RestAngles[1] or the mean of Windows[1].Min/MaxHorzAngle; the vertical pair is
// read and dropped). A non-ship plays "default" at PartAnims.Wings.1 (default 0) when that path
// resolves. Mode 1 (a turret): the root is parented under the vehicle root ([[+48Ch]+160h]+0Ch) and the
// vehicle's "underwater" nodes are hidden. Every mode then sets the root's placement (vtable +38h with
// job +20h), keeps the job in +5C4h, drops the callback's reference (destroying the job at zero),
// sets the vehicle's "gun" effect colour (0.5, 0.5, 0.5, 1.0), hides "belsocockpit" and shows
// "kulsocockpit". Allocation failures (instance, job) are dereferenced in the image; the model
// assumes success.
void library_preview_on_model_loaded_005072c0(Raw screen, void* resource, Raw job,
    Fe11PreviewModelLoadedHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-trivialslots: one ordinary function per kTrivialSlots row, re-read from
// the disk bytes (local generator, rows checked against the image). `Raw` is the
// ECX object; stack arguments the body never reads are noted in the ABI and not
// passed. Adjustor and tail-jump rows appear only where the target is an existing
// declaration; the descriptive table above stays as documentation.
// ---------------------------------------------------------------------------
// 004FA170 RET (C3; next byte CC). BSP_GuiWidgetListener_Empty1C. Original ABI: __thiscall(ECX
// = this), RET.
void gui_widget_listener_empty1c_004fa170(Raw) noexcept;
// 00509D60 MOV EAX,0Bh; RET (B8 0B 00 00 00 C3; next byte CC).
// BSP_TacticalLibraryScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 11, RET.
std::int32_t tactical_library_screen_get_screen_id_00509d60(Raw) noexcept;
// 00509D70 ADD ECX,64h; JMP 006833F0 (83 C1 64 E9 78 96 17 00; next byte CC).
// BSP_TacticalLibraryWidgetListener_Current1C. Original ABI: __thiscall(ECX = this), the
// target's RET; ECX adjusted, tail call to 006833F0 (Fe3Host::scroller_recapture_006833f0).
void tactical_library_widget_listener_current1c_00509d70(Raw self, Fe3Host& host);
// 00517680 MOV EAX,23h; RET (B8 23 00 00 00 C3; next byte CC). BSP_InGameScreen23_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 35, RET.
std::int32_t in_game_screen23_get_screen_id_00517680(Raw) noexcept;
// 00517690 RET 4 (C2 04 00; next byte CC). BSP_InGameScreen23_CollectChildren. Original ABI:
// __thiscall(ECX = this, 1 stack dword unread), RET 4.
void in_game_screen23_collect_children_00517690(Raw) noexcept;
// 005178A0 RET 4 (C2 04 00; next byte CC). BSP_InGameScreen23_Update. Original ABI:
// __thiscall(ECX = this, 1 stack dword unread), RET 4.
void in_game_screen23_update_005178a0(Raw) noexcept;
// 00517D30 MOV EAX,0Ch; RET (B8 0C 00 00 00 C3; next byte CC). BSP_FrontEndFrame_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 12, RET.
std::int32_t front_end_frame_get_screen_id_00517d30(Raw) noexcept;
// 00517D40 MOV AL,1; RET (B0 01 C3; next byte CC). BSP_FrontEndFrame_IsSelfManaged. Original
// ABI: __thiscall(ECX = this) -> AL = 1, RET.
bool front_end_frame_is_self_managed_00517d40(Raw) noexcept;
// 00519450 MOV EAX,25h; RET (B8 25 00 00 00 C3; next byte CC). BSP_HudBomberScreen_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 37, RET.
std::int32_t hud_bomber_screen_get_screen_id_00519450(Raw) noexcept;
// 00555800 MOV EAX,0Fh; RET (B8 0F 00 00 00 C3; next byte CC).
// BSP_ControlBindingScreen_ScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 15, RET.
std::int32_t control_binding_screen_screen_id_00555800(Raw) noexcept;
// 005610D0 RET 8 (C2 08 00; next byte CC). BSP_LeaderboardsScreen_OnListboxActivateNoOp.
// Original ABI: __thiscall(ECX = this, 2 stack dwords unread), RET 8.
void leaderboards_screen_on_listbox_activate_no_op_005610d0(Raw) noexcept;
// 005633F0 MOV EAX,0Ah; RET (B8 0A 00 00 00 C3; next byte CC).
// BSP_LeaderboardsScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 10, RET.
std::int32_t leaderboards_screen_get_screen_id_005633f0(Raw) noexcept;
// 00565D60 MOV EAX,32h; RET (B8 32 00 00 00 C3; next byte CC). BSP_HudLimboScreen_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 50, RET.
std::int32_t hud_limbo_screen_get_screen_id_00565d60(Raw) noexcept;

// ---------------------------------------------------------------------------
// Packet cc13-tailtargets: the tactical library screen's class destructor (the routine the
// scalar deleting destructor 0050D8E0 calls).
// ---------------------------------------------------------------------------
struct Fe3LibraryDestroyHost {
    virtual ~Fe3LibraryDestroyHost() = default;
    virtual std::int32_t interlocked_decrement(void* count) = 0;                        // [00CE2220]
    virtual void ref_counted_destroy(void* object) = 0;                                 // vtable +0, ECX only
    virtual void object_delete_04(void* object) = 0;                                    // vtable +4h(1)
    virtual void renderer_virtual_124() = 0;                                            // [00F8D394] vtable +124h
    virtual void render_resources_reset_remap_textures_00b0feb0() = 0;                  // ECX = [00F8D39C]
    virtual void gui_remove_page_00aa31f0(void* page) = 0;                              // ECX = 004C12B0()
    virtual void library_preview_clear_rotors_0050a950(Raw screen) = 0;
    virtual void library_entry_destruct_004fd1d0(void* entry) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void ref_ptr_array_resize_00504960(void* array, std::uint32_t size) = 0;
    // 00432050 / 005052B0: ECX = first, EDX = last, two unread stack words (vector, screen), RET 8.
    virtual void native_string_range_destroy_00432050(void* first, void* last, void* extra1, void* extra2) = 0;
    virtual void record_2c_destroy_range_005052b0(void* first, void* last) = 0;
    // 00BF7C6E `eh vector destructor iterator`(first, size, count, dtor).
    virtual void eh_vector_destruct_00bf7c6e(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t dtor) = 0;
    // 00505E60 STL_Vector_TidyDestroy (library instantiation, bound), ECX = vector {allocator,
    // first, last, end}: 004FD1D0 on each 6Ch entry, first freed (00BF65AC), fields zeroed.
    virtual void entry_vector_tidy_00505e60(void* vector) = 0;
    virtual void simple_3d_scene_destruct_0093cc80(void* scene) = 0;                    // ECX = screen +8h
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;
};

// 0050AF20 (Ghidra CG_vector_deleting_dtor_0050af20), __thiscall(ECX = screen), RET; SEH
// 00C69B52, states 13h..2, 0, -1. Vtables +0h 00CEB9B4, +8h 00CEB9B0, +64h 00CEB99C, +68h
// 00CEB97C. Ref-counted members +6Ch, +78h, +70h, +74h released (count +4h, vtable +0 at zero;
// cleared when set). [00F8D394] vtable +124h(); 00B0FEB0 on [00F8D39C]. Pages +4DCh, +4E0h,
// +4E4h removed. +4BCh released; +4B8h deleted (vtable +4h(1)) and cleared. 0050A950. +80h =
// +7Ch = 100.0f (00CE3D08). +5B0h: 004FD1D0, free (00BF65AC), cleared. +5C4h: 00504960(0), then
// its data freed (00BF6989). Plain vectors +4A0h, +490h freed and zeroed. String vector +47Ch
// (00432050), 2Ch-record vectors +46Ch, +45Ch (005052B0): range destroyed, freed, zeroed.
// 00505E60 on +444h; plain vector +424h; 00505E60 on +414h, +404h, +3F4h, +3E4h, +3D4h,
// +3C4h. Native string arrays (00BF7C6E, 8, n, 0041DD20) +358h [0Dh], +340h [3], +318h [5].
// +64h = 00CEB0FC; 0093CC80 on +8h; 004F71A0.
void library_screen_destroy_0050af20(Raw s, Fe3LibraryDestroyHost& host);

} // namespace bsp::cc13_fe3