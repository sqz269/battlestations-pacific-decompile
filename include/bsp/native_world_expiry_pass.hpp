#pragma once

#if defined(_MSC_VER) && defined(_M_IX86)

namespace bsp {

// Qualified actual-pointer reconstruction of 00903610 (93 bytes, 42 instructions).
// ECX is the actual World, retained for the entire pass. Counter +6Ch uses native
// signed gates and wrapping DWORD addition; only the aging branch updates the
// traversal anchor. Child retirement uses the actual recursive 009035E0 helper.
// The entity's then-current virtual slot 0 receives DWORD 1 by an ordinary CALL;
// that target must consume four argument bytes and preserve callee-saved registers.
// After retirement, continuation reads the retained anchor's current +38h or the
// captured World's freshly read +4h header/first pointer, never the retired entity.
// Actual World, header, parent, and anchor lifetimes and coherent hierarchy progress
// remain caller contracts. No typed result, ownership, unlink/free policy, exception
// recovery, application binding, or runtime compatibility is established.
void __fastcall release_native_world_expired_objects_00903610(void* actual_world);

} // namespace bsp

#endif
