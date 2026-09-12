#pragma once

#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Actual Win32 storage, not a typed map: tree head/count at +4/+8; head
// minimum/root/maximum at +0/+4/+8. Node allocation24h has links+0/+4/+8,
// key length/data+10/+14, color+20 (black1), nil+21. Provider+18 is borrowed
// here and is never destroyed. BDABF0 produces nodes; BE1DC0 initializes heads.
// Descriptive names are hypotheses. Complete logical bodies include the raw
// post-free tails documented in docs/NATIVE_VFS_MOUNT_TREE.md.

// ECX node, EAX selected node, RET. No owner validation.
void* maximum_native_vfs_mount_node_00bd94d0(void* node) noexcept;
void* minimum_native_vfs_mount_node_00bd94f0(void* node) noexcept;
// ECX tree, stack node, RET4; no specified result.
void rotate_native_vfs_mount_right_00bd9530(void* tree, void* node) noexcept;
void rotate_native_vfs_mount_left_00bda0e0(void* tree, void* node) noexcept;

// Complete82-byte BDF7E0. ECX tree is forwarded to recursion; stack node,
// RET4. Right recursion, capture current left, release current key, free node,
// then iterate captured left. Does not update the owner's head or count.
void destroy_native_vfs_mount_subtree_00bdf7e0(void* tree, void* node,
    NativeStringStorage& strings) noexcept;

// Complete716-byte BE0080. ECX tree; stack(output,owner,node); RET0C;
// EAX output. Input iterator is by value and advances before removal. No
// owner==tree check. Nonzero nil input constructs the native owning D6926C
// exception through the existing NativeHardwareLayoutInvalidIterator transport;
// its host RTTI/EH ABI differs. Returning CRT validation callbacks continue.
void* erase_native_vfs_mount_iterator_00be0080(void* tree, void* output,
    void* iterator_owner, void* iterator_node, NativeStringStorage& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Complete201-byte BE0D00. ECX tree; stack(output,first-owner,first-node,
// last-owner,last-node); RET14; EAX output. Both full and partial range paths,
// including checked-owner comparisons and successor-before-erase ordering.
void* erase_native_vfs_mount_range_00be0d00(void* tree, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringStorage& strings, const SingletonLifetimeCallbacks& invalid_parameters);

// Complete52-byte BE16C0. ECX tree, RET; EAX0 in the native tail. Clear via
// full range, release the current head, then zero current tree+4/+8.
void destroy_native_vfs_mount_tree_00be16c0(void* tree, NativeStringStorage& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Production composition supplies ActualNativeStringPoolStorage, repeating
// the actual getter/return per string. Its inherited noexcept release excludes
// native throwing-getter/FH3 equivalence. These C++ interfaces carry explicit
// services and are not drop-in native ABI or game-validated replacements.
} // namespace bsp
