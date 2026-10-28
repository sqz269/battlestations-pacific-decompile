#pragma once

// Packet cc13-reconclear (smbind finding). Header-only switch.

namespace bsp {

// kForcedReconMissionClearCc13Bound (OFF). In the image, SetForcedReconLevel's
// 00805CF0 writes a record held inside the target unit: unit+1E8h + party*34h, with
// the force byte at record+10h and the level at +8h. The record dies with the unit
// object, so nothing forced outlives the mission's world.
// The host keeps the forced pairs in a process-wide table in recon_sensor_pass.cpp,
// keyed by (party, unit index). bsp::clear_forced_recon_levels() had no caller, so the
// pairs survived into the next mission of a multi-mission process. There they forced
// whatever unit reused the index.
// ON: GameMissionLuaHost::attach_script_orders(nullptr), the world teardown
// (release_units) that already drops the world-lived dummy kamikaze rounds, catapult
// records and spawn queue, also calls clear_forced_recon_levels(). That empties the
// table and the pending 00803B50 clears.
// The clear runs at teardown only, not at attach, so a SetForcedReconLevel issued
// before the attach of the same mission is kept. The natives cannot write after the
// detach, because attach_units_hooks(nullptr) follows it and an unresolved entity
// writes nothing.
// Death and removal: the host never erases or reuses a unit slot within a mission
// (game_hosts_units.cpp has no slots erase, reset or clear). So a dead unit's pair can
// only force that same dead unit until teardown. LABELLED: kept until teardown, where
// the image would drop it when the unit object is deleted.
// Movers when ON: only the second and later missions of one process whose earlier
// mission called SetForcedReconLevel (561 calls in this installation's Lua, including
// global\commandhelpers.lua). A single-mission run is unchanged.
inline constexpr bool kForcedReconMissionClearCc13Bound = false;

}  // namespace bsp
