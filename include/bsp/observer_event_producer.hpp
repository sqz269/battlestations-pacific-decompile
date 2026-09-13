#pragma once

#include "bsp/observer_lifetime.hpp"
#include <cstdint>

namespace bsp {

// 00695F90 delivers the original observed endpoint itself. No event object is
// allocated or copied. The callback receiver is each actual edge+8 owner.
// Dispatch these captured table identities through their real slot targets;
// these providers are not fabricated source vtables or default handlers.
struct ObserverEndpointCallbackAccess {
    void* context;
    void (*callback_virtual_04)(void*, NativeObserverOwnerStorage& callback_owner,
        std::uint32_t captured_table, NativeObserverOwnerStorage& first);
    void (*callback_virtual_08)(void*, NativeObserverOwnerStorage& callback_owner,
        std::uint32_t captured_table, NativeObserverOwnerStorage& first);
};

// All three members are borrowed. Use the same actual E198E4 publication and
// lifetime/lock owners as registration and destruction. The dispatch owner
// must already be published by00CCD6A0 and remain alive through notification.
struct ObserverEventDeliveryContext {
    NativeObserverLifetime& lifetime;
    NativeObserverDispatchStorage* volatile& global_00e198e4;
    const ObserverEndpointCallbackAccess& callbacks;
};

// Complete00693550..55A and00693560..56A. Native ECX=first, EDX=callback owner,
// push first, ECX=callback owner, call captured table slot04/08, RET. The
// actual callback consumes its one stack argument (RET4). No event semantics
// or return value are invented; the dispatch caller ignores callback EAX.
void invoke_observer_slot04_00693550(NativeObserverOwnerStorage& first,
    NativeObserverOwnerStorage& callback_owner, const ObserverEndpointCallbackAccess&);
void invoke_observer_slot08_00693560(NativeObserverOwnerStorage& first,
    NativeObserverOwnerStorage& callback_owner, const ObserverEndpointCallbackAccess&);

// Complete native tail selectors00696330/40, ten bytes each. ECX=first;
// EDX=00693550/60; JMP00695F90. Directly reuse the recovered dispatcher with
// actual storage, so there is no substitute queue or scheduler callback.
void notify_observer_slot04_00696330(NativeObserverOwnerStorage&, ObserverEventDeliveryContext&);
void notify_observer_slot08_00696340(NativeObserverOwnerStorage&, ObserverEventDeliveryContext&);

// Partial source API for00925C45..00925C71 only: capture actual lock, enter/
// increment if nonnull, sample unsigned count+8>0, decrement/leave the captured
// lock, return the saved predicate. No notification is performed here. This
// also represents the identical inlined sample009274DE..00927509.
bool sample_observer_endpoint_presence_00925c45(
    NativeObserverOwnerStorage&, NativeObserverLifetime&);

// Complete normal00925C40..C83 and00925C90..CD3. ECX=first, RET or tail-JMP
// the selected notifier. The saved presence predicate gates dispatch AFTER
// releasing the sampling lock; the dispatcher then captures its own lock.
void notify_observer_slot04_if_present_00925c40(
    NativeObserverOwnerStorage&, ObserverEventDeliveryContext&);
void notify_observer_slot08_if_present_00925c90(
    NativeObserverOwnerStorage&, ObserverEventDeliveryContext&);

// Complete three-byte slot04 targets. ECX is returned in EAX by0042B970;
//00522E90 returns zero independently of ECX; both plain RET. These are actual
// bodies, not a universal getter policy. D19120/D192E0 and the21 verified unit
// primary tables select identity. D23084, installed by00A2D440 before its
// notification, selects null. Bind the actual producer's table/target.
NativeObserverOwnerStorage* observer_endpoint_identity_0042b970(
    NativeObserverOwnerStorage*) noexcept;
NativeObserverOwnerStorage* observer_endpoint_null_00522e90(
    NativeObserverOwnerStorage*) noexcept;

// New C++ interfaces, not native ABI/FH3 replacements. Raw endpoint prefixes
// must come from actual owners or explicit aliases; never reinterpret semantic
// unit metadata as an observer prefix. Producer/queue teardown outside the
// listed wrappers and arbitrary callback vtables remain externally owned.
} // namespace bsp
