#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Actual16B inner checked headers: opaque+0, begin+4,end+8,capacity+C.
// Complete006A3200 including prefix catch and private empty-header cleanup.
// Native ECX first,EDX last,stack(destination,owner,tag3,tag4),RET10h/EAX end.
// A private header has only4/8/C initialized to0. For each nonnull destination,
// construct an empty actual header through557BC0, then swap4/8/C with source.
// Null skips construction but NOT the following field accesses. Completed
// destinations are tidied on failure; prior source transfers are not undone.
// Extra context/private frames qualify original ABI/FH3/SEH identity.
void* __fastcall transfer_native_inner_vectors_to_empty_006a3200(
    void* first, const void* last, void* destination, const void* unused_owner,
    std::uint32_t unused_tag3, std::uint32_t unused_tag4,
    NativeStringRawPoolContext& strings);

// Complete006A4B50 ordinary dispatch: native ECX owner, stack first,last,dest,
// RET0Ch/EAX child result. Source reserves unused EDX and adds context.
void* __fastcall transfer_native_inner_vectors_dispatch_006a4b50(
    const void* owner, std::uint32_t unused_edx, void* first, const void* last,
    void* destination, NativeStringRawPoolContext& strings);

// Complete006A3300 backward raw field swaps. Native cdecl six physical words,
// only first3 consumed, plainRET; EAX advances from dest_end to final dest.
// Expose that stable machine result; caller006A4B80 ignores it. Empty range
// has no field accesses. No copy constructor, assignment or allocator is used.
void* __cdecl swap_native_inner_vectors_backward_006a3300(void* first,
    const void* last, void* destination_end, std::uint32_t unused4,
    std::uint32_t unused5, std::uint32_t unused6);

// Complete006A4B80: cdecl first,last,dest_end, plainRET; ignores child EAX
// and returns dest_end-SAR32(last-first,4)*16 with native DWORD wrapping.
void* __cdecl swap_native_inner_vectors_backward_dispatch_006a4b80(
    void* first, const void* last, void* destination_end);

// Complete006A43A0 fill core and006A43FB completed-prefix catch. Native
// ECX destination,EDX unsigned count; stack(value,owner,tag3,tag4),RET10h.
// No semantic result: Sourcevoid does not promise native ambient EAX residue.
// Value header ADDRESS is fixed, not snapshotted. Null destination skips the
// constructor but advances and joins the prefix; catch has no null skip.
void __fastcall fill_native_inner_vectors_uninitialized_006a43a0(
    void* destination, std::uint32_t count, const void* value,
    const void* unused_owner, std::uint32_t unused_tag3,
    std::uint32_t unused_tag4, NativeStringRawPoolContext& strings);

// Complete006A5170: ECX owner,stack dest/count/value,RET0Ch. Independent EAX
// result is original_dest+original_count*16; adds context/reserves SourceEDX.
void* __fastcall fill_native_inner_vectors_dispatch_006a5170(
    const void* owner, std::uint32_t unused_edx, void* destination,
    std::uint32_t count, const void* value, NativeStringRawPoolContext& strings);

// All pointer arithmetic wraps in DWORD space. Ordered raw field accesses,
// opaque+0 and partial transfers are preserved; no guard/snapshot/self shortcut
// or rollback is added. Taken storage, child ownership/pool/currentCRT contracts
// and reachable endpoints must be admitted; private-frame aliases excluded.
// Current C++ catches/cleanup and explicit contexts do not establish native
// private-stack/FH3/SEH/RTTI/fault/cleanup-failure ABI or live game behavior.
} // namespace bsp
