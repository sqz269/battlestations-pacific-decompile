#pragma once

#include "bsp/native_land_moveto_constructor.hpp"
#include "bsp/observer_event_producer.hpp"

namespace bsp {

class NativeLandMoveToObserverCallbackBindings {
public:
    virtual ~NativeLandMoveToObserverCallbackBindings() = default;
    // REQUIRED PURE actual callback18 -> SAME >=3Ch root/field aliases. No
    // represented loads, callbacks/native calls, allocation/default/cache or
    // semantic unit/endpoint translation. Returned aliases remain live.
    virtual NativeLandMoveToConstructorView callback_fields(
        NativeObserverOwnerStorage& callback) = 0;
    // REQUIRED COMPLETE actual delivered FIRST vtable+4 call once, including
    // its captured real target and side effects. This is FIRST, NEVER edge.
    // Existing0042B970 identity/00522E90 null bodies are admitted ONLY when
    // selected by the actual proved profile. No generic identity/null policy.
    virtual NativeObserverOwnerStorage* observed_virtual_04(
        NativeObserverOwnerStorage& first) = 0;
    // REQUIRED COMPLETE actual delegates for all other04 and all08 routes.
    // No noop/default profile or callback is supplied by this adapter.
    virtual void other_callback_virtual_04(NativeObserverOwnerStorage& callback,
        std::uint32_t captured_table, NativeObserverOwnerStorage& first) = 0;
    virtual void callback_virtual_08(NativeObserverOwnerStorage& callback,
        std::uint32_t captured_table, NativeObserverOwnerStorage& first) = 0;
};

// COMPLETE009BDEB0..009BDECE30B/11instructions. Original ECX=callback18,
// stacked original FIRST, RET4, no defined return contract. Actual getter once
// ->FRESHcallback14(ROOT2C) comparison ->null store iff equal, evenNULL==NULL.
// No unregister/edge erase/delete/queue retirement or lifetime proof.
void native_land_moveto_observer_callback_009bdeb0(
    const NativeLandMoveToConstructorView&, NativeObserverOwnerStorage& first,
    NativeLandMoveToObserverCallbackBindings&);

// Opt-in CONNECTED typed SOURCE access for real696330->695F90->693550 delivery.
// CapturedD20AD4 slot04 invokes the complete body above on SAME callback18;
// remaining04/08 routes call REQUIRED actual delegates. This is the existing
// SOURCE callback/context ABI, NOT a fabricated callable native profile table.
ObserverEndpointCallbackAccess native_land_moveto_observer_callback_access(
    NativeLandMoveToObserverCallbackBindings&) noexcept;

// Stable live constructor-root/FIRST/callback and genuine observer lock/global
// lifetime/dispatch storage are mandatory. Mapping is pure, getter/delegates
// execute their full actual selected bodies; unknown world bindings remain
// external. No structural reentry/concurrency/alias invalidation/fault/privateEH,
// whole profile/arena/controller/native ABI/game lifetime claim. A mismatch is
// a SOURCE admission error, not an invented native guard/fallback. Raw tables
// remain UNCALLABLE; clearing2C neither removes registrations nor frees owners.

} // namespace bsp
