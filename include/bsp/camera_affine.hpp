#pragma once
#include "bsp/camera_projection.hpp"

namespace bsp {
// Native ECX=destination, EDX=left, stack=right, RET4. New CameraMatrix API.
// Destination must not overlap either input; left and right may be identical.
// Row-major affine left*right: ignores input indices3/7/11/15 and forces output
// last column to +0,+0,+0,1. Retains native x87 operation/store order and caller
// floating-point environment; no validation, general perspective composition,
// object lifetime or camera dirty-cache behavior is added.
void compose_camera_affine_00b6d4d0(CameraMatrix& destination,
    const CameraMatrix& left, const CameraMatrix& right);
}
