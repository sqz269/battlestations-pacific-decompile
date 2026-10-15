#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual outer vector assignment requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete006A6680..006A67C5. Native ECX destination, EDX unconsumed;
// stacked source, RET4/EAX destination. Source retains those physical words
// and adds actual raw-pool context. Actual16B outer owning header contains
// actual16B inner checked-vector headers with8B pooled strings.
// Self returns before field reads. Source begin/end/count and destination
// begin are captured as native; later reads/reloads are retained separately.
// Empty source uses actual6A6310 clear, with returning diagnostic continuations.
// Shrink assigns then destroys suffix before CURRENT source-size publication.
// In-capacity growth assigns the old prefix then constructs the captured
// source suffix at CURRENT destination end. Reallocation destroys/free old
// backing BEFORE recomputing source count and actual557250 initialization;
// test only AL, then capture destination begin/source end/source begin in that
// order and construct through6A4C00. No local guard, catch or rollback.
void* __fastcall assign_native_outer_vector_006a6680(void* destination,
    std::uint32_t unused_edx, const void* source,
    NativeStringRawPoolContext& strings);

// Complete006A6C60..006A6C98. Native cdecl(first,last,destination), RET;
// EAX precomputed destination+(SAR4(DWORD(last-first))<<4). Source adds context.
// Forward16B/equality assignment, no source snapshot or overlap repair.
// Native caller7657 PUSH EDI saves a nonvolatile register; its later POP EDI
// and ADD ESP,0Ch distinguish that spill from a fourth argument.
void* __cdecl copy_native_outer_vector_range_006a6c60(const void* first,
    const void* last, void* destination, NativeStringRawPoolContext& strings);

// Complete006A6CA0..006A6CC5. Native cdecl(first,last,value), RET. Capture
// only value ADDRESS once on nonempty entry; every assignment reads CURRENT
// value fields, including forward aliases. Empty touches no pointed storage.
// Source void excludes native empty ambient EAX/nonempty last-child residue.
void __cdecl fill_native_outer_vector_range_006a6ca0(void* first,
    const void* last, const void* value, NativeStringRawPoolContext& strings);

// Opaque header+0 is not written by these operations. DWORD cursor/count
// arithmetic and exact returning child diagnostics are inherited. No callback,
// host container model, new validation, cleanup retry or allocator substitute.
// Child failures retain earlier effects and skip later publications. Reached
// storage, actual child ownership/pool/current CRT lifetimes and diagnostics
// must be admitted. Private argument/frame aliases are outside the interface.
// Added context/by-value Source inputs and normalized unconsumed child EDX0
// do not prove original stack/register ABI, heap/CRT/handler/FH3/SEH/fault/
// exception identity or gameplay. Existing projections/fixtures stay separate.
} // namespace bsp
