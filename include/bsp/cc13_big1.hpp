#pragma once

// cc13_big1: the large rows deferred by cc13-small2 (packet cc13-big1,
// docs/CC13_BIG1.md). Every name is a hypothesis, not a recovered symbol. This
// is a new C++ interface over the native records, not a drop-in replacement of
// the original ABI. Nothing here is wired into the game.
//
// Native objects are raw byte blocks touched only at the offsets the image
// reads or writes, and every call the image makes goes through a named host
// method at the same point with the same arguments.

#include "bsp/cc13_pkt642.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_big1 {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
template <class T>
inline const T& at(const void* base, std::size_t off) noexcept {
    return *reinterpret_cast<const T*>(static_cast<const std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }

// A checked-STL std::set / std::list object as the image lays it out: {proxy, head, size}.
struct NativeContainer {
    void* proxy;
    void* head;
    std::uint32_t size;
};

class Big1Host {
public:
    virtual ~Big1Host() = default;

    // ---- 004C7AA0 ----
    // 004C7730 BSP_World_FindClosestBorderZone (src/world_map_bounds.cpp models it
    // over a BorderZoneSet): __thiscall(ECX = world; const float* position, side,
    // float point_out[3], float direction_out[3]), RET 10h; EAX = the 44h zone
    // record or null.
    virtual void* world_find_closest_border_zone_004c7730(void* world, const float* position,
                                                           std::int32_t side, float* point_out,
                                                           float* direction_out) = 0;
    // 00419510 BSP_Vector3f_Normalize: __fastcall(ECX = out, EDX = in), RET; EAX = out.
    virtual const float* vector3f_normalize_00419510(float* out, const float* in) = 0;

    // ---- 005AAC00 (std::set<unsigned> {proxy, head, size}; std::list {proxy, head, size};
    //      tree node: left +0, parent +4, right +8, key +Ch, isnil +11h; list node: next +0,
    //      prev +4, value +8; checked iterators {owner, node}) ----
    // 00BF6713 CRT _invalid_parameter_noinfo (the checked-iterator failure call).
    virtual void crt_invalid_parameter_noinfo_00bf6713() = 0;
    // 00BF65AC CRT free (operator delete), cdecl(pointer).
    virtual void crt_free_00bf65ac(void* p) = 0;
    // 005A0300 STL_Tree_BuyHeadNode: __thiscall(ECX = tree), RET; EAX = the new node.
    virtual void* set_buy_head_node_005a0300(void* tree) = 0;
    // 0059FE60 STL_Tree_Erase (subtree): __thiscall(ECX = tree; node), RET 4.
    virtual void set_erase_subtree_0059fe60(void* tree, void* node) = 0;
    // 005A18E0 std::set<unsigned>::find: __thiscall(ECX = tree; iterator* out, const unsigned*
    // key), RET 8; EAX = out.
    virtual void* set_find_005a18e0(void* tree, void* out, const std::uint32_t* key) = 0;
    // 005A59C0 std::set<unsigned>::insert: __thiscall(ECX = tree; pair<iterator, bool>* out,
    // const unsigned* key), RET 8.
    virtual void set_insert_005a59c0(void* tree, void* out, const std::uint32_t* key) = 0;
    // 0059F0E0 STL_TreeIter_Inc: __thiscall(ECX = iterator), RET.
    virtual void set_iterator_increment_0059f0e0(void* iterator) = 0;
    // 005A6420 STL_Tree_EraseRange: __thiscall(ECX = tree; iterator* out, first {owner, node},
    // last {owner, node}), RET 14h.
    virtual void set_erase_range_005a6420(void* tree, void* out, void* first_owner, void* first_node,
                                          void* last_owner, void* last_node) = 0;
    // 005AAAF0 std::list copy constructor: __thiscall(ECX = destination; const list*), RET 4.
    virtual void list_copy_005aaaf0(void* out, const void* source) = 0;
    // 0084D560 BSP_SupportSite_BuildSupplyClassList: __thiscall(ECX = site; list* out (built
    // here), team, flag), RET 0Ch.
    virtual void support_site_build_supply_class_list_0084d560(void* site, void* out, std::int32_t team,
                                                               std::int32_t flag) = 0;
    // 00964790 BSP_VehicleClass_GetOrCreate: __fastcall(ECX = value, DL = flag); EAX = class.
    virtual void* vehicle_class_get_or_create_00964790(void* value, bool flag) = 0;
    // 0059CB90 BSP_Unit_GetHudIconIndex: __fastcall(ECX = unit, EDX = class); EAX.
    virtual std::uint32_t unit_hud_icon_index_0059cb90(void* unit, void* vehicle_class) = 0;
    // Unit vtable +5Ch: __thiscall(ECX = unit; flag), RET 4; AL.
    virtual bool unit_has_flag_vtable5c(void* unit, std::int32_t flag) = 0;
    // 00AAB4C0 BSP_GuiWidget_CloneSubtree: __thiscall(ECX = widget; parent clone), RET 4; EAX.
    virtual void* gui_widget_clone_subtree_00aab4c0(void* widget, void* parent_clone) = 0;
    // Widget vtable +34h: __thiscall(ECX = widget; byte), RET 4.
    virtual void widget_vtable34(void* widget, std::uint8_t arg) = 0;
    // Widget vtable +88h: __thiscall(ECX = widget; word frame (zero-extended), dword, float),
    // RET 0Ch.
    virtual void widget_vtable88(void* widget, std::uint32_t frame, std::int32_t arg, float value) = 0;
    // 0043DCA0 STL_Vector_PushBack: __thiscall(ECX = vector; const value*), RET 4.
    virtual void vector_push_back_0043dca0(void* vector, void* const* value) = 0;
    // The game singleton [00E188A8].
    virtual void* game_00e188a8() = 0;

    // ---- 0064AE20 (LuaPlus LuaObject, 14h bytes on the stack) ----
    // 00B67980 BSP_LuaStateOwner_GetGlobals: __thiscall(ECX = owner; LuaObject* out), RET 4; EAX = out.
    virtual void* lua_state_owner_get_globals_00b67980(void* owner, void* out) = 0;
    // 00B67800 BSP_LuaObject_GetByName: __thiscall(ECX = table; LuaObject* out, const char*),
    // RET 8; EAX = out.
    virtual void* lua_object_get_by_name_00b67800(void* table, void* out, const char* name) = 0;
    // 00B67720 BSP_LuaObject_GetByIndex: __thiscall(ECX = table; LuaObject* out, int), RET 8; EAX = out.
    virtual void* lua_object_get_by_index_00b67720(void* table, void* out, std::int32_t index) = 0;
    // 00B65F50 BSP_LuaObject_Construct: __thiscall(ECX = object), RET.
    virtual void lua_object_construct_00b65f50(void* object) = 0;
    // 00B67690 BSP_LuaObject_Assign: __thiscall(ECX = object; const LuaObject*), RET 4.
    virtual void lua_object_assign_00b67690(void* object, void* source) = 0;
    // 00B67700 BSP_LuaObject_Destruct: __thiscall(ECX = object), RET.
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    // 00B66000 BSP_LuaObject_IsBoolean: __thiscall(ECX = object), RET; AL.
    virtual bool lua_object_is_boolean_00b66000(void* object) = 0;
    // 00B66250 BSP_LuaObject_GetBoolean: __thiscall(ECX = object), RET; AL (compared with 1).
    virtual std::uint8_t lua_object_get_boolean_00b66250(void* object) = 0;
    // 0064AAD0 BSP_HudShipScreen45_UpdateDirIcon: __thiscall(ECX = screen), RET; ST0.
    virtual float hud_ship_screen45_update_dir_icon_0064aad0(void* screen) = 0;
    // 0064A9F0 BSP_HudShipScreen45_UpdateThrottleIndicator: __thiscall(ECX = screen; float), RET 4.
    virtual void hud_ship_screen45_update_throttle_indicator_0064a9f0(void* screen, float value) = 0;
    // 0064A960 BSP_HudShipScreen45_UpdateFormationIcon: __thiscall(ECX = screen), RET.
    virtual void hud_ship_screen45_update_formation_icon_0064a960(void* screen) = 0;
    // 00818300 BSP_Unit_TorpedoBarrelCount: __thiscall(ECX = unit), RET; EAX.
    virtual std::int32_t unit_torpedo_barrel_count_00818300(void* unit) = 0;
    // 00852350 BSP_Unit_CountBombPlatformChildren: __thiscall(ECX = unit), RET; EAX.
    virtual std::int32_t unit_count_bomb_platform_children_00852350(void* unit) = 0;

    // ---- 005E12A0 ----
    // 00B66BD0 BSP_LuaStateOwner_Construct: __thiscall(ECX = owner), RET.
    virtual void lua_state_owner_construct_00b66bd0(void* owner) = 0;
    // 00B6A020 BSP_LuaStateOwner_Open: __thiscall(ECX = owner; dword), RET 4.
    virtual void lua_state_owner_open_00b6a020(void* owner, std::int32_t flags) = 0;
    // 00B69D40 BSP_LuaStateOwner_RunScriptWithOverrides: __thiscall(ECX = owner; const
    // NativeString* path, dword), RET 8.
    virtual void lua_state_owner_run_script_00b69d40(void* owner, const void* path, std::int32_t arg) = 0;
    // 00B669A0 BSP_LuaStateOwner_Close: __thiscall(ECX = owner), RET.
    virtual void lua_state_owner_close_00b669a0(void* owner) = 0;
    // 00B65FB0 BSP_LuaObject_IsNil: __thiscall(ECX = object), RET; AL.
    virtual bool lua_object_is_nil_00b65fb0(void* object) = 0;
    // 00B66290 BSP_LuaObject_GetInteger: __thiscall(ECX = object), RET; EAX.
    virtual std::int32_t lua_object_get_integer_00b66290(void* object) = 0;
    // 00B662B0 BSP_LuaObject_GetString: __thiscall(ECX = object), RET; EAX.
    virtual const char* lua_object_get_string_00b662b0(void* object) = 0;
    // 0041DD40 BSP_NativeString_Resize: __thiscall(ECX = string {length, data}; length, dword),
    // RET 8.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::int32_t arg) = 0;
    // 00BF7680 CRT memcpy, cdecl(destination, source, count).
    virtual void crt_memcpy_00bf7680(void* destination, const void* source, std::uint32_t count) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton: no arguments; EAX.
    virtual void* sized_pool_get_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock: __thiscall(ECX = pool; data, bytes, dword), RET 0Ch.
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                  std::int32_t arg) = 0;
    // 005E11F0 (STL vector push_back, 24h-byte weather entries): __thiscall(ECX = vector; const
    // entry*), RET 4.
    virtual void weather_entry_vector_push_back_005e11f0(void* vector, const void* entry) = 0;
    // 005DACA0 BSP_LobbyMapEntry_Destruct: __thiscall(ECX = entry), RET.
    virtual void lobby_map_entry_destruct_005daca0(void* entry) = 0;
    // 00450540 STL_CheckedNativeStringVector_Append: __thiscall(ECX = vector; const
    // NativeString*), RET 4.
    virtual void native_string_vector_append_00450540(void* vector, const void* string) = 0;

    // ---- 006697E0 ----
    // 005A0B80 STL_List_Tidy: __thiscall(ECX = list), RET (frees the nodes and the head).
    virtual void list_tidy_005a0b80(void* list) = 0;
    // 0065B160 STL_Tree_Find (map keyed by a word at node+0Ch): __thiscall(ECX = tree;
    // iterator* out, const key*), RET 8; EAX = out.
    virtual void* support_tree_find_0065b160(void* tree, void* out, const std::uint32_t* key) = 0;
    // 00654F20 STL_TreeIter_Inc: __thiscall(ECX = iterator), RET.
    virtual void support_tree_iterator_increment_00654f20(void* iterator) = 0;
    // 00659400 STL_Tree_Erase (subtree): __thiscall(ECX = tree; node), RET 4.
    virtual void support_tree_erase_subtree_00659400(void* tree, void* node) = 0;
    // 006641B0 STL_Tree_EraseRange: __thiscall(ECX = tree; iterator* out, first {owner, node},
    // last {owner, node}), RET 14h.
    virtual void support_tree_erase_range_006641b0(void* tree, void* out, void* first_owner,
                                                   void* first_node, void* last_owner,
                                                   void* last_node) = 0;
    // 0065CE20 STL_Tree_CopyTree: __thiscall(ECX = destination tree; const source tree*), RET 4.
    virtual void support_tree_copy_0065ce20(void* tree, const void* source) = 0;
    // 00659AA0 STL_List_BuyNode (12-byte values): __thiscall(ECX = list; next, prev, const
    // value*), RET 0Ch; EAX = node.
    virtual void* support_list_buy_node_00659aa0(void* list, void* next, void* prev, const void* value) = 0;
    // 00662A70 (list _Incsize): __thiscall(ECX = list; count), RET 4.
    virtual void support_list_incsize_00662a70(void* list, std::uint32_t count) = 0;
    // Object vtable +198h: __thiscall(ECX = object; const NativeString* name, dword, bool* out),
    // RET 0Ch.
    virtual void object_vtable198(void* object, const void* name, std::int32_t arg, std::uint8_t* out) = 0;

    // ---- 0066A8A0 ----
    // 004C1630 STL_List_BuyHeadNode (list<void*>): __thiscall(ECX = list), RET; EAX = head.
    virtual void* unit_list_buy_head_node_004c1630(void* list) = 0;
    // 004C2220 STL_List_BuyNode (list<void*>): __thiscall(ECX = list; next, prev, const value*),
    // RET 0Ch; EAX = node.
    virtual void* unit_list_buy_node_004c2220(void* list, void* next, void* prev, const void* value) = 0;
    // 004CEE30 (list<void*> _Incsize): __thiscall(ECX = list; count), RET 4.
    virtual void unit_list_incsize_004cee30(void* list, std::uint32_t count) = 0;
    // 006681D0 (list<void*>::assign(first, last)): __thiscall(ECX = list; first {owner, node},
    // last {owner, node}, a by-value tag dword whose stack contents are stale), RET 14h.
    virtual void unit_list_assign_006681d0(void* list, void* first_owner, void* first_node,
                                           void* last_owner, void* last_node, std::uint32_t tag) = 0;
    // 00659D40 STL_List_BuyNode (14h-byte entries): __thiscall(ECX = list; next, prev, const
    // value*), RET 0Ch; EAX = node.
    virtual void* entry_list_buy_node_00659d40(void* list, void* next, void* prev, const void* value) = 0;
    // 006632D0 (entry list _Incsize): __thiscall(ECX = list; count), RET 4.
    virtual void entry_list_incsize_006632d0(void* list, std::uint32_t count) = 0;
    // 006652C0 STL_List_PushFront (14h-byte entries): __thiscall(ECX = list; const value*), RET 4.
    virtual void entry_list_push_front_006652c0(void* list, const void* value) = 0;
    // 00668350 (entry list assign(first, last)): as 006681D0.
    virtual void entry_list_assign_00668350(void* list, void* first_owner, void* first_node,
                                            void* last_owner, void* last_node, std::uint32_t tag) = 0;
    // 009FFD20 BSP_Unit_LossCountingSlot: __thiscall(ECX = unit), RET; EAX.
    virtual std::int32_t unit_loss_counting_slot_009ffd20(void* unit) = 0;
    // Object vtable +18h: __thiscall(ECX = object; dword), RET 4; AL.
    virtual bool object_vtable18(void* object, std::int32_t arg) = 0;
    // 008F2260 BSP_ScenePropertyBag_Find: __thiscall(ECX = bag; const char*), RET 4; EAX.
    virtual void* scene_property_bag_find_008f2260(void* bag, const char* name) = 0;
    // 004BCA50 BSP_Game_GetEffectiveGameMode: __thiscall(ECX = game), RET; EAX.
    virtual std::int32_t game_effective_mode_004bca50(void* game) = 0;
    // [00F89B3C]: the list object {proxy, head, size} of support records.
    virtual void* global_00f89b3c() = 0;

    // ---- 00674B80 ----
    // [00E198C4]: the HUD (screen slots +C0h powerup manager, +C8h counters, +54h a page).
    virtual void* hud_00e198c4() = 0;
    // [00E188D8]: the tracked-unit target the screen observes.
    virtual void* global_00e188d8() = 0;
    // 00529A30 BSP_HudCounters_Relayout / 00528F70 BSP_HudCounters_HideUnderManagerPanel:
    // __thiscall(ECX = [hud+C8h]), RET.
    virtual void hud_counters_relayout_00529a30(void* counters) = 0;
    virtual void hud_counters_hide_under_manager_panel_00528f70(void* counters) = 0;
    // 00612DA0 BSP_HudPowerupManagerScreen_RefreshPromptWidgets: __fastcall(ECX = [hud+C0h]).
    virtual void powerup_screen_refresh_prompt_widgets_00612da0(void* screen) = 0;
    // Support-manager screen routines, each __fastcall(ECX = screen), RET: 006567D0
    // LayoutWidget1C0, 00652A60, 0065D220, 0066FB00, 00668770, 0065A2B0, 0066A130 (the entry
    // lists), 0065BBC0 (the name label), 0066D020 (the selection refresh), 00669170, 0066A8A0,
    // 00670020 and 0065F950 (the open sound).
    virtual void support_layout_widget_1c0_006567d0(void* screen) = 0;
    virtual void support_delete_slot_clones_00652a60(void* screen) = 0;
    virtual void support_rebind_slots_0065d220(void* screen) = 0;
    virtual void support_refresh_sites_0066fb00(void* screen) = 0;
    virtual void support_rebuild_00668770(void* screen) = 0;
    virtual void support_sync_cursor_iterators_0065a2b0(void* screen) = 0;
    virtual void support_build_entries_0066a130(void* screen) = 0;
    virtual void support_layout_name_label_0065bbc0(void* screen) = 0;
    virtual void support_refresh_selection_0066d020(void* screen) = 0;
    virtual void support_select_weapon_value_00669170(void* screen) = 0;
    virtual void support_rebuild_lists_0066a8a0(void* screen) = 0;
    virtual void support_rebuild_00670020(void* screen) = 0;
    virtual void support_play_open_sound_0065f950(void* screen) = 0;
    // 0065BE80 BSP_HudSupportManagerScreen_SetTrackedUnitTarget: __thiscall(ECX = screen;
    // unit), RET 4.
    virtual void support_set_tracked_unit_target_0065be80(void* screen, void* unit) = 0;
    // 004E1CA0: the award tracker singleton (EAX). 0068E600: __thiscall(ECX = tracker; const
    // NativeString*), RET 4; AL = shown. 00690FD0: __thiscall(ECX = tracker; const
    // NativeString*, forced), RET 8.
    virtual void* award_tracker_get_singleton_004e1ca0() = 0;
    virtual bool award_tracker_has_hint_been_shown_0068e600(void* tracker, const void* name) = 0;
    virtual void award_tracker_trigger_hint_00690fd0(void* tracker, const void* name, std::int32_t forced) = 0;
    // 0041E870 NativeString from a C string: __thiscall(ECX = string; const char*), RET 4; EAX
    // = string. 0041DD20 its destructor: __thiscall(ECX = string), RET. A constructor, not an
    // assign: it zeroes the string first, so 00674B80 legitimately passes one it has released.
    virtual void* native_string_assign_0041e870(void* string, const char* text) = 0;
    virtual void native_string_destroy_0041dd20(void* string) = 0;
    // 004260B0 NativeString from an int: __thiscall(ECX = out; int), RET 4; EAX = out.
    virtual void* native_string_from_int_004260b0(void* out, std::int32_t value) = 0;
    // 004261A0 concat: __thiscall(ECX = left; out, const right*), RET 8; EAX = out.
    // 004263B0 concat an int: __thiscall(ECX = left; out, int), RET 8; EAX = out.
    virtual void* native_string_concat_004261a0(const void* left, void* out, const void* right) = 0;
    virtual void* native_string_concat_int_004263b0(const void* left, void* out, std::int32_t value) = 0;
    // 00665D20 (list<12-byte>::assign(first, last)): __thiscall(ECX = list; first {owner,
    // node}, last {owner, node}, a by-value tag dword), RET 14h.
    virtual void support_list_assign_00665d20(void* list, void* first_owner, void* first_node,
                                              void* last_owner, void* last_node, std::uint32_t tag) = 0;
    // 00ABBE50: __thiscall(ECX = text widget; const char*, localise), RET 8.
    virtual void gui_text_set_localised_cstring_00abbe50(void* text, const char* value, std::int32_t localise) = 0;
    // 00ABAED0: __thiscall(ECX = text widget; const NativeString*, localise), RET 8.
    virtual void gui_text_set_localised_source_00abaed0(void* text, const void* source, std::int32_t localise) = 0;
    // Object vtable +14h: __thiscall(ECX = object), RET; EAX = the class name text.
    virtual const char* object_vtable14_name(void* object) = 0;
    // 00658A60: ECX = the checked iterator at screen+300h; EAX = &node->value (the unit slot).
    // 00658B50: ECX = the checked iterator at screen+350h; EAX = &node->value.
    // 0059FB60: ECX = a checked list iterator {list, node}; EAX = &node->value.
    virtual void** support_unit_iterator_value_00658a60(void* iterator) = 0;
    virtual void* support_map_iterator_value_00658b50(void* iterator) = 0;
    virtual void** list_iterator_value_0059fb60(void* iterator) = 0;
    // 004E7A40 STL_List_Clear: __thiscall(ECX = list), RET.
    virtual void list_clear_004e7a40(void* list) = 0;
    // 0065B0F0 STL_Tree_Find (keys at +0Ch): __thiscall(ECX = tree; iterator* out, const key*),
    // RET 8; EAX = out. 00667D60 STL_Map_Subscript: __thiscall(ECX = tree; const key*), RET 4;
    // EAX = &mapped.
    virtual void* support_map_find_0065b0f0(void* tree, void* out, const void* key) = 0;
    virtual void* support_map_subscript_00667d60(void* tree, const void* key) = 0;
    // 009552E0: __thiscall(ECX = object; dword), RET 4; EAX. 00443490
    // BSP_DeviceClass_ResolveAndActivate: ECX = id, DL = flag; EAX.
    virtual void* object_class_device_set_009552e0(void* object, std::uint32_t id) = 0;
    virtual void* device_class_resolve_00443490(std::uint32_t id, std::uint8_t flag) = 0;
    // 00AB2690 BSP_GUI_SetStateTextureAndUV: __thiscall(ECX = widget; state, texture, const
    // float[4]*), RET 0Ch.
    virtual void gui_set_state_texture_uv_00ab2690(void* widget, std::uint32_t state, void* texture,
                                                   const void* uv) = 0;
    // 00946F00: __thiscall(ECX = player), RET; ST0 = the support points.
    virtual double player_support_points_00946f00(std::uint32_t player) = 0;
    // 008434D0 (shipyard idle slots): __thiscall(ECX = unit), RET; EAX. 006BDC30 (air-ops
    // idle slots): __thiscall(ECX = the unit's air-ops block or null), RET; EAX.
    virtual std::int32_t shipyard_count_idle_slots_008434d0(void* unit) = 0;
    virtual std::int32_t air_ops_count_idle_slots_006bdc30(void* block) = 0;

    // ---- 0066EA60 ----
    // 004F71D0 BSP_FrontEndScreen_Register: __thiscall(ECX = screen), RET.
    virtual void frontend_screen_register_004f71d0(void* screen) = 0;
    // 004C12B0 BSP_GuiManager_GetOrCreate: EAX = manager. 00AA5840 BSP_GuiManager_LoadPage:
    // __thiscall(ECX = manager; const NativeString*, dword, dword), RET 0Ch; EAX = page.
    virtual void* gui_manager_get_or_create_004c12b0() = 0;
    virtual void* gui_manager_load_page_00aa5840(void* manager, const void* name, std::int32_t a,
                                                 std::int32_t b) = 0;
    // Screen vtable +14h (the layout virtual, 0065DAA0 for this screen): __thiscall(ECX = screen).
    virtual void screen_vtable14_layout(void* screen) = 0;
    // 00AB64C0 BSP_GuiIcon_AddState: __thiscall(ECX = icon; texture, const float uv[4]), RET 8.
    virtual std::int32_t gui_icon_add_state_00ab64c0(void* icon, void* texture, const float* uv) = 0;
    // 00AA6740 BSP_GuiWidget_GetSize: __thiscall(ECX = widget), RET; EAX = &size[2].
    // 00AA6750 BSP_GuiWidget_GetResolvedPosition: __thiscall(ECX = widget; float out[3]), RET 4;
    // EAX = the position.
    virtual const float* gui_widget_get_size_00aa6740(void* widget) = 0;
    virtual const float* gui_widget_get_resolved_position_00aa6750(void* widget, float* out) = 0;
    // [00F8BBE8]: the sound sample cache. 00A83FD0 BSP_SoundSampleCache_AcquireByName:
    // __thiscall(ECX = cache; ref* out, const NativeString*), RET 8; EAX = out.
    virtual void* global_00f8bbe8() = 0;
    virtual void** sound_sample_cache_acquire_00a83fd0(void* cache, void** out, const void* name) = 0;
    // [00CE221C] InterlockedIncrement / [00CE2220] InterlockedDecrement (__stdcall, EAX), and
    // the released object's vtable +0h: __thiscall(ECX = object), no stack argument.
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* value) = 0;
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) = 0;
    virtual void refcounted_vtable0_release(void* object) = 0;
    // 004BCB80 BSP_NativeString_ReverseFindAnyByte: __thiscall(ECX = string; const char* set,
    // from), RET 8; EAX = position or -1.
    virtual std::int32_t native_string_rfind_any_004bcb80(void* string, const char* set, std::uint32_t from) = 0;
    // 00469840 BSP_NativeString_Substring: __thiscall(ECX = string; out, start, count), RET 0Ch;
    // EAX = out.
    virtual void* native_string_substring_00469840(const void* string, void* out, std::uint32_t start,
                                                   std::uint32_t count) = 0;
    // 00425E10 BSP_NativeString_Append: __thiscall(ECX = string; const NativeString*), RET 4.
    virtual void native_string_append_00425e10(void* string, const void* other) = 0;
    // 0043C130 BSP_NativeString_ConstructPrefixedHeader: __fastcall(ECX = out, EDX = const char*
    // prefix; const NativeString*), RET 4; EAX = out (= prefix + text).
    virtual void* native_string_prefixed_0043c130(void* out, const char* prefix, const void* text) = 0;
    // 00AB44B0 BSP_GUI_SetStateTexture: __thiscall(ECX = widget; state, const NativeString*
    // path, const float uv[4]), RET 0Ch.
    virtual void gui_set_state_texture_00ab44b0(void* widget, std::uint32_t state, const void* path,
                                                const float* uv) = 0;
    // Widget vtable +58h (set size): __thiscall(ECX = widget; const float size[2]), RET 4.
    // Widget vtable +8Ch: __thiscall(ECX = widget), RET.
    virtual void widget_vtable58_set_size(void* widget, const float* size) = 0;
    virtual void widget_vtable8c(void* widget) = 0;
    // 0066CC80 STL_Tree_Erase (subtree): __thiscall(ECX = tree; node), RET 4.
    virtual void support_tree_erase_0066cc80(void* tree, void* node) = 0;
    // 006952A0 BSP_Observer_UnregisterPair: __fastcall(ECX = observed, EDX = owner).
    virtual void observer_unregister_pair_006952a0(void* observed, void* owner) = 0;
    // 0066DF30 (seg27_cc13 support_sites_build_with_icons_0066df30): __fastcall(ECX = list,
    // EDX = float* depth).
    virtual void support_sites_build_with_icons_0066df30(void* list, float* depth) = 0;

    // ---- 00428420 ----
    // 00427D10 BSP_Aabb3_IncludePoint: __thiscall(ECX = box {min[3], max[3]}; const float*), RET 4.
    virtual void aabb_include_point_00427d10(float* box, const float* point) = 0;
    // 0085BF20 BSP_Aabb_Overlaps: __fastcall(ECX = a min, EDX = a max; b min, b max), RET 8; AL.
    virtual bool aabb_overlaps_0085bf20(const float* a_min, const float* a_max, const float* b_min,
                                        const float* b_max) = 0;
    // 00427FE0: the landing-group list iterator increment, __thiscall(ECX = iterator {owner,
    // node}), RET.
    virtual void group_iterator_increment_00427fe0(void* iterator) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld: __thiscall(ECX = entity), RET.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00413920 BSP_Matrix_Multiply4x4: __thiscall(ECX = left; out, right), RET 8; EAX = out.
    virtual const float* matrix_multiply_00413920(const void* left, float* out, const void* right) = 0;
    // 00428090 BSP_Geometry_SegmentIntersectsSphere: __fastcall(ECX = centre, EDX = start;
    // float radius, const float* end), RET 8; AL.
    virtual bool segment_intersects_sphere_00428090(const float* centre, const float* start, float radius,
                                                    const float* end) = 0;
    // 00480BF0: __thiscall(ECX = troop member entity), RET (the member's kill).
    // 00480C20: __thiscall(ECX = landing group; float damage), RET 4.
    virtual void troop_member_kill_00480bf0(void* member) = 0;
    virtual void landing_group_apply_damage_00480c20(void* group, float damage) = 0;
    // 00BD2FC0 BSP_RandomThreads_NextU32: ECX = stream; EAX.
    virtual std::uint32_t random_threads_next_u32_00bd2fc0(std::uint32_t stream) = 0;
    // 004B0F80 (soldier animation start): __thiscall(ECX = soldier; anim, random_start, dword),
    // RET 0Ch.
    virtual void soldier_start_animation_004b0f80(void* soldier, const void* anim, std::int32_t random_start,
                                                  std::int32_t arg) = 0;

    // ---- 004A0D20 (paths: point vector begin +8h / end +Ch, dword entries; end time +1Ch; the
    //      knots-derived byte +18h; list nodes 48h: next +0, prev +4, value +8..+47h) ----
    // 00BD2F10 BSP_Random_UniformFloatRange: __thiscall(ECX = stream; float lo, float hi), RET 8;
    // ST0. Every draw here is stream 0.
    virtual float random_uniform_float_00bd2f10(std::uint32_t stream, float lo, float hi) = 0;
    // 00BF55BE (thunk to operator new, used for new[]) and 00BF681B operator new: cdecl(size);
    // 00BF6989 (thunk to free): cdecl(pointer).
    virtual void* crt_operator_new_00bf55be(std::uint32_t size) = 0;
    virtual void* crt_operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void crt_free_00bf6989(void* p) = 0;
    // 00415870 STL_Vector_Size: __fastcall(ECX = path); EAX = (end - begin) >> 2, 0 for a null begin.
    virtual std::int32_t path_point_count_00415870(void* path) = 0;
    // 007B05E0: __thiscall(ECX = path; index), RET 4; ST0 = the float at entry[index] + 18h
    // (00BF6713 when index is out of range).
    virtual float path_knot_time_007b05e0(void* path, std::uint32_t index) = 0;
    // 007B03C0 BSP_CameraPath_SampleWorld: __thiscall(ECX = path; float t, float* position,
    // float* direction, int 1), RET 10h.
    virtual void camera_path_sample_world_007b03c0(void* path, float t, float* position, float* direction,
                                                   std::int32_t flag) = 0;
    // 007AFD60 BSP_CameraPath_SampleWorldLinear: __thiscall(ECX = path; float t, float* position,
    // float* direction), RET 0Ch.
    virtual void camera_path_sample_world_linear_007afd60(void* path, float t, float* position,
                                                          float* direction) = 0;
    // 007AF800 BSP_ScenePath_TransformPointToWorld: __thiscall(ECX = path; float* out, index), RET 8;
    // EAX = the point.
    virtual const float* scene_path_point_to_world_007af800(void* path, float* out, std::int32_t index) = 0;
    // 007AD180 BSP_PathPoints_SetPointLocal: __thiscall(ECX = path; index, const float*), RET 8.
    virtual void path_points_set_point_local_007ad180(void* path, std::int32_t index, const float* point) = 0;
    // 007AD330 BSP_PathPoints_AddPointLocal: __thiscall(ECX = path component; const float*), RET 4.
    virtual void path_points_add_point_local_007ad330(void* component, const float* point) = 0;
    // 007AF150 BSP_CameraPath_DeriveKnots: __thiscall(ECX = path), RET.
    virtual void camera_path_derive_knots_007af150(void* path) = 0;
    // 007B28C0 BSP_PathComponent_Construct: __thiscall(ECX = 2Ch-byte block; dword), RET 4; EAX = ECX.
    virtual void* path_component_construct_007b28c0(void* block, void* arg) = 0;
    // 00488E30 STL_List_BuyNode48: (ECX = list, not read; next, prev, const value*), RET 0Ch;
    // EAX = the new 48h node {next, prev, the 40h-byte value}.
    virtual void* list_buy_node48_00488e30(void* list, void* next, void* prev, const void* value) = 0;
    // 00490130 (list _Incsize, length_error over 3FFFFFFh): __thiscall(ECX = list; count), RET 4.
    virtual void list_incsize_00490130(void* list, std::uint32_t count) = 0;
    // 0049F6A0 (std::list::sort with a predicate; tagged CG_vector_deleting_dtor):
    // __thiscall(ECX = list; predicate code address), RET 4.
    virtual void list_sort_0049f6a0(void* list, std::uint32_t predicate_va) = 0;
    // 00B62D10 BSP_Vector4f_Transform: __thiscall(ECX = float[4]; float* out, const matrix), RET 8;
    // EAX = out.
    virtual const float* vector4f_transform_00b62d10(const float* vector, float* out, const void* matrix) = 0;
    // 0085DAD0 BSP_Matrix_OrthonormalizeFromRowYThenZ: __thiscall(ECX = float[16]), RET.
    virtual void matrix_orthonormalize_0085dad0(float* matrix) = 0;
    // 004134F0 BSP_Matrix_Copy4x4X87: __thiscall(ECX = destination; const float* source), RET 4.
    virtual void matrix_copy_004134f0(float* destination, const float* source) = 0;
    // The troop's +DCh unit, vtable +24Ch: __thiscall(ECX = unit; float* out, which), RET 8;
    // EAX = a point (only [0] and [2] are read).
    virtual const float* unit_vtable24c_point(void* unit, float* out, std::int32_t which) = 0;
    // The three floats at 00F8758C..00F87594 (a global direction copied into the matrix row 1).
    virtual const float* global_vector_00f8758c() = 0;

    // ---- 0059D4A0 (also 00AA6740 above) ----
    // 00AA8240 BSP_GuiWidget_SetResolvedPosition: __thiscall(ECX = widget; const float[3]), RET 4.
    virtual void gui_widget_set_resolved_position_00aa8240(void* widget, const float* position) = 0;
    // The icon's unit (+4h), vtable +C4h: __thiscall(ECX = unit; float out[3]), RET 4; EAX = the
    // position.
    virtual const float* unit_vtablec4_position(void* unit, float* out) = 0;
    // The two floats at 00E08890 / 00E08894 (the map screen offset added after the scale).
    virtual const float* map_offset_00e08890() = 0;
};

