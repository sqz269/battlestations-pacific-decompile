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

// 005E12A0 __thiscall(ECX = lobby screen), RET.
void multi_game_lobby_load_weather_table_005e12a0(void* screen, Big1Host& host);

// 00641910 __thiscall(ECX = markers; unit, is_self, is_friendly, is_target), RET 10h. The
// sibling of 00642040 (cc13_pkt642) for the IsKindOf(1Ch) arm; every callee already has a
// Cc13Pkt642Host hook, and 0063C620 / 0063D1E0 are the pkt642 bodies.
void hud_markers_add_kind1c_marker_00641910(void* markers, void* unit, std::uint32_t is_self,
                                            std::uint32_t is_friendly, std::uint32_t is_target,
                                            Cc13Pkt642Host& host);

} // namespace bsp::cc13_big1
