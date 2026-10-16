#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual input int/string tree erasure requires MSVC Win32.
#endif

namespace bsp {
struct NativeLegacySboStringStorage;
struct NativeStringRawPoolContext;

// Actual checked-iterator words. Passing this trivial object by value supplies
// the private, mutable owner/node slots advanced by 0069AE50 inside the eraser.
struct NativeInputIntStringTreeIterator {
    void* owner;
    void* node;
};
static_assert(sizeof(NativeInputIntStringTreeIterator) == 8);
static_assert(offsetof(NativeInputIntStringTreeIterator, owner) == 0);
static_assert(offsetof(NativeInputIntStringTreeIterator, node) == 4);
static_assert(std::is_trivially_copyable_v<NativeInputIntStringTreeIterator>);

// Whole006A07D0..006A0A9B[716B/241starts]. Native ECX tree, EDX unused,
// stack(output, owner, node), EAX output, RET0Ch. This Source adds the borrowed
// actual pool context after the by-value iterator, hence RET10h, not binary ABI
// compatibility. Fixed node links0/4/8, mapped length10/data14, color18/nil19;
// tree opaque0/head4/count8. No projected node/owner, callbacks or new guards.
// Test input nil before iterator owner diagnostics; retain original removed
// node across private iterator advance, transplant and rebalance. Six rotation
// calls discard incidental EAX. Payload release follows all topology/color
// writes, then free(original), current nonzero unsigned count decrement, and
// capture current owner/output/node before either ordered output store.
// RawCtx getter failure propagates before free/count/output with no rollback
// or normal payload unwind guard. Existing CurrentCpp/CRT boundaries apply.
NativeInputIntStringTreeIterator* __fastcall
erase_native_input_int_string_tree_iterator_006a07d0(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputIntStringTreeIterator* actual_output,
    NativeInputIntStringTreeIterator input,
    NativeStringRawPoolContext& strings);

// Whole8B/2starts logical completed-message action00C7F050. Native action uses
// runtime EBP-50h and tail4072D0; Source directly borrows completed real1Ch SBO
// storage. No EBP reader, raw-pool context or normal mapped-payload cleanup.
void __fastcall cleanup_native_input_int_string_erase_message_00c7f050(
    NativeLegacySboStringStorage& completed_message) noexcept;

// Names describe a provisional instantiation. Source/build/static evidence
// does not establish Original register residue/flags, private-frame aliases,
// heap/CRT/FH3/SEH/RTTI/fault identity, drop-in ABI or game behavior.
} // namespace bsp
