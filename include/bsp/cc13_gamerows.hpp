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

}  // namespace bsp::cc13_gamerows

#endif
