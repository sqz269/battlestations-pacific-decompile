// Open game-named rows of the approved segments, packet cc13-gamerows (marker cc13_gamerows).
//
// docs/CC13_GAMEROWS.md classes the 138 rows; this file holds batch 2, the small rebuilds:
// the special members of two ship-class record types and one game virtual. The two record
// types hold std::map members, so every tree operation goes through GameRowsHost (the same
// callees cc13_left8_seg8 hooks for the 4Ch traffic record constructor 0049F9B0).
//
// Model: raw bytes (Win32 layout, image offsets). A new C++ interface, not a drop-in binary
// replacement; nothing here is wired into bsp_game. Names are hypotheses.

#ifndef BSP_CC13_GAMEROWS_HPP
#define BSP_CC13_GAMEROWS_HPP

#include <cstdint>

namespace bsp::cc13_gamerows {

class GameRowsHost {
public:
    virtual ~GameRowsHost() = default;
    // 004858D0 __thiscall(ECX = map), EAX = a new head node (isnil byte at +15h).
    virtual void* tree_buy_head_node_004858d0(void* map) = 0;
    // 00443E20 __thiscall(ECX = map), EAX = a new head node (isnil byte at +19h).
    virtual void* string_float_map_blank_node_00443e20(void* map) = 0;
    // Map range erasers __thiscall(ECX = map; &result, container, first, container, last), RET 14h.
    virtual void tree_erase_range_00492a00(void* map, void* result, void* c0, void* first, void* c1,
                                           void* last) = 0;
    virtual void tree_erase_range_00444b10(void* map, void* result, void* c0, void* first, void* c1,
                                           void* last) = 0;
    // Map copy constructors __thiscall(ECX = map; const map* source), RET 4.
    virtual void tree_copy_construct_004990b0(void* map, const void* source) = 0;
    virtual void string_float_map_copy_construct_0049dec0(void* map, const void* source) = 0;
    // Map copy into an existing (emptied) map, __thiscall(ECX = map; const map* source), RET 4.
    virtual void tree_copy_0048d080(void* map, const void* source) = 0;
    virtual void scalar_tree_copy_00499680(void* map, const void* source) = 0;
    // 00BF65AC _free.
    virtual void free_00bf65ac(void* block) = 0;

    // --- Batch 4 callees ---
    // 00BF55BE operator new (cdecl, one size argument).
    virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
    // 00BF6989, a JMP thunk to _free.
    virtual void free_00bf6989(void* block) = 0;
    // 00BF6713, the checked-iterator report (_invalid_parameter_noinfo). The binary carries on
    // after the call; a host may treat it as fatal.
    virtual void invalid_parameter_00bf6713() = 0;
    // 00BF67A7 memmove_s(dst, dst_size, src, count) (cdecl).
    virtual void memmove_s_00bf67a7(void* dst, std::uint32_t dst_size, const void* src, std::uint32_t count) = 0;
    // `CALL [vtable+0]` with one stack word: the 14h-byte vector element's virtual destructor
    // (flags 0, so no free). __thiscall(ECX = element; flags).
    virtual void element_virtual_destroy_slot0(void* element, std::uint32_t flags) = 0;
    // `CALL [vtable+20h]` on a Point3PtrArray, no arguments: returns a new 12-byte point.
    virtual float* point3_ptr_array_new_point_slot20(void* array) = 0;
    // 007B26F0 STL_Vector_InsertOne, __thiscall(ECX = vector; &result, owner, where, const T&).
    virtual void vector_insert_one_007b26f0(void* vector, void* result, void* owner, void* where,
                                            void* const* value) = 0;
    // Recorded reserve routines of the same engine vectors, __thiscall(ECX = vector; n), RET 4:
    // 0086A220 (src/native_input_bin*), 0086A350 (src/cc13_front6.cpp) and 0086A4D0.
    virtual void dword_vector_reserve_0086a220(void* vector, std::int32_t n) = 0;
    virtual void qword_vector_reserve_0086a350(void* vector, std::int32_t n) = 0;
    virtual void particle_resource_pointers_reserve_0086a4d0(void* vector, std::int32_t n) = 0;
    // 00419CC0 returns the sized storage pool; 00BD1510 __thiscall(pool; block, size, 1), RET 0Ch.
    virtual void* pool_singleton_00419cc0() = 0;
    virtual void pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size, int flag) = 0;
};

