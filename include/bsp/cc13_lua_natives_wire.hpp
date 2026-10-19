#pragma once

// Packet cc13-luawire: switches that wire the mission Lua natives the census
// (docs/CC13_STUB_CENSUS.md, rows 3-6) found unhosted to their reconstructed
// sequences. One parent switch per native. Every switch starts OFF; cc9 flips.
// OFF, the trampoline keeps the row unhandled, so the native records its
// UNIMPLEMENTED line and returns no value, exactly as before the packet.

namespace bsp {

// 008BE7E0 HasFired(unit, weaponName, window) -> boolean. Argument 0 through
// 00888AA0 (no null test), then vtable[5Ch](2); a unit's director is
// vtable[114h](); a null director, when the unit is kind 0Fh (a plane), falls
// back to [unit+9D4h]'s vtable[114h]() (the squadron's command block). The name
// is indexed in the 20-entry table 00E08E58 by 006E8250 (_stricmp; no match: 0). The
// answer is stamp > (double([00F876A4]) - window): the stamp is
// [director + 1D0h + index*4], seeded -1e10 (bits D01502F9h, 00720180) and
// written only by 009555A0 (0095561B) at each projectile spawn with the clock
// [00F876A4]; FSUBR then FCOMIP/JA, so a NaN answers false.
// True: GameMissionLuaHost::run_has_fired_008be7e0 answers it, with the stamp
// taken as the latest gun+474h (last_fire_seconds) of the director's guns of
// that sub-type (SUBSTITUTION, labelled there: dual-purpose second ammunition
// and rack drops are not covered).
// False: the row stays unhandled (UNIMPLEMENTED, no value pushed).
inline constexpr bool kLuaHasFiredBound = false;

// 00893380 AddWaterLoad(unit, side, amount). The hull-local point
// (+-Width/2 by side parity, 0, +Length/2 / 0 / -Length/2 by side < 2 / < 4 /
// else), amount > 0 and not kind 8, count = trunc(clamp(amount / 10, 0, 63));
// a non-zero count builds message 90h (0080FF80) and routes it with flags 7
// (0077C2A0) to the unit's own receiver 008221A7 -> 0074F440, which takes the
// point into the world by the pose at +CCh before 0074F090.
// True: GameMissionLuaHost::run_add_water_load_00893380 hands the world point
// and count to GameUnitsHost::add_leak_0074f440, the host's 90h receiver (its
// own kLiveHullLeakBound / kLiveHullRepairBound still apply).
// False: the row stays unhandled (UNIMPLEMENTED, no value pushed).
inline constexpr bool kLuaAddWaterLoadBound = false;

// 008914F0 BreakShip(unit). Argument 0 through 00888AA0, then, when the unit's
// controller ([unit+1018h], 0080E490) has both halves (+370h by 0092BEA0 and
// +374h by 0092BE90), 00814520: 004A5AA0 on the breakup registry when the class
// has breakup pieces (0080F9F0), then 00935D30 BSP_UnitController_BreakInTwo,
// which takes 11 stream-1 draws, then 0092A240 per piece (front, back): n =
// trunc(clamp(Length x 0.2 / 10, 1, 2)) draws U(-w, w) each when the class has a
// DeathEfx (+55Ch) and the game view node is set; it clears +370h/+374h and
// kills the unit (00926D90(1)). Returns no value.
// True: GameMissionLuaHost::run_break_ship_008914f0 gates on the units host's
// front/back nodes less its own cleared-halves mark, takes 00935D30's eleven
// stream-1 draws and 0092A240's puff draws (GameGunneryHost::
// death_mode_draw_00bd2f10) and kills through the 00926D90 funnel with cause 1.
// False: the row stays unhandled (UNIMPLEMENTED, no value pushed).
inline constexpr bool kLuaBreakShipBound = false;

}  // namespace bsp
