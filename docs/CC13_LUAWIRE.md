# CC13 luawire: unhosted mission Lua natives wired behind OFF switches

Packet cc13-luawire (cc13-stubcensus, 2026-10-19), from census rows 3-5 of
`docs/CC13_STUB_CENSUS.md`. The switches are in `include/bsp/cc13_lua_natives_wire.hpp`,
and all of them are OFF. With OFF, the trampoline row stays unhandled, so the native keeps
its UNIMPLEMENTED record line and pushes no value. Each summary line prints only when its
switch is ON. Status: reconstructed and build-tested; not game-validated. The pairs go to cc9.

| Switch | Native | Runner (`src/game_hosts_lua.cpp`) | Body checked against the listing |
| --- | --- | --- | --- |
| `kLuaHasFiredBound` | `008BE7E0` HasFired | `run_has_fired_008be7e0` | `src/unit_weapons.cpp` `lua_has_fired_008be7e0` (fixed by this packet) |
| `kLuaAddWaterLoadBound` | `00893380` AddWaterLoad | `run_add_water_load_00893380` | `src/cc13_left55_seg56a.cpp` `lua_add_water_load` (unchanged, matches) |
| `kLuaBreakShipBound` | `008914F0` BreakShip | `run_break_ship_008914f0` | `src/options_seg56_cc13.cpp` `lua_break_ship` (unchanged, matches) |
| `kLuaUnitSetPlayerCommandsEnabledBound` | `008A6C30` UnitSetPlayerCommandsEnabled | `run_unit_set_player_commands_enabled_008a6c30` | `src/cc13_left55_seg56a.cpp` `lua_unit_set_player_commands_enabled` (unchanged, matches) |

## HasFired, 008BE7E0

Stack shape: argument 0 is an entity read through `00888AA0` with no null test; argument 1 is
a string (`00B662B0`); argument 2 is a number (`00B66270`, left in ST0 as a double). One
boolean is pushed on every path (`008BE9FA`), and the native returns `00B66400`.

The sequence:
1. `vtable[5Ch](2)` (`008BE901`).
2. The director comes from `vtable[114h]`. When it is null, only a plane (`vtable[5Ch](0Fh)`,
   `008BE926`) falls back to `[unit+9D4h]`'s `vtable[114h]`, again with no null test on
   `+9D4h`.
3. The name's index comes from `006E8250` over the table at `00E08E58`. The compare is
   `_stricmp` (`00BF7FBF`), so it ignores case; no match gives 0.
4. The stamp is `[director+1D0h+index*4]` (`008BE99C`).
5. The answer is `stamp > float(double([00F876A4]) - arg2)`: `FSUBR` at 24-bit precision,
   then `FCOMIP`/`JA`, so a NaN answers false.

Earlier body, now corrected: it tested class 18h; it used the `+9D4h` fallback without the
0Fh test and with a null test the image does not have; and it folded the predicate into one
host call. Its ledger record was replaced with `--replace`. The old status was
`reconstructed_ledger_backfilled_from_source`; the new one is `reconstructed_build_tested`.

The stamps are seeded by `00720180` (`BSP_CommandControllerBase_Construct`, called by the
ship director and the squadron command-block constructors) with `D01502F9h` (-1e10f).
`009555A0` (`0095561B`) is the only writer. It runs at each projectile spawn: `0072BF10` at
`0072C155` (gun shots), and `006E4D50` at `006E55D2` (rack drops).

SUBSTITUTIONS (labelled in the runner):
- The director is a units-host slot: the unit itself, or for a plane its squadron slot in the
  plane-squadron registry.
- The stamp is the latest `last_fire_seconds` (gun+474h) over the director's guns of that
  sub-type. This does not cover dual-purpose second ammunition or rack drops.
- `[00F876A4]` is `spawn_world_clock_`.

## AddWaterLoad, 00893380

Arguments: entity, integer side (`00B66290`), and a number stored as a float. No value is
pushed.

- The hull-local point:
  - x = Width (`class+A4h`) × (1 when side is odd, -1 when even) × 0.5;
  - y = 0;
  - z = Length (`class+A0h`) × (1 when side < 2, 0 when side < 4, -1 otherwise) × 0.5.
- The native does nothing unless amount > 0 and the unit is not kind 8.
- count = trunc(clamp(float(amount / 10.0), 0, 63)). The constants are `00D7A280`,
  `00CE3DC0`, `00D099A0` and `00D099A8`.