// --- The 1Ch-byte Idle record (ship_class_fields.hpp ShipClassIdleOffsets): a map at +0h
// (head +4h, size +8h; 004858D0 / 00492A00 / 004990B0), a map at +0Ch (head +10h, size +14h;
// 00443E20 / 00444B10 / 0049DEC0) and the dword posID - 1 at +18h. ---

// 0082E1F0 __thiscall(ECX = record), RET, EAX = record: default construct (+18h not written).
void* idle_record_construct_0082e1f0(GameRowsHost& host, void* record);
// 0082E280 __thiscall(ECX = record), RET: the out-of-line destructor. The map at +0Ch is
// emptied first, then the one at +0h; each head is freed and its head/size cleared.
void idle_record_destruct_0082e280(GameRowsHost& host, void* record);
// 0082E5A0 __thiscall(ECX = record; const record* source), RET 4, EAX = record: copy construct.
void* idle_record_copy_construct_0082e5a0(GameRowsHost& host, void* record, const void* source);

// --- The 4Ch traffic record (cc13_left8_seg8.cpp 0049F9B0 / 0049FA70 / 0049F910). ---

// 0082DF00 __thiscall(ECX = record; const record* source), RET 4, EAX = record: operator=.
// Dwords +0..+8, byte +0Ch, dwords +10h/+14h, floats +18h..+30h (FLD/FSTP pairs) are copied;
// then each map (+34h: 00492A00 then 0048D080; +40h: 00444B10 then 00499680) is emptied and
// refilled from the source's, skipped when it is the source's own map.
void* traffic_record_assign_0082df00(GameRowsHost& host, void* record, const void* source);

// --- A game virtual ---

// 009A7A40 __thiscall(ECX = object), RET: FLDZ, so ST0 = 0.0 (vtable slot 00D1F910).
float virtual_return_zero_float_009a7a40(void* object) noexcept;

// ===== Batch 4: engine containers and game virtuals (class E) =====

// --- Engine vectors {data +0h, count +4h, capacity +8h}. The "signed" ones compare count and
// capacity with JL/JGE; the "unsigned" ones with JB/JBE. Element copies test the destination
// for null first, as the binary does. ---

// 0086A280 __thiscall(ECX = vector; int n), RET 4: n raised to 1; when capacity < n, a new
// n*4 block (00BF55BE, no overflow check), the count dwords copied, the old block freed
// (00BF6989, unconditionally), data = block, capacity = n. Signed.
void dword_vector_reserve_0086a280(GameRowsHost& host, void* vector, std::int32_t n);
// 0086AA10 __thiscall(ECX = vector; int n), RET 4: resize to n, new slots zero-filled.
void dword_vector_resize_zero_fill_0086aa10(GameRowsHost& host, void* vector, std::int32_t n);
// 0086B630 __thiscall(ECX = vector), RET: 0086AA10(0), then free the block (00BF6989).
void dword_vector_destroy_0086b630(GameRowsHost& host, void* vector);
// 00872E70 __thiscall(ECX = vector; int index), RET 4: EAX = data + 4*index (no check).
std::uint32_t* dword_vector_at_00872e70(void* vector, std::int32_t index) noexcept;
// push_back(const dword&), __thiscall(ECX = vector; const T*), RET 4: when count == capacity,
// reserve max(2*capacity, 1) through the row's reserve routine; then the value is stored and
// count++. 0086A910 reserves with 0086A220, 0086A950 with 0086A280, 0086AAA0 and 0086AE90
// (byte-identical apart from the address) with 0086A4D0.
void dword_vector_push_back_0086a910(GameRowsHost& host, void* vector, const std::uint32_t* value);
void dword_vector_push_back_0086a950(GameRowsHost& host, void* vector, const std::uint32_t* value);
void particle_resource_pointers_push_back_0086aaa0(GameRowsHost& host, void* vector, const std::uint32_t* value);
void particle_resource_pointers_push_back_0086ae90(GameRowsHost& host, void* vector, const std::uint32_t* value);
// 0086A990: the same over 8-byte elements (two floats, FLD/FSTP), reserving with 0086A350.
void qword_vector_push_back_0086a990(GameRowsHost& host, void* vector, const float* pair);
// 0086AAE0 __thiscall(ECX = vector), RET: when capacity < 0, 0086A350(0); then count = 0.
void qword_vector_clear_0086aae0(GameRowsHost& host, void* vector);
// 0086AED0: 0086AAE0's body, then the block freed (00BF6989).
void qword_vector_destroy_0086aed0(GameRowsHost& host, void* vector);

