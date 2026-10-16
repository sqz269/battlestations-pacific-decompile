#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This reconstruction requires MSVC targeting Win32.
#endif

namespace bsp {

// Complete ordinary-storage projection of Original _strpbrk, 00BFF1D0..00BFF20F.
// Control bytes through their first NUL are read before any source byte. Source
// needs readable storage only through its first NUL or nonzero control member.
// Reached bytes must be genuine, live, stable and determinate; nonwrapping cursor
// advances stay within or one past their actual allocations. Stable read-only,
// interior and overlapping byte regions are admitted. Native private-frame/home
// alias protocols, faults, MMIO, mutation and lifetime changes are excluded.
// Returns the genuine borrowed first-member source pointer or null, transferring
// no ownership, lifetime or access permissions. No runtime checks are performed.
// This new, default potentially throwing API does not promise physical native ABI.
void* __cdecl find_native_crt_first_member_00bff1d0(
    void* actual_source, const void* actual_control);

} // namespace bsp
