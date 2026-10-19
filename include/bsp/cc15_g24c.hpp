#pragma once

#include <cstdint>

#include "bsp/cc13_seg47.hpp"

// Rows the cc15-stlA audit reclassified from stl_probable to game code
// (packet cc15-g24c): three mission-narrative list writers, the objective
// position-entry eraser and the node child-collector append. New C++
// interfaces; names are hypotheses, not recovered symbols. docs/CC15_G24C.md
// records the original ABIs, evidence and limits. Raw object pointers need the
// native Win32 layouts (VC9 std::list / std::vector with a leading proxy
// word). No host is wired to any of these.
namespace bsp::cc15_g24c {

// ---------------------------------------------------------------------------
// Mission narrative list (00734870, 007349C0, 00734E50)
// ---------------------------------------------------------------------------
// The narrative object (the client at game+21E8h) holds a VC9 std::list at
// +4h: head node at +8h, size at +0Ch. A node is {next +0h, prev +4h, entry
// +8h}; the 18h-byte entry is {vtable +0h (00CFE960), NativeString key +4h,
// NativeString text +0Ch, int value +14h}, so the node's key is at +0Ch, the
// text at +14h and the value at +1Ch. A NativeString is {length +0h, data
// +4h} (engine_seg4_6_9_cc13.hpp NativeNarrowString).
inline constexpr std::uint32_t kNarrativeEntryVtable = 0x00CFE960u;
inline constexpr std::uint32_t kNarrativeEntrySize = 0x18u;

struct NarrativeListHost {
    virtual ~NarrativeListHost() = default;
    // 007342B0 BSP_NarrativeList_BuyNode, __thiscall(ECX = list (narrative+4h);
    // next, prev, const entry*), RET 0Ch, EAX = new node holding a copy of entry.
    virtual std::uint8_t* narrative_list_buy_node_007342b0(std::uint8_t* list, std::uint8_t* next,
                                                           std::uint8_t* prev,
                                                           const std::uint8_t* entry) = 0;
    // 007341B0 (ledger STL_xlen_throw_007341b0), __thiscall(ECX = list; count),
    // RET 4: the size increment that throws length_error on overflow.
    virtual void narrative_list_incsize_007341b0(std::uint8_t* list, std::uint32_t count) = 0;
    // 00733DC0 BSP_NarrativeEntry_Destruct, __thiscall(ECX = entry), RET.
    virtual void narrative_entry_destruct_00733dc0(std::uint8_t* entry) = 0;
    // 00734CE0 BSP_NarrativeList_Clear, __thiscall(ECX = list), RET (00734E50 only).
    virtual void narrative_list_clear_00734ce0(std::uint8_t* list) = 0;
    // 00435C40 BSP_NativeString_EqualsInsensitive, __thiscall(ECX = string;
    // const NativeString* other), RET 4, AL (007349C0 only).
    virtual bool native_string_equals_insensitive_00435c40(const std::uint8_t* string,
                                                           const void* other) = 0;
    // 0041DD40 BSP_NativeString_Resize, __thiscall(ECX = string; length, 1), RET 8.
    virtual void native_string_resize_0041dd40(std::uint8_t* string, std::uint32_t length,
                                               std::int32_t flag) = 0;
    // 00BF7680 _memcpy, __cdecl(dst, src, size).
    virtual void crt_memcpy_00bf7680(void* dst, const void* src, std::uint32_t size) = 0;
    // 00BF6713 _invalid_parameter_noinfo (the checked-iterator failure). It
    // returns in this image, so execution falls through after the call.
    virtual void crt_invalid_parameter_noinfo_00bf6713() = 0;
};

// 00734870 BSP_NarrativeList_Append: __thiscall(ECX = narrative; const
// NativeString* key, const NativeString* text, int value), RET 0Ch. Pushes a
// default entry at the back, then copies key, text and value into back().
// Matches narrative_list_append_00734870 (cc13_front3.hpp, cc13_seg48U.hpp).
void narrative_list_append_00734870(void* narrative, const void* key, const void* text,
                                    std::uint32_t value, NarrativeListHost& host);

// 007349C0 BSP_NarrativeList_PushFrontUnlessSame: same ABI, RET 0Ch. Returns
// without change when the list is non-empty and front()'s key and text both
// equal the arguments (case-insensitive); otherwise pushes a default entry at
// the front and copies key, text and value into front(). Matches
// client_007349c0 (engine_seg4_6_9_cc13.hpp) with &key, &out, arg.
void narrative_list_push_front_unless_same_007349c0(void* narrative, const void* key,
                                                    const void* text, std::uint32_t value,
                                                    NarrativeListHost& host);

// 00734E50 BSP_NarrativeList_ReplaceAll: same ABI, RET 0Ch. Clears the list,
// then appends exactly as 00734870 does.
void narrative_list_replace_all_00734e50(void* narrative, const void* key, const void* text,
                                         std::uint32_t value, NarrativeListHost& host);

// ---------------------------------------------------------------------------
// 008DC500 BSP_Objective_RemovePositionEntries
// ---------------------------------------------------------------------------
// __thiscall(ECX = objective; const float pos[3]), RET 4. The objective holds
// a VC9 std::list at +20h (head +24h, size +28h) whose node value (+8h) points
// at a record {int kind +0h, float x +4h, y +8h, z +0Ch}. Every node whose
// record has kind 0 and x/y/z ordered-equal to pos (FUCOMIP, LAHF, TEST AH,44h,
// JP: NaN never matches) has its record freed (the slot zeroed) and is erased.
// Matches objective_erase_position_entries_008dc500 (objectives_seg57_argon.hpp).
struct ObjectivePositionHost {
    virtual ~ObjectivePositionHost() = default;
    // 00BF65AC _free, __cdecl(block).
    virtual void crt_free_00bf65ac(void* block) = 0;
    // 00BF6713 _invalid_parameter_noinfo; falls through.
    virtual void crt_invalid_parameter_noinfo_00bf6713() = 0;
};

void objective_erase_position_entries_008dc500(void* objective, const float* pos3,
                                               ObjectivePositionHost& host);

// ---------------------------------------------------------------------------
// 007D05E0 BSP_Node_AppendChildrenByNamePrefix
// ---------------------------------------------------------------------------
// __fastcall(ECX = node, EDX = const char* name; vector*), RET 4. Collects the
// matching children into a local 10h-byte vector through 0071BA20, appends
// each element to the argument vector (+4h first, +8h last, +0Ch end) in place
// while capacity allows and through 00505D50 otherwise, then frees the local
// buffer. Matches node_append_children_by_name_007d05e0 (cc13_seg47b.hpp).
struct NodeAppendChildrenHost {
    virtual ~NodeAppendChildrenHost() = default;
    // 0071BA20 BSP_Node_CollectChildrenByNamePrefix, __thiscall(ECX = node; out
    // vector (10h: +4h first, +8h last, +0Ch end), const char* prefix), RET 8.
    virtual void node_collect_children_by_name_prefix_0071ba20(void* node, void* out_vector,
                                                               const char* prefix) = 0;
    // 00505D50 STL_CheckedDwordVector_InsertOne, __thiscall(ECX = vector; out*,
    // container, where, const value*), RET 10h.
    virtual void dword_vector_insert_one_00505d50(void* vector, void* out, void* container,
                                                  void* where, const void* value) = 0;
    // 00BF65AC _free, __cdecl(block).
    virtual void crt_free_00bf65ac(void* block) = 0;
    // 00BF6713 _invalid_parameter_noinfo; falls through.
    virtual void crt_invalid_parameter_noinfo_00bf6713() = 0;
};

void node_append_children_by_name_007d05e0(cc13_seg47::Raw node, const char* name, void* vector,
                                           NodeAppendChildrenHost& host);

} // namespace bsp::cc15_g24c
