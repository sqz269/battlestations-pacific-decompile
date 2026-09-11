#include "bsp/camera_transform.hpp"
#include "bsp/camera_affine.hpp"
#include "bsp/camera_inverse.hpp"
#include "bsp/camera_multiply.hpp"

namespace bsp {
namespace {
void forward_world_changed(void* context, CameraTransform& transform) {
    (*static_cast<void (**)(CameraTransform&)>(context))(transform);
}
}
CameraState::CameraState() noexcept
    : transform(owned_.transform), projection(owned_.projection),
      view_projection(owned_.view_projection), direction(owned_.direction), target(owned_.target) {}
CameraState::CameraState(CameraTransform& actual_transform, CameraProjection& actual_projection,
    CameraMatrix& actual_view_projection, std::array<float, 3>& actual_direction,
    std::array<float, 3>& actual_target) noexcept
    : transform(actual_transform), projection(actual_projection),
      view_projection(actual_view_projection), direction(actual_direction), target(actual_target) {}
CameraState::CameraState(const CameraState& other) noexcept : CameraState() { *this = other; }
CameraState::CameraState(CameraState&& other) noexcept
    : CameraState(static_cast<const CameraState&>(other)) {}
CameraState& CameraState::operator=(const CameraState& other) noexcept {
    if (this == &other) return *this;
    transform = other.transform;
    projection = other.projection;
    view_projection = other.view_projection;
    direction = other.direction;
    target = other.target;
    return *this;
}
CameraState& CameraState::operator=(CameraState&& other) noexcept {
    return *this = static_cast<const CameraState&>(other);
}

CameraTransform::CameraTransform() noexcept
    : parent(owned_.parent), first_child(owned_.first_child), child_count(owned_.child_count),
      next_sibling(owned_.next_sibling), previous_sibling(owned_.previous_sibling),
      root_list(owned_.root_list), valid_flags(owned_.valid_flags),
      auxiliary_flags(owned_.auxiliary_flags), notification_context(owned_.notification_context),
      view(owned_.view), local(owned_.local), world(owned_.world) {}

CameraTransform::CameraTransform(CameraTransformBacking backing,
    void (*actual_notify_changed)(void*)) noexcept
    : parent(backing.parent), first_child(backing.first_child), child_count(backing.child_count),
      next_sibling(backing.next_sibling), previous_sibling(backing.previous_sibling),
      root_list(backing.root_list), valid_flags(backing.valid_flags),
      auxiliary_flags(backing.auxiliary_flags), notification_context(backing.notification_context),
      notify_changed(actual_notify_changed), view(backing.view), local(backing.local), world(backing.world) {}

CameraTransform::CameraTransform(const CameraTransform& other) noexcept : CameraTransform() {
    *this = other;
}
CameraTransform::CameraTransform(CameraTransform&& other) noexcept
    : CameraTransform(static_cast<const CameraTransform&>(other)) {}

CameraTransform& CameraTransform::operator=(const CameraTransform& other) noexcept {
    if (this == &other) return *this;
    parent = other.parent;
    first_child = other.first_child;
    child_count = other.child_count;
    next_sibling = other.next_sibling;
    previous_sibling = other.previous_sibling;
    root_list = other.root_list;
    valid_flags = other.valid_flags;
    auxiliary_flags = other.auxiliary_flags;
    notification_context = other.notification_context;
    notify_changed = other.notify_changed;
    view = other.view;
    local = other.local;
    world = other.world;
    return *this;
}
CameraTransform& CameraTransform::operator=(CameraTransform&& other) noexcept {
    return *this = static_cast<const CameraTransform&>(other);
}

void derive_camera_local_from_world_00b6e7e0(CameraTransform& transform) {
    if (transform.parent) {
        const auto& parent_inverse = get_camera_view_00b6fcb0(*transform.parent);
        CameraMatrix temporary;
        multiply_camera_matrices_00413920(temporary, transform.world, parent_inverse);
        copy_camera_matrix_004134f0(transform.local, temporary);
    } else {
        copy_camera_matrix_004134f0(transform.local, transform.world);
    }
}
void notify_camera_world_changed_00b6dbe0(CameraTransform& transform) {
    transform.auxiliary_flags &= 0xffffffcfu;
    if (transform.notify_changed) transform.notify_changed(transform.notification_context);
}
void set_transform_world_matrix_00b6e870(CameraTransform& transform, const CameraMatrix& source,
    void (&world_changed)(CameraTransform&)) {
    auto* callback = &world_changed;
    set_transform_world_matrix_00b6e870(transform, source, &callback, forward_world_changed);
}
void set_transform_world_matrix_00b6e870(CameraTransform& transform, const CameraMatrix& source,
    void* callback_context, void (&world_changed)(void*, CameraTransform&)) {
    copy_camera_matrix_004134f0(transform.world, source);
    if (transform.notify_changed) transform.notify_changed(transform.notification_context);
    derive_camera_local_from_world_00b6e7e0(transform);
    invalidate_camera_descendants_00b6da30(transform);
    world_changed(callback_context, transform);
    transform.valid_flags = 2;
}
void set_camera_world_matrix_00b71460(CameraState& camera, const CameraMatrix& source) {
    camera.projection.valid_flags &= 0xfffffe4bu;
    set_transform_world_matrix_00b6e870(camera.transform, source, notify_camera_world_changed_00b6dbe0);
    refresh_camera_direction_00b70660(camera);
}

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
