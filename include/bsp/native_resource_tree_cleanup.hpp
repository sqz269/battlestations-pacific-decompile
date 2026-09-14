#pragma once
#include "bsp/native_resource_cache_erase.hpp"

namespace bsp {
// Raw parser/cache trees: debug word0, head4, count8. Nodes1Ch contain
// links0/4/8, owned keyC/10, BORROWED mapped pointer14, color18 and nil19.
// These routines never destroy/release the mapped parser or resource.

// Complete B7CA20[28]/B7CA40[27]: ECX node, EAX extreme, RET.
void* maximum_native_resource_parser_node_00b7ca20(void*) noexcept;
void* minimum_native_resource_parser_node_00b7ca40(void*) noexcept;
// Complete B7CF80[99]: ECX actual8h iterator, RET. Returning validation and
// current node reloads match the existing cache specialization.
void increment_native_resource_parser_iterator_00b7cf80(void*, const SingletonLifetimeCallbacks&);
// Complete B7F790[716]: ECX tree; stack output, first owner/node; EAX output,
// RET0Ch. Same raw key ownership, transplant/rebalance and error as B7FA60.
void* erase_native_resource_parser_iterator_00b7f790(void* tree, void* output,
    NativeResourceCacheIteratorStorage first, NativeStringRawPoolContext&,
    const SingletonLifetimeCallbacks&);

// Complete B7F730/B7FF20[82 each]: ECX tree, stacked node, RET4.
// Recurse right, capture key data and left link, return captured key storage
// through the CURRENT pool, free captured node, continue left until nil.
void destroy_native_resource_parser_subtree_00b7f730(void* tree, void* node,
    NativeStringRawPoolContext&);
void destroy_native_resource_cache_subtree_00b7ff20(void* tree, void* node,
    NativeStringRawPoolContext&);

// Complete B80500/B805D0[201 each, including one unreachable alignment byte]:
// ECX tree, stack output, first owner/node, last owner/node; EAX output,
// RET14h. Full range destroys root and resets CURRENT head links/count.
// Partial range advances its local first iterator BEFORE erasing the captured
// old pair. Preserve pre-validation begin/head captures and returning handlers.
void* erase_native_resource_parser_range_00b80500(void* tree, void* output,
    NativeResourceCacheIteratorStorage first, NativeResourceCacheIteratorStorage last,
    NativeStringRawPoolContext&, const SingletonLifetimeCallbacks&);
void* erase_native_resource_cache_range_00b805d0(void* tree, void* output,
    NativeResourceCacheIteratorStorage first, NativeResourceCacheIteratorStorage last,
    NativeStringRawPoolContext&, const SingletonLifetimeCallbacks&);

// Complete B80E90/B80ED0[52 each]: ECX tree, RET. Erase captured [begin,end),
// free CURRENT head, zero current head/count; preserve debug word0.
void destroy_native_resource_parser_tree_00b80e90(void* tree,
    NativeStringRawPoolContext&, const SingletonLifetimeCallbacks&);
void destroy_native_resource_cache_tree_00b80ed0(void* tree,
    NativeStringRawPoolContext&, const SingletonLifetimeCallbacks&);

// Parser specializations reuse instruction- and EH-equivalent actual cache
// source. Source reuse does not invent native call edges. New C++ interfaces
// preserve source exceptions; native ABI/FH3/private spills and faults differ.
} // namespace bsp
