#pragma once
// Frontend and HUD menu screens, segments 20 (005805A0-005C5600) and 23
// (005E09A0-00604A50), packet cc13-fe1, docs/CC13_FE_1.md. Reconstructions of
// routines the naming passes (docs/CC13_FRONTEND_MENUS.md, docs/CC13_BAND_0050_0059.md,
// cc13-mopup1, cc13-ng2) named but left without source.
//
// Names are hypotheses, not recovered symbols. Screens and records are raw native
// storage: the routines read and write the fixed offsets the image uses. Engine
// routines that are not reconstructed stay behind host interfaces whose methods
// carry the native address. Widgets are native pointers passed as 32-bit values.
//
// Status: reconstructed and build-tested; not ABI-compatible (the native
// __thiscall/__fastcall register conventions are documented per routine, not
// reproduced), not fixture-tested, not game-validated.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "bsp/frontend_menus.hpp"
#include "bsp/native_string.hpp"

namespace bsp::cc13_fe1 {

// ---------------------------------------------------------------------------
// Screen vtable leaves
// ---------------------------------------------------------------------------

// Slot +00h of each screen's primary vtable: MOV EAX, imm32; RET. The value is
// the index into the 95-slot screen registry at 00E18B60 that
// BSP_FrontEndScreen_Register (004F71D0) fills.
struct ScreenIdSlot {
    std::uint32_t address;
    std::int32_t screen_id;
    std::string_view screen;
};
inline constexpr std::array<ScreenIdSlot, 12> kScreenIdSlots{{
    {0x00590570u, 0x01, "MainMenuScreen"},
    {0x005A8EE0u, 0x4C, "HudMapScreen"},
    {0x005B9E90u, 0x33, "HudNarrativeScreen"},
    {0x005C06B0u, 0x35, "HudMinimapScreen"},
    {0x005E10A0u, 0x16, "MultiGameLobbyScreen"},
    {0x005E6E00u, 0x10, "MultiMainMenuScreen"},
    {0x005EA930u, 0x11, "MultiModeSelectorScreen"},
    {0x005EDFD0u, 0x39, "HudObjectivesScreen"},
    {0x005F1CA0u, 0x0D, "OptionsScreen"},
    {0x005F9460u, 0x3B, "HudOrderScreen"},
    {0x005FFA80u, 0x3C, "HudPauseScreen"},
    {0x006047A0u, 0x3D, "HudPauseCheatScreen"},
}};
// Returns the slot's immediate, or -1 for an address that is not a screen-id leaf.
std::int32_t screen_id_slot(std::uint32_t address) noexcept;

// MOV AL, 1; RET leaves. Slot +08h ("occludes lower levels", docs/FRONTEND_SCREEN_SETS.md)
// for 005A8EF0 and the self-managed slot for the other three.
inline constexpr std::array<std::uint32_t, 4> kReturnTrueSlots{
    0x005A8EF0u, 0x005B9EA0u, 0x005FFA90u, 0x006047B0u};
bool screen_return_true_slot(std::uint32_t address) noexcept;

// Empty leaves: a single RET or RET n. The stack bytes released are the slot's
// argument size (8 for the listbox notifications (listbox, row), 4 for update(dt)).
struct NoOpSlot {
    std::uint32_t address;
    std::uint8_t ret_bytes;
};
inline constexpr std::array<NoOpSlot, 15> kNoOpSlots{{
    {0x00590580u, 0}, {0x00590590u, 0}, {0x005905A0u, 0}, {0x005905B0u, 0}, {0x005905C0u, 0},  // 00CEFC24 +10h..+20h
    {0x005C27F0u, 0}, {0x005C2800u, 0}, {0x005C2830u, 8}, {0x005C2840u, 8},  // MissionTreeScreen
    {0x005ED5C0u, 0}, {0x005EDFE0u, 8}, {0x005EDFF0u, 8},                    // HudObjectivesScreen
    {0x006047C0u, 0}, {0x006047D0u, 0}, {0x00604800u, 4},                    // HudPauseCheatScreen
}};
// The RET n byte count, or -1 for an address not in the table.
std::int32_t screen_noop_slot(std::uint32_t address) noexcept;

// Adjustor thunks on secondary bases: SUB ECX, delta; JMP target.
struct AdjustorThunk {
    std::uint32_t address;
    std::uint32_t delta;
    std::uint32_t target;
};
inline constexpr std::array<AdjustorThunk, 9> kAdjustorThunks{{
    {0x005905D0u, 0x08, 0x00590D60u},  // MainMenuScreen scalar deleting dtor
    {0x005A8F00u, 0x64, 0x005AB140u},  // HudMapScreen, base at +64h
    {0x005A8F10u, 0x08, 0x005AB140u},
    {0x005C06C0u, 0x08, 0x005C0910u},  // HudMinimapScreen
    {0x005E10B0u, 0x08, 0x005E11D0u},  // MultiGameLobbyScreen
    {0x005E6E10u, 0x08, 0x005E7140u},  // MultiMainMenuScreen
    {0x005EA950u, 0x08, 0x005EADA0u},  // MultiModeSelectorScreen
    {0x005EE000u, 0x08, 0x005EE100u},  // HudObjectivesScreen
    {0x005FFAB0u, 0x08, 0x005FFD80u},  // HudPauseScreen
}};
struct ScalarDeletingDtorHost {
    virtual ~ScalarDeletingDtorHost() = default;
    // The target scalar deleting destructor, __thiscall(object, flags), RET 4.
    virtual void* scalar_deleting_dtor(std::uint32_t target, void* object, std::uint32_t flags) = 0;
};
// Rebases the secondary-base pointer to the full object and forwards. Returns
// what the target returns; null for an address not in kAdjustorThunks.
void* adjustor_thunk(std::uint32_t address, void* secondary, std::uint32_t flags,
    ScalarDeletingDtorHost& host);

// Child/observer collectors: __thiscall(screen, list), RET 4. Each pushes
// &screen + offset, in table order, onto the caller's pointer vector through
// STL_Vector_PushBack_004d6790 (ECX = list).
struct ChildCollector {
    std::uint32_t address;
    std::array<std::uint16_t, 3> offsets;  // 0 ends the list
};
inline constexpr std::array<ChildCollector, 12> kChildCollectors{{
    {0x005905E0u, {0x470, 0, 0}},          // MainMenuScreen FillActiveList (slot FillList)
    {0x005A8F20u, {0x08C, 0x090, 0}},      // HudMapScreen
    {0x005BB0F0u, {0x020, 0x024, 0}},      // HudNarrativeScreen
    {0x005C0800u, {0x01C, 0, 0}},          // HudMinimapScreen
    {0x005E10F0u, {0x014, 0x018, 0}},      // MultiGameLobbyScreen
    {0x005E7AE0u, {0x018, 0x01C, 0}},      // MultiMainMenuScreen
    {0x005ECB70u, {0x014, 0x018, 0x01C}},  // MultiModeSelectorScreen
    {0x005EE510u, {0x010, 0, 0}},          // HudObjectivesScreen
    {0x005F6000u, {0x07C, 0x080, 0}},      // OptionsScreen
    {0x005FAD60u, {0x008, 0, 0}},          // HudOrderScreen
    {0x005FFAC0u, {0x014, 0x018, 0x01C}},  // HudPauseScreen
    {0x00604950u, {0x008, 0, 0}},          // HudPauseCheatScreen
}};
struct PointerVectorHost {
    virtual ~PointerVectorHost() = default;
    // STL_Vector_PushBack_004d6790, __thiscall(vector, const void* value), RET 4.
    virtual void push_back_004d6790(void* vector, void* value) = 0;
};
// Returns false for an address not in the table.
bool collect_children(std::uint32_t address, void* screen, void* list, PointerVectorHost& host);

// Listener / layout thunks that recapture GuiScroller base positions
// (BSP_GuiScroller_RecaptureBasePositions 006833F0, ECX = scroller).
struct ScrollerHost {
    virtual ~ScrollerHost() = default;
    virtual void recapture_base_positions_006833f0(void* scroller) = 0;
    // 005D6E30, ECX = lobby screen: BSP_MultiGameLobbyScreen_BuildMapPreview.
    virtual void lobby_build_map_preview_005d6e30(void* screen) = 0;
    // 005EA030, ECX = multi main menu screen: LayoutAchievementLocks.
    virtual void multi_main_layout_achievement_locks_005ea030(void* screen) = 0;
};
// 005EA940 MultiModeSelector listener +1Ch and 005FFAA0 HudPause listener +1Ch:
// ADD ECX, 30h; JMP 006833F0. ECX is the listener sub-object; the scroller sits
// 30h above it.
void listener_recapture_scroller_30(void* listener, ScrollerHost& host);
// 005E10C0 MultiGameLobby listener (ECX = screen+0Ch): BuildMapPreview on the
// screen, then recapture the scrollers at listener+14Ch, +C0h and +24h.
void lobby_listener_on_layout_changed_005e10c0(void* listener, ScrollerHost& host);
// 005EA700 MultiMainMenu listener +1Ch (ECX = screen+0Ch): LayoutAchievementLocks
// on the screen, then recapture the scroller at listener+60h.
void multi_main_listener_current1c_005ea700(void* listener, ScrollerHost& host);

// ---------------------------------------------------------------------------
// Native containers (MSVC/Dinkumware shapes over raw storage)
// ---------------------------------------------------------------------------

struct ContainerHost {
    virtual ~ContainerHost() = default;
    // The per-instantiation _Erase(subtree) routine named in the tables below.
    virtual void erase_subtree(std::uint32_t routine, void* node) = 0;
    // The per-instantiation erase(first, last) routine, __thiscall(tree, out
    // iterator, first_tree, first, last_tree, last).
    virtual void erase_range(std::uint32_t routine, void* tree, void* first, void* last) = 0;
    // The per-instantiation tree copy (_Copy of the source's root under this head).
    virtual void copy_tree(std::uint32_t routine, void* tree, const void* source) = 0;
    // The per-instantiation head-node allocator (blank node with the nil byte clear).
    virtual void* allocate_head(std::uint32_t routine) = 0;
    // operator new 00BF6A3E / delete 00BF65AC (CRT _free).
    virtual void* operator_new(std::size_t size) = 0;
    virtual void free_block(void* block) = 0;
    // The value constructor a buy-node routine calls on node+0Ch.
    virtual void construct_value(std::uint32_t routine, void* value, const void* source) = 0;
};

// Clear: erase the subtree under the root, then reset root, size, left, right in
// that order. Tree header at `tree` (+04h head, +08h size); node links +00h left,
// +04h parent (root for the head), +08h right.
struct TreeClear {
    std::uint32_t address;
    std::uint32_t erase_subtree;
};
inline constexpr std::array<TreeClear, 11> kTreeClears{{
    {0x00583720u, 0x00582B00u}, {0x00583800u, 0x00582B80u}, {0x005838E0u, 0x00582C00u},
    {0x005839C0u, 0x00582C80u}, {0x00583AA0u, 0x00582D00u}, {0x0058BD10u, 0x0058B520u},
    {0x0058D780u, 0x0058BCD0u}, {0x00591870u, 0x005914B0u}, {0x00591A00u, 0x00591800u},
    {0x00592AE0u, 0x00592A20u}, {0x00593060u, 0x00592EA0u},
}};
bool tree_clear(std::uint32_t address, void* tree, ContainerHost& host);

// Destroy: erase [head->left, head), free the head, head = 0, size = 0. The tree
// sits at object+04h (Destroy) or object+10h (DestroyAt10).
struct TreeDestroy {
    std::uint32_t address;
    std::uint32_t erase_range;
    std::uint8_t tree_offset;
};
inline constexpr std::array<TreeDestroy, 12> kTreeDestroys{{
    {0x0058F420u, 0x0058AD40u, 0x04}, {0x0058F480u, 0x0058AD40u, 0x04},
    {0x00591F70u, 0x00591A30u, 0x04}, {0x005920B0u, 0x00591C00u, 0x04},
    {0x00590CE0u, 0x0058AD40u, 0x10}, {0x00590D20u, 0x0058AD40u, 0x10},
    {0x00591150u, 0x0058AD40u, 0x10}, {0x00591190u, 0x0058AD40u, 0x10},
    {0x00592260u, 0x00591A30u, 0x10}, {0x00592520u, 0x00591A30u, 0x10},
    {0x00592600u, 0x00591C00u, 0x10}, {0x00592A90u, 0x00591C00u, 0x10},
}};
bool tree_destroy(std::uint32_t address, void* object, ContainerHost& host);

// 0058FFE0, 00590AD0 (identical): operator=, __thiscall(tree, source), RET 4.
// Unless self-assignment, erase [begin, end) with STL_MissionObjectiveTree_EraseRange
// (0058F100) and copy the source tree in (STL_Tree_CopyTree_0058f1d0). Returns tree.
inline constexpr std::uint32_t kObjectiveTreeEraseRange = 0x0058F100u;
inline constexpr std::uint32_t kObjectiveTreeCopy = 0x0058F1D0u;
void* objective_tree_assign(void* tree, const void* source, ContainerHost& host);

// Empty-tree constructors. 00583360: head from 005826B0, nil byte at head+19h.
// 00590080: the same, then copy the source (STL_Tree_CopyTree_0058d930).
void warning_tree28_construct_00583360(void* tree, ContainerHost& host);
void* tree_copy_construct_00590080(void* tree, const void* source, ContainerHost& host);
// 005840B0: list header {+04h sentinel from 00582280, +08h size 0}; returns the list.
void* objective_widget_list_construct_005840b0(void* list, ContainerHost& host);

// 00583120: std::list destroy. Unlink every node from the sentinel first
// (sentinel->next = sentinel->prev = sentinel, size 0), free each node, free the
// sentinel and clear +04h.
void list_destroy_00583120(void* list, ContainerHost& host);

// 00582ED0: uninitialized copy of 8-byte PODs, __fastcall(first, last) + stack
// dest; a null destination slot is skipped but still advanced.
void pod8_uninitialized_copy_00582ed0(const std::uint32_t* first, const std::uint32_t* last,
    std::uint32_t* dest) noexcept;

// Tree buy-node routines: operator new(size); unless null: left, parent, right,
// the value constructed at +0Ch from the source, then colour byte and nil byte 0.
struct TreeBuyNode {
    std::uint32_t address;
    std::uint16_t node_size;
    std::uint16_t colour_offset;  // the nil byte follows it
    std::uint32_t value_ctor;
};
inline constexpr std::array<TreeBuyNode, 7> kTreeBuyNodes{{
    {0x00589A90u, 0x1C, 0x18, 0x00589480u},   // Rec1C value (see rec1c_construct_00589480)
    {0x0058B1A0u, 0x24, 0x20, 0x005894F0u},   // two-string value
    {0x00590BA0u, 0x20, 0x1C, 0x0058D4B0u},   // {key dword, nested tree}
    {0x00590C40u, 0x20, 0x1C, 0x0058D4B0u},
    {0x00592180u, 0x20, 0x1C, 0x00591D90u},
    {0x00592560u, 0x20, 0x1C, 0x00591E20u},
    {0x005944A0u, 0x2A0, 0x29C, 0x00594340u},  // mission score pair
}};
void* tree_buy_node(std::uint32_t address, void* left, void* parent, void* right,
    const void* value, std::uint8_t colour, ContainerHost& host);

// 0058A980: vector<uint16_t> sized constructor body, __thiscall(vector, n), RET 4.
// Clears +04h/+08h/+0Ch; n == 0 returns false; n > 7FFFFFFFh throws through
// 00589620; else first = last = allocate(n) (005804B0, ECX = n) and end = first + 2n.
struct ShortVectorHost {
    virtual ~ShortVectorHost() = default;
    virtual void* allocate_shorts_005804b0(std::uint32_t count) = 0;
    [[noreturn]] virtual void throw_length_error_00589620() = 0;
};
bool short_vector_construct_0058a980(void* vector, std::uint32_t count, ShortVectorHost& host);

// 0059FA40: pointer array {+00h data, +04h size, +08h capacity} push_back.
// Full: reserve(max(2 * capacity, 1)) through BSP_PointerArray_Reserve; a null
// data slot is skipped; size always increments.
struct PointerArrayHost {
    virtual ~PointerArrayHost() = default;
    virtual void pointer_array_reserve(void* array, std::int32_t capacity) = 0;
};
void pointer_array_push_back_0059fa40(void* array, const std::uint32_t* value, PointerArrayHost& host);

// 005B7840: intrusive owned-object list {+00h count, +04h head, +08h tail};
// nodes {+00h prev, +04h next, +08h object}. Pops the head until empty: deletes
// the object through its vtable slot 0 (flags 1), unlinks, frees the node.
struct OwnedListHost {
    virtual ~OwnedListHost() = default;
    virtual void delete_object(void* object) = 0;  // (*vtable[0])(1)
    virtual void free_block(void* block) = 0;
};
void owned_object_list_clear_005b7840(void* list, OwnedListHost& host);

// 005F4A20: frees the two vectors of an options page (+14h..+1Ch, then
// +04h..+0Ch), each data pointer through _free when non-null, zeroing all six.
void option_page_tidy_005f4a20(void* page, ContainerHost& host);

// 005B9C40: BSP_EffectSoundSamples_Resize(vector, n, 0) then _free(data). The
// second argument is EDX at the native call (__fastcall).
struct SampleVectorHost {
    virtual ~SampleVectorHost() = default;
    virtual void resize(void* vector, std::uint32_t count, std::uint32_t fill) = 0;
    virtual void free_block(void* block) = 0;
};
void effect_sound_samples_tidy_005b9c40(void* vector, std::uint32_t count, SampleVectorHost& host);

// ---------------------------------------------------------------------------
// Native strings and small records
// ---------------------------------------------------------------------------

// 005840F0, 00584480: destroy an 8-byte native string header at object+00h
// (the 0041DD20 body inlined). 005FD540: the same for a header at object+04h.
void native_string_destroy_at(void* object, std::size_t offset, NativeStringStorage& storage) noexcept;

// String-plus-tail copy constructors, __thiscall(dst, src[, tail]) returning dst.
// The header is zeroed, the copy fragment of 00BE0A30 runs unless dst == src,
// then the tail is copied:
//   00585F20: byte from a separate source pointer (third argument) at +08h
//   00585FA0: byte at +08h          00586070: dword at +08h
//   00586150: the source string sits at src+68h and there is no tail
//             (BSP_PlayerProfile_CopyMissionId68, ECX = profile, out = stack arg)
void* string_byte_construct_00585f20(void* dst, const void* src, const std::uint8_t* tail,
    NativeStringStorage& storage);
void* string_byte_copy_00585fa0(void* dst, const void* src, NativeStringStorage& storage);
void* string_dword_copy_00586070(void* dst, const void* src, NativeStringStorage& storage);
void* profile_copy_mission_id_00586150(const void* profile, void* out, NativeStringStorage& storage);
// 005894F0: two native strings (+00h, +08h) and a dword at +10h.
void* string_pair_copy_005894f0(void* dst, const void* src, NativeStringStorage& storage);
// 00589480: Rec1C {+00h..+08h three dwords, +0Ch string copied from src (and the
// src byte at +08h of that record into +14h), +18h byte, +19h byte 0}.
void* rec1c_construct_00589480(void* dst, std::uint32_t a, std::uint32_t b, std::uint32_t c,
    const void* string_record, std::uint8_t flag, NativeStringStorage& storage);

// 00588640: copy constructor of a 3Ch-byte record whose +04h..+1Bh is a legacy
// small-buffer string (capacity +18h = 0Fh, size +14h = 0, first byte 0, then
// assign(src, 0, npos) through 00440D90-style AssignSubstring), followed by the
// dwords +20h..+2Ch, byte +30h, dword +34h and bytes +38h..+3Bh.
struct LegacyStringHost {
    virtual ~LegacyStringHost() = default;
    virtual void assign_substring(void* dst_string, const void* src_string, std::uint32_t pos,
        std::uint32_t count) = 0;
};
void* legacy_sbo_record_copy_00588640(void* dst, const void* src, LegacyStringHost& host);

// 005B69B0: find_first_of over a native string, __thiscall(string, set, pos), RET 8.
// -1 when the string's data or length is zero, the set is null, or pos > length;
// a negative pos is treated as 0; strcspn from pos; reaching the end is -1.
std::int32_t native_string_find_first_of_005b69b0(const void* string, const char* set, std::int32_t pos) noexcept;

// 005B7FA0: zero five dwords (+04h..+10h first, then +00h).
void record14_clear_005b7fa0(void* record) noexcept;

// 005B7F60, 005B7B40 (identical): observer slot at +14h. When the new target
// differs: unregister from the old one if set, store, register with the new one
// if non-null. Returns the slot owner.
struct ObserverHost {
    virtual ~ObserverHost() = default;
    virtual void unregister_pair(void* owner) = 0;  // BSP_Observer_UnregisterPair
    virtual void register_pair(void* owner) = 0;    // BSP_Observer_RegisterPair
};
void* observer_slot_set(void* owner, std::uint32_t target, ObserverHost& host);

// 0059BD20: optional vec3 {+01h present, +08h value}; out = value or the
// default at 00F87574.
void optional_vector3_get_or_default_0059bd20(const void* optional, const float* fallback_00f87574,
    float* out) noexcept;

// 005EF4D0: wraps an index into [lo, count-1]: below lo -> count-1, above
// count-1 -> lo. __fastcall(value, lo) + stack count.
std::int32_t int_wrap_into_range_005ef4d0(std::int32_t value, std::int32_t lo, std::int32_t count) noexcept;

// 005F0220: out = native string from table[index]. ECX = out, EDX = table,
// stack index, RET 4 (BSP_NativeString_Assign 0041E870).
void* options_row_label_at_005f0220(void* out, const char* const* table, std::int32_t index,
    NativeStringRawPoolContext& context);

// 005F97A0: true for session message types BCh, 49h and 46h.
bool session_message188_is_type_005f97a0(std::int32_t type) noexcept;

// Two-byte "pending" tests: 005F9410 (+156h, +157h) and 005FC620 (+81h, +82h).
bool hud_order_screen_has_pending_order_005f9410(const void* screen) noexcept;
bool hud_order_menu_is_open_005fc620(const void* menu) noexcept;
// 005F9480 HudOrderScreen exit: +81h = +82h = 0.
void hud_order_screen_exit_005f9480(void* screen) noexcept;

// 006049F0: the plane's ground-contact owner (+BF4h) when the byte +BF8h is set.
std::uint32_t plane_ground_contact_owner_006049f0(const void* plane) noexcept;

// 005FC640: prompt callback, ECX = result: result 1 sets the quit byte 00E1AE75.
void prompt_result_set_quit_flag_005fc640(std::int32_t result, std::uint8_t& quit_00e1ae75) noexcept;

// ---------------------------------------------------------------------------
// Entity casts and simple unit queries
// ---------------------------------------------------------------------------

struct EntityHost {
    virtual ~EntityHost() = default;
    // Entity vtable +5Ch: IsKindOf(class id).
    virtual bool is_kind(std::uint32_t entity, std::int32_t class_id) = 0;
};
// 005F9A30: the unit when it is a squadron (kind 18h), else null.
std::uint32_t unit_cast_to_squadron_005f9a30(std::uint32_t unit, EntityHost& host);

// 0059AC50: health shown on the map, __stdcall(unit), RET 4, result in ST0.
// A squadron (kind 18h) reads its lead unit at +3D0h; otherwise an airfield
// (kind 45h) answers BSP_AirField_HealthFraction (006D2560, ECX = unit); any
// other non-null unit answers BSP_UnitInstance_GetHealth (00923BE0, ECX = unit);
// a squadron without a lead returns 0.
struct UnitHealthHost : EntityHost {
    virtual float airfield_health_fraction_006d2560(std::uint32_t unit) = 0;
    virtual float unit_instance_health_00923be0(std::uint32_t unit) = 0;
};
float unit_hud_health_0059ac50(std::uint32_t unit, UnitHealthHost& host);

// ---------------------------------------------------------------------------
// Widgets: show/hide fan-outs and focus styling
// ---------------------------------------------------------------------------

struct WidgetHost {
    virtual ~WidgetHost() = default;
    // Widget vtable +34h(bool): show or hide.
    virtual void set_shown(std::uint32_t widget, bool shown) = 0;
    // Widget vtable +60h(bool): enable (listbox and button input).
    virtual void set_enabled(std::uint32_t widget, bool enabled) = 0;
    // Widget vtable +5Ch: the widget type (3 icon/text, 6 button frame, 12h toggle; 2 slider).
    virtual std::int32_t type(std::uint32_t widget) = 0;
    // Widget vtable +4Ch(float): the alpha/colour scale.
    virtual void set_scale(std::uint32_t widget, float value) = 0;
    // Widget vtable +88h(bool, int, float): button frame highlight.
    virtual void set_frame_highlight(std::uint32_t widget, bool on, std::int32_t mode, float weight) = 0;
    // Widget vtable +84h(bool): toggle highlight.
    virtual void set_toggle_highlight(std::uint32_t widget, bool on) = 0;
};

// Focus styling shared by several listeners. `focused` is the caller's flag or
// the widget's own highlight byte +D4h. Type 3: +4Ch(0.5 [00CE3800] when
// focused, else 1.0 [00D7A24C]); type 6: +88h(focused, 0, 1.0f); type 12h:
// +84h(focused).
//   005FC650 HudPause listener: __thiscall(listener, widget, focused), RET 8.
//   005E6B80 MultiMainMenu listener +18h: the same, but returns early when the
//            widget is the scroll thumb (+C0h) and the thumb drag byte +A8h is set.
//   005EF8D0 Options listener (RET 4): byte +D4h; type 2 does nothing; no 12h case.
//   005E6E20 MultiMainMenu listener +0Ch (RET 4): first the scroller at +60h:
//            the two arrow buttons (+54h/+58h) stop auto-scroll
//            (00683300(0, 0.0f, 0)), the thumb (+C0h) ends a drag (00683320);
//            then the styling from byte +D4h.
inline constexpr float kFocusedScale = 0.5f;    // 00CE3800
inline constexpr float kUnfocusedScale = 1.0f;  // 00D7A24C
void apply_focus_style(std::uint32_t widget, bool focused, bool has_toggle_case, WidgetHost& host);
void hud_pause_on_focus_005fc650(std::uint32_t widget, bool focused, WidgetHost& host);
void multi_main_listener_current18_005e6b80(const void* listener, std::uint32_t widget, bool focused,
    WidgetHost& host);
void options_refresh_focus_005ef8d0(std::uint32_t widget, WidgetHost& host);
struct ScrollerInputHost : WidgetHost {
    virtual void set_auto_scroll_00683300(void* scroller, std::int32_t a, float speed, std::int32_t b) = 0;
    virtual void end_thumb_drag_00683320(void* scroller) = 0;
};
void multi_main_listener_current0c_005e6e20(void* listener, std::uint32_t widget, ScrollerInputHost& host);

// Show/hide fan-outs over widget pointers stored in the screen.
//   005E6B10 MultiMainMenu: +38h, +3Ch, +54h, +58h, +5Ch get +34h(shown); RET 4.
//   005BCA30 HUD panel: +48h, +F0h, +1Ch get +34h(shown); RET 4.
//   005B5BD0 HudNarrative show-default: +28h hide, +ACh hide, +24h show.
//   005B5D40 HudNarrative hide-text: +ACh hide.
//   005F9490 HudOrder hide-all: +0Ch..+34h (eleven widgets) hide.
//   0059AD60 map legend: +04h then the eight widgets +08h..+24h hide.
void multi_main_set_buttons_visible_005e6b10(const void* screen, bool shown, WidgetHost& host);
void hud_panel_set_visible3_005bca30(const void* panel, bool shown, WidgetHost& host);
void hud_narrative_show_default_005b5bd0(const void* screen, WidgetHost& host);
void hud_narrative_hide_text_005b5d40(const void* screen, WidgetHost& host);
void hud_order_hide_all_005f9490(const void* screen, WidgetHost& host);
void hud_map_legend_hide_all_0059ad60(const void* legend, WidgetHost& host);
// 0059F9B0: hides every pooled arrow entry above the in-use index (+548h) of the
// pointer array at +544h (each entry: +04h and the eight widgets +08h..+24h,
// as 0059AD60), then sets the index to -1.
void hud_map_hide_unused_arrow_widgets_0059f9b0(void* screen, WidgetHost& host);

// 005BCA80 HUD panel pending value: +170h = value, +168h = 1, +174h = extra.
void hud_panel_set_pending_value_005bca80(void* panel, std::uint32_t value, std::uint32_t extra) noexcept;
// 0059AD20: +5F0h = 1, +5F4h = pos[0], +5F8h = pos[2] (the forced cursor x/z).
void hud_map_force_cursor_pos_0059ad20(void* screen, const std::uint32_t* pos) noexcept;

// ---------------------------------------------------------------------------
// Reference-counted handles and owned objects
// ---------------------------------------------------------------------------

struct RefCountHost {
    virtual ~RefCountHost() = default;
    // InterlockedDecrement (import at 00CE2220) on object+04h.
    virtual std::int32_t interlocked_decrement(void* counter) = 0;
    // The object's vtable slot 0 called with ECX only (no flags argument).
    virtual void release_object(void* object) = 0;
    // Vtable +08h on the preview object, ECX only (005830BB).
    virtual void stop_preview(void* object) = 0;
    // Vtable +04h(1): the deleting destructor of owned GUI objects.
    virtual void delete_owned(void* object) = 0;
    // BSP_Node_UnlinkAndRelease 00B6DFA0, ECX = node.
    virtual void node_unlink_and_release(void* node) = 0;
};
// Release a counted reference held in a slot: decrement object+04h, destroy at zero.
void release_counted(void*& slot, RefCountHost& host);
// 005830A0 MainMenuScreen: stop the preview (+574h) when set, release it, then
// release +570h.
void main_menu_release_mission_preview_005830a0(void* screen, RefCountHost& host);
// 0059F780 map model ref destructor: vtable 00CEFFE8, release +04h.
void hud_map_model_ref_destruct_0059f780(void* ref, RefCountHost& host);
// 005BD690 / 005BD6C0 minimap markers: vtable 00CF1158 / 00CF1154, then the
// owned widget at +08h / +04h through +04h(1).
void hud_minimap_marker_b_destruct_005bd690(void* marker, RefCountHost& host);
void hud_minimap_marker_destruct_005bd6c0(void* marker, RefCountHost& host);
// 0059BEF0: node handle release.
void node_handle_release_0059bef0(void* handle, RefCountHost& host);

// 0059FD80 / 005A09A0: map icon array {+00h pointer to {data, size, ...}}.
// 005A09A0 first deletes every element through vtable slot 0 (flags 1); both
// then reserve(0) (0059E690) and zero the size when the array exists and is
// non-empty.
struct IconArrayHost {
    virtual ~IconArrayHost() = default;
    virtual void reserve_0059e690(void* array_slot, std::int32_t capacity) = 0;
    virtual void delete_element(void* element) = 0;  // (*vtable[0])(1)
};
void hud_map_icon_array_clear_0059fd80(void* array_slot, IconArrayHost& host);
void hud_map_release_icon_array_005a09a0(void* array_slot, IconArrayHost& host);

// 005F97D0: session message scalar deleting destructor, __thiscall(msg, flags),
// RET 4: reset the vtable to the root (00CE4974) and _free when flags bit 0.
void* session_message188_scalar_delete_005f97d0(void* message, std::uint8_t flags,
    void (*free_block)(void*));

// ---------------------------------------------------------------------------
// Screen registration, construction and destruction
// ---------------------------------------------------------------------------

struct ScreenLifetimeHost {
    virtual ~ScreenLifetimeHost() = default;
    // BSP_FrontEndScreen_Register 004F71D0 / _Construct 004F7180 / _Destruct, ECX = screen.
    virtual void frontend_register_004f71d0(void* screen) = 0;
    virtual void frontend_construct_004f7180(void* screen) = 0;
    virtual void frontend_destruct(void* screen) = 0;
    // BSP_GuiManager_LoadPage 00AA5840(name, 1, 0) on the 004C12B0 manager.
    virtual std::uint32_t load_page_00aa5840(std::string_view name) = 0;
    // BSP_GuiManager_RemoveAndDestroyPage on the 004C12B0 manager.
    virtual void remove_and_destroy_page(std::uint32_t page) = 0;
    // The screen's own vtable +14h (layout binding).
    virtual void screen_bind_layout(void* screen) = 0;
    // BSP_GuiScroller_Construct 00683610, ECX = scroller.
    virtual void scroller_construct_00683610(void* scroller) = 0;
    // BSP_GuiWidget_GetResolvedPosition 00AA6750, ECX = widget.
    virtual std::array<float, 3> widget_resolved_position_00aa6750(std::uint32_t widget) = 0;
    // 005EA030 MultiMainMenu LayoutAchievementLocks, ECX = screen.
    virtual void multi_main_layout_achievement_locks_005ea030(void* screen) = 0;
    virtual void free_block(void* block) = 0;
};

// The page loads: FrontEndScreen_Register, a pooled native string with the page
// name (released after the load), LoadPage(name, 1, 0) into the page slot.
//   005EE010 HudObjectives "GUI_objectives" -> +10h; then +64h = 0, +60h = 0, +0Ch = 0.
//   005F9FF0 HudOrder "GUI_order" -> +08h; then vtable +14h; +88h = 0, +8Ch = 0,
//            +5Dh = +5Eh = +5Ch = 0, +81h = +82h = 0, +84h = 128h, +83h = 1,
//            +60h = +64h = +70h = 0.
//   00604890 HudPauseCheat "GUI_pause_cheat" -> +08h.
inline constexpr std::string_view kHudObjectivesPage = "GUI_objectives";
inline constexpr std::string_view kHudOrderPage = "GUI_order";
inline constexpr std::string_view kHudPauseCheatPage = "GUI_pause_cheat";
void hud_objectives_register_005ee010(void* screen, ScreenLifetimeHost& host);
void hud_order_register_005f9ff0(void* screen, ScreenLifetimeHost& host);
void hud_pause_cheat_register_00604890(void* screen, ScreenLifetimeHost& host);
// 005EA720 MultiMainMenu: Register, vtable +14h, +F0h = +E0h = +E4h = 0,
// +ECh = the y of widget +DCh's resolved position, +F4h = -1, then
// LayoutAchievementLocks (tail JMP).
void multi_main_register_005ea720(void* screen, ScreenLifetimeHost& host);
// 005EA7C0 MultiModeSelector: Register, vtable +14h, +F4h = -1, +10h = 0.
void multi_mode_selector_register_005ea7c0(void* screen, ScreenLifetimeHost& host);

// Constructors. FrontEndScreen_Construct, then the vtables (values are the
// image addresses) and fields:
//   005E6D90 MultiMainMenu: +00h 00CF2708, +08h 00CF26F4, +0Ch 00CF26D4, scroller at +6Ch.
//   005EA8C0 MultiModeSelector: 00CF2B44 / 00CF2B30 / 00CF2B10, scroller at +6Ch.
//   005EE440 HudObjectives: 00CF2D20 / 00CF2D0C; vectors +44h..+4Ch and +54h..+5Ch zeroed.
//   005F9440 HudOrder: 00CF3B7C.      00604780 HudPauseCheat: 00CF41A0.
// The base-class vtables written in between (00CEB0FC, 00CEB110) are overwritten
// before return and are not modelled.
void* multi_main_construct_005e6d90(void* screen, ScreenLifetimeHost& host);
void* multi_mode_selector_construct_005ea8c0(void* screen, ScreenLifetimeHost& host);
void* hud_objectives_construct_005ee440(void* screen, ScreenLifetimeHost& host);
void* hud_order_construct_005f9440(void* screen, ScreenLifetimeHost& host);
void* hud_pause_cheat_construct_00604780(void* screen, ScreenLifetimeHost& host);

// Destructors: restore the screen's vtables, destroy the loaded pages, free owned
// vectors, put the base vtable 00CEB0FC back at +08h (where the screen has one)
// and run FrontEndScreen_Destruct.
//   005E70B0 MultiMainMenu pages +18h, +1Ch.   005EABC0 MultiModeSelector +14h, +18h, +1Ch.
//   005EDF30 HudObjectives page +10h, then vectors +54h and +44h.
//   005F9D70 HudOrder page +08h.              00604810 HudPauseCheat page +08h.
void multi_main_destruct_005e70b0(void* screen, ScreenLifetimeHost& host);
void multi_mode_selector_destruct_005eabc0(void* screen, ScreenLifetimeHost& host);
void hud_objectives_destruct_005edf30(void* screen, ScreenLifetimeHost& host);
void hud_order_destruct_005f9d70(void* screen, ScreenLifetimeHost& host);
void hud_pause_cheat_destruct_00604810(void* screen, ScreenLifetimeHost& host);

// ---------------------------------------------------------------------------
// Main menu callbacks (main-menu screen = *(00E198AC) + 58h)
// ---------------------------------------------------------------------------

struct MainMenuCallbackHost {
    virtual ~MainMenuCallbackHost() = default;
    // [*(00E198AC) + 58h]; null when the manager or the screen is absent
    // (only 00588460 tests for that; the others dereference unconditionally).
    virtual void* main_menu_screen() = 0;
    virtual void advance_background_movie_005845d0(void* screen) = 0;
    virtual void finish_async_text_request_00590a00(void* screen, std::int32_t arg) = 0;
    virtual void play_test_speech_00585c50(void* screen, const void* file) = 0;
    // 004FC150 singleton, 00BE80B0 GetOrCreate, 00BE7130 RemoveFile(file).
    virtual void file_store_remove_file(const void* file) = 0;
    virtual void build_mission_detail_page_0058c010(void* screen) = 0;
    // [00E188A8 + 5D4h], the game state.
    virtual std::int32_t game_state() = 0;
    virtual void clear_objective_rows_00583e50(void* screen) = 0;
    virtual void build_mission_map_icons_0058d9e0(void* screen, bool arg) = 0;
};
// 005861A0 movie callback: AdvanceBackgroundMovie on the main-menu screen.
void main_menu_on_movie_advance_005861a0(MainMenuCallbackHost& host);
// 00590E40, 00590E50 (byte-identical): FinishAsyncTextRequest(1) on the screen.
void main_menu_on_async_text_request_done_00590e40(MainMenuCallbackHost& host);
// 00588460 file-store completion, __stdcall(file, unused), RET 8: when the menu
// screen exists, 00585C50(file) on it (one argument is pushed here), then
// remove the file from the file store.
void main_menu_remove_file_store_file_00588460(const void* file, MainMenuCallbackHost& host);
// 0058F540 prompt callback, ECX = result: non-zero clears screen+5Ch and
// rebuilds the mission-detail page.
void main_menu_on_mission_detail_prompt_result_0058f540(std::int32_t result, MainMenuCallbackHost& host);
// 0058F560 MainMenuScreen exit: hide +2F0h; unless the game state is 10 or the
// page global 00E08874 is already 1, 2 or 3, set it to 1; clear the objective
// rows and rebuild the mission map icons with 1.
void main_menu_screen_exit_0058f560(void* screen, std::int32_t& page_00e08874, WidgetHost& widgets,
    MainMenuCallbackHost& host);
// 00580780 (unreferenced): the same body as the new-profile cancel 005807D0
// (src/frontend_menus.cpp, new_profile_cancel_005807d0); ECX is the screen here
// (text widget +30h, editor +0Ch, list box +1B8h).
void main_menu_cancel_new_profile_unreferenced_00580780(NewProfileTextHost& host);

// 00583DE0: clear the new-profile text: resize the string at +24h to 0 and copy
// the empty literal 00CE3A0C over it when it has a buffer; list box +1B8h
// vtable +60h(0) and +77h = 1; editor (+0Ch) 00A966E0(1, 00425E50(list box));
// then the editor's vtable +04h.
struct TextEditorHost {
    virtual ~TextEditorHost() = default;
    virtual void resize_native_string(void* header, std::uint32_t length, bool preserve) = 0;
    virtual void list_box_set_enabled_60(std::uint32_t list_box, bool enabled) = 0;
    virtual std::uint32_t list_box_value_00425e50(std::uint32_t list_box) = 0;
    virtual void editor_set_active_00a966e0(void* editor, bool active, std::uint32_t value) = 0;
    virtual void editor_virtual_04(void* editor) = 0;
};
void main_menu_clear_text_editor_00583de0(void* screen, TextEditorHost& host);

// 005808B0 SlideWidget, __thiscall(screen, move, active), RET 8. When `move`:
// position the list box (+1B8h) at (+558h + 0.0, +55Ch + 0.0, +560h + d), d = 0
// when `active` else 1000 (the integer picked with NEG/SBB/AND, then FILD), through
// BSP_GuiWidget_SetResolvedPosition 00AA8240. Then 00AA6BC0(listener, 0) on the list
// box with listener = screen+40h when `active`, else null.
struct SlideHost {
    virtual ~SlideHost() = default;
    virtual void set_resolved_position_00aa8240(std::uint32_t widget, const float* xyz) = 0;
    virtual void set_listener_fields_00aa6bc0(std::uint32_t widget, void* listener, std::uint32_t arg) = 0;
};
void main_menu_slide_widget_005808b0(void* screen, bool move, bool active, SlideHost& host);

// 005C3850 MissionTreeScreen MissionIndexOf, __thiscall(screen, name), RET 4:
// 005C3470(&out, name) on the same ECX; returns out.first.
struct MissionTreeLookupHost {
    virtual ~MissionTreeLookupHost() = default;
    virtual std::int32_t find_mission_by_name_005c3470(void* screen, const void* name) = 0;
};
std::int32_t mission_tree_mission_index_of_005c3850(void* screen, const void* name, MissionTreeLookupHost& host);

// ---------------------------------------------------------------------------
// Options screen (options screen = *(00E198B8) + 40h; settings at 00F88980)
// ---------------------------------------------------------------------------

struct OptionsCallbackHost {
    virtual ~OptionsCallbackHost() = default;
    virtual void* options_manager() = 0;  // 00E198B8, may be null
    virtual void* options_screen() = 0;   // [00E198B8 + 40h]
    virtual void refresh_help_line_005f0cd0(void* screen) = 0;
    virtual void settings_apply_all_008d5b50() = 0;
    virtual void build_main_list_005f54f0(void* screen) = 0;
    virtual void commit_changes_005f65c0(void* screen) = 0;
    virtual void revert_video_settings_005efcf0(void* screen) = 0;
    virtual void reset_to_defaults_005efab0(void* screen) = 0;
    // [00E188A8 + 61Fh] / 00549620(0) on [00E1930C] / 005F6EF0 on the screen.
    virtual bool game_in_mission_61f() = 0;
    virtual void command_owner_set_help_variant_00549620(std::int32_t variant) = 0;
    virtual void sync_profile_values_005f6ef0(void* screen) = 0;
    // 005F01C0: 004C1710(1) -> 00A4C2D0, then save with the empty continuation 00BD53C0.
    virtual void media_manager_refresh_00a4c2d0() = 0;
    virtual void settings_save_to_profile_008d6c00(std::uint32_t continuation) = 0;
    // 005EFCF0: video globals and the XLive device reset.
    virtual bool ingame_interface_present() = 0;  // 00E198C4 != 0
    virtual void xlive_on_reset_device(std::uint32_t device_params) = 0;
};
// Prompt callbacks, ECX = result.
//   005EFCB0, 005EFCD0 (identical): manager present and result 2 -> ResetToDefaults.
//   005F5B90: manager present -> RefreshHelpLine; result 1 -> ApplyAll, BuildMainList.
//   005F6FB0: manager present -> RefreshHelpLine; result 1 -> commit 005F65C0.
//   005F6FE0: as 005F6FB0, and result 0 -> RevertVideoSettings.
//   005F01C0: result 1 -> media refresh, ApplyAll, SaveToProfile(00BD53C0); any
//             other result copies the four bytes 00F889C5..C8 back to 00F889C1..C4.
void options_prompt_reset_defaults_005efcb0(std::int32_t result, OptionsCallbackHost& host);
void options_prompt_apply_005f5b90(std::int32_t result, OptionsCallbackHost& host);
void options_prompt_commit_005f6fb0(std::int32_t result, OptionsCallbackHost& host);
void options_prompt_video_confirm_005f6fe0(std::int32_t result, OptionsCallbackHost& host);
void options_prompt_apply_and_save_005f01c0(std::int32_t result, std::uint8_t* settings_f889c1,
    OptionsCallbackHost& host);
// 005F5AC0: ApplyAll then BuildMainList on ECX.
void options_apply_and_rebuild_main_list_005f5ac0(void* screen, OptionsCallbackHost& host);
// 005F7020 OptionsScreen exit: outside a mission reset the command bar's help
// variant to 0; then SyncProfileValues on the screen.
void options_screen_exit_005f7020(void* screen, OptionsCallbackHost& host);
// 005EFCF0: copy the live video values back into the screen's edit copy
// (+ECh = [00F88994], +F0h = [00F88998], +150h = [00F889F8], byte +F6h =
// [00F8899E]), ApplyAll, and without an in-game interface call
// XLiveOnResetDevice([00F8D394] + 1A28h).
struct VideoSettingsSnapshot {
    std::uint32_t f88994, f88998, f889f8;
    std::uint8_t f8899e;
    std::uint32_t device_00f8d394;
};
void options_revert_video_settings_005efcf0(void* screen, const VideoSettingsSnapshot& live,
    OptionsCallbackHost& host);
// 005EF500: name-entry filter, __cdecl(char16_t*): anything other than a-z,
// A-Z, 0-9, space or NUL becomes a space; returns true when it replaced.
bool utf16_char_sanitize_for_name_005ef500(char16_t& c) noexcept;

// The four page builders (kOptionsPages in options_menu_screens.hpp gives the
// title, label table, counts and help table). Each stores the page number in
// 00E19650, builds a pooled title string and calls 005F24D0(title, labels,
// label_count, value_rows, help_table, 0) on the screen, then:
//   Game 005F58F0: before the title, 008D45C0 on screen+D8h; when it returned
//        1 or less, row widget 00A9BE00(0) of the list box +88h gets +77h = 1;
//        then RefreshGameRows 005F4C90.
//   Audio 005F59F0: 005F4FA0.   Video 005F41E0: RefreshVideoRows 005F2F20.
//   Control 005F40F0: 00527CB0(+119h, +11Ah, +11Bh, +118h) on [00E198B8 + 44h],
//        then RefreshControlRows 005F34D0.
struct OptionsPageHost {
    virtual ~OptionsPageHost() = default;
    virtual void build_settings_page_005f24d0(void* screen, std::string_view title, std::uint32_t labels,
        std::uint16_t label_count, std::uint16_t value_rows, std::uint32_t help_table) = 0;
    virtual std::int32_t edit_settings_query_008d45c0(void* settings_copy) = 0;
    virtual void* list_box_row_00a9be00(std::uint32_t list_box, std::int32_t row) = 0;
    virtual void refresh_page_rows(std::uint32_t routine, void* screen) = 0;
    virtual void control_layout_button_flags_00527cb0(std::uint8_t* b119, std::uint8_t* b11a,
        std::uint8_t* b11b, std::uint8_t* b118) = 0;
};
void options_build_game_page_005f58f0(void* screen, std::int32_t& page_00e19650, OptionsPageHost& host);
void options_build_audio_page_005f59f0(void* screen, std::int32_t& page_00e19650, OptionsPageHost& host);
void options_build_video_page_005f41e0(void* screen, std::int32_t& page_00e19650, OptionsPageHost& host);
void options_build_control_page_005f40f0(void* screen, std::int32_t& page_00e19650, OptionsPageHost& host);

// ---------------------------------------------------------------------------
// HUD pause, order and objectives
// ---------------------------------------------------------------------------

struct HudPauseHost {
    virtual ~HudPauseHost() = default;
    // [00E1930C + 4] (command bar) byte.
    virtual void set_command_bar_byte4(std::uint8_t value) = 0;
    // 00A933F0(index, level) on the 004BEC00 input manager.
    virtual void input_set_context_level(std::int32_t index, std::int32_t level) = 0;
    virtual void game_set_cinematic_mode_004cd0f0(std::int32_t a, std::int32_t b, std::int32_t c) = 0;
    virtual void game_set_system_pause_004d95f0(bool paused) = 0;
    // [004C1B90() + 1Ah] = 1 (sound request queue flush flag).
    virtual void sound_queue_set_flag_1a() = 0;
    virtual void frontend_restore_level_set_004f8390(std::int32_t level, void* set_00e19664) = 0;
    virtual void input_contexts_set_level_set_004d6410(std::int32_t level, void* set_00e19674) = 0;
    virtual void frontend_commit_visibility_004f83b0(void* screen) = 0;
    virtual void screen_virtual_18(void* screen) = 0;
    // 004F81D0 (ECX = 00E19664, source 00E18D18) and 004D3740 (ECX = 00E19674,
    // source game+590h): the saved level sets.
    virtual void save_level_sets() = 0;
    // [[00E198C4]+A8h]: when its byte +4 is set, its vtable +1Ch.
    virtual void stop_active_movie_a8() = 0;
    virtual void game_end_scene_004d7970(bool restart) = 0;
    virtual void game_request_state_004d7920(std::int32_t state) = 0;
    virtual void profile_clear_checkpoint_and_rewrite_00437c70() = 0;
    virtual void* hud_pause_screen() = 0;  // [00E198C4 + D8h]
    virtual void frontend_frame_select_default_004c9680() = 0;
};
// 005FD440 Resume, ECX = pause screen: command-bar byte 0; input context 1Bh
// level 0; SetCinematicMode(0, 0, 1); SetSystemPause(0); sound flag; screen
// +04h = 0; restore level set 3 from 00E19664; input level set 3 from 00E19674.
void hud_pause_resume_005fd440(void* screen, HudPauseHost& host);
// 005FF0D0 Open: input context 1Bh level 5; sound flag; +04h = +05h = 1;
// commit visibility; vtable +18h; save the level sets.
void hud_pause_open_005ff0d0(void* screen, HudPauseHost& host);
// 005FD610 quit-mission prompt (ECX = result): 1 -> stop movie, EndScene(1), Resume.
// 005FD5B0 restart prompt: 1 -> clear checkpoint, stop movie, EndScene(0),
// RequestState(0Bh), Resume.
void hud_pause_prompt_quit_mission_005fd610(std::int32_t result, HudPauseHost& host);
void hud_pause_prompt_restart_005fd5b0(std::int32_t result, HudPauseHost& host);
// 005FD880 HudPause exit: tail JMP 004C9680.
void hud_pause_exit_005fd880(HudPauseHost& host);

struct HudOrderCloseHost {
    virtual ~HudOrderCloseHost() = default;
    virtual void weapon_group_suspend_00545360() = 0;  // ECX = [00E198C4 + 50h]
    virtual void set_ingame_68_byte190() = 0;          // [[00E198C4] + 68h] + 190h = 1
    virtual void collapse_overlays_0068ab80(bool a, bool b) = 0;
    virtual void leave_unit_request_level_0068ab30() = 0;
};
// 005F9520 HudOrder close: +82h = +81h = 0, hide +08h, suspend the weapon-group
// screen, flag it, collapse overlays (0, 0), leave the unit request level.
void hud_order_close_005f9520(void* screen, WidgetHost& widgets, HudOrderCloseHost& host);

struct HudObjectivesExpandHost {
    virtual ~HudObjectivesExpandHost() = default;
    virtual void set_level3_set_004f8670(std::int32_t screen_id, std::int32_t arg) = 0;
    virtual void input_push_level_004d8b70(std::int32_t a, std::int32_t b, std::int32_t c) = 0;
    virtual void counters_hide_all_005290d0() = 0;     // ECX = [00E198C4 + C8h]
    virtual void counters_restore_hidden_00529350() = 0;
    virtual bool overlay_busy_006529e0() = 0;           // ECX = [00E198C4 + BCh]
    virtual void collapse_overlays_0068ab80(bool a, bool b) = 0;
};
// 005ED5D0 SetExpanded, __thiscall(screen, expanded), RET 4. Ignored while the
// byte +0Ch is set. Stores +70h, enables +34h. Expanded: raise level-3 set 39h,
// push input level (14h, 0Eh, 0), hide the counters. Collapsed: unless
// 006529E0 is busy, collapse overlays (0, 1). Then, when collapsed, restore the
// counters.
void hud_objectives_set_expanded_005ed5d0(void* screen, bool expanded, WidgetHost& widgets,
    HudObjectivesExpandHost& host);

// ---------------------------------------------------------------------------
// Multiplayer menus
// ---------------------------------------------------------------------------

struct MultiMenuHost {
    virtual ~MultiMenuHost() = default;
    virtual std::int32_t session_state_1fe4() = 0;  // [00E188A8 + 1FE4h]
    virtual void lobby_step_setting_005e1cf0(void* lobby_screen, std::int32_t delta) = 0;
    virtual void list_box_clear_rows_00a9bec0(std::uint32_t list_box) = 0;
    virtual void session_set_mode_0076fad0(std::int32_t mode) = 0;  // ECX = game+1EF0h
    virtual void push_request_interface_00688ad0(std::int32_t interface_id) = 0;  // ECX = [00E198B4]
    virtual std::uint8_t online_byte_4d() = 0;  // [[00F8A2FC] + 4Dh]
    virtual std::int32_t session_slot_18ec() = 0;  // [00E188A8 + 18ECh]
    virtual std::uint32_t game_scene_index_60c() = 0;  // [00E188A8 + 60Ch]
    virtual void lobby_refresh(std::uint32_t routine, void* screen) = 0;
};
// 005E4F90 lobby listener value-changed, __thiscall(listener, widget, value),
// RET 8: when value differs from +8Ch and the session state is not 2, step the
// setting by -1 on the screen (listener - 8).
void lobby_on_value_changed_005e4f90(void* listener, std::int32_t value, MultiMenuHost& host);
// 005E6B60 MultiMainMenu exit: clear the rows of the list boxes +20h and +24h.
void multi_main_exit_005e6b60(const void* screen, MultiMenuHost& host);
// 005EA7F0: 00E19610 = 00E19614 = 0 and +F4h = 0.
void multi_main_reset_flags_005ea7f0(void* screen, std::int32_t& mode_00e19610, std::int32_t& index_00e19614) noexcept;
// 005EA960 MultiModeSelector exit: [00E19698 + 4] = 0.
void multi_mode_selector_exit_005ea960(std::uint8_t* object_00e19698) noexcept;
// 005E6F00 start hosting: bytes 00E08700 = 1, 00E08701 = 00E08718 = 00E08720 = 0;
// session mode 2; request interface 1Ah; then game+2019h = 0, +2059h = the
// online byte, +2017h = 8, +201Dh = 1, +201Ch = 14h.
struct HostingFlags {
    std::uint8_t* e08700;
    std::uint8_t* e08701;
    std::uint8_t* e08718;
    std::uint8_t* e08720;
};
void multi_main_start_hosting_005e6f00(const HostingFlags& flags, void* game_00e188a8, MultiMenuHost& host);
// 005E5AE0 lobby scene index received: when 0 <= slot <= 7, 00E19594 = the
// scene index, +10h = 1, then the four refreshes 005E3580, 005D6C10, 005D98E0,
// 005D5340 on the screen.
void lobby_on_scene_index_received_005e5ae0(void* screen, std::uint32_t& scene_00e19594, MultiMenuHost& host);
// 005E1990: the lobby option table: with a main-menu mission tree
// ([00E198AC + 5Ch]) whose mission (005D7070, 004B4E60 on tree+24h) has the byte
// +424h set, that record + 428h; otherwise the option registry singleton + 28h.
struct LobbyOptionHost {
    virtual ~LobbyOptionHost() = default;
    virtual void* mission_tree() = 0;  // [00E198AC + 5Ch], null-safe
    virtual void* current_mission_record(void* tree) = 0;
    virtual void* option_registry_00576b10() = 0;
};
void* lobby_settings_option_table_005e1990(LobbyOptionHost& host);

// ---------------------------------------------------------------------------
// HUD narrative and map helpers
// ---------------------------------------------------------------------------

struct VoiceSlotHost {
    virtual ~VoiceSlotHost() = default;
    virtual bool poll_007027b0(void* slot) = 0;
    virtual void stop_007026f0(void* slot) = 0;
};
// 005B68D0: +88h = +8Ch = 0.0; when the voice index +70h is non-negative and
// the slot (this + 8 + index*18h) is still active, stop it.
void panel_owner_stop_voice_005b68d0(void* owner, VoiceSlotHost& host);

struct MapIconHost {
    virtual ~MapIconHost() = default;
    virtual void material_set_texture_slot_00b189f0(void* material, std::int32_t slot, void* texture) = 0;
    virtual float* material_diffuse_color_00b179f0(void* material, std::int32_t index) = 0;
    // 00803CE0 with ECX = [[game + 18CCh + 4*[game+18ECh]] + 28h] and no stack
    // argument (0059D2C5).
    virtual std::int32_t recon_relation_if_identified_00803ce0() = 0;
    virtual void* find_unit_icon_005a22d0(void* screen, std::uint32_t unit) = 0;
    virtual void icon_start_blink_005a21e0(void* icon) = 0;
    virtual void delete_owned(void* object) = 0;  // vtable +04h(1)
    virtual std::uint8_t* map_subscript_005a7af0(void* map, std::uint32_t key) = 0;
};
// 0059AA10 icon texture, __thiscall(icon, texture, value), RET 8: with a material
// at +13Ch, set slot 0 and store value at +140h.
void hud_map_set_icon_texture_0059aa10(void* icon, void* texture, float value, MapIconHost& host);
// 0059A9B0 icon colour, __thiscall(icon, const float rgba[4]), RET 4: the diffuse
// colour of material +134h, and of +138h when present.
void hud_map_icon_set_color_0059a9b0(void* icon, const float* rgba, MapIconHost& host);
// 0059D2A0 side texture: when +0Ch is set, index = the unit's side (+54h of +04h),
// 2 when the relation is 3 (identified enemy); slot 0 of materials +134h (and
// +138h) gets texture table (+14h)[index].
void hud_map_icon_refresh_side_texture_0059d2a0(void* icon, MapIconHost& host);
// 005A4860 clear speaking icon: when +52Ch names a unit whose icon has a blink
// state (+118h), delete its +20h widget and restart the blink; +530h = +52Ch,
// +52Ch = 0.
void hud_map_clear_speaking_icon_005a4860(void* screen, MapIconHost& host);
// 005AAAD0 SetUnitHighlight, __thiscall(screen, unit, value), RET 8: map at +600h.
void hud_map_set_unit_highlight_005aaad0(void* screen, std::uint32_t unit, std::uint8_t value, MapIconHost& host);

// ---------------------------------------------------------------------------
// Native string records (batch 2)
// ---------------------------------------------------------------------------

// Records made of consecutive 8-byte native strings, optionally followed by a
// sub-object the routine hands to another constructor.
//   005EFE80 OptionRow assign (two strings, no header reset; RET 4).
//   005F19E0 OptionValue copy (two strings).
//   005FD6F0 HintDefinition assign (three strings, no header reset).
//   005FD790 HintDefinition copy (three strings).
// Assign forms run the 00BE0A30 fragment on each string; copy forms zero each
// header first.
void* string_record_assign(void* dst, const void* src, int strings, NativeStringStorage& storage);
void* string_record_copy(void* dst, const void* src, int strings, NativeStringStorage& storage);
inline void* option_row_assign_005efe80(void* d, const void* s, NativeStringStorage& st) { return string_record_assign(d, s, 2, st); }
inline void* option_value_copy_005f19e0(void* d, const void* s, NativeStringStorage& st) { return string_record_copy(d, s, 2, st); }
inline void* hint_definition_assign_005fd6f0(void* d, const void* s, NativeStringStorage& st) { return string_record_assign(d, s, 3, st); }
inline void* hint_definition_copy_005fd790(void* d, const void* s, NativeStringStorage& st) { return string_record_copy(d, s, 3, st); }

// 005EFDC0 OptionRow destruct: destroy the string at +08h, then +00h.
void option_row_destruct_005efdc0(void* row, NativeStringStorage& storage) noexcept;

// 005EFFA0 OptionRow construct, __thiscall(row, NativeString a, NativeString b),
// RET 10h: both strings arrive by value (two dwords each); they are copied into
// +00h and +08h and the callee then destroys its by-value copies (data a, then b).
void* option_row_construct_005effa0(void* row, std::uint32_t a_length, const char* a_data,
    std::uint32_t b_length, const char* b_data, NativeStringStorage& storage);

// 005FDCA0, __thiscall(pair, const uint32* key, const NativeString* name), RET 8:
// +00h = *key, then the name copied into +04h.
void* id_name_pair_construct_005fdca0(void* pair, const std::uint32_t* key, const void* name,
    NativeStringStorage& storage);

// String followed by a sub-object at +08h, __thiscall(dst, src_string, arg), RET 8
// (005FE370, 005FF330) or __thiscall(dst, src), RET 4 with arg = src+08h
// (005FE430, 005FF4D0). The sub-object constructor is 005FE010 for the first pair
// and BSP_PooledStringList_CopyConstruct 004D48A0 for the second (ECX = dst+08h).
struct SubObjectHost {
    virtual ~SubObjectHost() = default;
    virtual void construct(std::uint32_t routine, void* dst, const void* arg) = 0;
};
void* string_then_subobject(std::uint32_t routine, void* dst, const void* src_string, const void* arg,
    NativeStringStorage& storage, SubObjectHost& host);
inline void* named_value_construct_005fe370(void* d, const void* s, const void* a, NativeStringStorage& st, SubObjectHost& h) { return string_then_subobject(0x005FE010u, d, s, a, st, h); }
inline void* string_record_copy_005fe430(void* d, const void* s, NativeStringStorage& st, SubObjectHost& h) { return string_then_subobject(0x005FE010u, d, s, static_cast<const std::byte*>(s) + 8, st, h); }
inline void* string_map_ci_node_value_construct_005ff330(void* d, const void* s, const void* a, NativeStringStorage& st, SubObjectHost& h) { return string_then_subobject(0x004D48A0u, d, s, a, st, h); }
inline void* string_record_copy_005ff4d0(void* d, const void* s, NativeStringStorage& st, SubObjectHost& h) { return string_then_subobject(0x004D48A0u, d, s, static_cast<const std::byte*>(s) + 8, st, h); }

// Mission score pairs {+00h name, +08h MissionScoreRecord (284h bytes), +28Ch dword}.
//   005943D0 construct(dst, name, record), RET 8: name copied, 00593CA0 assign
//            (ECX = dst+08h) from the record, +28Ch = record+284h.
//   00594340 copy(dst, src), RET 4: the same with src+08h and src+28Ch.
//   005942A0 destroy: 00593570 on +08h, then the name.
struct MissionScoreHost {
    virtual ~MissionScoreHost() = default;
    virtual void record_assign_00593ca0(void* dst_record, const void* src_record) = 0;
    virtual void record_destruct_00593570(void* record) = 0;
};
void* mission_score_pair_construct_005943d0(void* dst, const void* name, const void* record,
    NativeStringStorage& storage, MissionScoreHost& host);
void* mission_score_pair_copy_00594340(void* dst, const void* src, NativeStringStorage& storage,
    MissionScoreHost& host);
void mission_score_pair_destroy_005942a0(void* pair, NativeStringStorage& storage, MissionScoreHost& host);

// 005F1930: array {+00h data, +04h count} of 8-byte native strings,
// __thiscall(array, index), RET 4. Index -1 does nothing. Unless the index is the
// last element, assign the last string over it (resize + copy); then destroy the
// last string and decrement the count.
void native_string_array_remove_swap_last_005f1930(void* array, std::int32_t index, NativeStringStorage& storage);

// 005F1750: out = decimal text of an unsigned value, __thiscall(out, value), RET 4:
// sprintf("%u") into a 64-byte buffer, a temporary native string, then copied
// into the zeroed out header and the temporary destroyed.
void* native_string_from_unsigned_005f1750(void* out, std::uint32_t value, NativeStringStorage& storage);

// 00589690 vector<NativeString>::erase(where), __thiscall(vector, out_iter,
// iter_container, iter_ptr), RET 0Ch: copy [where+1, last) down (0048DA50),
// destroy the last element (00432050), last -= 8, out = {container, where}.
struct NativeStringRangeHost {
    virtual ~NativeStringRangeHost() = default;
    virtual void assign_range_0048da50(void* first, void* last, void* dest) = 0;
    virtual void destroy_range_00432050(void* first, void* last) = 0;
};
void native_string_vector_erase_00589690(void* vector, std::uint32_t* out_iter, std::uint32_t iter_container,
    void* where, NativeStringRangeHost& host);

// 0058BD40 tree _Copy(source_node, parent), __thiscall(tree, node, parent), RET 8,
// recursive. A nil source (byte +19h) returns the head. Otherwise a node from
// 00504BD0(head, parent, head, &value(+0Ch), colour(+18h)) whose left and right
// are the copies of the source's children; returns the new node.
struct TreeCopyHost {
    virtual ~TreeCopyHost() = default;
    virtual void* allocate_node_00504bd0(void* tree, void* left, void* parent, void* right, const void* value,
        std::uint8_t colour) = 0;
};
void* string_int_tree_copy_subtree_0058bd40(void* tree, const void* node, void* parent, TreeCopyHost& host);

// Element-destroying vector tidy (005E0AC0 jumps here at 005E0A40 for 24h-byte
// lobby map entries; 005F6210 at 005F5F80 for 10h-byte option rows): destroy each
// element of [+04h, +08h), free +04h and zero +04h..+0Ch, only when +04h is set.
void vector_tidy_destroy(void* vector, std::size_t element_size, void (*destroy)(void*, void*), void* context,
    ContainerHost& host);

// 00590250 DebugTextList destruct: vtable 00CEFC00, 00544A70 on the list, then
// the byte-SBO pair vector at +10h: when +14h is set, 00543C20 over [+14h, +18h),
// _free(+14h); zero +14h..+1Ch.
struct DebugTextListHost {
    virtual ~DebugTextListHost() = default;
    virtual void clear_00544a70(void* list) = 0;
    virtual void destroy_range_00543c20(void* first, void* last, void* allocator) = 0;
    virtual void free_block(void* block) = 0;
};
void debug_text_list_destruct_00590250(void* list, DebugTextListHost& host);

// 005B8070 voice observer destruct: 004B7EF0 on +18h, then 005B7FC0 on the object.
struct VoiceObserverHost {
    virtual ~VoiceObserverHost() = default;
    virtual void observer_link_destroy_base_004b7ef0(void* link) = 0;
    virtual void voice_slot_destroy_005b7fc0(void* slot) = 0;
};
void voice_observer_destruct_005b8070(void* observer, VoiceObserverHost& host);

// ---------------------------------------------------------------------------
// HUD map arithmetic and lookups (batch 2)
// ---------------------------------------------------------------------------

// 0059AB10 AnimateZoom, __thiscall(screen, float dt), RET 4. The direction byte
// +88h is read once (AL) for the update and the clamps:
//   zoom(+84h) = closing ? zoom + dt*1.5 : zoom - dt*1.5   (1.5 = double 00CE3D78)
//   opening and zoom < 0 -> 0;  closing and zoom > 1.0 -> 1.0
//   opening and zoom == 0 -> +88h = 1;  then (re-read) +88h set and zoom == 1.0 -> +88h = 0
// x87: dt*1.5 and the add run in 80-bit and round once to float; the C++ forms
// them in double (dt*1.5 is exact there), so only the add can differ, by double
// rounding, in the last float bit.
void hud_map_animate_zoom_0059ab10(void* screen, float dt) noexcept;
// 0059ABC0: +5BCh times the per-mode factor of +2Ch (0: 0.8, 1: 0.7, 2: 0.6,
// other: 0.5; doubles 00CE3D40, 00CEFFA0, 00CEFF98, 00D7A280, each the double of
// a float), rounded to float (FSTP/FLD). The product is exact in double.
float hud_map_zoom_scale_for_mode_0059abc0(const void* screen) noexcept;
// 0059C920 (unreferenced): out = (min + max) * 0.5 of the game's world box
// (+711Ch..+7124h, +7128h..+7130h); each sum is rounded to float before the
// multiply, as the native stores it.
void game_world_bounds_center_0059c920(const void* game, float* out) noexcept;
// 0059C9B0, __thiscall(game, const float* p, float margin), RET 8: true when p
// is beyond the world box by more than margin on x or z.
bool world_point_beyond_map_margin_0059c9b0(const void* game, const float* p, float margin) noexcept;
// 0059D410, __stdcall(const float* p), RET 4: inside the map area. Wide layout
// ([0109CF04] + 0Dh set): -0.1 < x < 1.1; otherwise 0.0604166674 < x <
// 0.939583321; and in both 0.05 < y < 0.95 (all comparisons strict; NaN outside).
bool hud_map_is_point_inside_map_area_0059d410(const float* p, bool wide_layout) noexcept;
// 0059A910, __thiscall(classifier, const float* p), RET 4: the off-screen edge
// code from the reference point (vtable +C4h of +04h, re-fetched for each test):
// x not above ref.x -> (ref.z > p.y ? 1 : 3); otherwise (ref.z > p.y ? 0 : 2).
// Comparisons are in double; NaN takes the "not above" branches.
struct EdgeReferenceHost {
    virtual ~EdgeReferenceHost() = default;
    virtual const float* reference_point_c4(void* object) = 0;
};
std::int32_t hud_map_classify_offscreen_edge_0059a910(const void* classifier, const float* p, EdgeReferenceHost& host);
// 005A21E0 StartBlink: +118h = 5, +110h = now + 0.0 (double 00D7A258), +114h =
// now ([00F876A4]), +108h/+10Ch = +100h/+104h, +F8h = +FCh = 0, then
// 005A1C60(0) (ReleaseWidgets).
struct BlinkHost {
    virtual ~BlinkHost() = default;
    virtual void release_widgets_005a1c60(void* icon, std::int32_t arg) = 0;
};
void hud_map_icon_start_blink_005a21e0(void* icon, float now_00f876a4, BlinkHost& host);
// 0059ADC0: the lazily initialised colour (1, 1, 1, 1) at 00E19518, guard bit 1
// of 00E19528.
const float* static_white_0059adc0(std::uint32_t& guard_00e19528, float* colour_00e19518) noexcept;
// 005B5D50: award hint active when +C0h != 0.0 (00D7A218).
bool award_hint_is_active_005b5d50(const void* hint) noexcept;

// Unit icon map at screen+4F4h (std::map<unit, icon*>; value at node+10h).
// 005A22D0 FindUnitIcon, __thiscall(screen, unit), RET 4: 005A1800 lower-bound
// pair; the checked-iterator tests call 00BF6713 on a foreign container; the
// node's value when it is not end(), else null.
// 005A31A0, same lookup when the byte +05h is set, then RefreshSideTexture on
// the found icon.
struct UnitIconMapHost {
    virtual ~UnitIconMapHost() = default;
    // Returns {container, node}.
    virtual std::array<void*, 2> lower_bound_pair_005a1800(void* map, std::uint32_t unit) = 0;
    virtual void invalid_iterator_00bf6713() = 0;
    virtual void refresh_side_texture_0059d2a0(void* icon) = 0;
};
void* hud_map_find_unit_icon_005a22d0(void* screen, std::uint32_t unit, UnitIconMapHost& host);
void hud_map_refresh_unit_icon_texture_005a31a0(void* screen, std::uint32_t unit, UnitIconMapHost& host);
// 005BE110 minimap icon map find, __thiscall(map, out_iter, const uint32* key),
// RET 8: lower bound down from the root (key at +0Ch, nil byte +15h), then
// end() when the bound is the head or the key is below the bound's key.
void hud_minimap_icon_map_find_005be110(void* map, void** out_iter, const std::uint32_t* key) noexcept;

// ---------------------------------------------------------------------------
// Multiplayer mode selector and lobby settings (batch 2)
// ---------------------------------------------------------------------------

struct ModeSelectorHost : WidgetHost {
    virtual void set_listener_fields_00aa6bc0(std::uint32_t widget, void* listener, std::uint32_t arg) = 0;
    virtual void list_box_set_listener_00a9ac40(std::uint32_t list_box, void* listener) = 0;
    virtual void list_box_select_index_00a9c7c0(std::uint32_t list_box, std::int32_t index) = 0;
    virtual std::int32_t list_box_selected_index_00a9c920(std::uint32_t list_box) = 0;
    virtual void refresh_server_info_005ebdd0(void* screen) = 0;
    // 00A92C40(0.0f) on the 004BEC00 input manager.
    virtual void input_update_zero() = 0;
    virtual std::int32_t multi_menu_current_interface_20() = 0;  // [[00E198B4] + 20h]
    virtual void session_set_mode_host_00770e40() = 0;           // ECX = game+1EF0h
    virtual void session_set_mode_0076fad0(std::int32_t mode) = 0;
    virtual void push_request_interface_00688ad0(std::int32_t interface_id) = 0;
    virtual void pop_request_interface_006881f0() = 0;
    virtual void mode_selector_set_mode_005d5660(std::int32_t mode) = 0;  // ECX = mode
    virtual void mode_selector_apply_005662f0(std::int32_t mode) = 0;     // ECX = mode
    virtual bool game_byte_218c() = 0;
    virtual void clear_game_byte_218c() = 0;
    virtual void multi_main_refresh_online_state_005e7900() = 0;  // ECX = [[00E198B4] + 40h]
};
// 005EC4C0 SetCustomMode, __thiscall(screen, custom), RET 4: +1Ch +34h(custom),
// +E0h = custom, +A8h +60h(custom), +20h +60h(!custom); the listener pair
// (screen+0Ch for 00AA6BC0, screen+8 for 00A9AC40) goes to +20h when not custom
// and to +A8h when custom, the other list box getting null; when custom: +A8h
// shown, +77h = 0, row 0 selected, RefreshServerInfo. Then 00A92C40(0.0f).
void mode_selector_set_custom_mode_005ec4c0(void* screen, bool custom, ModeSelectorHost& host);
// 005ECA90 GoBack: in custom mode (+E0h) and only when +F8h == 3, leave it the
// same way (hide +1Ch, +E0h = 0, enables, listeners back on +20h, input update);
// outside custom mode clear the game byte +218Ch and refresh the online state
// when it was set, else pop the request interface; both then 00E19614 = 0.
void mode_selector_go_back_005eca90(void* screen, std::int32_t& index_00e19614, ModeSelectorHost& host);
// 005EC990 list selection, __thiscall(listener = screen+8, list_box), RET 8. Only
// on interface 10h. 00E19614 = selected index; above 5 returns. By 00E19610:
// 0 -> SetModeHost and request interface 14h; 1 -> SetCustomMode(1); 2 ->
// session mode 2 and request 1Ah. Then the row maps to a mode
// {0:5, 1:6, 2:7, 3:4, 4:3, 5:-1} (any other: 0), +ECh = the selected index
// again, and 005D5660 / 005662F0 with that mode.
inline constexpr std::array<std::int32_t, 6> kModeSelectorRowModes{5, 6, 7, 4, 3, -1};  // table 005ECA6C
void mode_selector_on_list_select_005ec990(void* listener, std::uint32_t list_box, std::int32_t mode_00e19610,
    std::int32_t& index_00e19614, ModeSelectorHost& host);
// 005EAAC0 listener +18h, __thiscall(listener, widget, focused), RET 8. Type 3:
// only text widgets whose first byte (+F0h text, or the empty 00E19618) is A2h or
// A3h (the arrow glyphs) get the 0.5/1.0 scale. Otherwise, with +D4h set only
// the widgets +A4h and +A8h pass; with it clear, +90h passes only while +78h is
// clear; passing widgets get the type 6 / 12h styling.
void mode_selector_listener_current18_005eaac0(const void* listener, std::uint32_t widget, bool focused,
    const char* empty_00e19618, WidgetHost& host);

// Lobby settings: the 14 dwords at 00E1955C..00E19590 mirror screen+18h..+4Ch.
struct LobbySettingsHost {
    virtual ~LobbySettingsHost() = default;
    virtual void session_request_player_ready_00771dd0(std::int32_t a, std::int32_t b) = 0;  // ([game+18C8h], 0, 1)
    virtual bool menus_present() = 0;                    // 00E198AC and 00E198B4 non-null
    virtual bool multi_menu_query_00687860(std::int32_t a, std::int32_t b) = 0;
    virtual void tactical_library_leave_00515bc0() = 0;  // ECX = [00E198AC + 70h]
    virtual void game_set_game_mode_004bc890(std::uint32_t mode, std::int32_t arg) = 0;
    virtual void* lobby_screen_if_visible() = 0;         // [[00E198B4] + 48h] when its byte +05h is set
    virtual void refresh_setting_rows_005e3580(void* screen) = 0;
    // StepMapIndex: mission records (434h bytes) under [[00E198AC]+5Ch]+24h.
    virtual std::int32_t mission_record_count() = 0;
    virtual void invalid_iterator_00bf6713() = 0;
    virtual void* mission_record(std::int32_t index) = 0;
    virtual void* option_registry_00576b10() = 0;
    // 005C6850 table[2] then 005C49C0 [mode]: the availability byte.
    virtual std::uint8_t mode_available(void* option_table, std::int32_t key, std::uint32_t mode) = 0;
};
// 005E50D0 CheckSettingsChanged (ECX = screen).
void lobby_check_settings_changed_005e50d0(void* screen, std::uint32_t* settings_00e1955c, LobbySettingsHost& host);
// 005E1BF0 StepMapIndex, __thiscall(screen, delta), RET 4. With count =
// [00F1AF2C] > 0: up to count tries of index = (count + index + delta) % count
// (signed), stopping at the first map whose option table (record+428h when
// record+424h is set, else the registry's +28h) has byte [2][mode 00E19564] == 1;
// a changed index is stored to 00E19594. A non-zero delta clears +27Ch.
void lobby_step_map_index_005e1bf0(void* screen, std::int32_t delta, std::int32_t count_00f1af2c,
    std::uint32_t& map_00e19594, std::uint32_t mode_00e19564, LobbySettingsHost& host);

// ---------------------------------------------------------------------------
// Main-menu async completions (batch 2)
// ---------------------------------------------------------------------------

struct MainMenuAsyncHost {
    virtual ~MainMenuAsyncHost() = default;
    virtual void* online() = 0;  // [00F8A2FC], may be null
    // Online vtable +3Ch: the 3Ch-byte status record copied by 00588640.
    virtual const void* online_status_record_3c(void* online) = 0;
    virtual void online_virtual_38(void* online) = 0;
    virtual void build_top_level_page_00584ae0(void* screen, std::int32_t arg) = 0;
    virtual std::uint32_t lua_reader_pending_count() = 0;  // [[0109CECC] + 8]
    virtual void settings_apply_all_008d5b50() = 0;
    virtual void* main_menu_screen() = 0;   // [00E198AC + 58h]
    virtual void* mission_tree() = 0;       // [00E198AC + 5Ch]
    virtual void* profile_650() = 0;        // [00E188A8] + 650h
    virtual void mission_tree_select_by_id_005c35d0(void* tree, const void* id) = 0;
    virtual void on_play_mission_prompt_result_00592640(void* screen, std::int32_t result) = 0;
    virtual void free_block(void* block) = 0;
};
// 00590A00 FinishAsyncTextRequest, __thiscall(screen, unused), RET 4: with an
// online object, copy its status record into a local (00588640); when the
// record's byte +3Bh is set, online vtable +38h and screen+500h = 1; the local's
// legacy string is freed when its capacity (+18h) is 10h or more. Then
// BuildTopLevelPage(0) on the screen.
void main_menu_finish_async_text_request_00590a00(void* screen, MainMenuAsyncHost& host, LegacyStringHost& strings);
// 00590D80 profile-loaded callback: with nothing pending in the Lua reader,
// ApplyAll, BuildTopLevelPage(1), and select the profile's mission id (copied
// from profile+650h+68h through 00586150) in the mission tree; otherwise
// FinishAsyncTextRequest(1) on the menu screen.
void main_menu_on_profile_loaded_00590d80(MainMenuAsyncHost& host, LegacyStringHost& strings,
    NativeStringStorage& storage);
// 00592AD0 prompt thunk, ECX = result: 00592640(result) on the menu screen.
void main_menu_on_play_mission_prompt_result_thunk_00592ad0(std::int32_t result, MainMenuAsyncHost& host);

// ---------------------------------------------------------------------------
// Options and pause helpers (batch 2)
// ---------------------------------------------------------------------------

struct OptionsWidgetHost {
    virtual ~OptionsWidgetHost() = default;
    // BSP_GuiWidget_FindChildByName 00AA7E00(name, 1), ECX = parent.
    virtual std::uint32_t find_child_by_name_00aa7e00(std::uint32_t parent, std::string_view name) = 0;
    virtual void set_listener_fields_00aa6bc0(std::uint32_t widget, void* listener, std::uint32_t arg) = 0;
    virtual std::uint32_t list_box_row_00a9be00(std::uint32_t list_box, std::int32_t row) = 0;
    virtual void text_set_localised_source_00abaed0(std::uint32_t text, const void* source, std::int32_t arg) = 0;
    // Slider events.
    virtual void invalid_iterator_00bf6713() = 0;
    virtual bool input_devices_empty() = 0;  // [00F8BBF4 + 94h] null or empty
    virtual std::int32_t widget_type_5c(std::uint32_t widget) = 0;
    virtual std::int32_t slider_value_from_row_005f2290(std::uint32_t widget) = 0;
    virtual void list_box_select_row_00a9c740(std::uint32_t list_box, std::int32_t row) = 0;
    virtual void adjust_focused_value_005f42d0(void* screen, std::int32_t value) = 0;
};
// 005F0390: ECX = parent, EDX = listener: the child "dodgeListboxAutoControl_Group"
// (00CEE6D0) gets 00AA6BC0(listener, 0). The pooled name is released afterwards.
inline constexpr std::string_view kAutoControlGroupName = "dodgeListboxAutoControl_Group";
void options_find_auto_control_group_005f0390(std::uint32_t parent, void* listener, OptionsWidgetHost& host);
// 005F0730, __thiscall(screen, row, source), RET 8: the "var_Text" (00CEE66C)
// child of row `row` of the list box +88h gets the localised source (arg 1).
inline constexpr std::string_view kRowVarTextName = "var_Text";
void options_set_row_var_text_005f0730(void* screen, std::int32_t row, const void* source, OptionsWidgetHost& host);
// 005F4C10 slider listener (ECX = screen+0Ch), __thiscall(listener, widget), RET 4:
// the checked front() of the input-device vector at [00F8BBF4]+94h calls 00BF6713
// when it is empty; a slider (type 2) whose +D8h is not 1 and whose row gives a
// non-zero value selects its row (+70h) in the list box (listener+7Ch) and
// adjusts the focused value on the screen.
void options_on_slider_event_005f4c10(void* listener, std::uint32_t widget, OptionsWidgetHost& host);

struct LuaCommandHost {
    virtual ~LuaCommandHost() = default;
    // BSP_LuaMachine_RunString 006B8AD0(text, 0, 0, 2), ECX = [[game + 1A08h] + 4].
    virtual std::int32_t lua_run_string_006b8ad0(const char* text) = 0;
    // 004254B0(format, text): the debug log.
    virtual void log_004254b0(const char* format, const char* text) = 0;
    // 005F60F0: the reader set-up and teardown around the keyboard archive.
    virtual std::uint32_t lua_reader_pending_count() = 0;  // [[0109CECC] + 8]
    virtual void read_keyboard_archive_from_globals() = 0;  // 00B67980, 004425C0, 005547D0/006ABA50
    virtual void* owner_buffer_30() = 0;                    // [[0109CECC] + 30h]
    virtual void clear_owner_buffer_30() = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void lua_state_owner_close_00b65e80() = 0;      // ECX = [0109CECC] + 38h
    virtual void lua_reader_destroy_00441a20() = 0;
};
// 005FD360, __stdcall(NativeString command) by value (8 bytes), RET 8:
// with a non-empty command run it (null data runs the empty string 00E19684)
// and log success (00CF3E40) or failure (00CF3E78) in the original Hungarian;
// then release the by-value string.
inline constexpr const char* kLuaCommandRan = "%s lua parancs vegrehajtva az ingame pausemenuben.";
inline constexpr const char* kLuaCommandFailed = "Nem sikerult vegrehajtani a: %s lua parancsot az ingame pausemenuben.";
void hud_pause_run_lua_command_005fd360(std::uint32_t length, char* data, LuaCommandHost& host,
    NativeStringStorage& storage);
// 005F60F0: only with nothing pending in the Lua reader: read the keyboard
// archive through a reader over the globals, free and clear the owner's +30h
// buffer, close the state owner, destroy the reader.
void lua_globals_ensure_reader_005f60f0(LuaCommandHost& host);

// ---------------------------------------------------------------------------
// HUD map and minimap object pools (batch 3)
// ---------------------------------------------------------------------------

struct MapPoolHost {
    virtual ~MapPoolHost() = default;
    virtual void invalid_iterator_00bf6713() = 0;
    virtual void delete_object(void* object) = 0;  // (*vtable[0])(1)
    virtual void* operator_new(std::size_t size) = 0;
    virtual void* arrow_widget_construct_005a3610(void* memory, void* screen) = 0;
    virtual void pointer_array_push_back_0059fad0(void* array_slot, void* const* value) = 0;
    virtual void** map_subscript_00538910(void* map, std::uint32_t unit) = 0;
    virtual void* unit_marker_construct_005a6ad0(void* memory, std::uint32_t unit, std::uint32_t scene, void* owner_20) = 0;
    virtual void observer_register_pair_00694a60(std::uint32_t unit, void* observer) = 0;  // ECX, EDX
    virtual void update_unit_icon_005adda0(void* screen, void* icon) = 0;
    virtual void* tree_buy_head_node_005bdd40(void* tree) = 0;
    virtual void frontend_construct_004f7180(void* screen) = 0;
};
// 005A3AA0: delete every marker object of the vector [+580h, +584h) (index
// checked against the re-read size; each slot nulled), then erase the whole
// range (the memmove of the empty tail is skipped; +584h = +580h). The
// checked-iterator tests call 00BF6713 when first > last.
void hud_map_delete_marker_objects_005a3aa0(void* screen, MapPoolHost& host);
// 005A4A00 AcquireArrowWidget: ++(+548h); an index inside the pool (+544h)
// returns the pooled widget; otherwise a new 28h-byte arrow widget (005A3610 with
// the screen) is pushed onto the pool (0059FAD0) and returned.
void* hud_map_acquire_arrow_widget_005a4a00(void* screen, MapPoolHost& host);
// 005B2CB0, __thiscall(screen, unit), RET 4: the unit's icon slot in the map at
// +4F4h (00538910); when empty, a new 1A0h-byte unit marker (005A6AD0 with unit,
// [+0Ch], screen+20h), observer registration of the unit with screen+64h, and
// +598h = 1. Then icon+08h = 1 and UpdateUnitIcon(icon).
void hud_map_get_or_create_unit_icon_005b2cb0(void* screen, std::uint32_t unit, MapPoolHost& host);
// 005C0810 HudMinimapScreen constructor: FrontEndScreen_Construct; +08h base
// vtable 00CE3CD4 with +0Ch..+14h = 0 and +18h = 0; vtables 00CF1404 / 00CF13EC;
// the icon map at +F8h (head from 005BDD40, nil byte +15h, size +100h = 0); and
// the vector/array words +108h..+110h, +118h..+120h, +128h..+130h,
// +138h..+140h, +148h..+150h, +180h, +184h zeroed.
void* hud_minimap_screen_construct_005c0810(void* screen, MapPoolHost& host);

// Two-page registers (batch 3). Page names read from the image.
//   005E1820 MultiGameLobby: "FE_lobby" (00CF242C) -> +14h, "FE_lobby_settings"
//            (00CF2418) -> +18h; vtable +14h; LoadWeatherTable 005E12A0;
//            +2F4h = +2F0h = +2FCh = +2F8h = -1; +2C8h = +2CCh = +2C0h = +2C4h = 0;
//            byte +2DCh = 0; +274h = 1.
//   005F0250 Options: "_Options" (00CF38C4) -> +7Ch, "FE_options" (00CF38B8) ->
//            +80h; +A8h = 0, bytes +25Ch = +78h = 0; then vtable +14h.
inline constexpr std::string_view kLobbyPage = "FE_lobby";
inline constexpr std::string_view kLobbySettingsPage = "FE_lobby_settings";
inline constexpr std::string_view kOptionsBackgroundPage = "_Options";
inline constexpr std::string_view kOptionsPage = "FE_options";
struct LobbyRegisterHost : ScreenLifetimeHost {
    virtual void load_weather_table_005e12a0(void* screen) = 0;
};
void lobby_register_005e1820(void* screen, LobbyRegisterHost& host);
void options_register_005f0250(void* screen, ScreenLifetimeHost& host);

// HudMinimapScreen exit and destructor (batch 3).
struct MinimapLifetimeHost : ScreenLifetimeHost {
    virtual void invalid_iterator_00bf6713() = 0;
    virtual void delete_object(void* object) = 0;  // (*vtable[0])(1)
    virtual void observer_unregister_pair_006952a0(std::uint32_t unit, void* observer) = 0;  // ECX, EDX
    virtual void* tree_next_005bcf70(void* node) = 0;
    virtual void tree_erase_subtree_005bdc20(void* tree, void* node) = 0;
    virtual void tree_erase_range_005bfac0(void* tree, void* first, void* last) = 0;
    virtual void hide_unused_markers_005c0bd0(void* screen) = 0;
    virtual void callback_owner_destroy_00695870(void* owner) = 0;
    virtual void destroy_native_string(void* header) = 0;  // the 0041DD20 body inlined at 005C056A
};
// 005C0E10 exit: for every icon entry (map at +F8h, key +0Ch, value +10h) delete
// the value and unregister the unit from screen+8; then clear the map (the
// inline _Erase: right subtree through 005BDC20, free the node, continue left)
// and reset its head and size; +154h..+164h = -1; tail JMP HideUnusedMarkers.
void hud_minimap_screen_exit_005c0e10(void* screen, MinimapLifetimeHost& host);
// 005C0520 destructor: vtables 00CF1404 / 00CF13EC; destroy the page +1Ch; the
// native string at +180h; the vectors +148h, +138h, +128h, +118h, +108h; the
// icon map (erase range 005BFAC0, free the head, +FCh = +100h = 0); the
// callback owner at +08h (00695870); FrontEndScreen_Destruct.
void hud_minimap_screen_destruct_005c0520(void* screen, MinimapLifetimeHost& host);

// OptionsScreen constructor and destructor (batch 3).
struct OptionsLifetimeHost : ScreenLifetimeHost {
    virtual void game_settings_construct_008d7710(void* settings) = 0;
    virtual void game_settings_destroy_008d78d0(void* settings) = 0;
    virtual void* malloc_00bf9f1a(std::size_t size) = 0;
    virtual void free_00bf9dc8(void* block) = 0;
};
// 005F6030: FrontEndScreen_Construct; vtables 00CF3974 / 00CF3960 / 00CF3940;
// +C4h = 0 and the vector +CCh..+D4h zeroed; GameSettings_Construct on the two
// edit copies +D8h and +194h; bytes +264h = +265h = 0; +70h = 10 and +26Ch =
// malloc(10) (the language-name buffer 005EF550 clears).
void* options_screen_construct_005f6030(void* screen, OptionsLifetimeHost& host);
// 005F1BB0: vtables back; free +26Ch (00BF9DC8); destroy the pages +7Ch and
// +80h; GameSettings_Destroy on +194h then +D8h; +C4h = 0 and the vector +CCh
// freed; +08h = 00CEB0FC; FrontEndScreen_Destruct.
void options_screen_destruct_005f1bb0(void* screen, OptionsLifetimeHost& host);

// HudPauseScreen constructor and destructor (batch 3).
struct PauseLifetimeHost : ScreenLifetimeHost {
    virtual void* list_allocate_sentinel_00582280(void* list) = 0;
    virtual void* tree_buy_head_node_005fd1b0(void* tree) = 0;
    virtual void* string_list_allocate_sentinel_004c3020(void* list) = 0;
    virtual void string_list_clear_storage_004d05e0(void* list) = 0;
    virtual void tree_erase_range_005fedc0(void* tree, void* first, void* last) = 0;
    virtual void list_tidy_005822a0(void* list) = 0;
};
// 005FFCB0: FrontEndScreen_Construct; vtables 00CF3EF4 / 00CF3EE0 / 00CF3EC0;
// list +64h (sentinel 00582280 at +68h, size +6Ch = 0); scroller at +9Ch; hint
// map +11Ch (head 005FD1B0, nil byte +19h, size +124h = 0); pooled string list
// +128h (sentinel 004C3020 at +12Ch, size +130h = 0).
void* hud_pause_screen_construct_005ffcb0(void* screen, PauseLifetimeHost& host);
// 005FF980: vtables back; pages +14h, +18h, +1Ch; clear the string list and
// free its sentinel; erase the hint map (005FEDC0), free its head, zero +120h and
// +124h; tidy the list +64h; +08h = 00CEB0FC; FrontEndScreen_Destruct. The
// scroller at +9Ch is not destroyed here.
void hud_pause_screen_destruct_005ff980(void* screen, PauseLifetimeHost& host);

// ---------------------------------------------------------------------------
// Mission tree tables (batch 3)
// ---------------------------------------------------------------------------

// The table reader's virtuals take 8-byte keys {kind, value} by value; kind 1
// is an integer index.
struct TableReaderHost {
    virtual ~TableReaderHost() = default;
    virtual void enter_04(std::uint32_t key_kind, std::uint32_t key_value) = 0;  // vtable +04h
    virtual bool has_index_14(std::int32_t index) = 0;                           // +14h({1, index})
    virtual std::int32_t read_int_10(std::int32_t index) = 0;                    // +10h({1, index}, {1, &out})
    virtual void leave_08() = 0;                                                 // +08h
    virtual void invalid_iterator_00bf6713() = 0;
    virtual void erase_all_004685a0(void* vector) = 0;              // erase(begin, end)
    virtual void insert_one_00441f30(void* vector, void* where, const std::int32_t* value) = 0;
};
// 005C4AD0 ReadIntVector, __thiscall(reader, Key key, vector*), RET 0Ch: clear
// the vector {+04h first, +08h last, +0Ch end} (checked first <= last), enter
// the key, then for index 1, 2, ... while the reader has it: read the integer
// into the key argument's stack slot and push it (in place when capacity
// remains, else 00441F30 at last); finally leave.
void mission_tree_read_int_vector_005c4ad0(std::uint32_t key_kind, std::uint32_t key_value, void* vector,
    TableReaderHost& host);
// 005C4BE0 side-block constructor (154h bytes): byte +00h = 0, +04h = +08h = 0,
// and the twenty vectors whose first/last/end words sit at +18h + 10h*k
// (k = 0..19) zeroed.
void mission_side_block_construct_005c4be0(void* block) noexcept;

// 00594290: JMP 00593570 (MissionScoreRecord_Destruct), the target of the unwind
// funclet at 00C70F20. The Ghidra body shown under this name is the target's.
void mission_score_record_destroy_members_00594290(void* record, MissionScoreHost& host);

// ---------------------------------------------------------------------------
// Multiplayer main menu list handlers (packet cc13-fe1b)
// ---------------------------------------------------------------------------

// ECX is the screen's listener sub-object (screen+8); its +18h is the ranked /
// player list box and +1Ch the quick-match list box (screen+20h / +24h).
struct MultiMainListHost : WidgetHost {
    virtual std::int32_t list_box_selected_data_00a9c990(std::uint32_t list_box) = 0;
    virtual std::int32_t list_box_selected_index_00a9c920(std::uint32_t list_box) = 0;
    // The help-text tables: 00E08980 (quick match) and 00E0896C (ranked / player).
    virtual const char* help_text(std::uint32_t table, std::int32_t index) = 0;
    // 0054A0C0 on [00E1930C], with a pooled native string built by 0041E870.
    virtual void command_bar_set_help_line_0054a0c0(const char* text) = 0;
    virtual void start_hosting_005e6f00(void* screen) = 0;
    virtual void open_friends_005e6c70(void* screen) = 0;
    virtual void push_request_interface_00688ad0(std::int32_t interface_id) = 0;
    virtual void set_online_ranked_4d(bool ranked) = 0;  // [[00F8A2FC] + 4Dh]
    virtual void frame_set_title_005189b0(const char* title) = 0;
    virtual void set_buttons_visible_005e6b10(void* screen, bool shown) = 0;
    virtual void input_update_zero() = 0;  // 00A92C40(0.0f)
};
inline constexpr std::uint32_t kQuickHelpTable = 0x00E08980u;   // "FE.multi_quick_help", ...
inline constexpr std::uint32_t kRankedHelpTable = 0x00E0896Cu;  // "FE.multi_ranked_help", ...
inline constexpr const char* kRankedTitle = "FE.multi_ranked_title";  // 00CF2748
inline constexpr const char* kPlayerTitle = "FE.multi_player_title";  // 00CF2730
// 005E73A0 selection changed, __thiscall(listener, unused, list_box), RET 8: for
// the quick list (+1Ch) or the ranked list (+18h), when that list's byte +85h is
// set: enable it (+60h(1)) and show the help line of its selected row from the
// matching table.
void multi_main_on_listbox_selection_changed_005e73a0(void* listener, std::uint32_t list_box,
    MultiMainListHost& host);
// 005E7160 activate, __thiscall(listener, unused, list_box), RET 8. The selected
// row's data picks the action. Quick list (+1Ch): +ECh (listener) = the selected
// index; data 0 starts hosting, 1 clears 00E08701 / 00E08700, sets 00E19610 = 2
// and requests interface 10h, 2 opens the friends list. Ranked list (+18h): data
// 0 sets the online ranked byte and the ranked title, then shows the buttons;
// data 1 clears it, sets the player title and hides them; any other data skips
// to the swap. The swap: ranked list disabled, hidden, +77h = 1; quick list
// enabled, shown, +77h = 0; the help line of the ranked list's selected row from
// the quick table (as the image does); input update 0.0.
struct MultiMainHostingFlags {
    std::uint8_t* e08700;
    std::uint8_t* e08701;
    std::int32_t* e19610;
};
void multi_main_on_listbox_activate_005e7160(void* listener, std::uint32_t list_box,
    const MultiMainHostingFlags& flags, MultiMainListHost& host);

// Widget press handlers, widget-listener slot +0Ch, __thiscall(listener =
// screen+0Ch, widget), RET 4. Text widgets (type 3) act on their first glyph:
// A2h is "accept", A3h "back" (+F0h text, or an empty string when null).
// Buttons (6) and toggles (12h) end with +88h(2, 0, 1.0f) / +84h(2).
struct PressHost : WidgetHost {
    // The list-box listener sub-object at listener-4 (screen+8), its vtable +04h
    // (the activate slot) with (row value, list box).
    virtual void list_listener_activate_04(void* list_listener, std::uint32_t value, std::uint32_t list_box) = 0;
    virtual std::uint32_t list_box_value_00425e50(std::uint32_t list_box) = 0;
    virtual void scroller_begin_thumb_drag_00683a40(void* scroller) = 0;
    virtual void scroller_scroll_toward_end_00683700(void* scroller, float speed) = 0;
    virtual void scroller_scroll_toward_start_00683680(void* scroller, float speed) = 0;
    virtual void scroller_set_auto_scroll_00683300(void* scroller, std::int32_t a, float speed, std::int32_t b) = 0;
    virtual void hud_pause_resume_005fd440(void* screen) = 0;
    virtual void mode_selector_refresh_map_list_005ec250(void* screen) = 0;
    virtual void mode_selector_go_back_005eca90(void* screen) = 0;
    virtual void mode_selector_step_option_005ecba0(void* screen, std::int32_t delta) = 0;
    virtual void list_box_select_index_00a9c7c0(std::uint32_t list_box, std::int32_t index) = 0;
    // +88h / +84h with the integer state (2 = pressed; the focus styling passes 0/1).
    virtual void set_frame_state_88(std::uint32_t widget, std::int32_t state, std::int32_t mode, float weight) = 0;
    virtual void set_toggle_state_84(std::uint32_t widget, std::int32_t state) = 0;
};
// 005FDB50 HudPause: text widgets get +4Ch(1.0); A2h activates the current row of
// the list box (listener+130h) through the list listener; B7h/B8h do nothing;
// A3h clears 00E19660 and resumes. Other widgets: the scroll thumb (+70h)
// starts a drag, the arrows +6Ch / +68h scroll the scroller (+90h) toward the
// end / start at 0.5; then the press styling.
void hud_pause_on_press_005fdb50(void* listener, std::uint32_t widget, std::int32_t& page_00e19660,
    const char* empty_00e19688, PressHost& host);
// 005ED450 MultiModeSelector: A2h refreshes the map list in custom mode (+D4h)
// or activates through the list listener with (widget, [listener+14h]); A3h goes
// back; any other text widget equal to one of the four tabs +B4h..+C0h selects
// that index in the list box +9Ch. Other widgets in custom mode: +A4h / +A8h
// step the option by -1 / +1 (anything else returns without styling); outside
// it: +28h / +24h auto-scroll the scroller (+30h) with (2 / 1, 0.004f, 1), +90h
// starts a thumb drag; then the press styling.
inline constexpr float kModeSelectorAutoScrollSpeed = 0.004f;  // 00CEC178
void mode_selector_on_press_005ed450(void* listener, std::uint32_t widget, const char* empty_00e19618,
    PressHost& host);

// 005EBC90 MultiModeSelector selection changed, __thiscall(this, unused,
// list_box), RET 8 (offsets relative to ECX). The server list (+A0h) is ignored.
// 00E19614 = the selected index; help line from table 00E089A4[index]; for an
// index up to 5: title text +20h = 00E089BC[index] and description +24h =
// 00E0898C[index] (localised, arg 1), the mode icon +1Ch gets +88h(index & FFFFh,
// 0, 1.0f), the scroller at +34h re-attaches (00683790(0)) and gets the range
// height(+24h) * 1.5 + height(+20h) - size(+90h).y (x87, formed in double here).
struct ModeSelectorTextHost : WidgetHost {
    virtual std::int32_t list_box_selected_index_00a9c920(std::uint32_t list_box) = 0;
    virtual const char* table_string(std::uint32_t table, std::int32_t index) = 0;
    virtual void command_bar_set_help_line_0054a0c0(const char* text) = 0;
    virtual void text_set_localised_cstring_00abbe50(std::uint32_t text, const char* key, std::int32_t arg) = 0;
    virtual void set_frame_state_88(std::uint32_t widget, std::int32_t state, std::int32_t mode, float weight) = 0;
    virtual void scroller_attach_content_00683790(void* scroller, std::uint32_t content) = 0;
    virtual float widget_size_y_00aa6740(std::uint32_t widget) = 0;
    virtual float text_normalized_height_00ab6bd0(std::uint32_t text) = 0;
    virtual void scroller_set_scroll_range_006834a0(void* scroller, float range) = 0;
};
void mode_selector_on_listbox_selection_changed_005ebc90(void* self, std::uint32_t list_box,
    std::int32_t& index_00e19614, ModeSelectorTextHost& host);

// 005BC920 HudNarrative update, __thiscall(screen, float unused), RET 4. The
// panel shows while +54h > 0 or the narrative ([game+21E4h]) has +34h or +24h
// set: +34h, +38h, +3Ch, +40h get +34h(shown); hidden, the texts +34h and +38h
// are emptied (00CE3A0C). Then, with the game's frame time [game+21F0h] above
// zero: 005BBF10, 005BC640 (screen), 00735100 (narrative [game+21E8h]),
// 005B9800, 005B8C30 (screen), each with that time. The stack argument is not read.
struct NarrativeUpdateHost : WidgetHost {
    virtual std::int32_t narrative_34() = 0;   // [[game + 21E4h] + 34h]
    virtual std::int32_t narrative_24() = 0;   // [[game + 21E4h] + 24h]
    virtual float game_frame_time_21f0() = 0;  // [game + 21F0h]
    virtual void text_set_localised_cstring_00abbe50(std::uint32_t text, const char* key, std::int32_t arg) = 0;
    virtual void step(std::uint32_t routine, void* object, float dt) = 0;
    virtual void* mission_narrative_21e8() = 0;  // [game + 21E8h]
};
void hud_narrative_update_005bc920(void* screen, NarrativeUpdateHost& host);

// 005FD890 HudPause ShowPage, __thiscall(screen, ingame_page, list_page), RET 8.
// The "page" widget is the in-game interface's [[00E198C4]+E4h]+10h when
// ingame_page is set, else screen+14h.
//   list_page set: hide that widget, show +18h, SelectDefault, enable list box
//     +2Ch, select row 0, listener screen+8, list box vtable +80h(1). Returns
//     without touching +10h.
//   list_page clear: show the widget, hide +18h, clear the list box rows, delete
//     every row object of the list +64h (node+08h through +04h(1)) and free the
//     nodes; unless (ingame_page and [[00E198C4]+E4h]+1Ch is set), select the
//     frame "ingame.paused" (00CF1DE4) or, for an in-game page while the game
//     byte +218Ch is clear, "ingame.pause_multi_title" (00CF1DC8) through
//     00518D60(0, 0, 0, name); then +10h = 0.
struct PauseShowPageHost : WidgetHost {
    virtual std::uint32_t ingame_page_widget() = 0;  // [[00E198C4] + E4h] + 10h
    virtual bool ingame_page_locked_1c() = 0;        // [[00E198C4] + E4h] + 1Ch
    virtual bool game_byte_218c() = 0;
    virtual void frame_select_default_004c9680() = 0;
    virtual void frame_select_00518d60(const char* name) = 0;
    virtual void list_box_clear_rows_00a9bec0(std::uint32_t list_box) = 0;
    virtual void list_box_select_index_00a9c7c0(std::uint32_t list_box, std::int32_t index) = 0;
    virtual void list_box_set_listener_00a9ac40(std::uint32_t list_box, void* listener) = 0;
    virtual void list_box_virtual_80(std::uint32_t list_box, bool value) = 0;
    virtual void invalid_iterator_00bf6713() = 0;
    virtual void delete_owned(void* object) = 0;  // vtable +04h(1)
    virtual void free_block(void* block) = 0;
};
inline constexpr const char* kPausedFrame = "ingame.paused";                   // 00CF1DE4
inline constexpr const char* kPauseMultiTitleFrame = "ingame.pause_multi_title";  // 00CF1DC8
void hud_pause_show_page_005fd890(void* screen, bool ingame_page, bool list_page, PauseShowPageHost& host);

// 005F5BD0 OptionsScreen enter (ECX = screen). Unless the byte +25Ch is set,
// select frame "FE.opt_title_main" (00CEE884) with 00518D60(ECX = 6, EDX = 0, 0,
// name). Copy the live settings 00F88980 into the edit copy (005F52C0 on +D8h).
// The list box +84h is the "Main_Listbox" (00CEB334) child of page +7Ch in the
// front end ([game+61Fh] clear) and "Ingame_Main_Listbox" (00CEE830) in a
// mission; the other one is hidden. +8Ch = its "MainListbox_Text" (00CEC040)
// child; the list box is shown and enabled. The page in 00E19650 picks the
// builder (1 game, 4 control, else the main list). +260h = 0.1f, +251h = 0,
// then [00F8BBF4] vtable +0Ch.
struct OptionsEnterHost : WidgetHost {
    virtual void frame_select_00518d60(std::int32_t ecx, std::int32_t edx, std::int32_t arg, const char* name) = 0;
    virtual void copy_settings_005f52c0(void* edit_copy, const void* settings_00f88980) = 0;
    virtual bool game_in_mission_61f() = 0;
    virtual std::uint32_t find_child_by_name_00aa7e00(std::uint32_t parent, std::string_view name) = 0;
    virtual void build_page(std::uint32_t routine, void* screen) = 0;  // 005F58F0 / 005F40F0 / 005F54F0
    virtual void input_devices_virtual_0c() = 0;                       // [00F8BBF4] vtable +0Ch
    virtual const void* live_settings_00f88980() = 0;
};
inline constexpr const char* kOptionsMainTitle = "FE.opt_title_main";
inline constexpr std::string_view kMainListboxName = "Main_Listbox";
inline constexpr std::string_view kIngameMainListboxName = "Ingame_Main_Listbox";
inline constexpr std::string_view kMainListboxTextName = "MainListbox_Text";
void options_screen_enter_005f5bd0(void* screen, std::int32_t page_00e19650, OptionsEnterHost& host);

// 005FAD70 HudOrderScreen enter (ECX = screen). Hide +44h..+50h, show +54h.
// When the order code +84h is ECh: +74h = 0.0, +78h = 1, +7Ch = 0.0, +80h = 1,
// and the award tracker triggers hint "ORDPERMANENT" (00CF3CA4, arg 0) unless it
// is already the active hint (00449AF0 case-insensitive compare of the copied
// active name). Then by the controlled unit [00E188D8]:
//   kind 6: +83h = (unit[+738h] vtable +50h == 1); +10h shown when +83h is clear,
//     +0Ch when set; +18h shown, +14h hidden; +2Ch (when unit+284h is non-zero)
//     or +28h shown; +1Ch shown.
//   kind 18h (squadron): the same test on unit[+348h]; +10h / +0Ch as above;
//     +18h hidden, +14h, +1Ch, +34h shown, +30h hidden.
struct HudOrderEnterHost : WidgetHost {
    virtual std::uint32_t controlled_unit() = 0;  // [00E188D8]
    virtual bool is_kind(std::uint32_t unit, std::int32_t kind) = 0;
    virtual std::int32_t order_mode(std::uint32_t component) = 0;  // component vtable +50h
    virtual bool active_hint_is(const char* name) = 0;  // 0068E5B0 copy + 00449AF0
    virtual void trigger_hint_00690fd0(const char* name, std::int32_t arg) = 0;
};
inline constexpr const char* kPermanentOrderHint = "ORDPERMANENT";
void hud_order_screen_enter_005fad70(void* screen, HudOrderEnterHost& host);

// ---------------------------------------------------------------------------
// HudPauseScreen bodies (packet cc13-fe6, docs/CC13_FE_6.md)
// ---------------------------------------------------------------------------
//
// HudPauseScreen layout these bodies use (screen offsets): +04h active byte,
// +08h listbox listener, +10h hint page shown, +14h main page, +18h hint page,
// +1Ch objectives panel, +2Ch hint list box, +30h / +34h primary / secondary
// objective list boxes, +50h objective row template, +5Ch / +60h the
// "felki_bal_Icon" / "felki_jobb_Icon" arrows, +9Ch hint scroller, +100h float,
// +11Ch map<int, NativeString> hint row -> hint name (005FD220 find, value at
// node+10h), +128h list<NativeString> of expanded hint names (head +12Ch), +134h
// hint list top row, +138h hint selected row, +13Ch "Main_Listbox", the seven
// rows +140h resume, +148h options, +14Ch points, +150h restart, +154h quit,
// +158h exit, +15Ch tips, and +160h the last objective row.
struct PauseScreenHost : WidgetHost {
    virtual void* hud_interface_00e198c4() = 0;
    virtual void* game_00e188a8() = 0;
    virtual void* award_tracker_004e1ca0() = 0;
    virtual void* command_bar_00e1930c() = 0;
    virtual void frame_select_00518d60(const char* name) = 0;  // ECX = EDX = 0, stack 0
    virtual void frame_select_default_004c9680() = 0;
    virtual void hud_order_close_005f9520() = 0;
    virtual void hud_ship_close_order_menu_0064a2f0() = 0;
    virtual std::uint32_t find_child_by_name_00aa7e00(std::uint32_t parent, std::string_view name) = 0;
    virtual void list_box_clear_items_00a9bf90(std::uint32_t list_box) = 0;
    virtual std::uint32_t clone_subtree_00aab4c0(std::uint32_t widget, std::int32_t arg) = 0;
    virtual void text_set_localised_cstring_00abbe50(std::uint32_t text, const char* key, std::int32_t arg) = 0;
    virtual void text_set_localised_source_00abaed0(std::uint32_t text, const void* source, std::int32_t arg) = 0;
    virtual void list_box_insert_row_00a9d750(std::uint32_t list_box, std::uint32_t row, std::int32_t a,
        std::int32_t b) = 0;
    virtual void widget_virtual_7c(std::uint32_t widget, bool value) = 0;  // widget vtable +7Ch(bool)
    virtual void list_box_select_index_00a9c7c0(std::uint32_t list_box, std::int32_t index) = 0;
    virtual void list_box_set_listener_00a9ac40(std::uint32_t list_box, void* listener) = 0;
    virtual void list_box_virtual_80(std::uint32_t list_box, bool value) = 0;
    virtual void list_box_reset_highlight_00a9cf30(std::uint32_t list_box, std::int32_t a, std::int32_t b) = 0;
    virtual std::int32_t list_box_selected_index_00a9c920(std::uint32_t list_box) = 0;
    virtual void list_box_rebuild_page_00a9d870(std::uint32_t list_box) = 0;
    virtual void list_control_set_item_visible_00a9c380(std::uint32_t list_box, std::int32_t index,
        bool visible) = 0;
    virtual std::array<float, 3> widget_resolved_position_00aa6750(std::uint32_t widget) = 0;
    virtual void widget_set_resolved_position_00aa8240(std::uint32_t widget, const std::array<float, 3>& position) = 0;
    // 004B7DE0: in-order successor of a std::map node.
    virtual void* tree_next_004b7de0(void* node) = 0;
    virtual void scroller_update_00683820(void* scroller, std::int32_t arg) = 0;
    virtual void update_hints_006037d0(void* screen) = 0;
    virtual void build_hint_list_00601200(void* screen) = 0;
    virtual bool input_was_pressed_004c43c0(std::int32_t action) = 0;  // ECX = [00E188A8]
    virtual void show_page_005fd890(void* screen, bool ingame_page, bool list_page) = 0;
    virtual void resume_005fd440(void* screen) = 0;
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    virtual void command_bar_set_help_line_0054a0c0(const char* text) = 0;
    virtual void profile_hints_00427190(std::int32_t hint) = 0;  // ECX = 004C1E90()
    virtual void commit_visibility_004f83b0(void* screen) = 0;
    virtual void screen_virtual_18(void* screen) = 0;
    virtual void input_manager_update_00a92c40(float dt) = 0;  // ECX = 004BEC00()
    // [00E198B8]; null until created.
    virtual void* options_menu_00e198b8() = 0;
    // operator new(4Ch) + 00689800 into 00E198B8 (null when the allocation
    // fails), then its vtable +04h.
    virtual void options_menu_create_00689800() = 0;
    virtual void options_menu_virtual_08() = 0;
    virtual void set_options_page_00e19650(std::int32_t page) = 0;
    // 00531B00 on 00425D10() with (slot, message, kind, callback, 0, title "",
    // timeout 0.0f, timeout result 0, countdown "", dismissible 1), RET 2Ch.
    virtual void raise_prompt_00531b00(std::int32_t slot, const char* message, std::int32_t kind,
        std::uint32_t callback) = 0;
    virtual void focus_last_prompt_navigation_00530c20() = 0;  // ECX = 00425D10()
    // 005FD220 map find on +11Ch: the hint name, or null at end().
    virtual const NativeString* hint_name_at_row_005fd220(void* screen, std::int32_t row) = 0;
    virtual bool equals_insensitive_00435c40(const NativeString& a, const NativeString& b) = 0;
    // 005FE8B0: std::list::remove of every equal element (list +128h).
    virtual void expanded_hints_remove_005fe8b0(void* screen, const NativeString& name) = 0;
    // 004CE6F0 + 004CE780 + relink: push_back a copy onto list +128h.
    virtual void expanded_hints_push_back(void* screen, const NativeString& name) = 0;
};

// 00603120 HudPause enter, slot +18h of 00CF3EF4, __fastcall(ECX = screen), RET.
// Selects "ingame.paused"; closes the order screen ([[00E198C4]+64h] +81h or
// +82h set: 005F9520) and the ship order menu ([[00E198C4]+78h] +156h or +157h:
// 0064A2F0); clears [[00E198C4]+60h]+4. Binds +13Ch = "Main_Listbox" under
// page +14h, +5Ch / +60h = "felki_bal_Icon" / "felki_jobb_Icon" under +13Ch,
// hides both and +18h, clears +13Ch, hides its "MainListbox_Text" template and
// clones seven rows of it in order resume, tips, points, options, restart, quit,
// exit (keys 00E08C40..00E08C58). tips is greyed through +7Ch(tracker+9Ch == 0);
// points through +7Ch(game byte +218Ch clear and game+1FE4h == 0). The list box
// is enabled, row 00E19660 selected, listener screen+8, +80h(1), highlight
// reset (1, 1). The arrows sit either side of the tips row:
//   off = (float)((float)((float)(width+114h / 960.0) * 0.5) + 0.015f)
//   left  = pos(tips) + (-off, -0.001f, -0.0), right = pos(tips) + (off, -0.001f, +0.0)
// with every intermediate rounded to float (FSTP dword). The command bar byte
// [00E1930C]+4 = 1; the objective list boxes +30h / +34h are cleared and refilled
// from the local objective set ([game + 21A4h + [game+18ECh]*4] + 24h, a
// std::list): each active objective (+1Ch == 0, 008DA130) of kind +18h 0 goes
// to +30h and kind 1 to +34h while that box holds fewer than 3 rows (+104h), as
// a clone of +50h (kept in +160h) with localised source objective+0Ch. Then the
// objectives panel +1Ch is shown.
inline constexpr std::array<const char*, 7> kPauseRowKeys{
    "ingame.pause_resume", "ingame.pause_tips", "ingame.pause_points", "ingame.pause_options",
    "ingame.pause_restart", "ingame.pause_quit", "ingame.pause_exit"};  // via 00E08C40..00E08C58
inline constexpr std::array<std::uint16_t, 7> kPauseRowOffsets{0x140, 0x15C, 0x14C, 0x148, 0x150, 0x154, 0x158};
inline constexpr std::string_view kFelkiBalIcon = "felki_bal_Icon";    // 00CF1DB8
inline constexpr std::string_view kFelkiJobbIcon = "felki_jobb_Icon";  // 00CF1DA8
inline constexpr double kPauseArrowWidthScale = 960.0;  // 00CEC380
inline constexpr float kPauseArrowPad = 0.015f;         // 00CEE6C8 (float widened to double)
inline constexpr float kPauseArrowDrop = 0.001f;        // 00D7A318 (float widened to double)
void hud_pause_screen_enter_00603120(void* screen, std::int32_t page_00e19660, PauseScreenHost& host);

// 00603CA0 HudPause update, slot +20h of 00CF3EF4, __thiscall(screen, dt), RET 4
// (dt unread). Hint page shown (+10h): scroller +9Ch update(1) and 006037D0.
// Otherwise both arrows are shown only when the award tracker's map +7Ch holds
// an entry whose string list (node+14h, count +1Ch) is non-empty (the image
// copy-constructs that list to read the count); a tracker with +84h == 0 hides
// them. Row 1 (tips) is visible when (game+38h is null or its +60h != 1) and
// tracker+9Ch != 0. Input 4Bh: hint page -> ShowPage(0, 0), else Resume and
// 00E19660 = 0. Input 1 on the main page: the same Resume. Otherwise the command
// bar: hint page (scroller update(1) first) {0,"",1} {0,"",1} {B3h navigate,1}
// {A3h back,2} {0,"",1}; main page {A2h select,0} {0,"",1} {B3h,1} {A3h,2}
// {0,"",1} and help line "". Then profile hints 0Fh.
void hud_pause_screen_update_00603ca0(void* screen, std::int32_t& page_00e19660, PauseScreenHost& host);

// 00604190 HudPause listbox activate, slot +04h of 00CF3EE0, __thiscall(listener
// = screen+8, row, unused), RET 8.
//   resume: 00E19660 = 0, Resume.
//   points: 00E19660 = 2, screen+4 = 0; the points screen [[00E198C4]+E0h] gets
//     +4 = +5 = 1, CommitVisibility, vtable +18h; input update 0.0.
//   options: 00E19660 = 3, screen+4 = 0; create the options menu 00E198B8 if
//     needed; 00E19650 = 0; its vtable +08h.
//   restart / quit / exit: prompts (slot 4, 4, 6; YesNo) with callbacks
//     005FD5B0 / 005FD610 / 005FC640, then FocusLastNavigation.
//   tips: 00E19660 = 1, +100h = 0.0f, 00601200 BuildHintList, +10h = 1, +14h
//     hidden, +18h shown, SelectDefault, list box +2Ch enabled, row 0, listener
//     screen+8, +80h(1), arrows hidden.
// Then, whenever the hint page is shown: +134h = list top [+2Ch]+164h, +138h =
// selected row; the hint name of row top+sel (when mapped) is toggled in the
// expanded list (removed when present, case-insensitive test; else appended),
// the hint list rebuilt, the top restored, RebuildPage, the row reselected,
// +80h(0).
inline constexpr const char* kPauseRestartPrompt =
    "globals.restartmission|.\n|FE_xbox.ingame_quitting|.\n|globals.areyousure";  // 00CF4158
inline constexpr const char* kPauseQuitPrompt =
    "globals.quit|.\n|FE_xbox.ingame_quitting|.\n|globals.areyousure";  // 00CF4114
inline constexpr const char* kPauseExitPrompt = "globals.exitprogram|.\n|globals.areyousure";  // 00CF40E8
void hud_pause_on_listbox_activate_00604190(void* listener, std::uint32_t row, std::int32_t& page_00e19660,
    PauseScreenHost& host);

// 00602280 HudPause register, __fastcall(ECX = screen), RET. Loads
// "GUI_pause_single" (+14h), clears the award tracker's new-hint history
// (00690CA0), sets 00E19660 = 0, loads "GUI_pause_tip" (+18h) and
// "GUI_pause_objectives" (+1Ch), and makes 26 child lookups (title, info and
// objectives groups; the .cpp lists them in image order). Between the bindings: +38h, +3Ch, +44h hidden; list box +2Ch gets
// 00A9AC90(0, 0, 0), byte +78h = 0, 00AA6BC0(0, 0); +84h = the height of the
// title slider's "back_Icon"; +88h..+90h = position of +80h; +24h, +7Ch, +74h,
// +78h get 00AA6BC0(screen+0Ch, 0); +104h..+10Ch / +110h..+114h = position /
// size of +40h; the embedded scroller +9Ch gets content +28h (00683790), +118h =
// the info slider's "back_Icon" height, then 00683380(+7Ch, (float)(that height
// - height of +7Ch), +74h, +78h); +10h = 0; list boxes +30h / +34h get
// 00A9AC90(0, 0, 0); finally +1Ch is hidden.
struct PauseRegisterHost : ScreenLifetimeHost {
    virtual void award_tracker_clear_new_hint_history_00690ca0() = 0;  // ECX = 004E1CA0()
    virtual void set_pause_page_00e19660(std::int32_t page) = 0;
    virtual std::uint32_t find_child_by_name_00aa7e00(std::uint32_t parent, std::string_view name) = 0;
    virtual void set_shown(std::uint32_t widget, bool shown) = 0;
    virtual void list_box_set_layout_flags_00a9ac90(std::uint32_t list_box, std::int32_t a, std::int32_t b,
        std::int32_t c) = 0;
    virtual void widget_set_listener_fields_00aa6bc0(std::uint32_t widget, void* listener, std::int32_t arg) = 0;
    virtual std::array<float, 2> widget_size_00aa6740(std::uint32_t widget) = 0;
    virtual void scroller_attach_content_00683790(void* scroller, std::uint32_t content) = 0;
    virtual void scroller_attach_scroll_bar_00683380(void* scroller, std::uint32_t thumb, float span,
        std::uint32_t up, std::uint32_t down) = 0;
};
// 006037D0 HudPause UpdateHints, __fastcall(ECX = screen), RET. Runs while the
// hint list box +2Ch has rows (+154h) and its current row's (00425E50)
// "felkialtojel_Text" (exclamation mark, 00CF3F84) is shown (vtable +38h).
// +134h = top (+164h), +138h = selected; for row = top + selected that is not
// a hint title (005FD220 find on +11Ch misses), the nearest preceding title is
// the map node with the largest key below row (ties take the later node). Its
// name indexes the award tracker's line lists (+94h, 006002B0, case-insensitive)
// and the (row - title)th line (1-based; "" when past the end) is marked read
// (00690FB0(name, line)). The selected row's exclamation is hidden; when the
// title row is on the page (title >= top) and the tracker's unread list for the
// name (+7Ch) is empty, the title row's (00A9BE00(title - top)) exclamation is
// hidden too.
struct PauseHintsHost : WidgetHost {
    virtual void* award_tracker_004e1ca0() = 0;
    virtual std::uint32_t list_box_current_row_00425e50(std::uint32_t list_box) = 0;
    virtual std::uint32_t list_box_row_00a9be00(std::uint32_t list_box, std::int32_t index) = 0;
    virtual std::int32_t list_box_selected_index_00a9c920(std::uint32_t list_box) = 0;
    virtual std::uint32_t find_child_by_name_00aa7e00(std::uint32_t parent, std::string_view name) = 0;
    virtual bool widget_virtual_38(std::uint32_t widget) = 0;
    virtual const NativeString* hint_name_at_row_005fd220(void* screen, std::int32_t row) = 0;
    virtual void* tree_next_005fce00(void* node) = 0;
    // 005FF5D0: std::map operator[] on +11Ch (inserts a missing key).
    virtual const NativeString& hint_map_subscript_005ff5d0(void* screen, std::int32_t key) = 0;
    // The (index)th line (1-based) of a copy of tracker+94h[name]; null past the end.
    virtual const NativeString* tracker_hint_line(const NativeString& name, std::int32_t index) = 0;
    // A null name or line stands for the image's empty local NativeString.
    virtual void tracker_mark_hint_read_00690fb0(const NativeString* name, const NativeString* line) = 0;
    virtual std::int32_t tracker_unread_count_7c(const NativeString* name) = 0;  // tracker+7Ch[name] +8h
};
inline constexpr std::string_view kHintExclamationText = "felkialtojel_Text";  // 00CF3F84
void hud_pause_update_hints_006037d0(void* screen, PauseHintsHost& host);

inline constexpr std::string_view kPauseSinglePage = "GUI_pause_single";          // 00CF40D4
inline constexpr std::string_view kPauseTipPage = "GUI_pause_tip";                // 00CF40C4
inline constexpr std::string_view kPauseObjectivesPage = "GUI_pause_objectives";  // 00CF402C
void hud_pause_register_00602280(void* screen, PauseRegisterHost& host);

// ---------------------------------------------------------------------------
// HudObjectivesScreen bodies (packet cc13-fe6)
// ---------------------------------------------------------------------------
//
// Screen offsets: +0Ch SetExpanded lock, +10h page, +14h "Open_Group", +18h
// "obj_main_Icon", +1Ch "obj_text_top_Icon", +20h "obj_scale_Icon", +24h
// "obj_text_bottom_Icon", +28h "SelectedObjective_Text", +2Ch / +30h
// "PrimaryNumbers_Text" / "SecondaryNumbers_Text", +34h
// "ObjectiveNumbers_Listbox", +38h "ObjectiveNumbersTemplate_Text", +3Ch
// "KisPocset_Icon", +44h..+48h row widgets (vector), +54h..+58h objectives
// (vector; objective +0Ch localised source, +1Ch state), +60h selected index,
// +64h any active, +65h rows current, +68h / +6Ch selected / requested
// objective, +70h expanded.
struct HudObjectivesHost : WidgetHost {
    virtual std::uint32_t find_child_by_name_00aa7e00(std::uint32_t parent, std::string_view name) = 0;
    virtual void list_box_set_input_filter_00a9ac70(std::uint32_t list_box, std::uint32_t routine) = 0;
    virtual void list_box_set_listener_00a9ac40(std::uint32_t list_box, void* listener) = 0;
    virtual void set_expanded_005ed5d0(void* screen, bool expanded) = 0;
    virtual void refresh_rows_005ee520(void* screen) = 0;
    virtual bool input_was_pressed_004c43c0(std::int32_t action) = 0;  // ECX = [00E188A8]
    // The vector 00E18D1C..00E18D20: its element count, 0 when unallocated.
    virtual std::uint32_t vector_00e18d1c_count() = 0;
    virtual void list_box_select_index_00a9c7c0(std::uint32_t list_box, std::uint32_t index) = 0;
    virtual std::uint32_t list_box_selected_index_00a9c920(std::uint32_t list_box) = 0;
    virtual std::uint32_t list_box_row_count_00a9ac50(std::uint32_t list_box) = 0;  // [00A9AC50()] + 8
    virtual void text_set_localised_source_00abaed0(std::uint32_t text, const void* source, std::int32_t arg) = 0;
    virtual void text_set_localised_cstring_00abbe50(std::uint32_t text, const char* key, std::int32_t arg) = 0;
    virtual std::array<float, 3> widget_resolved_position_00aa6750(std::uint32_t widget) = 0;
    virtual void widget_set_resolved_position_00aa8240(std::uint32_t widget, const std::array<float, 3>& position) = 0;
    virtual std::array<float, 2> widget_size_00aa6740(std::uint32_t widget) = 0;
    virtual void widget_set_size_58(std::uint32_t widget, const std::array<float, 2>& size) = 0;  // vtable +58h
    virtual void widget_set_colour_50(std::uint32_t widget, const std::array<float, 4>& rgba) = 0;  // vtable +50h
    // 006DBAC0: ECX = element, DL = enable (a register argument), stack (a, b,
    // value), RET 0Ch.
    virtual void apply_highlight_pulse_006dbac0(std::uint32_t element, bool enable, std::int32_t a, std::int32_t b,
        float value) = 0;
};

// 005EEA00 HudObjectives enter, slot +18h of 00CF2D20, __fastcall(ECX = screen),
// RET. Binds +14h "Open_Group" under +10h; +18h..+24h, +2Ch, +30h, +34h and
// +28h under +14h; +38h "ObjectiveNumbersTemplate_Text" and +3Ch
// "KisPocset_Icon" under +34h. The list box +34h gets +12Ch = 0.0, +130h =
// 1000.0f (00CE3804), +134h = 0.0, input filter 00696540, listener screen+8.
// Then +6Ch = +68h = 0, +70h = 0, SetExpanded(0), RefreshRows.
void hud_objectives_screen_enter_005eea00(void* screen, HudObjectivesHost& host);

// 005EEF60 HudObjectives update, slot +20h of 00CF2D20, __thiscall(screen, dt),
// RET 4 (dt unread).
//   A requested objective +6Ch: when it differs from +68h, +68h = it and +60h =
//     the last index of it in +54h, then the list box selects +60h; +6Ch = 0.
//     Without one: input 4Bh, or 00E18D1C empty, collapses (SetExpanded(0)).
//   Input C8h toggles SetExpanded(list box +85h clear); +65h clear: RefreshRows.
//   Expanded (+70h): with no active objective (+64h clear) the text +28h reads
//   "ingame.objgui_noactive" and +3Ch is hidden. Otherwise input C9h cycles the
//   selection ((sel + 1) % rows, unsigned); +60h = the selection; +28h shows the
//   objective's source (set twice, as the image does); +3Ch sits at the row
//   widget's position + (0.0, 0.0, -1.0) and is shown. Then, with P / (w, h)
//   the position / size of +1Ch and S = (float)((float)(lineheight / 720.0) *
//   (double)(uint)lines) of +28h (font [+108h] short +14h, lines +110h):
//     +20h at (P.x + 0.001f, P.y + h, P.z + 0.0), size (its width, S)
//     +24h at (P.x + w, P.y + (float)(2h + S), P.z + 0.0)
//     +28h at (P.x + 0.02f, P.y + h, P.z - 10.0)
//   Every row widget gets a colour by objective state: 1 (0, 0.45f, 0, 1),
//   2 (0.75f, 0, 0, 1), else the static white at 00E1961C (guard 00E1962C bit 0).
//   +14h shown = +70h; 006DBAC0 on +3Ch with DL = +70h, (0, 1, 1.0f).
inline constexpr float kObjectivesTextDrop = 10.0f;     // 00CE3DC0 (double)
inline constexpr double kObjectivesLineScale = 720.0;   // 00CEF1B8
inline constexpr float kObjectivesTextIndent = 0.02f;   // 00D7A2F8 (float widened)
inline constexpr float kObjectiveDoneGreen = 0.45f;     // 00CF2E4C
inline constexpr float kObjectiveFailedRed = 0.75f;     // 00CEE07C
inline constexpr const char* kNoActiveObjective = "ingame.objgui_noactive";  // 00CF2E50
void hud_objectives_screen_update_005eef60(void* screen, HudObjectivesHost& host);

// 005F9DD0 HudOrder update, slot +20h of 00CF3B7C, __thiscall(screen, dt), RET 4.
// Runs only while +82h is set and [[00E198C4]+5Ch]+4 is clear. The page +08h's
// colour C (vtable +54h) fades: step = dt, or with no order code (+88h == 0)
// +70h is raised to at least 1.0 and step = (float)(dt * 2.0) (D8 C9 FMUL).
// +70h = (float)(+70h + step); when it exceeds 0.5f, C.a = (float)(C.a -
// 2.0 * step). Eight 004BEC00 calls whose results are discarded follow. C.a <=
// 0 closes the screen (005F9520) and C.a = 1.0. C is written back through
// 00AA85B0. The pulse +7Ch moves by step * 7.0 (up while +80h is set, else
// down); above 0.99f +80h = 0, below 0.01f +80h = 1; without an order code +7Ch
// = 0.5f and +80h = 1. The five widgets +44h..+54h get +44h's colour (vtable
// +54h) with alpha +7Ch through vtable +50h.
struct HudOrderUpdateHost {
    virtual ~HudOrderUpdateHost() = default;
    virtual void* hud_interface_00e198c4() = 0;
    virtual std::array<float, 4> widget_colour_54(std::uint32_t widget) = 0;  // vtable +54h(out)
    virtual void widget_set_colour_50(std::uint32_t widget, const std::array<float, 4>& rgba) = 0;
    virtual void widget_set_colour_00aa85b0(std::uint32_t widget, const std::array<float, 4>& rgba) = 0;
    virtual void input_manager_singleton_004bec00() = 0;
    virtual void hud_order_close_005f9520(void* screen) = 0;
};
inline constexpr double kOrderFadeRate = 2.0;    // 00D7A308
inline constexpr double kOrderPulseRate = 7.0;   // 00CED5D8
inline constexpr float kOrderPulseHigh = 0.99f;  // 00CED5D0 (float widened)
inline constexpr float kOrderPulseLow = 0.01f;   // 00D7A238
void hud_order_screen_update_005f9dd0(void* screen, float dt, HudOrderUpdateHost& host);

// 005EA2D0 MultiMainMenu update, slot +20h of 00CF2708, __thiscall(screen, dt),
// RET 4. Inputs through 004D92B0 (edge or repeat, ECX = [00E188A8]):
//   4Bh: 005E7900 refresh online state, nothing else.
//   list +20h byte +77h clear: 4Dh sets it. Else list +24h +77h clear: 4Dh sets
//   it. Else (both set): 4Ch steps column +E0h down, or at column 0 clears +77h
//   of +24h when its vtable +38h is true (else of +20h), sets +E8h's uniform
//   scale 1.0f (004FB9E0) and hides +D4h; 4Dh steps +E0h up to 2; 46h / 47h
//   step row +E4h within [0, +10h - 1].
//   Every accepted step: 005E9BE0 then [004C1B90()]+19h = 1.
// Scroll velocity v = +F0h: v > 1e-5f scrolls the embedded scroller +6Ch toward
// the end by (float)(2 * v * dt), v < -1e-5f toward the start by (float)(2 *
// -v * dt); in both cases +F0h = (float)(v - 2 * (dt * v)). Scroller
// update(1). Command bar: {A2h select, 0} when either list's +77h is clear, else
// {0, "", 1}; then {B3h navigate,1} {0,"",1} {A3h back,2} {0,"",1}.
struct MultiMainUpdateHost {
    virtual ~MultiMainUpdateHost() = default;
    virtual bool action_edge_or_repeat_004d92b0(std::int32_t action) = 0;
    virtual void refresh_online_state_005e7900(void* screen) = 0;
    virtual void update_session_list_005e9be0(void* screen) = 0;
    virtual void sound_queue_set_flag_19() = 0;  // [004C1B90() + 19h] = 1
    virtual bool widget_virtual_38(std::uint32_t widget) = 0;
    virtual void widget_set_uniform_scale_004fb9e0(std::uint32_t widget, float scale) = 0;
    virtual void set_shown(std::uint32_t widget, bool shown) = 0;
    virtual void scroller_toward_end_00683700(void* scroller, float delta) = 0;
    virtual void scroller_toward_start_00683680(void* scroller, float delta) = 0;
    virtual void scroller_update_00683820(void* scroller, std::int32_t arg) = 0;
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
};
inline constexpr float kMultiScrollDeadZone = 1e-5f;  // 00D7A310 / 00CF29AC (0x3727C5AC)
void multi_main_screen_update_005ea2d0(void* screen, float dt, MultiMainUpdateHost& host);

// 005E2320 LobbySettings option label, __fastcall(ECX = NativeString* result,
// EDX = slot) -> result. Returned here as a std::string projection; the native
// result is an 8-byte NativeString. The mission record is the [00E19594]th node
// (004BE940) of the list at game+5F0h (count at +0), valid when the index is
// below the count, the node exists and its byte +914h is set.
//   slot 1: the record's name +920h, else "FE.unknown_mapname" (00CEF1D8).
//   slot 0: game byte +218Ch clear: "<(char)game+2018h>/<(char)game+2017h>"
//     ("/" 00CE7898). Set: effective game mode (004BCA50) <= 3 gives "2";
//     otherwise the count of the eight player slots game+18CCh..+18E8h with
//     ((+8 set and +9 clear) or +0Ah set).
//   slot 0Dh: a copy of the NativeString 00E195BC.
//   other slots, valid record: when the option-enabled byte for (slot,
//     00E1955C[slot]) is clear, 005E21B0(ECX = slot) steps it. Then, if the
//     registry's label map (00576B10()+10h) has a label at (slot, 0), the label
//     at (slot, 00E1955C[slot]); else the integer (registry+1Ch) at that key.
//     Invalid record: "-" (00CEED40).
// 004260B0 / 004263B0 format a signed decimal; std::to_string stands in.
struct LobbyLabelHost {
    virtual ~LobbyLabelHost() = default;
    virtual void* game_00e188a8() = 0;
    virtual std::int32_t mission_index_00e19594() = 0;
    virtual const void* list_nth_node_004be940(const void* list, std::int32_t index) = 0;
    virtual std::int32_t stored_option_00e1955c(std::int32_t slot) = 0;
    // 005E1990 + 005C6850 + 005C49C0: the option-enabled byte at (slot, option).
    virtual bool option_enabled(std::int32_t slot, std::int32_t option) = 0;
    virtual void step_option_005e21b0(std::int32_t slot) = 0;
    virtual const char* option_label_00575e80(std::int32_t slot, std::int32_t option) = 0;  // registry +10h
    virtual std::int32_t option_value_00575fd0(std::int32_t slot, std::int32_t option) = 0;  // registry +1Ch
    virtual std::uint32_t effective_game_mode_004bca50() = 0;
    virtual std::string string_00e195bc() = 0;
    virtual std::string native_string(const void* native) = 0;  // NativeString -> text
};
std::string lobby_settings_option_label_005e2320(std::int32_t slot, LobbyLabelHost& host);

// 005F54F0 OptionsScreen BuildMainList, __fastcall(ECX = screen), RET. +252h = 0;
// command-bar help variant (00549620 on [00E1930C]) = game byte +61Fh. A pending
// +78h is cleared and only pushes interface 0Eh (004CC460 on [00E198B8], (0Eh,
// 0)). Otherwise: copy the live settings 00F88980 into the edit copy +D8h
// (005F52C0); 00E19650 = 0; 00527CB0 on [[00E198B8]+44h] fills the bytes
// +119h, +11Ah, +11Bh, +118h; list box +84h loses its listener and rows; title
// "FE.opt_title_main" (in a mission 00518D60(0, 0, 0, title), else 005189B0).
// Rows 0..6 (row 6 skipped in a mission): a clone of +8Ch named
// "Elem_<n>_Text" (vtable +2Ch), data +D8h = n, inserted, shown, text
// 00E08A48[n]; +7Ch(1) on row 5 when [00E198AC] and its +74h are set; row 6
// +7Ch([00F8A304] null or its +8h == 0); row 4 +7Ch(+188h == 0). Then
// highlight reset (1, 1), 005F0CD0, listener screen+8, select +A8h, +80h(1),
// hide +A4h, +A0h, +88h, +254h, +258h, and 00A9CDF0(ECX = +84h, EDX = +84h,
// stack +88h): its ECX is the kept list, EDX and the stack word the two lists to
// clear, each skipped when equal to ECX.
struct OptionsMainListHost : WidgetHost {
    virtual void* game_00e188a8() = 0;
    virtual void command_bar_set_help_variant_00549620(std::uint8_t variant) = 0;
    virtual void push_interface_request_004cc460(std::int32_t interface_id, std::int32_t arg) = 0;
    virtual void copy_settings_005f52c0(void* edit_copy) = 0;  // source 00F88980
    virtual void set_options_page_00e19650(std::int32_t page) = 0;
    // 00527CB0 on [[00E198B8] + 44h], RET 10h.
    virtual void control_layout_button_flags_00527cb0(std::uint8_t* a, std::uint8_t* b, std::uint8_t* c,
        std::uint8_t* d) = 0;
    virtual void list_box_set_listener_00a9ac40(std::uint32_t list_box, void* listener) = 0;
    virtual void list_box_clear_rows_00a9bec0(std::uint32_t list_box) = 0;
    virtual void frame_select_00518d60(const char* name) = 0;  // ECX = EDX = 0, stack 0
    virtual void frame_set_title_005189b0(const char* title) = 0;
    virtual std::uint32_t clone_subtree_00aab4c0(std::uint32_t widget, std::int32_t arg) = 0;
    virtual void widget_set_name_2c(std::uint32_t widget, const std::string& name) = 0;  // vtable +2Ch
    virtual void list_box_insert_row_00a9d750(std::uint32_t list_box, std::uint32_t row, std::int32_t a,
        std::int32_t b) = 0;
    virtual void text_set_localised_cstring_00abbe50(std::uint32_t text, const char* key, std::int32_t arg) = 0;
    virtual void widget_virtual_7c(std::uint32_t widget, bool value) = 0;
    virtual void* briefing_screen_00e198ac() = 0;
    virtual void* downloads_00f8a304() = 0;
    virtual void list_box_reset_highlight_00a9cf30(std::uint32_t list_box, std::int32_t a, std::int32_t b) = 0;
    virtual void refresh_help_line_005f0cd0(void* screen) = 0;
    virtual void list_box_select_index_00a9c7c0(std::uint32_t list_box, std::int32_t index) = 0;
    virtual void list_box_virtual_80(std::uint32_t list_box, bool value) = 0;
    virtual void list_box_clear_other_two_00a9cdf0(std::uint32_t keep, std::uint32_t edx_list,
        std::uint32_t stack_list) = 0;
};
void options_build_main_list_005f54f0(void* screen, OptionsMainListHost& host);

// 005F7C40 OptionsScreen ActivateListRow, slot +04h of 00CF3960, __thiscall(
// listener = screen+8, unused, row), RET 8. The whole routine; the page-0 arm
// alone is also modelled by bsp::options_activate_main_row_005f7c40
// (options_menu_screens.hpp). Screen offsets: +84h list box, +A8h main-list
// data, +ACh top row, +D8h edit settings, +118h..+11Bh layout bytes, +150h /
// +F6h video fields compared with 00F889F8 / 00F8899E, +250h defaults flag,
// +252h DLC-changed flag, +25Ch in-mission copy, +260h transition timer.
//   Ignored unless +260h == 0.0f exactly (UCOMISS, PF/ZF test).
//   Page 0 (00E19650): +A8h = the selected data, +25Ch = game byte +61Fh; data
//     0/1/2/6 build the game / audio / video / content page (005F58F0, 005F59F0,
//     005F41E0, 005F3B50); 3 pushes interface 0Eh; 4 sends the layout bytes
//     (+119h, +11Ah, +11Bh, +118h) to 00527C80 on [[00E198B8]+44h] and pushes
//     0Dh; 5 needs 00585B40 and 00585810(CL = 0), then +25Ch = 0 and 005EFAB0.
//     Afterwards, with +25Ch set, 00518D60(ECX = EDX = -1, 0, "").
//   Row == the list box: index = selected + top. Page 1: 9 apply, 10 game
//     defaults, 11 back. Page 4: 6 apply, 7 control defaults. Page 2: 4 apply, 5
//     audio defaults, 6 back. Page 3: 0Dh video apply, 0Eh video defaults (two
//     zero arguments), 0Fh back. Defaults: +250h = 1, reset on +D8h, 008D5B50.
//     Back: 005F5E60.
//   Any other row: pages 1, 2, 4 apply; page 3 video apply with
//     XLiveOnResetDevice([00F8D394]+1A28h) when [00E198C4] is null; page 7 sets
//     +252h then applies.
//   Apply: with neither 00F88A30 nor +252h set, 005F65C0. Otherwise prompt
//     slot 0 kind 1 callback 005F6FB0 "FE_xbox.xsm_dlcchanged" when +252h is
//     set, else "FE_xbox.xsm_saveconfirm" when 00A3E510 on [00F8ABE8] reports
//     a selected user, else 005F65C0. Every apply ends with +260h = 0.1f.
//   Video apply: changed = (00F889F8 != +150h) || (00F8899E != byte +F6h);
//     008D5B50 on +D8h; then 005F7050(changed).
struct OptionsActivateHost : WidgetHost {
    virtual void* game_00e188a8() = 0;
    virtual void* hud_interface_00e198c4() = 0;
    virtual std::int32_t options_page_00e19650() = 0;
    virtual std::int32_t list_box_selected_data_00a9c990(std::uint32_t list_box) = 0;
    virtual std::int32_t list_box_selected_index_00a9c920(std::uint32_t list_box) = 0;
    virtual void build_page(std::uint32_t routine, void* screen) = 0;  // 005F58F0 / 005F59F0 / 005F41E0 / 005F3B50
    virtual void push_interface_request_004cc460(std::int32_t interface_id, std::int32_t arg) = 0;
    virtual void control_layout_set_payload_00527c80(std::uint8_t a, std::uint8_t b, std::uint8_t c,
        std::uint8_t d) = 0;
    virtual bool main_menu_connection_gate_00585b40() = 0;
    virtual bool main_menu_account_gate_00585810(std::uint8_t cl) = 0;
    virtual void options_reset_005efab0(void* screen) = 0;
    virtual void frame_select_00518d60(std::int32_t ecx, std::int32_t edx, std::int32_t arg, const char* name) = 0;
    virtual void settings_reset_defaults(std::uint32_t routine, void* settings) = 0;  // 008D41C0 / 008D4820 / 008D41F0
    virtual void settings_reset_video_defaults_008d4520(void* settings, std::int32_t a, std::int32_t b) = 0;
    virtual void settings_apply_all_008d5b50(void* settings) = 0;
    virtual void return_to_list_005f5e60(void* screen) = 0;
    virtual void apply_and_rebuild_005f65c0(void* screen) = 0;
    virtual void confirm_changes_005f7050(void* screen, bool changed) = 0;
    virtual void xlive_on_reset_device_00a4d46a() = 0;  // argument [00F8D394] + 1A28h
    virtual bool settings_dirty_00f88a30() = 0;
    virtual std::int32_t live_video_00f889f8() = 0;
    virtual std::uint8_t live_video_00f8899e() = 0;
    virtual bool xenon_has_selected_user_00a3e510() = 0;  // ECX = [00F8ABE8]
    // 00531B00 on 00425D10() with (slot, message, kind, callback, 0, title "",
    // timeout 0.0f, timeout result 0, countdown "", dismissible 1), RET 2Ch.
    virtual void raise_prompt_00531b00(std::int32_t slot, const char* message, std::int32_t kind,
        std::uint32_t callback) = 0;
};
inline constexpr const char* kOptionsDlcChangedPrompt = "FE_xbox.xsm_dlcchanged";  // 00CF3B28
inline constexpr const char* kOptionsSaveConfirmPrompt = "FE_xbox.xsm_saveconfirm";
inline constexpr float kOptionsTransitionTime = 0.1f;  // 00D7A2F0
void options_activate_list_row_005f7c40(void* listener, std::uint32_t row, OptionsActivateHost& host);

// 005E4490 MultiGameLobby HandleGuiEvent, slot of 00CF23B4, __thiscall(this =
// lobby+0Ch, event widget), RET 4. Offsets below are this-relative (lobby =
// this-0Ch); +10h focused widget, +14h / +1Ch / +80h / +98h focusable panels,
// +24h / +C0h / +14Ch embedded scrollers, +9Ch "Players_Listbox", +144h chat
// panel. Player slot records are game+18CCh[i] (+9 AI, +0Ah ready, +2Ch AI
// skill, +80h/+84h XUID); the session object is game+1EF0h.
//   Event kind 2 (vtable +5Ch): ignored when +0Ch's vtable +38h is true and
//     [+21Ch]+85h is set; else +9Ch selects the event's row data (+D8h), the chat
//     sub-object ([[00E198B4]+54h]+8, vtable +0Ch) is notified when +10h is the
//     chat panel, and the lobby focuses +98h (005D67E0).
//   Kind 3: event vtable +4Ch(1.0f); the command byte is [event+F0h] or 00E19608:
//     A2h: settings open -> 005DA530; else by focus: +14h 005D9F90; +80h input
//       update 0.0 and 005D5870(1); +1Ch: [[00E198AC]+70h] +98h = the local slot
//       record's +28h, +94h = 8, request interface 0Bh (00688AD0 on [00E198B4]);
//       +98h: the listener slot +04h of lobby+8 with 00A9BE00(+9Ch, selected).
//     B7h: 005D9660(lobby, 0, 1).
//     B8h: unless [00F8A2FC] vtable +198h and its +4Dh are both set: rebind +9Ch,
//       then for the selected slot: empty / not ready and not standard mode
//       (005D4A10) -> RequestSlotAiEnable(sel, 1) and skill 3; else skill by the
//       AI kind +2Ch (1 or 3 -> 4, 4 -> 5); other kinds: unless standard mode,
//       RequestSlotAiEnable(sel, 0).
//     A3h: 005DA210 confirm leave.
//     A7h: focus +98h and host (game+1FE4h == 1): an AI slot toggles
//       RequestSlotAiEnable(sel, !ready); a human slot other than the local one
//       (005D5130) asks to kick (005D8530). Focus +80h: 0051DCE0 on
//       [[00E198AC]+64h] with 005D8650(local slot +28h), then interface 3.
//     A5h: list box +9Ch active, a human slot: XShowGamerCardUI(
//       00A3EAC0 on [00F8ABE8], +80h, +84h).
//     B1h: 005D71B0.
//   Other kinds, by widget: +98h, +14h, +80h, +1Ch: unless settings are open,
//     notify chat when +10h is the chat panel and focus it. +144h: unless
//     settings are open, with [00E198B4] and its chat +5 set and game +218Ch
//     clear: focus, 005D11F0(2) chat mode, input context 1Eh level 0. +90h / +8Ch
//     (focus +80h) and +130h / +12Ch, +B0h / +ACh: auto-scroll (00683300) of
//     +24h, +14Ch, +C0h by 2 / 1 at 0.004f; +84h (focus +80h), +148h, +BCh:
//     thumb drag (00683A40) of +24h, +14Ch, +C0h; +214h / +218h step the
//     setting by -1 / +1 (005E1CF0); +138h / +13Ch / +140h set team-only 0 / 1
//     (005D1000) and restyle the three (+88h(2 or 0, 0, 1.0f)), then return;
//     anything else returns while settings are open.
//   Tail (scroll, drag, step and unmatched widgets): kind 6 -> event
//     +88h(2, 0, 1.0f); kind 12h -> event +84h(2).
struct LobbyEventHost {
    virtual ~LobbyEventHost() = default;
    virtual void* game_00e188a8() = 0;
    virtual std::int32_t widget_kind_5c(std::uint32_t widget) = 0;
    virtual bool widget_virtual_38(std::uint32_t widget) = 0;
    virtual void widget_virtual_4c(std::uint32_t widget, float value) = 0;
    virtual void widget_virtual_84(std::uint32_t widget, std::int32_t value) = 0;
    virtual void widget_virtual_88(std::uint32_t widget, std::int32_t a, std::int32_t b, float c) = 0;
    virtual std::uint8_t default_command_00e19608() = 0;
    virtual void list_box_select_index_00a9c7c0(std::uint32_t list_box, std::int32_t index) = 0;
    virtual std::int32_t list_box_selected_index_00a9c920(std::uint32_t list_box) = 0;
    virtual std::uint32_t list_box_row_00a9be00(std::uint32_t list_box, std::int32_t index) = 0;
    virtual std::uint32_t find_child_by_name_00aa7e00(std::uint32_t parent, std::string_view name) = 0;
    virtual void* multi_menu_00e198b4() = 0;
    virtual void chat_notify_focus() = 0;  // [[00E198B4]+54h]+8, vtable +0Ch
    virtual void lobby_set_focus_005d67e0(void* lobby, std::uint32_t widget) = 0;
    virtual bool lobby_settings_open_005d5490(void* lobby) = 0;
    virtual void lobby_apply_selected_map_005da530(void* lobby) = 0;
    virtual void lobby_apply_local_map_005d9f90(void* lobby) = 0;
    virtual void input_manager_update_00a92c40(float dt) = 0;
    virtual void lobby_set_editable_005d5870(void* lobby, bool editable) = 0;
    virtual void* briefing_screen_00e198ac() = 0;
    virtual void push_request_interface_00688ad0(std::int32_t interface_id) = 0;
    virtual void lobby_listener_activate_04(void* listener, std::uint32_t row) = 0;  // [lobby+8] vtable +04h
    virtual void lobby_refresh_players_005d9660(void* lobby, std::int32_t a, std::int32_t b) = 0;
    virtual bool online_virtual_198_00f8a2fc() = 0;
    virtual std::uint8_t online_byte_4d_00f8a2fc() = 0;
    virtual bool game_is_standard_mode_005d4a10(void* lobby) = 0;
    virtual void session_request_slot_ai_enable_00771e80(std::int32_t slot, bool enable) = 0;  // ECX = game+1EF0h
    virtual void session_set_slot_ai_skill_0076cc40(std::int32_t slot, std::int32_t level) = 0;  // ECX = game+1EF0h
    virtual void lobby_confirm_leave_005da210(void* lobby) = 0;
    virtual const void* game_local_slot_record_005d5130() = 0;
    virtual void lobby_confirm_kick_005d8530() = 0;
    virtual const void* mission_record_005d8650(std::int32_t id) = 0;
    virtual void briefing_set_mission_data_0051dce0(void* briefing_64, const void* record) = 0;
    virtual void show_gamer_card(std::uint32_t xuid_lo, std::uint32_t xuid_hi) = 0;  // 00A3EAC0 + XShowGamerCardUI
    virtual void lobby_check_client_compatibility_005d71b0(void* lobby) = 0;
    virtual void chat_set_mode_005d11f0(std::int32_t mode) = 0;      // ECX = [[00E198B4]+54h]
    virtual void chat_set_team_only_005d1000(bool team_only) = 0;    // ECX = [[00E198B4]+54h]
    virtual void input_set_context_level_00a933f0(std::int32_t index, std::int32_t level) = 0;
    virtual void scroller_set_auto_scroll_00683300(void* scroller, std::int32_t direction, float speed,
        std::int32_t arg) = 0;
    virtual void scroller_begin_thumb_drag_00683a40(void* scroller) = 0;
    virtual void lobby_step_setting_005e1cf0(void* lobby, std::int32_t step) = 0;
};
inline constexpr float kLobbyAutoScrollSpeed = 0.004f;  // 00CEC178
void lobby_handle_gui_event_005e4490(void* self, std::uint32_t event, LobbyEventHost& host);
// ---------------------------------------------------------------------------
// Packet cc13-fe9 (docs/CC13_FE_9.md)
// ---------------------------------------------------------------------------
struct Fe9NarrativeRegisterHost {
    virtual ~Fe9NarrativeRegisterHost() = default;
    virtual void frontend_screen_register_004f71d0(void* screen) = 0;
    virtual void* gui_load_page_00aa5840(std::string_view name, std::uint32_t a, std::uint32_t b) = 0;  // ECX = 004C12B0()
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;  // (name, 1)
    virtual void voice_slot_stop_007026f0(void* slot) = 0;
    // Widget vtable +54h(out): a pointer to the text's 16-byte colour.
    virtual const std::array<float, 4>* text_colour_54(void* text) = 0;
    // The message-box vector +90h (34h-byte elements): erase(begin, end) (005B8E10), then resize(1,
    // a local element) (005BA6D0); returns the last element.
    virtual void* message_boxes_reset_to_one(void* vector) = 0;
    // A fresh Lua owner (00B66BD0), Open(libs), RunScriptWithOverrides(path, flag); globals reads.
    virtual void* lua_owner_open_run(std::uint32_t libs, std::string_view script, std::uint32_t flag) = 0;
    virtual double lua_global_number(void* owner, const char* name) = 0;  // GetGlobals, GetByName, GetNumber
    virtual void lua_owner_close_00b669a0(void* owner) = 0;
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;
};

// 005BB130 BSP_HudNarrativeScreen_Register, __fastcall(ECX = screen), RET. 004F71D0; page
// "GUI_blackout" (00AA5840(name, 1, 0)) -> +20h; its "Warnings_Group" -> "MessageBoxTemplate_Group"
// -> +2Ch; voice slot +8h stopped (007026F0), +60h = 0; page "GUI_subtitle" -> +28h,
// "textbox_main_Group" -> +30h with "textbox_main_Text" +34h and +38h (the same child twice),
// "textbox_main_FrameBox" +3Ch, "textbox_main_Icon" +40h; +44h..+50h = the colour from +34h vtable
// +54h. The message-box vector +90h is reset to one element: +0h = +30h, +4h frame box, +8h text,
// +0Ch = 0.0f, +10h..+1Ch = the colour, byte +20h = 1, +24h = +28h = 0, +2Ch = 0.0f, +30h = -1. A
// Lua owner (Open(1)) runs "Scripts/datatables/DialogGlobals.lua" (flag 0): +78h = DialogFadeTime,
// +88h = 0.0f, +84h = -1, +7Ch = TempCharWait, +80h = TempWaitBase (each FSTP to a float), +8Ch =
// 0.0f, +70h = -1, +74h = 0. +20h "Blackout_Icon" -> +0BCh; page "GUI_narrative" -> +24h,
// "MissionInfoTitle_Group" -> +0ACh with "textbox_obj_Text" +0B0h and "textbox_obj_FrameBox" +0B4h;
// +0D8h = 1.0f, +0D4h = 0.0f, +0DCh = 8.0f (00CE3918), +0C8h = +0C4h = +0C0h = 0.0f; +0BCh hidden;
// byte +4h = 1; the owner closed.
void hud_narrative_register_005bb130(void* screen, Fe9NarrativeRegisterHost& host);

// Award-tracker hint texts (00601200): 005FFB20 on [tracker]+30h looks a hint up; the record copy
// (005FE010) holds a default text +0h and per-language texts +20h (language 0) and +38h (language 1).
struct HintTexts {
    std::string fallback;   // +00h
    std::string language0;  // +20h
    std::string language1;  // +38h
};

struct Fe9HintListHost {
    virtual ~Fe9HintListHost() = default;
    virtual void list_box_clear_items_00a9bf90(void* list) = 0;
    virtual void index_tree_erase_005fe9e0(void* tree, void* root) = 0;                    // ECX = screen+11Ch
    virtual void* award_tracker_004e1ca0() = 0;
    virtual void* tree_iterator_next_004b7de0(void* node) = 0;
    virtual void* widget_clone_subtree_00aab4c0(void* widget, void* parent) = 0;
    virtual void* find_child_by_name_00aa7e00(void* parent, std::string_view name) = 0;     // (name, 1)
    virtual void widget_set_visible_34(void* widget, std::uint32_t visible) = 0;
    virtual std::array<float, 2> widget_size_00aa6740(void* widget) = 0;
    virtual void widget_set_size_58(void* widget, std::array<float, 2> size) = 0;           // vtable +58h
    virtual void widget_set_state_88(void* widget, std::uint32_t state, std::uint32_t zero, float one) = 0;
    virtual void gui_text_set_ellipsized_00abb000(void* text, std::string_view source, float width,
        std::uint32_t flag) = 0;
    virtual void list_box_insert_row_00a9d750(void* list, void* row, std::uint32_t a, std::uint32_t b) = 0;
    // 005FF5D0, ECX = screen+11Ch: map<int, NativeString>::operator[](index); the slot is assigned.
    virtual void index_map_assign_005ff5d0(void* map, std::int32_t index, std::string_view text) = 0;
    // The tracker's new-hint map +7Ch (case-insensitive): 005FD120 lower bound + compare; the
    // 006002B0 subscript's +8h count; 005FD4B0 find, its node +14h string list copied (004D26A0).
    virtual bool new_hints_contains_005fd120(std::string_view key) = 0;
    virtual std::uint32_t new_hints_count_006002b0(std::string_view key) = 0;
    virtual std::vector<std::string> new_hints_list_005fd4b0(std::string_view key) = 0;
    // 004D48A0 copy of a history node's +14h string list.
    virtual std::vector<std::string> history_sub_hints_004d48a0(void* node) = 0;
    virtual HintTexts hint_texts_005ffb20(std::string_view key) = 0;
    // [[00E188A8] + 18CCh + [00E188A8]+18ECh * 4] + 28h.
    virtual std::int32_t language_index() = 0;
    virtual void list_box_enable_paging_00a9e230(void* list, std::int32_t page) = 0;
};

// 00601200 BSP_HudPauseScreen_BuildHintList, __fastcall(ECX = pause screen), RET. Clears the list
// +2Ch (00A9BF90) and the index map +11Ch (erase from the root, head links back to the head, size
// 0). For each hint of the award tracker's history (map at tracker +94h, head +98h; key NativeString
// at node +0Ch, sub-hint list at node +14h), with a running row index from 0: a template +38h row
// with "hint_Text" (+30h = -0.0302083f (00CF3F80), +34h = 0.0f, byte +1F0h = 0) sized ((float)(w -
// 1/48.0) (00CF0000), h), then ellipsized at -1.0f to the key without the six characters before
// position 6 when it contains "AAAAAA" anywhere (substr(6, length - 6)), then sized (0.1979167f
// (00CF3F7C), h); "felkialtojel_Text" hidden; appended; index map[index] = the key; index + 1. When
// the tracker's new-hint map +7Ch holds the key with a non-zero count, "felkialtojel_Text" is shown and
// the new-hint list copied. "button_Icon" takes state 0 (vtable +88h(0, 0, 1.0f)), or state 1 when
// the key is in the expanded list +128h (case-insensitive; NativeString at node +8h); expanded hints
// add one template +3Ch row per sub-hint: "hint_Text" sized ((float)(w - 1/60.0) (00CF3F68), h), byte
// +0FCh = 0, text = the language's text (language 0 -> +20h, 1 -> +38h, when non-empty), else the
// record's default (+0h) when non-empty, else the sub-hint itself, ellipsized at -1.0f, +30h =
// -0.009375f (00CF3F64), +34h = 0.0f, +1F0h = 0, sized (0.1854167f (00CF3F60), h); "felkialtojel_Text"
// hidden, then shown when the sub-hint is in the new-hint list; the index advances without a map
// entry. Finally paging 10h (00A9E230); the scroll thumb +80h gets (its width, +84h) for up to 16
// rows, else (its width, (float)(16.0 / rows * +84h)) through vtable +58h, stored at +94h.
void hud_pause_build_hint_list_00601200(void* screen, Fe9HintListHost& host);
// ---------------------------------------------------------------------------
// Packet cc13-fe11 (docs/CC13_FE_11.md): 005AB190 BSP_HudMapScreen_Register, range by range.
// Field names follow bsp::MapScreenZoomState (include/bsp/map_screen_update_cc13.hpp).
// ---------------------------------------------------------------------------
struct Fe11HudMapRegisterHost {
    virtual ~Fe11HudMapRegisterHost() = default;
    virtual void frontend_screen_register_004f71d0(void* screen) = 0;
    virtual void simple_3d_scene_init_0093ccc0(void* scene) = 0;                         // ECX = screen+8h
    // World bounds of the game object [00E188A8]: +711Ch, +7124h, +7128h, +7130h.
    virtual float world_bound(std::uint32_t offset) = 0;
    // The image's per-level tables: 00E0CF40 extents (.data, rewritten here), 00E0CF50 crossfade,
    // 00F89AE4 scales (BSS). Each points at four floats.
    virtual float* map_zoom_extents_00e0cf40() = 0;
    virtual const float* map_zoom_crossfade_00e0cf50() = 0;
    virtual float* map_zoom_scales_00f89ae4() = 0;
    // 00A8FD30, ECX = light, DL = 0 -> a referenced shadow owner; 00B7BDF0 sets it.
    virtual void* directional_light_create_shadow_owner_00a8fd30(void* light) = 0;
    virtual void directional_light_set_shadow_owner_00b7bdf0(void* light, void* owner) = 0;
    virtual void ref_release(void* object) = 0;     // InterlockedDecrement(+4h) ([00CE2220]), vtable +0h at zero
    virtual void* gui_load_page_00aa5840(std::string_view name, std::uint32_t a, std::uint32_t b) = 0;  // ECX = 004C12B0()
};

// 005AB190..005AB518, the set-up range of BSP_HudMapScreen_Register (__fastcall(ECX = screen), RET).
// 004F71D0; 0093CCC0 on +8h; +84h = 1.0f, byte +88h = 0. X = (float)((float)(+7124h - +7130h) / 0.75 *
// 1.1) and Y = (float)((float)(+7128h - +711Ch) * 1.1) (00CEC9D8 / 00CE3DF0); m = X when X > Y, else
// Y. 8000.0 >= m (00CF0AA0): level 2, ext[2] = m. 25000.0f >= m (00CF0A98): level 1, ext[1] = m,
// ext[2] = (float)(m * 0.4) (00CE65D0). Otherwise level 0, ext[0] = m, ext[1] = (float)(0.375 * m),
// ext[2] = (float)(m * 0.166) (00CF0A90 / 00CF0A88). The level goes to +2Ch and +5FCh; extents not
// named keep their .data values. scales[i] = (float)(crossfade[i] * ext[i] / ext[3]) for i < 3 and
// scales[3] = (float)(crossfade[3] * ext[3] / ext[3]). +1Ch = (float)(level + 1.0); +30h = ext[level],
// +34h = (float)(0.75 * ext[level]). The light +18h gets a fresh shadow owner (released after the
// set). Pages "GUI_map" -> +8Ch, "GUI_classicons" -> +90h ((name, 1, 0)). The pan centre ((+7128h +
// +711Ch) * 0.5, (+7130h + +7124h) * 0.5), each a float, goes to +54Ch/+550h, +554h/+558h and
// +5E8h/+5ECh; +5ACh = 0, +40h = +41h = 0, +5B0h = +30h, +5B4h = +34h, +5D0h = 0, +5D4h = 1.0f.
void hud_map_register_setup_005ab190(void* screen, Fe11HudMapRegisterHost& host);

}  // namespace bsp::cc13_fe1