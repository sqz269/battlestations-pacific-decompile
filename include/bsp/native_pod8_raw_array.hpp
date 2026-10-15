#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native POD8 raw arrays require MSVC Win32.
#endif

namespace bsp {
// The actual 0Ch consumed owner prefix, not the checked-vector +4/+8/+Ch
// header and not a C++ container. It has no vtable and no implicit field writes.
struct NativePod8RawArrayStorage {
    void* data_00;
    std::uint32_t size_04;
    std::uint32_t capacity_08;
};
static_assert(sizeof(NativePod8RawArrayStorage) == 0x0c);
static_assert(offsetof(NativePod8RawArrayStorage, size_04) == 4);
static_assert(offsetof(NativePod8RawArrayStorage, capacity_08) == 8);
static_assert(std::is_trivially_default_constructible_v<NativePod8RawArrayStorage>);

// All five entries are complete literal Win32 bodies. Original ECX is actual
// owner; each has one stack argument/RET4 except Grow, which has no stack input
// and plain RET. Source unused EDX fixes binding, not an Original argument.
// No semantic EAX return is established. No new virtual/callback provider,
// count guard, rollback, value snapshot or std::vector is introduced.
//
// Capacity is written BEFORE actual allocation, including on allocation
// failure. Byte sizes and capacity*2+2 use literal DWORD wrap, not checked
// count allocation. Allocate/free bind to the real current malloc/new-handler
// retry service and its matching free domain; Original static CRT/EH identity
// is outside this Source boundary. Returned allocation is published only after
// old-data copying and the conditional late old-pointer free.
//
// Each old element uses native x87 FLD/FSTP word0 then word1. The source base
// is reloaded from owner each iteration; the second source address is captured
// BEFORE the first store, but its contents are reread AFTER that store. Count
// is reloaded for every unsigned loop comparison. Current null destination
// skips both pointed reads/stores but still increments cursor/index. Ambient
// FP state/traps and all reached aliases are borrowed without normalization.
// Every actual access must be backed; malformed counts are not made safe.

// 00414010..00414067, 88B. Unconditionally capacity=n and reallocate/copy/free,
// even when n==old capacity or zero; size is not changed or clamped to n.
void __fastcall set_capacity_native_pod8_raw_array_00414010(
    void* owner, void* unused_edx, std::uint32_t capacity);

// 00414070..004140C8, 89B. Unconditional capacity=capacity*2+2 moduloDWORD;
// same complete reallocation schedule, no overflow predicate.
void __fastcall grow_native_pod8_raw_array_00414070(void* owner, void* unused_edx);

// 00418EA0..00418F22, 131B including three unreachable padding bytes. Grow
// only for size==capacity, then reload size/data and the physical value slot.
// Append reads value word0/store0 THEN word1/store1 through x87; current null
// computed destination skips value-pointer load and both reads/stores. Finally
// increment the actual size DWORD. Value is consumed AFTER any free/growth;
// a value in retired old storage is not snapshotted or repaired.
void __fastcall append_native_pod8_raw_array_00418ea0(
    void* owner, void* unused_edx, const void* value);

// 00418F30..00418F98, 105B. Reallocate iff unsigned n>size AND n>capacity;
// ALWAYS write size=n afterward, including shrinking/no-allocation branches.
// New elements are not initialized. Retains historical Native ReserveIfLarger
// name as a hypothesis, with this stronger observed size-write contract.
void __fastcall resize_native_pod8_raw_array_00418f30(
    void* owner, void* unused_edx, std::uint32_t size);

// 00418FB0..0041900C, 93B. Reallocate iff unsigned capacity<n; size unchanged.
void __fastcall reserve_native_pod8_raw_array_00418fb0(
    void* owner, void* unused_edx, std::uint32_t capacity);

// These exact owned instructions and real Source services do not establish
// Original private-frame aliases, CRT/new-handler/EH/SEH identity, an installed
// binary replacement, captured-Native execution or game validation.
} // namespace bsp
