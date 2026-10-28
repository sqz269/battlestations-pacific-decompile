#pragma once

// Packet cc13-reconforce (census S14, src/game_hosts_lua.cpp GetProperty "reconlevel").

namespace bsp {

// kReconLevelForcedCc13Bound (OFF). 00779BB0's "reconlevel" arm (00779C0C..00779C33)
// walks the unit's three records at unit+1E8h + party * 34h and stores, per party,
// record+8h when the force byte record+10h is set, else record+4h: the forced level
// replaces the computed one for that party outright (not a max), whatever the side or
// coverage. 00805CF0 (from SetForcedReconLevel 008AA8F0) sets +10h = 1 and +8h = the
// level argument as given (00805D0A, 00805D0E), so a script's 4 reads back as 4;
// 00803B50 (ClearForcedReconLevel) clears +10h. The host keeps those writes in
// recon_sensor_pass.cpp's forced table, keyed (party, unit) like the record, with the
// raw level beside it (forced_recon_level_raw).
// ON: GetProperty answers the raw forced level for a forced (party, unit) and the
// host's computed level otherwise. OFF: the computed level only, as before.
// Not the image's lifetime: the forced table is process-wide and nothing calls
// clear_forced_recon_levels, while the image's records live and die with the unit.
// Movers: GetProperty(unit, "reconlevel")[party] after SetForcedReconLevel, read by
// luaGetReconLevel (commandhelpers.lua 1816), messagesender.lua 230 (ReconCheck message
// gates) and chg_7_cargo.lua (516, 707).
inline constexpr bool kReconLevelForcedCc13Bound = false;

}  // namespace bsp
