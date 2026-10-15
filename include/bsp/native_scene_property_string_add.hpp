#pragma once

#include "bsp/native_scene_property_insertion.hpp"

namespace bsp {

// Whole Original [008F38A0,008F3931), 145B/46 instructions, through a new
// context-bearing C++ interface. The name is descriptive, not recovered.
// Original ECX=actual114h bag; stack key/nullable CString; RET8; EAX=bag.
// Allocate genuine fresh uninitialized38h bytes in the canonical current heap.
// Store DWORD phaseCE89D4,+4 tag2; zero DWORD +18/+1C/+20/+24/+30 in order;
// call the genuine raw ECX/RET0 nullable00438E40 duplicate; store its full
// result at+C BEFORE zero+34; finish with BYTE1+2C. Writes37B/preserves19B
// at[08,0C),[10,18),[28,2C),[2D,30). No raw008EF1B0 substitution: it clears
// +34 before duplicate+C. Phase DATA is never a callable Source vtable.
//
// Value is borrowed and nullable. Null produces null+C without a copy;
// nonnull empty text produces an owning one-byte NUL copy. Nonnull input must
// remain stable/readable through NUL with representable length+1/no wrap.
// Successful whole33F0 publication transfers the distinct fresh root and its
// distinct duplicated child to the actual bag/map. Do not release them after
// publication; existing genuine type2 recursive/scalar lifetime does that.
//
// Require a genuine raw61-produced live114h bag/actual108h map at+4 and the
// SAME retained canonical string/physical processes in the insertion context.
// Complete explicit E175B0 startup before constructing it. Retain actual
// E175B0 generation/commonE188B4 list, raw keys/manager/return gate, distinct
// E177E4/E17654 cells and allocator+10 membership. Keys are stable nonnull
// closed ASCII/current-C-locale C strings. Root/payload/frame/key/query/map/
// node/replaced ownership are disjoint; borrowed inputs do not alias owners
// retired during insertion. Existing owning graphs are genuine, finite,
// unique, unshared and acyclic. Externally serialize shared-list mutations;
// retire records/payloads/keys/nodes before pool teardown. This starts nothing.
//
// Null allocation still syntactically reaches insertion. Ordinary admission
// requires successful nonthrowing services and stable valid/disjoint memory;
// no OOM/fault/overflow/alias/reentry/concurrent trim/mutation is admitted.
// The private physical helper calls are Source glue under that domain.
// Original saved-root state0 cleanup CA49AB/DD7670/DD7668/CA49A0 is evidence,
// not supplied FS/EH transport. No added cleanup/rollback/transaction/default,
// Original ECX/RET8/register/EFLAGS/class ABI, privateCRT/failure parity,
// startup/global binding, wholeparser, native runtime or game equivalence.
void* add_native_scene_property_string_008f38a0(void* actual_bag,
    const char* actual_key, const char* nullable_value,
    NativeScenePropertyInsertionContext& context);

} // namespace bsp
