// GUI routines of segments 16, 24 and 28, reconstruction pass (packet cc13-fe5).
//
// Segment 28 (00680DB0..00696450): the HUD warning screen, front-end managers,
// in-game interface helpers, title/attract screens, the unit "doing" icon, the
// award-tracker hint queue and the observer helpers. Segment 24
// (00604BC0..0060CB40): the plane HUD, plane effects and plane-spawn screens.
// Segment 16 (00544E60..00554600): the weapon-group HUD, the main-menu command
// owner, idle-camera shots and the control-binding screen.
// docs/CC13_FE_5.md carries the evidence, the backfilled records and the
// routines left to cc9.
//
// Model (as cc13-fe2): every routine works on the original object as raw bytes
// (Win32 layout, offsets as in the image) and reaches the engine through
// Fe5Host, whose methods are named after the image routine. Vtable stores keep
// the image's vtable addresses. New C++ interfaces, not drop-in binary
// replacements; nothing here is wired into bsp_game or was run there.

#ifndef BSP_FRONTEND_SEG16_24_28_CC13_HPP
#define BSP_FRONTEND_SEG16_24_28_CC13_HPP

#include "bsp/ingame_gui_cc13.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace bsp::cc13_fe5 {

using ingame_gui_cc13::Raw;
using ingame_gui_cc13::TrivialSlot;
using ingame_gui_cc13::TrivialSlotKind;

// ---------------------------------------------------------------------------
// Trivial bodies: screen-id getters, empty slots with their RET sizes,
// this-adjusting deleting-destructor thunks (SUB ECX,n; JMP) and whole-body
// tail jumps. Every row was read from disasm-raw.
// ---------------------------------------------------------------------------
inline constexpr std::array<TrivialSlot, 49> kTrivialSlots{{
    {0x00682140, TrivialSlotKind::ReturnInt, 0x50, 0, 0, "HudWarningScreen_GetScreenId"},
    {0x00687320, TrivialSlotKind::TailJump, 0, 0, 0x00683AA0, "MultiMenu_Deactivate_Thunk"},
    {0x00689BD0, TrivialSlotKind::Empty, 0, 0, 0, "AttractScreen_Virtual04NoOp"},
    {0x0068A320, TrivialSlotKind::ReturnInt, 0x24, 0, 0, "HudScreenSlot24_GetScreenId"},
    {0x0068A380, TrivialSlotKind::Adjustor, 0x8, 0, 0x0068A390, "scalar_deleting_dtor_0068a390_Adjustor8"},
    {0x0068A400, TrivialSlotKind::ReturnInt, 0x46, 0, 0, "HudScreenSlot46_GetScreenId"},
    {0x0068A410, TrivialSlotKind::Empty, 0, 4, 0, "HudScreenSlot46_CollectChildrenNoOp"},
    {0x0068A470, TrivialSlotKind::Adjustor, 0x8, 0, 0x0068A480, "scalar_deleting_dtor_0068a480_Adjustor8"},
    {0x0068A4E0, TrivialSlotKind::ReturnInt, 0x2f, 0, 0, "HudScreenSlot2F_GetScreenId"},
    {0x0068A540, TrivialSlotKind::Adjustor, 0x8, 0, 0x0068A550, "scalar_deleting_dtor_0068a550_Adjustor8"},
    {0x0068A5D0, TrivialSlotKind::ReturnInt, 0x4a, 0, 0, "HudBinocularsOverlayScreen_GetScreenId"},
    {0x0068A5E0, TrivialSlotKind::Empty, 0, 4, 0, "HudBinocularsOverlayScreen_CollectChildrenNoOp"},
    {0x0068A640, TrivialSlotKind::Adjustor, 0x8, 0, 0x0068A650, "scalar_deleting_dtor_0068a650_Adjustor8"},
    {0x0068A8F0, TrivialSlotKind::ReturnInt, 0x27, 0, 0, "HudCommonOverlayScreen_GetScreenId"},
    {0x0068A900, TrivialSlotKind::Adjustor, 0x8, 0, 0x0068A970, "scalar_deleting_dtor_0068a970_Adjustor8"},
    {0x006935B0, TrivialSlotKind::Empty, 0, 4, 0, "ObserverEndpoint_NoOp08_006935b0"},
    {0x006051B0, TrivialSlotKind::Adjustor, 0x10, 0, 0x00605250, "scalar_deleting_dtor_00605250_Adjustor10"},
    {0x006051C0, TrivialSlotKind::Adjustor, 0x170, 0, 0x00605250, "scalar_deleting_dtor_00605250_Adjustor170"},
    {0x006051D0, TrivialSlotKind::Adjustor, 0x24, 0, 0x00605250, "scalar_deleting_dtor_00605250_Adjustor24"},
    {0x00606220, TrivialSlotKind::ReturnInt, 0x3f, 0, 0, "HudPlaneEffectsScreen_GetScreenId"},
    {0x00606230, TrivialSlotKind::Empty, 0, 4, 0, "HudPlaneEffectsScreen_CollectChildrenNoOp"},
    {0x00606240, TrivialSlotKind::Adjustor, 0x8, 0, 0x006065B0, "scalar_deleting_dtor_006065b0_Adjustor8"},
    {0x006067D0, TrivialSlotKind::Adjustor, 0x10, 0, 0x00606880, "scalar_deleting_dtor_00606880_Adjustor10"},
    {0x006067E0, TrivialSlotKind::Adjustor, 0x170, 0, 0x00606880, "PlaneCockpitCamera_ScalarDeletingDtor_Adjustor170"},
    {0x006067F0, TrivialSlotKind::Adjustor, 0x24, 0, 0x00606880, "scalar_deleting_dtor_00606880_Adjustor24"},
    {0x00606A80, TrivialSlotKind::ReturnInt, 0x3e, 0, 0, "HudPlaneScreen_GetScreenId"},
    {0x0060C6B0, TrivialSlotKind::ReturnInt, 0x40, 0, 0, "HudScreenSlot40_GetScreenId"},
    {0x0060C750, TrivialSlotKind::TailJump, 0, 0, 0x004F71D0, "HudScreenSlot40_Register"},
    {0x0060C760, TrivialSlotKind::Empty, 0, 4, 0, "HudScreenSlot40_UpdateNoOp"},
    {0x0060C770, TrivialSlotKind::Empty, 0, 0, 0, "HudScreenSlot40_EnterNoOp"},
    {0x0060C910, TrivialSlotKind::Adjustor, 0x8, 0, 0x0060CA00, "scalar_deleting_dtor_0060ca00_Adjustor8"},
    {0x0060C960, TrivialSlotKind::ReturnInt, 0, 0, 0, "PlaneSpawnCamera_Slot130_ReturnZero"},
    {0x0060C980, TrivialSlotKind::Adjustor, 0x170, 0, 0x0060C9B0, "scalar_deleting_dtor_0060c9b0_Adjustor170"},
    {0x0060C990, TrivialSlotKind::Adjustor, 0x24, 0, 0x0060C9B0, "scalar_deleting_dtor_0060c9b0_Adjustor24"},
    {0x0060C9A0, TrivialSlotKind::Adjustor, 0x10, 0, 0x0060C9B0, "scalar_deleting_dtor_0060c9b0_Adjustor10"},
    {0x0060CB20, TrivialSlotKind::ReturnInt, 0x41, 0, 0, "HudPlaneSpawnScreen_GetScreenId"},
    {0x0060CB30, TrivialSlotKind::Adjustor, 0x8, 0, 0x0060D1A0, "scalar_deleting_dtor_0060d1a0_Adjustor8"},
    {0x00545AA0, TrivialSlotKind::TailJump, 0, 0, 0x00545600, "HudWeaponGroupEntryB_DestructUnwind"},
    {0x00545AB0, TrivialSlotKind::TailJump, 0, 0, 0x00545550, "HudWeaponGroupEntryA_DestructUnwind"},
    {0x00547370, TrivialSlotKind::ReturnInt, 0x2e, 0, 0, "HudScreen2E_GetScreenId"},
    {0x00547380, TrivialSlotKind::Adjustor, 0x8, 0, 0x00547A00, "HudScreen2E_Destroy_Adjustor8"},
    {0x00549590, TrivialSlotKind::Empty, 0, 0, 0, "MainMenuCommandOwner_ListenerNoOp1C"},
    {0x0054A980, TrivialSlotKind::Adjustor, 0x8, 0, 0x0054ABF0, "MainMenuCommandOwner_Destroy_Adjustor8"},
    {0x0054D660, TrivialSlotKind::ReturnInt, 0x34, 0, 0, "HudHintsScreen_GetScreenId"},
    {0x0054D670, TrivialSlotKind::ReturnByte, 1, 0, 0, "HudHintsScreen_Virtual04True"},
    {0x0054ECF0, TrivialSlotKind::ReturnInt, 0x2c, 0, 0, "HudIdleCameraScreen_GetScreenId"},
    {0x0054EF90, TrivialSlotKind::Adjustor, 0x8, 0, 0x0054EFA0, "HudIdleCameraScreen_Destroy_Adjustor8"},
    {0x00551B90, TrivialSlotKind::Empty, 0, 0, 0, "ControlBindingScreen_Exit"},
    {0x00551BB0, TrivialSlotKind::Empty, 0, 0, 0, "ControlBindingScreen_ListenerNoOp1C"},
}};

const TrivialSlot* find_trivial_slot(std::uint32_t address) noexcept;

// Four bodies that are one or two stores, or a register-loading tail jump.
// 0068DF50 __thiscall(listener, object), RET 4: +18h = 0.
void unit_doing_icon_listener_released_0068df50(Raw listener) noexcept;
// 00545360 __thiscall(screen), RET: +D4h = 1, +1Ch = 0.
void weapon_group_screen_suspend_00545360(Raw screen) noexcept;
// 0054D440 __thiscall(slot) -> EAX = slot: *slot = 0.
void* pointer_slot_construct_null_0054d440(Raw slot) noexcept;

// ---------------------------------------------------------------------------
// Engine services. Each method names the image routine it stands for; the
// register that carries `this` in the image is noted where it is not obvious.
// ---------------------------------------------------------------------------
struct Fe5Host {
    virtual ~Fe5Host() = default;

