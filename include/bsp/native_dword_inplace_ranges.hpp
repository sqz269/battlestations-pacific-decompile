#pragma once
#include <cstdint>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD in-place ranges require MSVC Win32.
#endif

namespace bsp {
// Complete006970B0..006970CD: cdecl(first,last,value_address), plain RET.
// ECX/EDX are unconsumed inputs; native EAX finishes at captured last (callers
// discard it). Source returns that observed address. Empty skips value-address
// capture/pointed access; otherwise capture ADDRESS once and reread its CURRENT
// DWORD before each store, advancing4 until captured last. No null skip.
void* __cdecl fill_native_dword_inplace_range_006970b0(void* first,
    const void* last, const void* value);

// Complete00697440..00697474: cdecl(first,last,destination_end), plain RET;
// ECX/EDX unconsumed. EAX is computed BEFORE any copy as
// destination_end - ((SAR2(last-first))*4), with DWORD wrap. For nonempty input,
// capture displacement=destination_end-last; read CURRENT last-4, decrement
// last, store at displacement+last, then repeat until captured first.
void* __cdecl copy_backward_native_dword_range_00697440(const void* first,
    const void* last, void* destination_end);

// Complete ordinary scalar schedules, not a memmove/std::fill/container model.
// Raw4B accesses and explicit x86 SAR2 retain progressive overlaps and unsigned
// address wrap. Reached storage must support all native reads/writes and finite
// equality termination. No extra guard, callback, snapshot, rollback, allocation
// or local EH. Source/private argument/frame aliases, asynchronous mutation,
// flags/register residue, native fault/SEH/binary ABI and game remain unverified.
// Established STL names and older qualified interfaces/fixtures stay separate.
} // namespace bsp
