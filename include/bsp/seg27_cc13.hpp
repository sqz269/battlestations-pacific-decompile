// Packet cc13-seg27: raw functions of segment 27 (00656250..0067F4D0) that had neither a
// reconstruction record nor an implementation on main. docs/CC13_SEG27.md has the per-function
// table, the functions verified elsewhere and the ones skipped.
//
// Most of the segment is the support-manager screen (HUD slot 4Eh, vtable 00CF6B44, 480h bytes,
// register 0066EA60). Its containers are MSVC 8 checked std::list / std::map objects embedded in
// the screen: a list is {proxy +0h, head node +4h, size +8h}, a list node {next +0h, prev +4h,
// value +8h}; a checked iterator is {container +0h, node +4h}. Every _SECURE_SCL test the native
// makes calls 00BF6713 (the checked-iterator report, which returns) and carries on; those calls are
// kept at the points where the native makes them, through
// CheckedIteratorHost::checked_iterator_report_00bf6713. Tests the compiler emitted as `cmp
// reg,reg` (always equal) are not repeated.
//
// Model: the original objects are reached as raw bytes (ingame_gui_cc13::Raw, Win32 layout,
// image offsets); every call the native makes is a named host method (address in the name).
// Names are hypotheses from the strings, the callers and the fields touched, not recovered
// symbols. New C++ interfaces, not drop-in binary replacements; nothing here was run in bsp_game.
#pragma once

#include <cstdint>

#include "bsp/ingame_gui_cc13.hpp"

