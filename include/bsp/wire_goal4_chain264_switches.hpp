#pragma once

namespace bsp {

// Goal 4 chain 264: the class-table resets of BSP_Game_LoadMissionScene
// (004E0AAF..004E0ACD) and BSP_EngineMovie_Stop (0045F6D5..0045F6F8). Both run
// "<Table>=nil\n" through 006B8AD0 on the mission Lua machine right after
// luaPrecacheUnits and the VehicleClass purge (0095CA70). True: the seven resets
// whose tables no host reader consults afterwards run through their bodies, in image
// order, on the host's mission Lua state: 004B1890 WreckClass, 004AF930 SoldierClass,
// 00474200 BuoyClass, 00476410 CloudClass, 004A7120 PlanePartClass, 006D74B0
// MarkerClasses, 00869D20 Effects. The three whose tables the host still reads during
// the mission (004423A0 DeviceClass, 006E9C50 Bullets, 0084EB60 StationaryClass, all
// read lazily in game_hosts_lua.cpp) stay records. False: the scene load runs none of
// them, and the movie stop keeps its single class_table_resets record.
// Only the mission scene load and the engine movie stop change; switch-only, no game
// run was made for this chain.
inline constexpr bool kWireGoal4Chain264Bound = true;

}  // namespace bsp
