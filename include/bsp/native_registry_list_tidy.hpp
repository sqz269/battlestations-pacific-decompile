#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native registry list tidy requires MSVC Win32.
#endif

namespace bsp {

// Complete 0041D710..0041D757 schedule on an existing 0Ch list header:
// untouched proxy word +0h, sentinel pointer +4h, count DWORD +8h. The
// observed registry caller supplies registry+4h; no outer class is declared.
// Self-link the sentinel and zero the count, then CRT-free the old node chain
// and the current sentinel. Finally null header+4h. Node payloads are unread.
// Header+4h is reloaded after each external free, rather than cached.
//
// Original input is ECX header, no stack arguments, plain RET. The Source
// function uses MSVC Win32 fastcall with one pointer. This does not establish
// native register/stack equivalence or the original CRT heap identity.
// Supply a valid writable header and sentinel/chain with matching CRT heap
// provenance. Count zero does not skip cleanup; an already-null sentinel is
// not accepted by the native schedule. No allocator or failure policy is added.
// Evidence: docs/GOAL15B_REGISTRY_LIST_TIDY.md.
void __fastcall tidy_native_registry_list_0041d710(void* actual_list_header);

// Complete 0041E290..0041E294 native thunk: JMP 0041D710. This Source wrapper
// delegates the same operation and does not promise emitted tail-call bytes.
void __fastcall tidy_native_registry_list_0041e290(void* actual_list_header);

} // namespace bsp
