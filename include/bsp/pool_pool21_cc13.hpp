#pragma once

// pool_pool21_cc13: 00675C40 BSP_HudSupportManagerScreen_Update (packet cc13-pool21). The
// name is a hypothesis, not a recovered symbol. This is a new C++ interface over the native
// screen record, not a drop-in replacement of the original ABI, and nothing here is wired
// into the game.
//
// Conventions follow cc13_big1: the screen is the raw 480h-byte block (constructor 006735D0,
// vtable 00CF6B44, slot 8 = +20h), touched only at the offsets the image reads or writes,
// and every call the image makes goes through a named host method at the same point with
// the same arguments. Pool21Host extends Big1Host, whose methods (the support-manager
// routines, the award-tracker hints, NativeString construction and release, the checked STL
// iterators) this body shares with the Enter 00674B80.

#include "bsp/cc13_big1.hpp"

#include <cstdint>

namespace bsp::pool_pool21_cc13 {

class Pool21Host : public cc13_big1::Big1Host {
public:
    // ---- globals ----
    // Byte [00F88A30]: nonzero selects the "_xbox" message variants.
    virtual std::uint8_t global_byte_00f88a30() = 0;

    // ---- strings, hints ----
    // 00682BD0 BSP_HudWarningScreen_SetWarning2Text: __thiscall(ECX = [hud+C4h]; NativeString
    // by value (8 bytes, built in the argument slot by 0041E870 and released by the callee)),
    // RET 8.
    virtual void warning_set_warning2_text_00682bd0(void* warning_screen, void* text_by_value) = 0;
    // 00449AF0 BSP_NativeString_NotEqualCaseInsensitive: __thiscall(ECX = left; const
    // NativeString* right), RET 4; AL.
    virtual bool native_string_not_equal_ci_00449af0(const void* left, const void* right) = 0;
    // 0068E5B0 BSP_AwardTracker_CopyActiveHintName: __thiscall(ECX = tracker; NativeString* out),
    // RET 4; EAX = out.
    virtual void* award_tracker_copy_active_hint_name_0068e5b0(void* tracker, void* out) = 0;
    // 00690B00 BSP_AwardTracker_IsActiveHintBlocking: __thiscall(ECX = tracker; const
    // NativeString*), RET 4; AL.
    virtual bool award_tracker_is_active_hint_blocking_00690b00(void* tracker, const void* name) = 0;
    // 00BF7FBF CRT _stricmp, cdecl(a, b); EAX.
    virtual std::int32_t crt_stricmp_00bf7fbf(const char* a, const char* b) = 0;

    // ---- support records ----
    // 00668540 BSP_IntListIntRecord_Assign: __thiscall(ECX = destination 14h-byte {int,
    // std::list<int>, int} record (an uninitialised temporary here); const record*), RET 4;
    // EAX = destination. Its list at +4 is released by 005A0B80 (list_tidy_005a0b80).
    virtual void* int_list_int_record_copy_00668540(void* destination, const void* source) = 0;

