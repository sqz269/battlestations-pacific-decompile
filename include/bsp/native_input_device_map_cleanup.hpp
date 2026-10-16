#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Whole006A7930..006A795A, 43B/16 instructions. Native ECX actual12B tree,
// EDX unused, no stacked inputs, plainRET; EAX is nonsemantic head residue.
// Capture head/root, destroy the actual nil99 subtree, then reload head for
// parent=self, a fresh head BEFORE count=0/left=self, and fresh head/right=self.
// The sentinel remains allocated and opaque+0 is untouched.
void __fastcall clear_native_input_device_tree_006a7930(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);

// ONLY the admitted full-selection prefix006A7AA0..006A7B1E (127B/49
// instructions), not the ordinary range entry or its partial branch. Native
// ECX tree, EDX unused, stacked output/first-owner/first-node/last-owner/
// last-node, RET14h and EAX output. Added context is a Source service input.
// Require valid exclusively owned actual storage, nonnull tree/head,
// first_owner==last_owner==tree, first_node==initial head.left, and
// last_node==fresh head; selection storage stays stable through selection.
// Output is writable8B storage (private in the actual consumers). Preserve
// native TEST/CMP/load order without defining out-of-fragment diagnostics.
// Capture the current result node before storing output owner then node.
void* __fastcall clear_native_input_device_tree_full_prefix_006a7aa0(
    void* tree, std::uint32_t unused_edx, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringRawPoolContext& strings);

// Three whole52B/23-instruction map destructors. Native ECX actual12B tree,
// unusedEDX, no stacked inputs, plainRET and defined EAX0. Capture head and
// head.left; pass owner twice/private8B output to the WHOLE actual200B range;
// free CURRENT head, clear+4 then+8, return0. Opaque+0 is untouched.
// This binding includes returning-diagnostic/partial continuation through the
// published actual range; the old admitted prefix above retains its narrower
// contract. Added RawCtx makes SourceRET4 versus native plainRET.
std::uint32_t __fastcall destroy_native_input_device_map_006a7ba0(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);
std::uint32_t __fastcall destroy_native_input_device_map_006aa3e0(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);
std::uint32_t __fastcall destroy_native_input_device_map_006aa420(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);

// Finite uniquely owned actual nodes/head and matching current-CRT ownership
// are required. No null/empty/repeated-destroy guards, rollback or local EH.
// Actual RawCtx key-getter failure propagates before later resets/free/clears;
// accepted pair cleanup owns its original key unwind, with CurrentCpp
// termination on a secondary C++ cleanup exception. Existing AV APIs remain.
// Extra context/current CRT/private frames do not establish native ABI,
// flags/register residue, original heap/RTTI/FH3/SEH/fault/stack-exhaustion
// identity or game behavior.
} // namespace bsp
