#include "bsp/camera_transform.hpp"
#include "bsp/camera_affine.hpp"
#include "bsp/camera_inverse.hpp"
#include "bsp/camera_multiply.hpp"

namespace bsp {
void refresh_camera_world_00b6db70(CameraTransform& transform) {
    if (transform.parent) {
        if (!(transform.parent->valid_flags & 2)) refresh_camera_world_00b6db70(*transform.parent);
        compose_camera_affine_00b6d4d0(transform.world, transform.local, transform.parent->world);
    } else {
        copy_camera_matrix_004134f0(transform.world, transform.local);
    }
    transform.valid_flags |= 2;
}
const CameraMatrix& get_camera_view_00b6fcb0(CameraTransform& transform) {
    if (!(transform.valid_flags & 8)) {
        if (!(transform.valid_flags & 2)) refresh_camera_world_00b6db70(transform);
        CameraMatrix temporary;
        invert_camera_affine_00b63b30(temporary, transform.world);
        copy_camera_matrix_004134f0(transform.view, temporary);
        transform.valid_flags |= 8;
    }
    return transform.view;
}
const CameraMatrix& get_camera_view_projection_00b70490(CameraState& camera) {
    if (!(camera.projection.valid_flags & 0x10)) {
        const auto& view = get_camera_view_00b6fcb0(camera.transform);
        const auto& projection = get_camera_projection_00b6fcf0(camera.projection);
        CameraMatrix temporary;
        multiply_camera_matrices_00413920(temporary, view, projection);
        copy_camera_matrix_004134f0(camera.view_projection, temporary);
        camera.projection.valid_flags |= 0x10;
    }
    return camera.view_projection;
}
}
