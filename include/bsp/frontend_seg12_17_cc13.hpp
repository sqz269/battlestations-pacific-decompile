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

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

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
inline constexpr std::array<TreeClear, 5> kTreeClear{{
    {0x00558740, 0x00558310}, {0x00559400, 0x00558E60}, {0x00559430, 0x00558ED0}, {0x0055B030, 0x0055AF60},
    {0x00558D50, 0x00558420},
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
inline constexpr std::array<StringTailCopy, 4> kStringTailCopy{{
    {0x00509830, 0x005071B0}, {0x0050A380, 0x0050A210}, {0x005577A0, 0x00557590}, {0x0055BA00, 0x0055B920},
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
inline constexpr std::array<TreeBuyNode, 5> kTreeBuyNode{{
    {0x0050D840, 0x6C, 0x0050A380, 0x68}, {0x00558DC0, 0x28, 0x005577A0, 0x24}, {0x005591E0, 0x2C, 0x00558280, 0x28},
    {0x00559510, 0x28, 0x005584D0, 0x24}, {0x00559C90, 0x2C, 0x00558F80, 0x28},
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
};
inline constexpr std::array<TreeDestroySubtree, 3> kTreeDestroySubtree{{
    {0x00554F10, 0x15, false}, {0x00558310, 0x25, true}, {0x00558E60, 0x25, true},
}};
void tree_destroy_subtree(const TreeDestroySubtree& row, void* node, Fe3TemplateHost& host);
// Case-insensitive lower bound, __thiscall(tree, const NativeString* key), RET 4, EAX = node:
// an empty node key is below any non-empty key, an empty search key is below everything else,
// otherwise _stricmp(node key, key) < 0 goes right. Node key at +0Ch/+10h.
struct TreeLowerBound {
    std::uint32_t address;
    std::uint8_t is_nil_offset;
};
inline constexpr std::array<TreeLowerBound, 2> kTreeLowerBound{{{0x005548C0, 0x25}, {0x00554920, 0x29}}};
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

} // namespace bsp::cc13_fe3