namespace bsp::seg27_cc13 {

using ingame_gui_cc13::Raw;

struct CheckedIteratorHost {
    virtual ~CheckedIteratorHost() = default;
    virtual void checked_iterator_report_00bf6713() = 0;
};

// The entity kind test every unit object answers: vtable +5Ch, __thiscall(entity, kind) -> AL.
struct EntityKindHost : CheckedIteratorHost {
    virtual bool entity_is_kind_5c(void* entity, int kind) = 0;
};

// A checked iterator as the native keeps it in two dwords (container, node).
struct CheckedIter {
    void* container;
    void* node;
};

// ---------------------------------------------------------------------------
// Session message 82h (vtable 00CF64C4): constructor and write. The read 006563A0 and the type
// test 006562F0 are in src/ingame_gui_cc13.cpp.
// ---------------------------------------------------------------------------

struct SessionMessage130Host : ingame_gui_cc13::SessionMessageHost {
    // 00429120, ECX = cursor, (word, bits).
    virtual void write_word_bits_00429120(void* cursor, std::uint16_t value, int bits) = 0;
};

inline constexpr std::uint8_t kSessionMessage130Type = 0x82;
inline constexpr std::uint32_t kSessionMessage130Vtable = 0x00CF64C4u;

// 00656280, __thiscall(msg, unused, a2, a3, entity, a5), RET 14h -> EAX msg.
Raw session_message130_construct_00656280(Raw m, std::uint32_t unused, std::uint32_t a2, std::uint32_t a3,
    void* entity, std::uint32_t a5, SessionMessage130Host& host);
// 00656320, __thiscall(msg, cursor), RET 4.
void session_message130_write_00656320(Raw m, void* cursor, SessionMessage130Host& host);

// ---------------------------------------------------------------------------
// 00656250: a locked dword read.
// ---------------------------------------------------------------------------
struct LockedReadHost {
    virtual ~LockedReadHost() = default;
    virtual void enter_critical_section(void* cs) = 0; // [00CE2218]
    virtual void leave_critical_section(void* cs) = 0; // [00CE2210]
};
// __fastcall(ECX = object), RET -> EAX.
std::uint32_t locked_read_50_00656250(Raw object, LockedReadHost& host);

// ---------------------------------------------------------------------------
// Support-manager screen: unit lists and the slot strip.
// ---------------------------------------------------------------------------

struct SupportStripHost : EntityKindHost {
    virtual void widget_set_visible_34(void* widget, bool visible) = 0;                // vtable +34h
    virtual void widget_set_frame_88(void* widget, std::uint16_t frame, int b, float alpha) = 0; // vtable +88h
};

// 0065A200, __fastcall(screen), RET -> AL: the icon kind of the selected unit (0Fh with the
// second list in use, 2 for kind 9, 15h for kind 45h, 14h otherwise, 0 with no units).
std::uint8_t support_manager_selected_icon_kind_0065a200(Raw s, EntityKindHost& host);
// 0065A4D0, __stdcall(CheckedIter it), RET 8 -> EAX: 0 (kind 45h), 1 (kind 9), 2 (kind 46h), -1.
std::int32_t support_unit_slot_kind_0065a4d0(CheckedIter it, EntityKindHost& host);
// 0065A550, __fastcall(screen), RET: fills the four slot icons +8Ch..+98h around the cursor.
void support_manager_update_slot_strip_0065a550(Raw s, SupportStripHost& host);
struct PayloadStripHost : SupportStripHost {
    // 00653200 BSP_Hud_PayloadIconFrame, ECX = screen, (entry) -> EAX frame.
    virtual std::int32_t payload_icon_frame_00653200(Raw s, void* entry) = 0;
};
// 0065A740, __fastcall(screen), RET.
void support_manager_update_payload_strip_0065a740(Raw s, PayloadStripHost& host);
// 0065AB40 / 0065AE90, __fastcall(screen), RET -> EAX: list +37Ch / +2E4h at index +310h.
void* support_manager_catalog_entry_0065ab40(Raw s, CheckedIteratorHost& host);
void* support_manager_catalog_entry_2e4_0065ae90(Raw s, CheckedIteratorHost& host);

struct SupportListRemoveHost {
    virtual ~SupportListRemoveHost() = default;
    // 0065B1F0, ECX = list, (const value&): std::list::remove.
    virtual void list_remove_0065b1f0(void* list, void* const& value) = 0;
};
// 0065BBA0, __thiscall(screen, entity), RET 4.
void support_manager_list_remove_0065bba0(Raw s, void* entity, SupportListRemoveHost& host);

struct SupportMapHost : CheckedIteratorHost {
    // 00654D50, ECX = &iterator: ++ on the map at +50h.
    virtual void tree_iterator_increment_00654d50(CheckedIter& it) = 0;
    // 0065CD40, ECX = map +414h, (&out, &key) -> EAX &out: map<u16,...>::find.
    virtual CheckedIter map_find_u16_0065cd40(void* map, const std::uint32_t& key) = 0;
    // 0057F5F0, ECX = &iterator: ++ on the map at +414h.
    virtual void tree_iterator_increment_0057f5f0(CheckedIter& it) = 0;
};
// 0065C9A0, __thiscall(screen, entity), RET 4: +334h from the map +50h, else entity+134h.
void support_manager_select_value_0065c9a0(Raw s, void* entity, SupportMapHost& host);
// 0065EE90, __fastcall(screen), RET.
void support_manager_sync_catalog_cursor_0065ee90(Raw s, SupportMapHost& host);
// 0065EFA0, __fastcall(screen), RET.
void support_manager_sync_weapon_cursor_0065efa0(Raw s, SupportMapHost& host);

struct SupportHealthHost : CheckedIteratorHost {
    // 00923BE0, ECX = unit -> ST0 (BSP_UnitInstance_GetHealth).
    virtual double unit_health_00923be0(void* unit) = 0;
    // 005AAAF0, ECX = destination holder, (source holder): copy of the reference holder that
    // follows the unit pointer in list +344h's values; 005A0B80, ECX = holder: its release.
    virtual void unit_ref_copy_005aaaf0(void* dst_holder, void* src_holder) = 0;
    virtual void unit_ref_release_005a0b80(void* holder) = 0;
    virtual void* controlled_unit() = 0; // *00E188D8
};
// 00668570, __fastcall(screen), RET -> ST0: the health shown for the selection.
double support_manager_selection_health_00668570(Raw s, SupportHealthHost& host);

struct SupportWeaponMapHost : CheckedIteratorHost {
    // 00665340, ECX = destination map, (source map): copy construction.
    virtual void map_copy_construct_00665340(void* dst, void* src) = 0;
    // 00664450: erase(begin, end) on a map; then 00BF65AC frees its head node.
    virtual void map_erase_range_00664450(void* map, void* first, void* last) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // ++ on the inner map (00654EB0) and the outer map +408h (006582D0), ECX = &iterator.
    virtual void tree_iterator_increment_00654eb0(CheckedIter& it) = 0;
    virtual void tree_iterator_increment_006582d0(CheckedIter& it) = 0;
};
// 00669170, __fastcall(screen), RET.
void support_manager_select_weapon_value_00669170(Raw s, SupportWeaponMapHost& host);

// ---------------------------------------------------------------------------
// Support sites: the lists of (site, units) pairs built from the game's site lists
// game+19CCh -> +16Ch (sites with a unit list at +784h, non-empty when +78Ch) and +3B8h (units
// with no carrier/site link at +344h). A pair node is {next, prev, site +8h, unit list
// {proxy +0Ch, head +10h, size +14h}}; the screen's pairs (list +344h, built by 0066DF30) add the
// map icon widget at +18h. A site of 0 marks a lone unit, the first element of its own list.
// ---------------------------------------------------------------------------
struct SupportSiteListHost : CheckedIteratorHost {
    virtual Raw game() = 0;               // *00E188A8
    virtual Raw ingame_interface() = 0;   // *00E198C4
    // std::list pieces, ECX = the list. _Buynode returns the node built from (next, prev, &value).
    virtual void* list_buy_head_0059fd60(void* list) = 0;
    virtual void* list_buy_head_006599c0(void* list) = 0;
    virtual void* pair_list_buy_node_0066b260(void* list, void* next, void* prev, const void* value) = 0;
    virtual void pair_list_inc_size_00663cc0(void* list, int count) = 0;
    virtual void* pair_list_buy_node_0066afa0(void* list, void* next, void* prev, const void* value) = 0;
    virtual void pair_list_inc_size_00663190(void* list, int count) = 0;
    virtual void pair_list_clear_006654c0(void* list) = 0;
    virtual void pair_list_clear_00665210(void* list) = 0;
    virtual void* unit_list_buy_node_005a0650(void* list, void* next, void* prev, const void* value) = 0;
    virtual void unit_list_inc_size_005a5880(void* list, int count) = 0;
    virtual void unit_list_tidy_005a0b80(void* list) = 0;
    // 00665D20, ECX = destination list, (first, last, site): assign from the site's +784h list.
    virtual void unit_list_assign_00665d20(void* list, CheckedIter first, CheckedIter last, void* site) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // 0084D050 BSP_SupportSite_CanSupplyClass, ECX = unit, (class) -> AL.
    virtual bool site_can_supply_class_0084d050(void* unit, std::int32_t cls) = 0;
    // GUI widgets.
    virtual void* widget_clone_subtree_00aab4c0(void* widget, int flag) = 0;
    virtual void widget_resolved_position_00aa6750(void* widget, float out[3]) = 0;
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float pos[3]) = 0;
    virtual void object_delete_04(void* object, int flags) = 0; // vtable +4h
};

