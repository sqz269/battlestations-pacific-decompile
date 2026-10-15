#pragma once

#include "bsp/native_scene_property_insertion.hpp"

#include <cstdint>

namespace bsp {

// Two whole Original bodies through new ordinary context-bearing C++ APIs.
// Names are descriptive hypotheses. These interfaces supply no Original
// ECX/RET/register/EFLAGS/FS/SEH/EH/class transport or historical private CRT.
//
// Require a genuine raw61-produced live114h destination bag/map+4, the SAME
// retained concrete canonical string/physical processes in context, completed
// explicit CC8A30 startup, E175B0 nodes/common E188B4 list, actual raw keys and
// E177E4/E17654 fallback cells. Keep owners, membership, generation and gates
// live; retire records/children/keys/nodes before pool teardown. No new owner,
// pool, callback, startup or allocator substitute is supplied here.
// Keys are stable nonnull NUL-terminated closed ASCII under current C locale.
// All roots, key/query storage, map/nodes, active frames and replaced/incoming
// owning graphs meet insertion's disjointness and valid nonwrapping ranges.
// Graphs are finite, unique, unshared and acyclic; serialize mutation, with no
// concurrent trim, alias, reentry, fault, overflow, allocation failure or throw.
// Literal phase words are Original DATA, never callable Source vtables.
//
// Whole [008F3AC0,008F3B56),150B/47 instructions. Original ECX=bag; stack
// key, target, full word18, full word08; RET10; insertion's EAX bag survives.
// Allocate a fresh uninitialized canonical current-CRT38h root. Physically
// store DWORD phase00CE89D4 at0, tag5 at4, full word08 at8, full word18 at18;
// call the actual admitted ECX/RET0 duplicate00438E40 with nullable target;
// store its full pointer at1C; DWORDzero at20,24,34; BYTE1 at2C; whole33F0.
// Writes33B/preserves23B: [0C,18),[28,2C),[2D,34), including preimage+C/+30.
// The two words are independent opaque32-bit values (DWORDs), not
// filtered enums, narrowed values, dereferenced pointers or owned objects.
// Target is borrowed nullptr or stable readable NUL-terminated bytes; it need
// not be ASCII. Null duplicate is0 without target allocation; nonnull empty
// text owns one NUL byte. Fresh root/duplicate are distinct owning allocations.
// On success the actual bag owns the root and its duplicate+1C. Type5 release
// frees that duplicate once/clears1C,18 and common4,C,28,8; input stays borrowed.
// Original state0 protects the saved raw root through construction with the
// CA49E0 free action/CA49EB handler/DD76C8 FuncInfo/DD76C0 map; state is reset
// before insertion. This successful-domain API adds no unwind or rollback.
void* add_native_scene_property_reference_008f3ac0(void* actual_bag,
    const char* actual_key, const char* nullable_target,
    std::uint32_t word_18_bits, std::uint32_t word_08_bits,
    NativeScenePropertyInsertionContext& context);

// Whole [008F3B60,008F3BCA),106B/36 instructions. Original ECX=bag; stack
// key, child; RET8; both normal exits preserve insertion's actual bag EAX.
// Allocate a fresh uninitialized canonical current-CRT38h root. Physically
// store DWORD phase0,tag6 at4,full child pointer+C,zero18,1C,20,24,30; store
// full fresh root pointer into actual child+110 BEFORE recordzero34 andzero8;
// BYTE1 at2C; whole33F0. Writes41 root bytes plus4 child bytes; preserves15
// root bytes [10,18),[28,2C),[2D,30). Never substitute raw008EF780: its final
// byte2C/word8 order differs. No zero-fill or coalesced stores are supplied.
// Child is a nonnull genuine canonical owning raw61-produced mutable114h bag,
// from the accepted empty/whole clone producer, with initial backlink+110=0.
// Its entire owning graph is disjoint from destination/replaced graph and the
// fresh root, query/key/node/map/frame. This ADOPTS the actual supplied child;
// it does not clone, copy, detach, retain a separate owner or borrow-only.
// Caller relinquishes ownership on successful publication; accepted type6
// lifetime scalar-deletes child with flags1. Child+110 is non-owning and is
// written before insertion/old mapped retirement. No same-child shortcut,
// old-owner repair, cycle check, added null guard or rollback is invented.
void* add_native_scene_property_subbag_008f3b60(void* actual_bag,
    const char* actual_key, void* actual_owned_child,
    NativeScenePropertyInsertionContext& context);

// Both Original allocation-null branches skip record stores (also duplicate
// or backlink) and still call insertion with0. Preserve that structure, while
// admitting only successful nonnull allocations/providers: whole insertion
// later dereferences its record. A null child with nonnull root is likewise
// outside admission. Source by-value/helper/stack timing differs from Original;
// no failure cleanup, Original ABI/EH/private CRT, startup/parser/World/runtime
// or game equivalence follows from this ordinary Source implementation.

} // namespace bsp
