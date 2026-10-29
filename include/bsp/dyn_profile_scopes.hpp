#pragma once

#include "bsp/dyn_engine_runtime.hpp"

namespace bsp {
// Native C57020 writes +0/+4 timestamp and +10 node; +8/+C stay untouched.
struct alignas(4) DynProfileScopeStorage { unsigned char bytes[0x14]; };

struct DynProfileScopeContext {
    AvoidZoneDynHullMemory memory;
    void* const* profile_slot_0109e9f8; // actual published 9Ch profile owner
};

//00C50390..00C50469: ESI parent48h node; stack name/id; EAX appended child;
//RET8. Complete normal child allocation/construction/vector append. This is
//not a destructor, name lookup, scope activation, or profile-global mutation.
//This existing overload remains normal-only; no cleanup binding is assumed.
DynProfileNodeStorage* dyn_profile_node_append_child_00c50390(
    DynProfileNodeStorage& parent, const char* name, std::uint32_t id,
    const AvoidZoneDynHullMemory& memory);

// Distinct original BF65AC raw-allocation release, not memory.release/BF6989.
// Required genuine matching-heap provider; it must return normally. No default.
using DynProfilePartialNodeDelete = void (__cdecl*)(void* original_allocation);

// Additive cleanup-qualified C++ interface. memory.release must bind the actual
// BF6989 vector release returning normally; delete_partial_00bf65ac is separate.
// The real qualified C44000 constructor first performs its own A900 cleanup.
// Only a constructor exception then releases the captured raw48h allocation
// and rethrows. Allocation and post-constructor parent growth are outside that
// catch: no parent rollback or fully constructed orphan cleanup is fabricated.
// Original FH3/SEH/fault/frame transport and caller adoption remain separate.
DynProfileNodeStorage* dyn_profile_node_append_child_00c50390(
    DynProfileNodeStorage& parent, const char* name, std::uint32_t id,
    const AvoidZoneDynHullMemory& memory,
    DynProfilePartialNodeDelete delete_partial_00bf65ac);

//00C57020..00C57063: EDI scope, EAX id, stack name; EAX scope; RET4.
//Cache key is id (valid owner indices0..35), independent of name/parent. On
//first use create a child of current; on every use link node+0 to current,
//publish it as profile+4, then execute actual RDTSC. Owners/services must live
//through the matching leave. A cached node's child-vector membership stays
//with its first parent even when its active parent changes on later entry.
DynProfileScopeStorage* dyn_profile_scope_enter_00c57020(DynProfileScopeStorage&,
    std::uint32_t id, const char* name, const DynProfileScopeContext&);

// Additive qualified child-construction path. Required matching raw-node
// BF65AC provider; null is rejected before any profile/scope access or mutation.
// The cold append selects the real five-argument C50390. The existing overload
// stays normal-only; ScopeContext/ScopeStorage layouts are unchanged. No Root,
// native collision/world/solver caller adoption or original FH3/SEH follows.
DynProfileScopeStorage* dyn_profile_scope_enter_00c57020(DynProfileScopeStorage&,
    std::uint32_t id, const char* name, const DynProfileScopeContext&,
    DynProfilePartialNodeDelete delete_partial_00bf65ac);

//Complete inline fragment00C5BCD7..00C5BD03 within00C5BB30, not a standalone
//native function. Scope was native ESP+4C. Execute RDTSC, update node last/sum
//and count modulo64/32, then pop the actual global current node through+0.
//Scope+8/+C remain untouched. Requires balanced nesting; does not validate or
//replace current with the scope's node. Native EH unwinding is separate.
void dyn_profile_scope_leave_00c5bcd7(const DynProfileScopeStorage&,
    const DynProfileScopeContext&);
} // namespace bsp