    // Widget virtuals (00AA8000 family), ECX = widget.
    virtual void widget_set_visible_34(void* widget, bool visible) = 0;          // +34h
    virtual void widget_set_alpha_4c(void* widget, float alpha) = 0;             // +4Ch
    virtual void widget_set_colour_50(void* widget, const float rgba[4]) = 0;    // +50h
    // 00ABBE50, ECX = text widget, (key, 1): set a localised C-string key.
    virtual void text_set_localised_key_00abbe50(void* text, const char* key) = 0;
    // 00AA7E00, ECX = parent widget, (name, 1): find a child by name.
    virtual void* gui_find_child_00aa7e00(void* parent, std::string_view name) = 0;
    // 004C12B0 then 00AA5840(name, 1, 0): load a GUI page.
    virtual void* gui_load_page_00aa5840(std::string_view name) = 0;
    // 004F71D0 BSP_FrontEndScreen_Register, ECX = screen.
    virtual void front_end_screen_register_004f71d0(Raw screen) = 0;
    // The screen's own vtable slot +14h (layout/bind), ECX = screen.
    virtual void screen_virtual_14(Raw screen) = 0;
    // 004D6790, ECX = vector, (address of a page slot).
    virtual void push_page_slot_004d6790(void* vector, void** slot) = 0;

    // Tracked sound handles (refcount at handle+4h, InterlockedIncrement /
    // InterlockedDecrement, vtable slot 0 deletes at zero).
    virtual void sound_stop_08(void* handle, bool immediately) = 0;              // +8h(flag)
    virtual void sound_handle_release(void* handle) = 0;                         // drop one ref
    virtual void sound_handle_add_ref(void* handle) = 0;
    // The function-local static of 00682800: 00E1988C, filled once (guard bit 1
    // of 00E19890) by 00A7ACF0("InGameGUI"), ECX = *00F8BBD8 (find class).
    virtual void* ingame_gui_sound_class_00e1988c() = 0;
    // 00A7B0A0("Normal") (find type) then 00A7E490(&out, sample, class, type,
    // 1), ECX = *00F8BBD8 (create tracked sound): returns a handle holding one
    // reference for the caller.
    virtual void* sound_create_tracked_normal(void* sample, void* sound_class) = 0;

    // 004E1CA0 then 00690FD0(name, 0): BSP_AwardTracker_TriggerHint.
    virtual void award_tracker_trigger_hint_00690fd0(std::string_view hint) = 0;

    // Red-black tree helpers of the front-end manager registry (a set of
    // manager pointers at 00E19898; ECX = the tree).
    virtual void tree_iterator_increment_00683d10(Raw iterator) = 0;            // ECX = iterator
    virtual void front_end_registry_insert_00684c40(void* manager) = 0;         // ECX = 00E19898
    virtual Raw front_end_registry_00e19898() = 0;
    virtual std::size_t tree_range_distance_00684340(const void* const range[4]) = 0;
    virtual void tree_erase_range_00684d00(Raw tree, const void* const range[4]) = 0;  // ECX = tree
    // 0042BCA0, ECX = an observer sub-object (non-virtual destructor).
    virtual void target_observer_destruct_0042bca0(Raw observer) = 0;

    // Logo sequence (the 80h object whose address 00E198A4 holds).
    virtual Raw logo_sequence_global_00e198a4() = 0;
    virtual void set_logo_sequence_global_00e198a4(void* logo) = 0;
    virtual void logo_sequence_advance_00685070(Raw logo) = 0;                  // ECX = logo
    // 00432050(first, last) then _free(first): destroy a native-string range
    // and free its block.
    virtual void native_string_range_destroy_free_00432050(void* first, void* last) = 0;
    virtual void crt_free(void* block) = 0;                                     // _free
    virtual void front_end_manager_destroy_base_00684fa0(Raw manager) = 0;
    virtual void front_end_manager_construct_base_00684e10(Raw manager) = 0;
    // Logos.lua through the Lua state owner (00B67700 family): open state 4,
    // run "Scripts/datatables/Logos.lua", read Szplessek[index].fmv and .delay;
    // false when the entry is nil.
    virtual void logo_table_open_00b66270() = 0;
    virtual bool logo_table_entry(int index, std::string& fmv, float& delay) = 0;
    virtual void logo_table_close() = 0;
    // 00439100: the Win32 user geography region string.
    virtual const char* user_geography_region_00439100() = 0;
    // 00450540 (ECX = logo+40h, the checked string vector), and the push of the
    // delay into the float vector at +50h (stored in place when capacity
    // allows, else 0045A120 inserts at the end).
    virtual void native_string_vector_append_00450540(Raw vector, const std::string& value) = 0;
    virtual void float_vector_insert_0045a120(Raw vector, float value) = 0;
    // 004DD5B0(1), ECX = *00E188A8: the game's one-time init.
    virtual void game_on_init_once_004dd5b0(int flag) = 0;

    // Interface music streams: 00A864F0 (ECX = stream, (float gain)) and
    // 00A874D0 (ECX = stream, (float seconds)); gain is *00F889A8.
    virtual void sound_stream_set_gain_00a864f0(void* stream, float gain) = 0;
    virtual void sound_stream_update_00a874d0(void* stream, float seconds) = 0;
    virtual float music_gain_00f889a8() = 0;
    // *00E198B8: the session-browser object whose +40h->+4h byte mutes the
    // interface music (null when absent).
    virtual Raw session_browser_00e198b8() = 0;
    // 004254B0, cdecl printf-style trace.
    virtual void trace_004254b0(const char* format, const char* argument) = 0;
    // *(00E08CD8 + 4 * id): the interface-id name table ("INTF_NONE", ...).
    virtual const char* interface_name_00e08cd8(int interface_id) = 0;
    // 00684600, ECX = manager, (interface, argument) -> AL: base apply.
    virtual bool front_end_manager_apply_request_00684600(Raw manager, int interface_id, int argument) = 0;
    // 00687330, ECX = manager, (interface, 0) -> EAX screen set.
    virtual int multi_menu_map_interface_00687330(Raw manager, int interface_id) = 0;
    // 004F8710(set, 0) and 004D8C00(*00E188A8, 1, 0), both cdecl.
    virtual void front_end_screens_set_level4_004f8710(int screen_set) = 0;
    virtual void game_input_contexts_set_level4_004d8c00() = 0;
    // 004C12B0 then 00AA0E00(flag), ECX = the GUI manager.
    virtual void gui_manager_set_enabled_00aa0e00(bool enabled) = 0;
    // 006881F0, ECX = manager: pop the interface request.
    virtual void multi_menu_pop_request_006881f0(Raw manager) = 0;
    // A no-argument virtual: (*obj)[slot](), ECX = obj.
    virtual void object_virtual(Raw object, std::uint16_t slot) = 0;
    // (*obj)[slot](arg), ECX = obj.
    virtual void object_virtual_int(Raw object, std::uint16_t slot, int argument) = 0;
    // *00E198C4: the in-game interface.
    virtual Raw ingame_interface_00e198c4() = 0;
    // 00AA0D30(name): register a localisation table.
    virtual void localization_register_table_00aa0d30(std::string_view table) = 0;
    // 004F83B0, ECX = screen: commit the visibility bytes.
    virtual void front_end_screen_commit_visibility_004f83b0(Raw screen) = 0;
    // Singleton teardown of 00687C00: 00415350 (the lifetime manager, whose
    // +10h is a CRITICAL_SECTION* with a nesting count at +18h), 00BCFCA0
    // unregister, the singleton's deleting destructor (*vtable)(1).
    virtual void* singleton_lifetime_lock_00415350() = 0;   // may return null
    virtual void critical_section_enter(void* lock) = 0;
    virtual void critical_section_leave(void* lock) = 0;
    virtual void singleton_lifetime_unregister_00bcfca0(void* object) = 0;
    virtual void** singleton_cell_00f8894c() = 0;

