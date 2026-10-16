#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual default scalar tree clear requires MSVC Win32.
#endif

namespace bsp {

// Whole0069DC70..0069DC9A:43B/16 starts. Native ECX is actual12B tree,
// EDX unconsumed, no stacked inputs, plain RET; incidental EAX is excluded
// from Sourcevoid. The explicit ignored EDX slot retains the consumed-input
// shape, not original C++ mangling/private frame/register/binary ABI identity.
void __fastcall clear_native_input_default_scalar_tree_0069dc70(
    void* actual_tree, std::uint32_t unused_edx);

// Read CURRENT tree+4 head, then head+4 root. Invoke actual accepted69D3F0
// subtree service with the tree forwarded unchanged and canonical dead EDX0.
// Only after success reload CURRENT head and set its root+4 to itself.
// Capture the SECOND postchild head BEFORE treecount+8=0, then store that
// captured head's left+0=self. Capture CURRENT head a THIRD time and reset
// its right+8=self. No shared/snapshotted head or reordered reset stores.
//
// Reached raw tree/sentinel storage must stay accessible/writable across
// actual subtree destruction and resets. The actual nil15 subtree/free domain
// applies; no null guard, node overlay, callback, header/count bridge, new
// cleanup, rollback or local EH. A child failure precedes every reset.
// Public captures and aliases follow Native ordering; Source private-frame
// aliases/asynchronous mutation, original CRT/FH3/SEH/fault/stack exhaustion,
// ambient registers/flags, drop-in ABI and game behavior remain unproved.
// Correct library identities and earlier Source/projection domains survive.

} // namespace bsp
