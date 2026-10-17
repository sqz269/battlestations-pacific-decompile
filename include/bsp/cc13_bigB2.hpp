#pragma once

// cc13_bigB2: large leftovers of the segment 15-51 sweep, each reconstructed
// as a whole body (packet cc13-bigB2, docs/CC13_BIGB2.md). Every name is a
// hypothesis, not a recovered symbol. These are new C++ interfaces over the
// native records, not drop-in replacements of the original ABI. Nothing here
// is wired into the game.
//
// Same conventions as cc13_sweepB..B9: native objects are raw byte blocks
// touched only at the offsets the image reads or writes, and every call the
// image makes goes through a named host method at the same point with the
// same arguments. Cc13BigB2Host extends Cc13SweepB9Host, so the earlier hooks
// are reused as they are.

#include "bsp/cc13_sweepB9.hpp"

#include <cstdint>

namespace bsp {

class Cc13BigB2Host : public Cc13SweepB9Host {
public:
    // ---- HUD support manager (segment 27, 00670020) ----
    // 0065A550 (support_manager_update_slot_strip_0065a550 in seg27_cc13):
    // __fastcall(ECX = screen), RET.
    virtual void hud_support_update_slot_strip_0065a550(void* screen) = 0;
    // 0066FB00 (the support-list refresh; ingame_gui_cc13 hook
    // support_refresh_0066fb00): __fastcall(ECX = screen), RET.
    virtual void hud_support_refresh_0066fb00(void* screen) = 0;
    // 004BCB80 BSP_NativeString_ReverseFindAnyByte: __thiscall(ECX = string;
    // const char* set, unsigned from), RET 8; EAX = position or -1.
    virtual std::int32_t native_string_rfind_any_004bcb80(void* string, const char* set, std::uint32_t from) = 0;
    // 00614060 (std::map<int, int> find, the icon-state cache at screen+42Ch):
    // __thiscall(ECX = map; iterator* result, const int* key), RET 8; EAX =
    // result ({container, node}).
    virtual void* icon_map_find_00614060(void* map, void* result, const void* key) = 0;
    // 006171B0 (the same map's operator[]): __thiscall(ECX = map; const int*
    // key), RET 4; EAX = &value.
    virtual std::int32_t* icon_map_subscript_006171b0(void* map, const void* key) = 0;
    // 00AB64C0 BSP_GuiIcon_AddState with its result: __thiscall(ECX = icon;
    // texture, const float uv[4]), RET 8; EAX = the new state's index.
    virtual std::int32_t gui_icon_add_state_index_00ab64c0(void* icon, void* texture, const float* uv) = 0;
    // 004E79F0 (std::list _Buyheadnode): ECX = the list, RET; EAX = the new
    // head node.
    virtual void* list_buy_head_node_004e79f0(void* list) = 0;
    // 004E8150 (std::list destructor body by its ledger name STL_List_Clear):
    // ECX = the list {proxy, head, size}, RET.
    virtual void list_clear_004e8150(void* list) = 0;
    // 00664AF0 (std::list _Insert(where, first, last, tag)): __thiscall(ECX =
    // list; where {container, node}, first {container, node}, last {container,
    // node}, tag dword), RET 1Ch. The tag is an empty iterator-category struct;
    // the image pushes an unrelated stack dword for it.
    virtual void list_insert_range_00664af0(void* list, void* where_container, void* where_node,
                                            void* first_container, void* first_node, void* last_container,
                                            void* last_node, std::uint32_t tag) = 0;
    // Unit vtable +198h: __thiscall(ECX = unit; const NativeString* name, 0,
    // bool* out), RET 0Ch (as cc13_seg33b's object_virtual_198).
    virtual void unit_vcall_198_query(void* unit, const void* name, std::int32_t zero, std::uint8_t* out) = 0;

    // ---- mission objective lists (segment 26, 00627960) ----
    // The vectors are image globals reached through image_global(address).
    // 006226F0 (std::vector<int>::erase(first, last)): __thiscall(ECX = vector;
    // iterator* result, first {container, pointer}, last {container,
    // pointer}), RET 14h.
    virtual void int_vector_erase_range_006226f0(void* vector, void* result, void* first_container, void* first,
                                                 void* last_container, void* last) = 0;
    // 005F6190 STL_BitVector_Clear: ECX = the std::vector<bool>, RET.
    virtual void bit_vector_clear_005f6190(void* vector) = 0;
    // 006257D0 (std::vector<int>::push_back): ECX = vector, (const int*), RET 4.
    virtual void int_vector_push_back_006257d0(void* vector, const std::int32_t* value) = 0;
    // 0049F2D0 (std::vector<bool>::push_back): ECX = vector, (bool by value in
    // a dword; the image leaves the upper bytes of EAX unspecified), RET 4.
    virtual void bit_vector_push_back_0049f2d0(void* vector, std::uint8_t value) = 0;
    // 0048D3B0 STL_BitIterator_Advance: ECX = iterator {container, word
    // pointer, bit offset}, (int count), RET 4.
    virtual void bit_iterator_advance_0048d3b0(void* iterator, std::int32_t count) = 0;
    // 0049E150 (std::vector<bool>::insert(where, value)): __thiscall(ECX =
    // vector; iterator* result, where {container, word pointer, bit offset}
    // by value, bool by value in a dword), RET 14h.
    virtual void bit_vector_insert_0049e150(void* vector, void* result, void* where_container, void* where_word,
                                            std::uint32_t where_offset, std::uint8_t value) = 0;
    // 00623170 STL_IntroSortLoop (int elements): ECX = first, EDX = last,
    // (int ideal = last - first), RET 4.
    virtual void int_sort_00623170(std::int32_t* first, std::int32_t* last, std::int32_t ideal) = 0;
    // 00773860 BSP_Session_SendGameSetupMessage: ECX = game+1EF0h, RET.
    virtual void session_send_game_setup_00773860(void* session) = 0;
};

// 00670020: support-manager screen refresh (slot strip, slot labels, the
// selected unit's icon, the map markers, the unit failure gauges and the
// list widgets). __fastcall(ECX = screen), RET.
void cc13_bigB2_hud_support_refresh_view_00670020(void* screen, Cc13BigB2Host& host);

// 00627960: rebuild the mission objective lists (image globals 00E196CC..
// 00E19794) from Mission.Objectives.primary / secondary / hidden, then send
// the game-setup message when the session is networked. __cdecl, no
// arguments, RET.
void cc13_bigB2_mission_objectives_rebuild_00627960(Cc13BigB2Host& host);

}  // namespace bsp
