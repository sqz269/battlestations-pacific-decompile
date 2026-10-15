#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene-record map node allocation requires MSVC Win32.
#endif

#include "bsp/native_resource_registry_node_leaves.hpp"
#include "bsp/native_string.hpp"

namespace bsp {

// Exact existing live-cell contracts. Incoming left/parent/right/pair/color,
// the distinct constructor argument cells, and allocation/placement cleanup
// cells stay address-stable through completion or unwind. Metadata and fresh
// Acquired are disjoint from payload, argument and local cells. Current payload
// aliases are admitted; original cross-provider private-stack aliases are not.
using NativeSceneRecordMapNodeAllocationFrame = NativeResourceRegistryNodeAllocationFrame;
using NativeSceneRecordMapNodeAcquired = NativeResourceRegistryNodeAcquired;

// Complete 0046A340 plus its qualified Source cleanup/catch scopes. Incoming
// native ECX/EDX are unused; five stacked DWORDs are left,parent,right,pair,
// color; RET14/EAX captured allocation. Allocate actual1Ch BEFORE reading the
// current argument cells. Bind the exact existing raw B1AD10 factory: genuine
// malloc/_callnewh retry/bad_alloc service and the matching current-CRT free,
// canonical raw B1ACA0 constructor (equivalent to00469F50), current cleanup
// cell reads, free and rethrow. No snapshot, callback, partial-key destructor,
// or source null-allocation fallback is introduced. The canonical null branch
// is retained; both conforming allocator services return storage or throw.
//
// Require a fresh zero/default-initialized Acquired. It records failure
// frontiers and addresses, not a guarantee that every captured block was freed.
// Normal return is the CAPTURED allocation even if cleanup cells have changed.
// New ordinary C++/explicit-frame/context interface; original CRT exception,
// global handler, native FH3/private-frame ABI and game execution are separate.
// Evidence: docs/GOAL_OCT15_SCENE_MAP_NODE_ALLOCATE.md.
void* allocate_native_scene_record_map_node_0046a340(
    const NativeSceneRecordMapNodeAllocationFrame& frame,
    NativeStringRawPoolContext& strings, NativeSceneRecordMapNodeAcquired& acquired);

} // namespace bsp
