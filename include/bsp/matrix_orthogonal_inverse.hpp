#pragma once

#include "bsp/camera_projection.hpp"  // CameraMatrix = std::array<float, 16>

// 00B63D50 - the affine inverse used by every cached entity-pose inverse at
// owner +110h, and the one link the gun gravity arc (docs/GUN_GRAVITY_ARC.md)
// left unread.
//
// Addresses: 00B63D50 (body 00B63D50..00B63F09, RET, 145 instructions, leaf,
// branch-free); constant 00D7A24C = 1.0f. Evidence, original ABI, the
// numerical verification and the uncertainty: docs/MATRIX_ORTHOGONAL_INVERSE.md.
//
// Original ABI: __fastcall(CameraMatrix* destination /*ECX*/,
//                          const CameraMatrix* source /*EDX*/), plain `RET`
// (no immediate, no stack arguments), EAX never written. `SUB ESP,18h` /
// `ADD ESP,18h` balance; the 18h is scratch for the float32 spills only.
//
// Matrix layout, settled by the producers and not inferred from one consumer:
// row-major 4x4 of float32 with a 16-byte row stride, row-vector convention
// (v' = v * M). Rows 0..2 are the basis, row 3 (+30h/+34h/+38h) is the
// translation, column 3 (+0Ch/+1Ch/+2Ch/+3Ch) is the homogeneous column, which
// the routine forces to (0,0,0,1) without reading it.
//
// What it computes, per the recovered arithmetic:
//
//     N[i][j] = M[j][i] / (M[j][0]^2 + M[j][1]^2 + M[j][2]^2)   for i,j < 3
//     N[3][j] = -(M[3] . column j of N)                          the translation
//     N[*][3] = (0,0,0,1)
//
// i.e. the transpose of the basis with each column divided by the squared
// length of the source ROW it came from. That is exactly M^-1 when the three
// source rows are mutually orthogonal and non-zero - per-axis scale and a
// mirrored (negative-determinant) basis included. It is NOT a plain transpose
// and NOT a uniform-scale-only inverse.
//
// The routine has no guard of any kind: it is branch-free. A sheared basis
// silently yields a non-inverse; a zero-length row divides by zero. The
// unchecked precondition is exposed below as matrix_orthogonal_row_residual so
// a caller can test what the native never tests.
//
// Not safe in place. The native writes destination +10h before it reads source
// +10h (00B63E52 then 00B63E5E), so destination == source corrupts the result.
// All 71 direct call sites in .text pass disjoint operands.

namespace bsp {

// The pure rule. Reproduces the native's float32 spill boundaries: each of the
// nine squared components and each of the three squared lengths is rounded to
// float32, and each of the nine basis quotients is one correctly rounded float
// division. The two additions inside a squared length and the three-term
// translation dot product stay wide in the native (x87 80-bit) and are modelled
// here in double, which is not bit-identical in extreme-exponent cases; see the
// precision section of the doc. For a bit-exact kernel that keeps the original
// x87 schedule see derive_pose_affine_inverse_00b63d50 in bsp/pose_derived.hpp.
// Destination and source must not overlap.
void build_orthogonal_scaled_affine_inverse_00b63d50(CameraMatrix& destination,
                                                     const CameraMatrix& source);

// The precondition 00B63D50 never checks, made testable. Returns the largest
// |r_i . r_j| / (|r_i| |r_j|) over the three distinct row pairs - 0 for an
// exactly orthogonal basis, growing with shear - or 1.0f if any of rows 0..2
// has zero length, the case in which the native divides by zero. This is a new
// C++ helper, not a recovered routine; no native code computes it.
float matrix_orthogonal_row_residual(const CameraMatrix& source);

}  // namespace bsp
