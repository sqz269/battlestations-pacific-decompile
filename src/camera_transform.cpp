#include "bsp/camera_transform.hpp"
#include "bsp/camera_affine.hpp"
#include "bsp/camera_inverse.hpp"
#include "bsp/camera_multiply.hpp"

namespace bsp {
void invalidate_camera_descendants_00b6da30(CameraTransform& transform) {
    for (auto* child = transform.first_child; child; child = child->next_sibling) {
        if (child->valid_flags & 2) {
            child->auxiliary_flags &= 0xffffffcfu;
            child->valid_flags &= 0xfffffff5u;
            if (child->first_child) invalidate_camera_descendants_00b6da30(*child);
        }
    }
}
void set_transform_local_matrix_00b6db10(CameraTransform& transform, const CameraMatrix& source) {
    copy_camera_matrix_004134f0(transform.local, source);
    if (transform.valid_flags & 10) {
        const auto notify = transform.notify_changed;
        void* context = transform.notification_context;
        transform.auxiliary_flags &= 0xffffffcfu;
        transform.valid_flags = 0;
        if (notify) notify(context);
        if (transform.first_child) invalidate_camera_descendants_00b6da30(transform);
    }
}
void refresh_camera_direction_00b70660(CameraState& camera) {
    if (!(camera.transform.valid_flags & 2)) refresh_camera_world_00b6db70(camera.transform);
    const float* world = camera.transform.world.data();
    float* direction = camera.direction.data();
    __asm {
        mov eax, world
        mov ecx, direction
        fld dword ptr [eax + 32]
        fstp dword ptr [ecx]
        fld dword ptr [eax + 36]
        fstp dword ptr [ecx + 4]
        fld dword ptr [eax + 40]
        fstp dword ptr [ecx + 8]
    }
    if (!(camera.transform.valid_flags & 2)) refresh_camera_world_00b6db70(camera.transform);
    float position[3], sum[3];
    float* target = camera.target.data();
    __asm {
        mov eax, world
        mov ecx, direction
        fld dword ptr [eax + 48]
        fstp dword ptr position[0]
        fld dword ptr [eax + 52]
        fstp dword ptr position[4]
        fld dword ptr [eax + 56]
        fstp dword ptr position[8]
        fld dword ptr position[0]
        fadd dword ptr [ecx]
        fstp dword ptr sum[0]
        fld dword ptr [ecx + 4]
        fadd dword ptr position[4]
        fstp dword ptr sum[4]
        fld dword ptr [ecx + 8]
        fadd dword ptr position[8]
        fstp dword ptr sum[8]
        mov eax, target
        fld dword ptr sum[0]
        fstp dword ptr [eax]
        fld dword ptr sum[4]
        fstp dword ptr [eax + 4]
        fld dword ptr sum[8]
        fstp dword ptr [eax + 8]
    }
}
void set_camera_local_matrix_00b71430(CameraState& camera, const CameraMatrix& source) {
    camera.projection.valid_flags &= 0xfffffe4bu;
    set_transform_local_matrix_00b6db10(camera.transform, source);
    refresh_camera_direction_00b70660(camera);
}

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
