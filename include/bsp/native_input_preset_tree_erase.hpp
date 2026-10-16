#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual input preset-tree erasure requires MSVC Win32.
#endif

namespace bsp {
struct NativeLegacySboStringStorage;
struct NativeStringRawPoolContext;

// Actual mutable private owner/node words, passed by value to the eraser.
// This is an iterator carrier, not a node or substitute container.
struct NativeInputPresetTreeIterator {
    void* owner;
    void* node;
};
static_assert(sizeof(NativeInputPresetTreeIterator) == 8);
static_assert(offsetof(NativeInputPresetTreeIterator, owner) == 0);
static_assert(offsetof(NativeInputPresetTreeIterator, node) == 4);
static_assert(std::is_trivially_copyable_v<NativeInputPresetTreeIterator>);

// Whole006A04A0..006A076B:716B/241starts. Preserve established STL identity.
// Native ECXtree, EDXunused, stack(output,owner,node), EAXoutput, RET0Ch.
// Source adds borrowed actual RawCtx after the8B iterator:RET10h/new ABI.
// Actual node links0/4/8, pooled key lengthC/data10, color28/nil29; tree
// opaque0/head4/count8. No node overlay, nil adapter, callback or new guard.
// Retain ORIGINAL removed node across actual private-iterator mutation,
// successor transplant and ordered fixups. Six rotation EAX results ignored.
NativeInputPresetTreeIterator* __fastcall
erase_native_input_preset_tree_iterator_006a04a0(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputPresetTreeIterator* actual_output,
    NativeInputPresetTreeIterator input,
    NativeStringRawPoolContext& strings);

// Logical completed-message action00C7F030[8B/2starts]. Original EBP-50h
// belongs to the compiler's error frame. Source directly borrows real1Ch SBO
// storage, delegates actual4072D0, and has no EBP reader or raw-pool context.
void __fastcall cleanup_native_input_preset_erase_message_00c7f030(
    NativeLegacySboStringStorage& completed_message) noexcept;

// Initial nil29 error constructs exact27-byte LegacySBO message; completed
// guard arms only AFTER successful counted assignment and before owning28h
// error construction/throw. Normal key is a separate8B pooled header atnode+C:
// data10 capture, conditional CURRENT lengthC+1, getter/return, no key clear,
// mapped20B cleanup, normal payload guard or rollback. Getter failure follows
// topology/colors and precedes ORIGINALnodefree/currentcount/output work.
// Decrement every unsigned nonzero CURRENT count; capture owner/output/node
// before ordered owner/node stores. Raw storage/ownership must remain valid.
// CurrentCpp/CRT, added RawCtx, unusedEDX0, privateiterator/argument frames,
// original heap/FH3/SEH/RTTI/fault/register/ABI/game identity stay qualified.
// Existing typed/AV, nil19, subtree, range, member and owner domains unchanged.
} // namespace bsp
