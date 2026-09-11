#pragma once

#include "bsp/pose_refresh.hpp"

namespace bsp {

// 0042D7E0: ECX=actual pose owner, no stack args, EAX=owner+CCh, RET.
// Borrows the existing world matrix after refreshing only a zero actual C8 byte.
// The returned matrix remains the owner's storage; no matrix/flag copy is owned.
CameraMatrix& pose_world_matrix_0042d7e0(PoseRefreshView&);

// 00414D10: ECX=output XYZ, EDX=input XYZ, stack=matrix, EAX=output, RET4.
// Transform with canonical 004142E0 into disjoint three-float scratch, then copy.
// Output must address three writable floats; it may alias input or matrix floats.
// The input and matrix are completely consumed before the first output store.
float* transform_point_staged_00414d10(float* output,
    const std::array<float, 3>& input, const CameraMatrix& matrix);

} // namespace bsp
