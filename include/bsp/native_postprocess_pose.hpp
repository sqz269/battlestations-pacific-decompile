#pragma once

#include "bsp/system_camera_axes.hpp"

namespace bsp {

// Complete native 00B630F0..00B632C7: ECX matrix, stack output XYZ, RET4,
// EAX output. The source adds borrowed CameraAxesCrtAccess in EDX. All three
// matrix basis rows are captured before the first output write; output may
// overlap any three writable matrix words. Matrix has at least eleven words.
// Preserves the literal FSIN/FCOS, x87 spills, SSE subtraction from negative
// zero, unordered branches and output order Y,Z,X. This is not a conventional
// Euler decomposition and does not clamp or repair exceptional operands.
//
// Uses canonical 00419440 with the required live sqrt CRT state/handler and
// the linked host CRT _CIatan2 entry (ST0=x, ST1=y). The original atan2 CRT
// dispatch globals, diagnostics and exception policy are not reconstructed.
// The matrix, output, CRT access and its two members must all be valid.
float* __fastcall extract_native_animator_angles_00b630f0(
    const float* matrix, const CameraAxesCrtAccess* crt, float* output);

} // namespace bsp
