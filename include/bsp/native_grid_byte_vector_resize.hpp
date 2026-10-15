#pragma once

#include <cstddef>

#include "bsp/cc13_avoid1.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native grid byte-vector kernels require MSVC Win32.
#endif

namespace bsp {

// Reuse the existing 10h-byte native vector view. TerrainGridLayer constructor
// 00424680 writes first/last/end at layer+1Ch/+20h/+24h; the callers pass
// layer+18h. The proxy word +00h is not read/written by this family.
using NativeGridByteVector = Cc13AvoidZoneRawVector;
struct NativeGridByteIterator {
    NativeGridByteVector* owner;            // +00h
    std::uint8_t* position;                 // +04h
};
static_assert(sizeof(NativeGridByteIterator) == 8);
static_assert(offsetof(NativeGridByteVector, first) == 4);
static_assert(offsetof(NativeGridByteVector, last) == 8);
static_assert(offsetof(NativeGridByteVector, end) == 0x0C);

// Complete instruction schedules for the three bodies, with concrete byte-range
// primitive reconstructions. Names are hypotheses. These new Source fastcall
// APIs put vector in ECX and consume an UNUSED EDX argument; the remaining stack
// payloads and RET immediates match the observed native boundaries. This is not
// proof of native C++ member identity or a production/drop-in binary binding.
//
// Allocations/free reuse singleton_lifetime_allocate/free, with actual requested
// native/host byte counts equal, including zero. Calls to the current SDK's
// memmove_s, length_error and existing qualified BF6713 Source entry are real
// services, not abstract success callbacks. Their CRT/exception/handler identity,
// fault/return addresses and provider frames differ from the original process.
// Native BF6713 may return: these kernels retain every continuation and reload.
//
// No new shape/range/non-null/admission checks are added. All addresses reached
// by the native schedule, stack/control slots, copied iterator payloads, heap
// backing and handler/provider state must remain valid when reached. Header,
// byte data, input fill and output iterator MAY overlap. Raw DWORD arithmetic,
// load/store order and signed-vs-unsigned guards are retained. No EH cleanup or
// rollback is added to a partially completed insert. See the packet evidence.

// 004219A0..00421A48: native ECX vector; stack(new_size, fill DWORD low byte);
// RET8. Grow invokes insert at old last. Shrink invokes checked erase, retaining
// cached old last across returning validation handlers. Equal size does nothing.
void __fastcall native_grid_byte_vector_resize_004219a0(NativeGridByteVector*,
    void* unused_source_edx, std::uint32_t new_size, std::uint8_t fill);

// 00420DD0..00420F72: native ECX vector; stack(iterator owner, position, count,
// fill pointer), RET10h. The owner word is unused. Captures *fill before reading
// the vector or count, even when count is zero. All later fills use that copy.
// Growth is max(old_capacity + old_capacity/2, old_size+count), with the native
// UINT32 overflow rule. Both spare-capacity tail cases and allocation are kept.
void __fastcall native_grid_byte_vector_insert_fill_00420dd0(NativeGridByteVector*,
    void* unused_source_edx, NativeGridByteIterator position, std::uint32_t count,
    const std::uint8_t* fill);

// 0041D7D0..0041D81F: native ECX vector; stack(out*, first iterator, last iterator),
// RET14h, EAX out. Checks only nonzero/equal iterator owners, not owner==vector.
// Copies only for signed-positive (vector.last-last.position), ignores secure
// move's status, writes vector.last, then out.position and out.owner in that order.
NativeGridByteIterator* __fastcall native_grid_byte_vector_erase_checked_0041d7d0(
    NativeGridByteVector*, void* unused_source_edx, NativeGridByteIterator* out,
    NativeGridByteIterator first, NativeGridByteIterator last);

} // namespace bsp
