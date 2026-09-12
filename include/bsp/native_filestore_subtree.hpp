#pragma once

namespace bsp {
class NativeStringStorage;
class NativeAdoptedSubstreamDispatch;
struct SingletonLifetimeCallbacks;

// Complete BE5D70 including state0 -> CC6C40 -> 41DD20 unwind action.
// Native ECX points to the actual 0Ch payload {length,data,retained owner}; RET.
// Capture owner+8, real InterlockedDecrement(owner+4), then current table/slot0
// dispatch only at zero. Clear CURRENT payload+8 after successful dispatch.
// Normal and throwing dispatch paths release the CURRENT string header once;
// throwing dispatch leaves payload+8 unchanged. String storage release is
// noexcept, so throwing lazy-pool-getter/FH3/SEH identity remains a boundary.
// Use NativeVfsRuntimeBindings for evidenced numeric D642C0/D68DB0/D691B0
// stream profiles; the callable adapter requires actual original-ABI entries.
// Other zero-reference targets need a concrete binding; none is guessed here.
void destroy_native_file_store_payload_00be5d70(void* actual_payload,
    NativeStringStorage&, NativeAdoptedSubstreamDispatch&);

// Complete 82/61-byte flows, including raw post-free continuations. Native
// ECX tree, stack node, RET4; these new C++ interfaces add explicit services.
// Nodes are actual 1Ch allocations: links +0/+4/+8, payload +0C, nil byte +19.
// Recurse right, capture left, release payload, free current, then iterate left.
// No tree head/count updates, sentinel free, balancing, rollback or null guard.
// BE66C0 releases only the pending key string; BE6720 releases resident payload.
void erase_native_file_store_pending_subtree_00be66c0(void* actual_tree,
    void* actual_node, NativeStringStorage&);
void erase_native_file_store_resident_subtree_00be6720(void* actual_tree,
    void* actual_node, NativeStringStorage&, NativeAdoptedSubstreamDispatch&);

// Complete distinct 99-byte iterator bodies: ECX actual {tree,node}, RET.
// Both advance in order. Null tree calls the returning BF6713 boundary before
// reloading node; a nil current node tail-calls it and returns. The upward walk
// publishes each traversed parent to CURRENT iterator+4. Owner is not rewritten.
// Reuses the NativeFileStoreNameIterator layout; arbitrary callable validation
// and C++ interfaces do not establish original binary/EH ABI compatibility.
void advance_native_file_store_resident_iterator_00be4c30(void* actual_iterator,
    const SingletonLifetimeCallbacks&);
void advance_native_file_store_pending_iterator_00be4e40(void* actual_iterator,
    const SingletonLifetimeCallbacks&);
} // namespace bsp
