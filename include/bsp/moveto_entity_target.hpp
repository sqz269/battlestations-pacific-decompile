#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

// The entity target produced by 009BE0F0. Its native DATA phase is a tag,
// never a callable Source vtable. The entity is borrowed for the task lifetime.
struct MoveToEntityTargetStorage {
    std::uint32_t profile_00;
    float initial_world_04[3];
    void* entity_10;
};
static_assert(sizeof(void*) == 4 && sizeof(MoveToEntityTargetStorage) == 0x14);
static_assert(offsetof(MoveToEntityTargetStorage, entity_10) == 0x10);

// 009C1C30 follows the target-point copy with 009BEBA0 before a state may
// steer. A reused host task has no constructor call: stage only X/Z, retain
// its current cruise/path height, and require the same full refresh before
// its next state tick. The target's raw altitude must never become a command.
inline void stage_move_to_entity_point(const MoveToEntityTargetStorage& target,
    float (&steer_point)[3], bool& refresh_pending) noexcept {
    steer_point[0] = target.initial_world_04[0];
    steer_point[2] = target.initial_world_04[2];
    refresh_pending = true;
}

// Merge retarget and periodic requests into one refresh. The caller retains
// the existing periodic countdown arithmetic/phase and runs this before the
// arrival tests and either move-to steering state.
template<class Refresh>
void refresh_move_to_target_before_steering(bool periodic_due,
    bool& retarget_pending, Refresh&& refresh) {
    if (!periodic_due && !retarget_pending) return;
    refresh();
    retarget_pending = false;
}

// Borrow the SAME entity's actual C8 flag and FC world-position cells. This
// permits the canonical host pose view without manufacturing a raw unit image.
struct MoveToEntityPoseView {
    const std::uint8_t& world_valid_c8;
    const float* world_position_fc;
};
class MoveToEntityPoseHost {
public:
    virtual ~MoveToEntityPoseHost() = default;
    // Existing raw callers retain their original field projection. A typed
    // runtime overrides only this lookup, not the constructor's branch/order.
    virtual MoveToEntityPoseView move_to_entity_pose(void* entity) {
        auto* bytes = static_cast<std::uint8_t*>(entity);
        return {*reinterpret_cast<std::uint8_t*>(bytes + 0xc8),
            reinterpret_cast<const float*>(bytes + 0xfc)};
    }
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
};

} // namespace bsp