    // ---- support-manager screen routines (ECX = screen) ----
    // 00652AA0 / 00652B00: show the arrow pairs +F8h/+FCh and +100h/+104h, __fastcall, RET.
    virtual void support_show_arrows_f8_00652aa0(void* screen) = 0;
    virtual void support_show_arrows_100_00652b00(void* screen) = 0;
    // 00668D40 / 00661B50: step the catalog and slot ring scrolls, __thiscall(float dt), RET 4.
    virtual void support_step_catalog_ring_00668d40(void* screen, float dt) = 0;
    virtual void support_step_slot_ring_00661b50(void* screen, float dt) = 0;
    // 006619E0 BSP_HudSupportManagerScreen_LayoutScrollRows, __fastcall, RET.
    virtual void support_layout_scroll_rows_006619e0(void* screen) = 0;
    // 0065BD90 BSP_HudSupportManagerScreen_UpdateNameLabelFade, __thiscall(float dt), RET 4.
    virtual void support_update_name_label_fade_0065bd90(void* screen, float dt) = 0;
    // 00661240 BSP_HudSupportManagerScreen_BeginScrollBack, __fastcall, RET.
    virtual void support_begin_scroll_back_00661240(void* screen) = 0;
    // 006605C0 BSP_HudSupportManager_PruneLargeSlots, __fastcall, RET.
    virtual void support_prune_large_slots_006605c0(void* screen) = 0;
    // 0065BF00 BSP_HudSupportManagerScreen_UpdateMapIcons, __fastcall, RET.
    virtual void support_update_map_icons_0065bf00(void* screen) = 0;
    // 0065FBF0 BSP_HudSupportManagerScreen_UpdateSlotHighlights, __thiscall(float dt), RET 4.
    virtual void support_update_slot_highlights_0065fbf0(void* screen, float dt) = 0;
    // 0065ABB0 BSP_HudSupportManager_ClearLoadoutIcons, __fastcall, RET.
    virtual void support_clear_loadout_icons_0065abb0(void* screen) = 0;
    // 0066B6C0 BSP_HudSupportManager_RefreshInfoPanel, __fastcall, RET.
    virtual void support_refresh_info_panel_0066b6c0(void* screen) = 0;
    // 00658580 BSP_HudSupportManagerScreen_WorldToMap: __thiscall(float out[2], x, y, z), RET 10h.
    virtual void support_world_to_map_00658580(void* screen, float* out, float x, float y, float z) = 0;
    // 00668570 BSP_HudSupportManagerScreen_SelectionHealth, __fastcall, RET; ST0 (stored as float).
    virtual float support_selection_health_00668570(void* screen) = 0;
    // 006723F0 BSP_HudSupportManagerScreen_StepCloseFade, __thiscall(float dt), RET 4.
    virtual void support_step_close_fade_006723f0(void* screen, float dt) = 0;
    // 00660190 BSP_HudSupportManagerScreen_SelectDeckSlot: __thiscall(the 58h stack item), RET 4.
    virtual void support_select_deck_slot_00660190(void* screen, void* item) = 0;
    // Sounds, __fastcall, RET: 0065F800 left/right, 0065F170 unavailable, 0065F6B0 unit-to-unit.
    virtual void support_play_left_right_sound_0065f800(void* screen) = 0;
    virtual void support_play_unavailable_sound_0065f170(void* screen) = 0;
    virtual void support_play_unit_to_unit_sound_0065f6b0(void* screen) = 0;
    // 00671D60 BSP_HudSupportManager_ShowSelectionInfo, __fastcall, RET.
    virtual void support_show_selection_info_00671d60(void* screen) = 0;
    // 00660B10 BSP_HudSupportManagerScreen_BeginScrollForward, __fastcall, RET.
    virtual void support_begin_scroll_forward_00660b10(void* screen) = 0;
    // 0065A740 BSP_HudSupportManagerScreen_UpdatePayloadStrip, __fastcall, RET.
    virtual void support_update_payload_strip_0065a740(void* screen) = 0;
    // 0065AB40 / 0065AE90: the catalog entry at the cursor of list +37Ch / +2E4h, __fastcall,
    // RET; EAX.
    virtual void* support_catalog_entry_at_index_0065ab40(void* screen) = 0;
    virtual void* support_catalog_entry_2e4_at_index_0065ae90(void* screen) = 0;
    // 0065C9A0 BSP_HudSupportManagerScreen_SelectValue: __thiscall(entry), RET 4.
    virtual void support_select_value_0065c9a0(void* screen, void* entry) = 0;
    // 00665F30 / 00662020 (loadout ring and highlight) and 00666DC0 / 00662210 (their hangar
    // versions), __fastcall, RET.
    virtual void support_build_loadout_ring_00665f30(void* screen) = 0;
    virtual void support_highlight_loadout_00662020(void* screen) = 0;
    virtual void support_build_hangar_loadout_ring_00666dc0(void* screen) = 0;
    virtual void support_highlight_hangar_loadout_00662210(void* screen) = 0;

    // ---- the 58h stack item (00675C40's [ESP+D0h], EH state 36h) ----
    // 006C1960 BSP_SupportManagerItem_Construct: __thiscall(ECX = item; dword), RET 4.
    // 006BC690 BSP_AirOpsSlotRecord_Destruct: __thiscall(ECX = item), RET.
    virtual void support_item_construct_006c1960(void* item, std::uint32_t arg) = 0;
    virtual void air_ops_slot_record_destruct_006bc690(void* item) = 0;

