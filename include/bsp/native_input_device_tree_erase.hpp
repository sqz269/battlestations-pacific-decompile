#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeLegacySboStringStorage;

// Whole006A71B0..006A74F7, 840B/235 instructions. Native ECX actualtree,
// unused incomingEDX; stacked output/owner/node; EAX output, RET0Ch.
// Added RawCtx changes the Source ABI (four stacked inputs/RET10h).
// Actual tree: opaque0/head4/count8; actual node: links0/4/8,color98,nil99.
// Nil99 is tested before owner diagnostics. Normal path advances a private8B
// iterator, retains original removed node, then transplants/rebalances before
// pair(node+C). Pair failure leaves topology changed and node/count/output
// uncommitted. On success free original node, read CURRENT count, decrement
// every nonzero DWORD, capture owner/output/advancednode, store owner then node.
// No rollback, outer normal cleanup guard, scalar-delete or added validation.
void* __fastcall erase_native_input_device_tree_iterator_006a71b0(
    void* tree, std::uint32_t unused_edx, void* output, void* owner, void* node,
    NativeStringRawPoolContext& strings);

// Whole8B action00C7F580..00C7F587 as a logical saved-SBO service.
// Native LEAECX,[runtimeEBP-50h]; JMP004072D0. Source receives the actual
// completed message directly; no original EBP/frame API or RawCtx is invented.
// Only the nil-error message is armed, AFTER assignment of27 bytes and BEFORE
// logic-error construction/throw. Accepted current-CRT SBO destruction is
// noexcept; normal RawCtx pair destruction above remains propagating.
void unwind_native_input_device_erase_message_00c7f580(
    NativeLegacySboStringStorage& completed_message) noexcept;

// Valid reached actual storage/owned topology, aligned pointer words and
// matching current-CRT ownership are required. Existing actual iterator,
// min/max, rotations, legacy exception owner and pair Source are composed.
// CurrentCpp owning exception transport, private iterator/argument frames,
// Source helper calling conventions, RawCtx/currentCRT and fault/RTTI/FH3/SEH
// limits are explicit; no original binary ABI or game validation is claimed.
// Existing AV/full-range Source remains separate; no ordinary6A7AA0 API here.
} // namespace bsp