// Unsigned pointer vector. 00865E90 __thiscall(ECX = vector; n), RET 4: when n > capacity,
// capacity = n first, then a block of n*4 (0xFFFFFFFF on overflow, SETO/NEG/OR); the old
// entries are copied and the old block freed only when data was set; data = block.
void pointer_vector_reserve_00865e90(GameRowsHost& host, void* vector, std::uint32_t n);
// 008661B0 __thiscall(ECX = vector), RET: the same with n = 2*capacity + 2, when that is larger.
void pointer_vector_grow_008661b0(GameRowsHost& host, void* vector);
// 00866690 __thiscall(ECX = vector; value), RET 4: 008661B0's body inline when count ==
// capacity, then data[count++] = value (no null test).
void pointer_vector_push_back_00866690(GameRowsHost& host, void* vector, std::uint32_t value);

// The 14h-byte element vector (signed): element vtable 00CFDC0C at +0h, word +4h, dwords
// +8h/+0Ch/+10h.
inline constexpr std::uint32_t kVector14ElementVtable = 0x00CFDC0C;
// 0072A3B0 __thiscall(ECX = vector; int n), RET 4: n raised to 1; when capacity < n, a new
// n*14h block; each old element copy-constructed into it (vtable, word +4h, +8h..+10h), then
// each old element's slot 0 called with 0, the old block freed (00BF6989), capacity = n.
void vector14_reserve_0072a3b0(GameRowsHost& host, void* vector, std::int32_t n);
// 0072B6D0 __thiscall(ECX = vector; int n), RET 4: reserve when n > capacity; new elements get
// the vtable and +0Ch = +10h = 0; surplus elements, last first, get slot 0 with 0; count = n.
void vector14_resize_0072b6d0(GameRowsHost& host, void* vector, std::int32_t n);
// 0072A6D0 __thiscall(ECX = vector; const 12-byte element*), RET 4 (unsigned): when count ==
// capacity, capacity = 2*capacity + 2 and a new block (no overflow check), elements copied,
// the old block freed when set; then the element is appended.
void vector_c_push_back_0072a6d0(GameRowsHost& host, void* vector, const std::uint32_t* element);

// 0086B290 __thiscall(ECX = object), RET: when the string data at +0Ch is set, it is returned
// to the sized pool with size (+8h) + 1 and flag 1. The fields are not cleared.
void native_string_free_storage_0086b290(GameRowsHost& host, void* object);

// --- Game virtuals over MSVC containers ---

// Point3PtrArray (vtable at +0h; a std::vector<Point3*> at +4h: first +8h, last +0Ch).
// 007B21A0 __thiscall(ECX = array), RET: while the vector is non-empty, free (00BF65AC) the
// last pointer when set and pop it. Seven vtable slots.
void point3_ptr_array_clear_007b21a0(GameRowsHost& host, void* array);
// 007B1B40 __thiscall(ECX = array; index), RET 4: free the point at index (zeroing its slot),
// then erase the slot (memmove_s of the tail) and pop.
void point3_ptr_array_erase_at_007b1b40(GameRowsHost& host, void* array, std::uint32_t index);
// 007B2850 __thiscall(ECX = array; index, const float* point), RET 8: a new point from
// vtable+20h, the three floats copied in, then 007B26F0 inserts it at begin + index.
void point3_ptr_array_insert_at_007b2850(GameRowsHost& host, void* array, std::uint32_t index,
                                         const float* point);
// The last of a std::vector of 12-byte points ({first, last} pair at `first_offset`), copied
// to out with FLD/FSTP. The index is size - 1 (size from (last - first) / 12, 0 when first is
// null) and is checked by an unsigned compare, so an empty vector reports 00BF6713 first.
// 007B0E60 __thiscall(ECX = object; float* out), RET 4: the vector object at [object+8h]
// (first +4h). 007B2B90: the object's own vector, first at +0Ch.
void point_list_last_point_007b0e60(GameRowsHost& host, const void* object, float* out);
void point3_vector_last_point_007b2b90(GameRowsHost& host, const void* object, float* out);
// 006CA5D0 __thiscall(ECX = block; n), RET 4: walk the std::list at block+40h (head [+44h])
// from begin, at most n steps and not past end; EAX = the reached node's +0Ch. Slot +8h of
// 00CF88A0, 00CF8BA0 and 00D015C4.
std::uint32_t air_ops_block_stock_node_for_class_006ca5d0(GameRowsHost& host, const void* block,
                                                         std::int32_t n);

}  // namespace bsp::cc13_gamerows

#endif
