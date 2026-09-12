#pragma once

namespace bsp {

// Complete B19C30[55]: no consumed input, EAX=raw 1Ch-byte allocation, RET.
// Directly uses the existing actual CRT allocation/new-handler service with
// native_bytes=host_bytes=1Ch. The original 32-bit LEA/TEST/store sequence is
// retained, including the null/+4/+8 branches; null is not a successful result.
// Initializes links at +0/+4/+8 and bytes +18/+19 only. Payload +0C..17 and
// padding +1A..1B remain unwritten. No string pool or private domain is created.
void* __cdecl allocate_native_resource_registry_sentinel_00b19c30();

// Complete B1AA70[104] through a new source exception boundary. Native entry
// was ECX=raw 10h-byte owner, EAX=same owner, RET with its own FH3 frame. This
// source fastcall adds EDX as a stable reference to the actual mutable F8D41C
// publication cell. Neither owner nor publication-reference binding may be
// replaced through aliases to the source function's private stack storage.
//
// Arm constructor cleanup before entering the raw normal body: install
// D5E594, allocate the sentinel, publish owner+8, set nil, reload current head
// before each parent/left/right self-link, then clear count+0C. Owner+4 stays
// untouched. The body preserves original 32-bit memory arithmetic and order.
// On a propagated C++ exception, directly reset actual publication then the
// captured owner's profile through full B19760, and rethrow. No sentinel free
// or owner free is added; successful construction does not publish F8D41C.
//
// Source C++ cleanup does not reproduce native FH3/SEH stack layout, mutable
// native EH spill aliases, arbitrary hardware-fault cleanup or original
// provider volatile-register/throw identity. Original callers without the
// added EDX binding, drop-in ABI compatibility and game behavior are unproved.
void* __fastcall construct_native_resource_registry_00b1aa70(
    void* actual_owner, void* volatile& actual_publication_00f8d41c);

// Complete B1B6E0[18], with the same added EDX reference binding. Capture owner
// in ESI, call the full base source provider, then write D5E59C only after a
// normal return. Returns captured owner in EAX with RET; no outer cleanup.
void* __fastcall construct_native_resource_registry_00b1b6e0(
    void* actual_owner, void* volatile& actual_publication_00f8d41c);

} // namespace bsp
