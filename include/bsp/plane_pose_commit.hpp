#pragma once
#include <cstdint>

// Reconstruction of 0085DC80, the routine 007CECBA calls at the tail of the plane's
// fixed-step motion dispatch. docs/PLANE_POSE_COMMIT.md carries the evidence, the
// ABI and the uncertainty. Every name here is a hypothesis, not a recovered symbol.
//
// WHAT IT IS NOT. The packet brief called 0085DC80 "the per-step plane pose commit"
// and expected it to be the step that turns the plane. It is not. 0085DC80 reads and
// writes one 4x4's three basis rows and nothing else: it re-normalises row 2 and
// rebuilds rows 1 and 0 around it by Gram-Schmidt. It integrates nothing, it reads no
// velocity, no control axis and no time step, and it takes no argument but the matrix
// pointer (0085DC80 SUB ESP,10h .. 0085DDFA RET, no imm16). Applied to a matrix that
// is already orthonormal it is a no-op to within rounding. The plane's orientation is
// advanced somewhere else; see the Wiring contract section of the doc.
//
// The same routine is called from 54 sites across the binary, so nothing in it is
// plane-specific. 0074260E (docs/LAND_AND_STRUCTURES.md, include/bsp/
// land_and_structures.hpp:301) is the clearest independent corroboration of the rule
// below: that caller writes a path tangent into row 2 and the world up axis into
// row 1 and then calls 0085DC80 to "complete the frame", which is exactly a reading
// of row 2 as authoritative and row 1 as a hint.

namespace bsp {

// ---------------------------------------------------------------------------
// The matrix this runs on, for the plane call site only.
// 007CECB4 LEA ECX,[ESI+364h] with ESI = unit+310h (docs/PLANE_UNIT_TICK.md), so
// the argument is unit+674h. 007D9F74 MOV ECX,10h / 007D9F7D MOVSD.REP copies
// 16 dwords out of it, which settles the size at a 64-byte row-major 4x4.
// docs/TICK_ELEMENT_OVERRIDES.md:39 settles the role: unit+674h holds the last
// fixed-step pose and unit+74h holds the pose that is drawn.
// ---------------------------------------------------------------------------
namespace plane_pose_off {
inline constexpr int kFixedStepPose = 0x674;  // 007CECB4, the matrix base
inline constexpr int kRow0 = 0x674;           // 0085DC85 MOV ESI,ECX      - right
inline constexpr int kRow1 = 0x684;           // 0085DCB9 LEA EDI,[ESI+10h] - up
inline constexpr int kRow2 = 0x694;           // 0085DC87 LEA EBX,[ESI+20h] - forward
inline constexpr int kRow3 = 0x6A4;           // never touched by 0085DC80  - translation
inline constexpr int kBytes = 0x40;           // 007D9F74 MOV ECX,10h
}  // namespace plane_pose_off

// 00D0D0A0, the only image constant in the body: the double 3FEFF7CEE0000000.
// That is 0.999f widened to double, so comparing a float against 0.999f reproduces
// 0085DD1C FCOMIP exactly - no value is lost either way.
inline constexpr float kBasisParallelLimit_00d0d0a0 = 0.999f;

// 00D7A208, the float 80000000h = -0.0f. 0085DDB9..0085DDC7 subtract each cross
// component from it, which is a sign flip, not an arithmetic operation.
inline constexpr float kNegativeZero_00d7a208 = -0.0f;

// The three basis rows of the 4x4, in the storage order 0085DC80 assumes. Row 3,
// the translation, and the fourth column (elements 3, 7 and 11) are deliberately
// absent: the routine never reads or writes them.
//
// The convention is the established one, not a new one. docs/ENTITY_LOCAL_MATRIX.md
// settles it from 00414DB0: row-major, row-vector, translation in the last row, so
// row i is body axis i expressed in world. Row 2 is forward, which is what
// src/system_camera_axes.cpp:387 reads as world[8..10] and what
// src/game_hosts_units.cpp:529 turns into atan2(row2[0], row2[2]).
struct PoseBasis {
    float row0[3];  // +00h  right
    float row1[3];  // +10h  up
    float row2[3];  // +20h  forward, and the authority
};

// Which of the two exits at 0085DD20 ran. Reported because the host cannot see it
// from the result and because the degenerate exit means the caller's row 1 was
// useless - a signal worth logging, not a failure.
enum class PoseOrthonormalizeBranch {
    // 0085DD20 JBE taken -> 0085DDFB. |dot(row1, row2)| <= 0.999f: row 1 is a
    // usable up hint, so row 1 is projected off row 2 and row 0 is derived.
    UpReference,
    // 0085DD20 JBE not taken -> 0085DD26. Row 1 is within 2.56 degrees of row 2
    // and cannot define a plane with it, so row 0 becomes the second reference
    // and row 1 is derived instead.
    RightReference,
};

struct PoseOrthonormalizeResult {
    PoseBasis basis;
    PoseOrthonormalizeBranch branch;
    // 0085DC8D's 00419440 return value, before the reciprocal. Exposed because
    // 0085DC96..0085DCB1 turn a zero or NaN length into a scale of +0.0f rather
    // than a NaN, and the whole basis then collapses to zero silently. The native
    // routine has no guard against that; a host that wants one needs this.
    float forward_length;
};

// 0085DC80, __fastcall(float* m /*ECX*/), plain RET, body 0085DC80-0085DE93.
// Pure here: it takes the three rows and returns the three rows.
PoseOrthonormalizeResult orthonormalize_basis_rows_0085dc80(const PoseBasis& in);

// The same rule against the native storage shape: a row-major 4x4, rows read and
// written at elements 0..2, 4..6 and 8..10. Elements 3, 7, 11 and the whole of
// row 3 are left exactly as they were, which is what the native body does by
// never addressing them.
void orthonormalize_pose_matrix_0085dc80(float m[16]);

}  // namespace bsp
