#pragma once

// PC24 wave 2, packet pc24w2-hull. The Dyn convex-hull build 00C5CAE0 runs on the main
// thread when a hull is built at load (avoid_zone_draft_bodies_00424ddf -> 00C5DF30 and
// hull_terrain_contact's hull_shape 006FAD70), whose x87 runs at 24-bit precision control.
// ON evaluates the listed chain per op through the cc13x87 helpers
// (include/bsp/cc13_x87_model.hpp); OFF keeps the earlier double chain rounded once.
namespace bsp {

// kPc24DynHullCc13Bound (OFF), src/avoid_zone_dyn_hull.cpp hull_00c5cae0:
//   the degenerate-face threshold eps * eps * 0.1: 00C5D3C9 FLD qword eps, 00C5D3CD FMUL
//   ST0,ST0, 00C5D3CF FMUL qword [00D7A3A0] (0.1f widened), compared unstored at 00C5D3D5
//   against the float face-area magnitude.
// Movers: which near-degenerate faces the repair pass extrudes, so hull shapes for avoid
// zones and terrain contact. Separate from kPc24AvoidZoneCc13Bound because the hull also
// serves terrain contact.
inline constexpr bool kPc24DynHullCc13Bound = false;

}  // namespace bsp
