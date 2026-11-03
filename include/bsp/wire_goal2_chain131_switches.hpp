#pragma once

// Goal 2, chain 131 (slot g2-w1): segment 33 [006DECA0-006FA320] range sweep.
// OFF keeps every host stand-in byte for byte. ON routes:
//   006E2220 projectile_extrapolate_position_006e2220 from the paratrooper round's
//            RoundAdapter::extrapolate_position (src/cc13_paratrooper_round.cpp);
//   006F9EE0 convex_object_compute_box_006f9ee0 from HullTerrainContactSolver::hull_shape
//            (src/hull_terrain_contact.cpp), the centre of the raw hull points.
// Runtime-unvalidated; flip in a batch across missions (JM08 for the paratrooper round,
// any ship-terrain contact for the hull shapes).

namespace bsp {

inline constexpr bool kWireGoal2Chain131Bound = true;

}  // namespace bsp
