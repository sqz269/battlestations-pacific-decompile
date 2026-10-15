#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene record map subtree destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete0046AD10..0046AD61[82B]. Native ECX is saved/forwarded only to
// recursive calls, never dereferenced; stack subtree root; RET4; no semantic
// result. This Source API omits that unused receiver and adds actual raw-pool
// context, so its interface is new rather than a binary calling-convention
// replacement. Root is actual1Ch-node storage, including a valid sentinel.
//
// Test current+19 sentinel; while zero recurse CURRENT right+8. After return,
// capture CURRENT key data+10h, THEN CURRENT left+0; only nonnull data reads
// CURRENT length+0Ch and returns captured block/length+1 through the concrete
// actual pool getter/return. Getter runs even for large/disabled returns.
// Free captured current node with the existing actual current-CRT service,
// then test captured left's sentinel and continue there. No reread of freed
// storage, header clearing, head/count reset or mapped+14 ownership is added.
//
// Full DWORD wrapping size arithmetic and actual pool/current-CRT domains are
// inherited, including zero returned size. No positive-only restriction,
// null/bounds/cycle guard, host callback, semantic owner, exception conversion,
// rollback, local cleanup or noexcept promise. Getter exceptions propagate
// after any completed right-subtree destruction, before current-node free.
// Valid taken backing and provider-compatible allocations remain required.
// Native private frame/register/SEH/fault identity, binary ABI, application
// adoption and game validation are not established by this Source body.
void destroy_native_scene_record_map_subtree_0046ad10(
    void* actual_root, NativeStringRawPoolContext& strings);

} // namespace bsp
