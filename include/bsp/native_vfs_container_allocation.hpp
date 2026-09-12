#pragma once

// Complete allocation leaves shared by VFS and other native containers.
// Descriptive names are hypotheses, not recovered symbols. MSVC Win32 only.
// Original ABI: no consumed incoming register/stack arguments; pointer in EAX;
// plain RET. These return raw storage, not owning C++ container objects.
//
// Uses the existing singleton_lifetime_allocate malloc/new-handler service with
// equal native/host byte counts; release with singleton_lifetime_free. Original
// static CRT handler state and exception identity remain a source CRT boundary.
// Allocation failure normally throws. If an allocator returns null, the native
// first store is skipped but the following +4 store still faults; do not turn
// this into an early null return. All unlisted payload/padding bytes stay intact.

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS container allocation requires MSVC Win32.
#endif

namespace bsp {

// BDA960[26]: allocate 0Ch; +0/+4 each receive the allocation address.
void* __cdecl allocate_native_list_head_00bda960();

// BDA980[26]: allocate 28h; +0/+4 each receive the allocation address.
void* __cdecl allocate_native_list_head_00bda980();

// BDABF0[55]: allocate 24h; zero DWORDs +0/+4/+8, byte +20=1,+21=0.
void* __cdecl allocate_native_tree_node_00bdabf0();

// BDABA0[55]: allocate 20h; zero DWORDs +0/+4/+8, byte +1C=1,+1D=0.
// Shared with BF4D30 physical-directory index construction.
void* __cdecl allocate_native_tree_node_00bdaba0();

// 4C26B0[55]: allocate 18h; zero DWORDs +0/+4/+8, byte +14=1,+15=0.
void* __cdecl allocate_native_tree_node_004c26b0();

// 7F82F0[26]: allocate 0Ch; +0/+4 each receive the allocation address.
void* __cdecl allocate_native_list_head_007f82f0();

// Tree leaves initially produce nonsentinel storage. BE1DC0 separately sets
// the final byte to one and all three links to self when constructing its heads.

} // namespace bsp
