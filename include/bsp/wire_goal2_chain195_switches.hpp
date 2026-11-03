#pragma once

// Goal 2, chain 195 (slot g2-w1): segment 61 [0092E0B0-00951F20] range sweep.
// OFF keeps every host stand-in byte for byte. ON routes:
//   009481A0 spawn_request_read_exclude_radius_009481a0 (typed view) from
//            GameMissionLuaHost::run_spawn_new_00949750 (src/game_hosts_lua.cpp): the
//            `excludeRadiusOverride` keys are read by the body through the reconstructed
//            00BD68D0 reader, over the block 00949750 presets (100, 2000, 100, 500, 0).
// Runtime-unvalidated; flip in a batch across missions that pass excludeRadiusOverride
// (bsm_01, bsm_02, commandhelpers.lua).

namespace bsp {

inline constexpr bool kWireGoal2Chain195Bound = true;

}  // namespace bsp
