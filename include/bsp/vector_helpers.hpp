#pragma once
#include "bsp/camera_affine.hpp"

namespace bsp {
// Native ECX=two floats, no stack arguments, RET, float result in ST0.
// x87 sum of squares is rounded to binary32 before comparison with double 1e-10.
// Ordered-greater calls the actual host CRT _CIsqrt and rounds twice to float;
// otherwise returns +0, including unordered inputs. Does not write the input.
// Retains caller x87 environment; original CRT diagnostic parity is unverified.
float length_2d_00414c60(const std::array<float, 2>& value);

// Native ECX=destination XYZ, EDX=source XYZ, stack=matrix; RET4; no useful result.
// Uses the canonical affine point kernel, then copies the three result words.
// Borrowed destination points to three writable floats and may equal source's
// data or overlap the matrix, including partial overlap. Matrix reads finish first.
// No native vector/matrix owner layout or public binary ABI is claimed.
void transform_point_copy_00414cd0(float* destination,
    const std::array<float, 3>& source, const CameraMatrix& matrix);
}
