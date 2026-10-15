#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD vector copy primitives require MSVC Win32.
#endif

namespace bsp {
// Whole00555000,38B/15instructions. Native ECX first,EDX last; FOUR stacked
// words are destination,unused owner,unused metadata,unused tag; RET10h and
// EAX is the advanced destination,including an empty input range. Full native
// bytes and physical ABI equal the accepted actual00696D40. Source directly
// reuses that service and forwards all six words without a count conversion.
// Capture destination even empty. Until first==last,a nonnull destination
// reads the CURRENT source DWORD before its output store;null output skips
// both accesses. Both cursors advance4 with DWORD wrap;no overlap snapshot.
void* __fastcall copy_native_dword_uninitialized_00555000(
    const void* first, const void* last, void* destination,
    void* unused_owner, std::uint32_t unused_metadata,
    std::uint32_t unused_tag);

// Whole004CB130,42B/13instructions. Native ECX actual16B storage;incomingEDX
// unused;no stacked inputs;plainRET,no semantic EAX result. Complete schedule
// equals accepted00468840,with only a proven same-target free rel32 difference.
// Capture DWORD+4 ONCE;conditionally free that captured pointer;then always
// zero+4,+8,+C in order. No direct+0 access,element cleanup,clear or validation.
void __fastcall tidy_native_dword_vector_004cb130(void* actual_header);

// Accepted raw DWORD and current CRT services are reused without providers,
// callbacks,object overlays,allocator models,extra guards or rollback. Every
// reached raw access/release must remain valid;finite copy equality termination
// and writable header storage through post-free clears are required. Public
// progressive aliases and captured-pointer/reset order remain unchanged.
// Source private arguments/frames,ambient registers/flags,original fault/SEH/
// FH3/RTTI/CRT/heap identity,binary ABI and game/runtime equivalence are unproved.
// No noexcept promise is added. Original library names and older qualified
// interfaces remain separate;new descriptive Source names are hypotheses.
// These leaves do not complete00557590 or either placement-wrapper entry.
} // namespace bsp