- A non-zero count builds message 90h (`0080FF80`) and routes it with flags 7 (`0077C2A0`) to
  `008221A7`. That handler calls `0074F440`, which takes the point into the world through the
  pose at `unit+CCh` (`004142E0`) before `0074F090`.

SUBSTITUTIONS: the receiver is `GameUnitsHost::add_leak_0074f440`, called on the same tick
with the point already in world space. Width and Length come from
`vehicle_class_launch_keys`.

## BreakShip, 008914F0

The argument is an entity. The native acts only when the controller `[unit+1018h]`
(`0080E490`) has both halves: `+370h` (`0092BEA0`) and `+374h` (`0092BE90`) must be
non-zero. It then calls `00814520`:
- `004A5AA0` runs when `0080F9F0` answers true.
- Then it jumps to `00935D30` `BSP_UnitController_BreakInTwo`. That function:
  - takes 11 stream-1 draws;
  - calls `0092A240` (`BSP_TickableEntityAttachFires`) unconditionally on each piece, front
    (`00936905`, ECX = EDI), then back (`0093697F`, ECX = ESI);
  - clears `+370h`/`+374h`;
  - kills the unit with `00926D90(1)` (`00936C45`).

`0092A240` works from the piece's class `[piece+348h]`, which is the owner unit's class
`[unit+538h]` (`0092B185`):
- w = float(Length `+A0h` × 0.2) (`00CE6648`, a double);
- n = trunc(clamp(float(w / 10.0), 1, 2)) (`00CE3DC0`, `00D7A24C`, `00CE3958`);
- it draws `00BD2F10(1, -w, w)` once per puff (`0092A34A`).

It draws only when both of these are non-null:
- `[class+55Ch]`, the `DeathEfx` handle (`00831BE8`);
- `[[00E188A8]+19FCh]`, the game view node.

So the image takes 11 + 0..4 draws.

No value is pushed.

SUBSTITUTIONS:
- The halves are the units host's `front_back`, minus this host's own cleared mark. A breakup
  from the death message 98h is not seen.
- The draws come from `death_mode_draw_00bd2f10(1, ...)`. They follow the image's order:
  the eleven, then the front piece's puffs, then the back piece's.
- `0092A240`'s class is the unit's VehicleClass row. A non-zero `DeathEfx` id stands for a
  resolved `+55Ch` handle. The game view node is taken as present, since it is set in any
  rendered mission.
- The gunnery host's ship death 98h records "the same eleven" draws, so it likely lacks the
  same puff draws. That is a pre-existing record and is not changed here.
- `004A5AA0` and the piece and presentation steps are recorded, not modelled.
- The kill uses the Lua `Kill` runner's funnel (`kill_unit_00926d90(unit, 1)`): the stored
  cause is 1 (harm), and the ship death message is skipped (`explicit_kill_cause` is not 0).

## UnitSetPlayerCommandsEnabled, 008A6C30 (part 7a)

Arguments: an entity (`00888AA0`), whose director (`vtable[114h]`) is fetched before the
integer mask is read. No value is pushed.

`0071DD30(director, mask)` builds message 5Ah with sub-kind 2 and routes it with flags 7
(`0077C2A0`) to `[director+34h]`. Arm 2 of `0071C1E0` then stores the mask at
`director+1CCh`.

Readers of `+1CCh` in the image:
- `0071C060` `BSP_Unit_DirectorFlagsAllow`. Its 16 callers are all HUD: the unit pick screen
  `00527260` / `00526720`, `005F9840` `BSP_HudOrder_CanAttackMove`, `005FB080` (the order
  screen idle) and the undefined chunk at `005F9620`.
- `00721280`, the network snapshot, which copies the mask.

`0071BD30` and the getter `00720E10` have no references, so no AI or simulation code reads
the mask.

The director constructor `00720180` seeds `+1CCh = 1FFh` (`007202F3`): every command is
enabled until a script narrows it. So any future host reader of this map must:
- take 0x1FF for a director with no entry;
- clear the map when a scene loads.

The runner stores the mask in a per-director map, which no host code reads, and records the
call.

SUBSTITUTIONS:
- The director is a units-host slot: the unit itself, or for a plane (kind 0Fh) its registry
  squadron slot. Unlike HasFired's runner, there is no kind-02h test before that mapping,
  because `008A6C30` has no class test. The entity goes straight to `vtable[114h]`.
- The message is applied at the call rather than on delivery.
- A unit with no director is counted and skipped. The image would send through a null
  director.

## Predicted ON effect on the WINS missions

