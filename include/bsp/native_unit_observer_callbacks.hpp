#pragma once

#include "bsp/native_unit_observer_endpoint.hpp"

namespace bsp {

// Borrowed fields of the SAME existing plane owner bound by alias. Native ECX
// is alias.prefixes.callback_10 (unit+10h), not alias.canonical_unit. The host
// resolves these references from that owner; this is not a second plane state
// or permission to cast a semantic GameUnitSlot to the native object layout.
struct NativePlaneObserverCallbackView {
    NativeUnitObserverAlias alias;
    void* const volatile& squadron_9d4;
    const volatile std::uint8_t& byte_5c;
    const volatile std::uint8_t& byte_5d;
    const volatile std::uint8_t& byte_5e;
    const volatile std::uint8_t& byte_60;
};

class NativePlaneObserverCallbackAccess {
public:
    virtual ~NativePlaneObserverCallbackAccess() = default;
    // Invoke the delivered first endpoint's ACTUAL +4 getter once. Its return
    // is another actual observed prefix; do not substitute canonical identity.
    virtual NativeObserverOwnerStorage* observed_virtual_04(
        NativeObserverOwnerStorage& first) = 0;
    // The producer's actual unit +5Ch provider. Keep target nullable: native
    // falls through to query9 even after a null getter and dereferences null.
    // A native-valid integration supplies a nonnull entity; silently accepting
    // null as false changes that fault path. No universal class predicate.
    virtual bool observed_virtual_5c(NativeObserverOwnerStorage* target,
        std::uint32_t literal_kind) = 0;
    // Existing effective_game_mode_004bca50, bound to the current game owner
    // published by E188A8. Reuse simulation_gate's body when binding raw fields.
    virtual int effective_game_mode_004bca50() = 0;
    // Existing shared plane prepass, externally owned; preserves float input.
    // Resolve alias.canonical_unit through its existing owner mapping. Native
    // ECX is the whole raw plane (secondary-10h), stack is step, RET4.
    virtual void call_007c5ac0(const NativeUnitObserverAlias&, float step) = 0;
};

// Complete normal body 007C6E90..007C6F16. Native ECX=secondary, stack=first,
// RET4. New C++ interface, not a binary replacement or full plane simulation.
// Keeps call order, short circuits and repeated native flag loads.
void native_plane_observer_callback_007c6e90(
    NativePlaneObserverCallbackView, NativeObserverOwnerStorage& first,
    NativePlaneObserverCallbackAccess&);

// Borrow actual +4/+8 lvalues of each checked vector header. Header word0 and
// capacity are unconsumed. No vector owner, records or allocation is created.
// Owner+780h records have stride10h; owner+790h records have stride4Ch. Bind
// only after resolving the actual callback+10h alias to that SAME shipyard.
struct NativeShipyardObserverVectorView {
    std::byte* volatile& begin;
    std::byte* volatile& end;
};
struct NativeShipyardObserverCallbackView {
    NativeUnitObserverAlias alias;
    NativeShipyardObserverVectorView vector_780;
    NativeShipyardObserverVectorView vector_790;
};

// Complete reachable normal body 008455A0..0084566D, ECX=secondary, stack=first,
// RET4. Compares the delivered first directly (no getter), clears first match
// in each vector only: +Ch in10h record; +4/+8/+30h in4Ch record. Numeric opaque
// fields retain producer-established widths; no erase, notification or release.
// Native validation may return: checks retain saved iterators/end and reload
// actual headers in native order. Invalid memory faults/SEH are not emulated.
void native_shipyard_observer_callback_008455a0(
    NativeShipyardObserverCallbackView, NativeObserverOwnerStorage* first,
    ObserverLifetimeServices&);

// Separate complete 3-byte RET4 bodies, each selected by its actual leaf table.
// Their native stack word is discarded, no register/argument value is read,
// and no return value is defined. Parameterless source interfaces deliberately
// make no inferred ECX/type promise. Not defaults for other callback providers.
void native_unit_observer_noop_0080dfc0() noexcept;
void native_unit_observer_noop_00952050() noexcept;

} // namespace bsp
