#pragma once

// PC24 wave 2, packet pc24w2-hull. The image runs the Dyn convex-hull build (00C5DEB0, then
// 00C5CAE0) on the main thread, whose x87 runs at 24-bit precision control: at mission load
// (avoid_zone_draft_bodies_00424ddf -> 00C5DF30) and in the ConvexObject resource parse
// (006FAD70 -> 00C5DEB0). The host builds hull_terrain_contact's hull_shape lazily on first
// use instead, from native_narrow_phase / dyn_hull_vertices, which run inside
// DynWorkerPrecisionScope (hull_terrain_contact.cpp), so with kHullShapeBuildPc24Cc13Bound
// OFF the build and its inline-asm helpers (magnitude, sum_products, fdiv in
// avoid_zone_dyn_hull.cpp) run at 53 bits there; only a hull that hull_hull_census builds
// first (outside that scope) is built at the main thread's 24 bits.
// Excluded image case: the frontend-only Tactical Library preview loader (00501510) builds
// its hulls at 53 bits; the host does not run it in a mission.
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

// kHullShapeBuildPc24Cc13Bound (OFF), packet cc13-hullscope (rev9 P9):
//   HullTerrainContactSolver::hull_shape (src/hull_terrain_contact.cpp), the host's copy of
//   006FAD70's shape build: ON wraps its 00C5DF30 construct (00C5DEB0 -> 00C5CAE0) in a
//   control-word scope that sets _PC_24 and restores the caller's precision on exit, so the
//   hull is built at the image's main-thread precision whichever caller builds it first
//   (inside DynWorkerPrecisionScope or not). Same thread, no shared state: the scope only
//   touches the calling thread's x87 control word, and the build still runs once per raw
//   point set (the hulls_ cache). OFF: the build runs at the caller's precision, as before.
// Movers: hull vertices for terrain contact and hull/fort/draft pairs (hull_shape's
//   vertices and convex records), so contact points, penetration depths and the contact
//   impulses ship hulls get; pairs with kPc24DynHullCc13Bound, whose ON chain is the
//   per-op form of the same build.
inline constexpr bool kHullShapeBuildPc24Cc13Bound = false;

// Census (cc9-nondet1, log only): repair-pass face compares, how often the per-op threshold
// differs from the double one, and how often that flips the extrude decision.
struct DynHullPc24Census {
    unsigned long long compares{0}, threshold_differs{0}, decision_differs{0};
};
const DynHullPc24Census& dyn_hull_pc24_census() noexcept;

}  // namespace bsp