    // ---- records, widgets, input, points ----
    // 006697A0 BSP_IntListIntRecord_Assign: __thiscall(ECX = destination record; const
    // record*), RET 4; EAX = destination.
    virtual void* int_list_int_record_assign_006697a0(void* destination, const void* source) = 0;
    // 00654650 STL_TreeIter_Inc: __thiscall(ECX = iterator {owner, node}), RET.
    virtual void tree_iterator_increment_00654650(void* iterator) = 0;
    // 00AA7D00 BSP_GuiWidget_SetLocalXYAndBounds: __thiscall(ECX = widget; float x, float y), RET 8.
    virtual void widget_set_local_xy_00aa7d00(void* widget, float x, float y) = 0;
    // Widget vtable +48h: __thiscall(ECX = widget; const float[2]*), RET 4.
    virtual void widget_vtable48(void* widget, const float* values) = 0;
    // 004C43C0 BSP_InputAction_WasPressedThisFrame: __thiscall(ECX = game; action), RET 4; AL.
    virtual bool input_action_was_pressed_004c43c0(void* game, std::int32_t action) = 0;
    // 00942130 (team_support_points_cap): __thiscall(ECX = player index), RET; ST0.
    virtual double team_support_points_cap_00942130(std::uint32_t player) = 0;
    // 00BF7420 CRT _ftol2: ST0 -> EAX (truncation).
    virtual std::int32_t crt_ftol2_00bf7420(double value) = 0;
    // 00435C40 BSP_NativeString_EqualsInsensitive: __thiscall(ECX = left; const NativeString*
    // right), RET 4; AL.
    virtual bool native_string_equals_ci_00435c40(const void* left, const void* right) = 0;

    // ---- the deck page's slot cursor (+324h) ----
    // 00656440 STL_Vector_Size: __thiscall(ECX = unit), RET; EAX = the unit's slot entry count.
    virtual std::int32_t unit_slot_count_00656440(void* unit) = 0;
    // 00846FD0 (checked vector element copy, 4Ch-byte entries): __thiscall(ECX = unit; out,
    // index), RET 8; EAX = out. 0065BB00 BSP_UnitSlotEntry_Destroy: __thiscall(ECX = entry), RET.
    virtual void* unit_slot_entry_copy_00846fd0(void* unit, void* out, std::uint32_t index) = 0;
    virtual void unit_slot_entry_destroy_0065bb00(void* entry) = 0;
    // 006626E0 BSP_ObserverStringRecord_Assign: __thiscall(ECX = screen+224h; const entry*), RET 4.
    virtual void observer_string_record_assign_006626e0(void* destination, const void* source) = 0;
    // 00656250 BSP_LockedRecord_ReadAt50: __thiscall(ECX = air-ops block or null), RET; EAX = the
    // slot count +50h.
    virtual std::int32_t locked_record_read_at50_00656250(void* block) = 0;
    // 0065CA50 BSP_AirOpsSlotRecord_Assign: __thiscall(ECX = record; const record*), RET 4.
    // 00658530 BSP_AirOpsSlotRecord_IsOut: __thiscall(ECX = record), RET; AL.
    virtual void air_ops_slot_record_assign_0065ca50(void* destination, const void* source) = 0;
    virtual bool air_ops_slot_record_is_out_00658530(void* record) = 0;
    // 00427EB0 BSP_EntityPose_GetWorldPositionRefreshed: __thiscall(ECX = entity), RET; EAX =
    // const float[3].
    virtual const float* entity_world_position_refreshed_00427eb0(void* entity) = 0;
    // 00414220 BSP_Vector3f_Assign: __thiscall(ECX = destination; const float*), RET 4.
    virtual void vector3f_assign_00414220(float* destination, const float* source) = 0;

