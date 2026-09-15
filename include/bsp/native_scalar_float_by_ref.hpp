#pragma once

namespace bsp {
// Complete raw 00415510 (62 bytes): ECX/EDX address actual binary32 operands;
// x87 ST0 result, no public stack arguments, plain RET. Each input is loaded
// and spilled as binary32 before comparison. Equality and masked unordered
// comparison select the right spill; FP status/exception timing is not guarded.
float __fastcall min_native_float_by_ref_00415510(
    const float* left, const float* right);

// Existing complete raw 00415550 (62 bytes), defined in native_traceline_render.
// Same pointer/return ABI and spill discipline; comparison order is mirrored.
// This declaration reuses that external definition and adds no second body.
float __fastcall max_native_float_by_ref_00415550(
    const float* left, const float* right);
} // namespace bsp
