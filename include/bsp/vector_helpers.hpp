#pragma once
#include "bsp/camera_affine.hpp"

namespace bsp {
// Native ECX=two floats, no stack arguments, RET, float result in ST0.
// x87 sum of squares is rounded to binary32 before comparison with double 1e-10.
// Ordered-greater calls the actual host CRT _CIsqrt and rounds twice to float;
// otherwise returns +0, including unordered inputs. Does not write the input.
// Retains caller x87 environment; original CRT diagnostic parity is unverified.
float length_2d_00414c60(const std::array<float, 2>& value);

// Same-TU raw tail entry to the actual 00414C60 kernel: ECX addresses two
// readable floats, RET consumes no arguments, ST0 holds the float-spilled
// result and ECX receives its raw float bits. The facade adds only a JMP, with no register,
// stack, flags or FP-environment operation. The kernel needs three free local
// x87 slots; genuine CRT entry requirements and private policy stay external.
// Input lifetime/validity and synchronization belong to the caller. Native
// arithmetic/status effects remain; no input copy or environment reset is added.
float __fastcall raw_length_2d_00414c60(const float* actual_values) noexcept;

// Native ECX=destination XYZ, EDX=source XYZ, stack=matrix; RET4; no useful result.
// Uses the canonical affine point kernel, then copies the three result words.
// Borrowed destination points to three writable floats and may equal source's
// data or overlap the matrix, including partial overlap. Matrix reads finish first.
// No native vector/matrix owner layout or public binary ABI is claimed.
void transform_point_copy_00414cd0(float* destination,
    const std::array<float, 3>& source, const CameraMatrix& matrix);
}
