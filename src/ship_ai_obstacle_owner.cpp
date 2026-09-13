#include "bsp/ship_ai_obstacle_owner.hpp"

namespace bsp {

void ship_ai_obstacle_owner_initialize_projection_009e52e0(
    ShipAiObstacleNode& node, ShipAiNeighbourNodeMotion& motion,
    const void* observed_owner, float lifetime) noexcept {
    // 009E530A clears +14; 009E532A assigns a nonnull owner before the native
    // observer registration. Only the resulting semantic field is projected.
    node.owner = observed_owner;

    // 009E533B..009E534D selects the argument or 00D7A260 (-1.0f), and
    // 009E5359 stores it. No duration clamp or floating-point arithmetic.
    node.lifetime_78 = observed_owner != nullptr ? lifetime : -1.0f;
    node.no_arc_69 = false; // 009E5361
    node.pass_side_88 = 0;  // 009E5364
    node.no_pose_68 = true; // 009E5377
    motion.bounds_min_y_84 = kShipAiNeighbourBoxNoBounds; // 009E5380
    motion.bounds_max_y_80 = kShipAiNeighbourBoxNoBounds; // 009E5388
}

} // namespace bsp
