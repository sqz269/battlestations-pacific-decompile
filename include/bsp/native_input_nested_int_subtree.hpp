#pragma once

#include <cstdint>

namespace bsp {
// ONLY the consumed full-path prefix 005572F0..0055736E, not the ordinary
// range eraser. Native ECX tree, EDX unused; stacked output, first_owner,
// first_node, last_owner, last_node; RET14h/EAXoutput. This new fastcall Source
// interface keeps those physical input words, without native ABI validation.
//
// Admitted path: a valid stable owned scalar-tree header, nonnull tree,
// first_owner==last_owner==tree, first_node==initial head.left and
// last_node==current head. The compared header/argument storage must remain
// stable through selection. Raw TEST/CMP and field reads retain their native
// order; their full-path outcomes are PRECONDITIONS. Failed diagnostics and
// partial dispatch 00557370..005573B8 are not implemented or replaced by guards.
// Output is the writable private 8-byte iterator cell consumed by 0055AF60.
// It is written owner then node, after capturing CURRENT post-clear head.left.
// No callback, unrelated storage provider, abort or unsupported-path stub.
void* __fastcall clear_native_input_scalar_tree_full_prefix_005572f0(
    void* tree, std::uint32_t unused_edx, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node);

// Complete ordinary 0055AF60..0055AFC5: ECX opaque outer-tree word captured
// and recursively forwarded, EDX unconsumed, one stacked node, RET4, void.
// Actual node20h: left+0/right+8, nested scalar-tree header+10h (head+14h,
// count+18h), nil+1Dh; inner node18h/nil15h. Opaque +10h is not written.
// Valid stable owned headers supply exactly the full-prefix arguments above.
// Right recursion precedes nested head/left capture, then OUTER left capture.
// Full-prefix cleanup precedes CURRENT inner-head free, zero+14/+18, outer-node
// free and only then the captured outer-left nil test. Any nonzero nil stops.
//
// Reached actual storage/nil sentinels must be readable; nonnil nodes must form
// finite uniquely owned trees with matching singleton_lifetime_free current-CRT
// allocations. No null/topology/alias repair, opaque initialization or provider.
// Source passes zero only in recursively unused EDX. No local FH3/catch exists
// in the owned native bodies; producer EH is separate context, not this scope.
// Source private frames/output placement, register residue, original static-CRT
// heap identity, fault/SEH/stack exhaustion, binary ABI and game are unverified.
void __fastcall destroy_native_input_nested_int_subtree_0055af60(
    void* opaque_tree, std::uint32_t unused_edx, void* node);
} // namespace bsp
