#pragma once
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Recovered register/stack interfaces; descriptive names are hypotheses.
// Required native-valid pointed-to storage; no sanitization or alias repair.
// Clamp spills all selected scalar operands through binary32 before comparing.
// Native ECX value, EDX lower, stack upper; RET4, x87 ST0 result.
float __fastcall clamp_native_float_004155b0(const float*, const float*, const float*) noexcept;
// Native ECX two-float vector; these interfaces add actual borrowed CRT in EDX.
// Both preserve all native binary32 spills and return the result in x87 ST0.
float __fastcall native_vector2_length_00419210(const float*, const CameraAxesCrtAccess*);
float __fastcall native_vector2_reciprocal_length_00419260(const float*, const CameraAxesCrtAccess*);
// Native ECX output, EDX left, stack right; RET4, EAX output. x87 operand
// spills/comparisons precede MOVSS stores; unordered choices are asymmetric.
float* __fastcall native_vector2_min_00b9a7a0(float*, const float*, const float*) noexcept;
float* __fastcall native_vector2_max_00b9a820(float*, const float*, const float*) noexcept;
}
