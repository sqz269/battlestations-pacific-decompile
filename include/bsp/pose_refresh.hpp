#pragma once

#include "bsp/camera_projection.hpp"

namespace bsp {

struct PoseRefreshView;
class PoseRefreshResolver {
public:
    virtual ~PoseRefreshResolver() = default;
    // Pure identity lookup of an existing borrowed view, with no allocation,
    // matrix copy, retention, or fallback for an unsupported nonnull owner.
    virtual PoseRefreshView& resolve_pose(void* actual_owner) = 0;
};

// A read-only binding to the owner's actual mutable +3Ch parent field. The
// typed-link form uses the canonical companion pointer directly; the raw-owner
// form resolves that identity on each read. Neither stores a parent snapshot.
class PoseRefreshParentSlot final {
public:
    explicit PoseRefreshParentSlot(PoseRefreshView*& actual_parent) noexcept;
    PoseRefreshParentSlot(void*& actual_parent, PoseRefreshResolver&) noexcept;
    PoseRefreshView* get() const;
private:
    PoseRefreshView* const* const typed_{};
    void* const* const owner_{};
    PoseRefreshResolver* const resolver_{};
};

// Borrowed field projection, not a new transform owner or native object ABI.
// It has no default matrices, flags, or hierarchy. All referenced storage and
// parent views must remain alive. This +74h/+CCh pose differs from the render
// CameraTransform's +B0h/+F0h matrices; only the CameraMatrix type is shared.
struct PoseRefreshView {
    PoseRefreshParentSlot parent_3c;
    CameraMatrix& local_74;
    std::uint8_t& world_valid_c8;
    CameraMatrix& world_cc;
    std::uint8_t& derived_valid_10c; // meaning beyond this zero store unresolved
};

// Native ECX=pose owner, no stack arguments, RET at 00414E08. No meaningful
// return contract: the clean fast path leaves EAX untouched. Any nonzero C8
// skips every parent/matrix access and preserves 10C. Dirty ancestry must not
// contain a cycle; native has no cycle guard or in-progress valid mark.
void refresh_pose_00414db0(PoseRefreshView&);

} // namespace bsp