// 0066DD00, __fastcall(ECX = pair list), RET: (re)builds a pair list without widgets.
void support_sites_build_0066dd00(void* list, SupportSiteListHost& host);
// 0066DF30, __fastcall(ECX = screen+338h list, EDX = float* depth), RET: deletes the old icons,
// rebuilds the pairs and clones one map icon per pair.
void support_sites_build_with_icons_0066df30(void* list, float* depth, SupportSiteListHost& host);
// 0066E2B0, __fastcall(ECX = side, EDX = pair list), (out list, class), RET 8.
void support_sites_filter_0066e2b0(std::int32_t side, void* pairs, void* out, std::int32_t cls,
    SupportSiteListHost& host);
// 0066E3C0, __fastcall(ECX = side, EDX = screen+338h), (out list, player index, float* depth),
// RET 0Ch.
void support_sites_rebuild_and_filter_0066e3c0(std::int32_t side, void* pairs, void* out, std::int32_t index,
    float* depth, SupportSiteListHost& host);
// 0066E510, __fastcall(ECX = side, EDX = out list), (class), RET 4.
void support_sites_collect_0066e510(std::int32_t side, void* out, std::int32_t cls, SupportSiteListHost& host);
// 0066E590, __fastcall(ECX = side, EDX = out list), (class), RET 4: 0066E510, then the pairs
// with an empty unit list are erased.
void support_sites_collect_nonempty_0066e590(std::int32_t side, void* out, std::int32_t cls,
    SupportSiteListHost& host);

