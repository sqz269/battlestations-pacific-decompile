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