    // 00408120 BSP_LegacySboString_AssignSubstring(src, 0, npos), ECX = dst.
    virtual void legacy_sbo_string_assign_whole(Raw dst, Raw src) = 0;
    // Raw allocation for the STL vectors (operator new through the allocator
    // helpers, e.g. 006876D0), count of 4-byte elements.
    virtual void* stl_allocate_dwords(std::size_t count) = 0;
    // 00688830, ECX = the request vector (manager+58h), (const int* value).
    virtual void dword_vector_push_back_00688830(Raw vector, std::int32_t value) = 0;
    // 004CC460, ECX = manager, (interface, 0): push an interface request.
    virtual void front_end_manager_push_request_004cc460(Raw manager, int interface_id) = 0;
    // 2-argument trace form of 004254B0.
    virtual void trace2_004254b0(const char* format, const char* a, const char* b) = 0;
    // Online system virtuals, ECX = *00F8A2FC.
    virtual bool online_virtual_bool(std::uint16_t slot) = 0;
    virtual void online_virtual_int(std::uint16_t slot, int argument) = 0;  // argument < 0: none
    // 0076FAD0(mode), ECX = *00E188A8 + 1EF0h: BSP_Session_SetMode.
    virtual void session_set_mode_0076fad0(int mode) = 0;
    // byte 00E08700.
    virtual void set_byte_00e08700(std::uint8_t value) = 0;
    // 004D7920(state), ECX = *00E188A8: BSP_Game_RequestState.
    virtual void game_request_state_004d7920(int state) = 0;
    // byte *00E188A8 + 218Ch.
    virtual std::uint8_t game_byte_218c() = 0;
    // 0068AB80(0, 0), ECX = *00E198C4: collapse the in-game overlays.
    virtual void ingame_interface_collapse_overlays_0068ab80() = 0;
    // BSP_FileBlock_Construct(&block, name, name, 1) / BSP_FileBlock_Destroy:
    // the scoped block 00688E40 opens around its body.
    virtual void file_block_begin(std::string_view name) = 0;
    virtual void file_block_end() = 0;
    // operator new(size) then the screen constructor; the image does not test
    // for a null allocation before calling the result's +10h virtual.
    virtual Raw multi_screen_new(std::uint32_t constructor, std::uint32_t size) = 0;
    // Online system (*00F8A2FC) +74h(&out) returns a std::string by value; the
    // caller only reads its size (+14h) and frees it.
    virtual std::size_t online_error_text_size_74() = 0;
    // 004D7970(0), ECX = *00E188A8: BSP_Game_EndScene.
    virtual void game_end_scene_004d7970() = 0;
    // 005D2E70(code), ECX = (*00E198B4)+50h: the multiplayer error screen.
    virtual void multi_error_screen_set_code_005d2e70(int code) = 0;
    // byte (*00F8ABE8)+128h.
    virtual std::uint8_t xenon_byte_128() = 0;
    // 00A3E510 / 00A3EAD0, ECX = *00F8ABE8.
    virtual bool xenon_has_selected_user_00a3e510() = 0;
    virtual int xenon_selected_sign_in_state_00a3ead0() = 0;
    // *(*00E188A8 + 1FE4h): the session mode (0 single player).
    virtual std::int32_t session_mode_1fe4() = 0;
    // Front-end manager base activate / deactivate (00684700 / 00683AA0), ECX = manager.
    virtual void front_end_manager_activate_00684700(Raw manager) = 0;
    virtual void front_end_manager_deactivate_00683aa0(Raw manager) = 0;
    // *00E18D48: the movie player screen.
    virtual Raw movie_player_00e18d48() = 0;
    // 004F8A20(&path, 1, float z, loop) and 004F8AC0(), ECX = the movie player.
    virtual void movie_player_play_004f8a20(std::string_view path, int flag, float z, int loop) = 0;
    virtual void movie_player_stop_004f8ac0() = 0;
    // The attract screen global 00E198BC.
    virtual void set_attract_screen_global_00e198bc(void* screen) = 0;
    // 004C12B0 then 00AA31F0(page): remove and destroy a GUI page.
    virtual void gui_remove_and_destroy_page_00aa31f0(void* page) = 0;
    // 004D8A50(*00E188A8, c0, c1, c2, c3, c4, c5, 0), cdecl: push an input-
    // context level (the list ends at the first 0).
    virtual void game_input_contexts_push_level_004d8a50(const std::int32_t contexts[6]) = 0;
    // Spectate helpers. Movie-camera screen (+98h): 005CD0F0(unit) spectate,
    // 005CD160 -> camera. Movie screen (+9Ch): 005CD1A0(unit) engage,
    // 005CD240 -> camera (may be null). ECX = that screen.
    virtual void spectate_screen_engage(Raw screen, std::uint32_t routine, void* unit) = 0;
    virtual Raw spectate_screen_camera(Raw screen, std::uint32_t routine) = 0;
    // 0078E470 / 00798C40, ECX = camera: the camera's target unit (or null).
    virtual Raw camera_target_unit(Raw camera, std::uint32_t routine) = 0;
    // Unit virtual +5Ch(kind) -> AL: is-kind-of.
    virtual bool unit_is_kind_5c(Raw unit, int kind) = 0;
    // 00645160, ECX = unit -> AL: selectable for the current player.
    virtual bool unit_selectable_for_player_00645160(Raw unit) = 0;
    // HUD root (interface+40h): 00647300(unit) set spectated, 006485A0 force select.
    virtual void hud_root_set_spectated_unit_00647300(Raw hud_root, Raw unit) = 0;
    virtual void hud_root_force_select_unit_006485a0(Raw hud_root) = 0;
    // *00E188D8: the controlled unit.
    virtual Raw controlled_unit_00e188d8() = 0;
    // Front-end screen base (src/frontend_state_machine.cpp), ECX = screen.
    virtual void front_end_screen_construct_004f7180(Raw screen) = 0;
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;
    // 00695870, ECX = a callback-owner sub-object (non-virtual destructor).
    virtual void callback_owner_destroy_00695870(Raw owner) = 0;
    // 004B7EF0, ECX = an observer-link sub-object.
    virtual void observer_link_destroy_base_004b7ef0(Raw link) = 0;
    // (*obj)[slot](a, b), ECX = obj.
    virtual void object_virtual_2(Raw object, std::uint16_t slot, std::int32_t a, std::int32_t b) = 0;
    // 004CC460(interface, argument), ECX = manager.
    virtual void front_end_manager_push_request_arg_004cc460(Raw manager, int interface_id, void* argument) = 0;
    // *(*00E188A8 + 1ED4h): the game's current scene object (may be null; no
    // game object at all is also tested).
    virtual Raw game_scene_1ed4() = 0;
    // Screen-set and input-context levels (cdecl, lists end at 0):
    // 004F85D0 level 2 screens, 004F8670 level 3 screens, 004D8AE0 / 004D8B70
    // level 2 / level 3 input contexts on *00E188A8.
    virtual void front_end_screens_set_level2_004f85d0(const std::int32_t* screens) = 0;
    virtual void front_end_screens_set_level3_004f8670(const std::int32_t* screens) = 0;
    virtual void game_input_contexts_level2_004d8ae0(const std::int32_t* contexts) = 0;
    virtual void game_input_contexts_level3_004d8b70(const std::int32_t* contexts) = 0;
    // 004B4B00: the controlled vehicle; 0068AA40(unit), ECX = interface.
    virtual void* game_controlled_vehicle_004b4b00() = 0;
    virtual void ingame_interface_request_scene_for_unit_0068aa40(Raw iface, void* unit) = 0;
    // byte 00E08880; 004C1B90 the sound request queue singleton.
    virtual std::uint8_t byte_00e08880() = 0;
    virtual Raw sound_request_queue_004c1b90() = 0;
    // 006529E0, ECX = (interface+BCh), the support-manager screen (4Eh) -> AL.
    virtual bool support_screen_query_006529e0(Raw screen) = 0;
    // The global dword vector 00E18D1C (first) / 00E18D20 (last).
    virtual std::size_t global_vector_00e18d1c_count() = 0;
    // 0051E8E0, ECX = *(*00E198C4 + 4Ch): binoculars reset.
    virtual void binoculars_reset_0051e8e0() = 0;
    // Support-manager screen (interface+BCh) routines, ECX = that screen:
    // 0066FFA0 has available entries, 00652A30 query, 0065F170 play the
    // unavailable sound, 0065BE80(target) set the tracked-unit target.
    virtual bool support_has_available_entries_0066ffa0(Raw screen) = 0;
    virtual bool support_screen_query_00652a30(Raw screen) = 0;
    virtual void support_play_unavailable_sound_0065f170(Raw screen) = 0;
    virtual void support_set_tracked_unit_target_0065be80(Raw screen, void* target) = 0;
    // 0068BC80, ECX = interface -> AL (reconstructed below; reached through the
    // host where another routine calls it).
    virtual bool ingame_interface_build_tactical_overlay_list_0068bc80(Raw iface) = 0;
    // byte *00E188A8 + 19C4h.
    virtual std::uint8_t game_byte_19c4() = 0;
    // The title screen global 00E198C8.
    virtual void set_title_screen_global_00e198c8(void* screen) = 0;
    // 00BD3450, ECX = *0109CECC: reset the storage-operation availability.
    virtual void storage_operation_reset_availability_00bd3450() = 0;
    // *00E188A8 + 5E8h (dword) and + 5ECh (byte).
    virtual std::int32_t game_dword_5e8() = 0;
    virtual void set_game_byte_5ec(std::uint8_t value) = 0;
    // Widget +38h -> AL: is visible.
    virtual bool widget_is_visible_38(void* widget) = 0;
    // Widget +0Ch(kind) -> AL and 00AB1060 (the kind token tested by 0068DB40).
    virtual bool widget_is_kind_0c(void* widget, void* kind) = 0;
    virtual void* gui_widget_kind_00ab1060() = 0;
    // 00AA7DC0(const float xyz[3]), ECX = widget: local position and bounds.
    virtual void widget_set_local_position_00aa7dc0(void* widget, const float xyz[3]) = 0;
    // Widget +48h(const float pair[2]) and +88h(image, 0, alpha).
    virtual void widget_virtual_48(void* widget, const float pair[2]) = 0;
    virtual void widget_set_image_88(void* widget, void* image, int flag, float alpha) = 0;
    // 007B8AD0, ECX = unit -> AL.
    virtual bool unit_is_squadron_flight_leader_007b8ad0(Raw unit) = 0;
    // (*obj)[slot]() -> EAX.
    virtual void* object_virtual_ret(Raw object, std::uint16_t slot) = 0;
    // Observer pair routines (register calls): 00694A60 register / 006952A0
    // unregister, ECX = the observed object, EDX = the observer member.
    virtual void observer_register_00694a60(void* observed, void* observer_member) = 0;
    virtual void observer_unregister_006952a0(void* observed, void* observer_member) = 0;
    // Native pooled strings {count +0h, data +4h} (src/native_string.hpp):
    // 0041DD40(size, flag) resize, the pool release of the data block, and
    // read/assign helpers standing for the resize + memcpy pairs.
    virtual void native_string_resize_0041dd40(Raw string, std::size_t size, int flag) = 0;
    virtual void native_string_release(Raw string) = 0;
    virtual std::string native_string_read(Raw string) = 0;
    virtual void native_string_assign(Raw string, std::string_view value) = 0;
    // 007F8E00(name), ECX = *00E188A8 + 650h: the profile's stored-hint set.
    virtual bool award_stored_hint_contains_007f8e00(const std::string& name) = 0;
    // 005FFB20(name), ECX = tracker+30h: the hint definition (+114h blocking byte).
    virtual Raw award_hint_definition_005ffb20(Raw definitions, const std::string& name) = 0;
    // 006002B0(key) on tracker+7Ch (case-insensitive string map) then
    // 005FE8B0(value), ECX = the mapped entry.
    virtual void award_hint_map_assign_00690fb0(Raw map, std::uint32_t key, std::uint32_t value) = 0;
    // 00690FD0(name, forced): BSP_AwardTracker_TriggerHint, ECX = tracker.
    virtual void award_tracker_trigger_hint_forced_00690fd0(Raw tracker, std::uint32_t name, int forced) = 0;
    // 004D05E0, ECX = tracker+48h: clear the pooled string list.
    virtual void pooled_string_list_clear_004d05e0(Raw list) = 0;
    // List nodes of the recent-hint list (+54h, head +58h): erase one node
    // (BSP_RenderAliasList_EraseNode) and push a copy at the end (the
    // allocate-copy node, count grow, link before the head).
    virtual void alias_list_erase_node(Raw list, void* node) = 0;
    virtual void alias_list_push_back_copy(Raw list, const void* value) = 0;
    // operator new (00BF6A5B family) for the engine's own dword vectors.
    virtual void* operator_new(std::size_t bytes) = 0;
    // The observer lock: 00694280 returns the lock owner (singleton
    // 00E198E0), whose +4h is a CRITICAL_SECTION* (nesting count at +18h).
    virtual void* observer_lock_00694280() = 0;
    // 006949D0, ECX = observed, EDX = observer -> the pair record or null.
    virtual Raw observer_find_pair_006949d0(void* observed, void* observer) = 0;
    // 00694AF0 is-registered (ECX = observed, EDX = observer) -> AL.
    virtual bool observer_is_registered_00694af0(void* observed, void* observer) = 0;
    // 00694F60, ECX = an object's edge array (object+4h), EDX = pair: erase.
    virtual void observer_edge_array_erase_00694f60(Raw edges, void* pair) = 0;
    // 00695530, ECX = callback owner: detach every callback.
    virtual void callback_owner_detach_all_00695530(Raw owner) = 0;
    // 00695C50(100h), ECX = *00E198E4: purge.
    virtual void observer_purge_00695c50(int budget) = 0;
    // 0041CC80, ECX = lock owner+4h: destroy the owned critical section.
    virtual void critical_section_destroy_owned_0041cc80(Raw cell) = 0;
    // The singleton global 00E198E0.
    virtual void set_observer_singleton_00e198e0(void* value) = 0;
    // memmove_s(dst, n, src, n).
    virtual void crt_memmove_s(void* dst, std::size_t dst_size, const void* src, std::size_t count) = 0;
    // 0078CFF0(view, float clock), ECX = game+19F0h; 00B101C0(scene, view),
    // ECX = *00F8D39C; 004C7E60, ECX = *00E188A8 (tail jump).
    virtual void game_update_view_0078cff0(void* view, float clock) = 0;
    virtual void scene_renderer_submit_00b101c0(void* scene, void* view) = 0;
    virtual void game_after_render_submit_004c7e60() = 0;
    virtual Raw game_00e188a8() = 0;
    // 0077C470 BSP_UnitInstance_SendRoleTransfer, ECX = unit, (mask, take): a
    // cc9 gameplay routine (docs/SCRIPTED_HELM.md).
    virtual void unit_send_role_transfer_0077c470(Raw unit, std::uint32_t mask, int take) = 0;
    // 004B4940(float), ECX = *00E188A8: set the camera near plane.
    virtual void game_set_camera_near_plane_004b4940(float near_plane) = 0;
    // *00F876A4: the real-time clock (float seconds).
    virtual float real_clock_00f876a4() = 0;
    // 00604C80, ECX = plane screen -> EAX: the payload icon frame, -1 for none.
    virtual std::int32_t plane_screen_payload_frame_00604c80(Raw screen) = 0;
    // Widget +88h(frame, 0, alpha) with an integer frame index.
    virtual void widget_set_frame_88(void* widget, std::int32_t frame, int flag, float alpha) = 0;
    // Drop one reference: InterlockedDecrement(object+4h), at zero the
    // object's vtable slot 0 (no argument).
    virtual void refcounted_release(void* object) = 0;
    // Widget +44h(float): set rotation.
    virtual void widget_set_rotation_44(void* widget, float radians) = 0;
    // 00BF857A, the x87 fmod helper: fmod(x, y) at extended precision.
    virtual double crt_fmod_00bf857a(double x, double y) = 0;
    // 004DC940(fov, flag), ECX = *00E188A8: BSP_Game_SetViewFov.
    virtual void game_set_view_fov_004dc940(float fov, int flag) = 0;
    // 00867B10, ECX = point effect: stop its children (then the caller sets
    // byte +9h = 1 and drops its reference).
    virtual void point_effect_stop_children_00867b10(Raw effect) = 0;
    // 00B0D6A0(a, b, c, x, y, z), ECX = *00F8D39C (floats by value).
    virtual void render_set_transition_00b0d6a0(float a, float b, float c, float x, float y, float z) = 0;
    // operator new(3FCh) + memset 0 + 00605130 (the plane chase camera
    // constructor); null when the allocation failed.
    virtual Raw plane_chase_camera_new_00605130() = 0;
    // 00923870(0, scene, const float matrix[16]), ECX = camera.
    virtual void camera_add_to_scene_00923870(Raw camera, void* scene, const float matrix[16]) = 0;
    // 0042AAF0(target), ECX = camera; 004BC410(camera), ECX = *00E188A8.
    virtual void camera_set_target_0042aaf0(Raw camera, void* target) = 0;
    virtual void game_set_camera_mover_004bc410(void* camera) = 0;
    // Camera virtual +DCh(float).
    virtual void camera_virtual_dc(Raw camera, float value) = 0;
    // 004BEC00: the input manager (+4h: action table of 30h-byte rows).
    virtual Raw input_manager_004bec00() = 0;
    // 0042E740: the game tuning block (float +84h: the cockpit fov).
    virtual Raw game_tuning_0042e740() = 0;
    // 00871BA0(&out, name, 1): acquire an effect handle by name (one reference
    // for the caller in out).
    virtual void* effect_handle_acquire_00871ba0(std::string_view name) = 0;
    // 00AA67F0, ECX = widget -> local position (x, y, z).
    virtual const float* widget_local_position_00aa67f0(void* widget) = 0;
    // Plane-screen routines reached through the host where they are not
    // reconstructed here: 00607F60 ensure cockpit camera, 00608F30 rebuild
    // widgets, ECX = screen.
    virtual void plane_screen_ensure_cockpit_camera_00607f60(Raw screen) = 0;
    virtual void plane_screen_rebuild_widgets_00608f30(Raw screen) = 0;
    // 00637620(x, y), ECX = (*00E198C4)+48h (the markers screen).
    virtual void markers_screen_set_view_centre_00637620(float x, float y) = 0;
    // 0048F930(&tmp, flag) on object+C4h, then 0048F230 on the result -> AL.
    virtual bool bit_flag_set_test_0048f930(Raw flags, std::uint32_t flag) = 0;
    // 0064A4F0, ECX = screen+16Ch: the widget quad-fade constructor.
    virtual void widget_quad_fade_construct_0064a4f0(Raw fade) = 0;
    // @host-end
};