// 004C7AA0's 28h-byte result.
struct BorderZoneApproach {
    void* zone;           // +0  the 004C7730 record, or null (then nothing else is written)
    float direction[3];   // +4  004C7730's outward direction
    float inner_point[3]; // +10 the edge point moved `radius` against the normalised direction
    float heading[3];     // +1C normalize(2 * direction + the x/z unit vector from the inner
                          //     point to the zone's corner average)
};
static_assert(sizeof(BorderZoneApproach) == 0x28 || sizeof(void*) != 4);

// 004C7AA0 __thiscall(ECX = world; out, position, radius, side) RET 10h.
void world_border_zone_approach_004c7aa0(void* world, BorderZoneApproach* out, const float* position,
                                         float radius, std::int32_t side, Big1Host& host);

// 005AAC00 __thiscall(ECX = icon; template widget) RET 4; EAX = the number of distinct
// carried-class icon indices.
std::uint32_t hud_map_icon_refresh_carried_unit_icons_005aac00(void* icon, void* template_widget,
                                                               Big1Host& host);

// 0064AE20 __fastcall(ECX = ship screen), RET.
void hud_ship_screen45_apply_unit_0064ae20(void* screen, Big1Host& host);

// 006697E0 __fastcall(ECX = support manager screen), RET.
void hud_support_manager_collect_catalog_006697e0(void* screen, Big1Host& host);

