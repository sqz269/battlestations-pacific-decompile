#pragma once

#include <cstdint>

namespace bsp {
struct NativeRetainedMemoryOwnerContext;
class NativeAdoptedSubstreamDispatch;

// Complete BEF840..BEF8E7 (168 bytes). Original ECX source; stacked offset
// low/high, length low/high; EAX independent memory wrapper; RET10h. Seek uses
// both offset words with origin0. Only the low length is consumed: its signed
// interpretation constructs the raw10h backing, while the one read receives
// its original unsigned bits and a null actual-count pointer.
//
// Borrow the source without changing its references. Capture the current read
// table, backing data, then that table's slot24 after backing construction.
// Wrap the backing through actual BEF6D0 and release its temporary reference.
// Construction failure frees only the raw backing allocation. Read/wrapper
// failures have no backing rollback; short-read tails remain untouched.
void* copy_native_stream_range_00bef840(void* actual_source,
    std::uint32_t offset_low, std::uint32_t offset_high,
    std::uint32_t length_low, std::uint32_t unused_length_high,
    NativeRetainedMemoryOwnerContext&, NativeAdoptedSubstreamDispatch&);

// New source interface over actual storage and shared allocation/counters.
// Original stack aliases, FH3/SEH, invalid memory and gameplay are separate.
} // namespace bsp
