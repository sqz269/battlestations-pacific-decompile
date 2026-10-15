#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual input bit/sensitivity-map cleanup requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// ONLY admitted full-path prefixes00559280..005592FE and
//00559A90..00559B0E (127B/49 instructions each). Native ECX tree,
// EDX unconsumed; stack output,first_owner,first_node,last_owner,last_node;
// RET14h/EAXoutput. Source retains these words and adds actual raw context.
// Ordinary partial erasers and their diagnostic branches are not supplied.
//
// Preconditions: nonnull actual owned12B tree/head; first_owner and last_owner
// both equal tree; first_node equals the initial head.left, last_node equals
// the fresh head; stable exclusive storage through selection. Reached actual
// nodes are2Ch/nil29h with distinct bit/sensitivity payloads and pooled keys.
// The bit leaf captures left, clears count14, then reads backing1C; sensitivity
// captures backing1C BEFORE left and preserves14/18. Both retain actual keys.
// Raw TEST/CMP/load order is retained; full-selection outcomes are admitted
// preconditions, with no replacement bad-path guard/diagnostic/abort/stub.
// Accepted actual subtrees then CURRENT-head resets and ordered output stores
// follow. Output is a writable8B iterator cell, private in the whole consumers.
// Its result node is captured before owner then node publication.
void* __fastcall clear_native_input_bit_vector_tree_full_prefix_00559280(
    void* tree, std::uint32_t unused_edx, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringRawPoolContext& strings);
void* __fastcall clear_native_input_sensitivity_tree_full_prefix_00559a90(
    void* tree, std::uint32_t unused_edx, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringRawPoolContext& strings);

// Complete0055A430..0055A463 and0055AAF0..0055AB23 (52B/23 instructions
// each). Native ECX actual12B header, EDX unused, no stacked inputs, plain
// RET/defined EAX0. Source retains dead EDX and adds actual raw context.
// Capture head then head.left; owner/header arguments select the above full
// prefixes before any subtree/getter. Private8B output; free CURRENT head,
// zero+4 then+8, preserve opaque+0, returnDWORD0. No null/repeated-destroy guard.
std::uint32_t __fastcall destroy_native_input_bit_vector_map_0055a430(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);
std::uint32_t __fastcall destroy_native_input_sensitivity_map_0055aaf0(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);

// Raw pool getters and current CRT frees are the accepted actual services.
// Failures retain earlier subtree effects, stop later reset/free/clear work,
// and propagate through current C++; no local EH/catch/rollback/retry.
// No NativeStringStorage noexcept bridge, callback, fabricated allocator or
// host tree model. Existing real AV/full+partial projection domains remain.
// Added context, by-value inputs/private cells and dead-EDX normalization do
// not establish original stack/register/flag/privateframe/heap/CRT/FH3/SEH/
// fault/exception identity, binary ABI, runtime or gameplay equivalence.
} // namespace bsp