// ---------------------------------------------------------------------------
// Screen 50h, the HUD warning screen (vtable 00CF7184). Page "GUI_Warning" at
// +70h; widgets first_Group +68h, warning_text +50h, warning_1_Icon +54h,
// warning_2_Icon +58h, second_Group +60h, third_Group +64h. Four alerts
// i = 0 stall, 1 oxygen, 2 shallow water, 3 exit zone: hold +8h+4i, phase
// +18h+4i, active byte +28h+i, sample +30h+4i, tracked sound +40h+4i;
// +2Ch is the "wait" latch, +6Ch a float set to 1.0, +78h the frame dt.
// Update 00683020 is reconstructed in src/hud_warning_screen.cpp (backfilled).
// ---------------------------------------------------------------------------
inline constexpr std::string_view kWarningPage = "GUI_Warning";
inline constexpr std::array<std::string_view, 4> kWarningKeys{
    "ingame.warning_stall", "ingame.warning_o2", "ingame.warning_shallowwater",
    "ingame.warning_exitezone"};                         // 00CF72AC 00CF724C 00CF7260 00CF727C
inline constexpr std::string_view kWarningWaitKey = "ingame.wait";  // 00CF7178
inline constexpr std::string_view kStallHint = "Advanced_Hint_Stall";  // 00CF7298
inline constexpr double kWarningPulseRate = 6.2831854820251465;    // 00CE3828 (2*pi as float)
inline constexpr double kWarningPulseOne = 1.0;                    // 00D7A210
inline constexpr double kWarningPulseHalf = 0.5;                   // 00D7A280
inline constexpr double kWarningPulseSpan = 0.699999988079071;     // 00CEFFA0
inline constexpr double kWarningPulseFloor = 0.30000001192092896;  // 00CE3DC8

// 00681E60 __thiscall(screen), RET: hides first/second/third group.
void warning_screen_enter_00681e60(Raw screen, Fe5Host& host);
// 00681E90 / 00681EC0 __thiscall(screen), RET: latch +2Ch and show/hide
// first_Group with "ingame.wait" or the empty string 00CE3A0C on warning_text.
void warning_screen_show_wait_00681e90(Raw screen, Fe5Host& host);
void warning_screen_hide_wait_00681ec0(Raw screen, Fe5Host& host);
// 00681EF0 __thiscall(screen), RET: unless latched, hide first_Group and set
// the three alert widgets' alpha to 1.0.
void warning_screen_reset_widgets_00681ef0(Raw screen, Fe5Host& host);
// 00682150 __thiscall(screen), RET: hide first_Group, stop (flag 0) and drop
// every tracked alert sound.
void warning_screen_exit_00682150(Raw screen, Fe5Host& host);
// 00682740 __thiscall(screen), RET: base register, load the page, run +14h,
// clear the latch.
void warning_screen_register_00682740(Raw screen, Fe5Host& host);
// 006823C0 __thiscall(screen), RET (vtable +14h): bind the widgets under the
// page and paint warning_2_text white (1,1,1,1 from 00D7A24C).
void warning_screen_layout_006823c0(Raw screen, Fe5Host& host);
// 00682800 __thiscall(screen, int alert), RET 4: start the alert's sample once.
void warning_screen_play_sound_00682800(Raw screen, int alert, Fe5Host& host);
// 00682CB0 / 00682D60 / 00682E10 / 00682ED0 __thiscall(screen), RET: advance
// the alert phase by dt, show first_Group with the alert's key, pulse the three
// widgets' alpha with 0.3 + 0.7 * (sin(2*pi*phase) + 1) / 2, start the sound.
// The stall variant (00682ED0) also triggers the "Advanced_Hint_Stall" hint.
void warning_screen_show_alert(Raw screen, int alert, Fe5Host& host);
// The pulse, with the image's float roundings (00682CDA..00682D1A).
float warning_pulse_alpha(float phase) noexcept;
// 00682EC0 __thiscall(screen, vector* out), RET 4: pushes &screen+70h.
void warning_screen_collect_children_00682ec0(Raw screen, void* out, Fe5Host& host);

// 00681F40 __thiscall(GGame*, const float pos[3], float margin), RET 8.
// True when pos is within `margin` of the world rectangle's edge or outside
// it, read straight from the x87 compares: x < [+711Ch]+m, x > [+7128h]-m,
// z > [+7124h]-m, z < [+7130h]+m. Every compare is JA, so a NaN coordinate
// returns false (the decompile's `<=` form would return true). Pure read of
// GGame; the cc9 hosts substitute it (game_hosts_hud.cpp, game_hosts_units.cpp).
bool game_near_world_edge_00681f40(Raw game, const float pos[3], float margin) noexcept;


// ---------------------------------------------------------------------------
// Tree helpers, the front-end manager base and the logo sequence.
// ---------------------------------------------------------------------------
// 00684010 / 00684220 __thiscall(iterator) -> EAX = iterator: 00683D10 then
// return the iterator (byte-identical bodies).
void* tree_iterator_pre_increment(Raw iterator, Fe5Host& host);

// A tree node of the manager registry: left +0h, parent +4h, right +8h,
// key +0Ch (unsigned compare), nil byte +11h; header pointer at tree+4h.
struct TreeEqualRange {
    void* lower_container;
    void* lower_node;
    void* upper_container;
    void* upper_node;
};
// 006845A0 __thiscall(tree, out, const uint32_t* key), RET 8. The image
// writes out as {tree, upper, tree, lower} (out[1] is the first loop's
// result), so lower_node here is out+4h and upper_node out+0Ch.
TreeEqualRange tree_equal_range_006845a0(Raw tree, std::uint32_t key) noexcept;
// 00684EF0 __thiscall(tree, const uint32_t* key), RET 4 -> EAX count.
std::size_t tree_erase_key_00684ef0(Raw tree, std::uint32_t key, Fe5Host& host);

