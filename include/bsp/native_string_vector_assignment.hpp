#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Complete00506C80 ordinary Source control flow over an actual10h-byte checked
// vector header: opaque/proxy0, begin4, end8, capacityC. Original
// ECX destination, stack source, RET4/EAX destination. This new Source entry
// takes an explicit actual pooled-string context and has a different ABI.
//
// Self returns before any header read. Empty source clears current live range
// while retaining backing. Shrink assigns then destroys the trailing range;
// grow assigns the prefix then constructs the suffix. Replacement destroys,
// frees CURRENT begin, recomputes CURRENT source count, buys, then constructs.
// Native captured values versus fresh header reads, unsigned comparisons and
// wrapped DWORD/SAR3 arithmetic are retained. Owner+0 is untouched here.
//
// Source/destination/storage may alias where all taken native memory accesses
// remain valid; no host vector or header overlay is constructed. Actual pool
// publications, backing ownership and current-CRT allocation/free must satisfy
// the accepted child-service contracts. No validation, overlap guard, local EH
// cleanup, rollback or pointer-field repair is added. Completed changes remain
// observable when a child throws. Private native frame aliases and original
// stack/FH3/SEH/CRT identity are outside this Source interface; current C++
// exception and existing child diagnostic/zero-copy limits are inherited.
// Evidence: docs/OCT15_STRING_VECTOR_ASSIGNMENT.md.
void* assign_native_string_header_vector_00506c80(void* destination,
    const void* source, NativeStringRawPoolContext& strings);
} // namespace bsp
