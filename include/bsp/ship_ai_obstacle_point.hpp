#pragma once
#include "bsp/ship_ai_sector_scan.hpp"
#include <array>

namespace bsp {
// Complete 009D80C0/009D8160 schedules over the existing semantic obstacle node.
// Native ECX=node, stack point, RET4, AL result; these are new typed interfaces.
// Point-centre deltas and each dot product spill to float32 before sign-bit
// clearing and ordered comparison. Boundaries are inclusive; NaNs reject.
// Axes are used as supplied without normalization or extent adjustment.
//
// Both predicates use native+28..34, represented by axis_beam/axis_forward.
// The producer identifies these as FORWARD/BEAM respectively despite their
// historical C++ names. Avoid containment does not use corner_beam/forward.
// The node and point are read only; no footprint production is performed.
bool ship_ai_obstacle_point_in_near_box_009d80c0(
    const ShipAiObstacleNode&, const std::array<float, 2>& point) noexcept;
// Near rejects no_pose_68; avoid rejects either no_pose_68 or no_arc_69.
bool ship_ai_obstacle_point_in_avoid_box_009d8160(
    const ShipAiObstacleNode&, const std::array<float, 2>& point) noexcept;
} // namespace bsp
