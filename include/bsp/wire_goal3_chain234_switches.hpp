#pragma once

#include "bsp/wire_goal4_chain340_switches.hpp"

// Goal 3 chain 234: the VehicleClass descriptor cache 00964790 builds
// (src/game_host_vehicle_class.cpp) and the 00964790 resolve over it in
// GameMissionLuaHost::mark_vehicle_class_row_00964790. OFF keeps the
// marked-row set as the cache stand-in and constructs no descriptor.
//
// Chain 234 landed OFF under the goal-3 rule, after the 2026-11-03 flip of
// every wiring switch (fa3b65aad), so it was never turned on. Goal 4 chain 340
// turns it on and owns the decision: kWireGoal4Chain340Bound OFF restores the
// old OFF state. Before the flip, all 22 kind rows were built through
// GameVehicleClassDescriptorCache::construct_descriptor, and IsKindOf was run
// on each (an offline probe; no constructor threw, ship kinds 0..7 alone
// answer kind 6, plane kinds 8..15 alone answer kind 0Fh). The installed
// vehicleclasses.lua has two LandingShip links (224 -> 90, 234 -> 40), and
// neither target has one, so the pre-cache LandingShip recursion ends.
namespace bsp {
inline constexpr bool kWireGoal3Chain234Bound = kWireGoal4Chain340Bound;
}