// 006844F0 __thiscall(ref): +0h, +8h, +0Ch, +10h, +18h = 0; byte +14h = 1;
// vtable 00CE3CF0 at +4h (the observer sub-object's base vtable).
void observer_ref_construct_006844f0(Raw ref) noexcept;
inline constexpr std::uint32_t kObserverBaseVtable = 0x00CE3CF0;

// 00684E10 __thiscall(manager) -> EAX: vtable 00CF76D0, two observer
// sub-objects (+4h..+1Ch with vtable at +8h, +20h..+38h with vtable at +24h),
// byte +3Ch = 0, then insert into the registry 00E19898.
// 00684FA0 __thiscall(manager): vtable back, erase from the registry, destroy
// +24h then +8h.
inline constexpr std::uint32_t kFrontEndManagerVtable = 0x00CF76D0;
void* front_end_manager_construct_base_00684e10(Raw manager, Fe5Host& host);
void front_end_manager_destroy_base_00684fa0(Raw manager, Fe5Host& host);

// The logo sequence object (80h, vtable 00CF76E4): fmv strings +40h (begin
// +44h, end +48h, capacity +4Ch), delays +50h (begin +54h, end +58h,
// capacity +5Ch), index +60h, timestamp +68h, +70h = 1, +74h = 0, entry
// delay +78h. Advance 00685070 and the skip poll 00685170 are reconstructed in
// src/title_init.cpp (00685170 backfilled).
inline constexpr std::uint32_t kLogoSequenceVtable = 0x00CF76E4;
inline constexpr std::string_view kLogosScript = "Scripts/datatables/Logos.lua";
inline constexpr std::string_view kLogosTable = "Szplessek";
inline constexpr std::string_view kLogoRegionToken = "(region)";
inline constexpr std::string_view kLogoPlatformToken = "(platform)";
inline constexpr std::string_view kLogoPlatform = "PC";  // 00CF770C
// 00685060 __cdecl(): the movie-complete trampoline, re-enters the advance
// when the logo object exists.
void logo_sequence_on_movie_complete_00685060(Fe5Host& host);
// 006851E0 __thiscall(logo) -> EAX.
void* logo_sequence_construct_006851e0(Raw logo, Fe5Host& host);
// 00685240 __thiscall(logo).
void logo_sequence_destruct_00685240(Raw logo, Fe5Host& host);
// 00685300 __thiscall(logo): read Logos.lua into the two vectors, then
// +60h = 0, 004DD5B0(1) and the first advance.
void logo_sequence_load_table_00685300(Raw logo, Fe5Host& host);
// BSP_String_ReplaceSubstrings with count 7FFFFFFFh: replace every
// non-overlapping occurrence, left to right.
std::string replace_all(std::string text, std::string_view find, std::string_view with);

// ---------------------------------------------------------------------------
// Front-end managers (base 00684E10): main menu, multiplayer menu, options.
// ---------------------------------------------------------------------------
// 00685C80 __thiscall(manager, float seconds), RET 4. Unless the session
// browser (*00E198B8) exists with +40h non-null and that object's +4h byte
// set, feed the music gain and the frame time to stream +50h and, only when
// +50h exists, also to stream +54h.
void interface_update_music_streams_00685c80(Raw manager, float seconds, Fe5Host& host);
// 00686170 __thiscall(menu) -> EAX: base construct, vtable 00CF774C, zero
// +40h..+70h, byte +74h = 0.
inline constexpr std::uint32_t kMainMenuVtable = 0x00CF774C;
void* main_menu_construct_00686170(Raw menu, Fe5Host& host);
// 00686240 __cdecl(): register the "FE" (00CF7768) and "FE_pc" (00CF7760)
// localisation tables.
void main_menu_register_localisation_tables_00686240(Fe5Host& host);
// 00687800 __thiscall(menu, int interface, int argument), RET 8 -> AL.
bool multi_menu_apply_pending_interface_00687800(Raw menu, int interface_id, int argument, Fe5Host& host);
// 006878D0 __thiscall(manager) -> EAX: element count of the dword vector
// +5Ch..+60h (0 when the buffer is null).
std::int32_t interface_request_count_006878d0(Raw manager) noexcept;
// 006878F0 __thiscall(manager): the manager's own +0Ch virtual, then for each
// of the four screens +40h..+4Ch: its +1Ch virtual if active (+5h), clear
// +4h/+5h, commit visibility; finally delete each (+0Ch(1)) and null it.
void front_end_manager_deactivate_and_hide_006878f0(Raw manager, Fe5Host& host);
// 00687A80: a forwarding wrapper, the whole body is CALL 006876D0; RET.
inline constexpr std::uint32_t kAllocatorForward00687a80 = 0x006876D0;
// 00687C00 __cdecl(): delete the singleton at 00F8894C under the lifetime
// manager's lock.
void singleton_destroy_00f8894c_00687c00(Fe5Host& host);
// 006882A0 __thiscall(menu): disable the GUI manager, pop the request, the
// menu's +0Ch virtual, then tail-jump to the in-game interface's +8h virtual.
void multi_menu_close_pause_interface_006882a0(Raw menu, Fe5Host& host);

// 006883B0 __thiscall(dst, const record* src), RET 4 -> EAX dst: assign the
// legacy string at +0h, then copy +20h..+3Bh field by field.
void* online_player_info_assign_006883b0(Raw dst, Raw src, Fe5Host& host);
// 00688780 __thiscall(list): unlink every node of a list of legacy strings
// (node: next +0h, prev +4h, string +8h with buffer/heap pointer +0Ch, size
// +1Ch, capacity +20h), reset each string (free the heap buffer when the
// capacity exceeds 0Fh, capacity 0Fh, size 0, first byte 0) and free it.
void string_list_clear_00688780(Raw list, Fe5Host& host);
// 00688470 __thiscall(vector, iterator* out, int* where, size_t count,
// const uint32_t* value), RET 10h: std::vector<uint32_t>::_Insert_n in a
// checked vector (proxy +0h, first +4h, last +8h, end +0Ch). Growth: capacity
// + capacity/2 (0 when that overflows 3FFFFFFFh), at least size + count.
void dword_vector_insert_n_00688470(Raw vector, std::uint32_t* where, std::size_t count,
                                    std::uint32_t value, Fe5Host& host);
// 006887E0 __thiscall(menu) -> EAX: base construct, vtable 00CF7820,
// +5Ch..+64h = 0 (the request vector).
inline constexpr std::uint32_t kMultiMenuVtable = 0x00CF7820;
void* multi_menu_construct_006887e0(Raw menu, Fe5Host& host);
// 00688B20 __thiscall(menu, int interface), RET 4: push the current set,
// then pop requests until the top is `interface`; pop that one too, or push
// INTF 0Fh when the stack ran empty. Close the online overlay (+90h/+8Ch);
// unless interface is 14h, also leave the online session (+180h/+170h(1)) and
// set the session mode to 0. Byte 00E08700 = 0.
void multi_menu_pop_request_up_to_00688b20(Raw menu, int interface_id, Fe5Host& host);

// 00688AD0 __thiscall(menu, int interface), RET 4: trace, push +20h onto the
// request stack (+58h), then 004CC460(interface, 0).
void multi_menu_push_request_logged_00688ad0(Raw menu, int interface_id, Fe5Host& host);
// 00688C70 __thiscall(menu): pop back to the first of 19h, 12h, 11h, 10h, 0Fh
// that is the current request (+20h) or anywhere on the stack; with none, and
// +20h == 14h while game byte +218Ch is clear, push 0Fh. Then, when the in-game
// interface exists, request game state 10h.
void multi_menu_leave_to_main_interface_00688c70(Raw menu, Fe5Host& host);
// 00689030 __thiscall(menu): collapse the overlays, the interface's +0Ch
// virtual, enable the GUI manager, push +20h, request 17h (INTF_MULTIINGAME)
// and tail-jump to the menu's +8h virtual.
void multi_menu_open_pause_interface_00689030(Raw menu, Fe5Host& host);

// 00688E40 __thiscall(menu): build the four multiplayer screens (+40h main
// F8h, +44h mode selector 100h, +48h lobby 300h, +4Ch session browser
// 2D0h; each gets its +10h virtual), pop back to 14h when it is current or on
// the stack (unless the current request is 1Bh), then the menu's +8h virtual.
struct MultiScreenNew {
    std::size_t slot;
    std::uint32_t size;
    std::uint32_t constructor;
};
inline constexpr std::array<MultiScreenNew, 4> kMultiMenuScreens{{
    {0x40, 0xF8, 0x005E6D90}, {0x44, 0x100, 0x005EA8C0}, {0x48, 0x300, 0x005E3290},
    {0x4C, 0x2D0, 0x00574240}}};
void multi_menu_on_scene_exit_00688e40(Raw menu, Fe5Host& host);
// 006890B0 __thiscall(menu): online error handling. With an online error text
// (and +20h != 1Bh): end the scene, leave to main unless +19Ch, push 1Bh,
// error code 6 or 1 (Xenon byte +128h), and the +8h virtual unless +3Ch. With
// no error while +14h reported offline and game byte +218Ch is clear: error 5,
// push 1Bh unless current, +8h when in game and +3Ch clear. Then mirror the
// session mode into (+54h)->+4h, and when the online +198h reports a lost
// session (and no signed-in user in state 2), pop to 0Fh, set the error code,
// push 1Bh and request game state 10h.
void multi_menu_update_online_interface_006890b0(Raw menu, Fe5Host& host);

// 00689800 __thiscall(menu) -> EAX: base construct, vtable 00CF78C4.
inline constexpr std::uint32_t kOptionsMenuVtable = 0x00CF78C4;
void* options_menu_construct_00689800(Raw menu, Fe5Host& host);

// The attract screen (vtable 00CF78E8, global 00E198BC): page "FE_attract" at
// +40h, idle countdown +44h (reset to 45.0 from 00CE3D60), byte +48h = 1.
// The blocking-screen frame that drives it is src/blocking_screen.cpp.
inline constexpr std::uint32_t kAttractScreenVtable = 0x00CF78E8;
inline constexpr float kAttractIdleSeconds = 45.0f;                      // 00CE3D60
inline constexpr std::string_view kAttractMovie = "movies/PacificTheme.bik";  // 00CF7908
inline constexpr float kAttractMovieZ = -902.0f;                        // 00CEFCB8
// 00689C00 __thiscall(screen): base deactivate, hide the page, +3Ch = 0,
// deactivate the movie player screen, stop the movie, reload the countdown.
void attract_screen_deactivate_00689c00(Raw screen, Fe5Host& host);
// 00689C60 __thiscall(screen): vtable, clear the global, remove the page, base destroy.
void attract_screen_destruct_00689c60(Raw screen, Fe5Host& host);
// 00689D90 __thiscall(screen) -> EAX.
void* attract_screen_construct_00689d90(Raw screen, Fe5Host& host);
// 00689E80 __thiscall(screen) (vtable +8h): base activate, show the page,
// activate the movie player screen (+4h/+5h = 1, commit, its +18h virtual) and
// play the theme movie with (1, -902.0f, loop 1). The plate comment's 1.0f is
// the movie path's flag; the float pushed at 00689EFF is -902.0.
void attract_screen_activate_00689e80(Raw screen, Fe5Host& host);

