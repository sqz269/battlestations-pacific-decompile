#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This reconstruction requires MSVC targeting Win32.
#endif

namespace bsp {

// Complete ordinary-storage projection of Original _strspn, 00BF9840..00BF9885.
// Control bytes through their first NUL are read before any source byte. Source
// needs readable storage only through its first NUL or nonzero nonmember.
// Reached bytes must be genuine, live, stable and determinate; nonwrapping cursor
// advances stay within or one past their actual allocations. Stable read-only,
// interior and overlapping byte regions are admitted. Native private-frame/home
// alias protocols, faults, MMIO, mutation and lifetime changes are excluded.
// Returns the exact unsigned count of preceding nonzero control members.
// No runtime checks. This new, default potentially throwing API does not promise
// physical Original ABI or instruction/fault ordering.
unsigned int __cdecl count_native_crt_prefix_in_set_00bf9840(
    const void* actual_source, const void* actual_control);

} // namespace bsp
