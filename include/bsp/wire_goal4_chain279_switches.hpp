#pragma once

namespace bsp {

// Goal 4 chain 279 (agent/g4-w4): the plane class climb-angle solver.
//
// True:
// - 007D98F0 (max_sustainable_climb_angle_007d98f0, src/plane_flight.cpp) runs the
//   image's bisection with its own predicate, the trial integration 007D9360
//   (src/gameplay_seg48_cc13.cpp), which seeds a fresh record with 007D7C00 each step
//   and integrates it through the 007D8470 path a fresh record takes; the image's
//   exact constants (00CF8858, 00D06850, 00CE3910) replace the rounded literals.
// - 007D9140 is one callable body (plane_drag_accel_007d9140, src/plane_flight.cpp),
//   used by the trial and by the live flight-law drag in src/game_hosts_units.cpp.
// False: the closed-form substituted predicate and the host's inline drag.
inline constexpr bool kWireGoal4Chain279Bound = true;

}  // namespace bsp
