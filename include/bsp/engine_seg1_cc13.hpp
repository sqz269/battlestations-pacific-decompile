#pragma once
// Segment 1 (00413D10-0042A830) engine helpers reconstructed by cc13-left2 (docs/CC13_LEFT_2.md). Read
// from the live decompile and the listing; the x87 expressions were traced from the bytes. Each routine
// keeps the native float spills: a value the image stores to a float slot is rounded to float here.
// New C++ interfaces, build-tested only; not ABI replacements.

namespace bsp {
struct CameraAxesCrtAccess;
}

namespace bsp::engine_seg1_cc13 {

// 00414130 BSP_Math_Lerp, __fastcall(ECX const float* a, EDX const float* b, const float* t) RET 4 -> ST0:
// a is spilled to a float, then float((b - a) * t + a) (DE EA is FSUBP ST(2): b - a), returned after its
// float store.
float math_lerp_00414130(const float* a, const float* b, const float* t) noexcept;

// 00414220 BSP_Vector3f_Assign, __thiscall(ECX destination, const float* source) RET 4 -> EAX destination:
// three FLD/FSTP copies (a signalling NaN is quieted on the way).
float* vector3_assign_00414220(float* destination, const float* source) noexcept;

// 00414260 BSP_Vector3f_ScaleTo, __thiscall(ECX const float* v, float* out, float s) RET 8 -> EAX out:
// out = {float(x * s), float(y * s), float(s * z)}.
float* vector3_scale_to_00414260(const float* v, float* out, float s) noexcept;

// 004142A0 BSP_Vector3f_AddTo, __thiscall(ECX const float* a, float* out, const float* b) RET 8 -> EAX out:
// out[i] = float(b[i] + a[i]).
float* vector3_add_to_004142a0(const float* a, float* out, const float* b) noexcept;

// 004191D0 BSP_Vector2f_LengthSquared, __fastcall(ECX const float*) -> ST0: float(float(x*x) + float(y*y)).
float vector2_length_squared_004191d0(const float* v) noexcept;

// 004193E0 BSP_Vector3f_LengthSquared, __fastcall(ECX const float*) -> ST0: float((float(x*x) + float(y*y))
// + float(z*z)); the partial sum stays in the x87 register (53-bit precision).
float vector3_length_squared_004193e0(const float* v) noexcept;

// 004192E0 BSP_Vector2f_Normalize, __fastcall(ECX float* out, EDX const float* in) -> EAX out: r =
// 00419260 (reciprocal length, ECX in) spilled to a float; out = {float(in.x * r), float(r * in.y)}.
float* vector2_normalize_004192e0(float* out, const float* in, const CameraAxesCrtAccess* crt);

}  // namespace bsp::engine_seg1_cc13
