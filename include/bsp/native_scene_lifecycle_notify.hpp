#pragma once

#include "bsp/native_unit_observer_endpoint.hpp"
#include "bsp/observer_event_producer.hpp"

namespace bsp {

// Borrow the SAME unit's established observer alias and actual flag lvalues.
// A semantic unit supplies explicit aliases to its existing fields; it is not
// reinterpreted as a native object. No flag snapshot, endpoint or owner is made.
struct NativeSceneLifecycleView {
    NativeUnitObserverAlias alias;
    volatile std::uint8_t& byte_5c;
    volatile std::uint8_t& byte_5d;
    volatile std::uint8_t& byte_5e;
    volatile std::uint8_t& byte_5f;
    volatile std::uint8_t& byte_60;
};

// Required actual producer-selected virtual targets. The captured table word
// selects the target; providers must not reload the unit's current table or
// substitute canonical identity as a render handle. No universal defaults.
class NativeSceneLifecycleAccess {
public:
    virtual ~NativeSceneLifecycleAccess() = default;
    virtual void* render_virtual_18(const NativeUnitObserverAlias&,
        std::uint32_t captured_table) = 0;
    // Complete external004BCA80 contract: store value to the context's SAME
    // actualE188DC cell, read currentF8D39C, callB0D7B0(value). That provider
    // writes renderer+1C0, and if current+30 is nonnull callsB4EC90 on it.
    // It can synchronously change this unit's flags. This wrapper passes null.
    virtual void set_controlled_listener_004bca80(void* value) = 0;
    virtual void call_virtual_7c(const NativeUnitObserverAlias&,
        std::uint32_t captured_table) = 0;
    virtual void call_virtual_80(const NativeUnitObserverAlias&,
        std::uint32_t captured_table) = 0;
};

struct NativeSceneLifecycleContext {
    ObserverEventDeliveryContext& observers;
    void* volatile& controlled_listener_00e188dc;
    NativeSceneLifecycleAccess& access;
};

// Complete00926390[34], native ECX unit, no stack inputs, RET or tailJMP7C.
// If current byte5D is zero: store5D=1 then60=1; conditional slot08 observer
// notification; reload the current table and invoke its slot7C on SAME unit.
void notify_native_scene_destroyed_00926390(
    NativeSceneLifecycleView, NativeSceneLifecycleContext&);

// Complete009263C0[85], native ECX unit, no stack inputs, RET or tailJMP80.
// First current-table getter18; if nonnull, capture second current table and
// actualE188DC BEFORE invoking the second getter18. Equality detaches through
// actual4BCA80. Then reload byte5E; if zero store5D/5E/5F=1,5C=0 in order,
// conditional slot04 notification, reload current table and invoke slot80.
// Even an already-destroyed unit performs the getter/detach prelude.
void remove_native_scene_immediate_009263c0(
    NativeSceneLifecycleView, NativeSceneLifecycleContext&);

// Both use the existing conditional observer wrapper: sample under the shared
// raw observer lock, release it, then dispatch under a separately acquired
// lock only if sampled count>0. Notification callbacks can mutate flags and
// profiles; tail selection uses the current profile after those callbacks.
// New C++ ABI, not native tail-call/SEH replacement. No runtime host binding,
// renderer implementation or base7C/80 implementation is supplied here.
} // namespace bsp