struct SupportMapIconsHost : SupportSiteListHost {
    virtual void entity_refresh_world_00414db0(void* parent) = 0;           // ECX = parent
    // 00413920, ECX = entity+74h (local matrix), (out, parent+CCh) -> EAX out.
    virtual void* matrix_multiply_00413920(void* local, void* out, void* parent_world) = 0;
    virtual void matrix_copy_004134f0(void* dst, const void* src) = 0;       // ECX = dst
    // 00658580 BSP_HudSupportManagerScreen_WorldToMap, ECX = screen, (out[2], x, y, z).
    virtual void world_to_map_00658580(Raw s, float out[2], float x, float y, float z) = 0;
    virtual void widget_set_local_xy_00aa7d00(void* widget, float x, float y) = 0; // ECX = widget
};
// 0066FB00, __fastcall(screen), RET.
void support_manager_refresh_site_icons_0066fb00(Raw s, SupportMapIconsHost& host);

struct SupportDeckSlotHost : EntityKindHost {
    virtual void widget_set_visible_34(void* widget, bool visible) = 0;
    virtual void enter_critical_section(void* cs) = 0; // [00CE2218]
    virtual void leave_critical_section(void* cs) = 0; // [00CE2210]
    // 0065CA50 BSP_AirOpsSlotRecord_Assign, ECX = record, (source record); 00658530
    // BSP_AirOpsSlotRecord_IsOut, ECX = record -> AL.
    virtual void air_ops_slot_record_assign_0065ca50(void* record, void* source) = 0;
    virtual bool air_ops_slot_record_is_out_00658530(void* record) = 0;
};
// 00660190, __thiscall(screen, record buffer), RET 4 (Ghidra shows no parameter; the only call,
// 006779BF..006779C9 in 00675C40, pushes `lea ecx,[esp+D0h]`, and the tail call at 006604F7 reuses
// that argument slot).
void support_manager_select_deck_slot_00660190(Raw s, void* record, SupportDeckSlotHost& host);

// ---------------------------------------------------------------------------
// 006735D0: the support-manager screen's constructor (caller BSP_InGameInterface_Init)
// ---------------------------------------------------------------------------
struct SupportScreenConstructHost {
    virtual ~SupportScreenConstructHost() = default;
    virtual void front_end_screen_construct_004f7180(Raw screen) = 0; // ECX = screen
    // The container's _Buyheadnode, ECX = the container; `routine` is its address (006593A0,
    // 00659B90, ...), one per element type.
    virtual void* container_buy_head_node(std::uint32_t routine, void* container) = 0;
};
inline constexpr std::uint32_t kSupportManagerScreenVtable = 0x00CF6B44u;
// One embedded container: its offset, its _Buyheadnode, and for a tree the offset of the head's
// _Isnil byte (0 for a list).
struct SupportScreenContainer {
    std::uint16_t offset;
    std::uint32_t buy_head;
    std::uint8_t isnil;
};
// 006735D0, __fastcall(ECX = screen) -> EAX screen.
Raw support_manager_screen_construct_006735d0(Raw s, SupportScreenConstructHost& host);

struct SupportScreenDestructHost : CheckedIteratorHost {
    virtual void object_delete_04(void* object, int flags) = 0;          // vtable +4h
    virtual void tree_iterator_increment_00654650(CheckedIter& it) = 0;  // ECX = &iterator
    // 00659850, ECX = tree, (subtree root): _Erase.
    virtual void tree_erase_subtree_00659850(void* tree, void* root) = 0;
    // A tree's erase(begin, end), ECX = tree, (&result, first, last); `routine` is its address.
    virtual void tree_erase_range(std::uint32_t routine, void* tree, CheckedIter first, CheckedIter last) = 0;
    // 00665210, ECX = list: clear of the site-pair lists.
    virtual void pair_list_clear_00665210(void* list) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void* gui_manager_004c12b0() = 0;
    virtual void gui_remove_and_destroy_page_00aa31f0(void* manager, void* page) = 0; // ECX = manager
    virtual void observer_unregister_pair_006952a0(void* endpoint, void* link) = 0;     // ECX, EDX
    virtual void callback_owner_destroy_00695870(void* owner) = 0;                       // ECX = owner
    virtual long interlocked_decrement(long* value) = 0;
    virtual void object_virtual_00(void* object) = 0;                                     // vtable +0h
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;                     // ECX = screen
};
// 00672660, __fastcall(ECX = screen), RET (the body of the scalar deleting destructor 006739F0).
void support_manager_screen_destruct_00672660(Raw s, SupportScreenDestructHost& host);

