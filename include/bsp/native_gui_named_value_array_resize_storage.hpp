#pragma once

#include <cstdint>

namespace bsp {
struct NativeGuiNamedValueArrayReserveStorageFrame;
struct NativeStringRawPoolContext;

// Whole normal AB4850 over the actual 12B {data, count, capacity} descriptor.
// Capture requested_count once. Only a reached reserve stages that captured
// DWORD into the existing ReserveFrame request HOME; reserve's clamp or later
// HOME mutation never replaces the outer target. No outer Frame/state is added.
//
// Growth captures post-reserve CURRENT count, then uses independent wrapping
// cursor/remaining registers. Every iteration reloads CURRENT data. Nonnull
// slots acquire the existing individual trivial EntryStorage lifetime without
// value initialization, then receive three DWORD zeros. Null skips birth/stores
// only; cursor/remaining still advance. Count is not incremented per slot.
// A fresh, independent count test can shrink after growth. Decrement CURRENT
// count first, reload updated count THEN data, return that raw tail header, and
// compare another CURRENT count. Every normal path finally stores the ORIGINAL
// captured target. No data/capacity clear, backing free or automatic cleanup.
//
// Caller proves genuine descriptor, live prefix/spare storage, individually
// reached C++ object/subobject/alignment/preimage validity and all actual raw
// string, allocation, payload, publication and retained-custody lifetimes.
// DWORD addressing grants neither a T[n] family nor typed queue admission.
// The entire retained ReserveFrame/HOMEs and active Source arguments/provider
// backing must be distinct per reentry and disjoint from Native write extents,
// including descriptor/records/payload, as required by genuine reserve/copy.
// Only normal nested return permits reuse; incomplete effects/debt stay live.
//
// Borrow SAME genuine raw pool cells/new-handler/deletion graph and SAME live
// authenticated 0109EEA4 feature reference. Forward it without early sampling;
// a shrink-only path adds no feature read. Inherit reserve/copy/BF extents,
// DF0/CPU, serialization and private-frame separation, including their Source
// argument/47CD/compare commutation qualifications. No original physical HOME,
// register/EFLAGS/fault/FH3/SEH/exception-Origin/runtime identity is established.
// All DWORD bit patterns and signed Native decisions remain; no count/null/
// overflow cap, semantic vector, fake owner/default, catch, RAII, noexcept,
// rollback, retry, compensating cleanup or Native issuer is introduced.
// Existing vector/partial-shrink/scalar/service consumers remain separate.
void resize_native_gui_named_value_array_storage_00ab4850(
    void* actual_header12, std::uint32_t requested_count,
    NativeGuiNamedValueArrayReserveStorageFrame&, NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
