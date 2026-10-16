#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual default scalar tree range erasure requires MSVC Win32.
#endif

namespace bsp {
// Whole 0069FE70: 200 defined bytes / 76 instructions in two ranges;
// the one intervening NOP at 0069FEEF is outside the typed body. Native
// ECX=actual tree, EDX unconsumed; stack output, first owner/node, last
// owner/node; RET14h; EAX=output. These are physical inputs, not a recovered
// original C++ declaration. Source forwards the same seven formal arguments
// to the accepted whole 005572F0 ordinary service, including dead EDX.
//
// Keep the initial head.left captured across the first returning diagnostic,
// and CURRENT head captured BEFORE the second diagnostic. Full-range reset
// reloads the current head in native order. Partial erasure increments the
// ACTUAL Source first-argument iterator before erasing a separate OLD owner/
// node copy, ignores that child's private result, then reloads first node and
// owner. Result words are captured before ordered output owner/node writes.
// External raw output aliases follow the accepted service contract.
//
// Actual raw header: opaque0/head4/count8; 18h node links0/4/8, unowned
// payloadC/10, color14/nil15. No payload release, pool/context, callback,
// overlay, revalidation, guard, rollback or local catch is introduced.
void* __fastcall erase_native_input_default_scalar_tree_range_0069fe70(
    void* tree, void* unused_edx, void* output, void* first_owner, void* first_node,
    void* last_owner, void* last_node);

// Reached raw storage must support the accesses, including continuation after
// returning diagnostics. The actual current CRT node-ownership/SDK diagnostic
// and owning C++ invalid-iterator transport domains apply. Source call/argument
// frames and child/recursive origins differ; original private spill aliases,
// volatile residue, FH3/RTTI/SEH/fault/stack-exhaustion/heap/binary ABI and game
// equivalence remain unproved. The old AV fragment and all narrower interfaces
// retain their contracts; this API does not promote any caller or 006AA460.
} // namespace bsp
