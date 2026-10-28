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
// (cc13-reconclear's kForcedReconMissionClearCc13Bound clears both tables at world
// teardown.)
// Image quirk, not modelled (latent): SetForcedReconLevel's squadron member loop
// (008AAA8E..008AAACB) reads the member from squadron+3D0h + i*4 only while i <= 4
// (008AAAA1 CMP EBP,4 / JA). Above 4, EAX = 0 (008AAAAE), so the record passed to
// 00805CF0 at 008AAAB8 is the null unit's: ECX = party*34h + 1E8h, a near-null
// address. The host's member loop in run_set_forced_recon_level_008aa8f0 forces every
// member_units entry instead.
// Movers: GetProperty(unit, "reconlevel")[party] after SetForcedReconLevel, read by
// luaGetReconLevel (commandhelpers.lua 1816), messagesender.lua 230 (ReconCheck message
// gates) and chg_7_cargo.lua (516, 707).
// ON (cc9-recon2): JM04, JM12, LOMP06, USN02 pairs gameplay identical (single-mission runs).
inline constexpr bool kReconLevelForcedCc13Bound = true;

}  // namespace bsp
