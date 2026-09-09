#pragma once
#include "bsp/camera_projection.hpp"

namespace bsp {
// Semantic entry for00b63b30: native ECX=destination, EDX=source, EAX returns
// destination, plain RET. This C++ interface has no original object layout.
// Requires distinct, nonoverlapping 64-byte matrices. Native is not safe in
// place. Inverts affine matrices with nonzero mutually orthogonal scaled rows;
// other inputs are processed without validation, not as a general inverse.
// Preserves source indices3/7/11/15, native x87 float32 spill boundaries and
// SSE negative-zero translation. Inherits caller x87/MXCSR configuration.
void invert_camera_affine_00b63b30(CameraMatrix& destination, const CameraMatrix& source);
}
