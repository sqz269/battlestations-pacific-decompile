#pragma once

#include "bsp/point_effect_children.hpp"

namespace bsp {

// Complete 00866F50: ECX=actual114h point, no stack arguments, RET. Clears
// +08/+0A, captures the definition span, and recreates only rows with nonzero
// low byte+10. Ignores admitted+1C and existing destination values. Factories
// transfer one owned reference; destination backing is reloaded after each.
// Captured definition backing and the point must survive all callbacks.
void restart_point_effect_00866f50(PointEffectInstanceStorage&, PointEffectRowRuntime&);

// Complete 00867B10: ECX=actual114h point, no stack arguments, RET. Captures
// the actual866440 manager lock before changing flags. On first stop, traverses
// the captured primary span, cancels active children, appends the current source
// slots to auxiliary, then releases/clears those same slots. Callback changes
// remain visible; exceptions only unwind the captured lock, without rollback.
void stop_point_effect_00867b10(PointEffectInstanceStorage&, PointEffectChildEvents&,
    EffectManager* volatile& actual_f87650, EffectManagerLifetimeAccess&);

// Complete0042D9A0: ECX=owned field address, stack=consumed owning reference,
// EAX=original field address, RET4. Release captured old before publishing the
// input. No incoming retain or identity shortcut, including old==incoming.
// Links must map the exact existing parent to its actual+04/current terminal.
CameraTransform*& consume_point_effect_parent_0042d9a0(CameraTransform*& actual_slot,
    CameraTransform* consumed, PointEffectInstanceLinks&);

// New typed Win32 interfaces, not native vtable overlays. Canonical reference
// terminal actions are nonthrowing; native throw-through virtual00 and native
// EH dispatcher execution are not implied by these C++ implementations.

} // namespace bsp
