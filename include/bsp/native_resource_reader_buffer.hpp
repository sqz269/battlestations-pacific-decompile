#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

struct NativeStringRawPoolContext;

inline constexpr std::size_t native_resource_reader_buffer_header_bytes = 0x0c;
inline constexpr std::size_t native_resource_reader_buffer_entry_bytes = 0x0c;

// Actual reader+4 header: DWORD data/count/capacity at +0/+4/+8. Each 0Ch
// entry has an actual8h native string and an opaque DWORD at +8. New C++ ABI;
// ECX was the header, one signed DWORD stack argument, RET4, no stable result.
// Caller supplies live backing for all native DWORD-wrapping accesses. No
// vector projection, count validation, length clamp or automatic ownership.

// BF05D0: signed requested capacity<1 becomes1; return if current signed
// capacity suffices. Allocate wrapping12*capacity bytes, copy current entries
// with current string headers, return old strings forward, free CURRENT old
// data, then publish replacement and capacity. Current count is never set.
// Native state0 invokes only no-op placement delete401130: no partial-entry,
// completed-entry or replacement-block cleanup is invented on failure.
void reserve_native_resource_reader_buffer_00bf05d0(void* actual_header,
    std::int32_t requested_capacity, NativeStringRawPoolContext&);

// BF0700: reserve if signed capacity<count; grow by zeroing only string words
// at +0/+4, leaving every +8 DWORD untouched. Shrink in reverse, publishing
// each decremented CURRENT count before returning that entry's string. Reload
// current header/count across returns, then publish requested count normally.
// No negative-count clamp, string-header clear or capacity shrink is added.
void resize_native_resource_reader_buffer_00bf0700(void* actual_header,
    std::int32_t requested_count, NativeStringRawPoolContext&);

// Raw pool context resolves the current getter for EVERY allocation/return,
// including disabled small returns. Getter failures propagate with completed
// writes intact. Native CRT/FH3/hardware-fault interoperability and full reader
// teardown are outside these source interfaces.

} // namespace bsp
