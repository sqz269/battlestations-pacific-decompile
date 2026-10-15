#pragma once

#include "bsp/avoid_zone_owner.hpp"

namespace bsp {
// Complete normal flow and allocator-exception cleanup of 0041A030..0041A1DE.
// ECX=destination, one source pointer on stack, EAX=destination, RET4.
// Uses the existing 24h storage/record types and actual allocation services.
// Destination must be fresh; its old header is overwritten without cleanup.
// Source count/array/records must remain readable and stable across callbacks.
// Successful paths require all native slot accesses to fit the allocated
// buffers. Unsigned capacity growth and saturated byte requests are retained.
//
// IMPORTANT: source count N is installed BEFORE appending N clones. For N>0,
// capacity grows N -> 2*N+2 and count becomes 2*N. The initial N pointer slots
// retain OPAQUE ALLOCATION BYTES, copied as representations, never evaluated
// as pointer values here. No valid record identities are established for them.
// This result must not be treated as an ordinary safe owned polygon, traversed
// or passed to the destructor below unless EVERY counted slot independently
// satisfies that operation's native ownership/readability preconditions.
// No replacement prefix, zero-fill, count repair or leak repair is inserted.
//
// Bounds and all nine record floats use native FLD/FSTP copies, including
// signaling-NaN quieting under ambient x87 state. Identity words copy raw bits.
// Unwind C5E130 -> 00412DC0 frees/zeros only the pointer buffer; count, capacity,
// metadata and every allocated clone survive/leak as in the native schedule.
// This models C++ allocator throws, not the original FH3/SEH/register ABI or
// full FP exception behavior. Names are hypotheses; no native/game validation.
AvoidZoneNativeStorage& avoid_zone_copy_construct_0041a030(
    AvoidZoneNativeStorage& destination, const AvoidZoneNativeStorage& source,
    const AvoidZoneAllocationAccess&);

// Complete 0041A1E0..0041A1FE. ECX=zone, no stack args, RET. First calls the
// existing 004167C0 clear (free every record, set count=0), then frees a nonnull
// pointer buffer and zeros ONLY that pointer. Capacity and all trailing fields
// remain unchanged. Unlike avoid_zone_release_owned_storage, no capacity reset.
// Each counted slot must be a readable valid owned record pointer (or null for
// the actual CRT free), uniquely releasable through the supplied actual service;
// the buffer must match free_array. A nonempty copy above does not establish
// these preconditions for its opaque prefix. New C++ interface, not native ABI.
void avoid_zone_destroy_owned_storage_0041a1e0(
    AvoidZoneNativeStorage&, const AvoidZoneAllocationAccess&) noexcept;
} // namespace bsp