Only BSM01, BSM09 and JM06 call HasFired, AddWaterLoad or BreakShip. JM08, USN04, USN21,
USN01, IJN08, USN15, USNOS and LOMP06 do not.

The WIN rows' scripts are as follows (`docs/CC13_WIN_READINESS.md`, `docs/CC13_VERIFY16.md`):

| Row | Script |
| --- | --- |
| USN01 | `usn/usn_1_marshall.lua` |
| USN04 | `usn/usn_19_coralus.lua` |
| USN21 | `usn/usn_sibuyan.lua` |
| USNOS | `COTP-USN/us_osumi.lua` |
| USN15 | `COTP-USN/PRCPUS/usn_15_battle_of_surigao_strait.lua` |
| JM06 | `COTP-IJN/PRCPIJN/jm06.lua` |
| JM08 | `COTP-IJN/PRCPIJN/prcpjm08.lua` |
| IJN08 | `ijn/ijn_8_north_sol.lua` |
| LOMP06 | `usn/LOMP/06_crucial_cargo.lua` |

The `COTP-USN` USN01/USN03 and similar scripts in the census below are different missions,
not these rows.

**UnitSetPlayerCommandsEnabled reach.** A grep of this installation's `scripts` finds 79 lines
in 34 files:
- 15 are commented out: the seven `traininggrounds` scripts (`PCF_RETREAT`), plus
  `prcp_13_iwojima:1133`, `usn_13_battle_of_iwo_jima:1133` (both copies),
  `usn_03_battle_of_santa_cruz:1373` (both copies), `PRCPUS/prcp_14_okinawa:1499`,
  `PRCPUS/usn_14_battle_of_okinawa:1499` and `PRCPUS/usn_09_leyte:657`.
- 64 are live calls in 26 files.

Live calls, WIN rows marked **WIN**:

| Script | Lines | Unit and mask |
| --- | --- | --- |
| **WIN BSM01** `bsm/bsm_01_stationed_at_pearl.lua` | 1797 | Henry, `PCF_NONE` |
| **WIN BSM09** `bsm/bsm_09_battle_of_the_coral_sea.lua` | 262 | Donald (a squadron), `PCF_MOVE+PCF_TARGET+PCF_JOIN_LEAVE+PCF_CHANGE_FORMATION+PCF_ORDERS+PCF_REPAIR` |
| **WIN USN04** `usn/usn_19_coralus.lua` | 460, 1947 | Lex and Town, `PCF_NONE+PCF_TARGET` |
| BSM04 `bsm/bsm_04_vengance_at_luzon.lua` | 921 | Lex, `PCF_NONE` |
| JM02 `COTP-IJN/jm02.lua` / `ijn/JM/jm02.lua` | 3404, 3406 / 3408, 3410 | `Mission.PlayerBetty`, `PCF_ALL` then `PCF_NONE`. A plane, so the squadron-slot arm is reached |
| JM04 `COTP-IJN/jm04.lua` / `ijn/JM/jm04.lua` | 164-166, 2248, 2474, 2476 / 164-166, 2241, 2467, 2469 | PlayerUnit, Shoho and a loop unit, with masks from `PCF_NONE` to `PCF_ALL` (6 calls each) |
| JM04 PRCP `COTP-IJN/PRCPIJN/jm04.lua` and `prcpjm04.lua` | 323, 505, 509 | Shoho, Shokaku, Zuikaku, `PCF_NONE+PCF_TARGET` |
| JM10 `COTP-IJN/PRCPIJN/jm10.lua` | 1227 | a loop unit, `PCF_NONE` |
| JM15 `COTP-IJN/PRCPIJN/jm15.lua` | 727, 729 | PlayerUnit, `PCF_ALL` / `PCF_NONE` |
| USN01 (COTP) `COTP-USN/PRCPUS/usn_01_battle_of_eastern_solomons.lua`, `prcp_01_eastern.lua` | 835, 1068 / 839, 1075 | Saratoga, Enterprise, `PCF_NONE+PCF_TARGET` |
| USN03 `COTP-USN/usn_03_battle_of_santa_cruz.lua` and the `PRCPUS` copy | 534, 1357, 1434, 1452, 1456, 1504 (each) | Enterprise, South Dakota and loop units |
| USN07 `COTP-USN/PRCPUS/usn_07_invasion_of_tarawa.lua` / `prcp_07_tarawa.lua` | 559, 562 / 547, 550 | CV26, CV27, `PCF_NONE` |
| USN09 `COTP-USN/usn_09_leyte.lua` | 580 | Atlanta, `PCF_NONE+PCF_TARGET` |
| USN13 `COTP-USN/usn_13_battle_of_iwo_jima.lua`, `PRCPUS/` copy, `COTP-USN/prcp_13_iwojima.lua` | 925, 1282 | loop units, `PCF_NONE+PCF_TARGET` |
| USN18 (PRCP Iwo Jima) `COTP-USN/PRCPUS/prcp_13_iwojima.lua` | 548, 1376 | loop units, `PCF_NONE+PCF_TARGET` |
| USN14 / USN19 `COTP-USN/PRCPUS/usn_14_battle_of_okinawa.lua`, `prcp_14_okinawa.lua` | 770, 1496 | loop units, `PCF_NONE+PCF_TARGET` then `PCF_ALL` |
| LOMP09 `usn/LOMP/09_samar.lua` | 363 | a loop unit, `PCF_NONE+PCF_TARGET` |

