#pragma once

#include "bsp/native_pending_entity_owners.hpp"
#include "bsp/observer_lifetime.hpp"

namespace bsp {

// Borrowed lvalues of the SAME actual entity. No flags snapshot, new unit,
// storage cast, or owner. Resolve only after the captured lock is entered.
// The parent pointer is read once, then its actual flag lvalues are resolved.
struct NativePendingEntityCancelView {
    void*& parent_3c;
    std::uint8_t& active_5c;
    std::uint8_t& simulate_5d;
    std::uint8_t& pending_5e;
    std::uint8_t& killed_5f;
    std::uint8_t& destroyed_60;
};

// Explicit required native-provider contracts, never default/no-op services.
// Context, entities, list owners and captured section must remain alive for
// the whole operation. Resolve returns actual lvalues without side effects.
struct NativePendingEntityCancelProviders {
    void* context;
    // Actual 009248D0 owner (same raw8 layout as NativeObserverLockOwner).
    // Its section_04 is captured once and used for both entry and exit.
    NativeObserverLockOwner* (*lock_owner_009248d0)(void*);
    NativePendingEntityCancelView (*resolve_entity)(void*, void* identity);
    // Actual 00781260: ECX=list, stack=&entity, RET4. Capture *value once,
    // scan the entire ring, unlink/free EVERY matching node, decrement actual
    // count for each; retain sentinel, other nodes, payloads and allocator word.
    // Preserve checked-iterator/returning-validation behavior. A different STL
    // layout, remove-first, private list, or ignored call does not satisfy it.
    // No canonical source provider has been proved; this component supplies none.
    void (*remove_all_matching_00781260)(void*, NativePendingEntityListStorage&,
        void* const* value);
};

// Complete normal-path 00925A00[138]: native ECX=entity, bare RET. Gate on
// parent+3C; no parent or parent 5C!=0/5D==0/60==0/5E==0 permits cancellation.
// Kill-remove then clear5F; capture60 then clear5E; conditional destroy-remove
// then clear60; finally clear5D. Preserve5C and leave the captured section.
// Uses the caller's actual F899A8/F899B4 owners and byte lvalues. Required
// providers are validated before any native operation; missing bindings throw
// logic_error (source-interface error, not an added native branch).
void cancel_native_pending_entity_00925a00(void* actual_entity,
    NativePendingEntityOwners&, const NativePendingEntityCancelProviders&);

// New source C++ ABI; not a binary replacement. The real getter can throw
// before section capture/entry, and that exception propagates. This routine
// has no native EH frame: no automatic section release or flag/list rollback
// is added if a later provider throws. Native exception transport, corruption
// recovery, asynchronous faults and concurrent destruction remain unproved.
} // namespace bsp
