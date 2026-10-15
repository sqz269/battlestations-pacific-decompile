#pragma once

#include <cstdint>

namespace bsp {

// Complete STL_UninitCopy_00696d40 over actual four-byte scalar storage.
// Original ECX=first, EDX=last; four stacked DWORDs are destination, owner,
// opaque metadata, tag; RET10h. The dispatcher duplicates destination in the
// metadata word; direct insertion calls instead pass their requested count.
// Only destination is consumed, and its
// stack slot is read even for an empty source range. EAX is the advanced
// destination. Capture last, then until first==last: if destination is
// nonzero, read the CURRENT source DWORD immediately before its output store;
// otherwise skip both accesses. Both cursors advance four with DWORD wrap.
// No element count, memcpy/memmove transform, overlap guard or snapshot.
//
// All physical argument positions are explicit. Dead metadata introduces no
// owning object, allocator, callback or cleanup. Volatile DWORD accesses retain
// progressive aliases on the required MSVC Win32 target. These C++ functions
// do not promise original private-stack aliasing, code generation, ambient
// register state, portable pointer/fault/SEH semantics, binary ABI or gameplay
// equivalence. Earlier qualified storage fragments remain separate.
void* __fastcall copy_native_dword_uninitialized_00696d40(
    const void* first, const void* last, void* destination,
    void* unused_owner, std::uint32_t unused_metadata,
    std::uint32_t unused_tag);

// Complete STL_TagDispatch_00697860. ECX=owner, EDX unconsumed; stacked
// first,last,destination; RET0Ch. Pass destination/owner/duplicate destination
// and dead(owner bits with low byte0) tag to the core, then propagate its EAX.
// Unlike the fill_n dispatcher, there is no independent count-based result.
// Matching physical shapes are not an original binary/private-frame promise.
void* __fastcall copy_native_dword_uninitialized_dispatch_00697860(
    void* owner, std::uint32_t unused_edx, const void* first,
    const void* last, void* destination);

} // namespace bsp
