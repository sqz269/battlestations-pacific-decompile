#pragma once

#include "bsp/gamepad_force_events.hpp"
#include "bsp/pose_refresh.hpp"

#include <functional>

namespace bsp {

// Pure lookups of the current game's +1ED4 target and the supplied subject's
// +110h canonical CameraTransform. The owner must reload the actual current
// game/target on each call and return existing views, never copied pose state.
using ForceEventCurrentTargetLookup = std::function<ForceEventTargetPose*()>;
using ForceEventSubjectTransformLookup = std::function<CameraTransform&(void*)>;

// Concrete composition of the recovered event and pose algorithms. It borrows
// both callable lvalues and the resolver; they and all returned views/fields
// must remain alive. No target, game, transform, matrix or parent is cached.
class ForceEventSpatialRuntime final : public ForceEventSpatialHost {
public:
    ForceEventSpatialRuntime(ForceEventCurrentTargetLookup&,
        ForceEventSubjectTransformLookup&, PoseRefreshResolver&);
    ForceEventTargetPose* current_target_00e188a8_1ed4() override;
    CameraTransform& subject_transform_110(void* actual_subject) override;
    // Rejects null identity or mismatched flag/matrix field addresses before
    // refreshing. Resolver identities must refer to the same existing owner.
    void refresh_target_pose_00414db0(ForceEventTargetPose&) override;
private:
    ForceEventCurrentTargetLookup& current_target_;
    ForceEventSubjectTransformLookup& subject_transform_;
    PoseRefreshResolver& poses_;
};

} // namespace bsp
