#include "bsp/force_event_spatial.hpp"

#include <stdexcept>

namespace bsp {

ForceEventSpatialRuntime::ForceEventSpatialRuntime(ForceEventCurrentTargetLookup& current_target,
    ForceEventSubjectTransformLookup& subject_transform, PoseRefreshResolver& poses)
    : current_target_(current_target), subject_transform_(subject_transform), poses_(poses) {
    if (!current_target_ || !subject_transform_)
        throw std::invalid_argument("force-event spatial lookups must be bound");
}

ForceEventTargetPose* ForceEventSpatialRuntime::current_target_00e188a8_1ed4() {
    return current_target_();
}

CameraTransform& ForceEventSpatialRuntime::subject_transform_110(void* actual_subject) {
    return subject_transform_(actual_subject);
}

void ForceEventSpatialRuntime::refresh_target_pose_00414db0(ForceEventTargetPose& target) {
    if (!target.identity) throw std::invalid_argument("force-event target has no actual owner identity");
    auto& pose = poses_.resolve_pose(target.identity);
    if (&pose.world_valid_c8 != &target.world_valid_c8 || &pose.world_cc != &target.world_cc)
        throw std::invalid_argument("force-event target and resolved pose must bind the same flag and world fields");
    refresh_pose_00414db0(pose);
}

} // namespace bsp
