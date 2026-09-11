#pragma once

#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_resource_registry_tree_leaves.hpp"

namespace bsp {

// Caller-owned actual tree header: preserved DWORD +0, current head +4,
// count +8. Nodes are 1Ch bytes: links +0/+4/+8, pooled key length/data
// +0C/+10, opaque unowned factory +14, color/nil bytes +18/+19.
// The existing NativeHardwareLayoutInvalidIterator is used only as the
// owning native out_of_range source transport (28h raw D6926C object).
// No hardware-layout tree operations or node layout are selected here.

// B19F90[716]: ECX tree; stack output pointer, input owner, input node;
// RET0Ch; EAX output. Input is by value. Nil input throws before owner
// validation; otherwise complete successor transplant and red-black fixup,
// current key release, actual node free, then current unsigned count update.
// Capture advanced owner/node before output owner then node stores; output
// may alias other live raw storage. Input owner need not equal tree.
NativeResourceRegistryTreeIterator* erase_native_resource_registry_iterator_00b19f90(
    void* tree, NativeResourceRegistryTreeIterator* output,
    NativeResourceRegistryTreeIterator input, ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// B1A2F0[201]: ECX tree; stack output, first owner/node, last owner/node;
// RET14h; EAX output. Complete current full-range fast clear and partial
// erase loop. Partial iteration advances its own first and independently
// advances the by-value old iterator inside B19F90; ignores the latter result.
// Returning invalid handlers retain each captured identity and current reread.
NativeResourceRegistryTreeIterator* erase_native_resource_registry_range_00b1a2f0(
    void* tree, NativeResourceRegistryTreeIterator* output,
    NativeResourceRegistryTreeIterator first, NativeResourceRegistryTreeIterator last,
    ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// New MSVC Win32 C++ interfaces. Existing actual pool release is noexcept:
// returning-getter composition is admitted; pool recreation failure/native
// SEH is not reconstructed here. Host exception RTTI/catch/throw ABI differs.
// No registry creation, factory ownership, enclosing destructor or game claim.
} // namespace bsp
