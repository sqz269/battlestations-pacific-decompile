#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT substring search requires MSVC Win32.
#endif

namespace bsp {

// New ordinary interface for the whole Original _strstr at 00BF9440.
// Reached raw BYTE/DWORD windows must be stable, live and readable. Every
// reached cursor/effective-address step, including terminal private steps,
// must stay nonwrapping within or one past its actual allocation. No unused
// terminator extent, signed length cap, alignment or nonoverlap is imposed.
// Empty needle returns the supplied haystack without reading its payload.
// A one-byte needle uses the unchanged ordinary 00BF86F0 provider and its
// full-word read/reread domain. Results borrow storage; no access permission,
// ownership or lifetime is transferred. Private-frame/home/backing aliases,
// faults, MMIO, concurrent mutation and lifetime/mapping protocols are outside
// the contract. This is not the Original physical ABI or game validation.
// Full ordered trace, domain and compiler qualifications: NATIVE_CRT_STRSTR.md.
void* __cdecl find_native_crt_substring_00bf9440(
    const void* actual_haystack, const void* actual_needle);

} // namespace bsp
