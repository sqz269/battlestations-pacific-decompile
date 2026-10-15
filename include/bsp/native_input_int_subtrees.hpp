#pragma once

#include <cstdint>

namespace bsp {
// Complete ordinary entries, at the accepted current-CRT free boundary.
// Native ECX is an opaque tree word, captured and forwarded but never read;
// EDX is unused, one stacked node, RET4, no semantic return value. Source
// preserves that input shape and forwards zero only in the unused EDX slot.
// No NativeStringStorage, RawPoolContext, callback or tree overlay is required.
//
// Reached nodes must have readable links +0/+8 and nil byte +11h/+15h,
// respectively. ANY nonzero nil byte stops traversal without freeing it.
// No null guard exists. Nonnil nodes must form a finite, uniquely owned tree
// allocated in the singleton_lifetime_free current-CRT domain. Right subtrees
// are destroyed first; left is read AFTER recursion, captured BEFORE current
// node free, and tested for nil AFTER that free. Payload/header/count/links are
// not written. Producer evidence establishes 14h/18h allocations and sentinel
// self-links; untouched bytes have no invented C++ type or ownership service.
//
// These are new MSVC Win32 Source APIs. Original CRT heap identity, register
// residue, private stack frame, instruction-level fault/SEH/stack exhaustion,
// original binary ABI compatibility and game execution remain unverified.
// Native bodies have no local FH3/EH scopes; no Source catch is introduced.
void __fastcall destroy_native_input_int_only_subtree_004c1890(
    void* opaque_tree, std::uint32_t unused_edx, void* node);
void __fastcall destroy_native_input_int_scalar_subtree_00554f10(
    void* opaque_tree, std::uint32_t unused_edx, void* node);
} // namespace bsp
