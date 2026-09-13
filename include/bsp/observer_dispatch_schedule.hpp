#pragma once

#include "bsp/observer_lifetime.hpp"

namespace bsp {

// Native 006960D1 calls the supplied function with ECX=original first and
// EDX=current nonnull edge+8. This explicit C++ callback/context is a new ABI;
// the provider owns the actual event-specific virtual slot dispatch.
using NativeObserverDispatchCallback = void (*)(void* context,
    NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& callback_owner);

// Complete normal 00695F90..00696112 schedule (native ECX=first, EDX=callback,
// RET0). Borrow the same actual E198E4 publication as lifetime's unregister
// operations. Capture its initial size, append first's current edge sequence,
// visit the captured index interval, skip nulled entries, then resize back.
// The captured recursive section remains held throughout callbacks. Nested
// calls append/trim their own interval; unregister/detach can null pending slots.
// Neither endpoint nor edge is retained. All callback owners must remain live.
void dispatch_observer_edges_00695f90(NativeObserverLifetime& lifetime,
    NativeObserverDispatchStorage* volatile& global_00e198e4,
    NativeObserverOwnerStorage& first, NativeObserverDispatchCallback callback,
    void* context);

// A private restoration adapter consumes only resize-to-saved-size. In the
// native-valid vector domain, shrinking erases through the captured end and
// copies no elements; growth reuses the existing canonical count insertion.
// No standalone STL resize/erase routine is reconstructed. Malformed storage
// and returning validation-handler mutations are outside adapter coverage.
// Fixed current source CRT invalid-parameter/heap services are used;
// original VS2005 CRT internals and malformed-memory/fault identity are not.
// C++ callback/provider exceptions release the captured lock, but do NOT trim
// the dispatch vector (matching the native cleanup's lack of resize). Native
// FH3/SEH transport and asynchronous/hardware faults are outside this new ABI.
} // namespace bsp
