#pragma once

#include "bsp/native_live_effect_manager.hpp"
#include "bsp/native_render_context.hpp"

namespace bsp {

// Required native008673B0 boundary. ECX is the freshly loaded actual array
// entry, including null on an unfiltered walk. Its captured lock, child owner
// releases, auxiliary queue cleanup and final byte+09 write are not supplied
// here. No successful default or substitution of00867B10 is valid.
class NativeWreckEffectStopAccess {
public:
    virtual ~NativeWreckEffectStopAccess() = default;
    virtual void call_008673b0(void* actual_entry) = 0;
};

// Complete008674C0 normal control flow: ECX actual28h manager, one stack
// filter, RET4. Capture count+14 before data+10 and fix the end address once.
// Call008673B0 for each current cell equal to filter, or every cell if null.
// The captured backing must remain live through callbacks. Header changes do
// not change this span; cell changes are observed. Exceptions stop the walk.
void stop_native_wreck_effects_008674c0(NativeLiveEffectManagerStorage&,
    void* filter, NativeWreckEffectStopAccess&);

// Complete00484620 normal control flow via the byte-identical canonical
//0054D510 provider. ECX actual pointer cell; stack replacement; EAX cell; RET4.
// Decrement captured old owner's actual+04 atomically; resolve only on zero
// and dispatch its CURRENT virtual0 through the required actual owner domain.
// Then clear the actual cell and adopt without retain, including old==new.
// The cell stays volatile throughout. A terminal callback can mutate it;
// those writes are overwritten after return. No pointer/count shadow exists.
// Domain contract: live aligned actual atomic at raw+04, companion borrowing
// that exact atomic, nonthrowing terminal callback with no missing identities
// or profiles. Contract violations terminate, never silently ignore release.
void* volatile& adopt_native_wreck_effect_00484620(void* volatile& actual_cell,
    void* replacement, NativeRenderActualOwners&) noexcept;

// New C++ interfaces, not native ABI replacements. Native exception transport,
// unknown destructor implementations and full wreck handling remain unproved.
} // namespace bsp
