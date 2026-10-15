#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD fill ranges require MSVC Win32.
#endif

namespace bsp {

// Complete0043F9E0..0043F9FD: cdecl(first,last,value_address), RET;
// EAX captured last, including an empty range. Capture the value ADDRESS only
// on nonempty entry; reread its current four bytes before every destination
// store. Advance by DWORD4 until equality with the captured last address.
void* __cdecl fill_native_dword_range_0043f9e0(void* first,
    const void* last, const void* value);

// Complete00441180..004411AD: stdcall(first,unsigned_count,value_address),
// RET0Ch; incoming ECX/EDX unconsumed. Count0 does not consume value_address.
// Each reached iteration rereads the current source DWORD, stores, decrements
// count, then advances by4. EAX independently wraps first+(count<<2).
void* __stdcall fill_native_dword_count_00441180(void* first,
    std::uint32_t count, const void* value);

// Actual raw4B accesses and uint32 address/count arithmetic; no value snapshot,
// null skip, range validation, overlap repair, ownership or allocation model.
// All reached storage must be admitted, including a reachable equality bound
// for the range form. Source/value overlap retains progressive reads/writes.
// Private argument/frame aliases, asynchronous mutation, native fault/SEH and
// register-spill identity are outside these new Source interfaces. Preserving
// calling-convention keywords and stable EAX values is not binaryABI/gameplay
// proof. Existing projections/fixtures and correct STL names remain separate.
} // namespace bsp
