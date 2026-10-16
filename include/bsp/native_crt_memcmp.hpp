#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT region comparison requires MSVC Win32.
#endif

namespace bsp {

// Whole ordinary-domain reconstruction of _memcmp at [00C1EF55,00C20553).
// This new typed interface does not establish Original physical ABI identity.
//
// Count zero admits any pointer values, including null, and performs no payload
// read or endpoint arithmetic. For a positive count, each pointer designates a
// genuine live allocation containing all count requested bytes. Each requested
// endpoint is WITHIN OR ONE PAST that same allocation: prefixes and interior
// subranges are admitted. Bytes are readable, determinate and stable, and all
// requested address additions/advances are nonwrapping in the 32-bit domain.
// Identical and overlapping regions are allowed; no signed count cap applies.
//
// Return exactly -1, 0 or +1 from the first differing unsigned byte. NUL is data.
// Preserve the reviewed mixed-width and byte-read schedule, including unaligned
// DWORD/WORD loads and cached low-byte fallbacks. No payload writes or validation.
// Ordinary flat Win32 memory only; no concurrency, volatile/MMIO synchronization,
// lifetime/fault/guard-page observation or Original register/stack/flags/segment/
// instruction/EH/runtime/game compatibility is supplied by this interface.
// Evidence and remaining validation: docs/NATIVE_CRT_MEMCMP.md.
std::int32_t __cdecl compare_native_regions_00c1ef55(
    const void* first, const void* second, std::uint32_t count) noexcept;

} // namespace bsp
