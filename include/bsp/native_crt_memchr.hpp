#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT counted byte search requires MSVC Win32.
#endif

namespace bsp {

// Whole ordinary-domain Source reconstruction of original _memchr at
// 00BFA700..00BFA7AC (173 bytes, 76 instructions). This is a new typed API.
// Count zero accepts any pointer, including null, without reading payload.
// Otherwise the requested bytes must be determinate, readable, stable storage
// in one genuine live allocation. The endpoint may be inside that allocation
// or one past it; the 32-bit extent and advances must not wrap.
// Every int selects its low byte. No NUL, padding, initial alignment or typed
// DWORD allocation is required. Read-only stable aliases are admitted.
// The returned first-match pointer borrows that allocation's lifetime.
//
// BYTE peel/tail and complete aligned DWORD reads follow the Original order;
// a candidate word is reread once before ordered register-lane comparisons.
// A word may read later requested bytes beyond its first matching byte.
// No writes, search helper, provider, global or caller binding is introduced.
// Invalid positive-count storage, faults, mutation, concurrency and volatile/
// MMIO observation protocols are outside this ordinary domain. This is not
// portable ISO C++ or Original physical ABI/flags/register/stack/runtime proof.
// Evidence and remaining integration checks: docs/NATIVE_CRT_MEMCHR.md.
void* __cdecl find_native_crt_byte_00bfa700(
    void* actual_buffer, int value, unsigned int count);

} // namespace bsp
