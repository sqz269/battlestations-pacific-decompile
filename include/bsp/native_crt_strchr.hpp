#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Original CRT byte/word character search requires MSVC Win32.
#endif

namespace bsp {

// Whole195B/89op discontiguous Original00BF86F0 algorithm,including its5B
// pre-entry match tail. Native cdecl(pointer,int),plainRET/rawEAX. This new
// ordinary interface binds actual stacked argument words instead: read the
// value BYTE at+4 FIRST,then capture pointer DWORD at+0. No class/owner is added.
//
// Preserve BYTE alignment reads,DWORD aligned reads,candidate DWORD reread,
// original masks/sign compensation and match-before-NUL. All address/arithmetic
// operations wrap32. No SDK search,byte-loop replacement,callback,locale/TLS,
// allocation,extra null/count guard or error recovery supplies this body.
// Binding addresses are stable;Source metadata is disjoint from reached cells
// and payload. Payload aliases remain admitted when every reached BYTE/DWORD
// access is valid,including whole aligned words beyond a terminator. No implicit
// compiler/native/helper/argument-home,return,spill,flags/register/fault or
// Original binary ABI identity is inferred. Result bits are borrowed storage;
// no ownership/lifetime or game validation is claimed.
// Evidence: docs/GOAL_OCT15_CRT_STRCHR.md.
void* find_native_crt_character_00bf86f0(const void* actual_parameters8);

} // namespace bsp
