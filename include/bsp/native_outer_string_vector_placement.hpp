#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Complete006A6810 normal effects and verified no-op placement unwind.
// Native ECX actual16B outer slot, EDX source outer header; no stacked input,
// plain RET. Source retains those inputs and adds stacked actual context.
// Null slot skips all constructor/source/header/pool work. Otherwise call
// actual0055B150, preserving EDX source bits as its unconsumed register word.
void __fastcall placement_copy_native_outer_string_vector_006a6810(
    void* slot, const void* source, NativeStringRawPoolContext& strings);

// Complete006A6B60 normal effects and its verified no-op placement unwind.
// Original entryECX/EDX unconsumed, stack(slot,source), RET8: callee cleanup,
// correcting the older cdecl description. Source reserves both register words,
// keeps original stack positions and adds context; Source RET0Ch. Forward
// incoming EDX to the actual constructor's ignored register word. Same null rule.
void __fastcall placement_copy_native_outer_string_vector_006a6b60(
    void* unused_ecx, std::uint32_t unused_edx, void* slot,
    const void* source, NativeStringRawPoolContext& strings);

// Actual16B outer header: opaque/proxy+0, begin+4,end+8,capacity+C, containing
// actual16B inner checked headers and actual8B pooled strings. Construction,
// current reads/ownership, +0preimage preservation, no-self-guard effects and
// child cleanup come solely from accepted0055B150. No host vector, callback,
// borrowed uninitialized assignment or additional cleanup/rollback is added.
//
// Native nonnull leaves ctor EAX=slot; null leaves ENTRY FS:[0] chain value,
// not necessarily0. Neither has live callers establishing a semantic result.
// These void Source entries make no register-residue promise or forged null
// result and do not introduce synthetic incoming-EAX/exception-list inputs.
// Native unwindC7F4D0/C7F510 passes slot words to RET-only00401130, no free.
// Added context, private Source frames and C++ exception domain differ from
// native stack/FH3/SEH/CRT/catch/fault/unwind identity; Source firstRET4 differs
// from nativeRET. Requires admitted child storage/pool/currentCRT ownership.
// Older projections/names stay separate; no native/binary/gameplay claim.
} // namespace bsp
