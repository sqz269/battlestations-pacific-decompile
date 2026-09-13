#pragma once

#include "bsp/native_pending_entity_owners.hpp"
#include "bsp/observer_lifetime.hpp"

namespace bsp {

// Borrow fields of the SAME actual entity denoted by identity. The resolver
// must be a pure mapping to live lvalues: no copies, sidecar owners, allocation,
// native-layout casts of semantic class metadata, or callback effects. Parent,
// child and sibling identities are the actual pointers stored in those fields.
struct NativePendingProducerEntityView {
    void* identity;
    void* const volatile& parent_3c;
    void* const volatile& next_sibling_44;
    void* const volatile& first_child_48;
    volatile std::uint8_t& byte_5f;
    volatile std::uint8_t& byte_60;
    volatile std::uint32_t& cause_70;
};

class NativePendingEntityProducerAccess {
public:
    virtual ~NativePendingEntityProducerAccess() = default;
    virtual NativePendingProducerEntityView resolve_entity(void* actual_entity) = 0;
    // Actual pending lock getter, no synthetic/default owner. Null return has
    // native invalid-memory behavior on owner+4; a null section is permitted.
    // Compatible with NativePendingEntityLockOwner's existing storage alias.
    virtual NativeObserverLockOwner* lock_owner_009248d0() = 0;
    // Required ACTUAL current entity vtable providers; preserve all arguments.
    virtual bool child_virtual_78(void* child, void* parent) = 0;
    virtual void destroy_virtual_70(void* entity, std::uint32_t recurse) = 0;
    // Concrete canonical reuse, not a new library port. Native ECX=list
    // (unused in callee), stack next/previous/source-cell, RET0C, EAX node.
    // Default implementation calls existing create_effect_deletion_node_008665f0;
    // complete51-byte alias proof is in reports/pending_entity_library_reuse.json.
    // Provider allocates/initializes the actual 0Ch node and must preserve its
    // native failure behavior. Source cell is read after allocation.
    virtual NativePendingEntityNode* call_00924b10(
        NativePendingEntityListStorage&, NativePendingEntityNode* next,
        NativePendingEntityNode* previous, const void* source_cell);
    // Concrete reuse of existing grow_effect_deletion_list_count_008675e0.
    // Complete147-byte and exception-graph alias proof is in the same report;
    // no new generic STL length guard/count implementation is introduced.
    // Native ECX=list, increment stack DWORD, RET4. Unsigned 3FFFFFFF-count
    // test; count overflow throws before link publication. Do not silently cap.
    virtual void call_009267f0(NativePendingEntityListStorage&,
        std::uint32_t increment);
};

// Complete normal body00926C80..00926D8A: native ECX=actual entity, stack
// recurse DWORD but only low byte tested, RET4, no defined return. Capture the
// actual9248D0 owner's section once, publish flags/cause, query actual children,
// allocate then grow count then publish captured-head/current-node links.
void native_pending_entity_destroy_00926c80(NativePendingEntityOwners&,
    void* actual_entity, std::uint32_t recurse, NativePendingEntityProducerAccess&);

// Complete normal body00926D90..00926E7D: native ECX=actual entity, stack raw
// cause DWORD, RET4. Stored7 becomes2; raw cause survives DIRECT recursive
// 926D90 child calls (not virtual dispatch). Both methods retain live sibling
// reloads after callbacks. Existing5F/60 gates still acquire/release the lock.
void native_pending_entity_kill_00926d90(NativePendingEntityOwners&,
    void* actual_entity, std::uint32_t cause, NativePendingEntityProducerAccess&);

// A C++ exception releases only the captured tracked section; flags, callback
// effects and allocated unlinked nodes are not rolled back. Native state0
// unwind funcletsCA6DC0/CA6DE0 only invoke411EE0 on that guard. New C++ ABI;
// full native FH3/SEH, hardware faults and game runtime behavior are not supplied.
} // namespace bsp
