#pragma once

#include "bsp/observer_lifetime.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual 18h element used by the state vector. This is NOT the 90h obstacle
// node. Only these fields are admitted; the three padding bytes are retained.
struct NativeLandStateObserverElement24 {
    NativeObserverOwnerStorage callback_00;
    volatile std::uint8_t enabled_10;
    std::uint8_t untouched_11[3];
    NativeObserverOwnerStorage* volatile first_endpoint_14;
};
struct NativeLandStateObserverVector {
    NativeLandStateObserverElement24* volatile data_00;
    volatile std::int32_t count_04;
    volatile std::int32_t capacity_08;
};
static_assert(sizeof(NativeLandStateObserverElement24) == 0x18);
static_assert(offsetof(NativeLandStateObserverElement24, enabled_10) == 0x10);
static_assert(offsetof(NativeLandStateObserverElement24, first_endpoint_14) == 0x14);
static_assert(sizeof(NativeLandStateObserverVector) == 0x0c);
static_assert(offsetof(NativeLandStateObserverVector, count_04) == 4);
static_assert(offsetof(NativeLandStateObserverVector, capacity_08) == 8);

// PURE caller aliases to the actual state profile and vector at state+0Ch.
// MoveTo's callback is the actual embedded state+18h prefix, not a sidecar.
struct NativeLandStateCleanupView {
    volatile std::uint32_t& profile_00;
    NativeLandStateObserverVector& elements_0c;
};
struct NativeLandMoveToCleanupView {
    NativeLandStateCleanupView state;
    NativeObserverOwnerStorage& callback_18;
};

// Seven COMPLETE ordinary bodies, new SOURCE interfaces. Actual mandatory
// observer lifetime/global/lock services and the canonical CRT allocator/free
// are reused. Raw CF5C20/CF5C94/D056D0 stamps are UNCALLABLE image identities;
// no executable table, constructor/arena or lifetime adapter is manufactured.
// Only live vector elements with actual CF5C94 (scalar slot0 0064B5F0/RET4)
// are admitted. CF5C20 slot0 is separate scalar0064A6A0/RET4; both wrappers
// call ordinary0064A610/RET. The different raw profiles are NOT interchanged.
// An unsupported profile reports a SOURCE admission error, never a fallback.
//
// Required normal domain: stable coherent actual storage, nonnegative vector
// counts/capacities and resize counts, count<=capacity, valid exact24B backing,
// disjoint growth/source/destination ranges, live actual FIRST endpoints and
// successful same-CRT allocation/free. Native24*n address arithmetic is modulo
// 32 bits; admitted sizes/ranges must also be representable and nonwrapping.
// No structural reentry/concurrency, aliased header mutation, invalid placement,
// null allocation, overflow, faults/privateEH or owner/death lifetime guarantee.
// All callback-bearing fresh loads are retained within that ordinary domain.

// 007B3FC0..007B4024: ECX=destination, stack=source, EAX=destination, RET4.
// Base-prefix copy constructor: source14 only; does NOT copy bytes11..13,
// edges, enabled10 or extra90h fields. Publishes14/10 BEFORE real registration.
NativeLandStateObserverElement24* copy_native_land_state_element_007b3fc0(
    NativeLandStateObserverElement24& destination,
    const NativeLandStateObserverElement24& source, NativeObserverLifetime&);
// 0064A610..0064A668: ECX=common prefix, RET. StampCF5C20, capture14 once,
// optional unregister, then complete695870. Preserve FINAL provider fields.
void destroy_native_land_state_element_base_0064a610(
    NativeLandStateObserverElement24&, NativeObserverLifetime&);
// 0064B5F0..0064B60E: ECX=element, stack=flags, EAX=original identity, RET4.
// Test LOW byte bit0 after complete helper; optional real self-free. flags1
// requires a separate actual CRT object, NEVER an interior array element.
NativeLandStateObserverElement24* scalar_delete_native_land_state_element_0064b5f0(
    NativeLandStateObserverElement24&, std::uint32_t flags, NativeObserverLifetime&);

// 007B4400..007B44F1: ECX=vector, stack=capacity, RET4. Minimum1; copy/register
// new elements before forward old destruction/free; publish DATA then CAPACITY.
void reserve_native_land_state_elements_007b4400(
    NativeLandStateObserverVector&, std::int32_t capacity, NativeObserverLifetime&);
// 007B4500..007B4585: ECX=vector, stack=count, RET4. Complete reserve/add/shrink
// branches; reverse shrink decrements published count BEFORE current cleanup.
void resize_native_land_state_elements_007b4500(
    NativeLandStateObserverVector&, std::int32_t count, NativeObserverLifetime&);
// 007B45F0..007B4644: ECX=state, RET. Full84B is byte/known-boundary evidence;
// stored Ghidra body still ends4629. resize0 ->fresharray/free ->D056D0 stamp.
// Array/capacity fields remain dangling after cleanup; do NOT reuse/dereference.
void destroy_native_land_state_007b45f0(
    const NativeLandStateCleanupView&, NativeObserverLifetime&);
// 007B65E0..007B662E: ECX=nonnull actual MoveTo, RET. callback18 destruction
// precedes complete shared-state cleanup. Whole9B2C80/constructors stay unbound.
void destroy_native_land_moveto_007b65e0(
    const NativeLandMoveToCleanupView&, NativeObserverLifetime&);

} // namespace bsp
