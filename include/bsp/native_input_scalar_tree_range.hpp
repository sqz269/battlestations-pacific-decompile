#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scalar-tree range erasure requires MSVC Win32.
#endif

namespace bsp {
// Whole 005572F0: 200 body bytes / 76 instructions, with one intervening NOP
// outside the typed body. Native ECX=actual tree; EDX is unconsumed; five stack
// words are output, first owner/node, last owner/node. RET14h; EAX=output.
// Actual header: opaque0/head4/count8. Actual 18h node: links0/4/8, opaque
// key/value C/10, color14, nil15. No payload release is introduced.
//
// Both complete native range entries match apart from six proved E8 rel32
// fields, with complete child and owning-error/EH equivalence. This API directly
// reuses the accepted whole 0086EE50 ordinary service, without a provider.
// Returning diagnostics retain the native captured bounds and subsequent raw
// reloads. Partial erasure increments the ACTUAL first argument iterator before
// erasing a separate OLD iterator copy, ignores its private result, then reloads
// first node/owner. Output may alias live raw storage under the callee contract;
// result words are captured before the ordered output writes.
void* __fastcall erase_native_input_scalar_tree_range_005572f0(
    void* tree, void* unused_edx, void* output, void* first_owner, void* first_node,
    void* last_owner, void* last_node);

// Reached raw storage must support the native accesses, including continuation
// after returning diagnostics. Current SDK/CRT diagnostics, current CRT node
// ownership and the accepted owning C++ invalid-iterator transport apply.
// The parent has no local catch or rollback. Source argument/private frames,
// original FH3/RTTI/catch/heap identity, native faults/SEH, volatile register
// residue and game ABI/runtime equivalence are not established. Descriptive
// names are hypotheses; the existing STL identity is retained. The earlier
// admitted full-path prefix and whole 55AF60 APIs keep their original contracts.
} // namespace bsp
