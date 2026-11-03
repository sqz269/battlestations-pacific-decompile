#pragma once

namespace bsp {
// Terrain normals and line/quad intersections use the recovered cross-product
// assignment instead of the host's duplicate arithmetic. OFF keeps that code.
inline constexpr bool kWireGoal4Chain329Bound = true;
}
