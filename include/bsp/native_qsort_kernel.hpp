#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error The valid-input private-sort kernel requires MSVC Win32.
#endif

#include <cstdint>

namespace bsp {

using NativeQsortCompare = std::int32_t (__cdecl *)(const void*, const void*);

// 00BFC350: partial_valid_input_entry_projection. The complete retained body
// was read, but this Source API excludes Original validation/failure plumbing.
// Private helpers cover whole normal geometry of 00BFC2C0/00BFC290 through new
// C++ interfaces. Correct Original library names _qsort/shortsort/swap remain.
//
// Caller preconditions for EVERY count: positive width, genuine nonnull cdecl
// comparator, and nonnull genuine base whenever count!=0. Original validation
// precedes its count<2 return; these preconditions are not runtime checks here.
// Count0/1 does no element/callback/extent work. For count>=2, require one actual
// live writable byte/trivially-copyable extent covering count*width, with the
// mathematical product <=INT32_MAX and nonwrapping base+extent/one-past address.
// Element starts must satisfy the real comparator's type/alignment/lifetime
// contract. Ordering is coherent/stable; no callback mutation, reentry, throws,
// faults, overlap, concurrency, teardown or locale/lifetime changes are admitted.
//
// Calls the exact passed comparator with actual element addresses. Preserves
// median/scan/pivot/tie/exchange order, cutoff8, and paired30-entry LIFO worklists.
// No validation policy, errno/TLS service, std sort, copied pivot or fallback.
// API is potentially throwing; native physical ABI, failure/EH/fault semantics,
// scene callbacks and application composition are not established by this body.
// Full bounds and evidence: docs/NATIVE_QSORT_KERNEL.md.
void sort_valid_native_bytes_00bfc350(void* actual_base, std::uint32_t count,
    std::uint32_t width, NativeQsortCompare compare);

// 00BFC350 whole entry (cc13-P4): the two validation arms ahead of the kernel above.
// 00BFC35E..00BFC393: base null with count != 0; 00BFC394..00BFC3D0: width 0 or a
// null comparator. Each sets *_errno() (00BFFB8B) = 16h (EINVAL), calls
// _invalid_parameter(0,0,0,0,0) (00BF66EF) and returns. Both CRT services are bound.
struct NativeQsortCrtHost {
    virtual ~NativeQsortCrtHost() = default;
    virtual int* errno_slot_00bffb8b() = 0;
    virtual void invalid_parameter_00bf66ef() = 0;
};
void qsort_00bfc350(NativeQsortCrtHost& crt, void* base, std::uint32_t count,
    std::uint32_t width, NativeQsortCompare compare);

} // namespace bsp
