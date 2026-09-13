#pragma once

#include "bsp/ship_ai_neighbour_box.hpp"

namespace bsp {

// Partial field projection of 009E52E0, whose native ABI is ECX=node,
// stack(owner, controller, lifetime), EAX=node, RET 0Ch. This initializes only
// its fields already represented by these two existing semantic records.
// Geometry, headings and owner_gone_5e are untouched. The latter belongs to
// the observed owner, not the native node. Lifetime is copied without arithmetic
// for a nonnull owner; null owner selects the original -1.0f word.
//
// This does not construct/register a native observer, allocate a 90h node, set
// its controller+1C or initialize its other unrepresented fields. It must run
// before the real near/avoid refresh sequence, not in place of that sequence.
// No observer teardown, node lifetime ownership or binary ABI is supplied.
void ship_ai_obstacle_owner_initialize_projection_009e52e0(
    ShipAiObstacleNode& node, ShipAiNeighbourNodeMotion& motion,
    const void* observed_owner, float lifetime) noexcept;

} // namespace bsp
