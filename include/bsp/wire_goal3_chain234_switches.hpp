#pragma once

// Goal 3 chain 234: the VehicleClass descriptor cache 00964790 builds
// (src/game_host_vehicle_class.cpp) and the 00964790 resolve over it in
// GameMissionLuaHost::mark_vehicle_class_row_00964790. OFF keeps the
// marked-row set as the cache stand-in and constructs no descriptor.
namespace bsp {
inline constexpr bool kWireGoal3Chain234Bound = false;
}
