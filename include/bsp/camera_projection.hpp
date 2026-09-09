#pragma once
#include <array>
#include <cstdint>

namespace bsp {
using CameraMatrix = std::array<float, 16>;
// New owning projection of scalar/cache fields, not a native camera object.
// Initial zeros are interface defaults, not recovered constructor defaults.
struct CameraProjection {
    float fov{}, aspect{}, near_plane{}, far_plane{}; // +1C4,+1C8,+1D4,+1D8
    CameraMatrix original{}, cached{}; // +1E0,+2A0
    std::uint32_t valid_flags{}; // +2F0
};
// Native ECX destination; four stack floats; RET10h. No input validation.
// Inlines00412e20 FSINCOS tangent with native float32 spill boundaries.
void build_projection_00b642f0(CameraMatrix&, float fov, float aspect,
    float near_plane, float far_plane);
void set_camera_fov_00b6fbb0(CameraProjection&, float);
void set_camera_aspect_00b6fbd0(CameraProjection&, float);
void set_camera_near_00b6fbf0(CameraProjection&, float);
void set_camera_far_00b6fc10(CameraProjection&, float);
void set_camera_projection_00b6fd60(CameraProjection&, const CameraMatrix&);
const CameraMatrix& get_camera_projection_00b6fcf0(CameraProjection&);
}
