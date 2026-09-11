#include "bsp/pose_refresh.hpp"

#include "bsp/camera_multiply.hpp"

namespace bsp {

PoseRefreshParentSlot::PoseRefreshParentSlot(PoseRefreshView*& actual_parent) noexcept
    : typed_(&actual_parent) {}

PoseRefreshParentSlot::PoseRefreshParentSlot(void*& actual_parent,
    PoseRefreshResolver& resolver) noexcept : owner_(&actual_parent), resolver_(&resolver) {}

PoseRefreshView* PoseRefreshParentSlot::get() const {
    if (typed_) return *typed_;
    void* const owner = *owner_;
    return owner ? &resolver_->resolve_pose(owner) : nullptr;
}

void refresh_pose_00414db0(PoseRefreshView& pose) {
    if (pose.world_valid_c8 != 0) return;
    if (auto* parent = pose.parent_3c.get()) refresh_pose_00414db0(*parent);

    CameraMatrix product; // native disjoint 40h stack scratch, filled by multiply
    const CameraMatrix* source = &pose.local_74;
    if (auto* parent = pose.parent_3c.get()) { // native reload after recursion
        multiply_camera_matrices_00413920(product, pose.local_74, parent->world_cc);
        source = &product;
    }
    copy_camera_matrix_004134f0(pose.world_cc, *source);
    pose.world_valid_c8 = 1;
    pose.derived_valid_10c = 0;
}

} // namespace bsp
