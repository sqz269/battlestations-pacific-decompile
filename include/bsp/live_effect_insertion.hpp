#pragma once

#include "bsp/effect_admission.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_render_pointer_arrays.hpp"

namespace bsp {

// Complete 4C5CE0/74D780/4C9550 specializations over the actual0Ch header.
// ECX header, one stack DWORD (signed count/capacity or source-cell address),
// RET4. Cells contain actual raw owners; +04 is their canonical atomic and
// zero release uses the REQUIRED current owner lookup/terminal implementation.
// These are owning-reference operations on the shared physical header layout.
void reserve_live_effect_references_004c5ce0(NativeRenderPointerArrayStorage&,
    std::int32_t requested_capacity, NativeRenderActualOwners&);
void append_live_effect_reference_0074d780(NativeRenderPointerArrayStorage&,
    const void* source_pointer_cell, NativeRenderActualOwners&);
void resize_live_effect_references_004c9550(NativeRenderPointerArrayStorage&,
    std::int32_t requested_count, NativeRenderActualOwners&);

// Complete 867500, native ECX actual28h F8765C owner, stack raw effect, RET4.
// This interface borrows that owner's actual +10 array explicitly. It calls
// canonical866440 even for null input, captures that owner's +04 section, and
// holds it across append, temporary release, then captured-effect retain.
// A throwing append releases/clears the current temporary (state1/440A30)
// before unlocking (state0/411EE0). A successful append disables that unwind
// before decrementing the captured input and incrementing its same +04 again.
void insert_live_effect_00867500(NativeRenderPointerArrayStorage& actual_manager_array_10,
    void* actual_effect, EffectManager* volatile& actual_lock_owner_00f87650,
    EffectManagerLifetimeAccess&, NativeRenderActualOwners&);

// Caller supplies a constructed owner, valid spans and actual live references.
// Preserve DWORD growth/address arithmetic, null-destination skipping, current
// field reloads and callback-sensitive clear order. No overflow recovery,
// duplicate owner registry, implicit replacement-array rollback or generic
// STL implementation is supplied. Source cells must survive any reserve.
// Native virtual terminal actions obey the existing nonthrowing interface;
// original exception dispatch and malformed-memory faults are not this C++ ABI.
} // namespace bsp
