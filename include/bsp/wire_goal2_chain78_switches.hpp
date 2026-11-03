#pragma once

namespace bsp {
// Goal 2, chain 78, one arm in src/game_hosts_avoid_zones.cpp:
// - 00423960 terrain_layer_set_dimension_00423960(10), called by the layer
//   constructor 00424680 at 004246B4, over a typed view of the host's
//   TerrainGridLayerRecord (TerrainGridLayerRecordDimensionFields); OFF keeps
//   the inline +04h/+08h/+0Ch/+10h/+18h stores of default_layer_00424680.
inline constexpr bool kWireGoal2Chain78Bound = true;
}
