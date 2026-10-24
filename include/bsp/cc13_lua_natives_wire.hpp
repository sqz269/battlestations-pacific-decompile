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
inline constexpr bool kLuaHasFiredBound = true;  // ON (cc9-lua71, 2026-10-19): BSM01 108 calls, gameplay identical; see WINS_TABLE

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
inline constexpr bool kLuaAddWaterLoadBound = true;  // ON (cc9-lua71, 2026-10-19): BSM09 6 Lexington loads, gameplay identical; see WINS_TABLE

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
inline constexpr bool kLuaBreakShipBound = true;  // ON (cc9-lua71, 2026-10-19): JM06 ambush breaks US Tanker 02 at 331.09 s (jm06.lua luaJM6BreakShips), row moves, still completes; see WINS_TABLE

// 008A6C30 UnitSetPlayerCommandsEnabled(unit, mask). Argument 0 through 00888AA0,
// its director vtable[114h], argument 1 as an integer, then 0071DD30: message 5Ah
// sub-kind 2, routed with flags 7, which 0071C1E0 stores at director+1CCh. The
// readers are the HUD's 0071C060 and the network snapshot 00721280, so nothing
// in the simulation changes. Returns no value. The director constructor seeds
// +1CCh = 1FFh (007202F3), so any future reader must default an absent entry to
// 0x1FF and clear the map per scene. There is no class test before vtable[114h].
// True: GameMissionLuaHost::run_unit_set_player_commands_enabled_008a6c30 stores
// the mask in the host's per-director map (no host reader) and records it.
// False: the row stays unhandled (UNIMPLEMENTED, no value pushed).
inline constexpr bool kLuaUnitSetPlayerCommandsEnabledBound = true;  // ON (cc9-lua71, 2026-10-19): BSM01/BSM09, no simulation reader, identical; see WINS_TABLE

// Part 7b. Four natives. Each switch selects both the Lua runner (the row is
// handled) and the units-host readers of the state it writes. OFF, the row
// stays unhandled and every reader keeps its old constant.

// 008A13D0 SetVisibility(entity, visible). A squadron (18h): for i below
// [+3CCh], member = i <= 4 ? [+3D0h + 4i] : null, then 00951F40(visible) on
// it, with no null test. Otherwise a kind-5 entity: 00951F40 on itself.
// 00951F40 attaches (1) or detaches (0) the part +4A4h's spatial node, which
// is this host's plane_hit_node_detached = !visible.
// True: GameMissionLuaHost::run_set_visibility_008a13d0 calls
// GameUnitsHost::set_plane_visible_00951f40 per member (at most five; the
// image's null sixth member, a crash there, is GUARDED and logged).
// False: the row stays unhandled (UNIMPLEMENTED, no value pushed).
inline constexpr bool kLuaSetVisibilityBound = true;  // ON (cc9-lua71, 2026-10-19): BSM09 D3A Val 5, row moves, same end; see WINS_TABLE

// 008A0000 SquadronEnableVehicleAvoidance(entity, on). No class test:
// [+3A4h] = on ? v | 2 : v & 0x000000FD (25 FD 00 00 00, so bits 8-31 clear
// too). The squadron constructor stores 0FFh (007F2C13). The reader is the
// pilot gate 009A1959-009A1969: 0099B670 runs when pilot+2E4h bit 2 is set
// AND (pilot+2FCh, the squadron, is null OR its +3A4h bit 2 is set).
// The second gate 009A1974-009A1987 also tests +2FCh and +3A4h bit 0; the
// host does not read it yet.
// True: run_squadron_enable_vehicle_avoidance_008a0000 writes the slot's
// sq_avoid_3a4 and the host's 009A1951 gate also tests the squadron's bit 2.
// False: the row stays unhandled and the gate tests +2E4h only.
inline constexpr bool kLuaSquadronEnableVehicleAvoidanceBound = true;  // ON (cc9-lua71, 2026-10-19): BSM09 five Val squadrons off, row moves, same end; see WINS_TABLE

// 0089FAF0 SquadronSetBaseUnsupported(squadron, base). 006CA730 appends the
// squadron to the base block's std::list at +B4h (base+72Ch for class 45h,
// else base+1188h). 006C4790 (false for a null squadron or a listed one)
// reads it at 007F1748 (the RTB home arm), 0099A404 (the land command arm)
// and 009B3537 (the land validity test), and at 006C564D, which the host
// does not carry (007C5D21's gear region).
// True: run_squadron_set_base_unsupported_0089faf0 adds the (deck, squadron)
// pair and those three host sites consult the pairs.
// False: the row stays unhandled and the list is taken as empty.
inline constexpr bool kLuaSquadronSetBaseUnsupportedBound = true;  // ON (cc9-lua71, 2026-10-19): BSM09 Donald on both carriers; USN01 identical; see WINS_TABLE

// 008A62A0 SetCheatTurbo(entity, value). A squadron (18h) writes float
// +340h on members 0..4, stopping at the first null; otherwise a kind-5
// entity gets +340h itself. The readers:
//  - 0099D479: dtScale = max(+340h x 0.4, 1). 1/dtScale feeds 0099DEBD (the
//    heading term), 0099E2D3 (the roll servo) and 0099E604 (the pitch
//    demand); dt/dtScale overwrites the dt slot that 0099D781 (+2B8h decay)
//    and the pending term read.
//  - 009FBB20: lead = max(+340h x 2, 10) and the followed distance scale
//    max(+340h x 0.4, 1).
// No constructor writes +340h; the host's 0 ASSUMES a zeroed allocation.
// True: run_set_cheat_turbo_008a62a0 stores it and those host sites take the
// plane's cheat_turbo_340 (0 until set).
// False: the row stays unhandled and the sites keep their constants (0).
inline constexpr bool kLuaSetCheatTurboBound = true;  // ON (cc9-lua71, 2026-10-19): BSM09 16 calls, row moves, same end; see WINS_TABLE

}  // namespace bsp
