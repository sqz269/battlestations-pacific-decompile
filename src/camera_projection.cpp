#include "bsp/camera_projection.hpp"
#include <cstddef>

namespace bsp {
static_assert(offsetof(CameraProjection, aspect) == 4
    && offsetof(CameraProjection, near_plane) == 8 && offsetof(CameraProjection, far_plane) == 12);
//004134f0 copy semantics; caller retains both native matrix copies.
void copy_camera_matrix_004134f0(CameraMatrix& output, const CameraMatrix& input) {
    for (unsigned i = 0; i < 16; ++i) {
        const float* source = input.data() + i;
        float* destination = output.data() + i;
        __asm {
            mov eax, source
            mov ecx, destination
            fld dword ptr [eax]
            fstp dword ptr [ecx]
        }
    }
}
void build_projection_00b642f0(CameraMatrix& output, float fov, float aspect,
    float near_plane, float far_plane) {
    const double half = 0.5;
    float half_angle, tangent, scale, ratio;
    float* destination = output.data();
    __asm {
        fld fov
        fmul half
        fstp half_angle
        fld half_angle
        fsincos
        fdivp st(1), st(0)
        fstp tangent
        fld tangent
        fld1
        fdivrp st(1), st(0)
        mov eax, destination
        xorps xmm0, xmm0
        movss dword ptr [eax + 4], xmm0
        movss dword ptr [eax + 8], xmm0
        movss dword ptr [eax + 12], xmm0
        movss dword ptr [eax + 16], xmm0
        movss dword ptr [eax + 24], xmm0
        movss dword ptr [eax + 28], xmm0
        movss dword ptr [eax + 32], xmm0
        movss dword ptr [eax + 36], xmm0
        movss dword ptr [eax + 48], xmm0
        movss dword ptr [eax + 52], xmm0
        movss dword ptr [eax + 60], xmm0
        fstp scale
        fld far_plane
        fld near_plane
        fld st(0)
        fsubr st(0), st(2)
        movss xmm1, scale
        movss dword ptr [eax + 20], xmm1
        fdivp st(2), st(0)
        fxch st(1)
        fstp ratio
        fld scale
        fdiv aspect
        movss xmm1, ratio
        movss dword ptr [eax + 40], xmm1
        mov dword ptr [eax + 44], 03f800000h
        fstp dword ptr [eax]
        fchs
        fmul ratio
        fstp dword ptr [eax + 56]
    }
}
void set_camera_fov_00b6fbb0(CameraProjection& camera, float value) {
    camera.valid_flags &= 0xffffff41u; camera.fov = value;
}
void set_camera_aspect_00b6fbd0(CameraProjection& camera, float value) {
    camera.valid_flags &= 0xffffff41u; camera.aspect = value;
}
void set_camera_near_00b6fbf0(CameraProjection& camera, float value) {
    camera.valid_flags &= 0xffffff41u; camera.near_plane = value;
}
void set_camera_far_00b6fc10(CameraProjection& camera, float value) {
    camera.valid_flags &= 0xffffff41u; camera.far_plane = value;
}
void set_camera_projection_00b6fd60(CameraProjection& camera, const CameraMatrix& source) {
    copy_camera_matrix_004134f0(camera.original, source);
    copy_camera_matrix_004134f0(camera.cached, source);
    camera.valid_flags = (camera.valid_flags & 0xffffff4bu) | 8;
}
const CameraMatrix& get_camera_projection_00b6fcf0(CameraProjection& camera) {
    if (!(camera.valid_flags & 8)) {
        CameraMatrix temporary;
        // Native getter spills each scalar through x87 while preparing stack args.
        float fov, aspect, near_plane, far_plane;
        const float* inputs = &camera.fov;
        __asm {
            mov eax, inputs
            fld dword ptr [eax + 12]
            fstp far_plane
            fld dword ptr [eax + 8]
            fstp near_plane
            fld dword ptr [eax + 4]
            fstp aspect
            fld dword ptr [eax]
            fstp fov
        }
        build_projection_00b642f0(temporary, fov, aspect, near_plane, far_plane);
        copy_camera_matrix_004134f0(camera.original, temporary);
        copy_camera_matrix_004134f0(camera.cached, camera.original);
        camera.valid_flags |= 8;
    }
    return camera.cached;
}
}
