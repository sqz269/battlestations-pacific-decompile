#pragma once

#include "bsp/native_pending_entity_owners.hpp"
#include "bsp/native_scene_lifecycle_notify.hpp"

namespace bsp {

class NativePendingEntityDrainAccess {
public:
    virtual ~NativePendingEntityDrainAccess() = default;
    // REQUIRED complete00926FA0/009269B0 copy-construction contract. Native
    // ECX=raw destination0Ch, stack=source header, EAX=destination, RET4.
    // Allocate a distinct0Ch sentinel, publish destination+4, count=0, and
    // copy source payload pointers in next-link order using the full checked
    // range/allocator/rollback behavior. Preserve destination+00 and sentinel
    // payload+08. Copy failure performs its own incomplete-object cleanup;
    // the caller owns only previously completed copies. No implicit lock.
    // This source component does not implement or default the library helpers.
    virtual void copy_list_00926fa0(NativePendingEntityListStorage& destination,
        NativePendingEntityListStorage& source) = 0;
    // Side-effect-free lookup of the SAME existing canonical unit's actual
    // observer alias and byte lvalues. The borrowed binding survives callbacks.
    // No allocation, endpoint fabrication or semantic-unit layout cast.
    virtual NativeSceneLifecycleView resolve_unit(void* canonical_unit) = 0;
    // Dispatch the actual producer-selected slot74 on this same unit, using
    // the profile captured immediately before the call. No universal default.
    virtual void call_virtual_74(const NativeUnitObserverAlias&,
        std::uint32_t captured_profile) = 0;
};

struct NativePendingEntityDrainContext {
    NativePendingEntityOwners& owners;
    NativeSceneLifecycleContext& lifecycle;
    NativePendingEntityDrainAccess& access;
};

// Complete normal009273A0..009275D1 (562-byte PE span). Native no inputs,
// no stack arguments, RET. New C++ source interface; NOT a native FH3 ABI.
// Recheck actual destroy/kill counts; copy BOTH before clearing either;
// detach/free original nodes while preserving global sentinels; visit copied
// destroy FIFO via current slot74, then copied kill FIFO via the complete U
// lifecycle sequence. Callback requeues are drained on the next outer pass.
// Scratch teardown is kill then destroy, including their sentinels. Native
// K-sentinel free -> capture D-head -> clear K-head ordering is retained.
// No pending-list lock or added synchronization is taken.
void drain_native_pending_entities_009273a0(NativePendingEntityDrainContext&);

// Copy and virtual/renderer providers are required external contracts. Inline
// list mechanics operate actual raw owner/scratch storage, not private queues.
// C++ exceptions destroy only completed scratch copies in reverse order using
// canonical4C5940 (proved alias of924A70). Original EH mapDDA1F4/FH3/SEH,
// hardware faults and mutable native frame aliases remain unproved. Stored
// Ghidra bodies omit free continuations; complete bytes support those regions.
// See docs/NATIVE_PENDING_ENTITY_DRAIN.md. No gameplay/runtime binding here.
} // namespace bsp
