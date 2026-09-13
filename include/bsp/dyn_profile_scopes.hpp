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
DynProfileNodeStorage* dyn_profile_node_append_child_00c50390(
    DynProfileNodeStorage& parent, const char* name, std::uint32_t id,
    const AvoidZoneDynHullMemory& memory);

//00C57020..00C57063: EDI scope, EAX id, stack name; EAX scope; RET4.
//Cache key is id (valid owner indices0..35), independent of name/parent. On
//first use create a child of current; on every use link node+0 to current,
//publish it as profile+4, then execute actual RDTSC. Owners/services must live
//through the matching leave. A cached node's child-vector membership stays
//with its first parent even when its active parent changes on later entry.
DynProfileScopeStorage* dyn_profile_scope_enter_00c57020(DynProfileScopeStorage&,
    std::uint32_t id, const char* name, const DynProfileScopeContext&);

//Complete inline fragment00C5BCD7..00C5BD03 within00C5BB30, not a standalone
//native function. Scope was native ESP+4C. Execute RDTSC, update node last/sum
//and count modulo64/32, then pop the actual global current node through+0.
//Scope+8/+C remain untouched. Requires balanced nesting; does not validate or
//replace current with the scope's node. Native EH unwinding is separate.
void dyn_profile_scope_leave_00c5bcd7(const DynProfileScopeStorage&,
    const DynProfileScopeContext&);
} // namespace bsp
