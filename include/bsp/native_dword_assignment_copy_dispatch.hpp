#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native DWORD assignment copy dispatch requires MSVC Win32.
#endif

namespace bsp {

// Complete STL_TagDispatch_006974b0,37 native bytes/14 instructions. Original
// ECX owner, incoming EDX unconsumed; three stacked words(first,last,destination),
// RET0Ch; EAX propagates the actual advanced output from 00696D40. The entire
// native body matches accepted 00697860 after relative-call encoding changes.
// Forward the same five physical inputs to that complete Source dispatcher.
// Its core stack words are destination,owner,duplicate destination bits and
// dead(owner bits with low byte0) tag. No count or pointer-type conversion.
//
// Actual progressive rawDWORD copy captures last and destination,including an
// empty range;before each nonnull output store it rereads the current source
// DWORD. Null output skips both accesses;both cursors still advance four with
// DWORD wrap. Return the core output,not an independently calculated distance.
// No owner/header/allocator/iterator object,callback,guard,snapshot or cleanup.
//
// The accepted child supplies volatile rawDWORD MSVC Win32 Source behavior.
// This new interface does not reproduce original private-stack aliases,dead
// register/frame state,CRT/fault/SEH identity,installed binary ABI or gameplay.
// Native STL identity and earlier Source/fragments keep their own provenance.
void* __fastcall copy_native_dword_assignment_uninitialized_dispatch_006974b0(
    void* owner, std::uint32_t unused_edx, const void* first,
    const void* last, void* destination);

} // namespace bsp