// ---------------------------------------------------------------------------
// In-game interface helpers (the interface object *00E198C4).
// ---------------------------------------------------------------------------
// 00689FA0 / 00689FC0 / 00689FE0 __cdecl(): push the ship, submarine and
// torpedo-boat input-context levels (the torpedo boat uses the ship list).
struct InputContextLevel {
    std::uint32_t address;
    std::array<std::int32_t, 6> contexts;
};
inline constexpr std::array<InputContextLevel, 3> kUnitInputContexts{{
    {0x00689FA0, {6, 4, 0x11, 0x12, 0x0C, 0x0B}},
    {0x00689FC0, {7, 4, 0x11, 0x12, 0x0C, 0x0B}},
    {0x00689FE0, {6, 4, 0x11, 0x12, 0x0C, 0x0B}},
}};
void set_unit_input_contexts(const InputContextLevel& level, Fe5Host& host);
// 0068A120 __thiscall(interface) -> AL: screen +8Ch exists and is shown (+4h).
bool ingame_interface_screen_8c_shown_0068a120(Raw iface) noexcept;
// 0068A160 / 0068A1F0 __thiscall(interface): when the spectate screen (+98h
// movie camera, +9Ch movie) is hidden, engage it on the controlled unit;
// otherwise take its camera's target (kind 2; a kind-0Fh unit forwards to its
// +9D4h) and, when selectable, make it the HUD root's spectated unit, else
// force a selection. 0068A1F0 returns early when the screen has no camera.
struct SpectateScreenRoutines {
    std::uint32_t address;
    std::size_t screen_offset;
    std::uint32_t engage;
    std::uint32_t camera;
    std::uint32_t target;
    bool null_camera_returns;
};
inline constexpr std::array<SpectateScreenRoutines, 2> kSpectateScreens{{
    {0x0068A160, 0x98, 0x005CD0F0, 0x005CD160, 0x0078E470, false},
    {0x0068A1F0, 0x9C, 0x005CD1A0, 0x005CD240, 0x00798C40, true},
}};
void ingame_interface_update_spectate(const SpectateScreenRoutines& r, Raw iface, Fe5Host& host);

// ---------------------------------------------------------------------------
// Small HUD screens built by the in-game interface: every constructor is the
// front-end screen base 004F7180, a callback-owner sub-object at +8h (base
// vtable 00CE3CD4 written first, then the screen's secondary vtable), a row
// of zeroed fields and the primary vtable. Every destructor (no vtable store)
// is 00695870 on +8h then 004F71A0; 0068A910 first destroys the observer link
// at +1Ch (004B7EF0).
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kCallbackOwnerBaseVtable = 0x00CE3CD4;
inline constexpr std::uint32_t kObserverLinkBaseVtable = 0x00CE75CC;
struct SmallHudScreenShape {
    std::uint32_t construct;
    std::uint32_t destruct;      // 0 when not in this segment's list
    std::uint32_t primary_vtable;
    std::uint32_t secondary_vtable;
    std::array<std::uint16_t, 9> zero_dwords;  // after +0Ch/+10h/+14h; 0 ends the row
    std::array<std::uint16_t, 2> zero_bytes;   // after +18h; 0 ends the row
    bool observer_link_1c;                     // +1Ch link: vtable 00CE75CC, +2Ch = 1
    std::string_view role;
};
inline constexpr std::array<SmallHudScreenShape, 6> kSmallHudScreens{{
    {0x0068A2D0, 0x0068A330, 0x00CF7938, 0x00CF7920, {0x34, 0x4C, 0x50, 0x40, 0x44, 0x54}, {0x48}, false, "screen 24h"},
    {0x0068A3B0, 0x0068A420, 0x00CF7978, 0x00CF7960, {0x1C, 0x20, 0x24, 0x28, 0x34}, {0x30}, false, "screen 46h"},
    {0x0068A4A0, 0x0068A4F0, 0x00CF79B8, 0x00CF79A0, {0x1C, 0x20}, {}, false, "screen 2Fh"},
    {0x0068A570, 0, 0x00CEC590, 0x00CEC578, {}, {}, false, "bomber view screen"},
    {0x0068A5A0, 0x0068A5F0, 0x00CF79F8, 0x00CF79E0, {}, {}, false, "binoculars overlay 4Ah"},
    {0x0068A8A0, 0x0068A910, 0x00CF7A38, 0x00CF7A20, {}, {}, true, "common overlay 27h"},
}};
// __thiscall(screen) -> EAX.
void* small_hud_screen_construct(const SmallHudScreenShape& shape, Raw screen, Fe5Host& host);
// __thiscall(screen).
void small_hud_screen_destruct(const SmallHudScreenShape& shape, Raw screen, Fe5Host& host);

// 0068A9C0 __thiscall(interface) -> AL. When the game scene is a kind-54h
// object the pending byte +FDh is dropped. With +FDh set: when the current
// set (+4h, +1Ch) already equals the pending one (+20h, +38h) push 34h;
// when it differs, the interface's +10h(+20h, +38h) virtual. Clears +FDh and
// returns 1.
bool ingame_interface_apply_pending_level_change_0068a9c0(Raw iface, Fe5Host& host);
// 0068AA90 __thiscall(interface): when screen +54h is shown, clear the
// level-2 screens and contexts and request the scene for the controlled vehicle.
void ingame_interface_request_scene_for_screen54_0068aa90(Raw iface, Fe5Host& host);
// 0068AAD0 __thiscall(interface): the controlled unit (*00E188D8) of kind 9
// or 45h pushes request 32h, of kind 46h pushes 31h (argument = the unit).
void ingame_interface_push_controlled_unit_request_0068aad0(Raw iface, Fe5Host& host);
// 0068AB30 __thiscall(interface): on request 31h/32h with no pending change,
// push 20h (argument = the unit), then the +10h virtual when a change is pending.
void ingame_interface_leave_unit_request_level_0068ab30(Raw iface, Fe5Host& host);
// 0068AC30 __thiscall(interface): with byte 00E08880 clear, set the sound
// request queue's +1Ch = 1 and return. Otherwise, when screen +54h is hidden:
// leave the unit level, copy HUD root +1Ch to +ECh and the controlled unit to
// +F0h, set level-2 screens {4Ch, 39h, 44h, 29h, 49h} and contexts {0Eh, 14h,
// 0Ch, 11h}.
inline constexpr std::array<std::int32_t, 6> kHudLevel2Screens{0x4C, 0x39, 0x44, 0x29, 0x49, 0};
inline constexpr std::array<std::int32_t, 5> kHudLevel2Contexts{0x0E, 0x14, 0x0C, 0x11, 0};
void ingame_interface_restore_hud_screen_set_0068ac30(Raw iface, Fe5Host& host);
// 0068B3F0 __thiscall(interface): while 006529E0 on the support-manager
// screen (+BCh) reports true: clear its +30h, and when the global vector 00E18D1C is non-empty
// (the +30h re-read is after the store, so the +0Ah write never runs), clear
// level 3 and request the scene for the controlled vehicle.
void ingame_interface_collapse_level3_when_spectating_0068b3f0(Raw iface, Fe5Host& host);

// The level-3 collapse block repeated in 0068B3F0 (0068B40E), 0068B470,
// 0068C0B0 and in 0068C9ED / 0068CA68: when the global vector 00E18D1C is
// non-empty, set (+BCh)+0Ah when (*00E198C4)->+BCh->+30h is set, clear the
// level-3 screens and contexts and request the scene for the controlled
// vehicle.
void collapse_level3(Raw iface, Fe5Host& host);
// 0068B470 __thiscall(interface): collapse level 3, reset the binoculars,
// then restore the HUD set (screen +54h hidden) or clear level 2 and request
// the scene for the controlled vehicle.
void ingame_interface_back_out_one_overlay_level_0068b470(Raw iface, Fe5Host& host);
// 0068C0B0 __thiscall(interface, void* target), RET 4: open the support
// overlay. Without available entries: collapse level 3 when 006529E0 holds,
// else play the unavailable sound. With entries: 00652A30 true plays the
// unavailable sound; 006529E0 true collapses level 3; otherwise build the
// tactical list and, unless (+64h)+82h or +81h is set, set level-3 screens
// {4Eh, 50h} and contexts {14h, 12h, 4, 0Bh, 6, 0Ah} (game byte +19C4h clear)
// or {14h, 0Eh}, then set the tracked target. A failed build plays the sound.
inline constexpr std::array<std::int32_t, 3> kSupportLevel3Screens{0x4E, 0x50, 0};
inline constexpr std::array<std::int32_t, 7> kSupportLevel3Contexts{0x14, 0x12, 4, 0x0B, 6, 0x0A, 0};
inline constexpr std::array<std::int32_t, 3> kSupportLevel3ContextsAlt{0x14, 0x0E, 0};
void ingame_interface_toggle_tactical_overlay_0068c0b0(Raw iface, void* target, Fe5Host& host);

// 0068BF60 __thiscall(screen) -> EAX: the HUD root screen 44h (vtables
// 00CF5A9C / 00CF5A84; the rest of the screen is cc13-fe2's). Front-end
// base, callback owner at +8h, zeroes +70h/+74h and the triples +90h, +A0h,
// +B0h, byte +C2h = 0, word +C4h = FFFFh, an observer sub-object (vtable
// 00CE3CF0 at +D0h; +D4h..+DCh, +E4h = 0, byte +E0h = 1) and seven
// vector triples at +108h + 10h*k.
inline constexpr std::uint32_t kHudRootVtable = 0x00CF5A9C;
inline constexpr std::uint32_t kHudRootSecondaryVtable = 0x00CF5A84;
void* hud_root_screen_construct_0068bf60(Raw screen, Fe5Host& host);

