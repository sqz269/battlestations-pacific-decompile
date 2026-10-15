#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Complete 0044CE20 normal loop and0044CE6D catch in the Source C++
// exception domain. Original ECX=destination, EDX=unsigned count; stack
// value-header, owner, unused value duplicate, unused tag; RET10h, void.
// Capture the value HEADER ADDRESS, not its contents. Construct consecutive
// actual8h headers with0044BCB0, advancing completed end only after return.
// On a C++ exception, destroy the completed prefix ascending via00431F40,
// excluding the failed current slot, then rethrow. A cleanup exception escapes
// immediately, leaving subsequent completed elements untouched.
//
// The existing actual pool/publication context is explicit in Source. The
// caller supplies readable source and writable uninitialized output accepted
// by the existing constructor/destructor services. No overlap rejection,
// backing allocation, validation, header clearing or rollback is added.
// Addresses advance with DWORD wrap. Native private-stack/ABI, FH3/SEH
// catchability and nested/double-exception identity are unproved; /EHsc C++
// exceptions only. See docs/OCT15_RAW_STRING_UNINITIALIZED_FILL.md.
void fill_native_string_uninitialized_0044ce20(void* destination,
    std::uint32_t count, const void* value, void* owner,
    NativeStringRawPoolContext& strings);

// Complete0044DB60 ordinary fill_n dispatcher. Original ECX=owner; stacked
// destination/count/value, RET0Ch. After the core returns, compute EAX from
// the CAPTURED original destination/count, with DWORD dest+count*8 wrap.
// Dead duplicate/tag words and the original private scratch word are not
// Source ABI promises. The value header is shared by all constructions.
void* fill_native_string_uninitialized_n_dispatch_0044db60(void* owner,
    void* destination, std::uint32_t count, const void* value,
    NativeStringRawPoolContext& strings);
} // namespace bsp
