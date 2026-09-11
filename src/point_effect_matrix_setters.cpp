#include "bsp/point_effect_matrix_setters.hpp"
#include "bsp/camera_multiply.hpp"
#include "bsp/material_transform_constants.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void dispatch_current_world_changed(void* context, CameraTransform& transform) {
    auto& scenes = *static_cast<SceneAttachmentRuntime*>(context);
    auto& binding = scenes.resolve(transform);
    const auto invoke = binding.world_changed;
    if (!invoke) throw std::logic_error("node world matrix requires current virtual+40");
    invoke(scenes, binding);
}
SceneWorldMatrixOverride require_world_matrix(SceneNodeAttachment& binding) {
    const auto invoke = binding.set_world_matrix;
    if (!invoke) throw std::logic_error("point-effect matrix setter requires current node virtual+34");
    return invoke;
}
}

void set_native_node_world_matrix_00b6e870(SceneAttachmentRuntime& scenes,
    SceneNodeAttachment& binding, const CameraMatrix& source) {
    if (binding.transform.notification_context && !binding.transform.notify_changed)
        throw std::logic_error("node world matrix requires actual attachment virtual+3C");
    // Bindings project actual native+A0 notification and current virtual+40;
    // the shared implementation retains both native callback points.
    set_transform_world_matrix_00b6e870(binding.transform, source,
        &scenes, dispatch_current_world_changed);
}

void set_point_effect_world_matrix_0053d9c0(PointEffectInstanceStorage& effect,
    const CameraMatrix& source, SceneAttachmentRuntime& scenes) {
    auto& node = effect.node_110->scene_attachment;
    const auto invoke = require_world_matrix(node);
    invoke(scenes, node, source); // 53D9D7, original matrix pointer preserved
    auto* parent = effect.parent_8c; // 53D9D9: reload AFTER the virtual call
    if (parent) {
        const auto& inverse = get_transform_inverse_world_00b6e0d0(*parent);
        CameraMatrix temporary;
        multiply_camera_matrices_00413920(temporary, source, inverse);
        copy_camera_matrix_004134f0(effect.relative_d0, temporary);
    } else {
        copy_camera_matrix_004134f0(effect.relative_d0, source);
    }
}

void set_point_effect_relative_matrix_0072aa80(PointEffectInstanceStorage& effect,
    const CameraMatrix& source, SceneAttachmentRuntime& scenes) {
    copy_camera_matrix_004134f0(effect.relative_d0, source); // 72AA95
    auto* parent = effect.parent_8c; // 72AA9A: captured for this entire branch
    if (parent) {
        if ((parent->valid_flags & 2u) == 0) refresh_camera_world_00b6db70(*parent);
        // Native captures the current node table before multiply, then loads
        // its +34 entry and the current node receiver after multiply. The
        // canonical multiply has no callback and cannot change the table.
        auto& dispatch_binding = effect.node_110->scene_attachment;
        CameraMatrix temporary;
        multiply_camera_matrices_00413920(temporary, effect.relative_d0, parent->world);
        const auto invoke = require_world_matrix(dispatch_binding);
        auto& receiver = effect.node_110->scene_attachment;
        invoke(scenes, receiver, temporary);
    } else {
        auto& node = effect.node_110->scene_attachment;
        const auto invoke = require_world_matrix(node);
        invoke(scenes, node, effect.relative_d0); // actual+D0, no temporary
    }
}

} // namespace bsp
