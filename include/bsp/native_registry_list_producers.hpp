#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native registry list producers require MSVC Win32.
#endif

namespace bsp {

struct Cc13TerrainGridLayerListNode;
struct Cc13TerrainGridLayerNative;

// Complete 0041BFD0: allocate raw 0Ch bytes, write next/previous self-links,
// leave payload+8h uninitialized. Native has no arguments or receiver, returns
// the allocation in EAX, and uses plain RET. Reuses the existing node schema.
Cc13TerrainGridLayerListNode* __cdecl buy_native_registry_list_head_0041bfd0();

// Complete 0041C1F0: allocate raw 0Ch bytes; write next, previous, then capture
// *value_address and write the payload. Do not snapshot *value_address before
// allocation or before the link writes: allocator-side changes and aliases to
// those bytes must be observed. The slot must be readable at its native use.
// Native incoming ECX is unused; three stack arguments, EAX node, RET 0Ch.
Cc13TerrainGridLayerListNode* __stdcall buy_native_registry_list_node_0041c1f0(
    Cc13TerrainGridLayerListNode* next,
    Cc13TerrainGridLayerListNode* previous,
    Cc13TerrainGridLayerNative* const* value_address);

// Complete 0041E270: allocate the head first, then write pointer header+4h and
// count DWORD header+8h, and return the same header. Proxy header+0h untouched.
// Operates on existing writable 0Ch header bytes; no outer object constructed.
// Native ECX header, EAX same header, no stack arguments, plain RET.
void* __fastcall construct_native_registry_list_0041e270(void* actual_list_header);

// All entries use the genuine singleton_lifetime_allocate object request with
// native/host size 0Ch/0Ch. Its current CRT retry/new-handler/bad_alloc policy
// is reused; no new allocation or failure policy, payload constructor, or outer
// owner lifetime is invented. Qualified Source CRT identity and calling shapes
// do not establish native binary/stack-frame/exception ABI equivalence.
// Evidence: docs/GOAL15B_REGISTRY_LIST_PRODUCERS.md.

} // namespace bsp
