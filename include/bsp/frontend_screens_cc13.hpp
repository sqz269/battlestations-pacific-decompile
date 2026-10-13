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
#include <string_view>

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

}  // namespace bsp::cc13_fe1