// ---------------------------------------------------------------------------
// 0066A130: rebuild the selectable entries (lists +2E4h and +37Ch)
// ---------------------------------------------------------------------------
struct SupportEntriesHost : EntityKindHost {
    virtual void free_00bf65ac(void* block) = 0;
    virtual void support_collect_catalog_006697e0(Raw s) = 0;                 // ECX = screen
    // 00964790, ECX = class id, DL = 1 -> EAX the vehicle class (BSP_VehicleClass_GetOrCreate).
    virtual void* vehicle_class_get_or_create_00964790(std::uint32_t id, bool create) = 0;
    // _Buynode / _Incsize pairs, ECX = the list: +37Ch (00659CF0, 00663230), +394h (004857F0,
    // 0048FC80), +2E4h (00659C90, 006630F0).
    virtual void* list37c_buy_node_00659cf0(void* list, void* next, void* prev, const void* value) = 0;
    virtual void list37c_inc_size_00663230(void* list, int count) = 0;
    virtual void* list394_buy_node_004857f0(void* list, void* next, void* prev, const void* value) = 0;
    virtual void list394_inc_size_0048fc80(void* list, int count) = 0;
    virtual void* list2e4_buy_node_00659c90(void* list, void* next, void* prev, const void* value) = 0;
    virtual void list2e4_inc_size_006630f0(void* list, int count) = 0;
    // The icon-state map +42Ch (int key): find (00614060, ECX = map, (&result, &key)) and
    // operator[] (006171B0, ECX = map, (&key)) -> &value.
    virtual CheckedIter icon_map_find_00614060(void* map, const void* key) = 0;
    virtual std::uint32_t* icon_map_subscript_006171b0(void* map, const void* key) = 0;
    // 00AB64C0, ECX = icon, (texture, float uv[4]) -> EAX the new state's index.
    virtual std::uint32_t gui_icon_add_state_00ab64c0(void* icon, void* texture, const float uv[4]) = 0;
    // 00658A60, ECX = &iterator: checked operator* -> &value.
    virtual void** list_iterator_deref_00658a60(void* iterator) = 0;
    // The vector at unit+770h (20h-byte elements): size (00653470) and checked operator[]
    // (00653490), ECX = the vector.
    virtual std::uint32_t vector_size_00653470(void* vec) = 0;
    virtual void* vector_at_00653490(void* vec, std::uint32_t index) = 0;
    // 00669EE0, ECX = list +388h, (&list +37Ch): operator=. 00668290, ECX = list +2F0h,
    // (first, last, extra): assign from +2E4h; the fifth dword is the native's uninitialised
    // tag slot ([esp+20h]).
    virtual void list_assign_00669ee0(void* dst, void* src) = 0;
    virtual void list_assign_range_00668290(void* dst, CheckedIter first, CheckedIter last, std::uint32_t extra) = 0;
};
// 0066A130, __fastcall(screen), RET -> EAX 1 when either list has entries.
std::int32_t support_manager_build_entries_0066a130(Raw s, SupportEntriesHost& host);

