#pragma once
#include "bsp/camera_projection.hpp"

namespace bsp {
// New storage/lifetime interface. Parents must remain alive and form an acyclic
// hierarchy. Zero initialization is not the native constructor. Callers must
// invalidate dependent caches when changing local/parent transforms.
struct CameraTransform {
    CameraTransform* parent{}; // native+30
    std::uint32_t valid_flags{}; // native+5C
    CameraMatrix view{}, local{}, world{}; // native+60,+B0,+F0
};
struct CameraState {
    CameraTransform transform;
    CameraProjection projection;
    CameraMatrix view_projection{}; // native+220; valid bit10 in projection flags
};
// Native thiscall, no stack arguments. Refresh always recomputes this world;
// only parent refresh is skipped when its world-valid bit2 is already set.
void refresh_camera_world_00b6db70(CameraTransform&);
const CameraMatrix& get_camera_view_00b6fcb0(CameraTransform&);
const CameraMatrix& get_camera_view_projection_00b70490(CameraState&);
}