// The title screen (vtable 00CF7A98, global 00E198C8): front-end manager with
// its screen object at +40h.
inline constexpr std::uint32_t kTitleScreenVtable = 0x00CF7A98;
// 0068D760 __thiscall(title) -> EAX.
void* title_screen_construct_0068d760(Raw title, Fe5Host& host);
// 0068D7B0 __thiscall(title): base activate, show screen +40h (+4h/+5h = 1,
// commit) and tail-jump to its +18h virtual.
void title_screen_activate_and_show_0068d7b0(Raw title, Fe5Host& host);
// 0068D7E0 __thiscall(title): vtable, clear the global, delete +40h (+0Ch(1)),
// base destroy.
void title_screen_destruct_0068d7e0(Raw title, Fe5Host& host);
// 0068D8A0 __cdecl(): reset the storage availability; when game +5E8h is 0,
// request game state 4 and clear game byte +5ECh.
void title_screen_skip_0068d8a0(Fe5Host& host);

// A widget blinker: widget +0h, active byte +4h, timer +8h, period +0Ch.
inline constexpr float kBlinkerPeriod = 0.6000000238418579f;  // 00CE3D30
inline constexpr double kBlinkerHalf = 0.5;                   // 00D7A280
// 0068D9B0 __thiscall(blinker): inactive, hide the widget, timer 0, period 0.6.
void gui_blinker_stop_0068d9b0(Raw blinker, Fe5Host& host);
// 0068DA10 __thiscall(blinker, float dt), RET 4: while active, timer -= dt
// (rounded to float); when 0 >= timer (FCOMIP/JB, so a NaN timer never
// toggles) flip the widget's visibility and add period * 0.5.
void gui_blinker_update_0068da10(Raw blinker, float dt, Fe5Host& host);
// 0068DB40 __thiscall(widget) -> EAX: the widget when its +0Ch(00AB1060())
// kind test passes, else null (null in, null out).
void* gui_widget_as_kind_0068db40(void* widget, Fe5Host& host);
// 0068DB80 __thiscall(icon, float x, float y), RET 8: 00AA7DC0(&{x, y, 0})
// on the widget at +20h (SSE stores).
void unit_doing_icon_set_local_position_0068db80(Raw icon, float x, float y, Fe5Host& host);
// 0068DC50 __thiscall(icon, float s), RET 4: the +20h widget's +48h(&{s, s}).
void unit_doing_icon_set_widget_scale_0068dc50(Raw icon, float scale, Fe5Host& host);
// 0068DC80 __thiscall(icon): show Doing_Icon (+24h) with the unit task's
// image when the icon is forced (+14h) or the session mode is not 2, a unit is
// observed, it is not a squadron flight leader (007B8AD0) and it has a task
// (+DF4h); the image is ((task+58h)->first)->+10h() when task+5Ch is non-zero,
// else null. Otherwise hide it.
void unit_doing_icon_update_0068dc80(Raw icon, Fe5Host& host);
// 0068DDE0 __thiscall(icon, widget), RET 4: bind Doing_Icon (+24h) and
// Ammo_Icon (+5Ch) under the widget (+20h), clear +14h, hide the widget.
void unit_doing_icon_bind_0068dde0(Raw icon, void* widget, Fe5Host& host);
// 0068DF60 __stdcall(parent, const char* name, void** out), RET 0Ch:
// *out = parent's child named name + "_Icon".
void gui_find_icon_child_0068df60(void* parent, const char* name, void** out, Fe5Host& host);

// 0060C870 __thiscall(screen), RET (the plane-spawn screen 41h enter): the
// whole body is MOV EAX,[00E198C4]; MOV ECX,[EAX+C4h]; JMP 00681E90, i.e.
// show the "wait" line on the interface's warning screen.
void plane_spawn_screen_enter_0060c870(Fe5Host& host);

// 0068DAA0 __thiscall(icon, unit), RET 4: re-point the observed unit +18h
// (unregister the old pair, register the new one; ECX = the unit, EDX =
// icon+14h) and copy unit+538h to +1Ch.
void unit_doing_icon_set_unit_0068daa0(Raw icon, void* unit, Fe5Host& host);
// 0068DAD0 __thiscall(icon, value), RET 4: unregister the observed unit when
// set, then +1Ch = value, +18h = 0.
void unit_doing_icon_clear_unit_0068dad0(Raw icon, std::uint32_t value, Fe5Host& host);

// ---------------------------------------------------------------------------
// The award tracker's hint queue (src/gui_screens_3.cpp drives it): stored
// list +48h (head +4Ch), recent list +54h (head +58h), active hint name +60h,
// pending deque of native strings +68h (map +6Ch, map size +70h, offset +74h,
// size +78h; two 8-byte strings per block), definitions map +30h, alias map
// +7Ch.
// ---------------------------------------------------------------------------
// 0068E420 __thiscall(tracker): resize the active name to 0 (flag 0) and copy
// the empty string 00CE3A0C into it.
void award_tracker_finish_active_hint_0068e420(Raw tracker, Fe5Host& host);
// 0068E5B0 __thiscall(tracker, string* out), RET 4 -> EAX out: copy the
// active name into a fresh string.
std::string award_tracker_copy_active_hint_name_0068e5b0(Raw tracker, Fe5Host& host);
// 0068E600 __thiscall(tracker, const string* name), RET 4 -> AL: the stored
// set, else a case-insensitive walk of the list at +48h (equal sizes, and
// both empty or _stricmp == 0).
bool award_tracker_has_hint_been_shown_0068e600(Raw tracker, const std::string& name, Fe5Host& host);
// Size-then-_stricmp equality used by 0068E600 and 0068EA70.
bool native_name_equal(const std::string& a, const std::string& b) noexcept;
// 0068E780 __thiscall(deque): pop the front native string (release its
// block, advance the offset modulo 2 * map size, size - 1, offset 0 when empty).
void native_string_deque_pop_front_0068e780(Raw deque, Fe5Host& host);
// 0068E850 __thiscall(list, const value*), RET 4: push a copy at the end.
void alias_list_insert_copy_0068e850(Raw list, const void* value, Fe5Host& host);
// 0068EA00 __thiscall(tracker): clear the stored list, then pop every pending
// hint (the pop is inlined from 0068E780 on tracker+68h).
void award_tracker_clear_hint_queue_0068ea00(Raw tracker, Fe5Host& host);
// 0068EA70 __thiscall(tracker, string name by value), RET 8: erase the first
// recent-list node equal to name, push name at the end, release the argument.
void award_tracker_touch_recent_hint_0068ea70(Raw tracker, Raw name, Fe5Host& host);
// 00690B00 __thiscall(tracker, const string* name), RET 4 -> AL: an empty
// name is not blocking; else the definition's +114h byte.
bool award_tracker_is_active_hint_blocking_00690b00(Raw tracker, Raw name, Fe5Host& host);
// 00690FB0 __thiscall(tracker, key, value), RET 8.
void award_tracker_hint_map_pair_00690fb0(Raw tracker, std::uint32_t key, std::uint32_t value, Fe5Host& host);
// 00692FB0 __thiscall(tracker, name), RET 4: +4h = 1, then 00690FD0(name, 1).
void award_tracker_trigger_forced_hint_00692fb0(Raw tracker, std::uint32_t name, Fe5Host& host);

// ---------------------------------------------------------------------------
// The engine's dword vector {data +0h, size +4h, capacity +8h} (not the STL
// one). Every element store is guarded by a null test on the slot address,
// as in the image (operator new never returns a block at address 0 here, so
// the guard only matters for a zero-length allocation).
// ---------------------------------------------------------------------------
// 006938D0 __thiscall(dst, const vector* src), RET 4 -> EAX dst.
void* native_dword_vector_copy_construct_006938d0(Raw dst, Raw src, Fe5Host& host);
// 00693990 __thiscall(vector): capacity = 2 * capacity + 2, move the elements.
void native_dword_vector_grow_00693990(Raw vector, Fe5Host& host);
// 00693FD0 __thiscall(vector, uint32_t size), RET 4: grow to exactly `size`
// when needed, zero the new tail, set the size (also when shrinking).
void native_dword_vector_resize_00693fd0(Raw vector, std::uint32_t size, Fe5Host& host);
// 00694060 __thiscall(dst, const vector* src), RET 4: assign. The size is
// zeroed before the reallocation, so the copy loop of the old elements never
// runs; capacity becomes the source size when it was smaller.
void native_dword_vector_assign_00694060(Raw dst, Raw src, Fe5Host& host);

// ---------------------------------------------------------------------------
// Observer bookkeeping (pair record: vtable +0h, observed +4h, observer +8h,
// registration count +0Ch). Every locked routine takes the lock owner's
// critical section (EnterCriticalSection then ++[cs+18h]; --[cs+18h] then
// LeaveCriticalSection), skipping both when the owner has none.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818;
inline constexpr std::uint32_t kObserverLockOwnerVtable = 0x00CF7E70;
// 00694250 __thiscall(owner, flags), RET 4 -> EAX: clear 00E198E0, base
// vtable 00CE3818, _free when bit 0.
void* observer_singleton_scalar_delete_00694250(Raw owner, std::uint8_t flags, Fe5Host& host);
// 00694B90 register call (ECX = observed, EDX = observer) -> EAX: the pair's
// registration count (+0Ch) under the lock, 0 when there is no pair.
std::uint32_t observer_find_pair_value_locked_00694b90(void* observed, void* observer, Fe5Host& host);
// 00694E70 __thiscall(owner): vtable 00CF7E70, destroy the owned critical
// section (+4h), clear 00E198E0, base vtable.
void observer_lock_owner_destruct_00694e70(Raw owner, Fe5Host& host);
// 00694EE0 / 00694F20 __thiscall(ref) -> EAX: +8h read under the lock
// (byte-identical bodies).
std::uint32_t observer_ref_get_target_locked(Raw ref, Fe5Host& host);
// 00695000 __thiscall(pair): erase the pair from both objects' edge arrays and
// delete it (+0h(1)). 00695030 does the same once its count drops to zero.
void observer_pair_destroy_00695000(Raw pair, Fe5Host& host);
void observer_pair_release_00695030(Raw pair, Fe5Host& host);
// 00695230 __cdecl(first, last, dest) -> EAX dest + bytes: memmove_s of the
// dword range.
void* move_dwords_00695230(const void* first, const void* last, void* dest, Fe5Host& host);
// 006956A0 register call (ECX = observed, EDX = observer): unregister under
// the lock when registered.
void observer_unregister_if_registered_006956a0(void* observed, void* observer, Fe5Host& host);
// 00695830 __thiscall(owner): read +8h under the lock, then tail-jump to
// 00695530 when it was non-null.
void callback_owner_detach_all_if_bound_00695830(Raw owner, Fe5Host& host);
// 00695E40 __cdecl(): 00695C50(100h) under the lock.
void observer_purge_locked_00695e40(Fe5Host& host);

// 0068A0D0 __cdecl(): update game+19F0h with (+19FCh, real clock *00F876A4),
// submit the main scene (+19ECh, +19FCh) to the scene renderer, then
// tail-jump to 004C7E60 on the game.
void game_submit_main_scene_render_0068a0d0(Fe5Host& host);

