#pragma once

#include <cstdint>

// Game rows the cc15-stlA audit moved from stl_probable to game_code (packet
// cc15-g24b). New C++ interfaces; names are hypotheses, not recovered symbols.
// docs/CC15_G24B.md records the original ABIs, evidence and limits. Every
// field access and call goes through the host; addresses are native Win32
// object addresses. No host is wired to any of these.
//
// The VC9 checked-iterator asserts call 00BF6713 (_invalid_parameter_noinfo)
// and fall through in this image; each one is a host call at the same place
// and execution continues as the binary does.
namespace bsp::cc15_g24b {

using Address = std::uintptr_t;

// A VC9 checked tree/list iterator as the image keeps it on the stack:
// +0 container proxy (the container address), +4 node.
struct TreeIterator {
    Address container;
    Address node;
};

struct MemoryHost {
    virtual ~MemoryHost() = default;
    virtual std::uint32_t read_u32(Address address) = 0;
    virtual void write_u32(Address address, std::uint32_t value) = 0;
    // 00BF6713 _invalid_parameter_noinfo (returns in this image).
    virtual void invalid_parameter_00bf6713() = 0;
};

// ---------------------------------------------------------------------------
// 004881F0 BSP_Manager21D0_DeleteKind1And2Objects
// ---------------------------------------------------------------------------
// __fastcall(ECX = manager [[00E188A8]+21D0h]), RET. Walks the std::list
// at [manager+8]+0Ch (head [manager+8]+10h; node +0 next, +8 object) and
// calls vtable slot 0 (scalar deleting destructor) with 1 on every non-null
// object whose word +4 is 2 or 1. The nodes stay in the list. Existing seam:
// game_object_list_walk_004881f0(void* object), include/bsp/cc13_sweepA1b.hpp.
struct DeleteKindObjectsHost : MemoryHost {
    // CALL [[object]] with ECX = object, stack 1 (scalar deleting destructor).
    virtual void object_scalar_deleting_destructor(Address object, std::uint32_t flags) = 0;
};

void manager21d0_delete_kind1_and_2_objects_004881f0(void* manager, DeleteKindObjectsHost& host);

// ---------------------------------------------------------------------------
// 0048CA60 / 0048CB30 / 0048CBB0: the entry tree at manager+28h
// ---------------------------------------------------------------------------
// All three run under the manager lock: EnterCriticalSection([manager+4])
// through [00CE2218], ++[[manager+4]+18h]; at the end --[[manager+4]+18h]
// (+4 re-read) and LeaveCriticalSection through [00CE2210]. The key is looked
// up with 00488630 STL_Tree_Find_00488630 (__thiscall(ECX = manager+28h,
// out iterator, &key), RET 8). A found node's +10h is the entry pointer.
struct Manager21D0TreeHost : MemoryHost {
    virtual void enter_critical_section_00ce2218(Address section) = 0;
    virtual void leave_critical_section_00ce2210(Address section) = 0;
    // 00488630: writes the iterator for *key (end() when absent) to *out.
    virtual void tree_find_00488630(Address tree, TreeIterator* out, const std::uint32_t* key) = 0;
};

// 0048CA60 BSP_Manager21D0_CountEntryEvent, __thiscall(ECX = manager,
// record, flag), RET 8. key = ([record] ?: [record+8]) + 1E4h, or 0 when both
// words are zero. Found: ++[[entry]+D8h]+10h when the low byte of flag is
// set, else ++[[entry]+D8h]+14h. Existing seam:
// game_21d0_register_0048ca60(owner, record, flag), include/bsp/cc13_seg29U.hpp.
void manager21d0_count_entry_event_0048ca60(void* manager, void* record, std::int32_t flag,
                                            Manager21D0TreeHost& host);

// 0048CB30 BSP_Manager21D0_ClearEntryActive, __thiscall(ECX = manager,
// key), RET 4. Found: [entry+DCh] = 0. Existing seam:
// manager21d0_call_0048cb30(manager, entry), include/bsp/cc13_sweepB.hpp.
void manager21d0_clear_entry_active_0048cb30(void* manager, void* key, Manager21D0TreeHost& host);

// 0048CBB0 BSP_Manager21D0_IsEntryActiveAndSet, __thiscall(ECX = manager,
// key), RET 4 -> AL. 1 when found and [entry+DCh] and [entry+BCh] are both
// non-zero. Existing seam: pad_group_query_0048cbb0(service, unit_sub),
// include/bsp/cc13_seg33b.hpp (Object = std::uint8_t*).
bool manager21d0_is_entry_active_and_set_0048cbb0(void* manager, void* key,
                                                  Manager21D0TreeHost& host);

// ---------------------------------------------------------------------------
// 00687860 BSP_MenuManager_HasScreen
// ---------------------------------------------------------------------------
// __thiscall(ECX = menu manager *00E198B4, id, flag), RET 8 -> AL. True when
// the low byte of flag is 0 and [this+20h] == id; otherwise true when id is
// in the dword vector [this+5Ch, this+60h). Existing seams (one per caller):
// object_00e198b4_test_00687860(a, b), interface_query_00687860(),
// multi_menu_query_00687860(a, b), multi_menu_has_interface_00687860(id).
bool menu_manager_has_screen_00687860(void* manager, std::int32_t id, std::int32_t flag,
                                      MemoryHost& host);

// ---------------------------------------------------------------------------
// 006A9EA0 / 006AA1C0 / 006AA2D0: input settings device maps
// ---------------------------------------------------------------------------
// The settings object holds two device maps, +8h (head +0Ch) and +6Ch. A map
// node keeps the device key at +0Ch and the mapped value from +14h; the value
// keeps three binding trees at +0Ch, +24h and +6Ch (node +20h, +38h, +80h).
struct InputSettingsHost : MemoryHost {
    // 0055C110 STL_InputDeviceMap_Index_N9C_0055C110, __thiscall(ECX = map,
    // key address), RET 4 -> EAX = mapped value (inserts a default when absent).
    virtual Address input_device_map_index_0055c110(Address map, Address key) = 0;
};

// 006A9EA0 BSP_InputSettings_DeviceHasBindingKind2, __thiscall(ECX = settings,
// key), RET 4 -> AL. Walks the device's binding tree (value+0Ch); for each node
// checks the first word of elements 0 and 1 of the 14h-byte vector at node+18h
// (end +1Ch) and answers 1 on the first equal to 2. Existing seam:
// input_device_has_kind2_binding_006a9ea0(settings, key),
// include/bsp/frontend_seg12_17_cc13.hpp.
struct DeviceBindingKindHost : InputSettingsHost {
    // 00552DB0 STL_TreeIter_Inc_00552db0, __thiscall(ECX = iterator), RET.
    virtual void tree_iterator_increment_00552db0(TreeIterator* iterator) = 0;
};

bool input_settings_device_has_binding_kind2_006a9ea0(void* settings, const void* key,
                                                      DeviceBindingKindHost& host);

// 006AA1C0 / 006AA2D0: for every node of the map at +8h, 0055C110 on the map
// at +6Ch with the node's key, then three tree assignments (each skipped when
// destination == source): erase(begin, end) then copy. 006AA1C0 assigns the
// +8h node's trees into the +6Ch value; 006AA2D0 assigns the other way. Which
// map is the live one is unproven; the callers (006AA1C0 from
// BSP_ControlBindingScreen_ListboxListener04, 006AA2D0 from the reload/revert/
// confirm paths) suggest +6Ch is the saved copy. Both __thiscall(ECX =
// settings), RET, EBP frame with ESP aligned to 8.
struct InputSettingsCopyHost : InputSettingsHost {
    // Erase ranges, __thiscall(ECX = tree, out iterator, first, last), RET 14h.
    virtual void binding_tree_erase_005599c0(Address tree, TreeIterator* out, TreeIterator first,
                                             TreeIterator last) = 0;
    virtual void scalar_tree_erase_00444b10(Address tree, TreeIterator* out, TreeIterator first,
                                            TreeIterator last) = 0;
    virtual void bit_vector_tree_erase_00559280(Address tree, TreeIterator* out, TreeIterator first,
                                                TreeIterator last) = 0;
    // Copies, __thiscall(ECX = destination tree, source tree), RET 4.
    virtual void binding_tree_copy_0055a160(Address destination, Address source) = 0;
    virtual void scalar_tree_copy_00499680(Address destination, Address source) = 0;
    virtual void bit_vector_tree_copy_0055a890(Address destination, Address source) = 0;
    // 005540C0 STL_InputDeviceIterator_Next_005540C0, __thiscall(ECX = iterator), RET.
    virtual void device_iterator_next_005540c0(TreeIterator* iterator) = 0;
};

// Existing seam: input_settings_006aa1c0(settings), frontend_seg12_17_cc13.hpp.
void input_settings_copy_primary_to_shadow_006aa1c0(void* settings, InputSettingsCopyHost& host);
// Existing seams: input_settings_reload_006aa2d0() and
// input_settings_save_006aa2d0() (ECX = 005547D0()), frontend_seg12_17_cc13.hpp.
void input_settings_copy_shadow_to_primary_006aa2d0(void* settings, InputSettingsCopyHost& host);

} // namespace bsp::cc15_g24b
