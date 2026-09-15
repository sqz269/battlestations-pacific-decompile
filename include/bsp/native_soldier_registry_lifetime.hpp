#pragma once

#include "bsp/native_soldier_class_resolution.hpp"
#include "bsp/native_soldier_registry_tree.hpp"

namespace bsp {

// Actual tree: opaque+0, head+4, count+8. Nodes are 1Ch bytes: links0/4/8,
// NativeString+C, borrowed mapped DWORD+14, color18, nil19. No mapped release.
// Complete27B/28B leaves: ECX node, EAX minimum/maximum, RET.
void* minimum_native_soldier_registry_node_004af680(void* node) noexcept;
void* maximum_native_soldier_registry_node_004af6c0(void* node) noexcept;

// Complete82B: ECX tree, stack node, RET4. Right subtree first; capture current
// left after recursion, return current key to the actual pool, free, loop left.
// Does not change the header or count. Raw pool getter exceptions propagate.
void destroy_native_soldier_registry_subtree_004b0600(
    void* tree, void* node, NativeStringRawPoolContext&);

// Complete716B: ECX tree; stack output, owner/node iterator; EAX output, RET0Ch.
// Actual successor/transplant/red-black fixup, raw key release, node free, then
// nonzero current-count decrement. Publish advanced owner before node. Nil
// input uses the existing owning native out_of_range source transport.
NativeKeyboardTreeIterator* erase_native_soldier_registry_iterator_004b0330(
    void* tree, NativeKeyboardTreeIterator* output, NativeKeyboardTreeIterator input,
    NativeStringRawPoolContext&, const SingletonLifetimeCallbacks&);

// Complete201B: ECX tree; stack output, first owner/node, last owner/node;
// EAX output, RET14h. Preserve captured/current loads around returning invalid
// handlers; full range drains and self-links the current head. Partial range
// advances first before single-node erase; no mapped-pointer ownership.
NativeKeyboardTreeIterator* erase_native_soldier_registry_range_004b09a0(
    void* tree, NativeKeyboardTreeIterator* output, NativeKeyboardTreeIterator first,
    NativeKeyboardTreeIterator last, NativeStringRawPoolContext&,
    const SingletonLifetimeCallbacks&);

// Complete111B and30B. Actual owner10h: table+0, opaque+4, head+8, count+C.
// Drain through owner+4, free CURRENT head, clear head/count, clear CURRENT
// publication, stamp CE3818. C++ unwind clears publication/stamps base only.
// Scalar deleter frees owner when flags bit0 and returns captured owner.
void destroy_native_soldier_registry_004b1210(void* actual_owner,
    const NativeSoldierClassConstructionAccess&, const SingletonLifetimeCallbacks&);
void* delete_native_soldier_registry_004b1280(void* actual_owner, std::uint32_t flags,
    const NativeSoldierClassConstructionAccess&, const SingletonLifetimeCallbacks&);

// Complete189B getter: no native inputs, EAX registry, RET. Actual manager is
// access.strings.actual_manager_publication_01090aa0. Use genuine415350, captured
// raw section+10 and physical depth+18, constructor4B11A0 and BD0C30. Allocation
// failure frees raw owner; registration failure retains publication. Slow return
// reloads after guard release. No invented teardown callback or private domain.
void* get_native_soldier_registry_004b1330(
    const NativeSoldierClassConstructionAccess&);

// New MSVC Win32 source interfaces. Valid reached native storage and allocator
// domains are required. Numeric table identities are not callable host vtables;
// mixed-owner manager drain still needs genuine profile dispatch integration.
// C++ exception cleanup is reconstructed; no original ABI/FH3/SEH/game proof.
} // namespace bsp
