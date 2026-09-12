#pragma once
#include <cstdint>

namespace bsp {
class NativeStringStorage;

// Actual24h entry: name length/data0/4, local-header offset8, resolved byteC,
// data offset10, method word14, compressed/decoded counts18/1C, retained CRC20.
// Padding D..F and16..17 is untouched. BB95B0 is the observed producer.
// Actual vector is pointer0/count4/capacity8; counts compare as signed DWORDs.
// Copy/reserve/resize/append take ECX owner and one stacked argument, RET4.

// Complete BB9330..BB939D; EAX original destination. Zero the destination name
// before the self-alias check. Copy name, then reread/copy each payload field.
void* copy_native_mpkg_entry_00bb9330(void* destination, const void* source,
    NativeStringStorage&);

// Complete BB93A0..BB9494. Clamp signed capacity to1. Copy ascending entries,
// release ascending old names, free CURRENT base, then publish new base/capacity.
// Current count/base are reread across callbacks. A failed copy leaves the new
// allocation/completed prefix orphaned: native state0 calls RET-only401130.
void reserve_native_mpkg_entries_00bb93a0(void* vector, std::int32_t capacity,
    NativeStringStorage&);

// Complete BB94A0..BB951C. Grow by zeroing only name headers; shrink backwards,
// decrementing current count before each name release. Finally store requested
// count. No bounds/overflow/null/negative-count policy is added.
void resize_native_mpkg_entries_00bb94a0(void* vector, std::int32_t count,
    NativeStringStorage&);

// Complete BB9520..BB9593. Grow only on count==capacity; signed doubled capacity
// <=1 becomes1. Copy into the current end and increment current count afterward.
// Do not protect aliases invalidated by growth. On copy failure CC47F0 rereads
// current count/base, invokes no-op placement deletion and performs no rollback.
void append_native_mpkg_entry_00bb9520(void* vector, const void* entry,
    NativeStringStorage&);

// Complete BB9900..BB9916; ECX vector, no stack arguments, plain RET. Resize0
// then free the CURRENT base. Count becomes0; base and capacity are not reset.
void destroy_native_mpkg_entries_00bb9900(void* vector, NativeStringStorage&);

// Explicit-service source interfaces, not binary/FH3/SEH replacements. Existing
// NativeStringStorage::release is noexcept; throwing lazy-pool recreation and
// invalid-memory/native CRT exception behavior remain outside this boundary.
} // namespace bsp
