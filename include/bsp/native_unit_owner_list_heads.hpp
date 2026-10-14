#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native unit-owner list head allocation requires MSVC Win32.
#endif

namespace bsp {
// Complete normal 26-byte leaves. Use the existing real BF681B malloc/new-
// handler allocation boundary, requesting 18h and 0Ch bytes respectively.
// Set only DWORD +0 and +4 to the returned address, retaining the Original
// independent wrapped-address guards. Every other byte remains untouched.
//
// Returns actual raw allocated storage. The enclosing real list/owner must
// retain it, tidy real elements and release it in its recovered teardown.
// These are no-argument Source C++ interfaces, not fixed-address replacements;
// Original residual ECX/flags, exceptional ABI and fault equivalence are not
// claimed. No fabricated list header, profile, receiver or default count.
void* buy_native_unit_owner_list_head_0077b940();
void* buy_native_unit_owner_list_head_0077b960();
} // namespace bsp
