#pragma once
#include "bsp/camera_projection.hpp"

namespace bsp {
// New storage/lifetime interface. Parents must remain alive and form an acyclic
// hierarchy. Zero initialization is not the native constructor. Use recovered
// setters for edits; parent changes/ancestor camera-cache callbacks remain open.
struct CameraTransform {
    CameraTransform* parent{}; // native+30
    CameraTransform* first_child{}; // +34; caller supplies consistent sibling links
    CameraTransform* next_sibling{}; // +3C
    std::uint32_t valid_flags{}; // native+5C
    std::uint32_t auxiliary_flags{}; // +138
    // Explicit adapter for attached+A0 virtual+3C. Null invoke means no attachment.
    void* notification_context{};
    void (*notify_changed)(void*){};
    CameraMatrix view{}, local{}, world{}; // native+60,+B0,+F0
};
struct CameraState {
    CameraTransform transform;
    CameraProjection projection;
    CameraMatrix view_projection{}; // native+220; valid bit10 in projection flags
    std::array<float, 3> direction{}, target{}; // native+1AC,+1A0; semantic names provisional
};
// Native thiscall, no stack arguments. Refresh always recomputes this world;
// only parent refresh is skipped when its world-valid bit2 is already set.
void refresh_camera_world_00b6db70(CameraTransform&);
const CameraMatrix& get_camera_view_00b6fcb0(CameraTransform&);
const CameraMatrix& get_camera_view_projection_00b70490(CameraState&);
void invalidate_camera_descendants_00b6da30(CameraTransform&);
void set_transform_local_matrix_00b6db10(CameraTransform&, const CameraMatrix&);
void refresh_camera_direction_00b70660(CameraState&);
void set_camera_local_matrix_00b71430(CameraState&, const CameraMatrix&);
}