// ---------------------------------------------------------------------------
// 0066D020: the selection refresh (callers: Enter 00674B80 and the update 00675C40)
// ---------------------------------------------------------------------------
struct SupportSelectionHost : EntityKindHost {
    virtual void widget_set_visible_34(void* widget, bool visible) = 0;
    virtual void widget_set_frame_88(void* widget, int frame, int b, float alpha) = 0;
    // Catalog entries (+2A0h / +3A4h) answer vtable +18h(flag).
    virtual bool entry_has_flag_18(void* entry, int flag) = 0;
    // 009552E0, ECX = entry, (index) -> EAX the weapon record; its +4h -> +8h -> +4h is a device
    // class id that 00443490 (ECX = id, DL = 0) resolves.
    virtual void* entry_weapon_009552e0(void* entry, std::int32_t index) = 0;
    virtual void* device_class_resolve_00443490(void* id, bool flag) = 0;
    // 00AB2690 BSP_GUI_SetStateTextureAndUV, ECX = widget, (state, texture, float uv[4]).
    virtual void gui_set_state_texture_uv_00ab2690(void* widget, int state, void* texture, const std::uint32_t uv[4]) = 0;
    // The screen's other routines, ECX = screen.
    virtual std::int32_t support_build_entries_0066a130(Raw s) = 0;
    virtual void support_update_payload_strip_0065a740(Raw s) = 0;
    virtual void support_sync_weapon_cursor_0065efa0(Raw s) = 0;
    virtual void* support_catalog_entry_2e4_0065ae90(Raw s) = 0;
    virtual void support_select_weapon_value_00669170(Raw s) = 0;
    virtual void support_build_hangar_loadout_ring_00666dc0(Raw s) = 0;
    virtual void support_highlight_hangar_loadout_00662210(Raw s) = 0;
    virtual void support_sync_catalog_cursor_0065ee90(Raw s) = 0;
    virtual void support_build_loadout_ring_00665f30(Raw s) = 0;
    virtual void support_refresh_info_panel_0066b6c0(Raw s) = 0;
    virtual void support_highlight_loadout_00662020(Raw s) = 0;
    // Native strings ({length, buffer}) and their inline release (00419CC0 pool, 00BD1510 return).
    virtual void native_string_from_cstr_0041e870(void* str, const char* text) = 0;
    virtual bool native_string_equals_insensitive_00435c40(void* str, void* other) = 0; // ECX = str
    virtual void native_string_pool_return(void* buffer, std::uint32_t bytes) = 0;
};
// 0066D020, __fastcall(screen), RET.
void support_manager_refresh_selection_0066d020(Raw s, SupportSelectionHost& host);

// ---------------------------------------------------------------------------
// 00673130: the support-manager info panel's per-frame state (caller BSP_InGameInterface_Update)
// ---------------------------------------------------------------------------
struct SupportInfoPanelHost {
    virtual ~SupportInfoPanelHost() = default;
    virtual Raw game() = 0;               // *00E188A8
    virtual Raw ingame_interface() = 0;   // *00E198C4
    virtual bool has_available_entries_0066ffa0(Raw s) = 0;             // ECX = screen
    virtual std::int32_t effective_game_mode_004bca50(Raw game) = 0;   // ECX = game
    virtual bool widget_is_visible_38(void* widget) = 0;                // vtable +38h
    virtual void widget_set_visible_34(void* widget, bool visible) = 0; // vtable +34h
    virtual void widget_set_size_58(void* widget, const float size[2]) = 0; // vtable +58h
    virtual const float* widget_size_00aa6740(void* widget) = 0;        // ECX = widget
    // Native strings (8-byte {length, buffer} objects in the caller's frame).
    virtual void native_string_from_cstr_0041e870(void* str, const char* text) = 0;
    virtual void native_string_from_int_004260b0(void* str, std::int32_t value) = 0;
    // 004261A0, ECX = left, (out, right) -> EAX out.
    virtual void* native_string_concat_004261a0(void* left, void* out, void* right) = 0;
    virtual void native_string_destroy_0041dd20(void* str) = 0;
    virtual void text_set_localised_source_00abaed0(void* text, void* str, bool flag) = 0; // ECX = text
    // Support points of a player (ECX = player index) -> ST0, and the cap; 00BF7420 is _ftol2.
    virtual double support_points_00946f00(std::int32_t player) = 0;
    virtual double support_points_cap_00942130(std::int32_t player) = 0;
    virtual std::int32_t ftol_00bf7420(double value) = 0;
};
// 00673130, __fastcall(screen), RET.
void support_manager_update_info_panel_00673130(Raw s, SupportInfoPanelHost& host);

// ---------------------------------------------------------------------------
// 0065D070: the spawn request's member orders (caller 00947A30, BSP_SpawnRequest_CreateMembers).
// ---------------------------------------------------------------------------
struct SpawnMemberOrdersHost {
    virtual ~SpawnMemberOrdersHost() = default;
    virtual bool entity_is_kind_5c(void* entity, int kind) = 0;
    // 0077D600, ECX = entity, (command descriptor, target block, 1).
    virtual void entity_issue_command_0077d600(void* entity, std::uint32_t command, void* target, int flag) = 0;
    // 00465080, ECX = out descriptor, (entity, float) -> EAX out.
    virtual void* command_target_from_entity_00465080(void* out, void* entity, float value) = 0;
    // 006952A0, ECX = entity, EDX = observer link.
    virtual void observer_unregister_pair_006952a0(void* entity, void* link) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // The read-only zero vector 00F87574/78/7C.
    virtual void zero_vector_00f87574(float out[3]) = 0;
};
// __thiscall(request, std::vector<entity*> by value {proxy, first, last, end}), RET 10h.
void spawn_request_order_members_0065d070(Raw request, std::uint32_t proxy, void** first, void** last,
    void** end, SpawnMemberOrdersHost& host);

