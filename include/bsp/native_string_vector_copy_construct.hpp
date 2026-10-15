#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual string vector copy construction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete 00556B10 normal body and 00556B5A completed-prefix catch.
// Native ECX=first, EDX=last; stack=(destination,owner,unused_tag3,unused_tag4),
// RET10h, EAX=completed destination end. Source retains those six physical
// inputs and adds the actual raw-pool context. Empty range does no element work.
// The accepted 0044D0C0 implementation has the same exact copy/advance/catch
// schedule: actual0044BCB0 copy construction; on failure destroy only completed
// headers in ascending order through00431F40, then rethrow. The failing current
// header is excluded. Cleanup failure stops the loop and escapes immediately.
void* __fastcall copy_native_string_uninitialized_range_00556b10(
    const void* first, const void* last, void* destination, void* owner,
    std::uint32_t unused_tag3, std::uint32_t unused_tag4,
    NativeStringRawPoolContext& strings);

// Complete 00432440..0043247C. Native ECX=actual10h vector, no stack input,
// plain RET, no semantic result. Source additionally uses EDX for context.
// Exact body/callee equivalence permits reuse of accepted actual004324A0:
// if captured begin!=0, destroy [captured begin,captured end), free CURRENT
// begin, then zero +4,+8,+C in that order. +0 is unchanged. Exceptions stop
// later destruction/free/clears; there is no local catch or retry.
void __fastcall tidy_native_string_header_vector_00432440(void* header,
    NativeStringRawPoolContext& strings);

// Complete 00557BC0..00557C83 and 00557C84 owner catch. Native __thiscall:
// ECX=destination, stack=source owner, RET4, EAX=destination; EDX not consumed.
// Source preserves those physical inputs and adds an explicit raw-pool context.
// Capture source count BEFORE clearing destination+4,+8,+C; null source begin
// yields count0 without reading source end. Otherwise DWORD wrapping subtraction
// followed by SAR3 is retained. Count0 skips allocation/validation/element work.
// Nonzero count uses actual00431F70 length throw and0042BF40 allocation, then
// publishes begin/end/capacity. Capture current source end, read/compare current
// source begin, THEN enter the native state0 exception domain. Report/continue;
// capture current source begin, compare with fresh end, report/continue; reload
// CURRENT destination begin and copy the captured source range. Publish returned
// end only after success. The outer catch tidies CURRENT destination then
// rethrows; allocation/length failures precede that catch. No self-copy guard,
// old-buffer cleanup or additional iterator/bounds/alignment check is added.
void* __fastcall construct_native_string_header_vector_copy_00557bc0(
    void* destination, std::uint32_t unused_edx, const void* source,
    NativeStringRawPoolContext& strings);

// Actual8h NativeString headers and accepted pool/manager/current-CRT services
// are used directly, without a generic vector/allocator/copy callback. These
// are new Source interfaces: added context, C++ catch/rethrow and by-value inputs
// do not reproduce original FH3/SEH/private-frame aliases, CRT exception/handler
// identity, fault addresses or cleanup-failure unwind identity. Native catch
// entries require their parent frame and are represented by Source catch scopes,
// not standalone callable APIs. Earlier qualified storage projections remain
// separate. No new-function native differential or gameplay validation is claimed.

} // namespace bsp
