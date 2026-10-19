#pragma once

#include <cstdint>

// Seven game-code rows the cc15-stlA audit moved out of stl_probable (packet
// cc15-g24a). New C++ interfaces; names are hypotheses, not recovered symbols.
// docs/CC15_G24A.md records the original ABIs, evidence and limits. Raw object
// pointers need the native Win32 layouts (32-bit pointers). No host is wired to
// any of these. Every 00BF6713 (_invalid_parameter_noinfo) call returns in the
// image and is modelled as a seam call at the same place; the walk continues.
namespace bsp::cc15_g24a {

// ---------------------------------------------------------------------------
// 0045B420 BSP_MovieSpline_AddPoint
// ---------------------------------------------------------------------------
// __thiscall(ECX = owner; float3 position, float time, float3 in tangent,
// float3 out tangent, int type, float alpha, NativeString comment by value
// {+0 length, +4 data}), RET 38h, FS:[0] frame (handler 00C60A80). Matches the
// seam movie_spline_add_point_0045b420 (include/bsp/cc13_sweepA3.hpp); its only
// caller is 0045ECA0. The owner's point list is a VC9 std::list at +0F4h
// (+0F8h head, +0FCh size); node {+0 next, +4 prev, +8 the 38h-byte record,
// whose +0 is the time float}. One 38h-byte record is built on the stack by
// 00465A40 and inserted:
//   empty list           -> before begin (buy node + link inline)
//   begin time > time    -> before begin (same inline insert)
//   otherwise            -> after 00455BC0's "sample before time" iterator,
//                           through the list insert 0045B0A0
// then the record's NativeString (+4 length, +8 data) and the by-value
// argument are released to the sized pool (length + 1, flag 1).
struct MovieSplineAddPointHost {
    virtual ~MovieSplineAddPointHost() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    // 0041DD40 BSP_NativeString_Resize, __thiscall(ECX = string; size, flag), RET 8.
    virtual void native_string_resize_0041dd40(std::uint8_t* string, std::uint32_t size,
                                               std::uint32_t flag) = 0;
    // 00BF7680 _memcpy, cdecl(destination, source, bytes).
    virtual void memcpy_00bf7680(void* destination, const void* source, std::uint32_t bytes) = 0;
    // 00426060 BSP_NativeString_CopyConstruct, __thiscall(ECX = destination;
    // const NativeString* source), RET 4.
    virtual void native_string_copy_construct_00426060(std::uint8_t* destination,
                                                       const std::uint8_t* source) = 0;
    // 00465A40 BSP_MovieTimedRecord_Construct, __thiscall(ECX = record; the
    // same 38h bytes of arguments as 0045B420, the NativeString last by value,
    // released by the callee), RET 38h, EAX = record.
    virtual std::uint8_t* movie_timed_record_construct_00465a40(
        std::uint8_t* record, const float* position, float time, const float* in_tangent,
        const float* out_tangent, std::int32_t type, float alpha, std::uint8_t* comment_by_value) = 0;
    // 00455BC0 BSP_EntityTrack_FindSampleBefore, __thiscall(ECX = owner;
    // iterator out {+0 container, +4 node}, float time), RET 8.
    virtual void entity_track_find_sample_before_00455bc0(std::uint8_t* owner, std::uint32_t* out,
                                                          float time) = 0;
    // 00459EA0 STL_List_BuyNode, __thiscall(ECX = list; next, prev, const
    // record*), RET 0Ch, EAX = node.
    virtual std::uint8_t* list_buy_node_00459ea0(std::uint8_t* list, std::uint8_t* next,
                                                 std::uint8_t* prev, const std::uint8_t* record) = 0;
    // 00459900 (list _Incsize), __thiscall(ECX = list; count), RET 4.
    virtual void list_inc_size_00459900(std::uint8_t* list, std::uint32_t count) = 0;
    // 0045B0A0 STL_inst (list insert), __thiscall(ECX = list; iterator* result,
    // where {container, node} by value, const record*), RET 10h.
    virtual void list_insert_0045b0a0(std::uint8_t* list, std::uint32_t* result,
                                      std::uint8_t* where_container, std::uint8_t* where_node,
                                      const std::uint8_t* record) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton, EAX = pool.
    virtual std::uint8_t* sized_pool_singleton_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(ECX = pool; data,
    // bytes, flag), RET 0Ch.
    virtual void sized_pool_return_block_00bd1510(std::uint8_t* pool, void* data, std::uint32_t bytes,
                                                  std::uint32_t flag) = 0;
};

// `comment` is the 8-byte by-value NativeString argument slot.
void movie_spline_add_point_0045b420(std::uint8_t* owner, const float* position, float time,
                                     const float* in_tangent, const float* out_tangent,
                                     std::int32_t type, float alpha, std::uint8_t* comment,
                                     MovieSplineAddPointHost& host);

// ---------------------------------------------------------------------------
// 0045DDA0 BSP_EngineMovie_Reset
// ---------------------------------------------------------------------------
// __thiscall(ECX = movie), RET. Matches the seam engine_movie_reset_0045dda0
// (include/bsp/cc13_sweepA3.hpp); caller 004602C0 BSP_EngineMovie_ParseFile.
// The track pointer vector is at +64h (+68h first, +6Ch last). Each non-null
// track goes through 0045B220 then _free; the slots are not cleared. The
// vector is then emptied by the inlined erase(begin, end) and movie+0 / +4
// are reloaded from the floats at 00D7A24C / 00CE3800 (1.0f and 0.5f on disk).
struct EngineMovieResetHost {
    virtual ~EngineMovieResetHost() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    // 0045B220 BSP_MovieEntityTrack_Destroy, __thiscall(ECX = track), RET.
    virtual void movie_entity_track_destroy_0045b220(std::uint8_t* track) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // 00BF67A7 _memmove_s, cdecl(destination, destination bytes, source, bytes).
    virtual void memmove_s_00bf67a7(void* destination, std::uint32_t destination_bytes,
                                    const void* source, std::uint32_t bytes) = 0;
    virtual float global_float_00d7a24c() = 0;
    virtual float global_float_00ce3800() = 0;
};

void engine_movie_reset_0045dda0(std::uint8_t* movie, EngineMovieResetHost& host);

// ---------------------------------------------------------------------------
// 00480A90 BSP_WeightedIndex_PickBySeed
// ---------------------------------------------------------------------------
// __fastcall(ECX = seed, EDX = std::vector<float>* weights), RET, EAX = index.
// Matches the seam weighted_pick_00480a90 (include/bsp/cc13_left8_seg8.hpp).
// threshold = float(((seed * 7B913h + 67933h) mod 5A1C9h) / [00CE6428]) with
// the double 369097.0 on disk; returns the first index whose running float
// sum exceeds the threshold, 0 when none does or the vector is empty.
struct WeightedPickHost {
    virtual ~WeightedPickHost() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    // 0043EC30 STL_Vector_Size, __thiscall(ECX = vector), EAX.
    virtual std::uint32_t vector_size_0043ec30(std::uint8_t* vector) = 0;
    virtual double global_double_00ce6428() = 0;
};

std::uint32_t weighted_pick_00480a90(std::uint32_t seed, std::uint8_t* weights, WeightedPickHost& host);

// ---------------------------------------------------------------------------
// 004872D0 BSP_ListNode_RemoveChild
// ---------------------------------------------------------------------------
// __thiscall(ECX = parent; child), RET 4. The inlined std::list::remove(child)
// over the parent's child list at +0Ch (+10h head, +14h size; node {+0 next,
// +4 prev, +8 value}). Callers 00489DD0 (at 00489E05) and 00487360.
struct ListNodeRemoveChildHost {
    virtual ~ListNodeRemoveChildHost() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void free_00bf65ac(void* block) = 0;
};

void list_node_remove_child_004872d0(std::uint8_t* parent, std::uint8_t* child,
                                     ListNodeRemoveChildHost& host);

// ---------------------------------------------------------------------------
// 00489DD0 BSP_ListNode_Destruct
// ---------------------------------------------------------------------------
// __thiscall(ECX = node), RET, FS:[0] frame (handler 00C629AB, state 0).
// Matches the seams list_node_destruct_00489dd0 (cc13_left8_seg8.hpp),
// scene_proxy_base_destroy_00489dd0 (classes_seg7_cc13.hpp) and
// base_destroy_00489dd0 (oct18_tail_a.hpp). Stores vtable 00CE6600, detaches
// from the parent at +8 through 004872D0, then deletes the front child until
// the list at +0Ch is empty (each child's own destructor unlinks it), then
// 00485290 tidies the list.
inline constexpr std::uint32_t kListNodeVtable = 0x00CE6600u;

struct ListNodeDestructHost {
    virtual ~ListNodeDestructHost() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    // 004872D0, __thiscall(ECX = parent; child), RET 4 (body above).
    virtual void list_node_remove_child_004872d0(std::uint8_t* parent, std::uint8_t* child) = 0;
    // Vtable slot 0 of the child, __thiscall(ECX = child; flags = 1), RET 4.
    virtual void scalar_deleting_destructor(std::uint8_t* object, std::uint32_t flags) = 0;
    // 00485290 STL_List_Tidy, __thiscall(ECX = list), RET.
    virtual void list_tidy_00485290(std::uint8_t* list) = 0;
};

void list_node_destruct_00489dd0(std::uint8_t* node, ListNodeDestructHost& host);

// ---------------------------------------------------------------------------
// 004A5AA0 BSP_BreakupRegistry_DestroyUnitEntries
// ---------------------------------------------------------------------------
// __thiscall(ECX = manager ([game+21D0h]); unit), RET 4. Matches the seam
// breakup_register_wreck_004a5aa0 (include/bsp/cc13_segmix.hpp); callers
// 00814520 and 00824B60. Map at manager+1Ch keyed by the unit word (the key
// pointer is the address of the argument slot). While map[unit] (004A5750)
// is non-empty, its front object is deleted (vtable slot 0, flags 1; the
// subscript is re-evaluated each pass); then map.erase(map.find(unit)).
struct BreakupDestroyUnitEntriesHost {
    virtual ~BreakupDestroyUnitEntriesHost() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    // 004A5750 STL_Map_Subscript_U32, __thiscall(ECX = map; const key*), RET 4,
    // EAX = the mapped vector {+4 first, +8 last}.
    virtual std::uint8_t* map_subscript_004a5750(std::uint8_t* map, const std::uint32_t* key) = 0;
    virtual void scalar_deleting_destructor(std::uint8_t* object, std::uint32_t flags) = 0;
    // 0048CF20 (ledger name STL_Tree_EqualRange; the body is find), __thiscall(
    // ECX = map; iterator* out {container, node}, const key*), RET 8.
    virtual void map_find_0048cf20(std::uint8_t* map, std::uint32_t* out, const std::uint32_t* key) = 0;
    // 00491F40 (ledger name STL_xlen_throw; the body is erase(iterator)),
    // __thiscall(ECX = map; iterator* result, where {container, node} by
    // value), RET 0Ch.
    virtual void map_erase_00491f40(std::uint8_t* map, std::uint32_t* result, std::uint32_t container,
                                    std::uint32_t node) = 0;
};

void breakup_registry_destroy_unit_entries_004a5aa0(void* manager, void* unit,
                                                    BreakupDestroyUnitEntriesHost& host);

// ---------------------------------------------------------------------------
// 004AD380 BSP_PtrVectorOwner00CE6F68_Destruct
// ---------------------------------------------------------------------------
// __thiscall(ECX = object), RET, FS:[0] frame (handler 00C6467B, state 0).
// Only caller 004AD5C0 (the scalar deleting destructor). Stores vtable
// 00CE6F68; deletes every non-null element of the pointer vector at +14h
// (+18h first, +1Ch last, +20h end) through vtable slot 0 (flags 1) and nulls
// the slot; erase(begin, end); frees the storage; zeroes +18h/+1Ch/+20h.
inline constexpr std::uint32_t kPtrVectorOwnerVtable = 0x00CE6F68u;

struct PtrVectorOwnerDestructHost {
    virtual ~PtrVectorOwnerDestructHost() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void scalar_deleting_destructor(std::uint8_t* object, std::uint32_t flags) = 0;
    virtual void memmove_s_00bf67a7(void* destination, std::uint32_t destination_bytes,
                                    const void* source, std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
};

void ptr_vector_owner_destruct_004ad380(std::uint8_t* object, PtrVectorOwnerDestructHost& host);

} // namespace bsp::cc15_g24a