// 0066A8A0 __fastcall(ECX = support manager screen), RET.
void hud_support_manager_rebuild_lists_0066a8a0(void* screen, Big1Host& host);

// 00428420 __thiscall(ECX = the projectile trace {segments[] of 18h bytes, count +C0h}; float),
// RET 4. The stack float (the caller's stream-1 draw at 0084C19F) is never read.
void projectile_trace_hit_landing_troops_00428420(void* trace, float unused, Big1Host& host);

// 0066EA60 __fastcall(ECX = support manager screen), RET: the screen's register (vtable +10h).
void hud_support_manager_screen_register_0066ea60(void* screen, Big1Host& host);

// 00674B80 __fastcall(ECX = support manager screen), RET: the screen's Enter.
void hud_support_manager_screen_enter_00674b80(void* screen, Big1Host& host);

// 0059D4A0 __thiscall(ECX = HUD map screen; icon), RET 4. cc13_big5ad's
// hud_map_icon_place_widget_0059d4a0 hook (callers 005B040C / 005B0420 in 005ADDA0).
void hud_map_icon_place_widget_0059d4a0(void* screen, void* icon, Big1Host& host);

// 004A0D20 __thiscall(ECX = landing troop), RET. Called only from 004A2FC0.
void landing_troop_update_004a0d20(void* troop, Big1Host& host);

// 005E12A0 __thiscall(ECX = lobby screen), RET.
void multi_game_lobby_load_weather_table_005e12a0(void* screen, Big1Host& host);

// 00641910 __thiscall(ECX = markers; unit, is_self, is_friendly, is_target), RET 10h. The
// sibling of 00642040 (cc13_pkt642) for the IsKindOf(1Ch) arm; every callee already has a
// Cc13Pkt642Host hook, and 0063C620 / 0063D1E0 are the pkt642 bodies.
void hud_markers_add_kind1c_marker_00641910(void* markers, void* unit, std::uint32_t is_self,
                                            std::uint32_t is_friendly, std::uint32_t is_target,
                                            Cc13Pkt642Host& host);

} // namespace bsp::cc13_big1
