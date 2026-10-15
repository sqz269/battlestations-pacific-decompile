#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native dynamics release queue requires MSVC Win32.
#endif

namespace bsp {
// Complete 00C34F70..00C35001 queue append. Actual manager bytes +438h hold
// the DWORD-array pointer, +43Ch current count, +440h capacity. On equality,
// publish capacity*2+2 BEFORE allocating wrapped capacity*4 bytes, copy words
// with current count/pointer reloads, free old storage, then publish new data.
// Append body to the computed nonzero slot and increment the CURRENT count.
// No body destruction, reference release, queue draining or shape guard.
//
// Source fastcall: ECX actual manager, unused EDX placeholder, one stacked
// DWORD body, RET4. It is a new entry over actual storage, not a binary/world
// binding. The naked kernel preserves the late body load from its current
// writable input stack cell, including changes made before that native use.
// Supply writable manager extent through +444h, valid reached word spans,
// and source-CRT-compatible old storage. Arithmetic wraps as DWORDs. Existing
// allocator/new-handler/failure policy is used; published capacity is retained
// if allocation throws. No local rollback is added. Original CRT/SEH identity
// and gameplay equivalence are not claimed.
// Evidence: docs/GOAL15C_DYN_RELEASE_QUEUE.md.
void __fastcall queue_native_dyn_body_release_00c34f70(
    void* actual_manager, void* unused_source_edx, std::uint32_t body);
} // namespace bsp