// ---------------------------------------------------------------------------
// Segment 24: plane HUD (3Eh, vtable 00CF4544), plane effects (3Fh, vtable
// 00CF43AC) and plane spawn (41h, vtable 00CF4A7C) screens.
// ---------------------------------------------------------------------------
// 00604BC0 __thiscall(screen, bool visible), RET 4: +B4h, +B8h, +0Ch.
void plane_effects_set_visible_00604bc0(Raw screen, bool visible, Fe5Host& host);
// 00604C20 __thiscall(screen): hide +88h and the 6x5 indicator grid at +10h
// (null slots skipped).
void plane_screen_hide_indicator_grid_00604c20(Raw screen, Fe5Host& host);
// 00604DF0 __thiscall(screen): clear +104h; with a plane (+B8h): release its
// role mask (1FEh, take 0), or, when the interface's screen 8Ch is shown,
// 1FFh/0 then 1/1; the plane's +1C8h virtual; near plane 1.5 (00CE380C).
// The role transfers are cc9's gameplay messages (listed in the doc).
inline constexpr float kPlaneReleaseNearPlane = 1.5f;  // 00CE380C
void plane_screen_release_plane_view_00604df0(Raw screen, Fe5Host& host);
// 00604E70 / 00604EC0 __thiscall(screen, object), RET 4: null the first of
// +B0h, +8Ch, +90h that holds the object (byte-identical bodies).
void plane_screen_on_object_released(Raw screen, const void* object) noexcept;
// 00604F30 __thiscall(screen) -> EAX: 0 without a plane or when it is of kind
// 10h or 16h, else +FCh.
std::uint32_t plane_screen_cockpit_view_flag_00604f30(Raw screen, Fe5Host& host);
// 00604FC0 __thiscall(cell): _free(*cell) and null it when set.
void owned_buffer_free_00604fc0(Raw cell, Fe5Host& host);
// 00604FE0 / 00605000 __thiscall(cell): delete *cell (+0h(1)) and null it
// when set (byte-identical bodies).
void owned_object_delete(Raw cell, Fe5Host& host);

// 00605270 / 006052B0 / 006052E0 / 00605300 / 00605320: session message C4h
// (the plane bomb-fire message sent by 006082D0), the fe2b model of
// include/bsp/ingame_gui_cc13.hpp with this row: base 0075B430(C4h), +18h
// word 0, +1Ah byte 0, +1Ch byte 0, +4h = 1, vtable 00CF4378; IsType accepts
// C4h, 49h and 46h; the payload is one bool at +1Ch. Its read passes ECX =
// cursor+4h to 00428D70 (as does fe2's 006503C0; the shared model's
// read_bool host call stands for that).
inline constexpr ingame_gui_cc13::SessionMessageClass kSessionMessageC4{
    0xC4, {0x49, 0x46}, 2, 0x00CF4378, ingame_gui_cc13::SessionCtorShape::Clear1C,
    0x00605270, 0x006052B0, 0x006052E0, 0x00605300, 0x00605320,
    {{{0x1C, ingame_gui_cc13::SessionFieldKind::Bool, 0}}}, 1};

// 00605380 __thiscall(screen): payload frame from 00604C80; a frame other
// than -1 goes to +110h (+88h(frame, 0, 1.0)) and shows +100h, +104h, +110h,
// else all three are hidden.
void plane_screen_update_payload_icon_00605380(Raw screen, Fe5Host& host);
// 00605410 __thiscall(needle, float v), RET 4. Needle: scale +0h, widget +4h,
// offset +10h, span +14h, maximum +18h, value +1Ch. value = max > v ? v : max
// (FCOMIP/JBE: a NaN v takes the maximum); angle = float(value / max * scale
// * span + offset) wrapped by fmod(angle, 2*pi) (00CE3828) and folded into
// (-pi, pi] (-pi from 00CE3D18 adds 2*pi when -pi >= a or unordered; above pi
// (00CE3D28) subtracts 2*pi); the widget's +44h gets the result.
inline constexpr double kNeedlePi = 3.1415927410125732;        // 00CE3D28 / -00CE3D18
inline constexpr double kNeedleTwoPi = 6.2831854820251465;     // 00CE3828
void gauge_needle_set_value_00605410(Raw needle, float value, Fe5Host& host);
// 00605600 __thiscall(cell): drop one reference of *cell (InterlockedDecrement
// of +4h, delete through vtable slot 0 at zero) and null it.
void refcounted_ptr_release_00605600(Raw cell, Fe5Host& host);

// 00605CC0 __thiscall(screen) (plane effects exit): clear +104h and +FCh,
// release the plane view (00604DF0), clear the two flash records +E8h/+ECh
// (byte +0h, dwords +4h..+10h), fov 1.0 flag 2, the plane's +1C8h virtual,
// near plane 1.5, stop and drop the point effects +64h, +6Ch, +70h (byte
// +9h = 1 first), and restore the render transition (+88h, +8Ch, +90h, 0, 0, 0).
void plane_effects_exit_00605cc0(Raw screen, Fe5Host& host);
// 00605E50 __thiscall(screen): create the chase camera at +94h once (3FCh
// zeroed block, 00605130, added to the scene list game+19CCh with an
// identity matrix, observed through screen+8h), then target the plane
// (+B8h), make it the camera mover, its +DCh(0.0), +98h = 0, the plane's
// +1C8h virtual, near plane 1.5.
void plane_screen_ensure_chase_camera_00605e50(Raw screen, Fe5Host& host);
// 00606000 / 00606020 __thiscall(unit) -> EAX: the unit when it is of kind
// 25h / 26h, else null.
void* unit_as_kind_or_null(void* unit, int kind, Fe5Host& host);
// 00606250 __stdcall(int action), RET 4 -> AL: byte +1h of the input
// manager's action row.
bool input_action_enabled_00606250(std::int32_t action, Fe5Host& host);

inline constexpr std::uint32_t kPlaneEffectsVtable = 0x00CF43AC;
inline constexpr std::uint32_t kPlaneEffectsSecondaryVtable = 0x00CF4394;
// 00606040 __thiscall(screen): vtables back; stop the children of the point
// effects +64h, +6Ch, +70h (byte +9h = 1, no release yet); _free the flash
// records +E8h/+ECh; destroy the observer at +D0h; drop the references
// +78h, +74h, +70h, +6Ch, +68h, +64h, +60h in that order; callback owner +8h;
// front-end base.
void plane_effects_destruct_00606040(Raw screen, Fe5Host& host);
// 00606470 __thiscall(screen) -> EAX: front-end base, callback owner, the
// reference slots +60h..+78h, +94h, +98h, +B8h = 0, byte +BDh = 0, +C4h,
// +C8h = 0, the observer at +CCh (vtable at +D0h), two zeroed 14h-byte flash
// records at +E8h/+ECh, bytes +104h, +10Ch = 0, +108h = 0.
void* plane_effects_construct_00606470(Raw screen, Fe5Host& host);
// 006068A0 __thiscall(screen): +100h = tuning +84h when a plane of neither
// kind 10h nor 16h is in cockpit view (+FCh), else 1.0; fov(+100h, 2).
void plane_screen_update_view_fov_006068a0(Raw screen, Fe5Host& host);
// 00606920 __thiscall(vector): _free(first) when set, first/last/end = 0.
void stl_vector_tidy_00606920(Raw vector, Fe5Host& host);

// The plane HUD screen 3Eh (vtable 00CF4544).
inline constexpr std::uint32_t kPlaneScreenVtable = 0x00CF4544;
inline constexpr std::string_view kPlaneRumbleEffect = "PlaneRumble";
inline constexpr double kPlaneScreenLineOffset = 0.006944444444444444;  // 00CF4570 (1/144)
// 00606950 __thiscall(screen): vtable; remove the pages +0Ch, +B8h, +B4h;
// drop +15Ch then +158h; free the vectors +A8h and +98h; front-end base.
void plane_screen_destruct_00606950(Raw screen, Fe5Host& host);
// 00606A90 __thiscall(screen) (vtable +10h): front-end register; clear
// +120h..+12Ch; acquire "PlaneRumble" into +158h (reference-counted swap);
// drop +15Ch; byte +160h = 0, +164h = 0.0; the layout virtual (+14h);
// +114h = float(local y of widget +C8h + 1/144); +134h = +13Ch = 0.0, bytes
// +8h, +130h, +140h = 0, +138h = 1.0; widget +E0h alpha 0.0. The decompile
// misplaces the +13Ch store as +13Ch = param[4Fh]; the listing stores
// floats at +13Ch and +134h (SSE).
void plane_screen_register_00606a90(Raw screen, Fe5Host& host);

// 00608280 __thiscall(screen): cockpit camera when a plane of neither kind
// 10h nor 16h is in cockpit view (+FCh), else the chase camera.
void plane_screen_apply_view_mode_00608280(Raw screen, Fe5Host& host);
// 006088A0 __thiscall(cell): free an owned STL vector object (its first
// +4h, then the 10h block) and null the cell.
void owned_vector_delete_006088a0(Raw cell, Fe5Host& host);
// 00608B60 __thiscall(screen): clear +104h and +FCh first, so the camera test
// always falls to the chase camera; with a plane, observe it through
// screen+8h (once) and centre the markers view at (0.5, 0.5) (00CE3800).
void plane_screen_select_camera_00608b60(Raw screen, Fe5Host& host);
// 00608C00 __thiscall(object, flag), RET 4 -> AL: test a flag in the bit set
// at +C4h.
bool bit_flag_set_test_flag_00608c00(Raw object, std::uint32_t flag, Fe5Host& host);
// 00608E60 __thiscall(screen) -> EAX: front-end base, vtable 00CF4544, the
// vectors +98h and +A8h and the references +158h/+15Ch zeroed, the quad
// fade at +16Ch, +10Ch = +108h = 0.
void* plane_screen_construct_00608e60(Raw screen, Fe5Host& host);
// 00608EF0 __thiscall(screen, vector* out), RET 4: pushes &+B4h, &+0Ch, &+B8h.
void plane_screen_collect_children_00608ef0(Raw screen, void* out, Fe5Host& host);
// 00609390 __thiscall(screen, plane), RET 4: +168h = plane; when the screen
// is active (+5h), rebuild the widgets, clear the two quad records' +18h
// bytes (+108h, +10Ch) and refresh the payload icon.
void plane_screen_set_plane_00609390(Raw screen, void* plane, Fe5Host& host);
// @decl-end

}  // namespace bsp::cc13_fe5

#endif  // BSP_FRONTEND_SEG16_24_28_CC13_HPP
