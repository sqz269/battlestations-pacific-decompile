#pragma once

#include "bsp/effect_deletion_queue.hpp"
#include "bsp/live_effect_insertion.hpp"

namespace bsp {

// Complete4BA0F0: ECX0Ch header, RET. Free a captured nonnull backing pointer,
// then clear current data field. No cell release or count/capacity mutation.
void destroy_live_effect_auxiliary_array_004ba0f0(NativeRenderPointerArrayStorage&) noexcept;
// Complete4CDEA0: ECX0Ch header, RET. Resize0 via4C9550 then free CURRENT
// backing. Native leaves the data pointer/capacity stale after free.
void destroy_live_effect_reference_array_004cdea0(
    NativeRenderPointerArrayStorage&, NativeRenderActualOwners&);
// Complete five-byte4C8C20 tail thunk to reconstructed4C5940, ECX list, RET.
void destroy_live_effect_pending_list_004c8c20(NativeEffectDeletionListStorage&) noexcept;

// Complete867C00: ECX actual28h owner, RET. PublishCE789C, resize effects+10
// to0, flush pending deletions, free/clear auxiliary+1C, resize effects+10 to0
// AGAIN, free current effects backing, destroy list+04, clear F8765C/baseCE3818.
// The second resize handles references added during earlier callbacks.
// Native state3 unwind: auxiliary -> owning array -> list -> base; state1
// during second resize: list -> base only. No pending-payload retry is added.
void destroy_live_effect_manager_00867c00(NativeLiveEffectManagerStorage&,
    NativeLiveEffectManagerStorage* volatile& actual_global_00f8765c,
    NativeRenderActualOwners&, EffectDeletionDispatch&);

// Complete4CF770: ECX owner, stackflags, EAX original address, RET4. Destroy
// unconditionally; free actual owner iff flags&1 only after successful teardown.
// No unregister: domain shutdown already pops its entry before dispatch.
NativeLiveEffectManagerStorage* delete_live_effect_manager_004cf770(
    NativeLiveEffectManagerStorage*, std::uint32_t flags,
    NativeLiveEffectManagerStorage* volatile& actual_global_00f8765c,
    NativeRenderActualOwners&, EffectDeletionDispatch&);

// Complete4D1100: no native inputs, EAX published owner, RET. Uses the SAME
// actual singleton domain, captures its section, rechecks global, allocates28h,
// constructs4CF700, publishes, gets lifetime manager again, registers current
// global, unlocks captured section and reloads result. Constructor failure frees
// raw owner before unlock; registration failure does not undo publication.
// Domain callbacks must dispatch this exact registered owner to4CF770 using its
// actual owner/scalar bindings. No implicit replacement lifetime domain exists.
NativeLiveEffectManagerStorage* live_effect_manager_singleton_004d1100(
    NativeLiveEffectManagerStorage* volatile& actual_global_00f8765c,
    SingletonLifetimeDomain& actual_singleton_domain_01090aa0);

// New C++ ABI. Native exception dispatch and concurrent unsynchronized mutation
// are not emulated. Actual owner bindings must survive their final native use;
// exceptions during C++ unwind cleanup obey its existing nonthrowing boundary.
} // namespace bsp
