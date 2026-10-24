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
#include <utility>

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
    // helpers, e.g. 006876D0; BSP_LegacySboString_AllocateBytes for bytes).
    virtual void* stl_allocate_bytes(std::size_t bytes) = 0;
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
    // reconstructed here (00607F60 and 00608F30 are reconstructed below by
    // cc13-fe7).
    // 00637620(x, y), ECX = (*00E198C4)+48h (the markers screen).
    virtual void markers_screen_set_view_centre_00637620(float x, float y) = 0;
    // 0048F930(&tmp, flag) on object+C4h, then 0048F230 on the result -> AL.
    virtual bool bit_flag_set_test_0048f930(Raw flags, std::uint32_t flag) = 0;
    // 0064A4F0, ECX = screen+16Ch: the widget quad-fade constructor.
    virtual void widget_quad_fade_construct_0064a4f0(Raw fade) = 0;
    // 006BCD20, ECX = the slot's +28h object: its air-ops block.
    virtual void* air_ops_block_006bcd20(Raw object) = 0;
    // 00432750, ECX = camera: the camera-mover base constructor.
    virtual void camera_mover_construct_base_00432750(Raw camera) = 0;
    // 00B0CFC0(&a, &b, &c), ECX = *00F8D39C: read the render transition.
    virtual void render_get_transition_00b0cfc0(float& a, float& b, float& c) = 0;
    // 0042AA00(0, 0, 0), ECX = the cockpit camera (+98h).
    virtual void cockpit_camera_reset_0042aa00(Raw camera) = 0;
    // 0042C940(float yaw), ECX = camera mover: orbit the target from a yaw.
    virtual void camera_orbit_target_from_yaw_0042c940(Raw camera, float yaw) = 0;
    // The function-local static of 00545750: the colour 00E192F4..00E19300
    // and its guard dword 00E19304 (bit 0), returned as one 14h-byte block.
    virtual Raw static_colour_block_00e192f4() = 0;
    // 00AA8B80(index), ECX = widget: drop a timed entry.
    virtual void widget_delete_timed_entry_00aa8b80(void* widget, int index) = 0;
    // 005471B0(count), ECX = array: resize a reference-counted pointer array.
    virtual void refptr_array_resize_005471b0(Raw array, std::int32_t count) = 0;
    // 00548410(forward, 0), ECX = screen 2Eh: cc9's group step
    // (src/hud_weapon_group_screen.cpp weapon_group_cycle_00548410).
    virtual void weapon_group_step_00548410(Raw screen, int forward) = 0;
    // 00545B30(group), ECX = screen 2Eh (cc9's weapon_group_any_available_00545b30).
    virtual bool weapon_group_any_available_00545b30(Raw screen, std::int32_t group) = 0;
    // Manager globals: *00E198AC main menu, *00E198B4 multiplayer menu,
    // *00E198B8 session browser, *00E198C4 in-game interface (null when absent).
    virtual Raw manager_global(std::uint32_t address) = 0;
    // Widget +5Ch() -> EAX: the widget kind; +84h(1): press.
    virtual std::int32_t widget_kind_5c(void* widget) = 0;
    virtual void widget_press_84(void* widget, int flag) = 0;
    // A listener sub-object's virtual (ECX = listener, (widget)).
    virtual void listener_forward(Raw listener, std::uint16_t slot, void* widget) = 0;
    // A listener sub-object's virtual (ECX = listener, (widget, flag)).
    virtual void listener_forward2(Raw listener, std::uint16_t slot, void* widget, std::uint32_t flag) = 0;
    // 0054AA70(c), ECX = string: BSP_NativeString_AppendByte.
    virtual void native_string_append_byte(Raw string, std::uint8_t value) = 0;
    // 00A3EAC0, ECX = *00F8ABE8: the selected user's index.
    virtual std::uint32_t xenon_selected_user_index_00a3eac0() = 0;
    // 00A4D43A: XShowKeyboardUI(user, flags, default, title, description,
    // result, result_chars, overlapped) (an XLive import thunk).
    virtual void x_show_keyboard_ui_00a4d43a(std::uint32_t user, std::uint32_t flags, const void* default_text,
                                              const void* title, const void* description, void* result,
                                              std::uint32_t result_chars, void* overlapped) = 0;
    // The scroller (src/frontend_screen_animation.cpp): 00683300(0, 0, 0)
    // and the thumb-drag end 00683320, ECX = the scroller.
    virtual void scroller_set_auto_scroll_00683300(Raw scroller, int direction, float step, bool override_input) = 0;
    virtual void scroller_end_thumb_drag(Raw scroller) = 0;
    // Widget +88h(bool, 0, 1.0) on a kind-6 widget.
    virtual void widget_set_state_88(void* widget, bool on, int flag, float alpha) = 0;
    // Tree-iterator increments: 00552770, 00552D40, 00552E20 (ECX = iterator).
    virtual void tree_iterator_increment(Raw iterator, std::uint32_t routine) = 0;
    // 0044BCB0, ECX = the slot, EDX = src: copy-construct a native string.
    virtual void native_string_construct_copy_0044bcb0(Raw slot, Raw src) = 0;
    // 00AA78F0(float y), ECX = widget: set the local y.
    virtual void widget_set_local_y_00aa78f0(void* widget, float y) = 0;
    // operator new(1Ch) then 0043DFF0(a, b, c) / 0043DF90(a, b), ECX = the
    // block: the two HUD widget-set records (null when the allocation failed).
    virtual void* hud_widget_set_new_0043dff0(std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    virtual void* hud_widget_set_new_0043df90(std::uint32_t a, std::uint32_t b) = 0;
    // 00ABAED0(const string* source, 1), ECX = text widget: set the localised
    // source from a native string.
    virtual void text_set_localised_source_00abaed0(void* text, Raw source) = 0;
    // 00AA6750(&out) / 00AA6740 -> size pair / 00AA8240(const xyz*), ECX =
    // widget: resolved position, size, set resolved position.
    virtual void widget_resolved_position_00aa6750(void* widget, float xyz[3]) = 0;
    virtual const float* widget_size_00aa6740(void* widget) = 0;
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float xyz[3]) = 0;
    // Widget +58h(const float size[2]): set size.
    virtual void widget_set_size_58(void* widget, const float size[2]) = 0;
    // 00870CD0(&out, 1) with EDX = effect id: acquire an effect handle by id
    // (one reference for the caller).
    virtual void* effect_handle_acquire_id_00870cd0(std::uint32_t id) = 0;
    // --- cc13-fe7 additions ---
    // 00AAB4C0(flag), ECX = template widget -> a cloned widget subtree.
    virtual void* gui_widget_clone_subtree_00aab4c0(void* widget, int flag) = 0;
    // The out-of-line insert of a std::vector<widget*> (the image passes a
    // scratch iterator, the vector, the end position and &value):
    // 0043DB60 for screen+94h, 0052EB90 for screen+A4h.
    virtual void widget_vector_insert_one(Raw vector, std::uint32_t routine, void* where, void* value) = 0;
    // 004D92B0(action), ECX = *00E188A8 -> AL: action edge or auto-repeat.
    virtual bool front_end_action_edge_or_repeat_004d92b0(int action) = 0;
    // 004C5090(action), ECX = *00E188A8 -> AL: action held.
    virtual bool input_action_held_004c5090(int action) = 0;
    // 00A9C920, ECX = listbox -> selected row.
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    // Lua objects (handles; one reference each, released with 00B67700):
    // 00B67980 globals of [00E188A8]+1A0Ch, 00B67800 GetByName, 00B67720
    // GetByIndex, 00B65F50 nil object, 00B67690 assign, 00B661B0 IsTable,
    // 00B660A0 IsString.
    virtual void* lua_globals_00b67980() = 0;
    virtual void* lua_get_by_name_00b67800(void* table, const char* name) = 0;
    virtual void* lua_get_by_index_00b67720(void* table, std::int32_t index) = 0;
    virtual void* lua_nil_00b65f50() = 0;
    virtual void lua_assign_00b67690(void* dst, void* src) = 0;
    virtual bool lua_is_table_00b661b0(void* object) = 0;
    virtual bool lua_is_string_00b660a0(void* object) = 0;
    virtual void lua_release_00b67700(void* object) = 0;
    // 0067E580, ECX = unit library screen (reconstructed with another host as
    // ingame_gui_cc13::unit_library_reset_selection_0067e580).
    virtual void unit_library_reset_selection_0067e580(Raw screen) = 0;
    // 0054EEF0(ECX = count, EDX = 0) -> storage for count 1Ch-byte shots;
    // 00550B50 throws length_error ("vector<T> too long").
    virtual std::uint8_t* idle_camera_shot_allocate_0054eef0(std::size_t count) = 0;
    [[noreturn]] virtual void stl_xlen_00550b50() = 0;
    // operator new(518h), memset 0, 006065D0 (ECX = block): the plane cockpit
    // camera; null when the allocation failed.
    virtual Raw plane_cockpit_camera_new_006065d0() = 0;
    // 0042CD00(&out), ECX = camera -> pointer to its position (x, y, z).
    virtual const float* camera_position_0042cd00(Raw camera, float out[3]) = 0;
    // 0042E030(const float* position), ECX = cockpit camera.
    virtual void cockpit_camera_set_position_0042e030(Raw camera, const float* position) = 0;
    // Unit virtual +18h(const float local[3], float out[3]) -> out: a local
    // point to world.
    virtual const float* unit_local_to_world_18(Raw unit, const float local[3], float out[3]) = 0;
    // Unit virtual +210h(a, b) -> EAX (the image tests the whole register).
    virtual std::uint32_t unit_virtual_210(Raw unit, std::int32_t a, std::int32_t b) = 0;
    // 007C7750 / 007CC8E0 (0), ECX = plane -> EAX: the level-bomb and drop
    // attitude gates (tested as whole registers by 0049F2D0).
    virtual std::uint32_t plane_level_bomb_attitude_gate_007c7750(Raw plane, std::int32_t arg) = 0;
    virtual std::uint32_t plane_drop_attitude_gate_007cc8e0(Raw plane, std::int32_t arg) = 0;
    // 00ABE6E0(a, b, c, d), ECX = widget: set the quad floats.
    virtual void widget_set_quad_floats_00abe6e0(void* widget, float a, float b, float c, float d) = 0;
    // 0064ABD0(dt), ECX = quad fade (screen+16Ch).
    virtual void quad_fade_update_0064abd0(Raw fade, float dt) = 0;
    // 00414DB0, ECX = unit: refresh the world pose (called when byte +C8h is 0).
    virtual void entity_pose_refresh_00414db0(Raw unit) = 0;
    // 0043B370(value, dt), ECX = digit gauge.
    virtual void digit_gauge_update_0043b370(void* gauge, float value, float dt) = 0;
    // 007C1DB0, ECX = plane -> EAX: remaining ordnance rounds.
    virtual std::int32_t unit_count_remaining_ordnance_rounds_007c1db0(Raw plane) = 0;
    // A unit virtual returning a float in ST0 (kept as double: x87 extended).
    virtual double unit_virtual_float(Raw unit, std::uint16_t slot) = 0;
    // An object virtual returning AL (ECX = object).
    virtual bool object_virtual_al(Raw object, std::uint16_t slot) = 0;
    // Byte global 00F88988 (speed shown in mph when set) and float global
    // 00F8757C.
    virtual std::uint8_t byte_00f88988() = 0;
    virtual float float_00f8757c() = 0;
    // The function-local static of 00609BD0: float 00E1968C and its guard
    // dword 00E19690 (bit 0), returned as one 8-byte block.
    virtual Raw static_needle_span_block_00e1968c() = 0;
    // 00B64780 / 00B64640 / 00B646E0 (ECX = out matrix, EDX = &angle): the
    // rotation builders about Z, X, Y (the image copies each result through
    // 004134F0).
    virtual void matrix_rotation_z_00b64780(float out[16], float angle) = 0;
    virtual void matrix_rotation_x_00b64640(float out[16], float angle) = 0;
    virtual void matrix_rotation_y_00b646e0(float out[16], float angle) = 0;
    // 00413920(out, b), ECX = a -> out: 4x4 multiply.
    virtual void matrix_multiply_00413920(const float a[16], const float b[16], float out[16]) = 0;
    // Node virtual +38h(const float matrix[16]).
    virtual void node_set_matrix_38(Raw node, const float matrix[16]) = 0;
    // 00427EB0, ECX = unit -> its world position (refreshed); 00427C90(out,
    // b), ECX = a: out = a - b; 00427E30, ECX = vector -> length squared in
    // ST0.
    virtual const float* unit_world_position_00427eb0(Raw unit) = 0;
    virtual void vector_subtract_00427c90(const float* a, float out[3], const float* b) = 0;
    virtual double vector_length_squared_00427e30(const float v[3]) = 0;
    // 00803CE0, ECX = recon block ([plane+54h]), EDX = unit -> EAX relation
    // (1 for the value the HUD treats as friendly).
    virtual std::int32_t recon_relation_if_identified_00803ce0(Raw recon, Raw unit) = 0;
    // 0043A660(a, b), ECX = &world, EDX = &out -> EAX (7: on screen).
    virtual std::int32_t camera_project_world_to_screen_0043a660(const float world[3], float out[3], int a, int b) = 0;
    // 00AA7D00(x, y), ECX = widget: set local x, y and bounds.
    virtual void widget_set_local_xy_00aa7d00(void* widget, float x, float y) = 0;
    // Plane ordnance queries (ECX = plane, (0)) -> AL: 007B9320 general bomb,
    // 007B9400 rocket, 007B9500 level bomb, 007B94F0 depth charge, 007B93F0
    // torpedo.
    virtual bool plane_weapon_query(Raw plane, std::uint32_t routine) = 0;
    // Plane ordnance points (ECX = plane, (&out)) -> EAX = &out: 007C21D0
    // bomb impact, 007C2450 rocket impact, 007BCCF0 release point.
    virtual const float* plane_ordnance_point(Raw plane, std::uint32_t routine, float out[3]) = 0;
    // 007BB110, ECX = plane -> AL: ordnance can be released.
    virtual bool plane_can_release_ordnance_007bb110(Raw plane) = 0;
    // 00604CF0, ECX = plane HUD screen: the pulse alpha step.
    virtual void plane_screen_pulse_alpha_00604cf0(Raw screen) = 0;
    // 0042B2F0, ECX = vector -> length in ST0.
    virtual double vector_length_0042b2f0(const float v[3]) = 0;
    // 00419010(x0, y0, x1, y1, x) -> ST0: clamped interpolation
    // (src/unit_rudder.cpp models it file-locally).
    virtual double clamped_interpolate_00419010(float x0, float y0, float x1, float y1, float x) = 0;
    // 00BD2F10(lo, hi) with the stream id in ECX -> ST0.
    virtual double random_uniform_00bd2f10(int stream, float lo, float hi) = 0;
    // 0042C450(amount), ECX = [*00E188A8+1ED4h].
    virtual void scene_apply_0042c450(void* scene, float amount) = 0;
    // 0064A540(top, bottom, left, right), ECX = quad fade (screen+16Ch): bind
    // the four edge-flash icons.
    virtual void quad_fade_bind_0064a540(Raw fade, void* top, void* bottom, void* left, void* right) = 0;
    // More Lua object routines (handles as above): 00B66A60 IsInteger,
    // 00B66290 GetInteger, 00B662B0 GetString, 00B66270 GetNumber (ST0),
    // 00B679F0 GetVector2(&out) -> &out, 00B67080 / 00B67190 iterate first /
    // next (ECX = table, &key, &value), 00B66420 iteration ended (ECX = table,
    // &key).
    virtual bool lua_is_integer_00b66a60(void* object) = 0;
    virtual std::int32_t lua_get_integer_00b66290(void* object) = 0;
    virtual const char* lua_get_string_00b662b0(void* object) = 0;
    virtual double lua_get_number_00b66270(void* object) = 0;
    virtual const float* lua_get_vector2_00b679f0(void* object, float out[2]) = 0;
    virtual void lua_iterate_first_00b67080(void* table, void* key, void* value) = 0;
    virtual void lua_iterate_next_00b67190(void* table, void* key, void* value) = 0;
    virtual bool lua_iterate_ended_00b66420(void* table, void* key) = 0;
    // 0067ED30, ECX = unit library screen: release the model parts
    // (ingame_gui_cc13::unit_library_release_model_parts_0067ed30).
    virtual void unit_library_release_model_parts_0067ed30(Raw screen) = 0;
    // 007188A0, ECX = &native string: load a model resource by name.
    virtual void* resource_load_with_game_factory_007188a0(std::string_view name) = 0;
    // operator new(1ACh); when it succeeded 007135C0(0, model+8h(0, 1.0f)),
    // ECX = block: a unit part instance; null when the allocation failed.
    virtual Raw unit_part_instance_new_007135c0(void* model) = 0;
    // 00B6D890(root), ECX = node; 00B6E680(parent), ECX = node.
    virtual void node_propagate_root_registration_00b6d890(Raw node, void* root) = 0;
    virtual void node_set_parent_00b6e680(Raw node, void* parent) = 0;
    // Node virtual +34h(const float matrix[16]).
    virtual void node_set_matrix_34(Raw node, const float matrix[16]) = 0;
    // 00710BB0(group), ECX = part instance: select the visible model group.
    virtual void model_groups_select_visible_00710bb0(Raw part, std::int32_t group) = 0;
    // 0071AD50(name), ECX = [part+160h] -> note node or null.
    virtual void* model_find_note_node_0071ad50(void* model, std::string_view name) = 0;
    // 0042B490 / 0042B450(angle), ECX = out matrix -> out: rotation about Y /
    // X by -angle.
    virtual const float* matrix_rotation_y_negated_0042b490(float out[16], float angle) = 0;
    virtual const float* matrix_rotation_x_negated_0042b450(float out[16], float angle) = 0;
    // 00718000(const std::string& name, index), ECX = model -> the last named
    // point group or null.
    virtual void* resource_find_last_named_point_group_00718000(void* model, std::string_view name,
                                                                std::int32_t index) = 0;
    // 004803A0(index), ECX = point vector (group+44h) -> &point (0Ch bytes).
    virtual const float* point_vector_at_004803a0(Raw vector, std::int32_t index) = 0;
    // 004FA230(const vec* stack_arg), ECX = out, EDX = first operand: cross.
    virtual void vector_cross_004fa230(float out[3], const float* edx_operand, const float* stack_operand) = 0;
    // 0085DC80, ECX = matrix: orthonormalize the basis rows.
    virtual void matrix_orthonormalize_basis_rows_0085dc80(float matrix[16]) = 0;
    // 00680CC0 / 00680D30(const T*), ECX = screen+110h / +120h: push_back of
    // the model and part vectors.
    virtual void unit_library_model_vector_push_00680cc0(Raw vector, void* model) = 0;
    virtual void unit_library_part_vector_push_00680d30(Raw vector, void* part) = 0;
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
// 0054B080 is the same body for another std::vector<uint32_t> (identical
// listing up to call targets). 0054AED0 __thiscall(vector, iterator* out,
// uint8_t* where, size_t count, const uint8_t* value), RET 10h: the byte
// version (max size FFFFFFFFh).
void byte_vector_insert_n_0054aed0(Raw vector, std::uint8_t* where, std::size_t count, std::uint8_t value,
                                   Fe5Host& host);
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
// play the theme movie with (1, -902.0f, loop 1). The float pushed at 00689EFF
// is -902.0 (00CEFCB8); the plate comment was corrected by cc13-fe7 (it said 1.0f).
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
inline constexpr std::array<SmallHudScreenShape, 8> kSmallHudScreens{{
    {0x0068A2D0, 0x0068A330, 0x00CF7938, 0x00CF7920, {0x34, 0x4C, 0x50, 0x40, 0x44, 0x54}, {0x48}, false, "screen 24h"},
    {0x0068A3B0, 0x0068A420, 0x00CF7978, 0x00CF7960, {0x1C, 0x20, 0x24, 0x28, 0x34}, {0x30}, false, "screen 46h"},
    {0x0068A4A0, 0x0068A4F0, 0x00CF79B8, 0x00CF79A0, {0x1C, 0x20}, {}, false, "screen 2Fh"},
    {0x0068A570, 0, 0x00CEC590, 0x00CEC578, {}, {}, false, "bomber view screen"},
    {0x0068A5A0, 0x0068A5F0, 0x00CF79F8, 0x00CF79E0, {}, {}, false, "binoculars overlay 4Ah"},
    {0x0068A8A0, 0x0068A910, 0x00CF7A38, 0x00CF7A20, {}, {}, true, "common overlay 27h"},
    {0x0060CAA0, 0, 0x00CF48C8, 0x00CF48B0, {0x1C, 0x20, 0x24}, {}, false, "screen 40h (destructor 0060C650 rewrites the vtables)"},
    {0x0060CAE0, 0, 0x00CF4A7C, 0x00CF4A64, {0x1C, 0x20, 0x24}, {}, false, "plane spawn screen 41h"},
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

// 0060C620 __thiscall(camera, object), RET 4: null +374h or +378h when it
// holds the object (the plane-spawn camera's observer slot).
void plane_spawn_camera_on_object_released_0060c620(Raw camera, const void* object) noexcept;
// 0060C650 __thiscall(screen): vtables 00CF48C8 / 00CF48B0, callback owner
// +8h, front-end base (screen 40h).
void hud_screen40_destruct_0060c650(Raw screen, Fe5Host& host);
// 0060C6C0 __thiscall(screen): with a plane at +20h, observe it (screen+8h);
// when the screen's +28h virtual returns an object, observe that too and keep
// its air-ops block (006BCD20) in +24h.
void hud_plane_slot_register_observers_0060c6c0(Raw screen, Fe5Host& host);
// 0060C700 / 0060C780 __thiscall(screen): stop observing +20h, (+24h)->+7Ch
// and +1Ch, nulling each (byte-identical bodies).
void hud_plane_slot_unregister_observers(Raw screen, Fe5Host& host);
// 0060C930 __thiscall(camera) -> EAX: camera-mover base, then vtables
// 00CF4930 (+0h), 00CF4914 (+10h), 00CF490C (+24h), 00CF48F4 (+170h).
void* plane_spawn_camera_construct_0060c930(Raw camera, Fe5Host& host);

// 00608C30 __thiscall(screen) (plane effects enter): clear +104h, update the
// fov, pick the camera (cockpit/chase, then 00608B60), +7Ch = -1.0
// (00D7A260), zero the 40h bytes at +1Ch and +5Ch, copy the scene view's
// +1C4h into +80h/+84h when game+19FCh exists, save the render transition into
// +88h..+90h and apply the tuning default (+D8h, +DCh, +E0h, 0, 0, 0).
inline constexpr float kPlaneEffectsUnsetTime = -1.0f;  // 00D7A260
void plane_effects_enter_00608c30(Raw screen, Fe5Host& host);
// 00608D50 __thiscall(screen, plane), RET 4: clear +104h, release the old
// view, stop observing the old plane, store the new one (+B8h), select the
// camera, +A8h = 0, update the fov, apply the view mode, clear +9Ch and reset
// the cockpit camera (+98h), zero the chase camera's +3E4h/+3E8h, clear
// +BCh/+BDh, +108h = input manager table +1EB4h, byte +10Ch = 1.
void plane_screen_bind_plane_00608d50(Raw screen, void* plane, Fe5Host& host);

// ---------------------------------------------------------------------------
// Segment 16: the weapon-group HUD screen 2Eh (src/hud_weapon_group_screen.cpp
// and src/mission_camera.cpp hold cc9's update/bind/layout models).
// ---------------------------------------------------------------------------
// 00545270 __thiscall(screen): orbit camera +20h from yaw 0.0 (when set).
void weapon_group_reset_orbit_camera_00545270(Raw screen, Fe5Host& host);
// 005452B0 __thiscall(screen, float yaw), RET 4: orbit camera +2Ch (when set).
void weapon_group_orbit_camera_005452b0(Raw screen, float yaw, Fe5Host& host);
// 005452F0 __thiscall(screen): hide the 5x5 slot widgets at +5Ch (null slots
// skipped) and the four widgets +C0h..+CCh.
void weapon_group_hide_slot_widgets_005452f0(Raw screen, Fe5Host& host);
// 005454B0 __thiscall(screen 2Eh, float x, float y, float z), RET 0Ch: store
// +48h..+50h; unless game byte +19C4h is set, centre the markers view at (x, y).
void screen2e_set_view_centre_005454b0(Raw screen, float x, float y, float z, Fe5Host& host);
// 00545550 / 00545600 __thiscall(entry): entry vtable (00CEDD70 / 00CEDD88),
// stop observing +14h (ECX = +14h, EDX = entry) when set, then the callback
// owner destructor on the entry itself.
void weapon_group_entry_destruct(Raw entry, std::uint32_t vtable, Fe5Host& host);
inline constexpr std::uint32_t kWeaponGroupEntryAVtable = 0x00CEDD70;
inline constexpr std::uint32_t kWeaponGroupEntryBVtable = 0x00CEDD88;

// 00545750 __cdecl() -> EAX = 00E192F4: the lazily built colour (1, 0, 0, 1).
const float* static_colour_red_00545750(Fe5Host& host);
// 005459B0 __thiscall(gun) -> AL: the weapon kind [[gun+3F4h]+80h] is 2, 3,
// 4 or 6. 005459E0 -> AL: kind 5 or 6. Pure reads of gun data; cc9's
// src/game_hosts_gunnery.cpp and include/bsp/gun_aiming.hpp use them (the
// latter calls 005459B0's answer "muzzle_blocked"; see the doc).
bool gun_is_artillery_kind_005459b0(Raw gun) noexcept;
bool gun_is_torpedo_class_launcher_005459e0(Raw gun) noexcept;
// 00545AC0 __thiscall(screen): +D4h = 0, +1Ch = 1, hide the two widget
// arrays +E4h and +F0h over +E8h entries, then the slot widgets.
void weapon_group_resume_00545ac0(Raw screen, Fe5Host& host);
// 00545C20 __thiscall(array {data, count}, const entry*), RET 4 -> EAX: the
// index of the first 18h-byte entry whose +14h matches, else -1.
std::int32_t entry_array_index_of_target_00545c20(Raw array, Raw entry) noexcept;

// The engine's pointer array {data +0h, count +4h, capacity +8h} and the
// 18h-byte observer-handle array (entry: vtable +0h, callback owner +4h..+0Ch,
// byte +10h, observed +14h).
// 00545CB0 / 00545D10 __thiscall(array, int capacity), RET 4: at least 1;
// grow only (copy, _free the old block). Byte-identical bodies.
void ptr_array_reserve(Raw array, std::int32_t capacity, Fe5Host& host);
// 00545ED0 / 00545F10 __thiscall(array, const void** value), RET 4: grow to
// twice the capacity (at least 1) when full, then append.
void ptr_array_push_back(Raw array, void* value, Fe5Host& host);
// 00545FE0 / 00546030 __thiscall(array, int count), RET 4: reserve, zero the
// new slots, set the count.
void ptr_array_resize(Raw array, std::int32_t count, Fe5Host& host);
// 005465B0 / 005465D0 __thiscall(array): resize to 0, _free the block.
void ptr_array_destroy(Raw array, Fe5Host& host);
// 00545F50 __thiscall(handle, const handle* other), RET 4 -> EAX: clear the
// callback-owner words, vtable 00CEDD88, copy other+14h, byte +10h = 1,
// observe it (ECX = observed, EDX = handle) when non-null.
void* observer_handle_copy_construct_00545f50(Raw handle, Raw other, Fe5Host& host);
// 005460A0 __thiscall(handle, observed), RET 4 -> EAX: the same from a
// pointer. 00546310 then rewrites the vtable to 00CEDDA0 (entry C).
void* weapon_group_entry_construct_005460a0(Raw handle, void* observed, Fe5Host& host);
void* weapon_group_entry_c_construct_00546310(Raw handle, void* observed, Fe5Host& host);
inline constexpr std::uint32_t kWeaponGroupEntryCVtable = 0x00CEDDA0;
// 00546100 __thiscall(handle, observed), RET 4: re-point +14h (unregister the
// old, register the new). 00546330 / 00546570 do the same and return EAX =
// handle.
void* observer_handle_set_target(Raw handle, void* observed, Fe5Host& host);
// 005461B0 / 00546200 / 00546250 / 00546280 __thiscall(handle, notifier*),
// RET 4: when the notifier's +4h virtual returns +14h, clear +14h and
// unregister (byte-identical bodies).
void observer_handle_on_released(Raw handle, Raw notifier, Fe5Host& host);
// 00546370 __thiscall(array, int capacity), RET 4: grow the 18h-byte entry
// array (copy-construct as entry C, destroy the old entries with +0h(0),
// _free the old block).
void entry_array_reserve_00546370(Raw array, std::int32_t capacity, Fe5Host& host);
// 00546730 __thiscall(array, const entry* value), RET 4: append a copy.
void entry_array_append_00546730(Raw array, Raw value, Fe5Host& host);
// 005467B0 __thiscall(array, int count), RET 4: default-construct new
// entries as entry C (+14h = 0, byte +10h = 1), destroy the surplus with
// +0h(0) from the back, set the count.
void entry_array_resize_005467b0(Raw array, std::int32_t count, Fe5Host& host);

// 005464E0 __thiscall(screen 2Eh): for each of the +E8h entries delete the
// +E4h object (+4h(1)) and, for the +F0h widget, drop timed entry 2 then
// delete it; resize both pointer arrays to 0.
void screen2e_clear_timed_entries_005464e0(Raw screen, Fe5Host& host);
// 005472B0 __thiscall(array): entry-array resize to 0, _free the block.
void entry_array_destroy_005472b0(Raw array, Fe5Host& host);
// 005472D0 __thiscall(array, int index), RET 4 -> EAX: grow the reference
// pointer array to index + 1 when needed and return &data[index].
void** refptr_array_at_grow_005472d0(Raw array, std::int32_t index, Fe5Host& host);
// 005472F0 __thiscall(screen) -> EAX: screen 2Eh (vtables 00CEDF34 /
// 00CEDF1C): front-end base, callback owner, the entry array +20h..+28h, the
// entry-A handle at +2Ch (vtable 00CEDDB8, byte +3Ch = 1), +40h = 0 and the
// pointer arrays +E4h..+F8h zeroed.
inline constexpr std::uint32_t kScreen2EVtable = 0x00CEDF34;
inline constexpr std::uint32_t kScreen2ESecondaryVtable = 0x00CEDF1C;
void* screen2e_construct_005472f0(Raw screen, Fe5Host& host);
// 00547390 __thiscall(screen): vtables, remove the pages +DCh and +58h,
// destroy the pointer arrays +F0h and +E4h, the entry-A handle +2Ch, the entry
// array +20h, the callback owner, the front-end base.
void screen2e_destruct_00547390(Raw screen, Fe5Host& host);
// 005482D0 __thiscall(screen, vector* out), RET 4: pushes &+DCh, &+58h.
void screen2e_collect_children_005482d0(Raw screen, void* out, Fe5Host& host);
// 00549200 __thiscall(ECX = screen+8h, object, event, unused), RET 0Ch: on
// event 5 for an object held by an entry, step to the next group (cc9's
// 00548410(1, 0)) unless the current group +44h is set and still available.
void weapon_group_on_observed_event_00549200(Raw listener, const void* object, std::int32_t event, Fe5Host& host);

// 005468B0 __thiscall(screen 2Eh) (vtable +10h): front-end register, load
// "GUI_cross_gunstate" into +DCh and "GUI_cross_ship" into +58h, clear bytes
// +100h and +1Ch, the layout virtual (+14h), byte +D6h = 0, hide the slot
// widgets, bytes +108h/+109h = 0, byte +E0h = 1, +104h = 0.
void screen2e_register_005468b0(Raw screen, Fe5Host& host);

// ---------------------------------------------------------------------------
// The main-menu command-bar listener (segment 16): 00549A80 (vtable slot
// +04h) and 00549C30 (+0Ch) route a widget event to the first active screen
// (+4h byte) of each manager. Both first redirect a kind-12h widget: when it
// is one of the five buttons at listener+7Ch, press it (+84h(1)) and route its
// partner at listener+2Ch instead.
// ---------------------------------------------------------------------------
struct RouteSlot {
    std::uint16_t screen;    // manager offset of the screen pointer
    std::uint16_t listener;  // offset of the listener sub-object in that screen
};
struct RouteGroup {
    std::uint32_t manager;
    std::array<RouteSlot, 5> slots;
    std::uint8_t count;
    bool first_slot_ends_routing;  // the first slot's hit jumps to the final call and returns
};
// 00549A80 (slot +04h). The session browser's +40h hit shares the final call
// with the interface group and returns from there (00549BA4 JNE 00549C1A), so
// the interface is not offered the event.
inline constexpr std::array<RouteGroup, 4> kCommandRoute04{{
    {0x00E198AC, {{{0x58, 0x40}, {0x70, 0x68}, {0x6C, 0x0C}, {0x60, 0x0C}, {0x68, 0x08}}}, 5, false},
    {0x00E198B4, {{{0x40, 0x0C}, {0x44, 0x0C}, {0x48, 0x0C}, {0x4C, 0x0C}}}, 4, false},
    {0x00E198B8, {{{0x40, 0x0C}, {0x48, 0x0C}, {0x44, 0x0C}}}, 3, true},
    {0x00E198C4, {{{0xE4, 0x0C}, {0xE0, 0x0C}, {0xD8, 0x0C}}}, 3, false},
}};
// 00549C30 (slot +0Ch): the multiplayer menu offers only +40h, +44h, +48h.
inline constexpr std::array<RouteGroup, 4> kCommandRoute0C{{
    {0x00E198AC, {{{0x58, 0x40}, {0x70, 0x68}, {0x6C, 0x0C}, {0x60, 0x0C}, {0x68, 0x08}}}, 5, false},
    {0x00E198B4, {{{0x40, 0x0C}, {0x44, 0x0C}, {0x48, 0x0C}}}, 3, false},
    {0x00E198B8, {{{0x40, 0x0C}, {0x48, 0x0C}, {0x44, 0x0C}}}, 3, true},
    {0x00E198C4, {{{0xE4, 0x0C}, {0xE0, 0x0C}, {0xD8, 0x0C}}}, 3, false},
}};
// __thiscall(listener, widget), RET 4.
void command_bar_route(const std::array<RouteGroup, 4>& route, std::uint16_t slot, Raw listener, void* widget,
                       Fe5Host& host);
// 00549DD0 __thiscall(listener, widget, flag), RET 8 (slot +18h, hover):
// a kind-12h widget either releases all five buttons (+84h(0)) when flag is
// clear, or, when it is one of them, highlights it (+84h(2)) and routes its
// partner. Every group then offers (widget, flag) to its first active screen;
// no group ends the routing.
inline constexpr std::array<RouteGroup, 4> kCommandRoute18{{
    {0x00E198AC, {{{0x58, 0x40}, {0x70, 0x68}, {0x6C, 0x0C}, {0x60, 0x0C}, {0x68, 0x08}}}, 5, false},
    {0x00E198B4, {{{0x40, 0x0C}, {0x44, 0x0C}, {0x48, 0x0C}, {0x4C, 0x0C}}}, 4, false},
    {0x00E198B8, {{{0x40, 0x0C}, {0x48, 0x0C}, {0x44, 0x0C}}}, 3, false},
    {0x00E198C4, {{{0xE4, 0x0C}, {0xE0, 0x0C}, {0xD8, 0x0C}}}, 3, false},
}};
void command_owner_on_widget_hover_00549dd0(Raw listener, void* widget, std::uint32_t flag, Fe5Host& host);

// 0054AC10 __thiscall(const string* self, string* out, char c), RET 8 ->
// EAX out: out = self + c.
void native_string_concat_char_0054ac10(Raw self, Raw out, std::uint8_t c, Fe5Host& host);

// The idle-camera screen 2Ch (vtables 00CEE408 / 00CEE3F0).
// 0054EC90 __thiscall(screen): vtables, callback owner +8h, front-end base.
void idle_camera_screen_destruct_0054ec90(Raw screen, Fe5Host& host);
// 0054F8C0 __thiscall(screen) -> EAX: front-end base, callback owner, +1Ch = 0.
void* idle_camera_screen_construct_0054f8c0(Raw screen, Fe5Host& host);

// An idle-camera shot (1Ch bytes): dwords +0h, +4h, native strings +8h and
// +10h, byte +18h.
// 00550390 __thiscall(shot): release the two strings (+10h first).
void idle_camera_shot_destroy_00550390(Raw shot, Fe5Host& host);
// 005506C0 __thiscall(dst, const shot* src), RET 4 -> EAX dst: assign.
void* idle_camera_shot_assign_005506c0(Raw dst, Raw src, Fe5Host& host);
// 005507E0 __thiscall(dst, const shot* src), RET 4 -> EAX dst: copy-construct.
void* idle_camera_shot_copy_construct_005507e0(Raw dst, Raw src, Fe5Host& host);
// 00550740 / 005508A0 __cdecl(first, last, dest) -> EAX dest end: assign
// forward (the second computes the end from the element count first).
std::uint8_t* idle_camera_shot_copy(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest, Fe5Host& host);
// 00550930 __cdecl(first, last, dest_end) -> EAX dest_end - (last - first):
// assign backward.
std::uint8_t* idle_camera_shot_copy_backward_00550930(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest_end,
                                                      Fe5Host& host);
// 00550990 register call (ECX = dest, EDX = src): copy-construct when dest
// is not null.
void idle_camera_shot_placement_copy_00550990(std::uint8_t* dest, Raw src, Fe5Host& host);
// 00550E10 __stdcall(first, last), RET 8: destroy each shot.
void idle_camera_shot_destroy_range_00550e10(std::uint8_t* first, std::uint8_t* last, Fe5Host& host);
// 00550EF0 __thiscall(vector): destroy [first, last), _free, zero the triple.
void idle_camera_shot_vector_tidy_00550ef0(Raw vector, Fe5Host& host);

// ---------------------------------------------------------------------------
// The control-binding screen (segment 16 tail).
// ---------------------------------------------------------------------------
// 00551BC0 __thiscall(screen, widget, bool hover), RET 8: ignored in mode 3
// (+15Ch) and for widgets of kind 10h or 2. Kind 3: alpha 0.5 (00CE3800)
// while hovered, else 1.0. Unless the widget is the thumb (+200h) while
// dragging (+1DCh): kind 6 gets +88h(hover, 0, 1.0), kind 12h +84h(hover).
void control_binding_on_widget_hover_00551bc0(Raw screen, void* widget, bool hover, Fe5Host& host);
// 005539E0 __thiscall(screen, widget), RET 4: the release twin; the state is
// the widget's own byte +D4h; the arrows (+1F8h, +1F4h) stop the auto scroll
// of the scroller at +194h (00683300(0, 0, 0)); the thumb (+200h) ends the
// drag (00683320); then the kind-6 / kind-12h refresh as for hover.
void control_binding_on_widget_released_005539e0(Raw screen, void* widget, Fe5Host& host);
// 00553BF0 __thiscall(screen): open the Xbox virtual keyboard for a text
// entry: user = the selected user (00A3EAC0) or 0, empty default at +E6h,
// result buffer +60h (80h bytes zeroed, 30h characters), overlapped +44h
// (seven dwords zeroed); then +E4h = 1. The ledger name "ResetSelection" does
// not fit the body (see the doc).
void control_binding_open_keyboard_00553bf0(Raw screen, Fe5Host& host);
// 00552C50 / 00552C70 __thiscall(node) -> EAX: the rightmost (+8h) /
// leftmost (+0h) node below it (nil byte +99h).
void* input_device_tree_max_00552c50(Raw node) noexcept;
void* input_device_tree_min_00552c70(Raw node) noexcept;
// 00553FA0 / 00554380 / 005543A0 __thiscall(iterator) -> EAX iterator: one
// increment routine each (00552770, 00552D40, 00552E20).
void* tree_iterator_pre_increment_via(Raw iterator, std::uint32_t routine, Fe5Host& host);
// 0068E8A0 __thiscall(deque, size_t count), RET 4: std::deque::_Growmap for
// the 2-strings-per-block deque (map +4h, map size +8h, offset +0Ch). The
// growth is max(count, max(map size / 2, 8)) when that fits under 0FFFFFFFh;
// the live block pointers keep their slot index (blocks before the front
// block are rotated past the old end when they fit), the rest are zeroed.
void native_string_deque_grow_map_0068e8a0(Raw deque, std::uint32_t count, Fe5Host& host);
// 0068EB90 __thiscall(deque, const string* value), RET 4: push_back
// (grow the map by one when the next slot opens a block and the map is full,
// allocate the 10h-byte block on demand, copy-construct the string).
void native_string_deque_push_back_0068eb90(Raw deque, Raw value, Fe5Host& host);
// 006088F0 __thiscall(screen) (plane HUD exit): _free +150h/+154h; free the
// two owned vectors +108h/+10Ch (006088A0's shape, inlined); restore widget
// +C8h's local y from +114h; stop and drop the point effect +15Ch; hide +B8h
// and +0Ch; byte +8h = 0; delete every element of the vectors +98h and +A8h
// (+4h(1)) and clear both (erase(begin, end)).
void plane_screen_exit_006088f0(Raw screen, Fe5Host& host);
// 006093D0 __thiscall(screen) (plane HUD enter, vtable +18h): build the
// widget set +108h from +F4h..+FCh, two 20h-byte gauge needles (+150h on
// widget +E4h with maximum tuning +684h, +154h on widget +E8h with maximum
// 50.0; both scale -1.0, +8h 768.0, +0Ch 644.0, offset 1.6406096, span
// 2.9670596, value 0, then SetValue(0)), show +B8h and +0Ch, build the widget
// set +10Ch from +100h/+104h, hide widget +D4h and copy plane +E84h into +138h
// when ((plane+538h)+600h) == 0.0 (UCOMISS, so NaN shows it), else show it;
// hide +E0h, byte +8h = 1, the payload icon, rebuild the widgets, byte +190h = 1.
inline constexpr float kNeedleCentreX = 768.0f;        // 00CF4864
inline constexpr float kNeedleCentreY = 644.0f;        // 00CF4860
inline constexpr float kNeedleOffset = 1.640609622001648f;   // 00CF485C
inline constexpr float kNeedleSpan = 2.96705961227417f;      // 00CF4858
inline constexpr float kNeedleSecondMaximum = 50.0f;   // 00CF41D8
void plane_screen_enter_006093d0(Raw screen, Fe5Host& host);
// 00682990 __thiscall(screen, const string* text, const float* rgba, frame),
// RET 0Ch (read from the listing; the decompile loses the stack arguments):
// under second_Group (+60h): second_Text gets the colour (+50h) and the
// localised source, second_1_Icon and second_2_Icon get +88h(frame, 0, 1.0);
// then second_Group is shown.
void warning_screen_show_second_text_00682990(Raw screen, Raw text, const float* rgba, std::uint32_t frame,
                                              Fe5Host& host);
// 00682BD0 __thiscall(screen, string by value), RET 8: warning_2_text under
// third_Group (+64h) takes the string as its localised source; the argument
// string is released.
void warning_screen_set_warning2_text_00682bd0(Raw screen, Raw text, Fe5Host& host);
// 00553B00 __thiscall(screen, widget), RET 4: with a widget, size the
// marker (+210h) to (w, h + 1/360) (00CEE4E8) and place it at the widget's
// resolved position minus (w * [widget+18h], (h + 1/360) * [widget+1Ch]),
// z + 0.01 (00D7A358), then show it; with null, hide the marker (tail jump).
inline constexpr double kMarkerHeightPad = 0.002777777777777778;  // 00CEE4E8
inline constexpr double kMarkerDepthBias = 0.009999999776482582;  // 00D7A358
void control_binding_position_marker_00553b00(Raw screen, void* widget, Fe5Host& host);
// 00607BE0 __thiscall(screen) (plane effects register, vtable +10h):
// front-end register; delete the cameras +94h and +98h (+0h(1)) and null
// them; +CCh = 0.0; stop observing through the observer at +D0h (its +14h)
// and clear it; +B8h = +FCh = 0; acquire the effect handles "Planewindsmoke"
// (00CF4838) into +60h, "warning heartbeat only" (00CF4820) into +68h,
// "Turbo_Effect" (00CF4810) into +74h and effect id 153h into +78h, each a
// reference-counted swap that releases the temporary.
inline constexpr std::array<std::pair<std::uint16_t, std::string_view>, 3> kPlaneEffectHandles{{
    {0x60, "Planewindsmoke"}, {0x68, "warning heartbeat only"}, {0x74, "Turbo_Effect"}}};
inline constexpr std::uint32_t kPlaneEffectHandleId78 = 0x153;
void plane_effects_register_00607be0(Raw screen, Fe5Host& host);

// --- cc13-fe7 (docs/CC13_FE_7.md) ---
// 00608F30 __fastcall(ECX = plane HUD screen 3Eh), plain RET: delete every
// widget of the vectors +94h (first +98h) and +A4h (first +A8h) through their
// virtual +4h(1), empty both; then, for the plane +168h, map its type +970h
// (9, 0Bh, 0Dh, 0Fh, 12h, 0Ah -> layout 0..5, else none), count its payload
// children (list +48h, next +44h): layouts 0-3 and 5 count children of kind
// 25h; layout 4 takes the first child of kind 26h (its rounds, +488h, the
// getter 006E3520) or of kind 25h (1). For each payload slot clone the
// template +8Ch at (x, 0.6, -100) and the template +90h at (x, 0.6, -101),
// pushed onto +94h / +A4h. x: odd counts centre slot 0 at 0.5 and step
// +-0.02 around 0.51 (00D7A2F8, 00CF4850); even counts step 0.005 around 0.5
// (00CEC8F8, 00D7A280), odd slots (i + 1), even slots -1, -2, ... per slot.
inline constexpr float kPlaneSlotDefaultX = 0.5f;          // 00CE3800
inline constexpr float kPlaneSlotY = 0.6f;                 // 00CE3D30 (0x3F19999A)
inline constexpr float kPlaneSlotZ = -100.0f;              // 00CE65D8
inline constexpr double kPlaneSlotOddStep = 0.019999999552965164;    // 00D7A2F8
inline constexpr double kPlaneSlotOddCentre = 0.5099999997764826;    // 00CF4850
inline constexpr double kPlaneSlotEvenStep = 0.004999999888241291;   // 00CEC8F8
inline constexpr double kPlaneSlotEvenCentre = 0.5;                  // 00D7A280
inline constexpr double kPlaneSlotBackDepth = 1.0;                   // 00D7A210
int plane_hud_payload_layout(std::int32_t plane_type) noexcept;
void plane_screen_rebuild_widgets_00608f30(Raw screen, Fe5Host& host);
// 00681BA0 __thiscall(ECX = unit library screen, dt unused), RET 4. Mode +E8h
// 0: action 4Bh pops to interface 1; 4Ch / 4Dh with listbox +70h on row 0
// move the digit cursor +ECh (-1 while 4Ch is held, else +1), on row 1 step
// the digit (0067DF40, -1 / +1). Mode 1: 4Bh resets the selection (0067E580),
// else 4Ah is polled. Then 4Ch sets +E4h = -1, 4Dh +1; when +E4h != 0 step
// the class index +130h ((i + 1000 + step) % 1000) until VehicleClass[i] is
// a table whose "Mesh" is a string, load its model (00680DB0), +E4h = 0.
void unit_library_update_00681ba0(Raw screen, float dt, Fe5Host& host);
// 00550DD0 (ECX = &value; dest, count, allocator proxy), RET 0Ch:
// uninitialized_fill_n of 1Ch-byte idle camera shots (00550BF0: copy
// construct 005507E0 each, destroy the constructed ones on a throw); returns
// dest + count.
std::uint8_t* idle_camera_shot_uninit_fill_00550dd0(std::uint8_t* dest, std::size_t count, Raw value, Fe5Host& host);
// 00550FC0 __thiscall(ECX = vector {proxy, first, last, end}; where (checked
// iterator: proxy, pointer), count, const shot&), RET 10h: std::vector<shot>::
// _Insert_n (VC8): copy the value, grow by half (or to size + count) and
// rebuild, or shift the tail in place.
void idle_camera_shot_vector_insert_n_00550fc0(Raw vector, std::uint8_t* where, std::size_t count, Raw value,
                                               Fe5Host& host);
// 00607F60 __fastcall(ECX = plane effects screen 3Fh), plain RET: build the
// cockpit camera +98h when absent (006065D0, identity matrix into the scene
// [00E188A8]+19CCh, observed through screen+8h); copy the chase camera +94h's
// position into +F0h..+F8h when one exists; install +98h as the camera mover
// (004BC410) and clear +94h. With the plane's +1C0h answer non-zero: call its
// +1C4h, near plane 0.5 (00CE3800), eye = model([plane+538h]+244h) to world
// through the plane's +18h into +F0h, set the camera position (0042E030),
// reset it (0042AA00(0, 0, 0)) and observe the plane (link +500h, observed
// +514h). Otherwise the eye is (0, 1.8, [model+A0h] * 0.6) (00CF4848,
// 00CEFF98; FMUL in x87, one float store), and the plane is not observed.
inline constexpr float kCockpitNearPlane = 0.5f;                         // 00CE3800
inline constexpr float kCockpitFallbackEyeY = 1.7999999523162842f;       // 00CF4848
inline constexpr double kCockpitFallbackEyeScale = 0.6000000238418579;  // 00CEFF98
void plane_screen_ensure_cockpit_camera_00607f60(Raw screen, Fe5Host& host);
// 00609620 __fastcall(ECX = plane HUD screen 3Eh), plain RET. With a plane
// (+168h), per payload layout (00604C80) collect per slot: ready (bit vector
// A), gate (bit vector B) and fill = 1 - remaining / full (float, x87 FDIV
// then FSUBR from 1, one float store):
//   layouts 0-3, 5: each child of kind 25h: A = child+210h(2Ah, 0), B = the
//     level-bomb gate 007C7750(0) when the plane is kind 10h else the drop
//     gate 007CC8E0(0), fill from child +490h / +48Ch;
//   layout 4: the first child of kind 26h gives one slot per round (+488h):
//     A = byte +654h+j or dword +514h+4j non-zero, B = 1, fill from
//     +5B4h+4j / +48Ch; else the first child of kind 25h one slot with the
//     drop gate.
// Then for each front widget i (+94h vector; count read once): show it with
// byte +190h; when A[i] is set, frame +88h(B[i] == 0, 0, 1.0) and hide the
// back widget i (+A4h); else frame +88h(2, 0, 1.0), show the back widget with
// +190h and set its quad floats (fill[i], 0, 0, 1.0).
void plane_screen_update_drop_indicator_00609620(Raw screen, Fe5Host& host);
// 00605B20 __thiscall(ECX = plane HUD screen, float altitude), RET 4: the
// low-altitude blinker. Phase +134h moves down while byte +130h is set, else
// up, by 0.12 below 10, 0.03 above 33.33, else 0.06 (FSUB / FADD qword, one
// float store); at <= 0 the direction flips up (+130h = 0), at >= 0.6 down
// (+130h = 1). When the plane's altitude (+100h, pose refreshed) is below 50
// the widget +ECh is shown with +130h (tail jump into its +34h).
inline constexpr float kBlinkSlowBelow = 10.0f;                       // 00CE38B8
inline constexpr float kBlinkFastAbove = 33.33333206176758f;          // 00CF4390
inline constexpr double kBlinkStepLow = 0.11999999731779099;          // 00CE67E8
inline constexpr double kBlinkStepHigh = 0.029999999329447746;        // 00CEB690
inline constexpr double kBlinkStepMid = 0.05999999865889549;          // 00CF0AC0
inline constexpr float kBlinkPhaseTop = 0.6000000238418579f;          // 00CE3D30
inline constexpr double kLowAltitude = 50.0;                          // 00CE3938
void plane_screen_altitude_blink_00605b20(Raw screen, float altitude, Fe5Host& host);
// 00609BD0 __thiscall(ECX = plane HUD screen, float dt, bool ordnance_view,
// bool aim_view), RET 0Ch: the instruments update (see docs/CC13_FE_7.md for
// the full walk). Altimeter needles +150h / +154h and low-altitude widgets
// +E8h / +ECh; speed (plane +204h * 1.946, * tuning +6A0h, * 0.621 when
// 00F88988 is set, shown when below 999 else 999) into the digit gauge
// +108h; remaining rounds into +10Ch; the drop indicator 00609620; the
// ladder +C8h at y = +114h - v^2 * 0.104 (v = plane +9F0h); the throttle
// needle +DCh at span*0.5 - clamp(+E84h)*span (span 00E1968C = 1.4660766);
// the horizon node ([+C4h]+ECh)+38h with RotY(-pi) * RotX(+C64h) *
// RotZ(+C68h - pi); the aim point (pose +FCh + 1000 * forward +ECh) or the
// ordnance points place the reticle rows (+10h + row*14h, five widgets per
// row); the warning +E0h pulse against +138h; the camera shake from the
// stick vector, timer +164h / byte +160h and the stream-1 RNG (ECX = 1 at
// 0060AAA2; not the gameplay stream 0).
inline constexpr double kSpeedToKnots = 1.9459999799728394;           // 00CF4898
inline constexpr double kKnotsToMph = 0.621;                          // 00CF4890
inline constexpr double kSpeedShownBelow = 999.0;                     // 00CF4880
inline constexpr float kSpeedShownCap = 999.0f;                       // 00CF4888
inline constexpr double kLadderScale = 0.10400000214576721;           // 00CF4878
inline constexpr float kThrottleNeedleSpan = 1.4660766124725342f;     // 00CF4870
inline constexpr double kThrottleNeedleHalf = 0.5;                    // 00D7A280
inline constexpr double kHorizonRollBias = 3.1415927410125732;        // 00CE3D28
inline constexpr float kHorizonYaw = -3.1415927410125732f;            // 00CE684C
inline constexpr double kAimDistance = 1000.0;                        // 00CE47A0
inline constexpr double kReticleOffset = 0.25;                        // 00D7A348
inline constexpr float kTorpedoMarkerY = 0.25f;                       // 00CE3868
inline constexpr float kTorpedoMarkerAbove = 45.0f;                   // 00CE3D60
inline constexpr double kTorpedoMarkerBase = 45.0;                    // 00D7A368
inline constexpr double kTorpedoMarkerSlope = 0.0005000000237487257;  // 00CF4868
inline constexpr double kTorpedoMarkerLimit = 0.05000000074505806;    // 00D7A270
inline constexpr float kTorpedoMarkerLimitF = 0.05000000074505806f;   // 00CE7638
inline constexpr double kTorpedoMarkerPad = 0.014999999664723873;     // 00CEE6C8
inline constexpr double kShakeHalf = 0.5;                             // 00D7A280
void plane_screen_update_instruments_00609bd0(Raw screen, float dt, bool ordnance_view, bool aim_view,
                                              Fe5Host& host);
// 00606C60 __fastcall(ECX = plane HUD screen 3Eh), plain RET (vtable +14h):
// the layout binder. Page "GUI_cross_all" into +0Ch; when it loaded:
// "cross_tilt_Icon" into +88h; for each reticle row i (kPlaneReticleRows,
// jump table 00607BB0) find the row group in +0Ch, then the five cross icons
// of the row into +10h + i*14h + j*4 (zeroed first; kPlaneReticleSlots, with
// the rows that skip a slot), and when the row's F icon exists place its
// child "cross_F2_Icon" at half the F icon's size (z 0; the pointer is not
// kept); "payload_template_Icon" (+8Ch) and "payload_template_Section"
// (+90h), both hidden. Then page "GUI_plane_effects" into +B8h with
// AttackRun_Text (+BCh), RepairZone_Text (+11Ch) and, when VillanasFelso_Icon
// exists, the four edge-flash icons bound to the quad fade +16Ch (0064A540);
// page "GUI_plane" into +B4h with kPlaneHudWidgets (the parent is the page
// unless the row names a widget offset).
inline constexpr std::array<std::string_view, 6> kPlaneReticleRows{{
    "plane_AA_Group", "plane_rocket_Group", "plane_bomb_Group", "plane_torpedo_Group", "plane_parat_Group",
    "plane_DC_Group"}};  // 00CF47E0, 00CF47CC, 00CF47B8, 00CF47A4, 00CF4780, 00CF4794
struct PlaneReticleSlot {
    std::string_view name;
    std::uint8_t skip_rows;  // bit i set: row i has no such icon
};
inline constexpr std::array<PlaneReticleSlot, 5> kPlaneReticleSlots{{
    {"cross__Icon", 0x00},                                    // 00CEDE80
    {"cross_F_Icon", (1u << 2) | (1u << 4)},                  // 00CEDE70
    {"cross_H_Icon", (1u << 4)},                              // 00CEDE60
    {"cross_HF_Icon", (1u << 3) | (1u << 4) | (1u << 5)},     // 00CEDE50
    {"cross_L_Icon", (1u << 2) | (1u << 4) | (1u << 5)}}};    // 00CEDE40
struct PlaneHudWidget {
    std::uint16_t offset;
    std::uint16_t parent;  // 0: the page +B4h, else the screen offset of the parent
    std::string_view name;
};
inline constexpr std::array<PlaneHudWidget, 15> kPlaneHudWidgets{{
    {0xC8, 0, "plane_stick_Icon"},
    {0xD4, 0, "turbo_Group"},
    {0xDC, 0xD4, "plane_turbo_arrow_Icon"},
    {0xE0, 0xD4, "plane_turbo_warning_Icon"},
    {0xE4, 0, "plane_arrow_blue_Icon"},
    {0xE8, 0, "plane_arrow_red_Icon"},
    {0xEC, 0, "plane_warning_Icon"},
    {0xF4, 0, "ship_speed_num1_Icon"},
    {0xF8, 0, "ship_speed_num2_Icon"},
    {0xFC, 0, "ship_speed_num3_Icon"},
    {0xC4, 0, "muhorizont_Model"},
    {0xD8, 0, "plane_payload_Group"},
    {0x100, 0xD8, "ship_payload_1_Icon"},
    {0x104, 0xD8, "ship_payload_2_Icon"},
    {0x110, 0xD8, "payload_type_Icon"}}};
inline constexpr double kCrossF2Scale = 0.5;  // 00D7A280
void plane_screen_load_layout_00606c60(Raw screen, Fe5Host& host);
// 00680DB0 __fastcall(ECX = unit library screen), plain RET: release the
// model parts (0067ED30); open the file block "GIUnitLib::ShowSelectedVehicle_"
// + class index (+130h); VehicleClass[+130h].Mesh loads the hull model
// (+108h) and its part instance (+10Ch, identity +34h matrix, group 0). For
// each integer-keyed entry of the class's Platforms table whose Gun[1] is an
// integer device index with a string DeviceClass[i].Mesh: the hull's last
// point group "slot" + key with more than two points gives the mount basis
// (row3 = p0, row2 = p2 - p0, row1 = cross(p2 - p0, p1 - p0), then
// orthonormalized); the rest angles come from RestAngles (a vector2) or else
// from Windows[1] (mean of Min/Max Horz and Vert angles, x87 sum * 0.5, one
// float store); the device model gets a part instance parented to the hull
// part with RotY(-yaw) * basis and its "barrel" note node RotX(-pitch);
// model and part are appended to +110h / +120h.
void unit_library_load_unit_model_00680db0(Raw screen, Fe5Host& host);
// @decl-end

// ---------------------------------------------------------------------------
// Packet cc13-trivialslots: one ordinary function per kTrivialSlots row, re-read from
// the disk bytes (local generator, rows checked against the image). `Raw` is the
// ECX object; stack arguments the body never reads are noted in the ABI and not
// passed. Adjustor and tail-jump rows appear only where the target is an existing
// declaration; the descriptive table above stays as documentation.
// ---------------------------------------------------------------------------
// 00682140 MOV EAX,50h; RET (B8 50 00 00 00 C3; next byte CC).
// BSP_HudWarningScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 80, RET.
std::int32_t hud_warning_screen_get_screen_id_00682140(Raw) noexcept;
// 00687320 JMP 00683AA0 (E9 7B C7 FF FF; next byte CC). BSP_MultiMenu_Deactivate_Thunk.
// Original ABI: __thiscall(ECX = this), the target's RET; tail call to 00683AA0
// (Fe5Host::front_end_manager_deactivate_00683aa0).
void multi_menu_deactivate_thunk_00687320(Raw self, Fe5Host& host);
// 00689BD0 RET (C3; next byte CC). BSP_AttractScreen_Virtual04NoOp. Original ABI:
// __thiscall(ECX = this), RET.
void attract_screen_virtual04no_op_00689bd0(Raw) noexcept;
// 0068A320 MOV EAX,24h; RET (B8 24 00 00 00 C3; next byte CC). BSP_HudScreenSlot24_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 36, RET.
std::int32_t hud_screen_slot24_get_screen_id_0068a320(Raw) noexcept;
// 0068A400 MOV EAX,46h; RET (B8 46 00 00 00 C3; next byte CC). BSP_HudScreenSlot46_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 70, RET.
std::int32_t hud_screen_slot46_get_screen_id_0068a400(Raw) noexcept;
// 0068A410 RET 4 (C2 04 00; next byte CC). BSP_HudScreenSlot46_CollectChildrenNoOp. Original
// ABI: __thiscall(ECX = this, 1 stack dword unread), RET 4.
void hud_screen_slot46_collect_children_no_op_0068a410(Raw) noexcept;
// 0068A4E0 MOV EAX,2Fh; RET (B8 2F 00 00 00 C3; next byte CC). BSP_HudScreenSlot2F_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 47, RET.
std::int32_t hud_screen_slot2f_get_screen_id_0068a4e0(Raw) noexcept;
// 0068A5D0 MOV EAX,4Ah; RET (B8 4A 00 00 00 C3; next byte CC).
// BSP_HudBinocularsOverlayScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 74,
// RET.
std::int32_t hud_binoculars_overlay_screen_get_screen_id_0068a5d0(Raw) noexcept;
// 0068A5E0 RET 4 (C2 04 00; next byte CC). BSP_HudBinocularsOverlayScreen_CollectChildrenNoOp.
// Original ABI: __thiscall(ECX = this, 1 stack dword unread), RET 4.
void hud_binoculars_overlay_screen_collect_children_no_op_0068a5e0(Raw) noexcept;
// 0068A8F0 MOV EAX,27h; RET (B8 27 00 00 00 C3; next byte CC).
// BSP_HudCommonOverlayScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 39,
// RET.
std::int32_t hud_common_overlay_screen_get_screen_id_0068a8f0(Raw) noexcept;
// 006935B0 RET 4 (C2 04 00; next byte CC). BSP_ObserverEndpoint_NoOp08_006935b0. Original ABI:
// __thiscall(ECX = this, 1 stack dword unread), RET 4.
void observer_endpoint_no_op08_006935b0(Raw) noexcept;
// 00606220 MOV EAX,3Fh; RET (B8 3F 00 00 00 C3; next byte CC).
// BSP_HudPlaneEffectsScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 63, RET.
std::int32_t hud_plane_effects_screen_get_screen_id_00606220(Raw) noexcept;
// 00606230 RET 4 (C2 04 00; next byte CC). BSP_HudPlaneEffectsScreen_CollectChildrenNoOp.
// Original ABI: __thiscall(ECX = this, 1 stack dword unread), RET 4.
void hud_plane_effects_screen_collect_children_no_op_00606230(Raw) noexcept;
// 00606A80 MOV EAX,3Eh; RET (B8 3E 00 00 00 C3; next byte CC). BSP_HudPlaneScreen_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 62, RET.
std::int32_t hud_plane_screen_get_screen_id_00606a80(Raw) noexcept;
// 0060C6B0 MOV EAX,40h; RET (B8 40 00 00 00 C3; next byte CC). BSP_HudScreenSlot40_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 64, RET.
std::int32_t hud_screen_slot40_get_screen_id_0060c6b0(Raw) noexcept;
// 0060C750 JMP 004F71D0 (E9 7B AA EE FF; next byte CC). BSP_HudScreenSlot40_Register. Original
// ABI: __thiscall(ECX = this), the target's RET; tail call to 004F71D0
// (Fe5Host::front_end_screen_register_004f71d0).
void hud_screen_slot40_register_0060c750(Raw self, Fe5Host& host);
// 0060C760 RET 4 (C2 04 00; next byte CC). BSP_HudScreenSlot40_UpdateNoOp. Original ABI:
// __thiscall(ECX = this, 1 stack dword unread), RET 4.
void hud_screen_slot40_update_no_op_0060c760(Raw) noexcept;
// 0060C770 RET (C3; next byte CC). BSP_HudScreenSlot40_EnterNoOp. Original ABI: __thiscall(ECX
// = this), RET.
void hud_screen_slot40_enter_no_op_0060c770(Raw) noexcept;
// 0060C960 XOR EAX,EAX; RET (33 C0 C3; next byte CC). BSP_PlaneSpawnCamera_Slot130_ReturnZero.
// Original ABI: __thiscall(ECX = this) -> EAX = 0, RET.
std::int32_t plane_spawn_camera_slot130_return_zero_0060c960(Raw) noexcept;
// 0060CB20 MOV EAX,41h; RET (B8 41 00 00 00 C3; next byte CC).
// BSP_HudPlaneSpawnScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 65, RET.
std::int32_t hud_plane_spawn_screen_get_screen_id_0060cb20(Raw) noexcept;
// 00547370 MOV EAX,2Eh; RET (B8 2E 00 00 00 C3; next byte CC). BSP_HudScreen2E_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 46, RET.
std::int32_t hud_screen2e_get_screen_id_00547370(Raw) noexcept;
// 00549590 RET (C3; next byte CC). BSP_MainMenuCommandOwner_ListenerNoOp1C. Original ABI:
// __thiscall(ECX = this), RET.
void main_menu_command_owner_listener_no_op1c_00549590(Raw) noexcept;
// 0054D660 MOV EAX,34h; RET (B8 34 00 00 00 C3; next byte CC). BSP_HudHintsScreen_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 52, RET.
std::int32_t hud_hints_screen_get_screen_id_0054d660(Raw) noexcept;
// 0054D670 MOV AL,1; RET (B0 01 C3; next byte CC). BSP_HudHintsScreen_Virtual04True. Original
// ABI: __thiscall(ECX = this) -> AL = 1, RET.
bool hud_hints_screen_virtual04true_0054d670(Raw) noexcept;
// 0054ECF0 MOV EAX,2Ch; RET (B8 2C 00 00 00 C3; next byte CC).
// BSP_HudIdleCameraScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 44, RET.
std::int32_t hud_idle_camera_screen_get_screen_id_0054ecf0(Raw) noexcept;
// 00551B90 RET (C3; next byte CC). BSP_ControlBindingScreen_Exit. Original ABI: __thiscall(ECX
// = this), RET.
void control_binding_screen_exit_00551b90(Raw) noexcept;
// 00551BB0 RET (C3; next byte CC). BSP_ControlBindingScreen_ListenerNoOp1C. Original ABI:
// __thiscall(ECX = this), RET.
void control_binding_screen_listener_no_op1c_00551bb0(Raw) noexcept;

}  // namespace bsp::cc13_fe5

#endif  // BSP_FRONTEND_SEG16_24_28_CC13_HPP