    // ---- the loadout, launch, order and spectate actions (00679C8E..0067AED4) ----
    // 00951F10 (vehicle_class_equipment_count): __thiscall(ECX = class entry), RET; EAX.
    virtual std::int32_t vehicle_class_equipment_count_00951f10(void* entry) = 0;
    // Sounds, __fastcall, RET: 0065F410 change weapon, 0065F2C0 support-points-to-unit.
    virtual void support_play_change_weapon_sound_0065f410(void* screen) = 0;
    virtual void support_play_sp_to_unit_sound_0065f2c0(void* screen) = 0;
    // 004BEC00 BSP_InputManager_GetSingleton, cdecl(); EAX.
    virtual void* input_manager_singleton_004bec00() = 0;
    // 006BF310 BSP_AirOps_StockCount: __thiscall(ECX = air-ops block; class id), RET 4; EAX.
    virtual std::int32_t air_ops_stock_count_006bf310(void* block, std::uint32_t class_id) = 0;
    // 006C4780 thunk_BSP_AirOps_FillSlot: __thiscall(ECX = air-ops block; slot, class id,
    // count), RET 0Ch.
    virtual void air_ops_fill_slot_006c4780(void* block, std::int32_t slot, std::uint32_t class_id,
                                            std::int32_t count) = 0;
    // 00656280 BSP_SessionMessage130_Construct: __thiscall(ECX = message storage; block, slot,
    // kind, arg, player), RET 14h; EAX = message. No destructor call follows in this body.
    virtual void* session_message130_construct_00656280(void* message, void* block, std::int32_t slot,
                                                        std::int32_t kind, std::uint32_t arg,
                                                        std::uint32_t player) = 0;
    // 0077C2A0 BSP_Session_RouteMessage: __thiscall(ECX = [block+7Ch]; message, 0, 0), RET 0Ch.
    virtual void session_route_message_0077c2a0(void* router, void* message, std::uint32_t a,
                                                std::uint32_t b) = 0;
    // 00673A10 BSP_HudSupportManager_ConfirmOrder, __fastcall, RET.
    virtual void support_confirm_order_00673a10(void* screen) = 0;
    // 00658FE0 (checked list iterator postfix ++): __thiscall(ECX = iterator; out, int), RET 8.
    virtual void list_iterator_post_increment_00658fe0(void* iterator, void* out, std::int32_t dummy) = 0;
    // 004F1830 BSP_CommandTarget_InitializeRawStorage: __thiscall(ECX = 18h-byte target
    // descriptor), RET; EAX = the descriptor.
    virtual void* command_target_init_raw_004f1830(void* storage) = 0;
    // 0077D600 BSP_Entity_IssueCommand: __thiscall(ECX = entity; command (00E08F08 /
    // 00E08F98), const target descriptor*, flags), RET 0Ch. The flags dword (1) is pushed
    // before 004F1830 runs.
    virtual void entity_issue_command_0077d600(void* entity, std::uint32_t command, void* target,
                                               std::int32_t flags) = 0;
    // Entity vtable +174h: __thiscall(ECX = entity), RET; EAX = its class tag (00E08F98 /
    // 00E08FA0 are compared).
    virtual std::uint32_t entity_vtable174(void* entity) = 0;
    // Widget vtable +4Ch: __thiscall(ECX = the slot widget +BCh[i]; float), RET 4.
    virtual void widget_vtable4c(void* widget, float value) = 0;
    // 00647300 BSP_InGameHudRoot_SetSpectatedUnit: __thiscall(ECX = [hud+40h]; unit), RET 4.
    virtual void ingame_hud_set_spectated_unit_00647300(void* root, void* unit) = 0;
    // 005A4860 BSP_HudMapScreen_ClearSpeakingIcon: __thiscall(ECX = [hud+54h]), RET.
    virtual void hud_map_clear_speaking_icon_005a4860(void* map_screen) = 0;
    // 00A91E20 BSP_InputAction_Reset: __thiscall(ECX = [[input manager]+4]+3600h), RET.
    virtual void input_action_reset_00a91e20(void* action) = 0;
};

// 00675C40 BSP_HudSupportManagerScreen_Update: __thiscall(ECX = support-manager screen;
// float dt), RET 4. The screen's per-frame update (vtable 00CF6B44 slot 8).
void hud_support_manager_screen_update_00675c40(void* screen, float dt, Pool21Host& host);

} // namespace bsp::pool_pool21_cc13
