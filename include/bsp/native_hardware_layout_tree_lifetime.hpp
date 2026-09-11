#pragma once

#include "bsp/native_hardware_layout_tree.hpp"

namespace bsp {

// Native ECX unused, no stack arguments, EAX allocation, RET. Allocates actual
// 28h raw node storage, clears links +0/+4/+8, writes black1 at +24h and
// nonsentinel0 at +25h. The initializer's caller must set its sentinel byte.
// Key/value bytes and trailing padding remain untouched.
void* allocate_native_hardware_layout_tree_sentinel_00b25dc0();

// Native ECX actual tree carried through recursion, stack node, RET4. Frees
// nonsentinel nodes right subtree first, then node, then captured left subtree.
// The tree header and borrowed node values are not destroyed or reset.
void destroy_native_hardware_layout_subtree_00b230b0(
    void* actual_tree, void* node) noexcept;

// Native ECX actual tree; stack output/first owner/node/last owner/node; EAX
// output, RET14h. Input iterators are values. Full range clears current root,
// count and extrema while preserving the head allocation. Other ranges use
// the actual checked increment and erase routines. Handlers may return/throw.
// Publishes output OWNER then NODE, retaining assembly-order alias behavior.
NativeHardwareLayoutTreeIterator* erase_native_hardware_layout_range_00b2f3a0(
    void* actual_tree, NativeHardwareLayoutTreeIterator* output,
    NativeHardwareLayoutTreeIterator first, NativeHardwareLayoutTreeIterator last,
    const SingletonLifetimeCallbacks& invalid_parameters);

// New MSVC Win32 interfaces; no binary replacement or game validation claim.
} // namespace bsp
