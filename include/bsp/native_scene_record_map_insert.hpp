#pragma once

#include "bsp/native_scene_record_map_node_allocate.hpp"

namespace bsp {

// Explicit backing for the original driver's private DWORD/iterator/call slots.
// Metadata itself must be stable and disjoint from all payload and live cells.
// Each backing region has the stated size/alignment and remains valid throughout
// reached providers; upper flag bytes are initialized by the caller and retained.
// Allocation frame cells follow the allocator's separate live-cell contract.
struct NativeSceneRecordMapInsertFrame {
    void* descent_flag4;
    void* predecessor8;
    void* linker_arguments16;
    const NativeSceneRecordMapNodeAllocationFrame& allocation;
};

// Complete0046AD70. Native ECX map; stacked output,left-word,parent,pair;
// RET10h/EAX output. Here actual_arguments16 holds those four LIVE DWORDs.
// Count >=15555554h throws the existing owning native-storage length error.
// Allocate, link, rebalance, blacken current root; then reread output and publish
// NODE before OWNER. Only the reached nonroot path reads left's LOW byte.
void* link_native_scene_record_map_node_0046ad70(void* actual_map,
    void* actual_arguments16, const NativeSceneRecordMapNodeAllocationFrame&,
    NativeStringRawPoolContext&, NativeSceneRecordMapNodeAcquired&);

// Complete0046B340. Native ECX map, stacked hidden output,pair; RET8/EAX output.
// actual_arguments8 contains those two live DWORDs. Pair is captured at entry;
// output is read at publication. Result is actual12h {owner,node,inserted-byte},
// with final three padding bytes untouched. Duplicate does not allocate/retain.
// Reached allocation requires fresh acquired state; duplicate leaves it alone.
void* insert_native_scene_record_map_pair_0046b340(void* actual_map,
    void* actual_arguments8, const NativeSceneRecordMapInsertFrame&,
    NativeStringRawPoolContext&, NativeSceneRecordMapNodeAcquired&);

// These are ordinary Source interfaces with explicit frame/context backing.
// They do not establish original physical stack aliases, FH3/CRT exception ABI,
// register residue, binary replacement, application adoption or game behavior.
} // namespace bsp
