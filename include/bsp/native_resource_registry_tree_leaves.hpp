#pragma once

#include <cstddef>
#include <type_traits>

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;

// Actual Win32 iterator words; owner is address identity, not a tree snapshot.
struct NativeResourceRegistryTreeIterator {
    void* owner;
    void* node;
};
static_assert(sizeof(NativeResourceRegistryTreeIterator) == 8);
static_assert(offsetof(NativeResourceRegistryTreeIterator, node) == 4);
static_assert(std::is_trivially_copyable_v<NativeResourceRegistryTreeIterator>);

// Actual caller-owned 1Ch nodes: left/parent/right +0/+4/+8, pooled key
// length/data +C/+10, factory value +14, color/nil bytes +18/+19. Tree+4
// points to a head whose +0/+4/+8 links are minimum/root/maximum. This API
// does not construct, populate or own the tree, iterator or factory values.

// Full B196D0/B196F0: ECX node; EAX selected node; RET. Read child then its
// nil BYTE; return the current candidate before a nil child. No owner check.
void* maximum_native_resource_registry_node_00b196d0(void* actual_node) noexcept;
void* minimum_native_resource_registry_node_00b196f0(void* actual_node) noexcept;

// Full B19640/B19830: ECX actual tree; stack node; RET4. Native EAX retains
// the replacement node, forwarded here though current native callers ignore
// it. Preserve child rereads, current head/parent selection and link stores.
void* rotate_native_resource_registry_right_00b19640(
    void* actual_tree, void* actual_node) noexcept;
void* rotate_native_resource_registry_left_00b19830(
    void* actual_tree, void* actual_node) noexcept;

// Full B19890: ECX actual iterator; RET or returning BF6713 tail. Null-owner
// handler precedes the current node read. A nil-node handler returns without
// advancing, even if it changes the node. Publish each intermediate parent
// during ascent and reread the current iterator node before comparing right.
void increment_native_resource_registry_iterator_00b19890(
    NativeResourceRegistryTreeIterator&, const SingletonLifetimeCallbacks&);

// Full B1A260: ECX tree; stack subtree node; RET4; no semantic result.
// Recurse current right, capture current data then current left BEFORE pool
// release, capture current length+1 with DWORD wrap, return key via complete
// actual getter/return composition, free captured node, then read captured
// left's current nil byte and continue. No header/head/count/value cleanup.
void destroy_native_resource_registry_subtree_00b1a260(
    void* actual_tree, void* actual_node, ActualNativeStringPoolStorage&);

// Use the application's actual pool publication/gate/canonical lifetime and
// established returning CRT service. Existing release() is noexcept; C++
// failure during lazy pool recreation terminates at that inherited boundary.
// No native recreation-failure EH, binary ABI or game parity is established.
// New source interfaces only; general erase/range/destructor remain separate.
} // namespace bsp
