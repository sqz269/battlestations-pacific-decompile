#pragma once
#include "bsp/avoid_zone_dyn_hull.hpp"

namespace bsp {
// Raw Win32 records: manager is world+448h {world,data,size,capacity,count};
// each 0Ch group is {manifold-pointer data,size,capacity}. The same native
// allocator used by the world owns all allocations. CALL-site IDs are evidence.
struct NativeDynContactGroupCalls {
    virtual ~NativeDynContactGroupCalls()=default;
    virtual void* allocate_00bf55be(std::uint32_t site,std::uint32_t bytes,
        const AvoidZoneDynHullMemory&);
    virtual void free_00bf6989(std::uint32_t site,void*,const AvoidZoneDynHullMemory&);
};
struct NativeDynContactGroupContext {
    const AvoidZoneDynHullMemory& memory;
    NativeDynContactGroupCalls& calls;
};
// Complete normal bodies; new C++ interfaces, not native register-ABI entry
// points. Valid records/successful allocation are required. Native EH frames,
// exception cleanup, allocation failure and concurrent mutation are not supplied.
void* native_dyn_group_copy_0040f810(void* destination,const void* source,
    const NativeDynContactGroupContext&); // ESI destination, EBX source, RET/EAX
void native_dyn_reset_group_marks_00c36ac0(void* manager); // ECX, RET
void native_dyn_append_manifold_00c36b60(void* group,void* manifold,
    const NativeDynContactGroupContext&); // ESI group, stack manifold, RET 4
void native_dyn_clear_contact_groups_00c3f410(void* manager,
    const NativeDynContactGroupContext&); // ESI, RET; retains outer capacity/data
void native_dyn_create_contact_groups_00c4b610(void* manager,
    const NativeDynContactGroupContext&); // stack manager, RET 4
void native_dyn_sleep_contact_groups_00c4b550(void* manager,
    const NativeDynContactGroupContext&); // stack manager, RET 4; clears groups
} // namespace bsp
