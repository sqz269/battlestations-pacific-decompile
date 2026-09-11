#pragma once

#include "bsp/point_effect_instance.hpp"
#include "bsp/native_render_context.hpp"

namespace bsp {

// Complete ECX=array, RET body. Resize the SAME actual 0Ch header to zero,
// then free its current backing. Native leaves begin/capacity unchanged.
void destroy_point_effect_entry_array_008675b0(PointEffectReferenceArray&);

// Borrowed existing dependencies only: no node registry, copied containers,
// reference counts or substitute terminal action are created. Current +110
// must have a real virtual18 binding in node_lifetimes; a terminal node +04
// release uses node_terminal_owners only after that actual word reaches zero.
// These dependencies and the effect/captured slot storage survive callbacks.
struct PointEffectTeardownBindings {
    GeneratedModelLifetimeRuntime& node_lifetimes;
    NativeRenderActualOwners& node_terminal_owners;
    PointEffectInstanceLinks& parent_links;
    PointEffectInstanceCounters counters;
};

// Complete direct ECX=actual114h instance, RET body, including member unwind.
// Calls canonical B6DFA0, then reloads +110 for a SEPARATE actual-count release.
// Releases/clears +8C, +84; destroys +18 then +0C arrays; publishes CEB130.
// F87600 decrements once; F87604, +04, +88 and stale +110 are not modified.
// No physical instance free or replacement of the effect's virtual0 dispatcher.
void destroy_point_effect_instance_00867680(
    PointEffectInstanceStorage&, PointEffectTeardownBindings);

// ECX=instance, stack flags DWORD, EAX=original pointer even after free, RET4.
// Complete scalar deleting destructor. Ordinary CRT free iff (flags & 1) != 0;
// failed destruction propagates without freeing the actual instance storage.
PointEffectInstanceStorage* delete_point_effect_instance_00867ce0(
    PointEffectInstanceStorage*, std::uint32_t flags, PointEffectTeardownBindings);

} // namespace bsp
