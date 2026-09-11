#pragma once

#include "bsp/gamepad_force_events.hpp"
#include "bsp/effect_admission.hpp"

namespace bsp {
class RenderCommandReference;

// Complete0086B7B0: XOR AL,AL; RET. Base component virtual+08, no inputs
// inspected and upper EAX unchanged. Not a fallback for unknown predicates.
std::uint8_t effect_component_restart_false_0086b7b0(const void*) noexcept;

// Complete0086B7D0..0086B7D4. Native ECX component, two unused stack
// arguments (point, reference); AL=1, RET8. Upper EAX remains stale. This is
// the verified base virtual+1C leaf, not a fallback for unknown predicates.
std::uint8_t admit_effect_component_0086b7d0(const void* actual_component,
    EffectPointView point, CameraTransform& reference) noexcept;

// Complete0042C3D0..0042C44B. Native ECX actual target, stack float, RET4;
// no meaningful return value. Borrow its actual float fields: current+1C0,
// maximum+1C4 and minimum+1C8. Store the float sum first, then clamp against
// the captured minimum BEFORE loading maximum. Preserve x87 spill/comparison
// order, including unordered values and inverted limits. No allocation.
// The actual target must remain valid and cover at least1CCh bytes.
void accumulate_target_shake_0042c3d0(void* actual_target, float contribution) noexcept;

// Complete0086A820..0086A902. Shake component D0D5F4 virtual+18: native
// ECX actual2Ch component; stack actual point effect; EAX always0, RET4.
// Persistent byte+20 skips everything. Otherwise capture current target,
// refresh its actual pose if needed, THEN capture subject+110 and refresh its
// canonical node. Distance uses that captured target. Positive strength uses
// a SECOND current-target lookup and the real0042C3D0 accumulator above.
// Radius+24 and amplitude+28 are loaded after distance calculation; there is
// no intermediate float gain spill or gain clamp. Null is the native result
// even when the side effect executes; no event owner/reference is fabricated.
//
// spatial must supply the real current game/target and subject-node bindings,
// with the actual414DB0 refresh. ForceEventSpatialRuntime is the concrete
// composition with PoseRefreshResolver. Its returned target.identity must be
// the actual target containing the amplitude fields. Live captured storage and
// a nonnull second target on the positive path are native preconditions.
RenderCommandReference* create_point_shake_0086a820(const void* actual_component,
    void* actual_subject, ForceEventSpatialHost& spatial);

// New explicit C++ interfaces; not native vtable or application bindings.
} // namespace bsp
