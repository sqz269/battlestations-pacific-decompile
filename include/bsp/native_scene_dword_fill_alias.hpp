#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error The actual DWORD fill alias requires MSVC Win32.
#endif

namespace bsp {

// Complete 00468440..0046845D: cdecl(first,last,value_address), plain RET,
// raw EAX captured last. Native captures first/last before comparing them; an
// empty range consumes neither stacked value_address nor any source DWORD.
// Nonempty captures the value ADDRESS once, rereads its current raw DWORD before
// every store, then advances by4 modulo32 until equality with captured last.
//
// Reuse the body-identical accepted 0043F9E0 ordinary Source and its raw memcpy4
// storage domain. No alignment/order/count guard, snapshot, null skip, overlap
// repair, allocator or callback is added. Actual reached accesses must be valid;
// the canonical range admission includes a reachable equality bound. Source/value
// overlap retains progressive reads/writes. Private Source/native argument homes,
// frames and asynchronous mutation remain outside this ordinary interface.
// Source delegation is not a native CALL claim: 00468440 has no outgoing call.
// Calling-convention keywords/raw result do not establish original binary ABI,
// instruction/fault/SEH identity or game behavior.
void* __cdecl fill_native_dword_range_00468440(void* first,
    const void* last, const void* value_address);

} // namespace bsp
