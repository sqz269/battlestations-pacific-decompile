#pragma once
// ---------------------------------------------------------------------------
// Mount-frame scale: the rule that decides whether a pose local frame is safe
// for 00B63D50's affine inverse, and what a non-unit frame scale does to the
// ballistic arc's pitch.
//
// Evidence, the producer census and the shipped-data audit are in
// docs/MOUNT_FRAME_SCALE.md. Report: reports/cc7_mount_frame_scale.json.
//
// This header publishes RULES, not an ABI. There is no native routine that
// computes any of these: the native code has no guard at all, which is the
// point of the packet. Every function here is an explicit-input, returned-
// value predicate a host can call BEFORE it hands a frame to the inverse or
// to the arc. Bounded implementation: reconstructed and build-tested, not
// ABI-compatible and not game-validated.
//
// The three contracts these rules encode, each byte-proven elsewhere:
//
//   * 00B63D50 computes N[i][j] = M[j][i] / |row j|^2 branch-free. It is a
//     true affine inverse IFF the three basis rows are mutually orthogonal
//     and non-zero, and a zero row divides by zero with no guard.
//     docs/MATRIX_ORTHOGONAL_INVERSE.md.
//   * 00955630 carries its solved world direction into the mount frame
//     through 0042D0D0 with normalize = 0, and 00521370 then takes asin of
//     the RAW y. docs/GUN_GRAVITY_ARC.md.
//   * 00414DB0 composes world = local * parent.world with
//     BSP_Matrix_Multiply4x4 and never normalises, so a scale in any local
//     on the chain reaches the mount frame intact.
// ---------------------------------------------------------------------------
#include "bsp/gun_gravity_arc.hpp"  // GunGravityArcMountFrame, the three basis rows

namespace bsp {

// The per-axis row lengths of a mount basis plus the worst normalised
// off-diagonal. A similarity frame (the shape the .scn localframe parser
// canonicalises to, 0046D1BF..0046D222) has x == y == z and a zero
// off-diagonal; the landing-ship frame built at 008218CE..00821913 has
// x == z != y == 1 and is still orthogonal.
struct MountFrameScale {
    float x{1.0f};  // |row 0|, the basis row at frame+00h
    float y{1.0f};  // |row 1|, the basis row at frame+10h
    float z{1.0f};  // |row 2|, the basis row at frame+20h

    // max over the three row pairs of |dot(ri, rj)| / (|ri| |rj|), i.e. the
    // largest |cos| between two basis rows. 0 for an orthogonal basis. Rows
    // of length zero contribute 0 here and are reported by `has_zero_row`
    // instead, because the cosine is undefined for them.
    float max_off_diagonal{0.0f};

    bool has_zero_row{false};
};

// |row i| and the off-diagonals of `basis`. Pure; reads nine floats.
MountFrameScale mount_frame_scale(const GunGravityArcMountFrame& basis);

// 00B63D50's precondition, stated as the guard the native routine does not
// have. True when no row is zero and every pair of rows is orthogonal to
// within `orthogonality_tolerance` (a cosine, so unit-free).
//
// NOTE the asymmetry that makes this packet's finding possible: a uniform
// scale s passes this predicate. The inverse 00B63D50 returns for s*R + t IS
// correct - it is 1/s * R^T - and the arc is still wrong, because 0042D0D0
// runs with normalize = 0. Invertibility and arc correctness are different
// questions; use mount_frame_arc_is_exact for the second.
bool mount_frame_is_invertible_by_00b63d50(const MountFrameScale& scale,
                                           float orthogonality_tolerance = 1.0e-3f);

// The arc's pitch is exact only when the basis is orthoNORMAL. Uniform scale
// is not enough.
bool mount_frame_arc_is_exact(const MountFrameScale& scale,
                              float unit_tolerance = 1.0e-3f,
                              float orthogonality_tolerance = 1.0e-3f);

// What 00521370 reports for a mount whose basis carries uniform scale
// `scale`, when the true elevation of the solved direction is
// `true_pitch_radians`.
//
//     reported = asin( sin(true_pitch) / scale )
//
// because 0042D0D0 with normalize = 0 hands the arc a y component divided by
// scale and 00521370 takes asin of it unchanged. Returns a quiet NaN when
// |sin(true_pitch) / scale| > 1, which is the native domain error, not a
// clamp: the native code has no clamp either.
float mount_frame_arc_reported_pitch(float true_pitch_radians, float scale);

// reported - true, in radians. NaN propagates from the function above.
float mount_frame_arc_pitch_error(float true_pitch_radians, float scale);

}  // namespace bsp
