#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual controller-tree erasure requires MSVC Win32.
#endif

namespace bsp {
struct NativeLegacySboStringStorage;
struct NativeStringRawPoolContext;

// Real private owner/node argument slots advanced by actual 0069AD50.
struct NativeInputControllerTreeIterator {
    void* owner;
    void* node;
};
static_assert(sizeof(NativeInputControllerTreeIterator) == 8);
static_assert(offsetof(NativeInputControllerTreeIterator, owner) == 0);
static_assert(offsetof(NativeInputControllerTreeIterator, node) == 4);
static_assert(std::is_trivially_copyable_v<NativeInputControllerTreeIterator>);
static_assert(std::is_trivial_v<NativeInputControllerTreeIterator>);
static_assert(std::is_standard_layout_v<NativeInputControllerTreeIterator>);

// Whole006A6010..006A62C2[691B/234starts]. Native ECX tree/unused EDX,
// stack(output, owner, node), EAX output, RET0Ch. Source adds borrowed actual
// RawCtx after the real by-value iterator, RET10h: a new, qualified ABI.
// Node links0/4/8, color20/nil21, actual pair+C; tree opaque0/head4/count8.
// Test node nil before owner diagnostics. Retain original removed node before
// advancing the private iterator. Preserve CURRENT captures/transplants and
// retained near-child pointers through far-child reads; six rotations discard
// incidental EAX. Actual pair completion precedes free(original), current
// nonzero unsigned count decrement and capture of owner/output/node before
// ordered output stores. Pair failure leaves topology progress and prevents
// free/count/output; no parent payload unwind guard or rollback is added.
NativeInputControllerTreeIterator* __fastcall
erase_native_input_controller_tree_iterator_006a6010(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputControllerTreeIterator* actual_output,
    NativeInputControllerTreeIterator input,
    NativeStringRawPoolContext& strings);

// Logical whole8B/2starts C7F490: completed real1Ch SBO storage ->4072D0.
// Original action addresses runtime EBP-50h. Source borrows the completed
// object directly, without EBP access, RawCtx or normal pair/key cleanup.
void __fastcall cleanup_native_input_controller_erase_message_00c7f490(
    NativeLegacySboStringStorage& completed_message) noexcept;

// Descriptive names are hypotheses; original library names are retained.
// CurrentCpp/RawCtx/CRT boundaries do not prove Original private frames,
// register/flag residue, heap/FH3/SEH/RTTI/fault identity, drop-in ABI or game.
} // namespace bsp
