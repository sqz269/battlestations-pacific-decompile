#pragma once
#include "bsp/camera_projection.hpp"

namespace bsp {
//00413920: original ECX=left, stack destination/right, RET8, EAX=destination.
// Row-major product dst=left*right. Exact dst==left, dst==right and all-equal
// aliasing are supported; arbitrary partial overlap is outside the API contract.
// Preserves original x87 order, float32 caches and ambient FP environment.
// New typed entry point, not a drop-in native binary replacement.
void multiply_camera_matrices_00413920(CameraMatrix& dst,
    const CameraMatrix& left, const CameraMatrix& right);
}
