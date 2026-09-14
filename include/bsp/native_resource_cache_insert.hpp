#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-cache insertion requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;
struct SingletonLifetimeCallbacks;

// Complete B7FF80..B8016B[492]. Native ECX actual tree; stack(output iterator,
// insert-left DWORD low byte,parent node,actual0Ch key/resource pair); EAX output,
// RET10h. Actual tree+4=head,+8=count; nodes use the integrated actual1Ch layout.
// Reject initial unsigned count>=15555554h with owning NativeAliasListLengthError
// carrying the exact19-byte "map/set<T> too long" message. Allocate node before
// incrementing CURRENT count, link against current head, rebalance, blacken
// current root. Output writes captured new node+4 before captured tree+0.
// No AddRef, null-allocation guard, inserted-node rollback, or old-key cleanup.
void* insert_native_resource_cache_at_00b7ff80(void* actual_tree,
    void* actual_iterator_output, std::uint32_t insert_left_word,
    void* actual_parent, const void* actual_source_pair,
    NativeStringRawPoolContext& strings);

// Complete B803B0..B804C3[276]. Native ECX actual tree; stack(result,pair);
// EAX result, RET8. Walk current links with stored-length0 empty and CRT
// case-insensitive ordering; retain insertion parent separately from predecessor.
// Equivalent keys return the existing node without allocation/replacement or
// resource reference changes. Result stores node+4, inserted byte+8, owner+0,
// in that order; other bytes remain untouched. Current iterator owner survives
// the existing predecessor's returning invalid-parameter boundary.
void* insert_native_resource_cache_unique_00b803b0(void* actual_tree,
    void* actual_result_output, const void* actual_source_pair,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Both source interfaces borrow actual storage and raw pool/lifetime cells.
// They add explicit services and host C++ exception transport; they are not
// original stack/register/FH3/SEH ABI bridges. No projected tree, manager glue,
// ownership policy for mapped resources, or validation of arbitrary pointers.
// Evidence and retained source boundaries: NATIVE_RESOURCE_CACHE_INSERT_BO.md.
} // namespace bsp