// ---------------------------------------------------------------------------
// Bomb / torpedo view (HUD interface 26h, INTF_BOMBVIEW; the view object is vtable 00CF7928)
// ---------------------------------------------------------------------------
struct BombViewHost {
    virtual ~BombViewHost() = default;
    virtual Raw game() = 0;               // *00E188A8
    virtual Raw ingame_interface() = 0;   // *00E198C4
    virtual bool entity_is_kind_5c(void* entity, int kind) = 0;
    virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
    virtual void memset_00bf79f0(void* dst, int value, std::uint32_t size) = 0;
    virtual void* bomb_view_camera_construct_00435280(void* memory) = 0;      // ECX = memory
    virtual void* torpedo_view_camera_construct_00435120(void* memory) = 0;   // ECX = memory
    // 00923870, ECX = camera, (0, game+19CCh, &matrix 4x4).
    virtual void camera_attach_00923870(void* camera, int a, void* scene, const float matrix[16]) = 0;
    // 00694A60 BSP_Observer_RegisterPair, ECX = endpoint, EDX = view+8h.
    virtual void observer_register_pair_00694a60(void* endpoint, void* link) = 0;
    virtual void bomb_view_set_target_004307f0(void* camera, void* target, std::uint32_t launcher) = 0;
    virtual void torpedo_view_set_target_0042fee0(void* camera, void* target) = 0;
    // 004BC410 BSP_Game_SetCameraMover, ECX = game.
    virtual void game_set_camera_mover_004bc410(Raw game, void* camera) = 0;
    virtual void entity_refresh_world_00414db0(void* entity) = 0;
    // 0067B7E0's services.
    virtual float game_time_00f876a4() = 0;
    virtual Raw input_manager_004bec00() = 0;
    virtual bool apply_pending_level_change_0068a9c0(Raw iface) = 0;   // ECX = interface
    virtual void force_select_unit_006485a0(void* root) = 0;           // ECX = (*00E198C4)+40h
    virtual Raw game_tuning_0042e740() = 0;
    // 00B0D6A0, ECX = *00F8D39C, (three tuning floats, velocity x, y, z).
    virtual void render_set_transition_00b0d6a0(float a, float b, float c, float vx, float vy, float vz) = 0;
};
// 0067B4D0, __thiscall(view, target, launcher), RET 8.
void bomb_view_ensure_camera_0067b4d0(Raw view, void* target, std::uint32_t launcher, BombViewHost& host);
// 0067B7E0, __thiscall(view, float dt), RET 4 (vtable 00CF7928 slot +20h).
void bomb_view_update_0067b7e0(Raw view, float dt, BombViewHost& host);

// ---------------------------------------------------------------------------
// Front end
// ---------------------------------------------------------------------------
struct FrontEndRegisterHost {
    virtual ~FrontEndRegisterHost() = default;
    virtual void front_end_screen_register_004f71d0(Raw screen) = 0;
};
// 0067B390, __thiscall(screen): `jmp 004F71D0` (HUD slot 24h's register).
void hud_screen24_register_0067b390(Raw screen, FrontEndRegisterHost& host);

struct PrimaryInputHost : CheckedIteratorHost {
    virtual Raw input_backend() = 0;                                  // *00F8BBF4
    virtual void input_remove_device_00a90ee0(Raw backend, void* device) = 0; // ECX = backend
    virtual void input_configure_class_00a917e0(Raw backend, int a, int b, int c) = 0;
    virtual void write_global_u8(std::uint32_t address, std::uint8_t value) = 0;
};
// 0067C970, no arguments, RET.
void rebind_primary_input_0067c970(PrimaryInputHost& host);

} // namespace bsp::seg27_cc13