Of the WIN rows, only BSM01, BSM09 and USN04 call it. USN01's row script
(`usn_1_marshall.lua`) does not; the calls under the USN01 name are the different COTP
mission. JM06, JM08, IJN08, USN15, USN21, USNOS and LOMP06 do not call it either.

ON is identical in every mission, apart from the record line and the `+1CCh` store, because
no host code reads the mask.

- **BSM01, HasFired** (`bsm_01:813`):
  - The call is `HasFired(Henry, "DEPTHCHARGE", GameTime())`. It becomes true at Henry's
    first depth charge LAUNCH, because the stamp is set at the projectile spawn. That is
    about t=422 in `l69r_b1c_main_ref.log`: the detonation at t=425.67 (:109250) less its
    life of 3.70 s.
  - That plays the dialogs "TARGET" and "OPERATIONAL". Dialogue only.
- **BSM01, BreakShip** (`bsm_01:1192`; the script runs `ExplodeToParts`, then `BreakShip`,
  then `SetDeadMeat` on Phoenix):
  - Phoenix is killed by `00926D90(1)` in the same tick in which the OFF run's `SetDeadMeat`
    destroys it (t=117.05).
  - `SetDeadMeat` then finds Phoenix dead, so its sink is refused; it still takes its own
    pivot draw.
  - BreakShip takes 15 stream-1 draws: the eleven, then 2 + 2 puffs. Phoenix is
    `VehicleClass[252]` Brooklyn 1942, with DeathEfx 136 and Length 180, so w = 36 and
    n = trunc(clamp(3.6, 1, 2)) = 2 per piece.
  - ON reorders the draws as well as shifting them:
    - In the image, `ExplodeToParts`' `00934150` draws run at the call, before
      BreakShip's.
    - The host queues `ExplodeToParts` to its next gunnery step
      (`src/game_hosts_gunnery.cpp:15083-15087`), so BreakShip's 15 draws come first.
- **JM06, BreakShip** (`jm06:2521`, `luaDelay(luaJM6BreakShips, 9, ...)`, 9 s into the
  ambush movie):
  - The target is USTroopTransport 04, `Mission.TrgTbl[1]`, which `luaPickRnd(Mission.Cargos)`
    picks (`jm06.lua:2496`). `p23c_off_j6.log` shows the loop at `jm06.lua:2500-2505`:
    - `SetForcedReconLevel` on every live cargo (US Tanker 02, US Cargo Transport 01 and
      USTroopTransport 04, :40683-40689);
    - `AddDamage` on all but `TrgTbl[1]` (:40685 and :40688).
    - USTroopTransport 04 is the only one with no AddDamage. BreakShip is called once
      (:41951).
  - It is killed at about t=250. OFF, it dies to I-400 #2.1's torpedo at 413.82.
  - It is not a CargoTarget: the "Ambush" objective, with 3 units, completed before the call.
  - Once the Hospital Ship dies (293.65), every Cargos entry is dead, so
    `luaJM6CheckIJNSubs` has no cargo left to attack-move the subs to. The later I-400
    torpedo run and its draws change.
  - BreakShip takes 15 stream-1 draws. `VehicleClass[234]` US Troop Transport has DeathEfx
    138 and Length 180, so it gets 2 puffs per piece. They shift the shared stream.
- **BSM09, AddWaterLoad** (`bsm_09:1197-1202`, t≈39):
  - The six calls send six leaks of count 63 into Lexington's leak model.
  - With OFF, Lexington already floods, becomes a wreck at 155.00 and is killed at 255.91.
  - ON should flood it sooner. Lexington is held at its 0.01 invincibility floor until
    `luaLexSink`.
