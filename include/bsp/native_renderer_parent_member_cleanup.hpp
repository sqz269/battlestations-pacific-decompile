#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer parent-member cleanup requires MSVC Win32.
#endif

namespace bsp {
// Actual 0Ch header: data+0, signed count+4, signed capacity+8. Native ECX
// receiver/plain RET. Raw extents and wrapping address arithmetic remain the
// caller's responsibility. The two scalar-array entries do not release pointees.
// 8D4E60 calls the actual pair reserve0 only for negative capacity, lowers
// CURRENT positive count, captures CURRENT data before final count0, then frees
// that captured allocation. Pointer/capacity words remain stale.
void __fastcall destroy_native_renderer_resolution_pairs_008d4e60(void*);

// 86AE00 calls the existing actual 86A430 resize0, then frees CURRENT data.
void __fastcall destroy_native_renderer_dword_array_0086ae00(void*);

// B2F690 receives renderer+1B18. Resize/free nested headers at+50 while state0
// arms B29E40 cleanup of the DWORD header at+44. Disarm before normal +44
// resize0/free. Source cleanup exceptions during unwinding terminate; original
// FH3/private frame, asynchronous SEH and arbitrary stack aliases are unproved.
void __fastcall destroy_native_renderer_capabilities_00b2f690(void*);
void __fastcall destroy_native_renderer_capabilities_thunk_00b2f700(void*);

// Actual embedded1Ch tracked Win32 section, not an allocated section owner.
// Decrement CURRENT signed-positive depth+18 before each LeaveCriticalSection,
// then DeleteCriticalSection. Never free the storage or clear a negative depth.
// Caller must own the corresponding recursive acquisitions on this thread.
void __fastcall destroy_native_embedded_tracked_section_00402f70(void*);
} // namespace bsp
