#pragma once

#include <cstdint>

namespace bsp {
struct NativeRetainedMemoryOwnerContext;

// Complete BF03E0 within the established current D642C0/+1C BEF540 and
// D691B0/+1C BF4F20 profile domain. Original: ECX actual reader, stacked low
// distance and ignored pointer, RET8; provider EAX forwarded without testing.
// Reader+0 is actual stream. Always seek(low, high0, origin1). The ignored
// pointer is neither read nor written. Memory returns the previous absolute
// cursor bits; physical returns the full SetFilePointerEx BOOL bits.
// This explicit context-bearing C++ interface is NOT the original ABI.
std::uint32_t skip_native_raw_reader_relative_00bf03e0(void* actual_reader,
    std::uint32_t distance_low, void* ignored,
    NativeRetainedMemoryOwnerContext&);

// Complete BE9C40 within the same provider domain. Original: ECX actual
// wrapper, RET, final EAX actual reader pointer (not a Boolean status).
// Wrapper -> node; parent+0C is charged child DECLARED+1C before any seek.
// Nonzero child REMAINING+20 is sought and then cleared regardless of returned
// seek status. Reload reader+8, decrement reader+60, clear node+8, return reader.
// No attached/null/extent/duplicate guard, reference release, or rollback.
// Zero remaining still decrements path and clears attachment. No seek on that
// branch. Do not substitute this operation for BE9DF0's no-seek destructor.
void* skip_and_detach_native_raw_node_00be9c40(const void* actual_wrapper,
    NativeRetainedMemoryOwnerContext&);

// Borrow the existing memory context's original profile words. For physical
// streams the actual numeric D691B0 words must be readable. Other current
// profiles/slot identities are outside the domain; there is no invented seek
// callback or safe fallback. Existing memory/physical providers are called by
// their source symbols, never by numeric original table words. No ownership,
// raw binary ABI, OS-failure fixture, or game-runtime validation is claimed.
} // namespace bsp
