#pragma once

#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Actual 10h row: float bits, two opaque DWORDs, and an owner pointer. +8 is
// unspecified in the reader's default row; neither zero it nor interpret it.
// This trivial transport does NOT retain/release the fourth word on copying.
struct NativeDamageableFakeEffectRow {
    std::uint32_t words[4];
};
static_assert(sizeof(NativeDamageableFakeEffectRow) == 0x10);

// Actual checked iterator storage: owner then raw position, both DWORDs.
struct NativeDamageableFakeEffectIterator {
    void* owner;
    void* position;
};

// Header+0 is opaque; +4/+8/+C are actual begin/end/capacity pointers. All
// interfaces below borrow this raw storage. They use the canonical CRT heap,
// actual owner+4 interlocked counts/vslot0, and the caller's invalid-parameter
// boundary (which may return and mutate the current header).
// Original ABIs are recorded below; these C++ interfaces do not install FH3
// metadata or promise native binary, asynchronous-SEH or game compatibility.

// 0087C920: ECX header; stack new-size + owned 10h by-value row; RET14h.
// Consumes the incoming row's existing owner reference, including on unwind.
void resize_native_damageable_fake_effect_vector_0087c920(void* header,
    std::uint32_t new_size, NativeDamageableFakeEffectRow owned_default,
    const SingletonLifetimeCallbacks& invalid_parameters);

// 0087C380: ECX header; stack iterator-owner, position, count, row*; RET10h.
// Iterator-owner is ignored by the original body; position is not revalidated.
void insert_native_damageable_fake_effect_vector_0087c380(void* header,
    void* iterator_owner, void* position, std::uint32_t count, const void* row);

// 0087B8F0: ECX header; stack output*, first-owner/position, last-owner/position;
// EAX output*, RET14h. The output may alias header/rows: write it last.
NativeDamageableFakeEffectIterator* erase_native_damageable_fake_effect_vector_0087b8f0(
    void* header, NativeDamageableFakeEffectIterator* output,
    NativeDamageableFakeEffectIterator first, NativeDamageableFakeEffectIterator last,
    const SingletonLifetimeCallbacks& invalid_parameters);

// 008769B0: ECX count, EDX ignored hint, EAX allocation, RET. Overflow throws
// bad_alloc through the canonical host CRT boundary, zero still allocates.
void* allocate_native_damageable_fake_effect_rows_008769b0(std::uint32_t count);
// 0087A510: no arguments, throws legacy length_error payload (host transport).
[[noreturn]] void throw_native_damageable_fake_effect_length_0087a510();
// 00878A60: ECX row, RET. Clear +C only after owner release returns.
void destroy_native_damageable_fake_effect_row_00878a60(void* row);

// 00878C40/00878820/008788D0: cdecl first,end,source-or-destination. Assignment
// uses x87 FLD/FSTP, publishes the new owner before retain then old release.
void fill_native_damageable_fake_effect_rows_00878c40(void* first,
    const void* end, const void* row);
void* copy_native_damageable_fake_effect_rows_00878820(const void* first,
    const void* end, void* destination);
void* copy_backward_native_damageable_fake_effect_rows_008788d0(const void* first,
    const void* end, void* destination_end);
// 00879330/00879870 cdecl wrappers recompute the return from the original span.
void* copy_native_damageable_fake_effect_rows_00879330(const void* first,
    const void* end, void* destination);
void* copy_backward_native_damageable_fake_effect_rows_00879870(const void* first,
    const void* end, void* destination_end);

// 008799F0: ECX first, EDX end, stack destination plus 3 ignored words, RET10h.
// 0087AB80: ECX destination, EDX count, stack row plus 3 ignored words, RET10h.
// Both leave FH3 state -1 throughout: no partial-range cleanup on failure.
// Null destinations skip that row. Zero +C before reading source +C, so an
// aliased construction can discard a reference without releasing it.
void* uninitialized_copy_native_damageable_fake_effect_rows_008799f0(
    const void* first, const void* end, void* destination);
void uninitialized_fill_native_damageable_fake_effect_rows_0087ab80(
    void* destination, std::uint32_t count, const void* row);
// 0087B5D0/0087BC50: ECX ignored header, three stack words, RETCh.
void* uninitialized_fill_native_damageable_fake_effect_rows_0087b5d0(
    void* destination, std::uint32_t count, const void* row);
void* uninitialized_copy_native_damageable_fake_effect_rows_0087bc50(
    const void* first, const void* end, void* destination);
// 0087B5B0: ECX ignored header, stack first/end, RET8.
void destroy_native_damageable_fake_effect_rows_0087b5b0(void* first, const void* end);

} // namespace bsp
