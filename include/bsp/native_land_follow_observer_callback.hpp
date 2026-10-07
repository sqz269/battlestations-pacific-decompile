#pragma once

#include "bsp/native_land_state_entries.hpp"
#include "bsp/observer_event_producer.hpp"

namespace bsp {

class NativeLandFollowObserverCallbackBindings {
public:
    explicit NativeLandFollowObserverCallbackBindings(NativeObserverLifetime& lifetime) noexcept
        : lifetime_(lifetime) {}
    virtual ~NativeLandFollowObserverCallbackBindings() = default;
    // REQUIRED PURE actual callback18 -> SAME live >=98h Follow root aliases.
    // Watch2C MUST be the actual FIRST endpoint identity, without semantic unit
    // translation, represented reads, callbacks, allocation/default/cache.
    virtual NativeLandFollowEntryView callback_fields(NativeObserverOwnerStorage&) = 0;
    // REQUIRED COMPLETE actual delivered FIRST primary table+4 getter once,
    // preserving its captured target/result/effects. FIRST is NEVER the edge.
    // Existing42B970/522E90 bodies apply ONLY to actual proved profile domains.
    virtual NativeObserverOwnerStorage* observed_virtual_04(NativeObserverOwnerStorage&) = 0;
    // REQUIRED COMPLETE actual delegates for other04 and all08 routes.
    virtual void other_callback_virtual_04(NativeObserverOwnerStorage& callback,
        std::uint32_t captured_table, NativeObserverOwnerStorage& first) = 0;
    virtual void callback_virtual_08(NativeObserverOwnerStorage& callback,
        std::uint32_t captured_table, NativeObserverOwnerStorage& first) = 0;
    // Borrow only the genuine existing context. Unregister is the DIRECT
    // complete006952A0 operation, never a substituted operation callback.
    NativeObserverLifetime& observer_lifetime() noexcept { return lifetime_; }
private:
    NativeObserverLifetime& lifetime_;
};

// COMPLETE006CEED0..006CEEF7,39B/14instructions. Original ECX=callback18,
// stack=original FIRST, RET4; no defined return contract. Getter once ->FRESH
// callback14/root2C compare ->clear BEFORE actual6952A0(returned FIRST,SAME
// callback). Matching identity MUST be nonnull and denote the actual FIRST
// prefix. Native NULL==NULL passes NULL into6952A0: invalid domain, NO guard
// or fallback is invented. A null mismatch executes no unregister.
void native_land_follow_observer_callback_006ceed0(
    const NativeLandFollowEntryView&, NativeObserverOwnerStorage& first,
    NativeLandFollowObserverCallbackBindings&);

// CONNECTED opt-in existing SOURCE access for696330->695F90->693550 delivery.
// CapturedCF89B4 slot04 selects this body on actual callback18/watch2C. The
// destructor's CF8900 is a DIFFERENT raw profile. Other04/all08 stay required.
// These raw tables remain UNCALLABLE; this is not a native class/table bridge.
ObserverEndpointCallbackAccess native_land_follow_observer_callback_access(
    NativeLandFollowObserverCallbackBindings&) noexcept;

// Require stable actual root/FIRST/callback storage, coherent registered pairs
// and genuine observer manager/current lock/dispatch/CRT lifetime context.
// Getter/delegates must execute their whole actual selected bodies. Ordinary
// successful returns only; no structural reentry/concurrency, invalidation,
// faults/privateEH, complete constructor/entry/arena/profile/world/classABI or
// game lifetime binding. Alias mismatch is Source admission error, not native
// policy. No receiver free or whole approach/task retirement is added.

} // namespace bsp
