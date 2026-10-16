#pragma once

#include <cstdint>

namespace bsp {
// Complete ordinary 0069D3F0 storage effects through the accepted raw
// 00554F10 service. Native ECX is an opaque tree word, forwarded without
// dereference; EDX is unconsumed; one stacked node is callee-cleaned (RET4).
// Source keeps that MSVC Win32 input shape and defines the dead EDX slot as
// zero. There is no semantic EAX result.
//
// Reached nodes need readable left +0, right +8 and nil byte +15h. Any
// nonzero nil byte stops traversal without freeing the sentinel. No null
// guard exists. Nonnil nodes must form a finite, uniquely owned tree in the
// singleton_lifetime_free current-CRT domain. Right recursion precedes the
// CURRENT left read; left is captured before current-node free and tested
// for nil after that free. No payload, link, header or count is written.
//
// Source delegates the whole accepted service, whose recursive call origin
// is 00554F10 rather than the Native 0069D3F0 self-call. Private frames,
// ambient EDX/EAX, original CRT heap, FH3/SEH/fault/stack-exhaustion behavior,
// binary compatibility and game execution are not established. No Source
// catch, NativeStringStorage, callback, tree overlay or context is added.
void __fastcall destroy_native_input_default_scalar_subtree_0069d3f0(
    void* opaque_tree, std::uint32_t unused_edx, void* node);
} // namespace bsp
