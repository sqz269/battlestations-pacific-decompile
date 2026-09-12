#pragma once

#include "bsp/native_filestore_subtree.hpp"

namespace bsp {

// Actual Win32 storage produced by BE55E0/BE7FA0: tree head/count +4/+8;
// 1Ch nodes links +0/+4/+8, resident payload +C, black color +18, nil +19.
// Names are hypotheses. Complete logical bodies include raw post-free tails.
// These explicit-service C++ interfaces are not original ABI replacements.

// Native ECX node, EAX selected node, RET; no owner validation.
void* maximum_native_file_store_resident_node_00be4920(void* node) noexcept;
void* minimum_native_file_store_resident_node_00be4940(void* node) noexcept;
// Native ECX tree, stack node, RET4; no specified semantic result.
void rotate_native_file_store_resident_right_00be4980(void* tree, void* node) noexcept;
void rotate_native_file_store_resident_left_00be50e0(void* tree, void* node) noexcept;

// Complete BE6760..BE6A12: ECX tree; stack output/owner/node; EAX output;
// RET0C. Advance the by-value iterator, transplant/rebalance, destroy the
// ORIGINAL node payload and free it, decrement CURRENT count only if nonzero,
// then publish advanced owner/node. No owner==tree validation. Nil throws the
// existing owning NativeHardwareLayoutInvalidIterator source transport.
void* erase_native_file_store_resident_iterator_00be6760(void* tree, void* output,
    void* iterator_owner, void* iterator_node, NativeStringStorage&,
    NativeAdoptedSubstreamDispatch&, const SingletonLifetimeCallbacks&);

// Complete BE7690..BE7758: ECX tree; stack output/first-owner/first-node/
// last-owner/last-node; EAX output; RET14. Full and partial checked ranges.
void* erase_native_file_store_resident_range_00be7690(void* tree, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringStorage&, NativeAdoptedSubstreamDispatch&,
    const SingletonLifetimeCallbacks&);

// Complete BE7A30..BE7A63: ECX tree, RET (native EAX0). Full-range erase,
// free CURRENT sentinel, then zero current head/count; tree+0 untouched.
void destroy_native_file_store_resident_tree_00be7a30(void* tree,
    NativeStringStorage&, NativeAdoptedSubstreamDispatch&,
    const SingletonLifetimeCallbacks&);

// Complete distinct BE7BB0..BE7BE3; same instructions except CALL offsets.
// Actual target of provider constructor/dtor unwind actions CC6E60/CC6DF8.
// Same ECX/RET/current-head free and head/count-zero contract as BE7A30.
void destroy_native_file_store_resident_tree_00be7bb0(void* tree,
    NativeStringStorage&, NativeAdoptedSubstreamDispatch&,
    const SingletonLifetimeCallbacks&);

// Complete BE72C0..BE733B: ECX actual FileStore provider, RET. Repeatedly
// erase resident tree+14's current minimum while current count+1C != 0.
// Retained owner+4 is read even though warning callee 4254B0 is only RET.
// No pending-tree changes, null-owner check, rollback or reference-count gate.
void clear_native_file_store_00be72c0(void* provider, NativeStringStorage&,
    NativeAdoptedSubstreamDispatch&, const SingletonLifetimeCallbacks&);

// NativeStringStorage::release is noexcept; throwing pool-getter/FH3/SEH,
// arbitrary stack aliasing, concurrent mutation and gameplay remain unproved.
} // namespace bsp
